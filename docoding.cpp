//
// Created by  ngs on 07/07/2018.
//

#include "decoding.h"

void Decoder::ReadModel(std::string model_file) {
    std::ifstream ifs(model_file);
    std::string str;
    while (std::getline(ifs,str)){
        std::string tag1, tag2, tag3;
        double weight;
        std::stringstream ss(str);
        ss >> tag1;
        ss >> tag2;
        ss >> tag3;
        ss >> weight;
        PCFG_Rule rule = std::make_pair(tag1, std::make_pair(tag2,tag3));
        ptr_rule_weight_map_->insert(std::make_pair(rule,weight));
    }
}

std::string Decoder::IterationExtract(Node *ptr_root) {
    if (NULL == ptr_root) {
        //return empty string;
        return std::string();
    }
    std::string root_str = ptr_root->GetData();
    std::string str_left = IterationExtract(ptr_root->GetLeftNode());
    if(str_left.empty()){
#ifdef IF_DEBUG_
        std::cout << root_str << " ";
#endif
        return root_str;
    }
    std::string str_right = IterationExtract(ptr_root->GetRightNode());
    return root_str;
}

void Decoder::Decoding(const char *file_name) {
    ReadModel(MODEL_FILE);
    CKY();
}

void Decoder::ExtractSentenceFile(std::string sentence_file) {
    ptr_sentence_ = new std::ofstream(sentence_file);
}