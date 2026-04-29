#include "TcpClient.h"
#include "ThirdLog.h"
#include "../../OuterFactoryImp.h"
#include "globe.h"
#include <fstream>
#include <iostream>
#include <string>
#include <google/protobuf/util/time_util.h>
#include <google/protobuf/text_format.h>
#include "RoomServant.h"
#include "../../OuterFactoryImp.h"
#include "RoomServer.h"

using namespace std;
using namespace Pb;

static uint64_t timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static uint64_t seqno()
{
    static uint64_t id = 0;
    if ((++id) == 0)
        id = 1;

    return id;
}

CTcpClient::CTcpClient(CAsyncEpoller *epoller, const string& roomid) : CAsyncSocket(epoller)
{
    setFd(-1);

    memset(_buffer, 0, sizeof(_buffer));
    _recvBuffer = new CRawBuffer();
    _recvBuffer->reset();
    _sendBuffer = new CRawBuffer();
    _recvBuffer->reset();
    _connected = false;
    _reconnect = false;
    memset(_svraddr, 0, sizeof(_svraddr));
    _svrport = 0;
    _svrtimeout = 5000;
    memset(_packet, 0, PACK_MAXLEN);
    _connid = 0;
    _svrid = 0;
    memset(_token, 0, sizeof(_token));

    //Test
    _connid = 99999;
    _svrid = 0x09000001;
    strncpy(_token, "pass110002", sizeof(_token) - 1);

    //Protobuf print
    _textFormatPrint = true;

    _roomid = roomid;
}

CTcpClient::~CTcpClient()
{
    if (_recvBuffer)
    {
        delete _recvBuffer;
        _recvBuffer = NULL;
    }

    if (_sendBuffer)
    {
        delete _sendBuffer;
        _sendBuffer = NULL;
    }
}

const int32_t CTcpClient::getSvrID()
{
    return _svrid;
}

void CTcpClient::setSvrID(const int32_t id)
{
    _svrid = id;
}

const char *CTcpClient::getSvrAddr()
{
    return &(_svraddr[0]);
}

void CTcpClient::setSvrAddr(const char *addr)
{
    memset(_svraddr, 0, sizeof(_svraddr));
    strncpy(_svraddr, addr, sizeof(_svraddr) - 1);
}

const int16_t CTcpClient::getSvrPort()
{
    return _svrport;
}

void CTcpClient::setSvrPort(const int16_t port)
{
    _svrport = port;
}

const char *CTcpClient::getSvrToken()
{
    return &(_token[0]);
}

void CTcpClient::setSvrToken(const char *token)
{
    memset(_token, 0, sizeof(_token));
    strncpy(_token, token, sizeof(_token) - 1);
}

bool CTcpClient::isConnected()
{
    return _connected;
}

void CTcpClient::intputNotify()
{
    Recv();
    Dispatch();
}

void CTcpClient::outputNotify()
{
    // THIRD_DEBUG("-------------outputNotify-------------");
    Send();
}

void CTcpClient::errorNotify()
{
    // THIRD_DEBUG("-------------errorNotify-------------");
    Error();
}

void CTcpClient::timerNotify()
{
    // THIRD_DEBUG("-------------timerNotify-------------");
}

void CTcpClient::connectChecker()
{
    uint64_t ts = timestamp();

    if ((0 == getLastAccessTime()) || (ts >  (getLastAccessTime() + _svrtimeout)))
    {
        if (_connected)
        {
            NodeKeepAliveMsg();
            setLastAccessTime(ts);
        }
        else
        {
            if (0 == Connect())
            {
                NodeKeepAliveMsg();
                setLastAccessTime(ts);
            }
        }

        //THIRD_DEBUG("connection to check. fd[%d] [%d] [%ld]", getFd(), _connected, getLastAccessTime());
    }
}

int CTcpClient::Connect(const char *addr, const uint16_t port, const int32_t timeout)
{
    if (timeout <= 0)
    {
        THIRD_ERROR("invalid params, timeout[%d]", timeout);
        return -1;
    }

    memset(_svraddr, 0, sizeof(_svraddr));
    strncpy(_svraddr, addr, sizeof(_svraddr) - 1);
    _svrport = port;
    _svrtimeout = timeout;
    if (-1 == Reconnect())
    {
        THIRD_ERROR("connect to server fail, timeout[%d]", timeout);
        return -1;
    }

    NodeLoginMsg();
    return 0;
}

int CTcpClient::Connect(const int32_t timeout)
{
    THIRD_DEBUG("Connect ckeck, timeout[%d]", timeout);
    if (timeout <= 0)
    {
        THIRD_ERROR("invalid params, timeout[%d]", timeout);
        return -1;
    }

    if (-1 == Reconnect())
    {
        THIRD_ERROR("connect to server fail, timeout[%d]", timeout);
        return -1;
    }

    NodeLoginMsg();
    return 0;
}

int CTcpClient::Reconnect()
{
    int32_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        THIRD_ERROR("socket(AF_INET) create fail: err=%s", strerror(errno));
        return -1;
    }

    int32_t opt = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt)))
    {
        Close();
        THIRD_ERROR("setsockopt(SO_REUSEADDR) fail: err=%s", strerror(errno));
        return -1;
    }
    // struct timeval tv;
    // tv.tv_sec = _svrtimeout;
    // tv.tv_usec = 0;
    // if (-1 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)))
    // {
    //     Close();
    //     THIRD_ERROR("setsockopt(SO_RCVTIMEO) fail: err=%s", strerror(errno));
    //     return -1;
    // }

    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(_svraddr);
    sockAddr.sin_port = htons(_svrport);
    if (-1 == connect(fd, (const struct sockaddr *)&sockAddr, sizeof(sockAddr)))
    {
        close(fd);
        Close();
        THIRD_ERROR("connect() fail: addr[%s:%d], err[%s]", _svraddr, _svrport, strerror(errno));
        return -1;
    }

    if (-1 == SetNonblocking(fd))
    {
        THIRD_ERROR("SetNonblocking(fd) fail: addr[%s:%d], err[%s]",  _svraddr, _svrport, strerror(errno));
        close(fd);
        Close();
        return -1;
    }

    _recvBuffer->reset();
    _sendBuffer->reset();

    setFd(fd);

    addEpoller();

    _connected = true;
    THIRD_ERROR("reconnect to serever succ: err=%s", strerror(errno));
    return 0;
}

int CTcpClient::Recv()
{
    if (-1 == getFd())
    {
        THIRD_ERROR("invalid fd");
        return -1;
    }

    if (!_recvBuffer)
    {
        THIRD_ERROR("_recvBuffer is null, fd=%d", getFd());
        Close();
        return -1;
    }

    bool bClose = false;
    while (true)
    {
        int nRecv = recv(getFd(), &(_buffer[0]), RECV_MAXLEN, 0);
        if (nRecv < 0)
        {
            if (errno == EINTR || errno == EAGAIN ||  errno == EWOULDBLOCK)
            {
                THIRD_DEBUG("Not data, fd=%d", getFd());
                break;
            }

            THIRD_ERROR("Failed to receive data, fd=%d, err=%s", getFd(), strerror(errno));
            bClose = true;
            break;
        }

        if (nRecv == 0)
        {
            THIRD_ERROR("Server closes connection, fd=%d, err=%s", getFd(), strerror(errno));
            bClose = true;
            break;
        }

        if (-1 == _recvBuffer->append(&(_buffer[0]), nRecv))
        {
            THIRD_ERROR("_recvBuffer->append() fail, fd=%d, nRecv=", getFd(), nRecv);
            bClose = true;
            break;
        }

        if (RECV_MAXLEN != nRecv)
        {
            // THIRD_DEBUG("The data has been received completely");
            break;
        }
    }

    if (bClose)
    {
        Close();
        return -1;
    }

    return 0;
}

int CTcpClient::Send(const char *vptr, const int32_t len)
{
    if (-1 == getFd())
    {
        THIRD_ERROR("invalid fd");
        return -1;
    }

    if (!_sendBuffer)
    {
        THIRD_ERROR("_sendBuffer is null, fd=%d", getFd());
        return -1;
    }

    if (-1 == _sendBuffer->append(vptr, len))
    {
        THIRD_ERROR("_sendBuffer->append() fail, fd=%d, len[%d]", getFd(), len);
        return -1;
    }

    return Send();
}

int CTcpClient::Send()
{
    if (-1 == getFd())
    {
        THIRD_ERROR("invalid fd");
        return -1;
    }

    if (!_sendBuffer)
    {
        THIRD_ERROR("_sendBuffer is null, fd=%d", getFd());
        return -1;
    }

    ssize_t nSent = 0;
    while (true)
    {
        if (_sendBuffer->size() <= 0)
            break;

        ssize_t nRet = send(getFd(), _sendBuffer->data(), _sendBuffer->size(), 0);
        if (-1 == nRet)
        {
            THIRD_ERROR("send() fail, fd=%d, err=%s, len=%d", getFd(), strerror(errno), _sendBuffer->size());
            break;
        }

        if (0 == nRet)
        {
            THIRD_DEBUG("send data zero, fd=%d, err=%s", getFd(), strerror(errno));
            break;
        }

        _sendBuffer->recycle(nRet);
        nSent += nRet;
    }

    if (_sendBuffer->size() > 0)
        enableOutput();
    else
        disableOutput();

    // FDLOG_AI_INFO << "_sendBuffer->size(): len:" << _sendBuffer->size() << ", nSent:" << nSent << endl; ;
    return nSent;
}

int CTcpClient::Error()
{
    THIRD_ERROR("socket error: err=%s", strerror(errno));
    return 0;
}

int CTcpClient::Dispatch()
{
    if (!_recvBuffer)
    {
        THIRD_ERROR("invalid _recvBuffer");
        return -1;
    }

    bool bClose = false;
    const uint32_t nHeadLen = HEAD_LEN;
    while (_recvBuffer->size() > 0)
    {
        uint32_t nLen = _recvBuffer->size();
        if (nLen < nHeadLen)
            break;

        MsgHead header;
        memcpy(&header, _recvBuffer->data(), HEAD_LEN);
        header.ToHostByteOrder();
        if (nLen < header.iLen)
            break;

        uint32_t nBody = header.iLen - nHeadLen;
        char *pBody = _recvBuffer->data(nHeadLen);
        if (pBody && nBody >= 0)
            dispatch(header, pBody, nBody);

        _recvBuffer->recycle(header.iLen);
    }

    if (bClose)
    {
        Close();
        THIRD_DEBUG("Active disconnection, fd=%d", getFd());
    }

    return 0;
}

void CTcpClient::Close()
{
    delEpoller();

    if (-1 != getFd())
    {
        close(getFd());
        setFd(-1);
    }

    _recvBuffer->reset();
    _sendBuffer->reset();

    _connected = false;
    setLastAccessTime(0);
}

int32_t CTcpClient::SetNonblocking(const int32_t fd)
{
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0)
    {
        THIRD_ERROR("fcntl(fd, GETFL) fail");
        return -1;
    }

    opts = opts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0)
    {
        THIRD_ERROR("fcntl(fd, SETFL, opts) fail");
        return -1;
    }

    return 0;
}

int CTcpClient::AddEpoller(int32_t fd)
{
    _recvBuffer->reset();
    _sendBuffer->reset();

    setFd(fd);

    return addEpoller();
}

void CTcpClient::dispatch(const MsgHead &header, const char *ptr, const int32_t len)
{
    uint32_t iRet = 0;
    if (header.iErr != 0)
    {
        THIRD_ERROR("errro response, cmd=%u, errno=%u", header.iCmd, header.iErr);
        return;
    }

    switch (header.iCmd)
    {
    case CMD_BEAT_HEART:
        iRet = OnNodeKeepAlive(header, ptr, len);
        break;
    case CMD_CREATE_ROOM_RSP:
        iRet = OnCreateRoom(header, ptr, len);
        break;
    case CMD_JOIN_ROOM_RSP:
        iRet = OnJoinRoom(header, ptr, len);
        break;
    case CMD_LEAVE_ROOM_RSP:
        iRet = OnLeaveRoom(header, ptr, len);
        break;
    case CMD_DESTROY_ROOM_RSP:
        iRet = OnDestroyRoom(header, ptr, len);
        break;
    default:
        iRet = -1;
        break;
    }

    if (iRet != 0)
    {
        THIRD_ERROR("Command processing fail, cmd=%d, ret=%d", header.iCmd, iRet);
    }
    else
    {
        //THIRD_DEBUG("Command processing succ, cmd=%d, ret=%d", header.iCmd, iRet);
    }
}

int CTcpClient::NodeLoginMsg()
{
    //通知游戏同步房间数据
    THIRD_DEBUG("NodeLoginMsg..");
    CAIMngSingleton::getInstance()->sendMsgToGame(Pb::CMD_ROOM_INFO_NOTIFY, "");
    return 0;
}

int CTcpClient::NodeKeepAliveMsg()
{
    std::string sBody;
    BeatHeart req;
    req.set_name("keep alive test");
    
    if (!req.SerializeToString(&sBody))
    {
        THIRD_ERROR("SerializeToString(BeatHeart) failed");
        return -1;
    }

    MsgHead head(HEAD_LEN + sBody.size(), CMD_BEAT_HEART, 0, 0, seqno());
    head.ToNetByteOrder();
    if (PACK_MAXLEN < (HEAD_LEN + sBody.size()))
    {
        THIRD_ERROR("Maximum package cap(BeatHeart)");
        return -1;
    }

    uint32_t iSendLen = 0;
    memcpy(&_packet[iSendLen], &head, HEAD_LEN);
    iSendLen += HEAD_LEN;
    memcpy(&_packet[iSendLen], sBody.data(), sBody.size());
    iSendLen += sBody.size();
    if (-1 == Send(&(_packet[0]), iSendLen))
    {
        THIRD_ERROR("Out of free space(BeatHeart), _connid[%lu], iSendLen[%d]", _connid, iSendLen);
        return -1;
    }

    if(_textFormatPrint)
    {
        THIRD_DEBUG("NodeKeepAliveMsg() succ, _connid[%lu], _roomid[%s]", _connid, _roomid.c_str());
    }

    return 0;
}

int CTcpClient::TransmitToPlatform(const long uid, const int type, const string &msg)
{
    THIRD_DEBUG("Msg transmit to platform, uid[%ld], type[%d], msgLen[%ld]", uid, type, (long)msg.size());

    if (_textFormatPrint)
    {
        switch (type)
        {

        case CMD_CREATE_ROOM_REQ:
        {
            Pb::CreateRoomReq rsp;
            rsp.ParseFromString(msg);
            std::string json;
            google::protobuf::TextFormat::PrintToString(rsp, &json);
            FDLOG_AI_INFO << "type: " << CMD_CREATE_ROOM_REQ << ", info:" << json.c_str() << endl;
            THIRD_DEBUG("----PrintToString(CreateRoomReq): %s", json.c_str());
            break;
        }
        case CMD_JOIN_ROOM_REQ:
        {
            Pb::JoinRoomReq rsp;
            rsp.ParseFromString(msg);
            std::string json;
            google::protobuf::TextFormat::PrintToString(rsp, &json);
            FDLOG_AI_INFO << "type: " << CMD_JOIN_ROOM_REQ << ", info:" << json.c_str() << endl;
            THIRD_DEBUG("----PrintToString(JoinRoomReq): %s", json.c_str());
            break;
        }
        case CMD_LEAVE_ROOM_REQ:
        {
            Pb::LeaveRoomReq rsp;
            rsp.ParseFromString(msg);
            std::string json;
            google::protobuf::TextFormat::PrintToString(rsp, &json);
            FDLOG_AI_INFO << "type: " << CMD_LEAVE_ROOM_REQ << ", info:" << json.c_str() << endl;
            THIRD_DEBUG("----PrintToString(LeaveRoomReq): %s", json.c_str());
            break;
        }
        case CMD_DESTROY_ROOM_REQ:
        {
            Pb::DestroyRoomReq rsp;
            rsp.ParseFromString(msg);
            std::string json;
            google::protobuf::TextFormat::PrintToString(rsp, &json);
            FDLOG_AI_INFO << "type: " << CMD_DESTROY_ROOM_REQ << ", info:" << json.c_str() << endl;
            THIRD_DEBUG("----PrintToString(DestroyRoomReq): %s", json.c_str());
            break;
        }
        case CMD_ROOM_INFO_NOTIFY:
        {
            Pb::RoomInfoNotify notify;
            notify.ParseFromString(msg);
            std::string json;
            google::protobuf::TextFormat::PrintToString(notify, &json);
            FDLOG_AI_INFO << "type: " << CMD_ROOM_INFO_NOTIFY << ", info:" << json.c_str() << endl;
            THIRD_DEBUG("----PrintToString(RoomInfoNotify): %s", json.c_str());

            //for test
            {
                THIRD_DEBUG("----PrintToString(RoomInfoNotify): cur_page:%d, total_page: %d ", notify.icurpage(), notify.itotalpage());
                for(auto info : notify.maproominfo())
                {
                    for(auto uid : info.second.userid())
                    {
                        THIRD_DEBUG("----PrintToString(RoomInfoNotify): room:%s, uid: %s ", info.first.c_str(), uid.c_str());
                    }
                }
            }
            break;
        }
        //未知消息
        default:
        {
            THIRD_DEBUG("PrintToString(unknow): type: %d", type);
            break;
        }
        }
    }

    MsgHead head(HEAD_LEN + msg.size(), type, uid, 0, seqno());
    head.ToNetByteOrder();
    if (PACK_MAXLEN < (HEAD_LEN + msg.size()))
    {
        THIRD_ERROR("Maximum package cap(TransmitToPlatform)");
        return -1;
    }

    uint32_t iSendLen = 0;
    memcpy(&_packet[iSendLen], &head, HEAD_LEN);
    iSendLen += HEAD_LEN;
    memcpy(&_packet[iSendLen], msg.data(), msg.size());
    iSendLen += msg.size();
    if (-1 == Send(&(_packet[0]), iSendLen))
    {
        THIRD_ERROR("Out of free space(ThirdKeepAliveReq), _connid[%lu], iSendLen[%d]", _connid, iSendLen);
        return -1;
    }

    //THIRD_DEBUG("TransmitToPlatform() succ, uid[%ld], type[%d], msgLen[%ld]", uid, type, (long)msg.size());
    return  0;
}

int CTcpClient::OnNodeKeepAlive(const MsgHead &header, const char *ptr, const int32_t len)
{
    Pb::BeatHeart rsp;
    if (!rsp.ParseFromString(std::string(ptr, len)))
    {
        THIRD_ERROR("ParseFromString(ThirdKeepAliveRsp) failed");
        return -1;
    }

    if (_textFormatPrint)
    {
        std::string json;
        google::protobuf::TextFormat::PrintToString(rsp, &json);
        THIRD_DEBUG("PrintToString(BeatHeart): %s", json.c_str());
    }

    //THIRD_DEBUG("Dispose(BeatHeart) succ");
    return 0;
}

int CTcpClient::OnCreateRoom(const MsgHead &header, const char *ptr, const int32_t len)
{
    Pb::CreateRoomRsp rsp;
    if (!rsp.ParseFromString(std::string(ptr, len)))
    {
        THIRD_ERROR("ParseFromString(CreateRoomRsp) failed");
        return -1;
    }

    if (_textFormatPrint)
    {
        std::string json;
        google::protobuf::TextFormat::PrintToString(rsp, &json);
        THIRD_DEBUG("PrintToString(CreateRoomRsp): %s", json.c_str());
    }

    std::string smsg;
    rsp.SerializeToString(&smsg);
    CAIMngSingleton::getInstance()->sendMsgToGame(header.iCmd, smsg);
    //THIRD_DEBUG("Dispose(CreateRoomRsp) succ");
    return 0;
}

int CTcpClient::OnJoinRoom(const MsgHead &header, const char *ptr, const int32_t len)
{
    Pb::JoinRoomRsp rsp;
    if (!rsp.ParseFromString(std::string(ptr, len)))
    {
        THIRD_ERROR("ParseFromString(OnJoinRoom) failed");
        return -1;
    }

    if (_textFormatPrint)
    {
        std::string json;
        google::protobuf::TextFormat::PrintToString(rsp, &json);
        THIRD_DEBUG("PrintToString(JoinRoomRsp): %s", json.c_str());
    }

    std::string smsg;
    rsp.SerializeToString(&smsg);
    CAIMngSingleton::getInstance()->sendMsgToGame(header.iCmd, smsg);
    //THIRD_DEBUG("Dispose(JoinRoomRsp) succ");
    return 0;
}


int CTcpClient::OnLeaveRoom(const MsgHead &header, const char *ptr, const int32_t len)
{
    Pb::LeaveRoomRsp rsp;
    if (!rsp.ParseFromString(std::string(ptr, len)))
    {
        THIRD_ERROR("ParseFromString(LeaveRoomRsp) failed");
        return -1;
    }

    if (_textFormatPrint)
    {
        std::string json;
        google::protobuf::TextFormat::PrintToString(rsp, &json);
        THIRD_DEBUG("PrintToString(LeaveRoomRsp): %s", json.c_str());
    }

    std::string smsg;
    rsp.SerializeToString(&smsg);
    CAIMngSingleton::getInstance()->sendMsgToGame(header.iCmd, smsg);
    //THIRD_DEBUG("Dispose(LeaveRoomRsp) succ");
    return 0;
}

int CTcpClient::OnDestroyRoom(const MsgHead &header, const char *ptr, const int32_t len)
{
    Pb::DestroyRoomReq rsp;
    if (!rsp.ParseFromString(std::string(ptr, len)))
    {
        THIRD_ERROR("ParseFromString(DestroyRoomReq) failed");
        return -1;
    }

    if (_textFormatPrint)
    {
        std::string json;
        google::protobuf::TextFormat::PrintToString(rsp, &json);
        THIRD_DEBUG("PrintToString(DestroyRoomReq): %s", json.c_str());
    }

    std::string smsg;
    rsp.SerializeToString(&smsg);
    CAIMngSingleton::getInstance()->sendMsgToGame(header.iCmd, smsg);
    //THIRD_DEBUG("Dispose(DestroyRoomReq) succ");
    return 0;
}

