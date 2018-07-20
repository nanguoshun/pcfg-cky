//
// Created by  ngs on 16/07/2018.
//

#ifndef PCFG_NLPBASE_H
#define PCFG_NLPBASE_H

#include "common.h"
#include "node.h"
#include "binarytree.h"

class NLPBase{
public:
    NLPBase();
    ~NLPBase();
    std::string IterationExtract(Node *ptr_root, int row_index);
    void ExtractSentenceFile(const char *file_name);
    void GeneratePhraseLevelRuleMap(const std::string file_name);
protected:
    std::vector<std::vector<std::string>> *ptr_str_matrix_;
    int instance_num_;
    std::unordered_set<std::string> *ptr_phrase_level_rule_set_;

private:


};

#endif //PCFG_NLPBASE_H
