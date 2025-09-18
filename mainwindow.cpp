#include "loanswidgets.h"
#include "MainWindow.h"
#include "personwidget.h"
#include "ui_MainWindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create a shared SQLite connection with a named connection
    const QString connName = "loaners_conn";
    if (!QSqlDatabase::contains(connName)) {
        db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName("loaners.db");
    } else {
        db = QSqlDatabase::database(connName);
    }

    if (!db.open()) {
        QMessageBox::critical(this, "Database error", db.lastError().text());
        return;
    }

    // Ensure required tables exist
    QSqlQuery q(db);
    if (!q.exec(R"(CREATE TABLE IF NOT EXISTS persons (
                      id INTEGER PRIMARY KEY AUTOINCREMENT,
                      name TEXT NOT NULL,
                      ssn TEXT,
                      job TEXT,
                      score INTEGER
                  ))")) {
        QMessageBox::warning(this, "DB warning", q.lastError().text());
    }
    if (!q.exec(R"(CREATE TABLE IF NOT EXISTS loans (
                      id INTEGER PRIMARY KEY AUTOINCREMENT,
                      person_id INTEGER,
                      amount REAL,
                      description TEXT,
                      date TEXT,
                      FOREIGN KEY(person_id) REFERENCES persons(id)
                  ))")) {
        QMessageBox::warning(this, "DB warning", q.lastError().text());
    }

    // Pass the connection name to child widgets so they use the same QSqlDatabase
    PersonWidget* person = new PersonWidget(this, connName);
    LoansWidgets* loans = new LoansWidgets(this, connName);

    // Add widgets to the tab widget
    ui->tabWidget->addTab(person, "اشخاص");
    ui->tabWidget->addTab(loans, "لیست تسهیلات");
}

MainWindow::~MainWindow() {
    delete ui;
}
