#pragma once
#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <unordered_map>

#include "lexer.h"
#include "node.h"

struct TableIdentifierItem
{
	std::string id;
	bool descrid;
	NodeType type;
	unsigned int addr;
};

class SemanticAnalyzer
{
public: 
	SemanticAnalyzer(Lexer* lexer);
	std::unordered_map<std::string, TableIdentifierItem> TI;

	void process_declared(std::shared_ptr<Node> node);
	void check_declared(std::shared_ptr<Node> node);
	void check_type_matching(std::shared_ptr<Node> node);
	void check_type_matching_operation(std::shared_ptr<Node> node);
	void check_type_matching_assignment(std::shared_ptr<Node> node);
	//подходит для узла to, while, if
	void check_type_matching_condition(std::shared_ptr<Node> node);
	NodeType get_node_type_operation(std::shared_ptr<Node> node);
private:
	Lexer* lexer;
};

#endif // !SEMANTIC_ANALYZER_H


