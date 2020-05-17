#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    Ui::MainWindow *ui;
    int m_mode;
};
#endif // MAINWINDOW_H
