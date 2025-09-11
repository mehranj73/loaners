//
// Created by Mehranj73 on 9/11/2025.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PersonWidget.h" resolved

#include "personwidget.h"
#include "ui_PersonWidget.h"


PersonWidget::PersonWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PersonWidget) {
    ui->setupUi(this);
}

PersonWidget::~PersonWidget() {
    delete ui;
}