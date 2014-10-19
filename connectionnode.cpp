#include "connectionnode.h"
#include "common.h"
#include <QTcpSocket>
#include <QDateTime>

ConnectionNode::ConnectionNode(QObject *parent , QTcpSocket *tcp) :
    QObject(parent)
{
    this->next = 0;
    this->m_sock = NULL;
    this->m_dtuId = "";
    this->m_dtuIp = 0;
    this->m_dtuPort = 0;
    this->m_nodeId = 0;

    //  this->connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection( )));
}

void ConnectionNode::readData(){

    this->m_lastUpdate = QDateTime::currentMSecsSinceEpoch();
    this->m_data.append( this->m_sock->readAll());



    //    emit this->newData(this);
}

void ConnectionNode::parseTcpSocketData(){

#if DEBUG_LEVEL&DEBUG_THREAD

#endif

    //     qDebug()<<"[CheckData] data:"<<value.toHex();

    //////////////////////////////////////////////////////////////////////
    //                      Base Packet                                 //
    //name:       Flag       |   CRC  |   Port  |  Len  |   Data        //
    //Len :   0xFF FF FF FF  |    2   |    2    |   2   |  1~65535      //
    //Note:   Every command packet means a data packet                  //
    //////////////////////////////////////////////////////////////////////
    //                      Data Packet                                 //
    //Name:   CRC | Command | Len | READER ID | Tag Counter  | Tag Data //
    //Len :    2  |    2    |  2  |     4     |      1       |    70*n  //
    //Note:   value of n equal the value of tag counter                 //
    //////////////////////////////////////////////////////////////////////

//    static qint32 data_crc_counter=0;
//    static qint32 no_start_counter=0;
//    static qint32 send_counter=0;
    static QByteArray rfid_start_flag;
    static const char start_mark=0xFF;
    rfid_start_flag.append(start_mark);
    rfid_start_flag.append(start_mark);
    rfid_start_flag.append(start_mark);
    rfid_start_flag.append(start_mark);

    while(this->m_data.size() > MINI_PACKET_LENGTH){
        qint16 offset = this->m_data.indexOf(rfid_start_flag);

        if(offset != 0){
            if(offset > 0){
                m_data.remove(0, offset);
            }else {
                m_data.clear();
            }
            continue;
        }

        offset += rfid_start_flag.size(); // skip start flag
        quint16 checksum = m_data.at(offset);
        checksum &= 0x00FF;
        checksum <<= 8;
        checksum |= (m_data.at(offset+1) & 0x00FF);
        offset += 2; // skip checksum char
        if(checksum == GetCRC16((quint8*)(m_data.data()+offset), 4)){
            offset += 2; // skip device port;
            quint16 data_len = m_data.at(offset);
            data_len &= 0x00FF;
            data_len <<= 8;
            data_len |= (m_data.at(offset+1) & 0x00FF);
            offset += 2; // skip data length
            if(m_data.size() >= (data_len+offset)){

                quint16 data_checksum = m_data.at(offset);
                data_checksum &= 0x00FF;
                data_checksum <<= 8;
                data_checksum |= (m_data.at(offset+1) & 0x00FF);
                offset += 2; // skip data crc
                if(data_checksum == GetCRC16((quint8*)(this->m_data.data()+offset),data_len-2)){

                    offset += (data_len-2);

                    QByteArray client_data;
                    client_data.append((char)m_nodeId);
                    client_data.append((char)m_nodeId>>8);
                    client_data.append((char)offset);
                    client_data.append((char)offset>>8);
                    client_data.append(this->m_data.left(offset));
                    emit this->newData(client_data);

                    this->m_data.remove(0, offset);

//                    send_counter++;
                }else{
                    qint16 next_offset = this->m_data.indexOf(rfid_start_flag, 1);
                    if(next_offset > 0){
                        this->m_data.remove(0, next_offset);
                    }else{
                        this->m_data.clear();
                    }
                    // crc checkout error
                }
            }else{
                // we donot have a complete packet.
                break ;
            }
        }else{
            // if the header CRC checksum error.
            qint16 next_offset = this->m_data.indexOf(rfid_start_flag, 1);

            if(next_offset > 0){
                this->m_data.remove(0, next_offset);
            }else{
                this->m_data.clear();
            }
        }
        // if a packet is send , we continue check the next packet.
    }
}


QTcpSocket *ConnectionNode::sock() const
{
    return m_sock;
}

void ConnectionNode::setSock(QTcpSocket *sock)
{

    if(m_sock != NULL){
        m_sock->disconnect(SIGNAL(readyRead()));
        m_sock->disconnectFromHost();
        delete m_sock;
        m_sock = NULL;
    }
    m_sock = sock;
    //    this->m_nodeId =  this->m_sock->localPort();
    this->connect(this->m_sock,SIGNAL(readyRead()),this, SLOT(readData()));
    this->connect(this->m_sock, SIGNAL(disconnected()), m_sock, SLOT(deleteLater()));

}


quint16 ConnectionNode::dtuPort() const
{
    return m_dtuPort;
}

void ConnectionNode::setDtuPort(const quint16 &dtuPort)
{
    m_dtuPort = dtuPort;
}
quint16 ConnectionNode::nodeId() const
{
    return m_nodeId;
}

void ConnectionNode::setNodeId(const quint16 &nodeId)
{
    m_nodeId = nodeId;
}

void ConnectionNode::initNode(qint16 node_id, qint32 ip, quint16 port, QString id){

    this->m_nodeId =  node_id;
    this->m_dtuIp = ip;
    this->m_dtuPort =  port;
    this->m_dtuId = id;
    this->iswork = true;

}

void ConnectionNode::responseDTU(){
    QByteArray reply;
    QDataStream out(&reply, QIODevice::ReadWrite);
    out.setVersion(QDataStream::Qt_5_3);
    out<<0x7b<<0x81<<0x00<<0x01<<this->m_dtuId<<0x7b;
    this->m_sock->write(reply);

#ifdef DATA_BUG
    qDebug()<<"[responseDTU] reply data:"<<reply.toHex();
#endif
}

