//
// Created by Mehranj73 on 9/11/2025.
//

// You may need to build the project (run Qt uic code generator) to get "ui_Mainwindow.h" resolved

#include "mainwindow.h"
#include "ui_Mainwindow.h"
#include "loanswidgets.h"
#include "personwidget.h"


Mainwindow::Mainwindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::Mainwindow) {
    ui->setupUi(this);


    PersonWidget* person = new PersonWidget(this);
    LoansWidgets* loans = new LoansWidgets(this);


    // Add widgets to the tab widget
    ui->tabWidget->addTab(person, "اشخاص");
    ui->tabWidget->addTab(loans, "لیست تسهیلات");
}

Mainwindow::~Mainwindow() {
    delete ui;
}