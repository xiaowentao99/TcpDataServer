#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>

class ConnectionNode;
class QTcpServer;
class QUdpSocket;
class QTcpSocket;

class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = 0);

    void ParseNodeData(ConnectionNode*);
    void SendDataToCenter(ConnectionNode*, QByteArray data);
    void connectToCenter();
    void readUdpData();

    ConnectionNode* getConnectNode(const QString&);

private:

    void initTcpServer();

signals:

public slots:
    void newConnection( );
    void readConnectionData(const QByteArray& data);
    void readCenterData();

private:

    QTcpServer *m_server;
    QUdpSocket *m_udpserver;
    QTcpSocket *m_center;

    ConnectionNode* m_node_list;

    QByteArray m_data;


};

#endif // TCPSERVER_H
