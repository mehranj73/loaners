//
// Created by Mehranj73 on 9/11/2025.
//

#ifndef LOANERS_LOANSWIDGETS_H
#define LOANERS_LOANSWIDGETS_H

#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui {
    class LoansWidgets;
}

QT_END_NAMESPACE

class LoansWidgets : public QWidget {
    Q_OBJECT

public:
    explicit LoansWidgets(QWidget *parent = nullptr);

    ~LoansWidgets() override;

private:
    Ui::LoansWidgets *ui;
};


#endif //LOANERS_LOANSWIDGETS_H