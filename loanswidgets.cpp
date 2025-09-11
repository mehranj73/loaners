//
// Created by Mehranj73 on 9/11/2025.
//

// You may need to build the project (run Qt uic code generator) to get "ui_LoansWidgets.h" resolved

#include "loanswidgets.h"
#include "ui_LoansWidgets.h"


LoansWidgets::LoansWidgets(QWidget *parent) : QWidget(parent), ui(new Ui::LoansWidgets) {
    ui->setupUi(this);
}

LoansWidgets::~LoansWidgets() {
    delete ui;
}