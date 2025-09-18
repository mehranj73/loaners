#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

class QPushButton;
class QDateEdit;
class QTableView;
class QLineEdit;
class QVBoxLayout;
QT_BEGIN_NAMESPACE
namespace Ui { class LoansWidgets; }
QT_END_NAMESPACE

class LoansWidgets : public QWidget {
    Q_OBJECT
public:
    explicit LoansWidgets(QWidget *parent = nullptr, const QString &connectionName = QString());
    ~LoansWidgets() override;

private slots:
    void addLoan();
    void filterBorrowers(const QString &text);
    void filterGuarantors(const QString &text);
    void refreshLoans();

private:
    // UI elements created in code (no .ui file)
    QWidget *mainWidget;
    QVBoxLayout *mainLayout;

    // borrower selection
    QLineEdit *searchBorrower;
    QTableView *borrowerTable;
    QSqlTableModel *borrowerModel;
    QSortFilterProxyModel *borrowerProxy;

    // guarantor selection
    QLineEdit *searchGuarantor;
    QTableView *guarantorTable;
    QSqlTableModel *guarantorModel;
    QSortFilterProxyModel *guarantorProxy;

    // loan form
    QLineEdit *amountEdit;
    QLineEdit *percentageEdit;
    QLineEdit *descEdit;
    QDateEdit *dateEdit;
    QPushButton *addLoanButton;

    // loans list
    QLineEdit *searchLoans;
    QTableView *loanTable;
    QSqlRelationalTableModel *loanModel;
    QSortFilterProxyModel *loanProxy;

    QSqlDatabase db;
    int selectedBorrowerId = -1;
    int selectedGuarantorId = -1;
};
