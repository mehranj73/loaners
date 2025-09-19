#ifndef LOANSWIDGETS_H
#define LOANSWIDGETS_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QVector>

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
    void loadLoans();

    Ui::LoansWidgets *ui;
    QSqlDatabase db;

    QSqlTableModel *borrowerModel;
    QSqlTableModel *guarantorModel;
    QSortFilterProxyModel *borrowerProxy;
    QSortFilterProxyModel *guarantorProxy;

    QSqlQueryModel *loanModel;
    QSortFilterProxyModel *loanProxy;

    int selectedBorrowerId;
    QVector<int> selectedGuarantorIds;
};

#endif // LOANSWIDGETS_H
