//
// Created by  ngs on 16/07/2018.
//

#ifndef PCFG_PCFG_EM_H
#define PCFG_PCFG_EM_H

#include "common.h"
#include "nlpbase.h"

class PCFGEM: public NLPBase{
public:
    PCFGEM();
    ~PCFGEM();
    void Training(const char * file_name);
    void InitCFG(const std::string &rule_file, const std::string &non_terminator, const std::string&terminator);
    void InitRuleMap(std::ifstream &ifs, std::string &str);
    void InitPhraseLevelMap(const std::string &str);
    void InitIO();
    void InitAlphaBeta(std::vector<std::string> &x_vector);
    void RandomizeRuleWeight();
    void EM();
    void EMSentence(std::vector<std::string> &x_vector);
    double CalcRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule);
    double CalcPhraseLevelRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z);
    double CalcWordLevelRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z);
    double CalcZ(std::vector<std::string> &x_vector);
    double CalcAlpha(int i, int j, std::string rule_str);
    void CalcAlpha(std::vector<std::string> &x_vector);
    double GetAlpha(double weight, PCFG_Rule &binary_rule, int i, int j);
    double CalcBeta(std::vector<std::string> &x_vector, std::string &rule_str, int i, int j);
    Rule_Weight_Vector GetRulesAsRightChild(std::string &rule_str);
    Rule_Weight_Vector GetRulesAsLeftChild(std::string &rule_str);
    double GetValue(const IO_Map *ptr_map, const std::string &rule_str, const int i, const int j);
    //void CalcBeta(std::vector<std::string> &x_vector);
    //double CalcBeta(std::vector<std::string> &x_vector, int i, int j, std::string rule_str);
    //double GetBeta(std::vector<std::string> &x_vector,double weight, PCFG_Rule &binary_rule, int i, int j, bool k_left);
    void InitExpectCount();
    double GetIOValue(const IO_Map *ptr_map, const IO_Tuple &tuple);
    void Reset();
private:
    std::unordered_map<std::string, int> *ptr_non_terminal_map_;
    std::unordered_map<std::string, int> *ptr_terminal_map_;
    Rule_Map *ptr_rule_map_;
    const std::string start_symbol_ = ROOT_NODE;
    IO_Map *ptr_alpha_map_;
    IO_Map *ptr_beta_map_;

    Rule_Weight_Map *ptr_binary_rule_weight_map_; //like X->YZ, and YZ are non-terminators.
    Rule_Weight_Map *ptr_rule_weight_map_;
    Rule_Weight_Map *ptr_rule_expected_count_;
    Rule_Weight_Map *ptr_rule_expected_count_temp_;
    Element_Expected_Map *ptr_element_expected_count_;
    int sentence_len_;
};


#endif //PCFG_PCFG_EM_H
