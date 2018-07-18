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
    ptr_binary_rule_weight_map_ = new Rule_Weight_Map;
    ptr_rule_expected_count_temp_ new Rule_Weight_Map;
    ptr_rule_expected_count_ = new Rule_Weight_Map;
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
    InitRuleMap(if_rule,str);
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
 * init rule map.
 *
 * @param ifs
 * @param str
 */
void PCFGEM::InitRuleMap(std::ifstream &ifs, std::string &str) {
    int index = 0;
    while (std::getline(ifs,str)){
        std::stringstream ss(str);
        std::string tag1,tag2,tag3;
        ss >> tag1;
        ss >> tag2;
        ss >> tag3;
        PCFG_Rule rule = std::make_pair(tag1, std::make_pair(tag2,tag3));
        ptr_rule_map_->insert(std::make_pair(rule,index));
        //create and then init weight map as 0;
        ptr_rule_weight_map_->insert(std::make_pair(rule,0));
        //create and then init weight map as 0;
        ptr_rule_expected_count_->insert(std::make_pair(rule,0));
        ptr_rule_expected_count_temp_->insert(std::make_pair(rule,0));
        //
        index ++;
    }
}

void PCFGEM::InitPhraseLevelMap(const std::string &str) {
    GeneratePhraseLevelRuleMap(str);
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
        ptr_beta_map_->insert(std::make_pair(beta_rule,beta_value));
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

/**
 *  EM algorithm
 *
 *  E step: calc the expected count using the parameters.
 *
 *  M step: update the rule weight using the expected count in E step.
 */

void PCFGEM::EM() {
    for (auto it= ptr_str_matrix_->begin(); it!=ptr_str_matrix_->end() ; ++it) {
        std::vector<std::string> x_vector = (*it);
        EMSentence(x_vector);
        Reset();
    }
}

/**
 * EM for a sentence x_vector.
 * @param x_vector
 */
void PCFGEM::EMSentence(std::vector<std::string> &x_vector) {
    //calculate the rule map. no need to reset tmp_map for each sentence.
    for(auto it = ptr_rule_expected_count_temp_->begin(); it!=ptr_rule_expected_count_temp_->end(); ++it){
        PCFG_Rule rule = (*it).first;
        double rule_value = 0;
        rule_value = CalcRuleCount(x_vector, rule);
        (*it).second = rule_value;
    }
}

/**
 * calc the expected count for a rule.
 *
 * @param x_vector
 * @param rule
 * @return
 */

double PCFGEM::CalcRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule) {
    double Z = CalcZ(x_vector);
    // calc the count (A-X) for sentence x_vector
    if(rule.second.second == NO_RIGHT_CHILD_FLAG){
        return CalcWordLevelRuleCount(x_vector, rule, Z);
    } else{
        //calc the count(A->BC) for sentence x_vector
        return CalcPhraseLevelRuleCount(x_vector, rule, Z);
    }
}

double PCFGEM::CalcZ(std::vector<std::string> &x_vector) {

}

/**
 * Calc all alpha for a sentence x_vector.
 * @param x_vector
 */

void PCFGEM::CalcAlpha(std::vector<std::string> &x_vector) {
    for(auto it = ptr_phrase_level_rule_map_->begin(); it != ptr_phrase_level_rule_map_->end(); ++it) {
        std::string rule_str = (*it).first;
        for(int l=1; l<x_vector.size(); ++l){
            for(int i=0; i<x_vector.size()-l; ++i){
                int j = i + l;
                double value = CalcAlpha(i,j,rule_str);
                IO_Tuple alpha_tuple = std::make_pair(rule_str,std::make_pair(i,j));
                ptr_alpha_map_->insert(std::make_pair(alpha_tuple,value));
            }
        }
    }
}

/**
 * Get the alpha(r, i, j)
 * @param i
 * @param j
 * @param rule_str
 * @return
 */

double PCFGEM::CalcAlpha(int i, int j, std::string rule_str) {
    double value = 0;
    for (auto itt = ptr_binary_rule_weight_map_->begin(); itt != ptr_binary_rule_weight_map_->end(); ++itt) {
        PCFG_Rule binary_rule = (*itt).first;
        if (rule_str == binary_rule.first) {
            double weight = (*itt).second;
            value += GetAlpha(weight,binary_rule,i,j);
        }
    }
    return value;
}

/**
 * Calc the sum of the product of \sum ( weigh(A->BC) * alpha(B, i, k) * alpha(C, k+1, j) )
 * @param weight: functinal weight.
 * @param binary_rule: for a rule A->BC
 * @param i: start position from 0 to size-1;
 * @param j: stop  position form i to size-1;
 * @return: the sum of product
 */

double PCFGEM::GetAlpha(double weight, PCFG_Rule &binary_rule, int i, int j) {
    double value = 0;
    for (int k = i; k < j; ++k) {
        std::string str_b = binary_rule.second.first;
        std::string str_c = binary_rule.second.second;
        IO_Tuple tuple_b = std::make_pair(str_b,std::make_pair(i, k));
        IO_Tuple tuple_c = std::make_pair(str_c,std::make_pair(k+1, j));
        double alpha_b_i_k = GetIOValue(ptr_alpha_map_,tuple_b);
        double alpha_b_k_j = GetIOValue(ptr_alpha_map_,tuple_c);
        value += weight * alpha_b_i_k * alpha_b_k_j;
    }
    return value;
}

/**
 * Get related alpha or beta value from the map.
 * @param ptr_map
 * @param tuple : alpha or beata tuple, eg. (A, (0,1))
 * @return
 */

double PCFGEM::GetIOValue(const IO_Map *ptr_map, const IO_Tuple &tuple) {
    double value = 0;
    if(ptr_map->find(tuple) != ptr_map->end()){
        value = ptr_map->find(tuple)->second;
    }
    return  value;
}

/**
 * Calculate the beta for a sentence x_vector.
 * @param x_vector
 */
void PCFGEM::CalcBeta(std::vector<std::string> &x_vector) {
    for(auto it = ptr_phrase_level_rule_map_->begin(); it != ptr_phrase_level_rule_map_->end(); ++it){
        std::string rule_str = (*it).first;

    }
}

/**
 * Calc beta(rule_str, i, j)
 *
 * Assume the rule rule_str indicates A, and rules are B->CA and B->AC respectively.
 *
 * @param i
 * @param j
 * @param rule_str
 * @return
 */
double PCFGEM::CalcBeta(std::vector<std::string> &x_vector,int i, int j, std::string rule_str) {
    double value = 0;
    for(auto it = ptr_binary_rule_weight_map_->begin(); it != ptr_binary_rule_weight_map_->end(); ++it){
        PCFG_Rule binary_rule = (*it).first;
        // for B->CA
        if(rule_str == binary_rule.second.second){
            double weight = (*it).second;
            value += GetBeta(x_vector,weight,binary_rule,i,j,true);
        }
        // for B->AC
        if(rule_str == binary_rule.second.first){
            double weight = (*it).second;
            value += GetBeta(x_vector,weight,binary_rule,i,j,false);
        }
    }
}

/**
 *
 * @param weight
 * @param binary_rule
 * @param i
 * @param j
 * @param k_right: k is bigger than i or not.
 * @return
 */
double PCFGEM::GetBeta(std::vector<std::string> &x_vector,double weight, PCFG_Rule &binary_rule, int i, int j, bool k_left) {
    double value = 0;
    if(k_left){
        for(int k=1; k<i; ++k){
            IO_Tuple tuple_alpha = std::make_pair(binary_rule.second.first,std::make_pair(k, i-1));
            IO_Tuple tuple_beta = std::make_pair(binary_rule.first,std::make_pair(k,j));
            double alpha_k_i = GetIOValue(ptr_alpha_map_,tuple_alpha);
            double beta_k_J = GetIOValue(ptr_beta_map_,tuple_beta);
            value += weight * alpha_k_i * beta_k_J;
        }
    } else{
       for(int k=j+1; k<x_vector.size(); ++k){

       }
    }
}

/**
 * Calc the expeccted count of Count(A->BC)
  *
  * @param x_vector
  * @param rule
  * @param Z
  * @return
  */
double PCFGEM::CalcPhraseLevelRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z) {
    for(int l=1; l < x_vector.size(); ++l){
        for(int i=0; i < x_vector.size()-l; ++i){
            int j = i + l;

        }
    }
}

/**
 *
 * @param x_vector
 * @param rule
 * @param Z
 * @return
 */

double PCFGEM::CalcWordLevelRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z) {

}

void PCFGEM::Reset() {
    ptr_alpha_map_->clear();
    ptr_beta_map_->clear();
}

void PCFGEM::Training(const char *file_name) {
    InitCFG(MODEL_FILE,NON_TERMINATOR_FILE,SENTENCE_FILE);
    InitPhraseLevelMap(PHRASE_LEVEL_FILE);
    ExtractSentenceFile(file_name);
    EM();
}



