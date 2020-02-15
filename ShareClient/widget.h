#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    static void start_stream_callback();
    static void stop_stream_callback();
    static void start_operate_callback(bool is_operater);
    static int get_buttons_change(unsigned int last_buttons_state, unsigned int new_buttons_state,
                              unsigned int mask, unsigned int down_flag, unsigned int up_flag);
    static void recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask);
    static void recv_keyboard_event_callback(unsigned int key_val, bool is_pressed);

private:
    void scale_to_screen(QPoint& point);

    Ui::Widget *ui;
    bool m_can_operate;
    unsigned int m_button_mask;

signals:
public slots:
    void connect_to_server();
    void start_stream();
    void stop_stream();
    void set_operater();

    void hide_buttons();
    void show_buttons();

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
};

#endif // WIDGET_H
