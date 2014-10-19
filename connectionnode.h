#ifndef CONNECTIONNODE_H
#define CONNECTIONNODE_H

#include <QObject>

class QTcpSocket;

class ConnectionNode : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionNode(QObject *parent , QTcpSocket * tcp);

    void responseDTU();
    void sendDataToDTU(QByteArray data);

    void initNode(qint16,qint32, quint16, QString);
    void initNode(QTcpSocket *sock);
    QTcpSocket *sock() const;
    void setSock(QTcpSocket *sock);

signals:
    void newData(const QByteArray &);

public slots:
    void readData();

public:

    ConnectionNode* next;
    QTcpSocket *m_sock;

    QByteArray m_data;

    bool iswork;

    quint16 dtuPort() const;
    void setDtuPort(const quint16 &dtuPort);
    QString m_dtuId;


    quint16 nodeId() const;
    void setNodeId(const quint16 &nodeId);


private:

    void parseTcpSocketData();

    quint16 m_nodeId;
    qint64 m_lastUpdate;

    qint32 m_dtuIp;
    quint16 m_dtuPort;


};

#endif // CONNECTIONNODE_H
