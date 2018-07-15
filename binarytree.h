//
// Created by  ngs on 06/07/2018.
//

#ifndef PCFG_BINARYTREE_H
#define PCFG_BINARYTREE_H

#include "common.h"
#include "node.h"

class BinaryTree{
public:
    BinaryTree(std::string str);
    ~BinaryTree();
    void ConvertToStringVector(std::string str);
    Node * BuildBinaryTree(std::string str);
    int FindNextBracketIndex(std::string str);
    bool PrintTree(Node *ptr_node);
    Node * GetRootNode();
    std::vector<std::string> *GetXVector();
private:
    Node *ptr_root_;
    std::vector<Node *> node_vector_;
    std::vector<std::string> *ptr_str_vector_;
    std::vector<std::string> *ptr_x_vector_;
    int index_offset_;
};

#endif //PCFG_BINARYTREE_H
