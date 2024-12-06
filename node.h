#pragma once

#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <memory>
#include <vector>

enum class NodeType {
    VARIABLE_DECLARATION, INT, DOUBLE, BOOL,
    IDENTIFIER, NUMBER,
    ASSIGNMENT,
    CONDITION,
    LOOP_FOR, LOOP_WHILE, TO, DO,
    READ, WRITE,
    EXPRESSION,
    BLOCK, BLOCK_ITEM,
    PROGRAM,
    NOT,
    IF, THEN, ELSE,
    BRACKET_OPEN, BRACKET_CLOSE,
    END
};

struct Node {
public:
    NodeType type;
    NodeType typeOperation; // тип только для операций, для проверки в семантике
    std::string description;
    std::vector<std::shared_ptr<Node>> children;
    std::pair<int, int> token;

    Node(NodeType type, std::pair<int, int> token,std::string description = "") : type(type), token(token), description(description) {}

    void addChild(std::shared_ptr<Node> child) {
        children.push_back(child);
    }

    static void printTree(const std::shared_ptr<Node>& node, const std::string& prefix = "", bool isLast = true) {
        // Печатаем текущий узел
        std::cout << prefix;
        if (isLast) {
            std::cout << "--- ";
        }
        else {
            std::cout << "--- ";
        }
        std::cout << "\033[32m" << node->description <<" ("<<node->token.first<<", "<< node->token.second<<") " <<"\033[0m"<< std::endl;

        // Рекурсивный вывод для всех детей
        for (size_t i = 0; i < node->children.size(); ++i) {
            printTree(node->children[i], prefix + (isLast ? "    " : "|-- "), i == node->children.size() - 1);
        }
    }
};

#endif // !Node


