#include "PersonWidget.h"
#include "ui_personwidget.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

PersonWidget::PersonWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::PersonWidget), model(nullptr), proxyModel(nullptr)
{
    ui->setupUi(this);
    setupDatabase();

    model = new QSqlTableModel(this, db);
    model->setTable("persons");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0); // filter by name

    ui->tableView->setModel(proxyModel);
    ui->tableView->resizeColumnsToContents();

    connect(ui->addButton, &QPushButton::clicked, this, &PersonWidget::addPerson);
    connect(ui->searchEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString);
}

PersonWidget::~PersonWidget() {
    delete ui;
}

void PersonWidget::setupDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("people.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS persons ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL,"
               "ssn TEXT NOT NULL UNIQUE,"
               "job TEXT,"
               "score TEXT)");
}

void PersonWidget::addPerson() {
    QString name = ui->nameEdit->text().trimmed();
    QString ssn = ui->ssnEdit->text().trimmed();
    QString job = ui->jobEdit->text().trimmed();
    QString score = ui->scoreCombo->currentText();

    if (name.isEmpty() || ssn.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Name and SSN are required!");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO persons (name, ssn, job, score) VALUES (?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(ssn);
    query.addBindValue(job);
    query.addBindValue(score);

    if (!query.exec()) {
        QMessageBox::critical(this, "Insert Error", query.lastError().text());
        return;
    }

    model->select(); // refresh
    ui->nameEdit->clear();
    ui->ssnEdit->clear();
    ui->jobEdit->clear();
    ui->scoreCombo->setCurrentIndex(0);
}
