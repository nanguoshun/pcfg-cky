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
    void ReadSentence(std::string sentence_file);
    void InitCKY(std::vector<std::string> *ptr_vector, CKY_Score *ptr_CKY_score);
    void InitScore();
    void InitCKY(std::vector<std::string> *ptr_x_vector);
    void CKY(std::vector<std::string> *ptr_x_vector);
private:
    Rule_Weight *ptr_rule_weight_map_;
    CKY_Map *ptr_cky_score_map_;
    std::vector<BinaryTree *> tree_vector_;
    bool isFirstWord_;
};


#endif //PCFG_DECODING_H
