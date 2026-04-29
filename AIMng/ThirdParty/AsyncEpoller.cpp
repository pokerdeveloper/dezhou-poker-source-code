#include "AsyncEpoller.h"
#include "AsyncSocket.h"
#include "ThirdLog.h"
#include "TcpClient.h"
#include "LogComm.h"

static bool setNonblocking(int32_t fd)
{
    int32_t opts = fcntl(fd, F_GETFL);
    if (opts < 0)
    {
        THIRD_ERROR("fcntl(fd,GETFL)");
        return false;
    }

    opts = opts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0)
    {
        THIRD_ERROR("fcntl(fd,SETFL,opts)");
        return false;
    }

    return true;
}

CAsyncEpoller::CAsyncEpoller(const string &roomid)
{
    _epfd = 0;
    _listenfd = -1;
    _roomid = roomid;
}

CAsyncEpoller::~CAsyncEpoller()
{
    _connMap.clear();
}

bool CAsyncEpoller::init(int32_t nMaxClients)
{
    _epfd = epoll_create(nMaxClients + 1);
    if (-1 == _epfd)
    {
        THIRD_ERROR("epoll_create(%d) fail, err=%s", nMaxClients, strerror(errno));
        return false;
    }

    THIRD_DEBUG("Maximum number of administrative connections: %d", nMaxClients);
    return true;
}

bool CAsyncEpoller::loop()
{
    struct epoll_event events[128];
    int32_t nfds = epoll_wait(_epfd, events, 128, 1);
    for (int32_t i = 0; i < nfds; i++)
    {
        if ((_listenfd > 0) && (events[i].data.fd == _listenfd))
        {
            socklen_t cliLen;
            struct sockaddr_in cliAddr;
            int32_t connFd = accept(_listenfd, (struct sockaddr *)&cliAddr, &cliLen);
            if (connFd > 0)
            {
                if (!setNonblocking(connFd))
                {
                    THIRD_ERROR("setNonblocking() fail, connFd=%d", connFd);
                    close(connFd);
                    continue;
                }

                auto client = new CTcpClient(this, _roomid);
                if (!client)
                {
                    THIRD_ERROR("Create CTcpClient object fail, connFd=%d", connFd);
                    close(connFd);
                    continue;
                }

                client->AddEpoller(connFd);
                THIRD_DEBUG("connect from %s", inet_ntoa(cliAddr.sin_addr));
            }
        }
        else if (events[i].events & EPOLLIN)
        {
            static_cast<CAsyncSocket *>(events[i].data.ptr)->intputNotify();
        }
        else if (events[i].events & EPOLLOUT)
        {
            static_cast<CAsyncSocket *>(events[i].data.ptr)->outputNotify();
        }
        else if (events[i].events & EPOLLERR)
        {
            static_cast<CAsyncSocket *>(events[i].data.ptr)->errorNotify();
        }
    }

    checkConnections();
    return true;
}

bool CAsyncEpoller::final()
{
    if (_epfd > 0)
    {
        close(_epfd);
        _epfd = -1;
    }

    return true;
}

int32_t CAsyncEpoller::getEpfd()
{
    return _epfd;
}

bool CAsyncEpoller::addEpoller(CAsyncSocket *as)
{
    if (!as)
    {
        THIRD_ERROR("Invalid params(1)");
        return false;
    }

    if (-1 == as->getFd())
    {
        THIRD_ERROR("Invalid params(2)");
        return false;
    }

    if (-1 == epoll_ctl(_epfd, EPOLL_CTL_ADD, as->getFd(), as->getEvents()))
    {
        THIRD_ERROR("Add events: fd=%d, error=%s", as->getFd(), strerror(errno));
        return false;
    }

    _connMap.insert(std::pair<int32_t, CAsyncSocket *>(as->getFd(), as));
    THIRD_DEBUG("Current number of objects<ADD>: %d", _connMap.size());
    return true;
}

bool CAsyncEpoller::delEpoller(CAsyncSocket *as)
{
    if (!as)
    {
        THIRD_ERROR("Invalid params(1)");
        return false;
    }

    if (-1 == as->getFd())
    {
        THIRD_ERROR("Invalid params(2)");
        return false;
    }

    if (-1 == epoll_ctl(_epfd, EPOLL_CTL_DEL, as->getFd(), as->getEvents()))
    {
        THIRD_ERROR("Del events: fd=%d, error=%s", as->getFd(), strerror(errno));
        return false;
    }

    auto iter = _connMap.find(as->getFd());
    if (iter != _connMap.end())
        _connMap.erase(iter);

    THIRD_DEBUG("Current number of objects<DEL>: %d", _connMap.size());
    return true;
}

bool CAsyncEpoller::openListen(const int16_t port)
{
    _listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == _listenfd)
    {
        THIRD_ERROR("socket() fail, err=%s", strerror(errno));
        return false;
    }

    if (setNonblocking(_listenfd))
    {
        close(_listenfd);
        _listenfd = -1;
        return false;
    }

    int flag = 1;
    if (-1 == setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)))
    {
        THIRD_ERROR("setsockopt(SO_REUSEADDR) fail, fd=%d, err=%s", _listenfd, strerror(errno));
        close(_listenfd);
        _listenfd = -1;
        return false;
    }

    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);
    if (-1 == bind(_listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)))
    {
        THIRD_ERROR("bind(%d) fail, err=%s", _listenfd, strerror(errno));
        close(_listenfd);
        _listenfd = -1;
        return false;
    }

    int32_t backlog = 128;
    if (-1 == listen(_listenfd, backlog))
    {
        THIRD_ERROR("listen(%d) fail, err=%s", _listenfd, strerror(errno));
        close(_listenfd);
        _listenfd = -1;
        return false;
    }

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(ev));
    ev.data.fd = _listenfd;
    ev.events = EPOLLIN | EPOLLET | EPOLLERR;
    if (-1 == epoll_ctl(_epfd, EPOLL_CTL_ADD, _listenfd, &ev))
    {
        THIRD_ERROR("epoll_ctl(%d) fail, err=%s", _listenfd, strerror(errno));
        close(_listenfd);
        _listenfd = -1;
        return false;
    }

    THIRD_DEBUG("Local listening port: %d", port);
    return true;
}

bool CAsyncEpoller::checkConnections()
{
    //Check every 10 seconds
    static uint32_t counter = 0;
    static uint32_t maximum = 10;
    if (0 == ((++counter) % maximum))
    {
        for (auto iter = _connMap.begin(); iter != _connMap.end(); iter++)
        {
            auto pAsyncSocket = (*iter).second;
            if (pAsyncSocket)
            {
                pAsyncSocket->connectChecker();
            }
        }
    }

    return true;
}
