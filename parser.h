#pragma once
#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <sstream>
#include <memory>
#include <unordered_map>
#include "node.h"
#include "syntax_analyzer.h"

enum VariableType
{
	INT,
	DOUBLE,
	BOOL
};

class Parser
{
public:
	Parser(std::shared_ptr<SyntaxAnalyzer> syner);

	void generate();
	std::string generate_code();
	std::string generate_code_1(std::shared_ptr<Node> node);

	std::string generate_declaration(std::shared_ptr<Node> node);
	std::string generate_assigment(std::shared_ptr<Node> node);
	std::string generate_not(std::shared_ptr<Node> node);
	std::string generate_expression(std::shared_ptr<Node> node);
	std::string generate_read(std::shared_ptr<Node> node);
	std::string generate_write(std::shared_ptr<Node> node);
	std::string generate_if(std::shared_ptr<Node> node);
	std::string generate_block(std::shared_ptr<Node> node);
	std::string generate_for(std::shared_ptr<Node> node);
	std::string generate_while(std::shared_ptr<Node> node);

	std::string get_name_identifier(std::shared_ptr<Node> node);
	std::string get_number(std::shared_ptr<Node> node);

	std::shared_ptr<SyntaxAnalyzer> syner;
	std::shared_ptr<Node> node;

	std::unordered_map<int, VariableType> variables_type;
	bool skip_after_else;
	bool is_child;
};

#endif // !PARSER_H


