#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    this->setWindowTitle("ClientTester");
    this->trayIcon_.setIcon(this->style()->standardIcon(QStyle::SP_CustomBase));
    this->trayIcon_.setToolTip("Statistic System");
    QMenu* menu = new QMenu(this);
    QAction* viewWindow = new QAction(trUtf8("Развернуть окно"), this);
    QAction* quitAction = new QAction(trUtf8("Выход"), this);
    QObject::connect(viewWindow, SIGNAL(triggered()), this, SLOT(show()));
    QObject::connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(viewWindow);
    menu->addAction(quitAction);
    this->trayIcon_.setContextMenu(menu);
    this->trayIcon_.show();
    QObject::connect(&this->trayIcon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(slotIconActivated(QSystemTrayIcon::ActivationReason)));
    QObject::connect(&this->watcher_, SIGNAL(directoryChanged(QString)), this, SLOT(slotDirChanged(QString)));

    this->ui_->textBrowser_info->setReadOnly(true);

    this->tcpSocket_ = new QTcpSocket(this);
    connect(tcpSocket_, SIGNAL(connected()),
            this, SLOT(slotConnected()));
    connect(tcpSocket_, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
    connect(tcpSocket_, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
    connect(tcpSocket_, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(slotSocketChangeState(QAbstractSocket::SocketState)));
    this->dataStreamIn_.setVersion(QDataStream::Qt_5_8);
    this->nextBlockSize_ = 0;

    this->Initialize(QCoreApplication::applicationDirPath().append("/ini.xml"));
}

MainWindow::MainWindow(const QString &strHost, int nPort, QMainWindow *pwgt)
    : QMainWindow(pwgt), ui_(new Ui::MainWindow), nextBlockSize_(0)
{
    this->tcpSocket_ = new QTcpSocket(this);
    connect(tcpSocket_, SIGNAL(connected()), SLOT(slotConnected()));
    connect(tcpSocket_, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(tcpSocket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
    this->dataStreamIn_.setVersion(QDataStream::Qt_5_8);
    this->ui_->setupUi(this);
    this->Initialize(QCoreApplication::applicationDirPath().append("/ini.xml"));
}

MainWindow::~MainWindow()
{
    this->AutoRun();
    delete ui_;
    if (this->tcpSocket_->state() == QAbstractSocket::ConnectedState)
        this->tcpSocket_->close();
}

void MainWindow::Initialize(const QString &_xmlPath)
{
    QDomDocument domDoc;
    QFile file(_xmlPath);
    if (file.open(QIODevice::ReadOnly))
    {
        if (domDoc.setContent(&file))
        {
            QDomElement domElement = domDoc.documentElement();
            QDomNode domNode = domElement.firstChild();
            while(!domNode.isNull())
            {
                if (domNode.isElement())
                {
                    QDomElement domElement = domNode.toElement();
                    if (!domElement.isNull())
                    {
                        if (domElement.tagName() == "pathToWatchedDir")
                        {
                            this->watcher_.addPath(domElement.text());
                            this->watchedDir_ = domElement.text();
                        } else if (domElement.tagName() == "testerNumber")
                        {
                            this->testerNumber_ = domElement.text();
                        } else if (domElement.tagName() == "serverIp")
                        {
                            this->serverAddress_ = domElement.text();
                        } else if (domElement.tagName() == "serverPort")
                        {
                            this->serverPort_ = domElement.text().toInt();
                        }
                    }
                    domNode = domNode.nextSibling();
                }
            }
        }
    }
}

void MainWindow::SendText(const QString &_text)
{
    if (this->tcpSocket_->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_8);
        out << quint32(0) << this->testerNumber_ << this->TEXT_IDENTIFIER << _text;
        out.device()->seek(0);
        out << quint32(arrBlock.size() - sizeof(quint32));
        this->tcpSocket_->write(arrBlock);
    } else this->ui_->textBrowser_info->append(QTime::currentTime().toString().
                                               append(QString(" Send text attempt !!!  UnConnectedState")));
}

bool MainWindow::SendFile(const QString &_filePath)
{
    if (this->tcpSocket_->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_8);
        QFile file(_filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            this->ui_->textBrowser_info->append(QString(_filePath).append(" not open"));
            return false;
        }
        else
        {
            QStringList slPath(_filePath.split("/"));
            QString fileName(slPath.last());
            out << quint32(0) << this->testerNumber_ << this->CSV_IDENTIFIER << fileName;
            QByteArray content = file.readAll();
            out << content;
            out.device()->seek(0);
            out << quint32(arrBlock.size() - sizeof(quint32));
            this->ui_->textBrowser_info->append(QTime::currentTime().toString().
                                                append(QString("   send csv   ").append(fileName)));
            file.close();
            this->tcpSocket_->write(arrBlock);
            return true;
        }
    } else
    {
        this->ui_->textBrowser_info->append(QTime::currentTime().toString().
                                            append(QString(" Send file attempt !!!  UnConnectedState")));
        return false;
    }
}

void MainWindow::SetAddressAndPort(const QString &_hostAddress, const quint16 &_hostPort)
{
    this->serverAddress_ = _hostAddress;
    this->serverPort_ = _hostPort;
}

void MainWindow::ConnectToServer()
{
    //    this->tcpSocket_->abort();
    //    this->tcpSocket_->connectToHost(this->serverAddress_, this->serverPort_);
    //    if (this->tcpSocket_->waitForConnected(1000))
    //    {
    //        if (!QDir(this->watchedDir_).exists())
    //            this->SendText(" для мониторинга установлена несуществующая директория");
    //        else this->SendText(QString(this->testerNumber_).append(": всё норм"));
    //    }
}

void MainWindow::AutoRun()
{
    QSettings setting("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
//    QString path(QDir::toNativeSeparators(QApplication::applicationFilePath()));
    setting.setValue("StatisticSystemClient", QDir::toNativeSeparators(QApplication::applicationFilePath()));
    setting.sync();
}

void MainWindow::slotDirChanged(const QString &_pathChangedDir)
{
    QDir changedDir(_pathChangedDir);
    QStringList dirEntry(changedDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
    for (QString& smth: dirEntry)
    {
        if (smth.contains(QRegExp("csv"))) // СДЕЛАТЬ НОРМ ТИПА *.CSV& (КОНЕЦ СТРОКИ)
        {
            QString path(_pathChangedDir);
            path.append("/").append(smth);
            if (this->SendFile(path))
                QFile::remove(path);
        }
    }
    this->nextBlockSize_ = 0;
}

void MainWindow::slotReadyRead()
{
    quint16 packageContent;
    int bytesAvailable;
    //    this->dataStreamIn_.setVersion(QDataStream::Qt_5_8);
    for ( ; ; )
    {
        if (!this->nextBlockSize_)
        {
            bytesAvailable = this->tcpSocket_->bytesAvailable();
            if (bytesAvailable < sizeof(quint32))
                break;
            this->dataStreamIn_ >> this->nextBlockSize_;
        }
        bytesAvailable = this->tcpSocket_->bytesAvailable();
        if (bytesAvailable < this->nextBlockSize_)
            break;
        this->dataStreamIn_ >> packageContent;
        if (packageContent == this->CHECK_CONNECTION_IDENTIFIER)
        {
            QDir changedDir(this->pathChangedDir_);
            QStringList dirEntry(changedDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
            for (QString& smth: dirEntry)
            {
                if (smth.contains(QRegExp("csv"))) // СДЕЛАТЬ НОРМ ТИПА *.CSV& (КОНЕЦ СТРОКИ)
                {
                    QString path(this->pathChangedDir_);
                    path.append("/").append(smth);
                    if (this->SendFile(path))
                        QFile::remove(path);
                }
            }
            this->nextBlockSize_ = 0;
        }
    }
}

void MainWindow::slotConnected()
{
    this->dataStreamIn_.setDevice(this->tcpSocket_);
    if (!QDir(this->watchedDir_).exists())
        this->SendText("для мониторинга установлена несуществующая директория");
    //    else this->SendText("Connection established");
}

void MainWindow::slotIconActivated(QSystemTrayIcon::ActivationReason _reason)
{
    switch (_reason)
    {
    case QSystemTrayIcon::Trigger:
        if (!this->isVisible())
            this->show();
        else
            this->hide();
        break;
    default:
        break;
    }
}

void MainWindow::slotError(QAbstractSocket::SocketError _err)
{
    QString strError =
            "Error: " + (_err == QAbstractSocket::HostNotFoundError ?
                             "The host was not found." :
                             _err == QAbstractSocket::RemoteHostClosedError ?
                                 "The remote host is closed." :
                                 _err == QAbstractSocket::ConnectionRefusedError ?
                                     "The connection was refused." :
                                     QString(this->tcpSocket_ ->errorString())
                                     );
    this->ui_->textBrowser_info->append(strError);
}

void MainWindow::slotSocketChangeState(QAbstractSocket::SocketState _socketState)
{
    qDebug() << "SOCKET STATE CHANGED !!!   " << _socketState;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (this->isVisible())
    {
        event->ignore();
        this->hide();
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
//        this->trayIcon_.showMessage("Statistic System", trUtf8("Приложение свернуто в трей."), icon, 200);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    this->ui_->textBrowser_info->setFixedSize(this->width() - 20, this->height() - 80);
    this->ui_->pushButton_connect->move(this->ui_->pushButton_connect->x(), this->ui_->textBrowser_info->height() + 20);
}

void MainWindow::on_pushButton_connect_clicked()
{
    this->tcpSocket_->connectToHost(this->serverAddress_, this->serverPort_);
    //        this->tcpSocket_->connectToHost("192.168.1.113", 5000);
}
