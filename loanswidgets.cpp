#include "loanswidgets.h"
#include "ui_loanswidgets.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QDebug>
#include <QHeaderView>
#include <QSqlRelationalDelegate>
#include <QSortFilterProxyModel>

LoansWidgets::LoansWidgets(QWidget *parent, const QString &connectionName) :
    QWidget(parent),
    ui(new Ui::LoansWidgets),
    borrowerModel(nullptr),
    guarantorModel(nullptr),
    borrowerProxy(nullptr),
    guarantorProxy(nullptr),
    loanModel(nullptr),
    loanProxy(nullptr),
    selectedBorrowerId(-1),
    selectedGuarantorId(-1)
{
    ui->setupUi(this);

    this->setLayoutDirection(Qt::RightToLeft);

    if (!connectionName.isEmpty() && QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::database();
    }

    // ---------------- Borrower ----------------
    borrowerModel = new QSqlTableModel(this, db);
    borrowerModel->setTable("persons");
    borrowerModel->select();

    borrowerProxy = new QSortFilterProxyModel(this);
    borrowerProxy->setSourceModel(borrowerModel);
    borrowerProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    borrowerProxy->setFilterKeyColumn(1); // name column

    ui->borrowerTable->setModel(borrowerProxy);
    ui->borrowerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->borrowerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->borrowerTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->searchBorrower, &QLineEdit::textChanged, this, &LoansWidgets::filterBorrowers);

    connect(ui->borrowerTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() {
        QModelIndex index = ui->borrowerTable->currentIndex();
        if (!index.isValid()) return;
        QModelIndex src = borrowerProxy->mapToSource(index);
        selectedBorrowerId = borrowerModel->data(borrowerModel->index(src.row(), 0)).toInt();
        QString name = borrowerModel->data(borrowerModel->index(src.row(), 1)).toString();
        ui->borrowerSelectedLabel->setText(QStringLiteral("وام‌گیرنده انتخاب‌شده: ") + name);
    });

    // ---------------- Guarantor ----------------
    guarantorModel = new QSqlTableModel(this, db);
    guarantorModel->setTable("persons");
    guarantorModel->select();

    guarantorProxy = new QSortFilterProxyModel(this);
    guarantorProxy->setSourceModel(guarantorModel);
    guarantorProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    guarantorProxy->setFilterKeyColumn(1);

    ui->guarantorTable->setModel(guarantorProxy);
    ui->guarantorTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->guarantorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->guarantorTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->searchGuarantor, &QLineEdit::textChanged, this, &LoansWidgets::filterGuarantors);

    connect(ui->guarantorTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() {
        QModelIndex index = ui->guarantorTable->currentIndex();
        if (!index.isValid()) return;
        QModelIndex src = guarantorProxy->mapToSource(index);
        selectedGuarantorId = guarantorModel->data(guarantorModel->index(src.row(), 0)).toInt();
        QString name = guarantorModel->data(guarantorModel->index(src.row(), 1)).toString();
        ui->guarantorSelectedLabel->setText(QStringLiteral("ضامن انتخاب‌شده: ") + name);
    });

    // ---------------- Loan ----------------
    loanModel = new QSqlRelationalTableModel(this, db);
    loanModel->setTable("loans");
    loanModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    loanModel->setRelation(loanModel->fieldIndex("borrower_id"),
                           QSqlRelation("persons", "id", "name"));
    loanModel->setRelation(loanModel->fieldIndex("guarantor_id"),
                           QSqlRelation("persons", "id", "name"));
    loanModel->select();

    // Proxy for search
    loanProxy = new QSortFilterProxyModel(this);
    loanProxy->setSourceModel(loanModel);
    loanProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->loanTable->setModel(loanProxy);
    ui->loanTable->setItemDelegate(new QSqlRelationalDelegate(ui->loanTable));
    ui->loanTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->loanTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->loanTable->horizontalHeader()->setStretchLastSection(true);

    // Hide ID columns
    ui->loanTable->hideColumn(loanModel->fieldIndex("id"));
    ui->loanTable->hideColumn(loanModel->fieldIndex("borrower_id")); // already shown as name
    ui->loanTable->hideColumn(loanModel->fieldIndex("guarantor_id")); // shown as name

    // Set Persian headers
    loanModel->setHeaderData(loanModel->fieldIndex("amount"), Qt::Horizontal, "مبلغ");
    loanModel->setHeaderData(loanModel->fieldIndex("percentage"), Qt::Horizontal, "درصد سود");
    loanModel->setHeaderData(loanModel->fieldIndex("description"), Qt::Horizontal, "توضیحات");
    loanModel->setHeaderData(loanModel->fieldIndex("date"), Qt::Horizontal, "تاریخ");
    loanModel->setHeaderData(loanModel->fieldIndex("borrower_id"), Qt::Horizontal, "وام‌گیرنده");
    loanModel->setHeaderData(loanModel->fieldIndex("guarantor_id"), Qt::Horizontal, "ضامن");

    connect(ui->searchLoan, &QLineEdit::textChanged, this, &LoansWidgets::filterLoans);

    connect(ui->addLoanButton, &QPushButton::clicked, this, &LoansWidgets::addLoan);
}

LoansWidgets::~LoansWidgets() {
    delete ui;
}

void LoansWidgets::addLoan() {
    if (selectedBorrowerId < 0) {
        QMessageBox::warning(this, "خطا", "لطفا وام‌گیرنده را انتخاب کنید.");
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
    if (selectedGuarantorId > 0)
        q.addBindValue(selectedGuarantorId);
    else
        q.addBindValue(QVariant(QVariant::Int));
    q.addBindValue(amount);
    q.addBindValue(percent);
    q.addBindValue(desc);
    q.addBindValue(date);

    if (!q.exec())
        QMessageBox::critical(this, "خطا در DB", q.lastError().text());
    else
        loanModel->select();
}

void LoansWidgets::filterBorrowers(const QString &text) {
    borrowerProxy->setFilterWildcard("*" + text + "*");
}

void LoansWidgets::filterGuarantors(const QString &text) {
    guarantorProxy->setFilterWildcard("*" + text + "*");
}

void LoansWidgets::filterLoans(const QString &text) {
    loanProxy->setFilterWildcard("*" + text + "*");
}
