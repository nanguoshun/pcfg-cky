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
    delete ptr_non_terminator_map_;
}

void PCFG::Allocate() {
    ptr_rule_cout_map_ = new Rule_Map;
    ptr_element_cout_map_ = new Element_Map;
    ptr_rule_weight_map_ = new Rule_Weight_Map;
    ptr_non_terminator_map_ = new std::unordered_map<std::string, int>;
}

void PCFG::BuildTreeFromPTB(const char *file_name) {
    std::ifstream ifs(file_name);
    std::string str;
    while (std::getline(ifs, str)) {
        BinaryTree *ptr_tree = new BinaryTree(str);
        tree_vector_.push_back(ptr_tree);
    }
}

void PCFG::GenerateNonTerminatorMap() {
    int i = 0;
    for (auto it = ptr_rule_weight_map_->begin(); it != ptr_rule_weight_map_->end(); ++it) {
        std::string non_terminator_str = (*it).first.first;
        if (ptr_non_terminator_map_->find(non_terminator_str) == ptr_non_terminator_map_->end()) {
            ptr_non_terminator_map_->insert(std::make_pair(non_terminator_str, i));
            ++i;
        }
    }
}

void PCFG::CountAll() {
    for (std::vector<BinaryTree *>::iterator it = tree_vector_.begin(); it != tree_vector_.end(); ++it) {
 //       start_index_ = 0;
 //       end_index_ = 0;
 //       std::pair<std::string, int> pair = IterationTree((*it)->GetRootNode(), true, (*it)->GetXVector());
   //     (*it)->GetRootNode()->SetEndIndex((*it)->GetXVector()->size() -1);
        IterationTree((*it)->GetRootNode(),true,(*it)->GetXVector());
        std::cout << std::endl;
    }
}

/**
 * Iterate the tree and count the non-terminator and the rule
 * @param ptr_root
 * @param isTraining
 * @param p_x_vector
 * @return
 */

std::string PCFG::IterationTree(Node *ptr_root, bool isTraining, std::vector<std::string> *p_x_vector) {
    if (NULL == ptr_root) {
        //return empty string;
        return std::string();
    }
    std::string root_str = ptr_root->GetData();
    std::string str_left = IterationTree(ptr_root->GetLeftNode(), isTraining, p_x_vector);
    if(str_left.empty()){
        //the terminator don't have left and right child.
#ifdef IF_DEBUG_
        std::cout << root_str << " ";
#endif
        p_x_vector->push_back(root_str);
        return root_str;
    }
    std::string str_right = IterationTree(ptr_root->GetRightNode(), isTraining, p_x_vector);
    if(str_right.empty()){
        str_right = NO_RIGHT_CHILD_FLAG;
    }
    //just count the value in the training mode;
    if(isTraining){
        CountValue(root_str);
        CountRule(std::make_pair(root_str, std::make_pair(str_left, str_right)));
    }
    return root_str;
}


/*
std::pair<std::string, int> PCFG::IterationTree(Node *ptr_root, bool isTraining, std::vector<std::string> *p_x_vector) {
    if (NULL == ptr_root) {
        //return empty string;
        std::pair<std::string, int> pair = std::make_pair(TERMINATOR_FLAG,NEGATIVE_VALUE);
        return pair;
    }
    std::string root_str = ptr_root->GetData();
    std::pair<std::string, int>  pair_left = IterationTree(ptr_root->GetLeftNode(), isTraining, p_x_vector);
    if(pair_left.second == NEGATIVE_VALUE){
        //the terminator don't have left and right child.
#ifdef IF_DEBUG__
        std::cout << root_str << " ";
#endif
        p_x_vector->push_back(root_str);
        std::pair<std::string, int> pair = std::make_pair(root_str,start_index_);
        ptr_root->SetStartIndex(start_index_);
        start_index_ ++;
        return pair;
    }
    ptr_root->SetStartIndex(pair_left.second);
    std::pair<std::string, int> pair_right = IterationTree(ptr_root->GetRightNode(), isTraining, p_x_vector);
    if(pair_right.second == NEGATIVE_VALUE){
        pair_right = std::make_pair(NO_RIGHT_CHILD_FLAG,end_index_);
        end_index_++;
    }
    ptr_root->SetEndIndex(pair_right.second);
    //just count the value in the training mode;
    if(isTraining){
        CountValue(root_str);
        CountRule(std::make_pair(root_str, std::make_pair(pair_left.first, pair_right.first)));
    }
    return pair_left;
}
*/

/**
 * count value for each non-terminator.
 *
 * @param str
 */

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

/**
 * write the model to a file to facilitate testing in an offline mananer.
 * @param model_file
 */
void PCFG::SaveModel(std::string model_file) {
    std::ofstream ofs(model_file);
    for(Rule_Weight_Map::iterator it = ptr_rule_weight_map_->begin(); it!=ptr_rule_weight_map_->end();++it){
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

void PCFG::SupervisedTraining(const char *file_name) {
    BuildTreeFromPTB(file_name);
    CountAll();
    CalcWeight();
    SaveModel(MODEL_FILE);
    GenerateNonTerminatorMap();
    for(auto it = tree_vector_.begin(); it!=tree_vector_.end();++it){
        (*it)->PrintTree((*it)->GetRootNode());
    }
}

void PCFG::UnsupervisedTraining(const char *file_name) {

}