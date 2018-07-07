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
#include <boost/functional/hash.hpp>

const char LEFT_BRACKET_CHAR = '(';
const char RIGHT_BRACKET_CHAR = ')';
const std::string SPACE_STRING = " ";
const std::string MODEL_FILE = "model.txt";
const std::string NO_RIGHT_CHILD_FLAG = "%%%";

#define IF_DEBUG_ = 1;

typedef std::pair<std::string, std::pair<std::string, std::string>> PCFG_Rule;
typedef std::unordered_map<PCFG_Rule, int, boost::hash<PCFG_Rule>> Rule_Map;
typedef std::unordered_map<std::string, int> Element_Map;
typedef std::unordered_map<PCFG_Rule, double, boost::hash<PCFG_Rule>> Rule_Weight;
typedef std::unordered_map<std::pair<int, int>, std::string, boost::hash<std::pair(int,int)>> CKY_Score;


#endif //PCFG_COMMON_H
