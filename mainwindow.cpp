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

    // Ensure a default shared SQLite DB (people.db) exists and has required tables.
    QSqlDatabase db;
    const QString defaultConn = QSqlDatabase::defaultConnection;
    if (QSqlDatabase::contains(defaultConn)) {
        db = QSqlDatabase::database(defaultConn);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("people.db");
    }

    if (!db.open()) {
        QMessageBox::critical(this, "خطای پایگاه داده", db.lastError().text());
        // still continue; widgets will show errors if DB unavailable
    } else {
        QSqlQuery q(db);
        // persons table (if not exists)
        q.exec(R"(
            CREATE TABLE IF NOT EXISTS persons (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                ssn TEXT,
                job TEXT,
                score INTEGER
            )
        )");
        // loans table with borrower and guarantor referencing persons
        q.exec(R"(
            CREATE TABLE IF NOT EXISTS loans (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                borrower_id INTEGER NOT NULL,
                guarantor_id INTEGER,
                amount REAL NOT NULL,
                percentage REAL DEFAULT 0,
                date TEXT,
                description TEXT,
                FOREIGN KEY(borrower_id) REFERENCES persons(id),
                FOREIGN KEY(guarantor_id) REFERENCES persons(id)
            )
        )");
    }

    // Create widgets (they will use the default DB connection)
    PersonWidget* person = new PersonWidget(this);
    LoansWidgets* loans = new LoansWidgets(this);

    // Add widgets to the tab widget
    ui->tabWidget->addTab(person, "اشخاص");
    ui->tabWidget->addTab(loans, "لیست تسهیلات");
}

MainWindow::~MainWindow() {
    delete ui;
}
