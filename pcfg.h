//
// Created by  ngs on 06/07/2018.
//

#ifndef PCFG_PCFG_H
#define PCFG_PCFG_H

#include "common.h"
#include "binarytree.h"
class PCFG{
public:
    PCFG();
    ~PCFG();
    void Allocate();
    void BuildTreeFromPTB(const char *file_name);
    void SupervisedTraining(const char *file_name);
    void CountAll();
    void CountValue(std::string str);
    void CountRule(PCFG_Rule rule);
    void CalcWeight();
    void SaveModel(std::string model_file);
    std::string IterationTree(Node *ptr_root, bool isTraining);
private:
    std::vector<BinaryTree *> tree_vector_;
    Rule_Map *ptr_rule_cout_map_;
    Element_Map *ptr_element_cout_map_;
    Rule_Weight_Map *ptr_rule_weight_map_;
};

#endif //PCFG_PCFG_H
