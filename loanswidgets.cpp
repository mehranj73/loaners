//
// Created by Mehranj73 on 9/11/2025.
//

// You may need to build the project (run Qt uic code generator) to get "ui_LoansWidgets.h" resolved

#include "loanswidgets.h"
#include "ui_LoansWidgets.h"


LoansWidgets::LoansWidgets(QWidget *parent, const QString &connectionName) : QWidget(parent), ui(new Ui::LoansWidgets) {
    // use provided DB connection name if available
    if (!connectionName.isEmpty() && QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::database();
    }
    ui->setupUi(this);

    // No need to redeclare, just use ui->tableWidget
    ui->tableWidget->setRowCount(3);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setWindowTitle("QTableWidget Example");

    // Set headers
    QStringList headers = {"Name", "Age", "Country"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Add data
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem("Alice"));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem("25"));
    ui->tableWidget->setItem(0, 2, new QTableWidgetItem("USA"));

    ui->tableWidget->setItem(1, 0, new QTableWidgetItem("Bob"));
    ui->tableWidget->setItem(1, 1, new QTableWidgetItem("30"));
    ui->tableWidget->setItem(1, 2, new QTableWidgetItem("UK"));

    ui->tableWidget->setItem(2, 0, new QTableWidgetItem("Charlie"));
    ui->tableWidget->setItem(2, 1, new QTableWidgetItem("22"));
    ui->tableWidget->setItem(2, 2, new QTableWidgetItem("Canada"));


}

LoansWidgets::~LoansWidgets() {
    delete ui;
}
