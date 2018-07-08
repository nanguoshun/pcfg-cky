//
// Created by  ngs on 07/07/2018.
//

#include "decoding.h"

Decoder::Decoder() {
    ptr_rule_weight_map_ = new Rule_Weight_Map;
    ptr_cky_score_map_ = new CKY_Score_Map;
    ptr_non_terminator_map_ = new std::unordered_map<std::string, int>;
    isFirstWord_ = true;

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
    }
}


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
    if (isFirstWord_) {
        isFirstWord_ = false;
    } else {
        if (root_str != STOP_STRING) {
            (*ofs) << SPACE_STRING;
        }
    }
    (*ofs) << root_str;
}


void Decoder::InitCKY(std::vector<std::string> *ptr_x_vector) {
    int size = ptr_x_vector->size();
    for (int i = 0; i < size; ++i) {
        std::pair<int, int> pos_pair = std::make_pair(i, i);
        for (auto it = ptr_non_terminator_map_->begin(); it != ptr_non_terminator_map_->end(); ++it) {
            std::string x_str = (*it).first;
            CKY_Tuple cky_tuple = std::make_pair(pos_pair, x_str);
            double weight = 0;
            //check the weight of the rule;
            std::string x_i = (*ptr_x_vector)[i];
            PCFG_Rule rule = std::make_pair(x_str, std::make_pair(x_i, NO_RIGHT_CHILD_FLAG));
            if (ptr_rule_weight_map_->find(rule) != ptr_rule_weight_map_->end()) {
                weight = ptr_rule_weight_map_->find(rule)->second;
                //calculation in log space;
//                 weight = std::log(weight);
            }
            ptr_cky_score_map_->insert(std::make_pair(cky_tuple, weight));
        }
    }
}

/**
 * CKY main body.
 *
 * @param ptr_x_vector
 */
void Decoder::CKY(std::vector<std::string> *ptr_x_vector) {
    int size = ptr_x_vector->size();
    for (int l = 1; l < size; ++l) {
        for (int i = 0; i < size - l - 1; ++i) {
            int j = i + l;
            for (auto it = ptr_non_terminator_map_->begin(); it != ptr_non_terminator_map_->end(); ++it) {
                std::string x_str = (*it).first;
                CKY_Tuple cky_tuple = std::make_pair(std::make_pair(i,j),x_str);
                Rule_Weight_Vector rule_vector;
                GetRuleWeight(x_str, rule_vector);
                double max_score = CalcMaxRule(i, j, x_str, rule_vector, ptr_x_vector);
                ptr_cky_score_map_->insert(std::make_pair(cky_tuple, max_score));
            }
        }
    }
}

/*
 *  The first choice of CKY, put the rule and weight of X->YZ to a vector.
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

double Decoder::CalcMaxRule(int i, int j, const std::string &x_str, const Rule_Weight_Vector &rule_vector,std::vector<std::string> *ptr_x_vector) {
    double max_score = 0;
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
            double cky_score = weight_x_yz * score_i_s_y * score_s_j_z;
            if (cky_score > max_score) {
                max_score = cky_score;
                cky_tuple_y_max = cky_tuple_y;
                cky_tuple_z_max = cky_tuple_z;
            }
        }
    }
#ifdef IF_DEBUG_
    std::cout << "the max score for " << (*ptr_x_vector)[i] <<","<<(*ptr_x_vector)[j]<<","<<x_str<<" is: "<< max_score<< ". the left and right childs are: "<<cky_tuple_y_max.second<<","<<cky_tuple_z_max.second<<std::endl;
#endif
    return max_score;
}

/**
 *
 * @param ptr_vector
 * @param ptr_CKY_score
 */
/*
void Decoder::InitCKY(std::vector<std::string> *ptr_x_vector, CKY_Tuple *ptr_CKY_score) {
   int size = ptr_x_vector->size();
   int weight_id = 0;
   for(auto it = ptr_rule_weight_map_->begin(); it != ptr_rule_weight_map_->end(); ++it) {
       std::string tag1 = (*it).first.first;
       for (int i=0; i<ptr_x_vector->size(); ++i) {
           //create row
           std::vector<Point> * ptr_score_vector = new std::vector<Point>;
           (*ptr_CKY_score)[weight_id].push_back(*ptr_score_vector);
           for (int j= i; j<; ++ittt) {
               Point point;
               point.x = i;
               point.y = j;
               point.weight = 0;
               if(i==j){
                   std::string tag2 = (*ptr_x_vector)[i];
                   PCFG_Rule rule = std::make_pair(tag1, std::make_pair(tag2, NO_RIGHT_CHILD_FLAG));
                   if (ptr_rule_weight_map_->find(rule) != ptr_rule_weight_map_->end()) {
                       double weight = ptr_rule_weight_map_->find(rule)->second;
                       point.weight = weight;
                   }
               }
               //create each point in each row.
               (*ptr_CKY_score)[weight_id][i].push_back(point);
               j++;
           }
       }
     weight_id++;
   }
}*/

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
        isFirstWord_ = true;
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
        /*
        for(int i=0; i<ptr_rule_weight_map_->size(); ++i){
            std::vector<std::vector<Point>> *pscore_matrix = new std::vector<std::vector<Point>>;
            score.push_back(*pscore_matrix);
        }
        InitCKY(&str_vector, &score);
        std::cout << std::endl;*/
    }
}