
// ме мсфем

#pragma once
#ifndef ASM_PARSER
#define ASM_PARSER

#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include "node.h"
#include "syntax_analyzer.h"

class ASMParser
{
public:
	ASMParser(std::shared_ptr<SyntaxAnalyzer> syner);
	void generate_program();

private:
	std::string add_data_assembly();
	std::string generate_assembly(std::shared_ptr<Node> root_node);
	void generate_assembly_1(std::shared_ptr<Node> root_node, std::ostringstream& code);
	std::string generate_declaration(std::shared_ptr<Node> node);
	std::string generate_expression(std::shared_ptr<Node> node);
	std::string get_expression_operand(std::shared_ptr<Node> node, std::string operation);
	std::shared_ptr<Node> findAssignmentNodes(const std::shared_ptr<Node>& node);
	std::string get_number(std::shared_ptr<Node> node);
	std::shared_ptr<SyntaxAnalyzer> syner = nullptr;
	std::shared_ptr<Node> program_node = nullptr;
	std::unordered_map<int, int> variable_offsets;
	std::unordered_map<int, int> variable_types; // 4-int, 1-bool, 8-double
	std::unordered_map<int, int> constant_types; // 4-int, 1-bool, 8-double
	int current_offset;
	bool is_expression_double;
};

#endif // !ASM_PARSER


