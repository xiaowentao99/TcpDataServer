#ifndef CONNECTIONNODE_H
#define CONNECTIONNODE_H

#include <QObject>

class QTcpSocket;

class ConnectionNode : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionNode(QObject *parent = 0);

signals:

public slots:
       void readData();

private:

    unsigned int node_id;
    QTcpSocket *sock;

};

#endif // CONNECTIONNODE_H
