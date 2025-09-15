#ifndef RTLLINEEDIT_H
#define RTLLINEEDIT_H

#include <QLineEdit>
#include <QFocusEvent>

class RtlLineEdit : public QLineEdit {
    Q_OBJECT
public:
    using QLineEdit::QLineEdit;

protected:
    void focusInEvent(QFocusEvent *event) override {
        QLineEdit::focusInEvent(event);
        setCursorPosition(text().length()); // cursor starts on the right
    }
};

#endif // RTLLINEEDIT_H
