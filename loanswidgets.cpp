#include "loanswidgets.h"
#include "ui_loanswidgets.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QDate>

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

    if (connectionName.isEmpty()) {
        db = QSqlDatabase::database(); // default connection
    } else {
        db = QSqlDatabase::database(connectionName);
    }

    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(this, "Database error", "Database is not open. Please open the database before creating this widget.");
        return;
    }

    setupModels();

    // Connect search fields
    connect(ui->searchBorrower, &QLineEdit::textChanged, this, &LoansWidgets::filterBorrowers);
    connect(ui->searchGuarantor, &QLineEdit::textChanged, this, &LoansWidgets::filterGuarantors);
    connect(ui->searchLoan, &QLineEdit::textChanged, this, &LoansWidgets::filterLoans);

    // Add loan
    connect(ui->addLoanButton, &QPushButton::clicked, this, &LoansWidgets::addLoan);

    // Borrower selection
    connect(ui->borrowerTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &LoansWidgets::borrowerSelected);

    // Guarantor selection (multi)
    connect(ui->guarantorTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &LoansWidgets::guarantorSelectionChanged);

    loadLoans();
}

LoansWidgets::~LoansWidgets() {
    delete ui;
}

void LoansWidgets::setupModels()
{
    // Borrowers
    borrowerModel = new QSqlTableModel(this, db);
    borrowerModel->setTable("persons");
    borrowerModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    borrowerModel->select();

    borrowerProxy = new QSortFilterProxyModel(this);
    borrowerProxy->setSourceModel(borrowerModel);
    borrowerProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->borrowerTable->setModel(borrowerProxy);
    ui->borrowerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->borrowerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->borrowerTable->horizontalHeader()->setStretchLastSection(true);

    // Guarantors
    guarantorModel = new QSqlTableModel(this, db);
    guarantorModel->setTable("persons");
    guarantorModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    guarantorModel->select();

    guarantorProxy = new QSortFilterProxyModel(this);
    guarantorProxy->setSourceModel(guarantorModel);
    guarantorProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->guarantorTable->setModel(guarantorProxy);
    ui->guarantorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->guarantorTable->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->guarantorTable->horizontalHeader()->setStretchLastSection(true);

    // Loans
    loanModel = new QSqlQueryModel(this);
    loanProxy = new QSortFilterProxyModel(this);
    loanProxy->setSourceModel(loanModel);
    loanProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->loanTable->setModel(loanProxy);
    ui->loanTable->horizontalHeader()->setStretchLastSection(true);
}

void LoansWidgets::loadLoans()
{
    QString q = QStringLiteral(
        "SELECT l.id, p.name AS borrower, l.amount, l.percentage, l.description, l.date "
        "FROM loans l "
        "LEFT JOIN persons p ON p.id = l.borrower_id "
        "ORDER BY l.date DESC"
    );
    loanModel->setQuery(q, db);
    if (loanModel->lastError().isValid()) {
        qDebug() << "Failed to load loans:" << loanModel->lastError().text();
    }
}

void LoansWidgets::borrowerSelected(const QModelIndex &index)
{
    if (!index.isValid()) {
        selectedBorrowerId = -1;
        return;
    }
    QModelIndex source = borrowerProxy->mapToSource(index);
    QVariant id = borrowerModel->data(borrowerModel->index(source.row(), 0));
    selectedBorrowerId = id.isValid() ? id.toInt() : -1;
}

void LoansWidgets::guarantorSelectionChanged()
{
    selectedGuarantorIds.clear();
    QItemSelectionModel *sel = ui->guarantorTable->selectionModel();
    QModelIndexList indexes = sel->selectedRows();
    for (const QModelIndex &proxyIndex : indexes) {
        QModelIndex sourceIndex = guarantorProxy->mapToSource(proxyIndex);
        QVariant id = guarantorModel->data(guarantorModel->index(sourceIndex.row(), 0));
        if (id.isValid()) selectedGuarantorIds.append(id.toInt());
    }
}

void LoansWidgets::addLoan()
{
    if (selectedBorrowerId < 0) {
        QMessageBox::warning(this, "خطا", "لطفا وام‌گیرنده را انتخاب کنید.");
        return;
    }

    bool okA=false, okP=false;
    double amount = ui->amountEdit->text().toDouble(&okA);
    double percent = ui->percentEdit->text().toDouble(&okP);
    QString desc = ui->descEdit->text().trimmed();
    QString date = ui->dateEdit->date().toString("yyyy-MM-dd");

    if (!okA || amount <= 0.0) {
        QMessageBox::warning(this, "خطا", "مبلغ نامعتبر است.");
        return;
    }

    // Insert loan
    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO loans (borrower_id, amount, percentage, description, date)
        VALUES (?, ?, ?, ?, ?)
    )");
    q.addBindValue(selectedBorrowerId);
    q.addBindValue(amount);
    q.addBindValue(percent);
    q.addBindValue(desc);
    q.addBindValue(date);

    if (!q.exec()) {
        QMessageBox::critical(this, "خطا هنگام درج وام", q.lastError().text());
        return;
    }

    int loanId = 0;
    QVariant lastId = q.lastInsertId();
    if (lastId.isValid()) {
        loanId = lastId.toInt();
    } else {
        QSqlQuery getId(db);
        if (db.driverName().toLower().contains("sqlite")) {
            if (getId.exec("SELECT last_insert_rowid()") && getId.next())
                loanId = getId.value(0).toInt();
        }
    }

    if (loanId > 0) {
        QSqlQuery g(db);
        g.prepare("INSERT INTO loan_guarantors (loan_id, person_id) VALUES (?, ?)");
        for (int pid : selectedGuarantorIds) {
            g.addBindValue(loanId);
            g.addBindValue(pid);
            if (!g.exec()) {
                qDebug() << "Failed to insert guarantor:" << g.lastError().text();
            }
        }
    }

    loadLoans();

    ui->amountEdit->clear();
    ui->percentEdit->clear();
    ui->descEdit->clear();
    ui->dateEdit->setDate(QDate::currentDate());
    ui->borrowerTable->clearSelection();
    ui->guarantorTable->clearSelection();
    selectedBorrowerId = -1;
    selectedGuarantorIds.clear();
}

void LoansWidgets::onLoanSelected()
{
    QModelIndex idx = ui->loanTable->currentIndex();
    if (!idx.isValid()) return;

    int row = loanProxy->mapToSource(idx).row();
    int loanId = loanModel->data(loanModel->index(row, 0)).toInt();

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT l.id, l.amount, l.percentage, l.description, l.date,
               b.name as borrower
        FROM loans l
        LEFT JOIN persons b ON b.id = l.borrower_id
        WHERE l.id = ?
    )");
    q.addBindValue(loanId);

    QString details;
    if (q.exec() && q.next()) {
        details += QString("شناسه وام: %1\n").arg(q.value("id").toInt());
        details += QString("وام‌گیرنده: %1\n").arg(q.value("borrower").toString());
        details += QString("مبلغ: %1\n").arg(q.value("amount").toDouble());
        details += QString("درصد سود: %1%%\n").arg(q.value("percentage").toDouble());
        details += QString("تاریخ: %1\n").arg(q.value("date").toString());
        details += QString("توضیحات: %1\n").arg(q.value("description").toString());
    }

    QSqlQuery g(db);
    g.prepare(R"(
        SELECT p.name
        FROM loan_guarantors lg
        JOIN persons p ON p.id = lg.person_id
        WHERE lg.loan_id = ?
    )");
    g.addBindValue(loanId);

    QStringList guarantors;
    if (g.exec()) {
        while (g.next()) guarantors << g.value(0).toString();
    }

    details += "\nضامن‌ها: " + (guarantors.isEmpty() ? "هیچ‌کدام" : guarantors.join(", "));

  //  ui->textLoanDetails->setPlainText(details);
}
