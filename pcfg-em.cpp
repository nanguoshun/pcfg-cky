//
// Created by  ngs on 16/07/2018.
//
#include "pcfg-em.h"

PCFGEM::PCFGEM() {
    ptr_rule_map_ = new Rule_Map;
    ptr_non_terminal_set_ = new std::unordered_set<std::string>;
    ptr_terminal_set_ = new std::unordered_set<std::string>;
    instance_num_ = 0;
    ptr_alpha_map_ = new IO_Map;
    ptr_beta_map_ = new IO_Map;
    ptr_rule_weight_map_ = new Rule_Weight_Map;
    ptr_phrase_rule_set_ = new PCFG_Rule_Set;
    ptr_rule_expected_count_temp_ = new Rule_Weight_Map;
    ptr_rule_expected_count_ = new Rule_Weight_Map;
    ptr_u_ = new IO_Map;
    ptr_bin_rule_u_ = new Binary_Rule_U_Map;
    ptr_symbol_rule_vector_map_ = new Symbol_Rule_Vector_Map;
}

PCFGEM::~PCFGEM() {
   delete ptr_terminal_set_;
   delete ptr_rule_map_;
   delete ptr_non_terminal_set_;
   //delete each row in matrix;
   for(auto it = ptr_str_matrix_->begin(); it != ptr_str_matrix_->end(); ++it){
       delete &(*it);
   }
   //delete the matrix;
   delete ptr_str_matrix_;
   delete ptr_alpha_map_;
   delete ptr_beta_map_;
   delete ptr_rule_weight_map_;
   delete ptr_phrase_rule_set_;
   delete ptr_rule_expected_count_temp_;
   delete ptr_rule_expected_count_;
   delete ptr_u_;
   delete ptr_bin_rule_u_;
   //delete the vector map;
   for(auto it = ptr_symbol_rule_vector_map_->begin(); it!= ptr_symbol_rule_vector_map_->end(); ++it){
       std::vector<PCFG_Rule> *ptr_vector = (*it).second;
       delete ptr_vector;
   }
   delete ptr_symbol_rule_vector_map_;
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
       ptr_non_terminal_set_->insert(non_str);
       index++;
    }
    index = 0;
    while (std::getline(if_t,str)){
        std::stringstream ss(str);
        std::string t_str;
        while (ss >> t_str){
            ptr_terminal_set_->insert(t_str);
            index++;
        }
    }
    InitSymbolRuleVectorMap();
    RandomizeRuleWeight();
}

/**
 * init rule map.
 *
 * @param ifs
 * @param str
 */
void PCFGEM::InitRuleMap(std::ifstream &ifs, std::string &str) {
    int index = 0;
    double a[] = {0.1,1,0.2,0.7,1,1,1,1,1};
    while (std::getline(ifs,str)){
        std::stringstream ss(str);
        std::string tag1,tag2,tag3;
        ss >> tag1;
        ss >> tag2;
        ss >> tag3;
        PCFG_Rule rule = std::make_pair(tag1, std::make_pair(tag2,tag3));
        ptr_rule_map_->insert(std::make_pair(rule,index));
        //create and then init weight map as 0;
//        ptr_rule_weight_map_->insert(std::make_pair(rule,a[index]));
        ptr_rule_weight_map_->insert(std::make_pair(rule,0));
        //create and then init weight map as 0;
        ptr_rule_expected_count_->insert(std::make_pair(rule,0));
        ptr_rule_expected_count_temp_->insert(std::make_pair(rule,0));
        if(tag3 != NO_RIGHT_CHILD_FLAG){
            ptr_phrase_rule_set_->insert(rule);
        }
        //
        index ++;
    }
#ifdef IF_DEBUG_
bool loop = true;
    auto it1 = ptr_rule_weight_map_->begin();
    auto it2 = ptr_rule_expected_count_->begin();
    auto it3 = ptr_rule_expected_count_temp_->begin();
    while (loop){
        if(it1 == ptr_rule_weight_map_->end()){
            return;
        }
        std::cout << "it1:"<<(*it1).first.first<<","<<(*it1).first.second.first<<","<<(*it1).first.second.second<<std::endl;
        std::cout << "it2:"<<(*it2).first.first<<","<<(*it2).first.second.first<<","<<(*it2).first.second.second<<std::endl;
        std::cout << "it3:"<<(*it3).first.first<<","<<(*it3).first.second.first<<","<<(*it3).first.second.second<<std::endl;
        it1 ++;
        it2 ++;
        it3 ++;
    }
    #endif
}

void PCFGEM::InitPhraseLevelMap(const std::string &str) {
    GeneratePhraseLevelRuleMap(str);
}

void PCFGEM::InitSymbolRuleVectorMap() {
    for(auto it = ptr_rule_weight_map_->begin(); it!=ptr_rule_weight_map_->end(); ++it){
        std::string symbol_str = (*it).first.first;
        PCFG_Rule rule = (*it).first;
        if(ptr_symbol_rule_vector_map_->find(symbol_str) == ptr_symbol_rule_vector_map_->end()){
            //insert a new item.
            std::vector<PCFG_Rule> *ptr_rule_vector = new std::vector<PCFG_Rule>;
            ptr_rule_vector->push_back(rule);
            ptr_symbol_rule_vector_map_->insert(std::make_pair(symbol_str,ptr_rule_vector));
        } else{
            //std::vector<PCFG_Rule>
            std::vector<PCFG_Rule> *ptr_rule_vector =  ptr_symbol_rule_vector_map_->find(symbol_str)->second;
            PCFG_Rule rule = (*it).first;
            std::vector<PCFG_Rule>::iterator it = std::find(ptr_rule_vector->begin(), ptr_rule_vector->end(),rule);
            if(it == ptr_rule_vector->end()){
              //insert the vector
              ptr_rule_vector->push_back(rule);
            }
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
    for (auto itt = ptr_non_terminal_set_->begin(); itt != ptr_non_terminal_set_->end(); ++itt) {
        std::string rule_str = (*itt);
        //for each word in the sentence.
        for (int i = 1; i <= x_vector.size(); ++i) {
            IO_Tuple alpha_rule = std::make_pair(rule_str, std::make_pair(i, i));
            double alpha_value = 0;
            PCFG_Rule weight_rule = std::make_pair(rule_str, std::make_pair(x_vector[i - 1], NO_RIGHT_CHILD_FLAG));
            if (ptr_rule_weight_map_->find(weight_rule) != ptr_rule_weight_map_->end()) {
                alpha_value = ptr_rule_weight_map_->find(weight_rule)->second;
            }
            if (ptr_alpha_map_->find(alpha_rule) == ptr_alpha_map_->end()) {
                ptr_alpha_map_->insert(std::make_pair(alpha_rule, alpha_value));
#ifdef IF_DEBUG__
                std::cout << "alpha: " << alpha_rule.first << "," << alpha_rule.second.first << ","
                          << alpha_rule.second.second << " value is: " << alpha_value << std::endl;
#endif
            }
        }
        int beta_value = 0;
        IO_Tuple beta_rule = std::make_pair(rule_str, std::make_pair(1, x_vector.size()));
        if (rule_str == ROOT_NODE) {
            beta_value = 1;
        }
        ptr_beta_map_->insert(std::make_pair(beta_rule, beta_value));
    }
    //set \Beta(S, i, n) = 0, i>1;
    for(int i=2; i<=x_vector.size(); ++i){
        IO_Tuple beta_rule = std::make_pair(ROOT_NODE,std::make_pair(i,x_vector.size()));
        ptr_beta_map_->insert(std::make_pair(beta_rule,0));
    }
}

/**
 * randomize the rule weight for the inside/outside algorithm.
 */
void PCFGEM::RandomizeRuleWeight() {
    for(auto it = ptr_symbol_rule_vector_map_->begin(); it!= ptr_symbol_rule_vector_map_->end(); ++it){
       std::vector<PCFG_Rule> *ptr_vector = (*it).second;
       double weight = ((double)1) / ((double)ptr_vector->size());
       for(auto itt = ptr_vector->begin(); itt != ptr_vector->end(); ++itt){
           auto it_weight = ptr_rule_weight_map_->find((*itt));
           if(it_weight != ptr_rule_weight_map_->end()){
               (*it_weight).second = weight;
#ifdef IF_DEBUG_
               std::cout << "Rule "<<(*itt).first<<","<<(*itt).second.first<<","<<(*itt).second.second<<" Weight is: "<<weight<<std::endl;
#endif
           } else{
               std::cout << "Randomized Error: no rules found in the rule weight map"<<std::endl;
           }
       }
   }
}

double PCFGEM::CalcZ(std::vector<std::string> &x_vector) {
    IO_Tuple tuple = std::make_pair(ROOT_NODE,std::make_pair(1,x_vector.size()));
    double z = ptr_alpha_map_->find(tuple)->second;
    return  z;
}

/**
 * Calc all alpha for a sentence x_vector.
 * @param x_vector
 */

void PCFGEM::CalcAlpha(std::vector<std::string> &x_vector) {
    for (int l = 1; l < x_vector.size(); ++l) {
        for (int i = 1; i <= x_vector.size() - l; ++i) {
            int j = i + l;
            for (auto it = ptr_phrase_rule_set_->begin(); it != ptr_phrase_rule_set_->end(); ++it) {
                std::string rule_str = (*it).first;
                PCFG_Rule rule = (*it);
                double weight = GetRuleWeight(rule);
                double value = CalcAlpha(i, j, rule_str, weight);
                IO_Tuple alpha_tuple = std::make_pair(rule_str, std::make_pair(i, j));
                ptr_alpha_map_->insert(std::make_pair(alpha_tuple, value));
#ifdef IF_DEBUG_
                std::cout << "The rule is: " << rule_str << "," << rule.second.first << "," << rule.second.second
                          << " The index is: " << i << "," << j << " Value is: " << value << std::endl;
#endif

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

double PCFGEM::CalcAlpha(int i, int j, std::string &rule_str,double weight) {
    double value = 0;
    std::vector<PCFG_Rule> *ptr_rule_vector =  GetRuleVector(rule_str);
    if(NULL == ptr_rule_vector){
        return 0;
    }
    //for all rules of A->BC,  which indicates all rules start with symbol A.
    for (auto itt = ptr_rule_vector->begin(); itt != ptr_rule_vector->end(); ++itt) {
        value += GetAlpha(i,j,(*itt),weight);
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

double PCFGEM::GetAlpha(int i, int j,PCFG_Rule &binary_rule, double weight) {
    double value = 0;
    for (int k = i; k < j; ++k) {
        std::string str_b = binary_rule.second.first;
        std::string str_c = binary_rule.second.second;
        double alpha_b_i_k = GetValue(ptr_alpha_map_,str_b,i,k);
        double alpha_b_k_j = GetValue(ptr_alpha_map_,str_c,k+1,j);
        if(alpha_b_i_k == NEGATIVE_VALUE){
            alpha_b_i_k = 0;
        }
        if(alpha_b_k_j == NEGATIVE_VALUE){
            alpha_b_k_j = 0;
        }
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

/*
double PCFGEM::GetIOValue(const IO_Map *ptr_map, const IO_Tuple &tuple) {
    double value = 0;
    if(ptr_map->find(tuple) != ptr_map->end()){
        value = ptr_map->find(tuple)->second;
    }
    return  value;
}
*/

double PCFGEM::CalcBeta(std::vector<std::string> &x_vector, std::string &rule_str, int i, int j) {
    //check if the beta is stored in the map;
    double value = GetValue(ptr_beta_map_,rule_str,i,j);
    if(i == 1 && j == x_vector.size()){
        std::cout << "beta at the end"<<std::endl;
        if(ROOT_NODE == rule_str){
            tmp_count_ ++;
            return  1;
        } else{
            return  0;
        }
    }
    if(NEGATIVE_VALUE != value){
        return value;
    } else{
        value = 0;
    }
    //check if
    Rule_Weight_Vector rule_vector_right = GetRulesAsRightChild(rule_str);
    //right part
    double right_value = 0;
    if(0 != rule_vector_right.size()){
        for(auto it = rule_vector_right.begin(); it!=rule_vector_right.end(); ++it){
            double weight = (*it).second;
            std::string rule_str_c = (*it).first.second.first;
            std::string rule_str_b = (*it).first.first;
            for(int k=1; k <= i-1; ++k){
                double alpha_c_k_i = GetValue(ptr_alpha_map_,rule_str_c,k,i-1);
                if(NEGATIVE_VALUE == alpha_c_k_i){
                    alpha_c_k_i = 0;
                }
                double beta_k_j = CalcBeta(x_vector,rule_str_b,k,j);
                std::cout << "RIGHT: To calc beta: " << rule_str << "," << " the rules are " << rule_str_b << "," << rule_str_c
                          << "," << rule_str << ", the index are " << k << "," << i << " beta is: "<<beta_k_j<<" temp count is: "<<tmp_count_<< std::endl;
                right_value += weight * alpha_c_k_i * beta_k_j;
            }
        }
    }
    //left part
    Rule_Weight_Vector rule_vector_left = GetRulesAsLeftChild(rule_str);
    double left_value = 0;
    if (0 != rule_vector_left.size()) {
        for (auto it = rule_vector_left.begin(); it != rule_vector_left.end(); ++it) {
            double weight = (*it).second;
            std::string rule_str_c = (*it).first.second.second;
            std::string rule_str_b = (*it).first.first;
            for (int k = j + 1; k <= x_vector.size(); ++k) {
                double alpha_j_k = GetValue(ptr_alpha_map_, rule_str_c, j + 1, k);
                if(NEGATIVE_VALUE == alpha_j_k){
                    alpha_j_k = 0;
                }
                double beta_i_k = CalcBeta(x_vector, rule_str_b, i, k);
                std::cout << "LEFT: To calc beta: " << rule_str << "," << " the rules are " << rule_str_b << "," << rule_str
                          << "," << rule_str_c << ", the index are " << i << "," << k << " beta is: "<<beta_i_k<<" temp count is: "<<tmp_count_<<std::endl;
                left_value += weight * alpha_j_k * beta_i_k;
            }
        }
    }
    value = right_value + left_value;
    return  value;
}

std::vector<PCFG_Rule>* PCFGEM::GetRuleVector(std::string &rule_str) {
    auto it = ptr_symbol_rule_vector_map_->find(rule_str);
    if(it != ptr_symbol_rule_vector_map_->end()){
        return (*it).second;
    } else{
        return NULL;
    }
}

Rule_Weight_Vector PCFGEM::GetRulesAsRightChild(std::string &rule_str) {
    Rule_Weight_Vector vector;
    for(auto it = ptr_phrase_rule_set_->begin(); it!=ptr_phrase_rule_set_->end(); ++it){
        if((*it).second.second == rule_str){
            PCFG_Rule rule = (*it);
            double weight = GetRuleWeight(rule);
            vector.push_back(std::make_pair(rule,weight));
        }
    }
    return vector;
}

Rule_Weight_Vector PCFGEM::GetRulesAsLeftChild(std::string &rule_str) {
    Rule_Weight_Vector vector;
    for (auto it = ptr_phrase_rule_set_->begin(); it != ptr_phrase_rule_set_->end(); ++it) {
        if((*it).second.first == rule_str){
            PCFG_Rule rule = (*it);
            double weight = GetRuleWeight(rule);
            vector.push_back(std::make_pair(rule,weight));
        }
    }
    return vector;
}
double PCFGEM::GetValue(const IO_Map *ptr_map, const std::string &rule_str, const int i, const int j) {
    double value = 0;
    IO_Tuple tuple = std::make_pair(rule_str, std::make_pair(i,j));
    if(ptr_map->find(tuple) != ptr_map->end()){
        value = ptr_map->find(tuple)->second;
    } else{
        return  NEGATIVE_VALUE;
    }
    return  value;
}

/**
 * Calculate the beta for a sentence x_vector.
 * @param x_vector
 */

/*
void PCFGEM::CalcBeta(std::vector<std::string> &x_vector) {
    for (auto it = ptr_phrase_level_rule_set_->begin(); it != ptr_phrase_level_rule_set_->end(); ++it) {
        std::string rule_str = (*it).first;
        for (int l = 1; l < x_vector.size(); ++l) {
            for (int i = 0; i < x_vector.size() - l; ++i) {
                int j = i + l;
                double value = CalcBeta(x_vector, i, j,rule_str);
                IO_Tuple beta_tuple = std::make_pair(rule_str,std::make_pair(i,j));
                ptr_beta_map_->insert(std::make_pair(beta_tuple,value));
            }
        }
    }
}
*/
/**
 * Calc beta(rule_str, i, j)
 *
 * Assume the rule rule_str indicates A, which is the first and second child of the rules B->CA and B->AC.
 *
 * @param i
 * @param j
 * @param rule_str
 * @return
 */
 /*
double PCFGEM::CalcBeta(std::vector<std::string> &x_vector,int i, int j, std::string rule_str) {
    double value = 0;
    for(auto it = ptr_phrase_rule_set_->begin(); it != ptr_phrase_rule_set_->end(); ++it){
        PCFG_Rule binary_rule = (*it).first;
        double weight = 0;
        // for B-> CA
        if(rule_str == binary_rule.second.second){
            weight = (*it).second;
            value += GetBeta(x_vector,weight,binary_rule,i,j,true);
        }
        // for B-> AC
        if(rule_str == binary_rule.second.first){
            weight = (*it).second;
            value += GetBeta(x_vector,weight,binary_rule,i,j,false);
        }
    }
}
*/
/**
 *
 * @param weight
 * @param binary_rule
 * @param i
 * @param j
 * @param k_right: k is bigger than i or not.
 * @return
 */

/*
double PCFGEM::GetBeta(std::vector<std::string> &x_vector,double weight, PCFG_Rule &binary_rule, int i, int j, bool k_left) {
    double value = 0;
    // if the k is smaller than i;
    if(k_left){
        for(int k=1; k<i; ++k){
            IO_Tuple tuple_alpha = std::make_pair(binary_rule.second.first,std::make_pair(k, i-1));
            IO_Tuple tuple_beta = std::make_pair(binary_rule.first,std::make_pair(k,j));
            double alpha_k_i = GetIOValue(ptr_alpha_map_,tuple_alpha);
            double beta_k_J = GetIOValue(ptr_beta_map_,tuple_beta);
            value += weight * alpha_k_i * beta_k_J;
        }
        //if the k is bigger than i;
    } else{
       for(int k=j+1; k<x_vector.size(); ++k){
            IO_Tuple tuple_alpha = std::make_pair(binary_rule.second.second,std::make_pair(j+1,k));
            IO_Tuple tuple_beta = std::make_pair(binary_rule.first,std::make_pair(i,k));
            double alpha_j_k = GetIOValue(ptr_alpha_map_,tuple_alpha);
            double beta_i_k = GetIOValue(ptr_beta_map_,tuple_beta);
            value += weight * alpha_j_k * beta_i_k;
       }
    }
    return value;
}
*/

void PCFGEM::Reset() {
    ptr_alpha_map_->clear();
    ptr_beta_map_->clear();
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
    double value = 0;
    std::string str_a = rule.first;
    std::string str_b = rule.second.first;
    std::string str_c = rule.second.second;
    double weight = GetRuleWeight(rule);
    for(int l= 1; l < x_vector.size(); ++l){
        for(int i = 1; i <= x_vector.size()-l; ++i){
            int j = i + l;
            double beta_a = CalcBeta(x_vector,str_a,i,j);
#ifdef IF_DEBUG__
            std::cout << "the rule and beta is: " << str_a << "," << str_b << "," << str_c << "," << i << "," << j
                          << ", the beta is:" << beta_a << std::endl;
#endif
            for(int k = i; k <= j-1; ++k){
                double alpha_b = GetValue(ptr_alpha_map_,str_b,i,k);
                double alpha_c = GetValue(ptr_alpha_map_,str_c,k+1,j);
                if(alpha_b == NEGATIVE_VALUE){
                    alpha_b = 0;
                }
                if(alpha_c ==NEGATIVE_VALUE){
                    alpha_c = 0;
                }
                // u(A->BC, i, k, j) = beta(A, i, j) * weight(A->BC) * alpha(B, i, k) * alpha(C, k+1, j)
                value += beta_a * weight * alpha_b * alpha_c;
            }
            // store the result in the map.
        }
    }
    return  value / Z;
}

/**
 * Calc U(i,j).
 * @param x_vector
 * @param rule_str
 * @param i
 * @param j
 */

void PCFGEM::CalcSymbol_U(std::vector<std::string> &x_vector, std::string &rule_str, int i, int j) {
    double alpha = GetValue(ptr_alpha_map_,rule_str,i,j);
    double beta = CalcBeta(x_vector,rule_str,i,j);
    double value = alpha * beta;
    IO_Tuple tuple = std::make_pair(rule_str,std::make_pair(i,j));
    //ptr_u_->insert(std::make_pair(tuple,value));
}


double PCFGEM::GetRuleWeight(PCFG_Rule &rule) {
    double weight = 0;
    if(ptr_rule_weight_map_->find(rule) != ptr_rule_weight_map_->end()){
        weight = ptr_rule_weight_map_->find(rule)->second;
    }
    return weight;
}

/**
 *
 * @param x_vector
 * @param rule
 * @param Z
 * @return
 */
double PCFGEM::CalcWordLevelRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z) {
    /*
    for(int l= 1; l < x_vector.size(); ++l){
        for(int i = 1; i <= x_vector.size()-l; ++i){
            int j = i + l;
            CalcSymbol_U(x_vector,rule.first,i,j);
        }
    }
    */
    double value = 0;
    std::string rule_str = rule.first;
    for(int i = 1; i<=x_vector.size(); ++i){
        // x_i = x;
        if(x_vector[i-1] == rule.second.first){
            double alpha = GetValue(ptr_alpha_map_,rule_str,i,i);
            double beta = CalcBeta(x_vector,rule_str,i,i);
            value += alpha * beta;
        }
        //ptr_u_->insert(std::make_pair(tuple,value));
    }
    return value / Z;
}

/**
 * calc the expected count for a rule.
 *
 * @param x_vector
 * @param rule
 * @return
 */
double PCFGEM::CalcRuleCount(std::vector<std::string> &x_vector, PCFG_Rule &rule, double Z) {
    // calc the count (A-X) for sentence x_vector
    if(rule.second.second == NO_RIGHT_CHILD_FLAG){
        return CalcWordLevelRuleCount(x_vector, rule, Z);
    } else{
        //calc the count(A->BC) for sentence x_vector
        return CalcPhraseLevelRuleCount(x_vector, rule, Z);
    }
}

/**
 * EM for a sentence x_vector.
 * @param x_vector
 */
double PCFGEM::CalcExpectedCount(std::vector<std::string> &x_vector) {
    //calculate the rule map. no need to reset tmp_map for each sentence.
    double Z = CalcZ(x_vector);
    for (auto it = ptr_rule_expected_count_temp_->begin(); it != ptr_rule_expected_count_temp_->end(); ++it) {
        PCFG_Rule rule = (*it).first;
        double rule_count = 0;
        rule_count = CalcRuleCount(x_vector, rule, Z);
        (*it).second = rule_count;
#ifdef IF_DEBUG_
        std::cout << "the rule and beta is: " << rule.first << "," << rule.second.first << "," << rule.second.second
                  << " count value is: " << rule_count << std::endl;
#endif
    }
    return Z;
}

/**
 * Calc the summation of rule in the M step.
 *
 * @param ptr_rule_vector
 * @return
 */
double PCFGEM::CalcDenominator(std::vector<PCFG_Rule> *ptr_rule_vector) {
    double value = 0;
    for(auto it = ptr_rule_vector->begin(); it!=ptr_rule_vector->end(); ++it){
        auto it_expected = ptr_rule_expected_count_->find((*it));
        if(it_expected != ptr_rule_expected_count_->end()){
            value += (*it_expected).second;
        }else{
            std::cout <<"denominator error: no rule in the expected count map"<<std::endl;
        }
    }
    return value;
}

void PCFGEM::SumExpectedCount() {
    //summing expected count of all sentences;
    auto itt = ptr_rule_expected_count_->begin();
    for(auto it_temp = ptr_rule_expected_count_temp_->begin(); it_temp != ptr_rule_expected_count_temp_->end(); ++it_temp) {
        (*itt).second += (*it_temp).second;
        ++itt;
    }
}


double PCFGEM::EStep() {
    double Z = 0;
    for (auto it = ptr_str_matrix_->begin(); it != ptr_str_matrix_->end(); ++it) {
        tmp_count_ = 0;
        std::vector<std::string> x_vector = (*it);
        InitAlphaBeta(x_vector);
        CalcAlpha(x_vector);
        Z += CalcExpectedCount(x_vector);
        SumExpectedCount();
        Reset();
    }
    return Z;
}

void PCFGEM::MStep() {
    for (auto it = ptr_rule_weight_map_->begin(); it != ptr_rule_weight_map_->end(); ++it) {
        PCFG_Rule rule = (*it).first;
        std::string rule_str = rule.first;
        double numerator = 0;
        auto it_n = ptr_rule_expected_count_->find(rule);
        if (it_n != ptr_rule_expected_count_->end()) {
            numerator = (*it_n).second;
        } else {
            std::cout << "error: no rule in the expected count map" << std::endl;
        }
        double denominator = 0;
        auto it_d = ptr_symbol_rule_vector_map_->find(rule_str);
        if (it_d != ptr_symbol_rule_vector_map_->end()) {
            std::vector<PCFG_Rule> *ptr_vector = (*it_d).second;
            denominator = CalcDenominator(ptr_vector);
        } else {
            std::cout << "error: no rule in the rule vector map" << std::endl;
        }
        double weight = 0;
        if (denominator != 0) {
            weight = numerator / denominator;
        }
        //update the weight of the rule.
        (*it).second = weight;
#ifdef IF_DEBUG_
        std::cout << "the updated weight " << (*it).first.first << "," << (*it).first.second.first << ","
                  << (*it).first.second.second << " value is:" << (*it).second << std::endl;
#endif
    }
}

/**
 *  EM algorithm
 *
 *  E step: calc the expected count using the parameters.
 *  M step: update the rule weight using the expected count in E step.
 */
void PCFGEM::EM() {
    bool isConverge = true;
    double Current_Z = 0;
    double Pref_Z = 0;
    while(isConverge){
        //E-Step;
        Current_Z = EStep();
        //M-Step;
        MStep();
        //reset expected count;
        for(auto it = ptr_rule_expected_count_->begin(); it != ptr_rule_expected_count_->end(); ++it){
            (*it).second = 0;
        }
        Pref_Z = Current_Z;
        if(Current_Z - Pref_Z <= 0.01){
            isConverge = false;
            std::cout << "Training is finished"<<std::endl;
        }
        std::cout << "The value of Z is:"<<Current_Z<<std::endl;
    }
}

/**
 * write the model to a file to facilitate testing in an offline mananer.
 * @param model_file
 */
void PCFGEM::SaveModel(std::string model_file) {
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

void PCFGEM::Training(const char *file_name) {
    InitCFG(MODEL_FILE,MODEL_FILE,SENTENCE_FILE);
    InitPhraseLevelMap(PHRASE_LEVEL_FILE);
    ExtractSentenceFile(file_name);
    EM();
    SaveModel(UNSUPERVISED_MODEL_FILE);
}



