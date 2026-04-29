#ifndef  __AI_MNG_H__
#define  __AI_MNG_H__

#include "servant/Application.h"
#include "RoomServant.h"
#include "../OuterFactoryImp.h"
#include "MonteCarlo/simulator.h"
#include "XGameComm.pb.h"
#include "third.pb.h"
#include "PlayerMng.h"
#include "Player.h"
#include <vector>
#include <map>
#include <set>

#include "ThirdParty/ThirdPartyManager.h"

/**
 *  机器人管理
 */
class CAiMng
{
public:
    /**
     *
    */
    CAiMng();

    /**
     *
    */
    ~CAiMng();

public:
    void initialize();
    void initTableMng(CTableMng *p, const string &roomid);
    CThirdPartyManager *getThirdMgr();
    void closeThirdPartyMng();
    void loopThirdPartyMng();
    void checkThirdPartyMng();
    void sendMsgToPlatform(Pb::TMsgTransmitReq& req);
    void sendMsgToGame(int cmd, const string& msg);

private:
    std::map<string, std::vector<tars::Int64>> m_robots;
     /**
     * 算法模拟器
     */
    Simulator *simulator;
    /**
     * 接口连接池
     */
    CThirdPartyManager *thirdMgr;

    CTableMng *_pTableMng;
};

////
typedef TC_Singleton<CAiMng, CreateStatic, DefaultLifetime> CAIMngSingleton;

#endif
