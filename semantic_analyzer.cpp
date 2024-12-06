#include "semantic_analyzer.h"
#include <stdexcept>

SemanticAnalyzer::SemanticAnalyzer(Lexer* lexer)
	:lexer(lexer)
{
}

void SemanticAnalyzer::process_declared(std::shared_ptr<Node> node)
{
	if (!node)
	{
		throw std::runtime_error("Error (semantic (declared)): empty node");
	}

	if (!(node->type == NodeType::INT || node->type == NodeType::DOUBLE || node->type == NodeType::BOOL))
	{
		throw std::runtime_error("Error (semantic (declared)): uncorrect type (" + node->description +")");
	}

	NodeType type = node->type;
	for (std::shared_ptr<Node> node : node->children)
	{
		if (node->token.first != 4)
		{
			throw std::runtime_error("Error (semantic (declared)): declared object is not an identifier");
		}
		auto it = lexer->table_identifiers1.find(node->token.second);
		if (it != lexer->table_identifiers1.end())
		{
			auto it1 = TI.find(it->second);
			if (it1 != TI.end())
			{
				if (it1->second.descrid)
				{
					throw std::runtime_error("Error (semantic (declared)): duplicate declaration: ( " + it1->first + " ) ");
				}
			}

			TableIdentifierItem tii;
			tii.id = it->second;
			tii.descrid = true;
			tii.type = type;

			TI[tii.id] = tii;
		}
		else
		{
			throw std::runtime_error("Ñritical error (lexer, semantic (declared)): not found identifier");
		}
	}
}

void SemanticAnalyzer::check_declared(std::shared_ptr<Node> node)
{
	if (!node)
	{
		throw std::runtime_error("Error (semantic (check declared)): empty node");
	}

	if (!(node->type == NodeType::IDENTIFIER))
	{
		throw std::runtime_error("Error (semantic (check declared)): is not identifier (" + node->description + ")");
	}

	auto it = lexer->table_identifiers1.find(node->token.second);
	if (it != lexer->table_identifiers1.end())
	{
		auto it1 = TI.find(it->second);
		if (it1 == TI.end())
		{
			throw std::runtime_error("Error (semantic (check declared)): identifier is not declared (" + node->description + ")");
		}
	}
	else
	{
		throw std::runtime_error("Error (semantic (check declared)): identifier is not declared (" + node->description + ")");
	}
}

void SemanticAnalyzer::check_type_matching(std::shared_ptr<Node> node)
{
	if (!node)
	{
		throw std::runtime_error("Error (semantic (type matching)): empty node");
	}

	if (node->type == NodeType::ASSIGNMENT)
	{
		check_type_matching_assignment(node);
	}
	else if (node->type == NodeType::TO || node->type == NodeType::LOOP_WHILE || node->type == NodeType::IF)
	{
		check_type_matching_condition(node);
	}
	else if(node->type == NodeType::EXPRESSION || node->type == NodeType::NOT)
	{
		check_type_matching_operation(node);
	}
}

void SemanticAnalyzer::check_type_matching_operation(std::shared_ptr<Node> node)
{
	if (!node)
	{
		throw std::runtime_error("Error (semantic (type matching operation)): empty node");
	}

	if (node->type == NodeType::NOT)
	{
		if (node->children.size() > 1)
		{
			throw std::runtime_error("Error (semantic (type matching operation)): error using the unary operator (not)");
		}
		std::shared_ptr<Node> nodeC = node->children[0];
		if (nodeC->type == NodeType::EXPRESSION)
		{
			if (nodeC->typeOperation != NodeType::BOOL)
			{
				throw std::runtime_error("Error (semantic (type matching operation)): error using the unary operator (not) operand type is not $");
			}
			return;
		}
		if (nodeC->typeOperation != NodeType::BOOL)
		{
			throw std::runtime_error("Error (semantic (type matching operation)): error using the unary operator (not) operand type is not $");
		}
		return;
	}
	else
	{
		if (node->type == NodeType::EXPRESSION)
		{
			if (node->children.size() != 2)
			{
				throw std::runtime_error("Error (semantic (type matching operation)): error using the binary operator");
			}
			std::shared_ptr<Node> node_left = node->children[0];
			NodeType type_left = get_node_type_operation(node_left);
			std::shared_ptr<Node> node_right = node->children[1];
			NodeType type_right = get_node_type_operation(node_right);
			
			if (type_left != type_right)
			{
				throw std::runtime_error("Error (semantic (type matching operation)): error using the binary operator (type left operator != type right operator) (" + node_left->description + " " + node->description + " " + node_right->description + ")");
			}
			else
			{
				node->typeOperation = type_left;
			}
		}
		else
		{
			throw std::runtime_error("Error (semantic (type matching operation)): unknown operation");
		}
	}
}

void SemanticAnalyzer::check_type_matching_assignment(std::shared_ptr<Node> node)
{
	if (!node)
	{
		throw std::runtime_error("Error (semantic (type matching assignment)): empty node");
	}

	NodeType main_type = node->children[0]->type;
	if (node->children.size() != 2)
	{
		throw std::runtime_error("Error (semantic (type matching assignment)): error using the assignment operator");
	}
	std::shared_ptr<Node> node_left = node->children[0];
	NodeType type_left = get_node_type_operation(node_left);
	std::shared_ptr<Node> node_right = node->children[1];
	NodeType type_right = get_node_type_operation(node_right);

	if (type_left != type_right)
	{
		throw std::runtime_error("Error (semantic (type matching assignment)): error using the assignment operator (type left operator != type right operator) (" + node_left->description + " " + node->description + " " + node_right->description+")");
	}
}

void SemanticAnalyzer::check_type_matching_condition(std::shared_ptr<Node> node)
{
	if (!node)
	{
		throw std::runtime_error("Error (semantic (type matching condition)): empty node");
	}

	std::shared_ptr<Node> nodeC = node->children[0];
	NodeType type_operation = get_node_type_operation(nodeC);
	if (type_operation == NodeType::BOOL)
	{
		return;
	}

	throw std::runtime_error("Error (semantic (type matching condition)): returned type is not &");
}

NodeType SemanticAnalyzer::get_node_type_operation(std::shared_ptr<Node> node)
{
	NodeType type_operation;
	if (node->type == NodeType::IDENTIFIER)
	{
		auto it = lexer->table_identifiers1.find(node->token.second);
		if (it != lexer->table_identifiers1.end())
		{
			auto it1 = TI.find(it->second);
			if (it1 != TI.end())
			{
				type_operation = it1->second.type;
			}
		}
	}
	else if (node->type == NodeType::NUMBER)
	{
		auto it = lexer->table_numbers1.find(node->token.second);
		if (it != lexer->table_numbers1.end())
		{
			int type = lexer->variant_type(it->second);
			if (type == 1)
			{
				type_operation = NodeType::INT;
			}
			if (type == 2)
			{
				type_operation = NodeType::DOUBLE;
			}
		}
	}
	else if (node->type == NodeType::EXPRESSION)
	{
		if (node->token.first == 1)
		{
			if (node->token.second == 14 || node->token.second == 15)
			{
				type_operation = NodeType::BOOL;
			}
			else
			{
				type_operation = node->typeOperation;
			}
		}
		if (node->token.first == 2)
		{
			type_operation = node->typeOperation;
			if (node->token.second == 13)
			{
				type_operation = NodeType::DOUBLE;
			}
			else if (node->token.second >= 1 && node->token.second <= 7)
			{
				type_operation = NodeType::BOOL;
			}
		}
		else if (node->token.first == 4)
		{
			auto it = lexer->table_identifiers1.find(node->token.second);
			if (it != lexer->table_identifiers1.end())
			{
				auto it1 = TI.find(it->second);
				if (it1 != TI.end())
				{
					type_operation = it1->second.type;
				}
			}
		}
	}
	else if (node->type == NodeType::NOT)
	{
		type_operation = NodeType::BOOL;
	}

	return type_operation;
}
