//
// Created by  ngs on 06/07/2018.
//
#include "node.h"

Node::Node(std::string value) {
    value_ = value;
    ptr_l_node_ = ptr_r_node_ = NULL;
}

Node* Node::GetLeftNode() {
    return ptr_l_node_;
}

Node* Node::GetRightNode() {
    return ptr_r_node_;
}

void Node::SetLeftNode(Node *ptr_node) {
    ptr_l_node_ = ptr_node;
}

void Node::SetRightNode(Node *ptr_node) {
    ptr_r_node_ = ptr_node;
}

std::string Node::GetData() {
    return value_;
}