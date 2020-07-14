
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QProcess>
#include <QRadioButton>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDebug>
#include <windows.h>
#include <Wtsapi32.h>

#define APP_LIST_KEY "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\TSAppAllowList"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setFocusPolicy(Qt::NoFocus);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    ui->setupUi(this);
    ui->publish_type->addItem("本机桌面");
    ui->publish_type->addItem("本机应用");
    ui->publish_type->addItem("远程应用");
    ui->publish_type->setCurrentIndex(0);

    ui->list->hide();
    ui->server_url->hide();
    ui->rdsh_url->hide();
    ui->rdsh_domain->hide();
    ui->rdsh_user->hide();
    ui->rdsh_password->hide();
    ui->app_alias->hide();
    ui->app_view->hide();

    setStyleSheet("QMainWindow{border-image: url(:/res/background.png)}");
    ui->publish->setStyleSheet("QPushButton{border-image: url(:/res/publish.png)}"
                               "QPushButton:hover{border-image: url(:/res/publish-hover.png)}");
    ui->query->setStyleSheet("QPushButton{border-image: url(:/res/query.png)}"
                               "QPushButton:hover{border-image: url(:/res/query-hover.png)}");
    ui->config->setStyleSheet("QPushButton{border-image: url(:/res/config.png)}"
                               "QPushButton:hover{border-image: url(:/res/config-hover.png)}");
    ui->confirm->setStyleSheet("QPushButton{border-image: url(:/res/confirm.png)}"
                               "QPushButton:hover{border-image: url(:/res/confirm-hover.png)}");
    ui->cancel->setStyleSheet("QPushButton{border-image: url(:/res/cancel.png)}"
                               "QPushButton:hover{border-image: url(:/res/cancel-hover.png)}");
    ui->app_view->setStyleSheet("QPushButton{border-image: url(:/res/view.png)}"
                               "QPushButton:hover{border-image: url(:/res/view-hover.png)}");
    ui->publish_type->setStyleSheet("QComboBox QAbstractItemView::item {min-height: 40px;}");
    ui->publish_type->setView(new QListView());
    m_mode = 0;

    connect(ui->query, SIGNAL(clicked()), this, SLOT(query_request()));
    connect(ui->publish, SIGNAL(clicked()), this, SLOT(publish_request()));
    connect(ui->config, SIGNAL(clicked()), this, SLOT(config_request()));
    connect(ui->publish_type, SIGNAL(currentIndexChanged(QString)), this, SLOT(publish_type_changed(const QString&)));
    connect(ui->confirm, SIGNAL(clicked()), this, SLOT(button_confirm_clicked()));
    connect(ui->cancel, SIGNAL(clicked()), this, SLOT(button_cancel_clicked()));
    connect(ui->app_view, SIGNAL(clicked()), this, SLOT(button_view_clicked()));

    QSettings setting(QCoreApplication::applicationDirPath() + "/server.ini", QSettings::IniFormat);
    setting.beginGroup("setting");
    m_server_url = setting.value("URL").toString();
    setting.endGroup();
    ui->server_url->setText(m_server_url);

    coclient_create(m_server_url.toStdString());
    mount_net_path();
}

MainWindow::~MainWindow()
{
    umount_net_path();
    coclient_destroy();
    delete ui;
}

void MainWindow::query_request()
{
    m_mode = 1;
    ui->list->show();
    ui->confirm->show();
    ui->publish_type->hide();
    ui->rdsh_url->hide();
    ui->rdsh_domain->hide();
    ui->rdsh_user->hide();
    ui->rdsh_password->hide();
    ui->app_alias->hide();
    ui->app_view->hide();
    ui->cancel->hide();
    ui->server_url->hide();
    ui->confirm->setEnabled(true);

    ui->list->clear();
    QFont font("MicrosoftYaHei", 10, 75);
    coclient_get_compute_node_list(m_node_list);
    QString showName;
    for (unsigned int i = 0; i < m_node_list.size(); i++) {
        if (m_node_list[i].app_name == "") {
            if (m_node_list[i].host_name == get_local_host_name()) {
                continue;
            }
            showName = "桌面";
        } else {
            showName = m_node_list[i].app_name.c_str();
        }
        QRadioButton* radio = new QRadioButton(QString("%1 %2").arg(m_node_list[i].app_guid.c_str()).arg(showName));
        radio->setFont(font);
        if (m_node_list[i].status != 0) {
            radio->setEnabled(false);
        }
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(QSize(700, 40));
        ui->list->addItem(item);
        ui->list->setItemWidget(item, radio);
    }
}

void MainWindow::publish_request()
{
    m_mode = 0;
    ui->publish_type->show();
    ui->publish_type->setCurrentIndex(0);
    ui->confirm->show();
    ui->cancel->show();
    ui->list->hide();
    ui->server_url->hide();
    if (!m_app_guid.empty()) {
        ui->confirm->setEnabled(false);
    }
}

void MainWindow::config_request()
{
    m_mode = 2;
    ui->server_url->show();
    ui->confirm->show();
    ui->cancel->show();
    ui->list->hide();
    ui->publish_type->hide();
    ui->rdsh_url->hide();
    ui->rdsh_domain->hide();
    ui->rdsh_user->hide();
    ui->rdsh_password->hide();
    ui->app_alias->hide();
    ui->app_view->hide();
    ui->confirm->setEnabled(true);
}

void MainWindow::publish_type_changed(const QString& text)
{
    if (text == "本机桌面") {
        ui->rdsh_url->hide();
        ui->rdsh_domain->hide();
        ui->rdsh_user->hide();
        ui->rdsh_password->hide();
        ui->app_alias->hide();
        ui->app_view->hide();
    } else if (text == "本机应用") {
        ui->rdsh_url->hide();
        ui->rdsh_domain->hide();
        ui->rdsh_user->show();
        ui->rdsh_password->show();
        ui->app_alias->show();
        ui->app_alias->setPlaceholderText("应用路径");
        ui->app_view->show();
    } else {
        ui->rdsh_url->show();
        ui->rdsh_domain->show();
        ui->rdsh_user->show();
        ui->rdsh_password->show();
        ui->app_alias->show();
        ui->app_alias->setPlaceholderText("应用别名");
        ui->app_view->hide();
    }
}

void MainWindow::button_confirm_clicked()
{
    switch (m_mode) {
    case 0:
    {
        std::string alias;
        bool ret = false;
        if (ui->publish_type->currentText() == "本机桌面") {
            alias = "";
        } else if (ui->publish_type->currentText() == "本机应用") {
            QString appPath = ui->app_alias->text();
            QString appExeFile = appPath.split('\\').last();
            appExeFile = appExeFile.split('/').last();
            int pos = appExeFile.indexOf(".exe", 0, Qt::CaseInsensitive);
            m_app_alias = appExeFile.left(pos);
            alias = m_app_alias.toStdString();
            publish_app();
            m_rdsh_info.user = ui->rdsh_user->text().toStdString();
            m_rdsh_info.password = ui->rdsh_password->text().toStdString();
            m_rdsh_info.rdsh_ip = std::string("localhost");
        } else {
            alias = ui->app_alias->text().toStdString();
            m_rdsh_info.rdsh_ip = ui->rdsh_url->text().toStdString();
            m_rdsh_info.domain = ui->rdsh_domain->text().toStdString();
            m_rdsh_info.user = ui->rdsh_user->text().toStdString();
            m_rdsh_info.password = ui->rdsh_password->text().toStdString();
        }
        ret = coclient_register_compute_node(alias, m_rdsh_info, m_app_guid);
        if (ret) {
            ui->confirm->setEnabled(false);
            ui->publish_type->setEnabled(false);
        }
    }
        break;
    case 1:
    {
        for (int i = 0; i < ui->list->count(); i++) {
            QListWidgetItem* item = ui->list->item(i);
            QRadioButton* radio = (QRadioButton*)ui->list->itemWidget(item);
            int ins_id = -1;
            if (radio->isChecked()) {
                if (m_node_list[i].host_name == get_local_host_name() && !m_node_list[i].app_name.empty()) {
                    coclient_start_compute_node(m_node_list[i].app_name, m_rdsh_info);
                } else {
                    ins_id = coclient_start_receiver(m_node_list[i]);
                    std::vector<int> receiver_list = coclient_get_current_receiver();
                    QVector<int> qt_receiver_list;
                    for (auto &it : receiver_list) {
                        qt_receiver_list.push_back(it);
                    }
                    m_toolbar.set_receiver_list(qt_receiver_list);
                    CoUsersInfo users_info;
                    coclient_get_users_info(ins_id, users_info);
                    if (users_info.operater.empty() || users_info.operater == get_local_host_name()) {
                        coclient_start_operate(ins_id);
                    }
                }
                m_toolbar.show();
                hide();
                break;
            }
        }
    }
        break;
    case 2:
    {
        m_server_url = ui->server_url->text();
        QSettings setting(QCoreApplication::applicationDirPath() + "/server.ini", QSettings::IniFormat);
        setting.beginGroup("setting");
        setting.setValue("URL", QVariant(m_server_url));
        setting.endGroup();
        QProcess::execute("sync");
        coclient_create(m_server_url.toStdString());
        publish_request();
    }
        break;
    default:
        break;
    }
}

void MainWindow::button_cancel_clicked()
{
    switch (m_mode) {
    case 0:
    {
        coclient_unregister_compute_node(m_app_guid);
        clear_app();
        ui->confirm->setEnabled(true);
        ui->publish_type->setEnabled(true);
    }
        break;
    case 2:
    {
        ui->server_url->setText(m_server_url);
        publish_request();
    }
        break;
    default:
        break;
    }
}

void MainWindow::button_view_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "C:\\", tr("All files(*.*)"));
    ui->app_alias->setText(filename);
}

std::string MainWindow::get_local_host_name()
{
    std::string host_name;
    DWORD name_len = 0;
    GetComputerName(NULL, &name_len);
    char* name = new char[name_len];
    GetComputerNameA(name, &name_len);
    host_name += name;
    host_name += "|";
    delete[] name;
    name_len = 0;
    GetUserNameA(NULL, &name_len);
    name = new char[name_len];
    GetUserNameA(name, &name_len);
    host_name += name;
    delete[] name;
    return host_name;
}

void MainWindow::mount_net_path()
{
    /*QProcess process;
    QStringList para;
    para << "-o";
    para << "nolock";
    para << "119.45.119.227:/home/ubuntu/nfsdoc";
    para << "N:";
    process.start("mount", para);
    process.waitForFinished(5000);
    qDebug() << "result " << process.readAll();*/
    QProcess::startDetached("mount_net_path.bat");
}

void MainWindow::umount_net_path()
{
    QProcess::startDetached("umount_net_path.bat");
}

void MainWindow::publish_app()
{
    QSettings settings(APP_LIST_KEY, QSettings::Registry64Format);
    settings.setValue("fDisabledAllowList", 1);
    settings.beginGroup(m_app_alias);
    settings.setValue("Name", m_app_alias);
    settings.setValue("Path", ui->app_alias->text());
    settings.setValue("UserName", ui->rdsh_user->text());
    settings.endGroup();
}

void MainWindow::clear_app()
{
    QSettings settings(APP_LIST_KEY, QSettings::Registry64Format);
    settings.remove(m_app_alias);
}
