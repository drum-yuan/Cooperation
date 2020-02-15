#include "widget.h"
#include "ui_widget.h"
#include "DaemonApi.h"
#include <windows.h>
#include <QMouseEvent>
#include <QDebug>

#define AGENT_LBUTTON_MASK (1 << 1)
#define AGENT_MBUTTON_MASK (1 << 2)
#define AGENT_RBUTTON_MASK (1 << 3)
#define AGENT_UBUTTON_MASK (1 << 4)
#define AGENT_DBUTTON_MASK (1 << 5)

static Widget* _instance = NULL;
static DWORD m_dwButtonState = 0;
static DWORD m_dwInputTime = 0;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{    
    ui->setupUi(this);
    connect(ui->Connect, SIGNAL(clicked()), this, SLOT(connect_to_server()));
    connect(ui->StartStream, SIGNAL(clicked()), this, SLOT(start_stream()));
    connect(ui->StopStream, SIGNAL(clicked()), this, SLOT(stop_stream()));
    connect(ui->Operate, SIGNAL(clicked()), this, SLOT(set_operater()));
    _instance = this;

    m_can_operate = false;
    m_button_mask = 0;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::connect_to_server()
{
    if (ui->lineEdit->text().isEmpty()) {
        return;
    }
    QString url = ui->lineEdit->text();
    QByteArray baUrl = url.toLatin1();
    bool ret = daemon_start(baUrl.data());
    if (ret) {
        daemon_set_start_stream_callback(start_stream_callback);
        daemon_set_stop_stream_callback(stop_stream_callback);
        daemon_set_operater_callback(start_operate_callback);
        daemon_set_mouse_callback(recv_mouse_event_callback);
        daemon_set_keyboard_callback(recv_keyboard_event_callback);
        ui->Connect->setEnabled(false);
    }
}

void Widget::start_stream()
{
    daemon_start_stream();
}

void Widget::stop_stream()
{
    daemon_stop_stream();
}

void Widget::set_operater()
{
    daemon_start_operate();
}

void Widget::hide_buttons()
{
    ui->lineEdit->hide();
    ui->Connect->hide();
    ui->StartStream->hide();
    ui->StopStream->hide();
    ui->Operate->hide();
}

void Widget::show_buttons()
{
    ui->lineEdit->show();
    ui->Connect->show();
    ui->StartStream->show();
    ui->StopStream->show();
    ui->Operate->show();
}

void Widget::start_stream_callback()
{
    _instance->hide_buttons();
    _instance->showFullScreen();
    daemon_show_stream((HWND)_instance->winId());
}

void Widget::stop_stream_callback()
{
    _instance->repaint();
    _instance->show_buttons();
    _instance->resize(400, 300);
}

void Widget::start_operate_callback(bool is_operater)
{
    if (is_operater) {
        _instance->setWindowTitle("主控");
    }
    else {
        _instance->setWindowTitle("被控");
    }
    _instance->m_can_operate = is_operater;
}

int Widget::get_buttons_change(unsigned int last_buttons_state, unsigned int new_buttons_state,
                        unsigned int mask, unsigned int down_flag, unsigned int up_flag)
{
    int ret = 0;
    if (!(last_buttons_state & mask) && (new_buttons_state & mask)) {
        ret = down_flag;
    }
    else if ((last_buttons_state & mask) && !(new_buttons_state & mask)) {
        ret = up_flag;
    }
    return ret;
}

void Widget::recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask)
{
    INPUT _input;
    DWORD mouse_move = 0;
    DWORD buttons_change = 0;
    DWORD mouse_wheel = 0;

    qDebug() << "recv mouse event " << x << "-" << y << "-" << button_mask;
    ZeroMemory(&_input, sizeof(INPUT));
    _input.type = INPUT_MOUSE;

    DWORD w = ::GetSystemMetrics(SM_CXSCREEN);
    DWORD h = ::GetSystemMetrics(SM_CYSCREEN);
    w = (w > 1) ? w - 1 : 1; /* coordinates are 0..w-1, protect w==0 */
    h = (h > 1) ? h - 1 : 1; /* coordinates are 0..h-1, protect h==0 */
    mouse_move = MOUSEEVENTF_MOVE;
    _input.mi.dx = x * 0xffff / w;
    _input.mi.dy = y * 0xffff / h;

    if (button_mask != m_dwButtonState) {
        buttons_change = get_buttons_change(m_dwButtonState, button_mask, AGENT_LBUTTON_MASK,
            MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP) |
            get_buttons_change(m_dwButtonState, button_mask, AGENT_MBUTTON_MASK,
                MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP) |
            get_buttons_change(m_dwButtonState, button_mask, AGENT_RBUTTON_MASK,
                MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
        m_dwButtonState = button_mask;
    }
    if (button_mask & (AGENT_UBUTTON_MASK | AGENT_DBUTTON_MASK)) {
        mouse_wheel = MOUSEEVENTF_WHEEL;
        if (button_mask & AGENT_UBUTTON_MASK) {
            _input.mi.mouseData = WHEEL_DELTA;
        }
        else if (button_mask & AGENT_DBUTTON_MASK) {
            _input.mi.mouseData = (DWORD)(-WHEEL_DELTA);
        }
    }
    else {
        mouse_wheel = 0;
        _input.mi.mouseData = 0;
    }

    _input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | mouse_move |
        mouse_wheel | buttons_change;

    if ((mouse_move && GetTickCount() - m_dwInputTime > 10) || buttons_change || mouse_wheel) {
        SendInput(1, &_input, sizeof(INPUT));
        m_dwInputTime = GetTickCount();
    }
}

void Widget::recv_keyboard_event_callback(unsigned int key_val, bool is_pressed)
{
    if (is_pressed) {
        keybd_event(key_val, 0, 0, 0);
    }
    else {
        keybd_event(key_val, 0, KEYEVENTF_KEYUP, 0);
    }
}

void Widget::scale_to_screen(QPoint& point)
{
    int w = 0;
    int h = 0;
    daemon_get_stream_size(&w, &h);
    qDebug() << "stream size " << w << "-" << h;

    if (w > 0 && h > 0) {
        point.setX(point.x() * w / width());
        point.setY(point.y() * h / height());
    }
    qDebug() << "cur point " << point.x() << "-" << point.y();
}

void Widget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_can_operate) {
        QPoint point = event->globalPos();
        scale_to_screen(point);
        daemon_send_mouse_event(point.x(), point.y(), m_button_mask);
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if (m_can_operate) {
        if (event->button() == Qt::LeftButton) {
            m_button_mask |= AGENT_LBUTTON_MASK;
        } else if (event->button() == Qt::RightButton) {
            m_button_mask |= AGENT_RBUTTON_MASK;
        } else {
            m_button_mask |= AGENT_MBUTTON_MASK;
        }
        QPoint point = event->globalPos();
        scale_to_screen(point);
        daemon_send_mouse_event(point.x(), point.y(), m_button_mask);
    } else {
        QWidget::mousePressEvent(event);
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_can_operate) {
        if (event->button() == Qt::LeftButton) {
            m_button_mask &= ~AGENT_LBUTTON_MASK;
        } else if (event->button() == Qt::RightButton) {
            m_button_mask &= ~AGENT_RBUTTON_MASK;
        } else {
            m_button_mask &= ~AGENT_MBUTTON_MASK;
        }
        QPoint point = event->globalPos();
        scale_to_screen(point);
        daemon_send_mouse_event(point.x(), point.y(), m_button_mask);
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void Widget::wheelEvent(QWheelEvent *event)
{
    if (m_can_operate) {
        if (event->delta() > 0) {
            m_button_mask |= AGENT_UBUTTON_MASK;
        } else {
            m_button_mask |= AGENT_DBUTTON_MASK;
        }
        QPoint point = event->globalPos();
        scale_to_screen(point);
        daemon_send_mouse_event(point.x(), point.y(), m_button_mask);
        if (event->delta() > 0) {
            m_button_mask &= ~AGENT_UBUTTON_MASK;
        } else {
            m_button_mask &= ~AGENT_DBUTTON_MASK;
        }
    } else {
        QWidget::wheelEvent(event);
    }
}

void Widget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_can_operate) {
        m_button_mask |= AGENT_LBUTTON_MASK;
        QPoint point = event->globalPos();
        scale_to_screen(point);
        daemon_send_mouse_event(point.x(), point.y(), m_button_mask);
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if (m_can_operate) {
        daemon_send_keyboard_event(event->key(), true);
    } else {
        QWidget::keyPressEvent(event);
    }
}

void Widget::keyReleaseEvent(QKeyEvent *event)
{
    if (m_can_operate) {
        daemon_send_keyboard_event(event->key(), false);
    } else {
        QWidget::keyReleaseEvent(event);
    }
}
