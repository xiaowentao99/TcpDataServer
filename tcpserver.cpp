#include "tcpserver.h"
#include <QTcpServer>
#include <QTcpSocket>


TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{
}

TcpServer::init(){

    this->m_server =  new QTcpServer(this);
   if( this->m_server->listen(QHostAddress::Any, this->g_server_port)){

   }

    this->connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnect( )));

}
