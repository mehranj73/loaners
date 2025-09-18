
#include "loanswidgets.h"
#include "ui_loanswidgets.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QDebug>

LoansWidgets::LoansWidgets(QWidget *parent, const QString &connectionName) :
    QWidget(parent),
    ui(new Ui::LoansWidgets),
    borrowerModel(nullptr),
    guarantorModel(nullptr),
    borrowerProxy(nullptr),
    guarantorProxy(nullptr),
    loanModel(nullptr),
    selectedBorrowerId(-1),
    selectedGuarantorId(-1)
{
    ui->setupUi(this);

    // RTL layout for Persian
    this->setLayoutDirection(Qt::RightToLeft);

    // connect to shared DB
    if (!connectionName.isEmpty() && QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::database();
    }

    // borrower model
    borrowerModel = new QSqlTableModel(this, db);
    borrowerModel->setTable("persons");
    borrowerModel->select();

    borrowerProxy = new QSortFilterProxyModel(this);
    borrowerProxy->setSourceModel(borrowerModel);
    borrowerProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->borrowerTable->setModel(borrowerProxy);
    ui->borrowerTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->searchBorrower, &QLineEdit::textChanged,
            this, &LoansWidgets::filterBorrowers);

    connect(ui->borrowerTable, &QTableView::doubleClicked,
            this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;
        QModelIndex src = borrowerProxy->mapToSource(index);
        selectedBorrowerId = borrowerModel->data(borrowerModel->index(src.row(), 0)).toInt();
        QString name = borrowerModel->data(borrowerModel->index(src.row(), 1)).toString();
        ui->borrowerSelectedLabel->setText(QStringLiteral("وام‌گیرنده انتخاب‌شده: ") + name);
    });

    // guarantor model
    guarantorModel = new QSqlTableModel(this, db);
    guarantorModel->setTable("persons");
    guarantorModel->select();

    guarantorProxy = new QSortFilterProxyModel(this);
    guarantorProxy->setSourceModel(guarantorModel);
    guarantorProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->guarantorTable->setModel(guarantorProxy);
    ui->guarantorTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->searchGuarantor, &QLineEdit::textChanged,
            this, &LoansWidgets::filterGuarantors);

    connect(ui->guarantorTable, &QTableView::doubleClicked,
            this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;
        QModelIndex src = guarantorProxy->mapToSource(index);
        selectedGuarantorId = guarantorModel->data(guarantorModel->index(src.row(), 0)).toInt();
        QString name = guarantorModel->data(guarantorModel->index(src.row(), 1)).toString();
        ui->guarantorSelectedLabel->setText(QStringLiteral("ضامن انتخاب‌شده: ") + name);
    });

    // loan table model
    loanModel = new QSqlRelationalTableModel(this, db);
    loanModel->setTable("loans");
    loanModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    loanModel->select();

    ui->loanTable->setModel(loanModel);

    connect(ui->addLoanButton, &QPushButton::clicked,
            this, &LoansWidgets::addLoan);
}

LoansWidgets::~LoansWidgets()
{
    delete ui;
}

void LoansWidgets::addLoan()
{
    if (selectedBorrowerId < 0) {
        QMessageBox::warning(this, QStringLiteral("خطا"), QStringLiteral("لطفا وام‌گیرنده را انتخاب کنید."));
        return;
    }

    double amount = ui->amountEdit->text().toDouble();
    double percent = ui->percentEdit->text().toDouble();
    QString desc = ui->descEdit->text();
    QString date = ui->dateEdit->date().toString("yyyy-MM-dd");

    QSqlQuery q(db);
    q.prepare("INSERT INTO loans (borrower_id, guarantor_id, amount, percentage, description, date) "
              "VALUES (?, ?, ?, ?, ?, ?)");
    q.addBindValue(selectedBorrowerId);
    if (selectedGuarantorId > 0) {
        q.addBindValue(selectedGuarantorId);
    } else {
        q.addBindValue(QVariant(QVariant::Int));
    }
    q.addBindValue(amount);
    q.addBindValue(percent);
    q.addBindValue(desc);
    q.addBindValue(date);

    if (!q.exec()) {
        QMessageBox::critical(this, "DB Error", q.lastError().text());
    } else {
        loanModel->select();
    }
}

void LoansWidgets::filterBorrowers(const QString &text)
{
    borrowerProxy->setFilterWildcard("*" + text + "*");
}

void LoansWidgets::filterGuarantors(const QString &text)
{
    guarantorProxy->setFilterWildcard("*" + text + "*");
}
