//
// Created by  ngs on 06/07/2018.
//

#ifndef PCFG_NODE_H
#define PCFG_NODE_H

#include "common.h"
class Node{
public:
    Node(std::string value);
    std::string GetData();
    void SetLeftNode(Node *ptr_node);
    void SetRightNode(Node *ptr_node);
    Node * GetLeftNode();
    Node * GetRightNode();
private:
    std::string value_;
    int count_;
    Node *ptr_l_node_;
    Node *ptr_r_node_;
};

#endif //PCFG_NODE_H
