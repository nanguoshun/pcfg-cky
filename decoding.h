//
// Created by  ngs on 07/07/2018.
//

#ifndef PCFG_DECODING_H
#define PCFG_DECODING_H

#include "common.h"
#include "binarytree.h"
#include "node.h"
class Decoder{
public:
    Decoder();
    ~Decoder();
    void ExtractSentenceFile(const char *file_name);
    void SaveSentenceFile(std::string str,std::ofstream *ofs);
    std::string IterationExtract(Node *ptr_root, std::ofstream *ofs);
    void Decoding(const char *file_name);
    void ReadModel(std::string model_file);
    void GenerateNonTerminatorMap();
    void InitCKY(std::vector<std::string> *ptr_x_vector);
    void CKY(std::vector<std::string> *ptr_x_vector);
    void GetRuleWeight(std::string &x_str, Rule_Weight_Vector &rule_vector);
    double CalcMaxRule(int i, int j, const std::string &x_str,const Rule_Weight_Vector &rule_vector,std::vector<std::string> *ptr_x_vector);
private:
    Rule_Weight_Map *ptr_rule_weight_map_;
    std::unordered_map<std::string, int> *ptr_non_terminator_map_;
    CKY_Score_Map *ptr_cky_score_map_;
    std::vector<BinaryTree *> tree_vector_;
};


#endif //PCFG_DECODING_H
