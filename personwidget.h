//
// Created by Mehranj73 on 9/11/2025.
//

#ifndef LOANERS_PERSONWIDGET_H
#define LOANERS_PERSONWIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui {
    class PersonWidget;
}

QT_END_NAMESPACE

class PersonWidget : public QWidget {
    Q_OBJECT

public:
    explicit PersonWidget(QWidget *parent = nullptr);

    ~PersonWidget() override;

private:
    Ui::PersonWidget *ui;
};


#endif //LOANERS_PERSONWIDGET_H