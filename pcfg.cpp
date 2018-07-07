//
// Created by  ngs on 06/07/2018.
//
#include <sstream>
#include "pcfg.h"

PCFG::PCFG(const char *file_name) {
    BuildTreeFromPTB(file_name);
}

void PCFG::BuildTreeFromPTB(const char *file_name) {
    std::ifstream ifs(file_name);
    std::string str;
    if(std::getline(ifs,str)){
        BinaryTree *ptr_tree = new BinaryTree(str);
        tree_vector_.push_back(ptr_tree);
    }
}

void PCFG::SupervisedTraining() {

}
