#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <thread>

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
    static void recv_cursor_shape_callback(int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);

private:
    void hide_buttons();
    void show_buttons();
    void scale_to_screen(QPoint& point);
    void monitor_thread();

    Ui::Widget *ui;
    bool m_quit;
    bool m_can_operate;
    bool m_is_streaming;
    unsigned int m_button_mask;
    std::thread* m_monitor_thread;
    HCURSOR m_cursor;

signals:
public slots:
    void connect_to_server();
    void start_stream();
    void stop_stream();
    void set_operater();

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
};

#endif // WIDGET_H
