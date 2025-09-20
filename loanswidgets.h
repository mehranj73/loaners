#ifndef LOANSWIDGETS_H
#define LOANSWIDGETS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
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
    void filterBorrowers(const QString &text) {
    borrowerProxy->setFilterFixedString(text);
}
    void filterGuarantors(const QString &text) {
    guarantorProxy->setFilterFixedString(text);
}
    void filterLoans(const QString &text) {
    loanProxy->setFilterFixedString(text);
}
    void borrowerSelected(const QModelIndex &index);
    void guarantorSelectionChanged();
    void addLoan();
    void onLoanSelected();

private:
    void setupModels();
    void loadLoans();

    Ui::LoansWidgets *ui;
    QSqlDatabase db;

    QSqlTableModel *borrowerModel;
    QSortFilterProxyModel *borrowerProxy;

    QSqlTableModel *guarantorModel;
    QSortFilterProxyModel *guarantorProxy;

    QSqlQueryModel *loanModel;
    QSortFilterProxyModel *loanProxy;

    int selectedBorrowerId;
    QList<int> selectedGuarantorIds;
};

#endif // LOANSWIDGETS_H
