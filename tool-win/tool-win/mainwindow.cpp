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
}

MainWindow::~MainWindow()
{
    delete ui;
}

