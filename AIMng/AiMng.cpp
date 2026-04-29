#include "AiMng.h"
#include "RoomServer.h"
#include "AsyncUserInfoCallback.h"

#define DEBUG_FALG  0
#define ROOM_LIST "1000|1000"
/**
 *
*/
CAiMng::CAiMng()
{
	initialize();
}

/**
 *
*/
CAiMng::~CAiMng()
{

}

void CAiMng::initialize()
{
	//OuterFactorySingleton::getInstance()->getConfigServantPrx()->ListRoomRobot(m_robots);
	return;
}

/**
 * 接口连接池
 */
CThirdPartyManager *CAiMng::getThirdMgr()
{
    return thirdMgr;
}

void CAiMng::closeThirdPartyMng()
{
	if(thirdMgr)
	{
		thirdMgr->final();
	}
}

void CAiMng::loopThirdPartyMng()
{
    if(thirdMgr)
    {
        thirdMgr->loop();
        thirdMgr->check();
    }
}

void CAiMng::checkThirdPartyMng()
{
    if(thirdMgr)
    {
        thirdMgr->check();
    }
}

void CAiMng::initTableMng(CTableMng *p, const string &roomid)
{
    string room_list_cfg = ROOM_LIST;
    if(room_list_cfg.find(roomid) != std::string::npos && g_app.getOuterFactoryPtr()->getAiPlatformOpen() == 1)
    {
        _pTableMng = p;

        //初始化平台连接池
        thirdMgr = new CThirdPartyManager();
        ROLLLOG_DEBUG << "getThirdMgr create... " << endl;
        //创建AI连接池
        getThirdMgr()->init(roomid);
        ROLLLOG_DEBUG << "getThirdMgr init... " << endl;
        //测试连接池
        getThirdMgr()->testing(roomid);
        ROLLLOG_DEBUG << "getThirdMgr test... " << endl;
    }
    else
    {
        ROLLLOG_ERROR << "room_list_cfg: "<< room_list_cfg << ", roomid: "<< roomid << endl;
    }
}

void CAiMng::sendMsgToPlatform(Pb::TMsgTransmitReq& req)
{
    if(getThirdMgr())
    {
        getThirdMgr()->getAiProxy(0)->TransmitToPlatform(req.uid(), req.type(), req.msg());
    }
}

void CAiMng::sendMsgToGame(int cmd, const string& msg)
{
   if (_pTableMng)
    {
        _pTableMng->onPlatformMsg(cmd, msg);
    }
    else
    {
        LOG_DEBUG << "sendMsgToGame err.  _pTableMng is nullptr " << endl;
    }
}
