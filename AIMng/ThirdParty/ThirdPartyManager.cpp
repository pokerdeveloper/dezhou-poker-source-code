#include "ThirdPartyManager.h"
#include "RoomServer.h"

//application
extern RoomServer g_app;

CThirdPartyManager::CThirdPartyManager(): _asyncEpoller(NULL), _tcpClient(NULL)
{

}

CThirdPartyManager::~CThirdPartyManager()
{
    if (_tcpClient)
    {
        delete _tcpClient;
        _tcpClient = NULL;
    }

    if (_asyncEpoller)
    {
        delete _asyncEpoller;
        _asyncEpoller = NULL;
    }
}

bool CThirdPartyManager::init(const string &roomid)
{
    if (_asyncEpoller)
    {
        _asyncEpoller->final();
        delete _asyncEpoller;
        _asyncEpoller = NULL;
    }

    _asyncEpoller = new CAsyncEpoller(roomid);
    if (!_asyncEpoller)
    {
        LOG_ERROR << "_asyncEpoller is null." << endl;
        return false;
    }

    THIRD_DEBUG("CThirdPartyManager init..");

    return _asyncEpoller->init(1000);
}

bool CThirdPartyManager::loop()
{
    if (!_asyncEpoller)
    {
        LOG_ERROR << "_asyncEpoller is null." << endl;
        return false;
    }

    return _asyncEpoller->loop();
}

bool CThirdPartyManager::final()
{
    if (!_asyncEpoller)
    {
        LOG_ERROR << "_asyncEpoller is null." << endl;
        return false;
    }

    return _asyncEpoller->final();
}

bool CThirdPartyManager::check()
{
    if (!_tcpClient)
    {
        return false;
    }

    //Check every 10 seconds
    static uint32_t counter = 0;
    static uint32_t maximum = 100;
    if (0 == ((++counter) % maximum))
    {
        if(!_tcpClient->isConnected())
        {
            _tcpClient->connectChecker();
        }
    }

    return true;
}

void CThirdPartyManager::testing(const string &roomid)
{
    _tcpClient = new CTcpClient(_asyncEpoller, roomid);
    if (!_tcpClient)
    {
        LOG_ERROR << "_tcpClient is null" << endl;
        return;
    }

    THIRD_DEBUG("testing begin.");

    try{
        
        _tcpClient->setSvrAddr(g_app.getOuterFactoryPtr()->getAiPlatformAddr().c_str());
        _tcpClient->setSvrPort(g_app.getOuterFactoryPtr()->getAiPlatformPort());
        if (-1 == _tcpClient->Connect())
        {
            THIRD_ERROR("Connection to server failed, err=%s", strerror(errno));
            return;
        }
    }catch (exception &e)
    {
        THIRD_ERROR("Connection to server failed, err=%s", e.what());
    }

    THIRD_DEBUG("Connection to server succeed, err=%s", strerror(errno));
    return;
}

CTcpClient *CThirdPartyManager::getAiProxy(const uint32_t &tableId)
{
    if (_tcpClient)
    {
        _tcpClient->connectChecker();
    }

    // THIRD_DEBUG("Get the proxy object, tableId:%u", tableId);
    return _tcpClient;
}

