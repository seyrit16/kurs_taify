#pragma once
#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.h"
#include "semantic_analyzer.h"
#include "node.h"

class SyntaxAnalyzer
{
public:
	SyntaxAnalyzer(std::string fileName, Lexer* lexer);
	void run();

	void createTree();
	std::shared_ptr<Node> parse_program();
	std::shared_ptr<Node> parse_assignment();
	std::shared_ptr<Node> parse_statement();
	std::shared_ptr<Node> parse_expression();
	std::shared_ptr<Node> parse_expression_with_precedence(int min_precedence = 0);
	std::shared_ptr<Node> parse_operand(NodeType typeOperation = NodeType::IDENTIFIER);
	std::shared_ptr<Node> parse_loop_for();
	std::shared_ptr<Node> parse_loop_while();
	std::shared_ptr<Node> parse_condition();
	std::shared_ptr<Node> parse_block();
	std::shared_ptr<Node> parse_block_item();
	std::shared_ptr<Node> parse_read();
	std::shared_ptr<Node> parse_write();
	std::shared_ptr<Node> parse_variable_declaration();
	int get_precedence(int op);
	int getIndentLevel();

	bool parseLexem();

	std::unordered_map<int, int> bool_identifier;

	Lexer* lexer = nullptr;
	std::shared_ptr<Node> program_node = nullptr;
	SemanticAnalyzer* semer = nullptr;
	std::ifstream lexeme_file;
	std::vector<std::pair<int, int>> lexems;
	int pos = 0;
	int current_indent_level = 0;
	bool error_detected =false;
};

#endif
