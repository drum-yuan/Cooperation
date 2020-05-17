#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->publish_type->addItem("本机桌面");
    ui->publish_type->addItem("本机应用");
    ui->publish_type->addItem("远程应用");
    ui->publish_type->setCurrentIndex(0);

    ui->list->hide();
    ui->server_url->hide();

    m_mode = 0;

    connect(ui->query, SIGNAL(clicked()), this, SLOT(query_request()));
    connect(ui->publish, SIGNAL(clicked()), this, SLOT(publish_request()));
    connect(ui->config, SIGNAL(clicked()), this, SLOT(config_request()));
    connect(ui->publish_type, SIGNAL(currentIndexChanged(QString)), this, SLOT(publish_type_changed(const QString&)));
}

MainWindow::~MainWindow()
{
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
}

void MainWindow::publish_request()
{
    m_mode = 0;
    ui->publish_type->show();
    ui->rdsh_url->show();
    ui->rdsh_domain->show();
    ui->rdsh_user->show();
    ui->rdsh_password->show();
    ui->app_alias->show();
    ui->app_view->show();
    ui->confirm->show();
    ui->cancel->show();
    ui->list->hide();
    ui->server_url->hide();
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
}

void MainWindow::publish_type_changed(const QString& text)
{
    if (text == "本机桌面") {
        ui->rdsh_url->setEnabled(false);
        ui->rdsh_domain->setEnabled(false);
        ui->rdsh_user->setEnabled(false);
        ui->rdsh_password->setEnabled(false);
        ui->app_alias->setEnabled(false);
        ui->app_view->setEnabled(false);
    } else if (text == "本机应用") {
        ui->rdsh_url->setEnabled(false);
        ui->rdsh_domain->setEnabled(false);
        ui->rdsh_user->setEnabled(false);
        ui->rdsh_password->setEnabled(false);
        ui->app_alias->setEnabled(true);
        ui->app_alias->setPlaceholderText("应用路径");
        ui->app_view->setEnabled(true);
    } else {
        ui->rdsh_url->setEnabled(true);
        ui->rdsh_domain->setEnabled(true);
        ui->rdsh_user->setEnabled(true);
        ui->rdsh_password->setEnabled(true);
        ui->app_alias->setEnabled(true);
        ui->app_alias->setPlaceholderText("应用别名");
        ui->app_view->setEnabled(false);
    }
}

void MainWindow::button_confirm_clicked()
{
    switch (m_mode) {
    case 0:
    {
        ui->confirm->setEnabled(false);
        ui->publish_type->setEnabled(false);
        ui->rdsh_url->setEnabled(false);
        ui->rdsh_domain->setEnabled(false);
        ui->rdsh_user->setEnabled(false);
        ui->rdsh_password->setEnabled(false);
        ui->app_alias->setEnabled(false);
        ui->app_view->setEnabled(false);
    }
        break;
    case 1:
        break;
    case 2:
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
        ui->confirm->setEnabled(true);
        ui->publish_type->setEnabled(true);
        ui->rdsh_url->setEnabled(true);
        ui->rdsh_domain->setEnabled(true);
        ui->rdsh_user->setEnabled(true);
        ui->rdsh_password->setEnabled(true);
        ui->app_alias->setEnabled(true);
        ui->app_view->setEnabled(true);
    }
        break;
    case 1:
        break;
    case 2:
        break;
    default:
        break;
    }
}

void MainWindow::button_view_clicked()
{
    switch (m_mode) {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    default:
        break;
    }
}
