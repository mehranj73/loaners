#ifndef LOANSWIDGETS_H
#define LOANSWIDGETS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

namespace Ui {
    class LoansWidgets;
}

class LoansWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit LoansWidgets(QWidget *parent = nullptr, const QString &connectionName = QString());
    ~LoansWidgets();

private slots:
    void addLoan();
    void filterBorrowers(const QString &text);
    void filterGuarantors(const QString &text);
    void filterLoans(const QString &text);

private:
    Ui::LoansWidgets *ui;
    QSqlDatabase db;

    // Models
    QSqlTableModel *borrowerModel;
    QSqlTableModel *guarantorModel;
    QSortFilterProxyModel *borrowerProxy;
    QSortFilterProxyModel *guarantorProxy;

    QSqlRelationalTableModel *loanModel;
    QSortFilterProxyModel *loanProxy; // for loan search

    // Selected IDs
    int selectedBorrowerId;
    int selectedGuarantorId;
};

#endif // LOANSWIDGETS_H
