//
// Created by  ngs on 06/07/2018.
//
#include "pcfg.h"

PCFG::PCFG() {
    Allocate();
}

PCFG::~PCFG() {
    delete ptr_rule_cout_map_;
    delete ptr_element_cout_map_;
    delete ptr_rule_weight_map_;
}

void PCFG::Allocate() {
    ptr_rule_cout_map_ = new std::unordered_map<PCFG_Rule, int, boost::hash<PCFG_Rule>>;
    ptr_element_cout_map_ = new std::unordered_map<std::string, int>;
    ptr_rule_weight_map_ = new std::unordered_map<PCFG_Rule,double,  boost::hash<PCFG_Rule>>;
}

void PCFG::BuildTreeFromPTB(const char *file_name) {
    std::ifstream ifs(file_name);
    std::string str;
    while (std::getline(ifs, str)) {
        BinaryTree *ptr_tree = new BinaryTree(str);
        tree_vector_.push_back(ptr_tree);
    }
}

void PCFG::SupervisedTraining(const char *file_name) {
    BuildTreeFromPTB(file_name);
    CountAll();
    CalcWeight();
    SaveModel(MODEL_FILE);
}

void PCFG::CountAll() {
    for (std::vector<BinaryTree *>::iterator it = tree_vector_.begin(); it != tree_vector_.end(); ++it) {
        Node *ptr_root = (*it)->GetRootNode();
        IterationTree(ptr_root);
        std::cout << std::endl;
    }
}

std::string PCFG::IterationTree(Node *ptr_root) {
    if (NULL == ptr_root) {
        //return empty string;
        return std::string();
    }
    std::string root_str = ptr_root->GetData();
    std::string str_left = IterationTree(ptr_root->GetLeftNode());
    if(str_left.empty()){
        //the terminator don't have left and right child.
#ifdef IF_DEBUG_
        std::cout << root_str << " ";
#endif
        return root_str;
    }
    std::string str_right = IterationTree(ptr_root->GetRightNode());
    if(str_right.empty()){
        str_right = NO_RIGHT_CHILD_FLAG;
    }
    CountValue(root_str);
    CountRule(std::make_pair(root_str, std::make_pair(str_left, str_right)));
    return root_str;
}

void PCFG::CountValue(std::string str) {
    if (ptr_element_cout_map_->find(str) == ptr_element_cout_map_->end()) {
        //if the element is not existed, then insert it and set count as 1;
        ptr_element_cout_map_->insert(std::make_pair(str, 1));
    } else {
        //count increases 1;
        ptr_element_cout_map_->find(str)->second += 1;
    }
#ifdef IF_DEBUG__
    std::cout << "the count of " << str << " is: " << ptr_element_cout_map_->find(str)->second << std::endl;
#endif
}

void PCFG::CountRule(PCFG_Rule rule) {
    if (ptr_rule_cout_map_->find(rule) == ptr_rule_cout_map_->end()) {
        ptr_rule_cout_map_->insert(std::make_pair(rule, 1));
    } else {
        ptr_rule_cout_map_->find(rule)->second += 1;
    }
#ifdef IF_DEBUG__
    std::cout << "the count of " << rule.first << " to " << rule.second.first << " , " << rule.second.second
              << " is: " << ptr_rule_cout_map_->find(rule)->second << std::endl;
#endif
}

void PCFG::CalcWeight() {
    for (Rule_Map::iterator it = ptr_rule_cout_map_->begin(); it != ptr_rule_cout_map_->end(); ++it) {
        std::string str = (*it).first.first;
        double weight = (double) (*it).second / (double) ptr_element_cout_map_->find(str)->second;
        PCFG_Rule rule = (*it).first;
        ptr_rule_weight_map_->insert(std::make_pair(rule,weight));
#ifdef IF_DEBUG__
        std::cout << "the weight of " << rule.first << " to " << rule.second.first << " , " << rule.second.second
                   << " is: "<< weight <<std::endl;
#endif
    }
}

void PCFG::SaveModel(std::string model_file) {
    std::ofstream ofs(model_file);
    for(Rule_Weight::iterator it = ptr_rule_weight_map_->begin(); it!=ptr_rule_weight_map_->end();++it){
        ofs << (*it).first.first;
        ofs << " ";
        ofs << (*it).first.second.first;
        ofs << " ";
        ofs << (*it).first.second.second;
        ofs << " ";
        ofs << (*it).second;
        ofs << "\n";
    }
}

void PCFG::ReadModel(std::string model_file) {
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

void PCFG::Decoding(const char *file_name) {
    ReadModel(MODEL_FILE);
    CKY();
}

void PCFG::CKY() {

}