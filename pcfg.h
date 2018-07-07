//
// Created by  ngs on 06/07/2018.
//

#ifndef PCFG_PCFG_H
#define PCFG_PCFG_H

#include "common.h"
#include "binarytree.h"
class PCFG{
public:
    PCFG(const char *file_name);
    void BuildTreeFromPTB(const char *file_name);
    void SupervisedTraining();

private:
    std::vector<BinaryTree *> tree_vector_;

};
#endif //PCFG_PCFG_H
