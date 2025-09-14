#ifndef PERSONWIDGET_H
#define PERSONWIDGET_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
namespace Ui { class PersonWidget; }
QT_END_NAMESPACE

class PersonWidget : public QWidget {
    Q_OBJECT

public:
    explicit PersonWidget(QWidget *parent = nullptr);
    ~PersonWidget();

private slots:
    void addPerson();

private:
    void setupDatabase();

    Ui::PersonWidget *ui;
    QSqlDatabase db;
    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel;
};

#endif // PERSONWIDGET_H
