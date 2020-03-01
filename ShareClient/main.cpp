#include "widget.h"
#include <QApplication>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QDateTime>

void verboseMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString text = QString("");
    static const char* typeStr[] = {"[Debug]", "[Warning]", "[Critical]", "[Fatal]" };

    if (type <= QtFatalMsg)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        QString current_date(QDateTime::currentDateTime().toString("dd-MM-yy HH:mm:ss:zzz"));

        QString message = text.append(typeStr[type]).append(" ").append(current_date).append(": ").append(msg);
        QFile file("client.log");
        if (file.size() > 1024 * 1024) {
            file.remove();
        }
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream(&file);
        text_stream << message << "\r\n";
        file.close();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(verboseMessageHandler);

    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
