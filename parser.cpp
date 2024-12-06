#include "parser.h"

Parser::Parser(std::shared_ptr<SyntaxAnalyzer> syner)
	:syner(syner), skip_after_else(false), is_child(false)
{
	node = syner->program_node;
}

void Parser::generate()
{
	std::ofstream out("program.c");
	out << "#include <stdio.h>\n#include <stdbool.h>\n";
	out << "int main()\n{\n";

	out << generate_code();

	out << "\nreturn 0;\n}";
	out.close();
}

std::string Parser::generate_code()
{
	std::ostringstream oss;

	if (node->token.first == 2)
	{
		oss << generate_code_1(node);
		return oss.str();
	}
	if (node->children.size() > 0)
	{
		for (std::shared_ptr<Node> node : node->children) {
			is_child = true;
			oss << generate_code_1(node);
		}
	}
	else
	{
		oss << generate_code_1(node);
	}

	return oss.str();
}

std::string Parser::generate_code_1(std::shared_ptr<Node> node)
{
	std::ostringstream oss;
	switch (node->type)
	{
	case NodeType::INT:
	case NodeType::DOUBLE:
	case NodeType::BOOL:
	{

		is_child = false;
		oss << generate_declaration(node);
		break;
	}
	case NodeType::ASSIGNMENT:
	{
		is_child = false;
		oss << generate_assigment(node);
		break;
	}
	case NodeType::IDENTIFIER:
	{
		if (!is_child)
		{
			oss << get_name_identifier(node);
		}
		is_child = false;
		break;
	}
	case NodeType::NUMBER:
	{
		if (!is_child)
		{
			oss << get_number(node);
		}
		is_child = false;
		break;
	}
	case NodeType::NOT:
	{
		is_child = false;
		oss << generate_not(node);
		break;
	}
	case NodeType::EXPRESSION:
	{
		is_child = false;
		if(node->token.first == 4)
		{
			oss << get_name_identifier(node);
		}
		else if(node->token.first == 1)
		{
			if(node->token.second == 14)
			{
				oss << "true";
			}
			else if (node->token.second == 15)
			{
				oss << "false";
			}
		}
		else
		{
			oss << generate_expression(node);
		}
		break;
	}
	case NodeType::READ:
	{
		is_child = false;
		oss << generate_read(node);
		break;
	}
	case NodeType::WRITE:
	{
		is_child = false;
		oss << generate_write(node);
		break;
	}
	case NodeType::IF:
	{
		is_child = false;
		oss << generate_if(node);
		break;
	}
	case NodeType::LOOP_FOR:
	{
		is_child = false;
		oss << generate_for(node);
		break;
	}
	case NodeType::LOOP_WHILE:
	{
		is_child = false;
		oss << generate_while(node);
		break;
	}
	default:
		is_child = false;
		break;
	}
	is_child = false;
	return oss.str();
}

std::string Parser::generate_declaration(std::shared_ptr<Node> node)
{
	std::ostringstream oss;
	VariableType type;
	switch (node->type)
	{
	case(NodeType::INT):
		oss << "int ";
		type = VariableType::INT;
		break;
	case(NodeType::DOUBLE):
		oss << "double ";
		type = VariableType::DOUBLE;
		break;
	case(NodeType::BOOL):
		oss << "bool ";
		type = VariableType::BOOL;
		break;
	default:
		break;
	}

	for (int i = 0; i < node->children.size(); i++)
	{
		oss << generate_code_1(node->children[i]);
		variables_type[node->children[i]->token.second] = type;
		if (i + 1 != node->children.size()) oss << ", ";
	}

	oss << ";\n";

	return oss.str();
}

std::string Parser::generate_assigment(std::shared_ptr<Node> node)
{
	std::ostringstream oss;
	std::string l = generate_code_1(node->children[0]);
	std::string r = generate_code_1(node->children[1]);

	oss << l << " = " << r << ";\n";

	return oss.str();
}

std::string Parser::generate_not(std::shared_ptr<Node> node)
{
	std::ostringstream oss;
	oss << "!" << generate_code_1(node->children[0]);

	return oss.str();
}

std::string Parser::generate_expression(std::shared_ptr<Node> node)
{
	std::ostringstream oss;

	std::string l = generate_code_1(node->children[0]);
	std::string r = generate_code_1(node->children[1]);

	oss << l << " "<< node->description <<" " << r;

	return oss.str();
}

std::string Parser::generate_read(std::shared_ptr<Node> node)
{
	std::ostringstream oss;
	int count = node->children.size();
	oss << "scanf(\"";
	for (int i = 0; i < node->children.size(); i++)
	{
		oss << "%" << (variables_type[node->children[i]->token.second] == VariableType::DOUBLE ? "lf" : "d");
		if (i + 1 != node->children.size()) oss << " ";
	}
	oss << "\", ";
	for (int i = 0; i < node->children.size(); i++)
	{
		oss << "&" << get_name_identifier(node->children[i]);
		if (i + 1 != node->children.size()) oss << ", ";
	}
	oss << ");\n";

	return oss.str();
}

std::string Parser::generate_write(std::shared_ptr<Node> node)
{
	std::ostringstream oss;
	int count = node->children.size();
	oss << "printf(\"";
	for (int i = 0; i < node->children.size(); i++)
	{
		oss << "%" << (variables_type[node->children[i]->token.second] == VariableType::DOUBLE ? "f" : "d");
		if (i + 1 != node->children.size()) oss << " ";
	}
	oss << "\", ";
	for (int i = 0; i < node->children.size(); i++)
	{
		oss << generate_code_1(node->children[i]);
		if (i + 1 != node->children.size()) oss << ", ";
	}
	oss << ");\n";

	return oss.str();
}

std::string Parser::generate_if(std::shared_ptr<Node> node)
{
	std::ostringstream oss;
	oss << "if(" << generate_code_1(node->children[0]) << ")\n{\n";

	if (node->children[1]->type == NodeType::THEN)
	{
		std::shared_ptr<Node> then = node->children[1];
		if (then->children[0]->type == NodeType::BLOCK)
		{
			oss << generate_block(then->children[0]);
		}
		else
		{
			oss << generate_code_1(then->children[0]);
		}
	}

	oss << "}\n";

	if (node->children.size() > 2)
	{
		if (node->children[2]->type == NodeType::ELSE)
		{
			oss << "else\n{\n";
			std::shared_ptr<Node> else_ = node->children[2];
			if (else_->children[0]->type == NodeType::BLOCK)
			{
				oss << generate_block(else_->children[0]);
			}
			else
			{
				oss << generate_code_1(else_->children[0]);
			}
			oss << "}\n";
		}
	}

	return oss.str();
}

std::string Parser::generate_block(std::shared_ptr<Node> node)
{
	std::ostringstream oss;

	for (int i = 0; i < node->children.size(); i++)
	{
		std::shared_ptr<Node> item = node->children[i];
		for (std::shared_ptr<Node> ii: item->children)
		{
			oss << generate_code_1(ii);
		}
	}

	return oss.str();
}

std::string Parser::generate_for(std::shared_ptr<Node> node)
{
	std::ostringstream oss;

	oss << "for(" << generate_code_1(node->children[0]) << " "
		<< generate_code_1(node->children[1]->children[0]) << "; )\n{\n";

	std::shared_ptr<Node> do_ = node->children[2];
	
	if (do_->children[0]->type == NodeType::BLOCK)
	{
		oss << generate_block(do_->children[0]);
	}
	else
	{
		oss << generate_code_1(do_->children[0]);
	}

	oss << "}\n";

	return oss.str();
}

std::string Parser::generate_while(std::shared_ptr<Node> node)
{
	std::ostringstream oss;

	oss << "while(" << generate_code_1(node->children[0]) << ")\n{\n";


	std::shared_ptr<Node> do_ = node->children[1];

	if (do_->children[0]->type == NodeType::BLOCK)
	{
		oss << generate_block(do_->children[0]);
	}
	else
	{
		oss << generate_code_1(do_->children[0]);
	}

	oss << "}\n";

	return oss.str();
}

std::string Parser::get_name_identifier(std::shared_ptr<Node> node)
{
	return syner->lexer->table_identifiers1[node->token.second];
}

std::string Parser::get_number(std::shared_ptr<Node> node)
{
	Lexer::number var = syner->lexer->table_numbers1.find(node->token.second)->second;
	if (std::holds_alternative<int>(var)) {
		int value = std::get<int>(var);
		std::ostringstream oss;
		oss << value;
		std::string str = oss.str();
		return str;
	}
	else if (std::holds_alternative<double>(var)) {
		double value = std::get<double>(var);
		std::ostringstream oss;
		oss << value;
		std::string str = oss.str();
		return str;
	}

	return std::string();
}

