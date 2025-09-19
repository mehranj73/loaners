#include "loanswidgets.h"
#include "ui_loanswidgets.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QDebug>
#include <QHeaderView>
#include <QSqlRelationalDelegate>

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

    // Connect to shared DB
    if (!connectionName.isEmpty() && QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::database();
    }

    // -------------------------
    // Borrower model & table
    // -------------------------
    borrowerModel = new QSqlTableModel(this, db);
    borrowerModel->setTable("persons");
    borrowerModel->select();

    borrowerProxy = new QSortFilterProxyModel(this);
    borrowerProxy->setSourceModel(borrowerModel);
    borrowerProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    borrowerProxy->setFilterKeyColumn(1); // assume column 1 = "name"

    ui->borrowerTable->setModel(borrowerProxy);
    ui->borrowerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->borrowerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->borrowerTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->searchBorrower, &QLineEdit::textChanged,
            this, &LoansWidgets::filterBorrowers);

    connect(ui->borrowerTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() {
        QModelIndex index = ui->borrowerTable->currentIndex();
        if (!index.isValid()) return;
        QModelIndex src = borrowerProxy->mapToSource(index);
        selectedBorrowerId = borrowerModel->data(borrowerModel->index(src.row(), 0)).toInt();
        QString name = borrowerModel->data(borrowerModel->index(src.row(), 1)).toString();
        ui->borrowerSelectedLabel->setText(QStringLiteral("وام‌گیرنده انتخاب‌شده: ") + name);
    });

    // -------------------------
    // Guarantor model & table
    // -------------------------
    guarantorModel = new QSqlTableModel(this, db);
    guarantorModel->setTable("persons");
    guarantorModel->select();

    guarantorProxy = new QSortFilterProxyModel(this);
    guarantorProxy->setSourceModel(guarantorModel);
    guarantorProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    guarantorProxy->setFilterKeyColumn(1); // assume column 1 = "name"

    ui->guarantorTable->setModel(guarantorProxy);
    ui->guarantorTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->guarantorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->guarantorTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->searchGuarantor, &QLineEdit::textChanged,
            this, &LoansWidgets::filterGuarantors);

    connect(ui->guarantorTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() {
        QModelIndex index = ui->guarantorTable->currentIndex();
        if (!index.isValid()) return;
        QModelIndex src = guarantorProxy->mapToSource(index);
        selectedGuarantorId = guarantorModel->data(guarantorModel->index(src.row(), 0)).toInt();
        QString name = guarantorModel->data(guarantorModel->index(src.row(), 1)).toString();
        ui->guarantorSelectedLabel->setText(QStringLiteral("ضامن انتخاب‌شده: ") + name);
    });

    // -------------------------
    // Loan model & table
    // -------------------------
    loanModel = new QSqlRelationalTableModel(this, db);
    loanModel->setTable("loans");
    loanModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    // Show borrower/guarantor names instead of IDs
    loanModel->setRelation(
        loanModel->fieldIndex("borrower_id"),
        QSqlRelation("persons", "id", "name")
    );

    loanModel->setRelation(
        loanModel->fieldIndex("guarantor_id"),
        QSqlRelation("persons", "id", "name")
    );

    loanModel->select();

    ui->loanTable->setModel(loanModel);
    ui->loanTable->setItemDelegate(new QSqlRelationalDelegate(ui->loanTable));
    ui->loanTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->loanTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->loanTable->horizontalHeader()->setStretchLastSection(true);

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
        loanModel->select(); // refresh table
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
