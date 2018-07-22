//
// Created by  ngs on 06/07/2018.
//

#ifndef PCFG_COMMON_H
#define PCFG_COMMON_H

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <fstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <boost/functional/hash.hpp>


const char LEFT_BRACKET_CHAR = '(';
const char RIGHT_BRACKET_CHAR = ')';
const std::string SPACE_STRING = " ";
const std::string MODEL_FILE = "model.txt";
const std::string NO_RIGHT_CHILD_FLAG = "%%%";
const std::string TRAINING_DATA_SET_FILE = "ptb-binary.train";
const std::string TESTING_DATA_SET_FILE = "ptr-binary.test";
const std::string SENTENCE_FILE = "sentence.txt";
const std::string SENTENCE_TRAINING_FILE = "training_sentence.txt";
const std::string PHRASE_LEVEL_FILE = "phraselevel.txt";
const std::string NON_TERMINATOR_FILE = "non-terminator.txt";
const std::string STOP_STRING = ".";
const double NEGATIVE_VALUE = -100000;
const std::string ROOT_NODE = "ROOT";
const std::string TERMINATOR_FLAG = "TERNMINATOR";
const std::string UNSUPERVISED_MODEL_FILE = "unsupervised-model.txt";
#define IF_DEBUG___ = 1;
typedef std::pair<std::string, std::pair<std::string, std::string>> PCFG_Rule;
typedef std::vector<std::pair<PCFG_Rule, double>> Rule_Weight_Vector;
typedef std::unordered_map<PCFG_Rule, int, boost::hash<PCFG_Rule>> Rule_Map;
typedef std::unordered_map<std::string, int> Element_Map;
typedef std::unordered_map<PCFG_Rule, double, boost::hash<PCFG_Rule>> Rule_Weight_Map;
typedef std::pair<std::pair<int,int>, std::string> CKY_Tuple;
typedef std::unordered_map<CKY_Tuple, double, boost::hash<CKY_Tuple>> CKY_Score_Map;
//typedef std::pair<CKY_Tuple, double> CKY_Score_Pair;
//typedef std::pair<CKY_Score_Pair,std::pair<std::string, std::string>> MAX_CKY_Pair;
typedef std::pair<std::pair<std::string, std::string>,int> Child_Split;
//typedef std::unordered_map<MAX_CKY_Pair, int, boost::hash<MAX_CKY_Pair>> MAX_CKY_Sore_Map;
typedef std::unordered_map<CKY_Tuple, Child_Split, boost::hash<CKY_Tuple>> MAX_CKY_Sore_Map;
typedef std::unordered_map<std::string, double> Element_Expected_Map;
typedef std::pair<std::string,std::pair<int,int>> IO_Tuple;
typedef std::unordered_map<IO_Tuple, double, boost::hash<IO_Tuple>> IO_Map;
typedef std::tuple<int, int, int> Pos_Tuple; // i, k, j;
typedef std::unordered_map<PCFG_Rule, Pos_Tuple, boost::hash<PCFG_Rule>> Binary_Rule_U_Map;
typedef std::unordered_map<std::string, std::vector<PCFG_Rule> *>Symbol_Rule_Vector_Map;
typedef std::unordered_set<PCFG_Rule, boost::hash<PCFG_Rule>> PCFG_Rule_Set;

#endif //PCFG_COMMON_H
