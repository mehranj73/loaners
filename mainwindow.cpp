#include "loanswidgets.h"
#include "MainWindow.h"
#include "personwidget.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    PersonWidget* person = new PersonWidget(this);
    LoansWidgets* loans = new LoansWidgets(this);


    // Add widgets to the tab widget
    ui->tabWidget->addTab(person, "اشخاص");
    ui->tabWidget->addTab(loans, "لیست تسهیلات");
}

MainWindow::~MainWindow() {
    delete ui;
}
