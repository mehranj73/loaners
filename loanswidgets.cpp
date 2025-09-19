#include "loanswidgets.h"
#include "ui_loanswidgets.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QHeaderView>

LoansWidgets::LoansWidgets(QWidget *parent, const QString &connectionName) :
    QWidget(parent),
    ui(new Ui::LoansWidgets),
    borrowerModel(nullptr),
    guarantorModel(nullptr),
    borrowerProxy(nullptr),
    guarantorProxy(nullptr),
    loanModel(nullptr),
    loanProxy(nullptr),
    selectedBorrowerId(-1)
{
    ui->setupUi(this);
    this->setLayoutDirection(Qt::RightToLeft);

    // Database
    if (!connectionName.isEmpty() && QSqlDatabase::contains(connectionName))
        db = QSqlDatabase::database(connectionName);
    else
        db = QSqlDatabase::database();

    // ---------------- Borrowers ----------------
    borrowerModel = new QSqlTableModel(this, db);
    borrowerModel->setTable("persons");
    borrowerModel->select();

    borrowerProxy = new QSortFilterProxyModel(this);
    borrowerProxy->setSourceModel(borrowerModel);
    borrowerProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    borrowerProxy->setFilterKeyColumn(1); // Name column

    ui->borrowerTable->setModel(borrowerProxy);
    ui->borrowerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->borrowerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->borrowerTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->searchBorrower, &QLineEdit::textChanged, this, &LoansWidgets::filterBorrowers);

    connect(ui->borrowerTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
        QModelIndex index = ui->borrowerTable->currentIndex();
        if (!index.isValid()) return;
        QModelIndex src = borrowerProxy->mapToSource(index);
        selectedBorrowerId = borrowerModel->data(borrowerModel->index(src.row(), 0)).toInt();
        QString name = borrowerModel->data(borrowerModel->index(src.row(), 1)).toString();
        ui->borrowerSelectedLabel->setText(QStringLiteral("وام‌گیرنده انتخاب‌شده: ") + name);
    });

    // ---------------- Guarantors ----------------
    guarantorModel = new QSqlTableModel(this, db);
    guarantorModel->setTable("persons");
    guarantorModel->select();

    guarantorProxy = new QSortFilterProxyModel(this);
    guarantorProxy->setSourceModel(guarantorModel);
    guarantorProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    guarantorProxy->setFilterKeyColumn(1);

    ui->guarantorTable->setModel(guarantorProxy);
    ui->guarantorTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->guarantorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->guarantorTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->searchGuarantor, &QLineEdit::textChanged, this, &LoansWidgets::filterGuarantors);

    connect(ui->guarantorTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
        selectedGuarantorIds.clear();
        QStringList names;
        for (const QModelIndex &index : ui->guarantorTable->selectionModel()->selectedRows()) {
            QModelIndex src = guarantorProxy->mapToSource(index);
            int id = guarantorModel->data(guarantorModel->index(src.row(), 0)).toInt();
            selectedGuarantorIds.append(id);
            QString name = guarantorModel->data(guarantorModel->index(src.row(), 1)).toString();
            names.append(name);
        }
        ui->guarantorSelectedLabel->setText(QStringLiteral("ضامن‌های انتخاب‌شده: ") + names.join(", "));
    });

    // ---------------- Loans ----------------
    loanModel = new QSqlQueryModel(this);
    loanProxy = new QSortFilterProxyModel(this);
    loanProxy->setSourceModel(loanModel);
    loanProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->loanTable->setModel(loanProxy);
    ui->loanTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->searchLoan, &QLineEdit::textChanged, this, &LoansWidgets::filterLoans);

    connect(ui->addLoanButton, &QPushButton::clicked, this, &LoansWidgets::addLoan);

    loadLoans();
}

LoansWidgets::~LoansWidgets() {
    delete ui;
}

// ---------------- Load Loans ----------------
void LoansWidgets::loadLoans() {
    QString queryStr =
        "SELECT l.id, p.name AS borrower, "
        "IFNULL(GROUP_CONCAT(gp.name, ', '), '') AS guarantors, "
        "l.amount, l.percentage, l.description, l.date "
        "FROM loans l "
        "JOIN persons p ON l.borrower_id = p.id "
        "LEFT JOIN loan_guarantors lg ON l.id = lg.loan_id "
        "LEFT JOIN persons gp ON lg.person_id = gp.id "
        "GROUP BY l.id "
        "ORDER BY l.id DESC;";

    loanModel->setQuery(queryStr, db);

    // Persian headers
    loanModel->setHeaderData(1, Qt::Horizontal, "وام‌گیرنده");
    loanModel->setHeaderData(2, Qt::Horizontal, "ضامن‌ها");
    loanModel->setHeaderData(3, Qt::Horizontal, "مبلغ");
    loanModel->setHeaderData(4, Qt::Horizontal, "درصد سود");
    loanModel->setHeaderData(5, Qt::Horizontal, "توضیحات");
    loanModel->setHeaderData(6, Qt::Horizontal, "تاریخ");

    ui->loanTable->hideColumn(0); // hide loan ID
}

// ---------------- Add Loan ----------------
void LoansWidgets::addLoan() {
    if (selectedBorrowerId < 0) {
        QMessageBox::warning(this, "خطا", "لطفا وام‌گیرنده را انتخاب کنید.");
        return;
    }

    double amount = ui->amountEdit->text().toDouble();
    double percent = ui->percentEdit->text().toDouble();
    QString desc = ui->descEdit->text();
    QString date = ui->dateEdit->date().toString("yyyy-MM-dd");

    // Insert loan
    QSqlQuery q(db);
    q.prepare("INSERT INTO loans (borrower_id, amount, percentage, description, date) VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(selectedBorrowerId);
    q.addBindValue(amount);
    q.addBindValue(percent);
    q.addBindValue(desc);
    q.addBindValue(date);

    if (!q.exec()) {
        QMessageBox::critical(this, "خطا", q.lastError().text());
        return;
    }

    // ---------------- Get last inserted loan ID (SQLite-safe) ----------------
    QSqlQuery getId(db);
    if (!getId.exec("SELECT last_insert_rowid()")) {
        QMessageBox::critical(this, "خطا", getId.lastError().text());
        return;
    }
    int loanId = 0;
    if (getId.next()) {
        loanId = getId.value(0).toInt();
    }

    // ---------------- Insert multiple guarantors safely ----------------
    QSqlQuery g(db);
    g.prepare("INSERT INTO loan_guarantors (loan_id, person_id) VALUES (?, ?)");
    for (int id : selectedGuarantorIds) {
        g.bindValue(0, loanId); // loan_id
        g.bindValue(1, id);     // guarantor_id
        if (!g.exec()) {
            qDebug() << "Failed to insert guarantor:" << g.lastError().text();
        }
    }

    loadLoans(); // refresh loan table
}

// ---------------- Filter Functions ----------------
void LoansWidgets::filterBorrowers(const QString &text) {
    borrowerProxy->setFilterWildcard("*" + text + "*");
}

void LoansWidgets::filterGuarantors(const QString &text) {
    guarantorProxy->setFilterWildcard("*" + text + "*");
}

void LoansWidgets::filterLoans(const QString &text) {
    loanProxy->setFilterWildcard("*" + text + "*");
}
