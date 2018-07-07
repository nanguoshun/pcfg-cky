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
    void ExtractSentenceFile(std::string sentence_file);
    std::string IterationExtract(Node *ptr_root);
    void Decoding(const char *file_name);
    void ReadModel(std::string model_file);
    void CKY();

private:
    Rule_Weight *ptr_rule_weight_map_;
    CKY_Score *ptr_CKY_score_map_;
    std::ofstream *ptr_sentence_;
};


#endif //PCFG_DECODING_H
