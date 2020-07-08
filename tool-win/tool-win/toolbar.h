#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QLabel>
#include <QTimer>


class toolbar : public QLabel
{
    Q_OBJECT

public:
    toolbar();
    ~toolbar();

public slots:
    void raise_time_out();
    void hide_time_out();
    void show_time_out();
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    QLabel *m_bg;

    QTimer m_raise_timer;
    QTimer m_hide_timer;
    QTimer m_show_timer;
    int hide_y;
    int show_y;
    int cur_y;
};

#endif // TOOLBAR_H
