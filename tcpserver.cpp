#include "tcpserver.h"
#include "common.h"
#include "connectionnode.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QCoreApplication>
#include <QByteArray>
#include <QUdpSocket>

TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{
    this->m_node_list = NULL;
    this->m_server = 0;


    this->initTcpServer();
}

void TcpServer::initTcpServer(){

    extern quint16 g_server_port;

    // Init Tcp Server For Tcp Node
    this->m_server =  new QTcpServer(this);
    if( g_server_port && this->m_server->listen(QHostAddress::Any,  g_server_port)){
        qDebug()<<"[TcpServer] listen port:"<<g_server_port << " success!";
    }else{
        qCritical()<<"[TcpServer] listen port:"<< g_server_port<< " error, \nExit Programs";
        QCoreApplication::exit(1);
    }

    this->connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection( )));

    // init socket client to connnect center
    this->m_center = new QTcpSocket(this);

    QHostAddress address("127.0.0.1");

    this->m_center->connectToHost(address,60006);
    if(this->m_center->waitForConnected(3000)){
        qDebug()<<"[initTcpServer] connect to host"<<address.toString()<<" success" ;
    }else{
       qDebug()<<"[initTcpServer] connect to host"<<address.toString()<<" error" ;
    }

    // Init Udp Server For Udp-mode Node
    this->m_udpserver = new QUdpSocket(this);
    this->m_udpserver->bind(g_server_port);
}

void TcpServer::newConnection(){

#ifdef FUNC_BUG
    qDebug()<<"[f-TcpServer] func newConnection";
#endif

    QTcpSocket *temp  = this->m_server->nextPendingConnection();
    if(temp->waitForReadyRead(5000)){   // ready for regedit packet
        QByteArray regedit_p= temp->read(8);
        QString dtu_id(regedit_p);
        ConnectionNode* tmp  = this->getConnectNode(dtu_id);
        if(tmp == NULL){ // if cannot find the node , than create a now one
            ConnectionNode *node  = new ConnectionNode(this, temp);
            node->next = this->m_node_list;
            this->m_node_list = node;
            qDebug()<<"[newConnection] create new node nodeId:" ;
        }else{ // if this node exsit, just replase the sock
            tmp->setSock(temp);
        }
    }else{
        qDebug()<<"[neConnection] get the regedit packet timeout , disconnect ";
        temp->disconnectFromHost();
        return ;
    }
    qDebug()<<"[newConntion] ip:"<<temp->peerAddress().toString()<<":"<<temp->peerPort();
    //    this->m_node_list->next = node;
}

void TcpServer::readConnectionData(const QByteArray& data){
    this->m_center->write(data);
    this->m_center->waitForBytesWritten(2000);
}

void TcpServer::ParseNodeData(ConnectionNode *node){

    qint8 d=0;
    qint16 offset = 0;
    qint8 cmd=0;
    qint16 packet_len;
    QString dtu_id;

    // if the data size is to short, return wait for more data;
    if(node->m_data.size() < 0x10){
        return ;
    }

    // get the first start mark location.
    while(offset< node->m_data.size()){
        d=node->m_data.at(offset++) ;
        if(d != 0x7b) continue;
    }

    // get the packet type
    if(offset < node->m_data.size())
    {
        cmd = node->m_data.at(offset++);
    }

#ifdef DATA_BUG
    qDebug()<<"packet_cmd:"<< cmd <<endl;
#endif

    // left data must contain the id and end mark
    if(node->m_data.size() - offset < 14 )
    {
        return ;
    }

    packet_len  = node->m_data.mid(offset, 2).toUShort();
    offset += 2; // skip the len bytes

    dtu_id = QString( node->m_data.mid(offset, 11));
    offset += 11;  // skip the id vlaue

#ifdef DATA_BUG
    qDebug()<<"packet_len:"<<packet_len<<" dtu_id:"<<dtu_id<<endl;
#endif

    switch (cmd) {

    /*
               *  Regedit Packet
               *
               *  启始标志	 包类型 	  包长度	    DTU身份识别码	  DTU IP 地址	DTU通信端口	  结束标志
               *   1 byte	 1 byte   2 bytes	 11 bytes	    4 bytes	     2 bytes	  1 byte
               *  0x7b	0x01	0x00 0x16				                               0x7b
               *
               */
    case 0x01:  // regedit packet
    {
        quint32 ip = node->m_data.mid(offset, 4).toUInt();
        offset += 4;
        quint16 port  = node->m_data.mid(offset, 2).toUShort();
        offset += 2;

#ifdef DATA_BUG
        qDebug()<<"ip:"<<ip<<" port:"<<port;
#endif
        //              quint node_id = g_node_id++;

        ConnectionNode* tmp  = this->m_node_list;
        while(tmp != NULL){ // check if the node exsit
            if(tmp->m_dtuId ==  dtu_id) break;
            tmp = tmp->next;
        }

        if(tmp == NULL){   // the node isnot exsit , add the node to list
            node->initNode(g_node_id++,port,ip,dtu_id);
            //            node->next = this->m_node_list;
            //            this->m_node_list = node;
        }else{ // the node have exsit, just replase the argment to old node
            tmp->setSock(node->sock());
            //            tmp->initNode(tmp->m_dtuId, port,ip,dtu_id);=
            //            tmp->InitNode(node);
        }

        break;
    }
        /*
                   *  Application  Packet
                   *
                   *  启始标志   包类型     包长度      DTU身份识别码     结束标志           应用数据
                   *   1 byte   1 byte     2 bytes     11 bytes        1 byte      小于1024 bytes
                   *   0x7b      0x09      0x00 0x10                   0x7b
                   *
                   */
        // application data packet
    case 9:
    {
        offset += 1 ; // skip the end mark ;
        QByteArray app_data =node->m_data.right(node->m_data.size() - offset);
        this->SendDataToCenter(node,app_data);
        break;
    }
        /*
                   *  Reply Packet
                   *
                   *  启始标志     包类型        包长度         DTU身份识别码        结束标志
                   *   1 byte     1 byte       2 bytes         11 bytes        1 byte
                   *   0x7b        0x81        0x00 0x10                       0x7b
                   *
                   */

    case 0x81:
        qDebug("0x81");
        break;

    default:
        qDebug("errer");
        break;
    }
}

void TcpServer::SendDataToCenter(ConnectionNode* node, QByteArray data){
#ifdef FUNC_BUG
    qDebug()<<"[TcpServer] func SendDataToCenter ";
#endif

#ifdef DATA_BUG
    qDebug()<<"node id:"<<node->nodeId()<<"  dtu_id:"<<node->m_dtuId<< " data size:"<<data.size();
    qDebug()<<"data:"<<data.toHex();
#endif

    QByteArray send_data;
    QDataStream out(&send_data,QIODevice::ReadWrite);
    out.setVersion(QDataStream::Qt_5_3);
    out<<(data.size()+2) << node->nodeId()<<data;

    this->m_center->write(send_data);

#ifdef DATA_BUG
    qDebug()<<"[TcpServer] send data OVER";
#endif

}

void TcpServer::connectToCenter(){

#ifdef FUNC_BUG
    qDebug()<<"[TcpServer] func connectToCenter";
#endif

    QString center_ip = "10.22.3.11";
    quint16 center_port= 60000;
    if(this->m_center == NULL){
        this->m_center = new QTcpSocket(this);
    }
    QHostAddress address(center_ip);
    m_center->connectToHost(address, center_port);
    if(m_center->waitForConnected()){

        qDebug()<<"[TcpServer] connect IP:"<<center_ip<<" port:"<<center_port
               <<" success"<<", loca port:"<<m_center->localPort();
        connect(m_center,SIGNAL(readyRead()), this, SLOT(readCenterData()));

    }else{
        qDebug()<<"[TcpServer] connect IP:"<<center_ip<<" port:"<<center_port<<" timeout";
    }
}


void TcpServer::readCenterData(){
#ifdef FUNC_BUG
    qDebug()<<"[TcpServer] func readCenterData" <<;
#endif

    QByteArray data = this->m_center->readAll();

#ifdef DATA_BUG
    qDebug()<<" [f-readCenterData] read data:"<<data.toHex();
#endif
    qDebug()<<"read data:" << data.toHex();
}

ConnectionNode* TcpServer::getConnectNode(const QString &id){
    ConnectionNode* tmp = this->m_node_list;
    while(tmp != NULL){
        if(id == tmp->m_dtuId) break;
    }
    return tmp;
}


void TcpServer::readUdpData(){
   QByteArray f_data = this->m_udpserver->readAll();

}
