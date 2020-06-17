#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "co-interface.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void query_request();
    void publish_request();
    void config_request();
    void publish_type_changed(const QString& text);
    void button_confirm_clicked();
    void button_cancel_clicked();
    void button_view_clicked();

private:
    std::string get_local_host_name();
    void mount_net_path();
    void umount_net_path();
    void publish_app();
    void clear_app();

    Ui::MainWindow *ui;
    int m_mode;
    std::vector<NodeInfo> m_node_list;
    RDSHInfo m_rdsh_info;
    std::string m_app_guid;
    QString m_server_url;
    QString m_app_alias;
};
#endif // MAINWINDOW_H
