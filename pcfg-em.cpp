//
// Created by  ngs on 16/07/2018.
//
#include "pcfg-em.h"

PCFGEM::PCFGEM() {
    ptr_rule_map_ = new Rule_Map;
    ptr_non_terminal_map_ = new std::unordered_map<std::string, int>;
    ptr_terminal_map_ = new std::unordered_map<std::string, int>;
    instance_num_ = 0;
    ptr_alpha_map_ = new IO_Map;
    ptr_beta_map_ = new IO_Map;
    ptr_rule_weight_map_ = new Rule_Weight_Map;
}

PCFGEM::~PCFGEM() {
   delete ptr_terminal_map_;
   delete ptr_rule_map_;
   delete ptr_non_terminal_map_;
   //delete each row in matrix;
   for(auto it = ptr_str_matrix_->begin(); it != ptr_str_matrix_->end(); ++it){
       delete &(*it);
   }
   //delete the matrix;
   delete ptr_str_matrix_;
   delete ptr_alpha_map_;
   delete ptr_beta_map_;
   delete ptr_rule_weight_map_;
}

/**
 * read the model.txt, non-terminator.txt and sentence.txt to init CFG.
 *
 * @param rule_file
 * @param non_terminator
 * @param terminator
 */

void PCFGEM::InitCFG(const std::string &rule_file, const std::string &non_terminator, const std::string &terminator) {
    std::ifstream if_rule(rule_file), if_non(non_terminator), if_t(terminator);
    std::string str;
    int index = 0;
    while (std::getline(if_rule,str)){
        std::stringstream ss(str);
        std::string tag1,tag2,tag3;
        ss >> tag1;
        ss >> tag2;
        ss >> tag3;
        PCFG_Rule rule = std::make_pair(tag1, std::make_pair(tag2,tag3));
        ptr_rule_map_->insert(std::make_pair(rule,index));
        //init weight map as 0;
        ptr_rule_weight_map_->insert(std::make_pair(rule,0));
        index ++;
    }
    index = 0;
    while (std::getline(if_non,str)){
       std::stringstream ss(str);
       std::string non_str;
       ss >> non_str;
       ptr_non_terminal_map_->insert(std::make_pair(non_str,index));
       index++;
    }
    index = 0;
    while (std::getline(if_t,str)){
        std::stringstream ss(str);
        std::string t_str;
        while (ss >> t_str){
            ptr_terminal_map_->insert(std::make_pair(t_str,index));
            index++;
        }
    }
}
/**
 * Init alpha and beta for each Inside and Outside calculation.
 *
 * @param vector
 */

void PCFGEM::InitAlphaBeta(std::vector<std::string> &x_vector) {
    //iterate each rule.
    for(auto itt = ptr_non_terminal_map_->begin(); itt!=ptr_non_terminal_map_->end(); ++itt){
        std::string rule_str = (*itt).first;
        //for each word in the sentence.
        for(int i=0; i<x_vector.size(); ++i){
            IO_Tuple alpha_rule = std::make_pair(rule_str,std::make_pair(i,i));
            double alpha_value = 0;
            PCFG_Rule weight_rule = std::make_pair(rule_str,std::make_pair(x_vector[i],NO_RIGHT_CHILD_FLAG));
            if(ptr_rule_weight_map_->find(weight_rule) != ptr_rule_weight_map_->end()){
                alpha_value = ptr_rule_weight_map_->find(weight_rule)->second;
            }
            if(ptr_rule_map_->find(alpha_rule) == ptr_rule_map_->end()){
                ptr_alpha_map_->insert(std::make_pair(alpha_rule,alpha_value));
            }
        }
        int beta_value = 0;
        IO_Tuple beta_rule = std::make_pair(rule_str,std::make_pair(1,x_vector.size()));
        if(rule_str == ROOT_NODE){
            beta_value = 1;
        }
        ptr_beta_map_->insert(std::make_pair(beta_rule,beta_rule));
    }
}

/**
 * init the alpha and beta
 */
void PCFGEM::InitIO() {
    //init alpha(A, i, i), find the value in the weight map, otherwise set it as 0;
    //iterate each sentence.
    for(auto it = ptr_str_matrix_->begin(); it != ptr_str_matrix_->end(); ++it){
        std::vector<std::string> x_vector = (*it);
    }

    for(auto it = ptr_str_matrix_->begin(); it != ptr_str_matrix_->end(); ++it){
        std::vector<std::string> str_vector = (*it);
        for(int l=1; l<str_vector.size(); ++l){
            for(int i=0; i< str_vector.size() -l; ++i){

            }
        }
    }
}

/**
 * randomize the rule weight for the inside/outside algorithm.
 */
void PCFGEM::RandomizeRuleWeight() {

}

void PCFGEM::InitExpectCount() {

}

void PCFGEM::EM() {

    for (auto it= ptr_str_matrix_->begin(); it!=ptr_str_matrix_->end() ; ++it) {
        std::vector<std::string> x_vector = (*it);
        EMSentence(x_vector);
    }
}

void PCFGEM::EMSentence(std::vector<std::string> &x_vector) {

}

void PCFGEM::Training(const char *file_name) {
    InitCFG(MODEL_FILE,NON_TERMINATOR_FILE,SENTENCE_FILE);
    ExtractSentenceFile(file_name);
    EM();
}



