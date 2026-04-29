#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <string>
#include "cards.h"
#include "tools.h"
#include <vector>

using namespace std;

class Simulator
{
public:
    Simulator();
    virtual ~Simulator();

    //
    int init();

private:
    //
    vector<int> get_remaining(vector<int> &comm_hand, vector<vector<int>> &known_hands);
    //
    vector<vector<int>> fill_empty(int N, vector<int> &comm_hand, vector<vector<int>> &known_hands, int players_unknown);
    //
    int evaluate_selection(vector<int> selection);
    //
    void update_winners(int my_val, int &max_val, int ix, vector<int> &winners);
    //
    void format_result(int N, vector<int> result, std::vector<float> &vWins);

public:
    //
    int to_ckey(const vector<int> &hand);
    //
    vector<int> simulate(vector<int> &selection, vector<vector<int>> &known_hands, vector<int> &sample, int start);
    //
    void compute_probabilities(int N, vector<string> comm_hand_str, vector<vector<string>> known_hands_str, int players_unknown, std::vector<float> &vWins);
    //
    void print_results(int N, vector<vector<int>> hands, vector<vector<int>> results, std::vector<float> &vWins);
    //
    vector<vector<int>> calculate(int N, vector<int>comm_hand, vector<vector<int>>known_hands, int players_unknown);
    //
    string tranCard(short card);
    //
    //int getWRate(const RoomSo::TRobotDecideReq &req, RoomSo::TRobotDecideRsp &rsp);

private:
    //
    vector<int> cTable;
    //
    vector<int> table;
    //
    vector<int> replace;
};

#endif
