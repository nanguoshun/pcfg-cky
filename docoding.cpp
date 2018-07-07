//
// Created by  ngs on 07/07/2018.
//

#include "decoding.h"

Decoder::Decoder() {
    ptr_rule_weight_map_ = new Rule_Weight;
    ptr_cky_score_map_ = new CKY_Map;
    isFirstWord_ = true;
}

Decoder::~Decoder() {
    delete ptr_rule_weight_map_;
    delete ptr_cky_score_map_;
}

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


std::string Decoder::IterationExtract(Node *ptr_root, std::ofstream *ofs) {
    if (NULL == ptr_root) {
        //return empty string;
        return std::string();
    }
    std::string root_str = ptr_root->GetData();
    std::string str_left = IterationExtract(ptr_root->GetLeftNode(),ofs);
    if(str_left.empty()){
#ifdef IF_DEBUG__
        std::cout << root_str << " ";
#endif
        SaveSentenceFile(root_str,ofs);
        return root_str;
    }
    std::string str_right = IterationExtract(ptr_root->GetRightNode(),ofs);
    return root_str;
}

void Decoder::SaveSentenceFile(std::string root_str, std::ofstream *ofs) {
    if(isFirstWord_){
        isFirstWord_ = false;
    } else{
        if(root_str != STOP_STRING){
            (*ofs) << SPACE_STRING;
        }
    }
    (*ofs) << root_str;
}

void Decoder::Decoding(const char *test_file_name) {
    ReadModel(MODEL_FILE);
    ExtractSentenceFile(test_file_name);
    std::ifstream ifs(SENTENCE_FILE);
    std::string str;
    while(std::getline(ifs,str)){
        std::vector<std::string> str_vector;
        std::stringstream ss(str);
        std::string tmp;
        while(ss >> tmp){
            str_vector.push_back(tmp);
        }
        InitCKY(&str_vector);

        /*
        for(int i=0; i<ptr_rule_weight_map_->size(); ++i){
            std::vector<std::vector<Point>> *pscore_matrix = new std::vector<std::vector<Point>>;
            score.push_back(*pscore_matrix);
        }
        InitCKY(&str_vector, &score);
        std::cout << std::endl;*/
    }
}

void Decoder::InitCKY(std::vector<std::string> *ptr_x_vector) {
    int size = ptr_x_vector->size();
    for (int i = 0; i < size; ++i) {
        std::pair<int, int> pos_pair = std::make_pair(i, i);
        for (auto it = ptr_rule_weight_map_->begin(); it != ptr_rule_weight_map_->end(); ++it) {
            std::string tag1 = (*it).first.first;
            CKY_Score cky_score = std::make_pair(pos_pair,tag1);
            double weight = 0;
            //check the weight of the rule;
            std::string tag2 = (*ptr_x_vector)[i];
            PCFG_Rule rule = std::make_pair(tag1, std::make_pair(tag2, NO_RIGHT_CHILD_FLAG));
            if (ptr_rule_weight_map_->find(rule) != ptr_rule_weight_map_->end()) {
                 weight = ptr_rule_weight_map_->find(rule)->second;
            }
            ptr_cky_score_map_->insert(std::make_pair(cky_score,weight));
        }
    }
}

void Decoder::CKY(std::vector<std::string> *ptr_x_vector) {
    int size = ptr_x_vector->size();
    for(int l = 1; l < size; ++l){
        for(int i = 0; i < size-1; ++i){
            int j = i + l;
            for (auto it = ptr_rule_weight_map_->begin(); it != ptr_rule_weight_map_->end(); ++it){

            }
        }
    }
}

/**
 *
 * @param ptr_vector
 * @param ptr_CKY_score
 */
 /*
void Decoder::InitCKY(std::vector<std::string> *ptr_x_vector, CKY_Score *ptr_CKY_score) {
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
    for(std::vector<BinaryTree *>::iterator it = tree_vector_.begin(); it!=tree_vector_.end(); ++it){
        Node *ptr_root = (*it)->GetRootNode();
        IterationExtract(ptr_root, &ofs);
        ofs << "\n";
        isFirstWord_ = true;
    }
}
