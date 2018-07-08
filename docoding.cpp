//
// Created by  ngs on 07/07/2018.
//

#include "decoding.h"

Decoder::Decoder() {
    ptr_rule_weight_map_ = new Rule_Weight_Map;
    ptr_cky_score_map_ = new CKY_Score_Map;
    ptr_non_terminator_map_ = new std::unordered_map<std::string, int>;
}

Decoder::~Decoder() {
    delete ptr_rule_weight_map_;
    delete ptr_cky_score_map_;
}

void Decoder::GenerateNonTerminatorMap() {
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
 * read the weight of each rule from a file generated in training phase.
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
#ifdef IF_DEBUG_
        std::cout << tag1 << " "<< tag2 <<" "<<tag3<<": "<<weight<<std::endl;
#endif
    }
}

/**
 * Extract in an iteration manner and then write to a file to faciliate parsing.
 *
 * @param ptr_root
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
            for (auto it = ptr_non_terminator_map_->begin(); it != ptr_non_terminator_map_->end(); ++it) {
                std::string x_str = (*it).first;
                CKY_Tuple cky_tuple = std::make_pair(std::make_pair(i, j), x_str);
                Rule_Weight_Vector rule_vector;
                GetRuleWeight(x_str, rule_vector);
                double max_score = CalcMaxRule(i, j, x_str, rule_vector, ptr_x_vector);
                ptr_cky_score_map_->insert(std::make_pair(cky_tuple, max_score));
            }
        }
    }
}

/**
 * Put the rule and weight of X->YZ to a vector, which is the first choice of CKY when calculating the maximum score.
 * @param x_str : the rule X.
 * @param rule_vector: The rules that start with the rule X.
 */
void Decoder::GetRuleWeight(std::string &x_str, Rule_Weight_Vector &rule_vector) {
    for (auto itt = ptr_rule_weight_map_->begin(); itt != ptr_rule_weight_map_->end(); ++itt) {
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
    for (auto itt = rule_vector.begin(); itt != rule_vector.end(); ++itt) {
        std::string y_str = (*itt).first.second.first;
        std::string z_str = (*itt).first.second.second;
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
            }
        }
    }
#ifdef IF_DEBUG_
    std::cout << "the max score for " << (*ptr_x_vector)[i] << "," << (*ptr_x_vector)[j] << "," << x_str << " is: "
              << max_score << ". the left and right childs are: " << cky_tuple_y_max.second << ","
              << cky_tuple_z_max.second << std::endl;
#endif
    cky_tuple_y_max
    return max_score;
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

void Decoder::Decoding(const char *test_file_name) {
    ReadModel(MODEL_FILE);
    ExtractSentenceFile(test_file_name);
    GenerateNonTerminatorMap();
    std::ifstream ifs(SENTENCE_FILE);
    std::string str;
    while (std::getline(ifs, str)) {
        std::vector<std::string> str_vector;
        std::stringstream ss(str);
        std::string tmp;
        while (ss >> tmp) {
            str_vector.push_back(tmp);
        }
        InitCKY(&str_vector);
        CKY(&str_vector);
    }
}