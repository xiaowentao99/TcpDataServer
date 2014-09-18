#include "tcpserver.h"
#include "common.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QCoreApplication>

TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{
}

TcpServer::init(){

    this->m_server =  new QTcpServer(this);
   if( g_server_port && this->m_server->listen(QHostAddress::Any,  g_server_port)){
        qDebug()<<"[TcpServer] listen port"<<g_server_port << " success!";
   }else{
        qCritical()<<"[TcpServer] listen port  "<< g_server_port<< " error, \nExit Programs";
        QCoreApplication::exit(1);
   }

   this->connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection( )));

}

TcpServer::newConnection(){

    QTcpSocket *tmep  = this->m_server->nextPendingConnection();


}
