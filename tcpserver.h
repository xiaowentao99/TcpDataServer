#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);

private:

    void init();


signals:


public slots:
    void newConnection( );

private:

    QTcpServer *m_server;


};

#endif // TCPSERVER_H
