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
    void InitCFG(const std::string &rule_file);
    void InitRuleMap(std::ifstream &ifs, std::string &str);
    void InitPhraseLevelMap(const std::string &str);
    void InitSymbolRuleVectorMap();
    void InitAlphaBeta(std::vector<std::string> &x_vector);
    void RandomizeRuleWeight();
    void EM();
    double EStep();
    void MStep();
    void SaveModel(std::string model_file);
    void SumExpectedCount();
    double CalcExpectedCount(std::vector<std::string> &x_vector);
    double CalcRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z);
    double CalcPhraseLevelRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z);
    double CalcWordLevelRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z);
    double CalcZ(std::vector<std::string> &x_vector);
    double CalcAlpha(int i, int j, std::string &rule_str,double weight);
    void CalcAlpha(std::vector<std::string> &x_vector);
    void CalcBeta(std::vector<std::string> &x_vector);
    double GetAlpha(int i, int j,PCFG_Rule &binary_rule, double weight);
    //double CalcBeta(std::vector<std::string> &x_vector, std::string &rule_str, int i, int j);
    std::vector<PCFG_Rule> * GetRuleVector(std::string &rule_str);
    Rule_Weight_Vector GetRulesAsRightChild(std::string &rule_str);
    Rule_Weight_Vector GetRulesAsLeftChild(std::string &rule_str);
    //Rule_Weight_Vector GetRulesAsRoot(std::string &root_str);
    double GetValue(const IO_Map *ptr_map, const std::string &rule_str, const int i, const int j);
    //void CalcBeta(std::vector<std::string> &x_vector);
    //double CalcBeta(std::vector<std::string> &x_vector, int i, int j, std::string rule_str);
    //double GetBeta(std::vector<std::string> &x_vector,double weight, PCFG_Rule &binary_rule, int i, int j, bool k_left);
    //double GetIOValue(const IO_Map *ptr_map, const IO_Tuple &tuple);
    void Reset();
    //void CalcSymbol_U(std::vector<std::string> &x_vector, std::string &str, int i, int j);
    double GetRuleWeight(PCFG_Rule &rule);
    double CalcDenominator(std::vector<PCFG_Rule> *ptr_rule_vector);
private:
    std::unordered_set<std::string> *ptr_non_terminal_set_;
    std::unordered_set<std::string> *ptr_terminal_set_;
    Rule_Map *ptr_rule_map_;
    IO_Map *ptr_alpha_map_;
    IO_Map *ptr_beta_map_;
    PCFG_Rule_Set *ptr_phrase_rule_set_; //like X->YZ, and YZ are non-terminators.
    Rule_Weight_Map *ptr_rule_weight_map_;
    Rule_Weight_Map *ptr_rule_expected_count_;
    Rule_Weight_Map *ptr_rule_expected_count_temp_;

    //Element_Expected_Map *ptr_element_expected_count_;

    IO_Map *ptr_u_;
    Binary_Rule_U_Map *ptr_bin_rule_u_;
    Symbol_Rule_Vector_Map *ptr_symbol_rule_vector_map_;
};


#endif //PCFG_PCFG_EM_H
