//
// Created by  ngs on 07/07/2018.
//

#include "decoding.h"

Decoder::Decoder() {
    ptr_rule_weight_map_ = new Rule_Weight_Map;
    ptr_binary_rule_weight_map_ = new Rule_Weight_Map;
    ptr_cky_score_map_ = new CKY_Score_Map;
    ptr_non_terminator_map_ = new std::unordered_map<std::string, int>;
    ptr_max_cky_score_map_ = new MAX_CKY_Sore_Map;
    ptr_parsing_map_ = new std::unordered_map<CKY_Tuple, int, boost::hash<CKY_Tuple>>;
    ptr_groundtruth_map_ = new std::unordered_map<CKY_Tuple, int, boost::hash<CKY_Tuple>>;
    ptr_phrase_level_rule_map_ = new std::unordered_map<std::string, int>;
    result_numerator_ = 0;
    result_denominator_precision_ = 0;
    result_denominator_recall_ = 0;
    start_index_ = 0;
    end_index_ = 0;
    line_index_ = 0;
}

Decoder::~Decoder() {
    delete ptr_rule_weight_map_;
    delete ptr_cky_score_map_;
    delete ptr_non_terminator_map_;
    delete ptr_max_cky_score_map_;
    delete ptr_parsing_map_;
    delete ptr_groundtruth_map_;
    delete ptr_binary_rule_weight_map_;
    delete ptr_phrase_level_rule_map_;
    for(std::vector<Node *>::iterator it = node_vector_.begin(); it!=node_vector_.end();++it){
        delete(*it);
    }
}

void Decoder::GeneratePhraseLevelRuleVector(const std::string file_name) {
    std::ifstream ifs(file_name);
    std::string str;
    int index = 0;
    while (std::getline(ifs,str)){
        ptr_phrase_level_rule_map_->insert(std::make_pair(str,index));
        index++;
    }
}

void Decoder::GenerateNonTerminatorMapFromWeighMap() {
    int i = 0;
    for (auto it = ptr_rule_weight_map_->begin(); it != ptr_rule_weight_map_->end(); ++it) {
        std::string non_terminator_str = (*it).first.first;
        if (ptr_non_terminator_map_->find(non_terminator_str) == ptr_non_terminator_map_->end()) {
            ptr_non_terminator_map_->insert(std::make_pair(non_terminator_str, i));
            ++i;
        }
    }
}
/**
 * Read the weight of each rule from a file generated in training phase.
 *
 * @param model_file
 */
void Decoder::ReadModel(std::string model_file) {
    std::ifstream ifs(model_file);
    std::string str;
    while (std::getline(ifs, str)) {
        std::string tag1, tag2, tag3;
        double weight;
        std::stringstream ss(str);
        ss >> tag1;
        ss >> tag2;
        ss >> tag3;
        ss >> weight;
        PCFG_Rule rule = std::make_pair(tag1, std::make_pair(tag2, tag3));
        ptr_rule_weight_map_->insert(std::make_pair(rule, weight));
        if(ptr_phrase_level_rule_map_->find(tag1) != ptr_phrase_level_rule_map_->end()){
            ptr_binary_rule_weight_map_->insert(std::make_pair(rule, weight));
#ifdef IF_DEBUG_
            std::cout << tag1 << " "<< tag2 <<" "<<tag3<<": "<<weight<<std::endl;
#endif
        }
    }
}

/**
 * Extract in an iteration manner and then write to a file to faciliate parsing.
 *
 * @para
 * @param ofs
 * @return
 */
std::string Decoder::IterationExtract(Node *ptr_root, std::ofstream *ofs) {
    if (NULL == ptr_root) {
        //return empty string;
        return std::string();
    }
    std::string root_str = ptr_root->GetData();
    std::string str_left = IterationExtract(ptr_root->GetLeftNode(), ofs);
    if (str_left.empty()) {
#ifdef IF_DEBUG__
        std::cout << root_str << " ";
#endif
        SaveSentenceFile(root_str, ofs);
        return root_str;
    }
    std::string str_right = IterationExtract(ptr_root->GetRightNode(), ofs);
    return root_str;
}

void Decoder::SaveSentenceFile(std::string root_str, std::ofstream *ofs) {
    (*ofs) << root_str;
    (*ofs) << SPACE_STRING;
}

/**
 * Init CKY score, each \pi(i, i, X) is set by P(X--> x_i)
 * @param ptr_x_vector: the parsing sentence.
 */
void Decoder::InitCKY(std::vector<std::string> *ptr_x_vector) {
    int size = ptr_x_vector->size();
    for (int i = 0; i < size; ++i) {
        std::pair<int, int> pos_pair = std::make_pair(i, i);
        for (auto it = ptr_non_terminator_map_->begin(); it != ptr_non_terminator_map_->end(); ++it) {
            std::string x_str = (*it).first;
            CKY_Tuple cky_tuple = std::make_pair(pos_pair, x_str);
            double weight = NEGATIVE_VALUE;
            //check the weight of the rule;
            std::string x_i = (*ptr_x_vector)[i];
            PCFG_Rule rule = std::make_pair(x_str, std::make_pair(x_i, NO_RIGHT_CHILD_FLAG));
            if (ptr_rule_weight_map_->find(rule) != ptr_rule_weight_map_->end()) {
                weight = ptr_rule_weight_map_->find(rule)->second;
                //calculation in log space;
                weight = std::log(weight);
            }
            ptr_cky_score_map_->insert(std::make_pair(cky_tuple, weight));
        }
    }
}

/**
 * CKY main body.
 *
 * @param ptr_x_vector: the parsing sentence.
 */
void Decoder::CKY(std::vector<std::string> *ptr_x_vector) {
    int size = ptr_x_vector->size();
    for (int l = 1; l < size; ++l) {
        for (int i = 0; i <= size - l -1; ++i) {
            int j = i + l;
            std::string max_str;
            for (auto it = ptr_non_terminator_map_->begin(); it != ptr_non_terminator_map_->end(); ++it) {
                std::string x_str = (*it).first;
                CKY_Tuple cky_tuple = std::make_pair(std::make_pair(i, j), x_str);
                Rule_Weight_Vector rule_vector;
                GetRuleWeight(x_str, rule_vector);
                double max_score = CalcMaxRule(i, j, x_str, rule_vector, ptr_x_vector);
                ptr_cky_score_map_->insert(std::make_pair(cky_tuple, max_score));
#ifdef IF_DEBUG__
                if(max_score > max){
                    max = max_score;
                    max_str = x_str;
                }
#endif
            }
#ifdef IF_DEBUG__
    std::cout << (*ptr_x_vector)[i] << "," << (*ptr_x_vector)[j] << "," << max_str << " is: " << max<<std::endl;
#endif
        }
    }
}

/**
 * Put the rule and weight of X->YZ to a vector, which is the first choice of CKY when calculating the maximum score.
 * @param x_str : the rule X.
 * @param rule_vector: The rules that start with the rule X.
 */
void Decoder::GetRuleWeight(std::string &x_str, Rule_Weight_Vector &rule_vector) {
    for (auto itt = ptr_binary_rule_weight_map_->begin(); itt != ptr_binary_rule_weight_map_->end(); ++itt) {
        std::string xx_str = (*itt).first.first;
        if (x_str == xx_str) {
            if ((*itt).first.second.second != NO_RIGHT_CHILD_FLAG) {
                rule_vector.push_back(std::make_pair((*itt).first, (*itt).second));
#ifdef IF_DEBUG__
                std::cout << (*itt).first.first << " to " << (*itt).first.second.first << " and "
                          << (*itt).first.second.second << ", weight is: " << (*itt).second << std::endl;
#endif
            }
        }
    }
}

/**
 * Get the Max score for each \pi(i, j, X);
 * @param i
 * @param j
 * @param x_str: X rule
 * @param rule_vector: the rules that start with the X rule
 * @param ptr_x_vector: the parsing sentence.
 * @return
 */
double Decoder::CalcMaxRule(int i, int j, const std::string &x_str, const Rule_Weight_Vector &rule_vector,
                            std::vector<std::string> *ptr_x_vector) {
    double max_score = NEGATIVE_VALUE;
    CKY_Tuple cky_tuple_y_max;
    CKY_Tuple cky_tuple_z_max;
    int split_point = 0;
    for (auto itt = rule_vector.begin(); itt != rule_vector.end(); ++itt) {
        std::string y_str =  (*itt).first.second.first;
        std::string z_str =  (*itt).first.second.second;
        double weight_x_yz = (*itt).second;
        for (int s = i; s < j; ++s) {
            CKY_Tuple cky_tuple_y = std::make_pair(std::make_pair(i, s), y_str);
            CKY_Tuple cky_tuple_z = std::make_pair(std::make_pair(s + 1, j), z_str);
            double score_i_s_y = ptr_cky_score_map_->find(cky_tuple_y)->second;
            double score_s_j_z = ptr_cky_score_map_->find(cky_tuple_z)->second;
            //calculation in log space;
            double cky_score = std::log(weight_x_yz) + score_i_s_y + score_s_j_z;
            if (cky_score > max_score) {
                max_score = cky_score;
                cky_tuple_y_max = cky_tuple_y;
                cky_tuple_z_max = cky_tuple_z;
                split_point = s;
            }
        }
    }
#ifdef IF_DEBUG__
    std::cout << "the max score for " << (*ptr_x_vector)[i] << "," << (*ptr_x_vector)[j] << "," << x_str << " is: "
              << max_score << ". the left and right childs are: " << cky_tuple_y_max.second << ","
              << cky_tuple_z_max.second << "/// the length is: "<<ptr_x_vector->size()<< ", the split point is: "<<split_point<<std::endl;
#endif
    InsertMaxScoreMap(i,j,x_str,cky_tuple_y_max,cky_tuple_z_max,split_point);
    return max_score;
}

/**
 * Save the the max info for construction of a tree.
 *
 * @param i
 * @param j
 * @param x_str: X
 * @param max_score
 * @param cky_tuple_y_max: left_child
 * @param cky_tuple_z_max: right_child
 * @param split_point
 */
void Decoder::InsertMaxScoreMap(int i, int j, const std::string &x_str, CKY_Tuple &cky_tuple_y_max, CKY_Tuple &cky_tuple_z_max, int split_point){
    CKY_Tuple cky_tuple = std::make_pair(std::make_pair(i, j), x_str);
    Child_Split child_split = std::make_pair(std::make_pair(cky_tuple_y_max.second,cky_tuple_z_max.second),split_point);
    ptr_max_cky_score_map_->insert(std::make_pair(cky_tuple,child_split));
}

/**
 * Extracting a complete sentence in a PTB format to faciliate parsing.
 * @param file_name
 */
void Decoder::ExtractSentenceFile(const char *file_name) {
    std::ofstream ofs(SENTENCE_FILE);
    std::ifstream ifs(file_name);
    std::string str;
    while (std::getline(ifs, str)) {
        BinaryTree *ptr_tree = new BinaryTree(str);
        tree_vector_.push_back(ptr_tree);
    }
    for (std::vector<BinaryTree *>::iterator it = tree_vector_.begin(); it != tree_vector_.end(); ++it) {
        Node *ptr_root = (*it)->GetRootNode();
        IterationExtract(ptr_root, &ofs);
        ofs << "\n";
    }
}

void Decoder::Clear() {
    ptr_cky_score_map_->clear();
    ptr_max_cky_score_map_->clear();
    ptr_parsing_map_->clear();
    ptr_groundtruth_map_->clear();
    start_index_ = 0;
    end_index_ = 0;
}

/**
 * build a binary tree for a sentence x_vector
 * @param x_vector sentence
 */

void Decoder::ConstructTree(const std::vector<std::string> &x_vector) {
    map_index_ = 0;
    Node *ptr_root_node = new Node(ROOT_NODE);
    IterationTree(x_vector, ptr_root_node, 0, x_vector.size()-1);
}

/**
 * Recursively build binary parsing tree and annotate the position for each
 * non-terminator after CKY algorithm.
 *
 * @param x_vector
 * @param pnode
 * @param i
 * @param j
 *
 */

void Decoder::IterationTree(const std::vector<std::string> &x_vector, Node *pnode, int i, int j) {
    pnode->SetStartIndex(i);
    pnode->SetEndIndex(j);
    int split_point = 0;
    std::string left_str;
    std::string right_str;
    std::string parent_str = pnode->GetData();
    CKY_Tuple tuple = std::make_pair(std::make_pair(i,j),parent_str);
    if(ptr_max_cky_score_map_->find(tuple) == ptr_max_cky_score_map_->end()){
        //if it is the last non-terminator.
        Node *ptr_left_node = new Node(x_vector[i]);
        pnode->SetLeftNode(ptr_left_node);
        pnode->SetStartIndex(i);
        pnode->SetEndIndex(i);
        //insert into map
        ptr_parsing_map_->insert(std::make_pair(tuple,map_index_));
        map_index_++;
        return;
    } else{
        split_point  = ptr_max_cky_score_map_->find(tuple)->second.second;
        left_str = ptr_max_cky_score_map_->find(tuple)->second.first.first;
        right_str = ptr_max_cky_score_map_->find(tuple)->second.first.second;
        Node *ptr_left_node = new Node(left_str);
        Node *ptr_right_node = new Node(right_str);
        pnode->SetLeftNode(ptr_left_node);
        pnode->SetRightNode(ptr_right_node);
        IterationTree(x_vector,ptr_left_node,i,split_point);
        IterationTree(x_vector,ptr_right_node,split_point+1,j);
        //insert into map
        ptr_parsing_map_->insert(std::make_pair(tuple,map_index_));
        map_index_++;
    }
}

/**
 *
 * annotate the non-terminator with the position pair i, j of a sentence p_x_vector.
 *
 * @param ptr_root
 * @param isTraining
 * @param p_x_vector
 * @return
 */
std::pair<std::string, int> Decoder::IterationGroundTruthTree(Node *ptr_root, bool isTraining,
                                                              std::vector<std::string> *p_x_vector) {
    if (NULL == ptr_root) {
        //return empty string;
        std::pair<std::string, int> pair = std::make_pair(TERMINATOR_FLAG,NEGATIVE_VALUE);
        return pair;
    }
    std::string root_str = ptr_root->GetData();
    std::pair<std::string, int>  pair_left = IterationGroundTruthTree(ptr_root->GetLeftNode(), isTraining, p_x_vector);
    if(pair_left.second == NEGATIVE_VALUE){
        //the terminator don't have left and right child.
#ifdef IF_DEBUG_
        std::cout << root_str << " ";
#endif
        p_x_vector->push_back(root_str);
        std::pair<std::string, int> pair = std::make_pair(root_str,start_index_);
        ptr_root->SetStartIndex(start_index_);
        start_index_ ++;
        return pair;
    }
    ptr_root->SetStartIndex(pair_left.second);
    std::pair<std::string, int> pair_right = IterationGroundTruthTree(ptr_root->GetRightNode(), isTraining, p_x_vector);
    if(pair_right.second == NEGATIVE_VALUE){
        pair_right = std::make_pair(NO_RIGHT_CHILD_FLAG,end_index_);
        end_index_++;
    }
    ptr_root->SetEndIndex(pair_right.second);
    //insert into a map for calculation the precision and recall.
    if(ptr_non_terminator_map_->find(root_str)!=ptr_non_terminator_map_->end()){
        CKY_Tuple tuple= std::make_pair(std::make_pair(pair_left.second,pair_right.second),root_str);
        ptr_groundtruth_map_->insert(std::make_pair(tuple,map_index_));
    }
    return pair_left;
}
/**
 * Parse PTB data and annotate the position pair for each non-terminator.
 *
 * @param str
 */
void Decoder::ParsePTB(std::string str) {
        BinaryTree *ptree = new BinaryTree(str);
        map_index_ = 0;
        //annotate the position pair for each non-terminator.
        std::pair<std::string, int> pair = IterationGroundTruthTree(ptree->GetRootNode(), true, ptree->GetXVector());
        ptree->GetRootNode()->SetEndIndex(ptree->GetXVector()->size() -1);
        //insert the start symbol ROOT to the groundtruth map.
        if(ptr_non_terminator_map_->find(ptree->GetRootNode()->GetData())!=ptr_non_terminator_map_->end()){
            CKY_Tuple tuple= std::make_pair(std::make_pair(0,ptree->GetXVector()->size()-1),ptree->GetRootNode()->GetData());
            ptr_groundtruth_map_->insert(std::make_pair(tuple,map_index_));
        }
        std::cout << std::endl;
        delete ptree;
}

/**
 *  Calculate the precision and recall denominator.
 */
void Decoder::Compare() {
    for(auto it = ptr_parsing_map_->begin(); it != ptr_parsing_map_->end(); ++it){
#ifdef IF_DEBUG_
        std::cout << (*it).first.first.first << ";"<<(*it).first.first.second <<";"<<(*it).first.second <<std::endl;
#endif
        if(ptr_groundtruth_map_->find((*it).first) != ptr_groundtruth_map_->end()){
            result_numerator_ ++;
        }
    }
    result_denominator_precision_ += ptr_parsing_map_->size();
    result_denominator_recall_ += ptr_groundtruth_map_->size();
    std::cout << line_index_ << " th line: " << "the numerrator, precsion and recall denominationator are:"
              << result_numerator_ << ", " << result_denominator_precision_ << " , " << result_denominator_recall_
              << std::endl;
}

void Decoder::Decoding(const char *test_file_name) {
    GeneratePhraseLevelRuleVector(PHRASE_LEVEL_FILE);
    ReadModel(MODEL_FILE);
    ExtractSentenceFile(test_file_name);
    GenerateNonTerminatorMapFromWeighMap();
    std::ifstream ifs(SENTENCE_FILE);
    std::ifstream ifs_test(test_file_name);
    std::string str_x;
    std::string str_test;
    while (std::getline(ifs, str_x) && std::getline(ifs_test, str_test)) {
        std::cout << "calc the "<<line_index_ <<" th sentence"<< std::endl;
        std::vector<std::string> str_vector;
        std::stringstream ss(str_x);
        std::string tmp;
        while (ss >> tmp) {
            str_vector.push_back(tmp);
        }
        InitCKY(&str_vector);
        CKY(&str_vector);
        ConstructTree(str_vector);
        ParsePTB(str_test);
        Compare();
        Clear();
        line_index_ ++;
    }
    double precision = (double)result_numerator_ / (double)result_denominator_precision_;
    double recall = (double)result_numerator_ / (double)result_denominator_recall_;
    std::cout << "precision and recall are" << precision<<" , "<<recall<<std::endl;
}