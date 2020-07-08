#include "toolbar.h"
#include <QIcon>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

toolbar::toolbar()
{
    setWindowTitle("远程桌面");
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFixedSize(QImage(":/res/toolbar-bg.png").size());

    int screen_width = QApplication::desktop()->screenGeometry().width();
    int screen_height = QApplication::desktop()->screenGeometry().height();
    setGeometry((screen_width - this->width()) * 0.5, 0, width(), height());

    m_bg = new QLabel(this);
    m_bg->setStyleSheet("QLabel{border-image: url(:/res/toolbar-bg.png)}");
    m_bg->setGeometry(0, 0, width(), height());

    m_raise_timer.start(1000);
    connect(&m_raise_timer, SIGNAL(timeout()), this, SLOT(raise_time_out()));
    connect(&m_hide_timer, SIGNAL(timeout()), this, SLOT(hide_time_out()));
    connect(&m_show_timer, SIGNAL(timeout()), this, SLOT(show_time_out()));
    hide_y = 0;
    show_y = 0;
    cur_y = 0;
}

toolbar::~toolbar()
{

}

void toolbar::raise_time_out()
{
    int screen_width = QApplication::desktop()->screenGeometry().width();
    QPoint pt = QCursor::pos();
    if (!isActiveWindow() &&
            pt.ry() < height() && pt.rx() > (screen_width - width()) / 2 && pt.rx() < (screen_width + width()) / 2)
    {
        m_bg->setGeometry(0, 0, this->width(), this->height());
        show();
    }
}

void toolbar::hide_time_out()
{
    if (hide_y < this->height())
    {
        hide_y++;
        m_bg->setGeometry(0, -hide_y, this->width(), this->height());
        cur_y = -hide_y;
    }
    else
    {
        if (m_hide_timer.isActive())
            m_hide_timer.stop();
    }
}

void toolbar::show_time_out()
{
    if (show_y < 0)
    {
        show_y++;
        m_bg->setGeometry(0, show_y, this->width(), this->height());
        cur_y = show_y;
    }
    else
    {
        if (m_show_timer.isActive())
            m_show_timer.stop();
    }
}

void toolbar::enterEvent(QEvent *event)
{
    if (!m_show_timer.isActive())
    {
        if (m_hide_timer.isActive())
            m_hide_timer.stop();
        show_y = -height();
        m_show_timer.start(10);
    }

    return QWidget::enterEvent(event);
}

void toolbar::leaveEvent(QEvent *event)
{
    if (!m_hide_timer.isActive())
    {
        if (m_show_timer.isActive())
            m_show_timer.stop();
        hide_y = 0;
        m_hide_timer.start(10);
    }

    return QWidget::leaveEvent(event);
}
