//
// Created by  ngs on 07/07/2018.
//

#ifndef PCFG_DECODING_H
#define PCFG_DECODING_H

#include "common.h"
#include "binarytree.h"
#include "node.h"

class Decoder{
public:
    Decoder();
    ~Decoder();
    void ExtractSentenceFile(const char *file_name);
    void SaveSentenceFile(std::string str,std::ofstream *ofs);
    std::string IterationExtract(Node *ptr_root, std::ofstream *ofs);
    void Decoding(const char *file_name);
    void ReadModel(std::string model_file);
    void GenerateNonTerminatorMapFromWeighMap();
    void GeneratePhraseLevelRuleVector(const std::string file_name);
    void InitCKY(std::vector<std::string> *ptr_x_vector);
    void CKY(std::vector<std::string> *ptr_x_vector);
    void GetRuleWeight(std::string &x_str, Rule_Weight_Vector &rule_vector);
    double CalcMaxRule(int i, int j, const std::string &x_str,const Rule_Weight_Vector &rule_vector,std::vector<std::string> *ptr_x_vector);
    void InsertMaxScoreMap(int i, int j, const std::string &x_str, CKY_Tuple &cky_tuple_y_max, CKY_Tuple &cky_tuple_z_max, int split_point);
    void Clear();
    void ConstructTree(const std::vector<std::string> &x_vector);
    void IterationTree(const std::vector<std::string> &x_vector, Node *pnode, int i, int j);
    std::pair<std::string, int> IterationGroundTruthTree(Node *ptr_root, bool isTraining,
                                                         std::vector<std::string> *p_x_vector);
    void ParsePTB(std::string str);
    void Compare();

private:
    Rule_Weight_Map *ptr_rule_weight_map_;
    Rule_Weight_Map *ptr_binary_rule_weight_map_; //like X->YZ, and YZ are non-terminators.
    std::unordered_map<std::string, int> *ptr_non_terminator_map_;
    CKY_Score_Map *ptr_cky_score_map_;
    std::vector<BinaryTree *> tree_vector_;
    MAX_CKY_Sore_Map *ptr_max_cky_score_map_;
    std::vector<Node *> node_vector_;
    int start_index_;
    int end_index_;
    int map_index_; //actually it is meaningless.
    std::unordered_map<CKY_Tuple, int, boost::hash<CKY_Tuple>> *ptr_parsing_map_;
    std::unordered_map<CKY_Tuple, int, boost::hash<CKY_Tuple>> *ptr_groundtruth_map_;
    int result_numerator_;
    int result_denominator_precision_;
    int result_denominator_recall_;
    int line_index_;
    std::unordered_map<std::string, int> *ptr_phrase_level_rule_map_;
};

#endif //PCFG_DECODING_H
