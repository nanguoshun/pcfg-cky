//
// Created by  ngs on 16/07/2018.
//

#include "nlpbase.h"

NLPBase::NLPBase() {
    ptr_str_matrix_ = new std::vector<std::vector<std::string>>;
    ptr_phrase_level_rule_set_ = new std::unordered_set<std::string>;
}

NLPBase::~NLPBase() {
    delete ptr_str_matrix_;
    delete ptr_phrase_level_rule_set_;
}

std::string NLPBase::IterationExtract(Node *ptr_root, int row) {
    if (NULL == ptr_root) {
        //return empty string;
        return std::string();
    }
    std::string root_str = ptr_root->GetData();
    std::string str_left = IterationExtract(ptr_root->GetLeftNode(), row);
    if (str_left.empty()) {
#ifdef IF_DEBUG_
        std::cout << root_str << " ";
#endif
        (*ptr_str_matrix_)[row].push_back(root_str);
        return root_str;
    }
    std::string str_right = IterationExtract(ptr_root->GetRightNode(), row);
    return root_str;
}

void NLPBase::ExtractSentenceFile(const char *file_name) {
    std::ifstream ifs(file_name);
    std::string str;
    int row = 0;
    while (std::getline(ifs, str)) {
        BinaryTree *ptr_tree = new BinaryTree(str);
        Node *ptr_root = ptr_tree->GetRootNode();
        std::vector<std::string> *ptr_str_vector = new std::vector<std::string>;
        ptr_str_matrix_->push_back(*ptr_str_vector);
        IterationExtract(ptr_root,row);
        row++;
        delete ptr_tree;
        std::cout << std::endl;
    }
    instance_num_ = row;
}

void NLPBase::GeneratePhraseLevelRuleMap(const std::string file_name) {
    std::ifstream ifs(file_name);
    std::string str;
    while (std::getline(ifs,str)){
        ptr_phrase_level_rule_set_->insert(str);
    }
}


