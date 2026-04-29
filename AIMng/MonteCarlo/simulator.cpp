#include "simulator.h"
#include "tools.h"
#include "samples.h"
#include "cards.h"
#include "RoomServer.h"
#include "LogComm.h"

using namespace std;

//application
extern RoomServer g_app;

map<int, string> mSuitTran =
{
    { 0, "d"},
    { 16, "c"},
    { 32, "h"},
    { 48, "s"},
};

std::vector<short> vCards =
{
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62
};

Simulator::Simulator()
{
    init();
}

Simulator::~Simulator()
{

}

int Simulator::init()
{
    //
    cTable = gen_combo_table(52, 5);
    ROLLLOG_DEBUG<<"cTable size:"<< cTable.size() << endl;
    //
    replace =
    {
        0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 0, 0, 4,
        6, 3, 5, 2, 4, 1, 3, 1, 1, 2, 3, 3, 4,
        4, 5, 2, 2, 4, 6, 3, 5, 3, 3, 4, 5, 4, 4
    };
    //
    std::string algPath = g_app.getOuterFactoryPtr()->getAlgConfigPath();
    std::string algFile = algPath + "lookup_tablev3.bin";
    table = read_vect(algFile.c_str());
    if (table.empty())
    {
        return -1;
    }

    return 0;
}

int Simulator::to_ckey(const vector<int> &hand)
{
    int key = 0;
    for (uint32_t i = 0; i < hand.size(); ++i)
    {
        key += cTable[(i + 1) * 53 + hand[i]];
    }

    return key;
}

vector<int> Simulator::get_remaining(vector<int> &comm_hand, vector<vector<int>> &known_hands)
{
    vector<int> remaining;
    vector<int> filled(52);

    for (int card : comm_hand)
    {
        card = card <= 0 ? 1 : card;
        filled[card - 1] = 1;
    }

    for (vector<int> known_hand : known_hands)
    {
        for (int card : known_hand)
        {
            card = card <= 0 ? 1 : card;
            filled[card - 1] = 1;
        }
    }

    for (int i = 0; i < 52; ++i)
    {
        if (!filled[i])
        {
            remaining.push_back(i);
        }
    }

    return remaining;
}

vector<vector<int>> Simulator::fill_empty(int N, vector<int> &comm_hand, vector<vector<int>> &known_hands, int players_unknown)
{
    vector<int> remaining = get_remaining(comm_hand, known_hands);
    int c = players_unknown * 2 + 5 - comm_hand.size();
    vector<vector<int>> samples = gen_samples(N, c, remaining.size());
    for (vector<int> &sample : samples)
    {
        for (int i = 0; i < c; ++i)
        {
            sample[i] = remaining[(unsigned int)sample[i] > remaining.size() - 1 ?  remaining.size() - 1 : sample[i]];
        }
    }

    return samples;
}

int Simulator::evaluate_selection(vector<int> selection)
{
    sort(selection.begin(), selection.end());

    vector<int> hand(5);
    for (int i = 0; i < 5; ++i)
    {
        hand[i] = selection[i + 2];
    }

    int key = (unsigned int)to_ckey(hand) > table.size() - 1 ? table.size() - 1 : to_ckey(hand);
    if(key < 0 || table.size() < 1) return 0;
    int max_score = table[key];
    for (int j = 0; j < 40; j += 2)
    {
        int ix_k = (replace[j] + 1) * 53;
        key += cTable[ix_k + selection[replace[j + 1]]] - cTable[ix_k + hand[replace[j]]];
        hand[replace[j]] = selection[replace[j + 1]];
        key = (key < 0 || key > (int)table.size() - 1) ? (int)table.size() - 1 : key;
        if(key < 0) return max_score;
        max_score = max(max_score, table[key]);
    }

    return max_score;
}

void Simulator::update_winners(int my_val, int &max_val, int ix, vector<int> &winners)
{
    if (my_val > max_val)
    {
        winners = {ix};
        max_val = my_val;
    }
    else if (my_val == max_val)
    {
        winners.push_back(ix);
    }
}

vector<int> Simulator::simulate(vector<int> &selection, vector<vector<int>> &known_hands, vector<int> &sample, int start)
{
    vector<int> winners;
    int max_val = 0;

    for (uint32_t i = 0; i < known_hands.size(); i++)
    {
        selection[5] = known_hands[i][0];
        selection[6] = known_hands[i][1];
        update_winners(evaluate_selection(selection), max_val, i, winners);
    }

    // int opp_max_val = 0;
    // int opp_max = 0;
    for (uint32_t i = 0; start + i * 2 < sample.size(); i ++)
    {
        selection[5] = sample[start + i * 2];
        selection[6] = sample[start + 1 + i * 2];
        update_winners(evaluate_selection(selection), max_val, known_hands.size() + i, winners);
    }

    return winners;
}

vector<vector<int>> Simulator::calculate(int N, vector<int>comm_hand, vector<vector<int>>known_hands, int players_unknown)
{
    vector<vector<int>> samples = fill_empty(N, comm_hand, known_hands, players_unknown);
    vector<int> selection(7);
    vector<vector<int>> results(known_hands.size() + players_unknown, vector<int>(2, 0));

    for (uint32_t i = 0; i < comm_hand.size(); ++i)
    {
        selection[i] = comm_hand[i];
    }

    int s_comm = 5 - (int)comm_hand.size();
    int cnt = 0;
    for (vector<int> sample : samples)
    {
        cnt++;

        for (int i = 0; i < s_comm; ++i)
        {
            selection[4 - i] = sample[i];
        }

        vector<int> winners = simulate(selection, known_hands, sample, s_comm);
        if (winners.size() == 1)
        {
            results[winners[0]][0]++;
        }
        else
        {
            for (int winner : winners)
            {
                results[winner][1]++;
            }
        }
    }

    return results;
}

void Simulator::print_results(int N, vector<vector<int>> hands, vector<vector<int>> results, std::vector<float> &vWins)
{
    for (uint32_t i = 0; i < hands.size(); i++)
    {
        //print_hand(hands[i]);
        format_result(N, results[i], vWins );
    }

    ROLLLOG_DEBUG << "results size:" << results.size() << ", hands size:" << hands.size() << endl;

    int unknown = results.size() - hands.size();
    if (unknown > 0)
    {
        float wins = 0;
        float factor = 100.0 / N / unknown;

        for (uint32_t i = hands.size(); i < results.size(); i++)
        {
            wins += results[i][0];
            ROLLLOG_DEBUG << " win:" << wins << endl;
        }

        vWins.push_back( wins * factor);
    }

    return ;
}

void Simulator::format_result(int N, vector<int> result, std::vector<float> &vWins)
{
    ROLLLOG_DEBUG << result[0] * 100.0 / N << "\t" << result[1] * 100.0 / N << endl;
    vWins.push_back(result[0] * 100.0 / N);
}

void Simulator::compute_probabilities(int N, vector<string> comm_hand_str, vector<vector<string>> known_hands_str, int players_unknown, std::vector<float> &vWins)
{
    vector<int> comm_hand = convert_hand(comm_hand_str);
    vector<vector<int>> known_hands;

    for (vector<string> known_hand_str : known_hands_str)
    {
        known_hands.push_back(convert_hand(known_hand_str));
    }

    vector<vector<int>> results = calculate(N, comm_hand, known_hands, players_unknown);

    print_results(N, known_hands, results, vWins);

    return;
}

/**
 * 用例
 */

string Simulator::tranCard(short card)
{
    int suit = card & 0x00f0;
    int face = card & 0x000f;
    auto it = mSuitTran.find(suit);
    if(it == mSuitTran.end())
    {
        return "";
    }

    ostringstream os;
    if(face == 14)
    {
        os << "A";
    }
    else if(face == 13)
    {
        os << "K";
    }
    else if(face == 12)
    {
        os << "Q";
    }
    else if(face == 11)
    {
        os << "J";
    }
    else
    {
        os << face;
    }
    os << it->second;
    return os.str();
}

/*int Simulator::getWRate(const RoomSo::TRobotDecideReq &req, RoomSo::TRobotDecideRsp &rsp)
{
    __TRY__
    vector<string> commHand;
    ostringstream os;
    for(auto card : req.comCards)
    {
        string trancard = tranCard(card);
        if(trancard.size() != 2 && trancard.size() != 3)
        {
            ROLLLOG_ERROR << "card tran err. card:" << card << ", trancard:" << trancard << endl;
            return -1;
        }
        os << trancard << "|";
        commHand.push_back(trancard);
    }

    if(commHand.size() > 5 )
    {
        ROLLLOG_ERROR << "comm size:" << commHand.size() << ", allHd size:" << req.allHd.size() << ", req.robotHd:" <<  req.robotHd.size() << endl;
        return -1;
    }

    vector<vector<string>> knownHands;
    for(auto it : req.robotHd)
    {
        vector<string> vItem;
        for(auto card : it.second)
        {
            if(it.second.size() != 2)
            {
                continue;
            }
            string trancard = tranCard(card);
            vItem.push_back(trancard);
        }
        knownHands.push_back(vItem);
    }

    std::vector<float> vWins;
    int N = 10000;

    for(auto item : knownHands)
    {
        vector<vector<string>> oneKnownHands;
        oneKnownHands.push_back(item);
        std::vector<float> vTWins;
        compute_probabilities(N, commHand, oneKnownHands, 1, vTWins);
        if(vTWins.size() > 0)
        {
            vWins.push_back(*vTWins.begin());
        }
        else
        {
            vWins.push_back(0);
        }
    }

    ROLLLOG_DEBUG << "----------------" << endl;
    ROLLLOG_DEBUG << "robotHd size:" << req.robotHd.size() << ", vWins:" << vWins.size() << endl;
    ROLLLOG_DEBUG << "comCards: " << os.str() << endl;

    int index = 0;
    for(auto it : req.robotHd)
    {
        if(index > (int)vWins.size() - 1)
        {
            rsp.robotRate[it.first] = 0;
        }
        else
        {
            rsp.robotRate[it.first] = vWins[index];
        }
        index++;

        ROLLLOG_DEBUG << "uid:" << it.first << ", wrate:" << rsp.robotRate[it.first] << ", hdCard:" << tranCard(it.second[0]) << "|" << tranCard(it.second[1]) << endl;
    }
    __CATCH__
    return 0;
}*/
