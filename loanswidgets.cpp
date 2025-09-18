#include "loanswidgets.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QDateEdit>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QDebug>
#include <QStandardItemModel>

LoansWidgets::LoansWidgets(QWidget *parent, const QString &connectionName)
    : QWidget(parent)
{
    // database connection
    if (!connectionName.isEmpty() && QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::database();
    }

    // --- UI layout ---
    mainLayout = new QVBoxLayout(this);

    // Borrower selector
    QLabel *bLabel = new QLabel(QString::fromUtf8("جستجوی وام‌گیرنده:"), this);
    searchBorrower = new QLineEdit(this);
    searchBorrower->setPlaceholderText(QString::fromUtf8("نام/شماره ملی/شغل را وارد کنید..."));
    borrowerTable = new QTableView(this);
    borrowerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    borrowerTable->horizontalHeader()->setStretchLastSection(true);

    mainLayout->addWidget(bLabel);
    mainLayout->addWidget(searchBorrower);
    mainLayout->addWidget(borrowerTable);

    // Guarantor selector
    QLabel *gLabel = new QLabel(QString::fromUtf8("جستجوی ضامن:"), this);
    searchGuarantor = new QLineEdit(this);
    searchGuarantor->setPlaceholderText(QString::fromUtf8("نام/شماره ملی/شغل را وارد کنید..."));
    guarantorTable = new QTableView(this);
    guarantorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    guarantorTable->setSelectionMode(QAbstractItemView::SingleSelection);
    guarantorTable->horizontalHeader()->setStretchLastSection(true);

    mainLayout->addWidget(gLabel);
    mainLayout->addWidget(searchGuarantor);
    mainLayout->addWidget(guarantorTable);

    // Loan form
    QHBoxLayout *formLayout = new QHBoxLayout();
    QLabel *amountLabel = new QLabel(QString::fromUtf8("مبلغ:"), this);
    amountEdit = new QLineEdit(this);
    amountEdit->setPlaceholderText(QString::fromUtf8("مقدار پول"));

    QLabel *percLabel = new QLabel(QString::fromUtf8("درصد وام:"), this);
    percentageEdit = new QLineEdit(this);
    percentageEdit->setPlaceholderText(QString::fromUtf8("مثلا 5.5"));

    QLabel *descLabel = new QLabel(QString::fromUtf8("توضیحات:"), this);
    descEdit = new QLineEdit(this);
    descEdit->setPlaceholderText(QString::fromUtf8("شرح وام"));

    QLabel *dateLabel = new QLabel(QString::fromUtf8("تاریخ:"), this);
    dateEdit = new QDateEdit(QDate::currentDate(), this);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");

    addLoanButton = new QPushButton(QString::fromUtf8("افزودن وام"), this);

    formLayout->addWidget(amountLabel);
    formLayout->addWidget(amountEdit);
    formLayout->addWidget(percLabel);
    formLayout->addWidget(percentageEdit);
    formLayout->addWidget(descLabel);
    formLayout->addWidget(descEdit);
    formLayout->addWidget(dateLabel);
    formLayout->addWidget(dateEdit);
    formLayout->addWidget(addLoanButton);

    mainLayout->addLayout(formLayout);

    // Loans list
    QLabel *loansLabel = new QLabel(QString::fromUtf8("لیست وام‌ها:"), this);
    searchLoans = new QLineEdit(this);
    searchLoans->setPlaceholderText(QString::fromUtf8("جستجو بر اساس مبلغ، وام‌گیرنده، ضامن، تاریخ..."));
    loanTable = new QTableView(this);
    loanTable->horizontalHeader()->setStretchLastSection(true);

    mainLayout->addWidget(loansLabel);
    mainLayout->addWidget(searchLoans);
    mainLayout->addWidget(loanTable);

    // --- Models ---
    borrowerModel = new QSqlTableModel(this, db);
    borrowerModel->setTable("persons");
    borrowerModel->select();
    // show id,name,ssn,job
    borrowerModel->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("id"));
    borrowerModel->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("نام"));
    borrowerModel->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("شماره ملی"));
    borrowerModel->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("شغل"));

    borrowerProxy = new QSortFilterProxyModel(this);
    borrowerProxy->setSourceModel(borrowerModel);
    borrowerProxy->setFilterKeyColumn(-1); // filter all columns
    borrowerProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    borrowerTable->setModel(borrowerProxy);
    borrowerTable->hideColumn(0); // hide id column

    guarantorModel = new QSqlTableModel(this, db);
    guarantorModel->setTable("persons");
    guarantorModel->select();
    guarantorModel->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("id"));
    guarantorModel->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("نام"));
    guarantorModel->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("شماره ملی"));
    guarantorModel->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("شغل"));

    guarantorProxy = new QSortFilterProxyModel(this);
    guarantorProxy->setSourceModel(guarantorModel);
    guarantorProxy->setFilterKeyColumn(-1);
    guarantorProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    guarantorTable->setModel(guarantorProxy);
    guarantorTable->hideColumn(0);

    // Loan model using relations so borrower/guarantor names show
    loanModel = new QSqlRelationalTableModel(this, db);
    loanModel->setTable("loans");
    loanModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    // set relations: show person.name instead of raw id
    int borrowerCol = loanModel->fieldIndex("borrower_id");
    int guarantorCol = loanModel->fieldIndex("guarantor_id");
    if (borrowerCol >= 0) loanModel->setRelation(borrowerCol, QSqlRelation("persons", "id", "name"));
    if (guarantorCol >= 0) loanModel->setRelation(guarantorCol, QSqlRelation("persons", "id", "name"));
    loanModel->select();

    loanProxy = new QSortFilterProxyModel(this);
    loanProxy->setSourceModel(loanModel);
    loanProxy->setFilterKeyColumn(-1);
    loanProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    loanTable->setModel(loanProxy);
    // show human-friendly headers
    loanModel->setHeaderData(loanModel->fieldIndex("id"), Qt::Horizontal, QString::fromUtf8("شناسه"));
    loanModel->setHeaderData(loanModel->fieldIndex("amount"), Qt::Horizontal, QString::fromUtf8("مبلغ"));
    loanModel->setHeaderData(loanModel->fieldIndex("percentage"), Qt::Horizontal, QString::fromUtf8("درصد"));
    loanModel->setHeaderData(loanModel->fieldIndex("description"), Qt::Horizontal, QString::fromUtf8("توضیحات"));
    loanModel->setHeaderData(loanModel->fieldIndex("date"), Qt::Horizontal, QString::fromUtf8("تاریخ"));
    if (borrowerCol >= 0) loanModel->setHeaderData(borrowerCol, Qt::Horizontal, QString::fromUtf8("وام‌گیرنده"));
    if (guarantorCol >= 0) loanModel->setHeaderData(guarantorCol, Qt::Horizontal, QString::fromUtf8("ضامن"));

    // --- Connections ---
    connect(searchBorrower, &QLineEdit::textChanged, this, &LoansWidgets::filterBorrowers);
    connect(searchGuarantor, &QLineEdit::textChanged, this, &LoansWidgets::filterGuarantors);
    connect(searchLoans, &QLineEdit::textChanged, loanProxy, &QSortFilterProxyModel::setFilterFixedString);

    connect(borrowerTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this](const QModelIndex &current, const QModelIndex &) {
                if (!current.isValid()) return;
                QModelIndex src = borrowerProxy->mapToSource(current);
                selectedBorrowerId = borrowerModel->data(borrowerModel->index(src.row(), 0)).toInt();
            });

    connect(guarantorTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this](const QModelIndex &current, const QModelIndex &) {
                if (!current.isValid()) return;
                QModelIndex src = guarantorProxy->mapToSource(current);
                selectedGuarantorId = guarantorModel->data(guarantorModel->index(src.row(), 0)).toInt();
            });

    connect(addLoanButton, &QPushButton::clicked, this, &LoansWidgets::addLoan);

    // initial selection if available
    if (borrowerModel->rowCount() > 0) {
        borrowerTable->selectRow(0);
    }
    if (guarantorModel->rowCount() > 0) {
        guarantorTable->selectRow(0);
    }
}

LoansWidgets::~LoansWidgets() {
    // Qt parent-child will clean widgets
}

void LoansWidgets::filterBorrowers(const QString &text) {
    borrowerProxy->setFilterFixedString(text);
    borrowerProxy->setFilterWildcard("*" + text + "*");
}

void LoansWidgets::filterGuarantors(const QString &text) {
    guarantorProxy->setFilterFixedString(text);
    guarantorProxy->setFilterWildcard("*" + text + "*");
}

void LoansWidgets::addLoan() {
    if (selectedBorrowerId < 0) {
        QMessageBox::warning(this, QString::fromUtf8("خطا"), QString::fromUtf8("لطفا وام‌گیرنده را انتخاب کنید."));
        return;
    }

    bool ok = false;
    double amount = amountEdit->text().replace(',',".").toDouble(&ok);
    if (!ok || amount <= 0.0) {
        QMessageBox::warning(this, QString::fromUtf8("خطا"), QString::fromUtf8("مبلغ نامعتبر است."));
        return;
    }

    QString desc = descEdit->text();
    QString date = dateEdit->date().toString("yyyy-MM-dd");

    bool okp = false;
    double percentage = percentageEdit->text().replace(',', '.').toDouble(&okp);
    if (!okp) percentage = 0.0;

    QSqlQuery q(db);
    q.prepare("INSERT INTO loans (borrower_id, guarantor_id, amount, percentage, description, date) VALUES (?, ?, ?, ?, ?, ?)");
    q.addBindValue(selectedBorrowerId);
    if (selectedGuarantorId > 0) q.addBindValue(selectedGuarantorId); else q.addBindValue(QVariant(QVariant::Int));
    q.addBindValue(amount);
    q.addBindValue(percentage);
    q.addBindValue(desc);
    q.addBindValue(date);

    if (!q.exec()) {
        QMessageBox::critical(this, QString::fromUtf8("خطای پایگاه داده"), q.lastError().text());
        return;
    }

    // clear inputs
    amountEdit->clear();
    percentageEdit->clear();
    descEdit->clear();
    dateEdit->setDate(QDate::currentDate());

    // refresh model
    loanModel->select();
}

void LoansWidgets::refreshLoans() {
    // simply refresh the relational model
    if (loanModel) loanModel->select();
}
