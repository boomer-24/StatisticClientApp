#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QSystemTrayIcon>
#include <QTcpSocket>
#include <QTextCodec>
#include <QtXml>
#include <QFile>
#include <QDebug>
#include <QCloseEvent>
#include <QDir>
#include <QRegExp>
#include <QFileDialog>
#include <QDateTime>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QMainWindow *parent = 0);
    MainWindow(const QString& strHost, int nPort, QMainWindow* pwgt = 0) ;
    ~MainWindow();

    void Initialize(const QString& _xmlPath);    
    void SendText(const QString& _text);
    bool SendFile(const QString& _filePath);
    void SetAddressAndPort(const QString& _hostAddress, const quint16& _hostPort);
    void ConnectToServer();
    void AutoRun();

private slots:
    void slotDirChanged(const QString& _pathChangedDir);
    void slotConnected();
    void slotReadyRead();
    void slotIconActivated(QSystemTrayIcon::ActivationReason _reason);
    void slotError(QAbstractSocket::SocketError _err);
    void slotSocketChangeState(QAbstractSocket::SocketState _socketState);

    void on_pushButton_connect_clicked();    

private:
    Ui::MainWindow *ui_;
    QSystemTrayIcon trayIcon_;
    QFileSystemWatcher watcher_;

    QTcpSocket* tcpSocket_;
    QString serverAddress_;
    quint16 serverPort_;

    QDataStream dataStreamIn_;
    quint32 nextBlockSize_;
    QString testerNumber_;
    QString watchedDir_;
    QString pathChangedDir_;

    const quint16 CHECK_CONNECTION_IDENTIFIER = 0;
    const quint16 TEXT_IDENTIFIER = 1;
    const quint16 CSV_IDENTIFIER = 2;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MAINWINDOW_H
