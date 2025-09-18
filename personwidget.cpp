#include "PersonWidget.h"
#include "ui_personwidget.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QRegularExpression>



#include <QProxyStyle>
#include <QStyleOption>
#include <QLineEdit>
#include <QPainter>



#include <QLineEdit>
#include <QFocusEvent>



class PaddingDelegate : public QStyledItemDelegate {
public:
    PaddingDelegate(int left, int top, int right, int bottom, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), m_left(left), m_top(top), m_right(right), m_bottom(bottom) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        opt.rect.adjust(m_left, m_top, -m_right, -m_bottom); // shrink rect for padding
        QStyledItemDelegate::paint(painter, opt, index);
    }

private:
    int m_left, m_top, m_right, m_bottom;
};


// Combo delegate for score column
class ComboBoxDelegate : public QStyledItemDelegate {
public:
    explicit ComboBoxDelegate(const QStringList &items, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), m_items(items) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override {
        QComboBox *editor = new QComboBox(parent);
        editor->addItems(m_items);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QComboBox *combo = qobject_cast<QComboBox*>(editor);
        if (!combo) return;
        QString cur = index.data(Qt::EditRole).toString();
        int idx = combo->findText(cur.isEmpty() ? "هیچکدام" : cur);
        combo->setCurrentIndex(idx >= 0 ? idx : 0);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QComboBox *combo = qobject_cast<QComboBox*>(editor);
        if (!combo) return;
        QString val = combo->currentText();
        model->setData(index, val == "هیچکدام" ? QVariant(QVariant::String) : val);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override {
        editor->setGeometry(option.rect);
    }

private:
    QStringList m_items;
};

// --- PersonWidget Implementation ---

PersonWidget::PersonWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::PersonWidget), model(nullptr), proxyModel(nullptr)
{
    ui->setupUi(this);
    ui->gridLayout->setHorizontalSpacing(10);
    ui->gridLayout->setVerticalSpacing(15);
    setupDatabase();

    ui->tableView->setItemDelegate(new PaddingDelegate(5, 2, 5, 2, this));

    // --- Model ---
    model = new QSqlTableModel(this, db);
    model->setTable("persons");
    model->setEditStrategy(QSqlTableModel::OnFieldChange); // live updates
    model->select();
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Set Persian headers
    model->setHeaderData(1, Qt::Horizontal, "نام");
    model->setHeaderData(2, Qt::Horizontal, "شماره ملی");
    model->setHeaderData(3, Qt::Horizontal, "شغل");
    model->setHeaderData(4, Qt::Horizontal, "نمره (اختیاری)");


    ui->tableView->setLayoutDirection(Qt::RightToLeft);


    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableView->verticalHeader()->setVisible(false); // hide row numbers if desired

    // --- Proxy model for live filtering ---
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1); // all columns

    ui->tableView->setModel(proxyModel);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::EditKeyPressed |
        QAbstractItemView::SelectedClicked
    );

    // Hide id column
    int idCol = model->fieldIndex("id");
    if (idCol >= 0) ui->tableView->hideColumn(idCol);

    // Delegate for score column
    int scoreCol = model->fieldIndex("score");
    if (scoreCol >= 0) {
        QStringList scores = {"هیچکدام","A1","A2","A3","B1","B2","B3","C1","C2","C3","D1","D2","D3","E1","E2","E3"};
        ui->tableView->setItemDelegateForColumn(scoreCol, new ComboBoxDelegate(scores, this));
    }

    // --- Signals ---
    connect(ui->addButton, &QPushButton::clicked, this, &PersonWidget::addPerson);
    connect(ui->deleteButton, &QPushButton::clicked, this, &PersonWidget::deletePerson);

    // Live search filter
    connect(ui->searchEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        QRegularExpression regex(text, QRegularExpression::CaseInsensitiveOption);
        proxyModel->setFilterRegularExpression(regex);
    });

    // SSN validation during edit
    connect(model, &QSqlTableModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &){
        int ssnCol = model->fieldIndex("ssn");
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            QString ssn = model->data(model->index(row, ssnCol)).toString();
            QSqlQuery query;
            query.prepare("SELECT COUNT(*) FROM persons WHERE ssn = ? AND id != ?");
            int id = model->data(model->index(row, model->fieldIndex("id"))).toInt();
            query.addBindValue(ssn);
            query.addBindValue(id);
            query.exec();
            if (query.next() && query.value(0).toInt() > 0) {
                QMessageBox::warning(this, "خطا", "این شماره ملی قبلاً استفاده شده است.");
                model->revertRow(row);
            }
        }
    });
}

PersonWidget::~PersonWidget() { delete ui; }

void PersonWidget::setupDatabase()
{
    // Use existing default connection if present, otherwise create the default SQLite DB
    const QString defaultConn = QSqlDatabase::defaultConnection;
    if (QSqlDatabase::contains(defaultConn)) {
        db = QSqlDatabase::database(defaultConn);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("people.db");
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            QMessageBox::critical(this, "خطای پایگاه داده", db.lastError().text());
            return;
        }
    }

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS persons ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "name TEXT NOT NULL,"
                    "ssn TEXT NOT NULL UNIQUE,"
                    "job TEXT,"
                    "score TEXT NULL)"))
    {
        QMessageBox::critical(this, "خطای ایجاد جدول", query.lastError().text());
    }
}

void PersonWidget::addPerson()
{
    QString name = ui->nameEdit->text().trimmed();
    QString ssn = ui->ssnEdit->text().trimmed();
    QString job = ui->jobEdit->text().trimmed();
    QString score = ui->scoreCombo->currentText();
    if (score == "هیچکدام") score.clear();

    if (name.isEmpty() || ssn.isEmpty()) {
        QMessageBox::warning(this, "اعتبارسنجی", "نام و شماره ملی الزامی هستند.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO persons (name, ssn, job, score) VALUES (?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(ssn);
    query.addBindValue(job);
    query.addBindValue(score.isEmpty() ? QVariant(QVariant::String) : score);

    if (!query.exec()) {
        QMessageBox::critical(this, "خطای درج", query.lastError().text());
        return;
    }

    model->select();
    ui->nameEdit->clear();
    ui->ssnEdit->clear();
    ui->jobEdit->clear();
    ui->scoreCombo->setCurrentIndex(0);
}

void PersonWidget::deletePerson()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, "حذف", "هیچ رکوردی انتخاب نشده است.");
        return;
    }

    int ret = QMessageBox::question(this, "حذف", "آیا مطمئن هستید می‌خواهید رکوردهای انتخاب‌شده حذف شوند؟",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    for (const QModelIndex &proxyIndex : selection) {
        QModelIndex index = proxyModel->mapToSource(proxyIndex);
        model->removeRow(index.row());
    }

    if (!model->submitAll()) {
        QMessageBox::critical(this, "خطای حذف", model->lastError().text());
    } else {
        model->select();
    }
}
