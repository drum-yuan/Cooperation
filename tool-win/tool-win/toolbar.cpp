#include "toolbar.h"
#include "co-interface.h"
#include <QIcon>
#include <QApplication>
#include <QDesktopWidget>
#include <QListView>
#include <QDebug>

toolbar::toolbar()
{
    setWindowTitle(QString::fromLocal8Bit("远程桌面"));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFixedSize(QImage(":/res/toolbar-bg.png").size());

    int screen_width = QApplication::desktop()->width();
    //int screen_height = QApplication::desktop()->height();
    setGeometry((screen_width - this->width()) / 2, 0, width(), height());

    m_bg = new QLabel(this);
    m_bg->setStyleSheet("QLabel{border-image: url(:/res/toolbar-bg.png)}");
    m_bg->setGeometry(0, 0, width(), height());

    m_close_all = new QPushButton(m_bg);
    connect(m_close_all, SIGNAL(clicked()), this, SLOT(button_close_all_clicked()));
    m_close_all->setFocusPolicy(Qt::NoFocus);
    m_close_all->setStyleSheet("QPushButton{border-image: url(:/res/close_all.png)}"
                            "QPushButton:hover{border-image: url(:/res/close_all-hover.png)}"
                            "QPushButton:pressed{border-image: url(:/res/close_all-press.png)}");
    m_close_all->resize(QImage(":/res/close_all.png").size());
    m_close_all->move(width() - 70, 5);

    m_operate = new QPushButton(m_bg);
    connect(m_operate, SIGNAL(clicked()), this, SLOT(button_operate_clicked()));
    m_operate->setFocusPolicy(Qt::NoFocus);
    m_operate->setStyleSheet("QPushButton{border-image: url(:/res/operate.png)}"
                            "QPushButton:hover{border-image: url(:/res/operate-hover.png)}"
                            "QPushButton:pressed{border-image: url(:/res/operate-press.png)}");
    m_operate->resize(QImage(":/res/operate.png").size());
    m_operate->move(30, 5);

    m_receiver_list = new QComboBox(m_bg);
    m_receiver_list->setFocusPolicy(Qt::NoFocus);
    m_receiver_list->resize(120, height() - 10);
    m_receiver_list->move(width() - 200, 3);
    m_receiver_list->setStyleSheet("QComboBox QAbstractItemView::item {min-height: 30px;}");
    m_receiver_list->setView(new QListView());

    m_raise_timer.start(1000);
    connect(&m_raise_timer, SIGNAL(timeout()), this, SLOT(raise_time_out()));
    connect(&m_hide_timer, SIGNAL(timeout()), this, SLOT(hide_time_out()));
    hide_y = 0;
}

toolbar::~toolbar()
{

}

void toolbar::set_receiver_list(QVector<int> receiver_list)
{
    m_receiver_list->clear();
    for (auto &it : receiver_list) {
        m_receiver_list->addItem(QString::fromLocal8Bit("远程桌面%1").arg(it), it);
    }
}

void toolbar::raise_time_out()
{
    int screen_width = QApplication::desktop()->width();
    QPoint pt = QCursor::pos();
    if (pt.ry() < height() && pt.rx() > (screen_width - width()) / 2 && pt.rx() < (screen_width + width()) / 2)
    {
        m_hide_timer.stop();
        m_bg->move(0, 0);
    }
}

void toolbar::hide_time_out()
{
    if (hide_y < height())
    {
        hide_y++;
        m_bg->move(0, -hide_y);
    }
    else
    {
        if (m_hide_timer.isActive())
            m_hide_timer.stop();
    }
}

void toolbar::leaveEvent(QEvent *event)
{
    if (!m_hide_timer.isActive())
    {
        hide_y = 0;
        m_hide_timer.start(10);
    }

    return QWidget::leaveEvent(event);
}

void toolbar::button_close_all_clicked()
{
    int ins_id = m_receiver_list->currentData().toInt();
    qDebug() << "ins id " << ins_id;
    if (ins_id >= 0) {
        coclient_stop_receiver(ins_id);
        m_receiver_list->removeItem(m_receiver_list->currentIndex());
    }
    if (m_receiver_list->count() == 0) {
        close();
        emit sig_show_panel();
    }
}

void toolbar::button_operate_clicked()
{
    int ins_id = m_receiver_list->currentData().toInt();
    qDebug() << "ins id " << ins_id;
    if (ins_id >= 0) {
        coclient_start_operate(ins_id);
    }
}
