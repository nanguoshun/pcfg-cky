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
    void Delete(Node *ptr_node);
    Node * BuildBinaryTree(std::string str);
    int FindNextBracketIndex(std::string str);
    void PrintTree(Node *ptr_node);
    Node * GetRootNode();
private:
    Node *ptr_root_;
    std::vector<Node *> node_vector_;
};

#endif //PCFG_BINARYTREE_H
