#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QComboBox>


class toolbar : public QLabel
{
    Q_OBJECT

public:
    toolbar();
    ~toolbar();

    void set_receiver_list(QVector<int> receiver_list);

public slots:
    void raise_time_out();
    void hide_time_out();
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

    void button_close_all_clicked();
    void button_operate_clicked();

signals:
    void sig_show_panel();

private:
    QLabel *m_bg;
    QPushButton *m_close_all;
    QPushButton *m_operate;
    QComboBox *m_receiver_list;

    QTimer m_raise_timer;
    QTimer m_hide_timer;
    int hide_y;

    QVector<int> m_insid_list;
};

#endif // TOOLBAR_H
