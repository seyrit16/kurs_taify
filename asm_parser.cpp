
// НЕ НУЖЕН

#include "asm_parser.h"
#include <variant>

ASMParser::ASMParser(std::shared_ptr<SyntaxAnalyzer> syner)
	:syner(syner),current_offset(0),is_expression_double(false)
{
	program_node = syner->program_node;
}

void ASMParser::generate_program()
{
	std::ofstream out("program.asm");

	std::string data_assembly = add_data_assembly();
	out << data_assembly << "\n";
	out << "section .text\n";
	out << "global _start\n";
	out << "_start:\n";
	out << "mov rbp, rsp\n\n"; // Инициализация базового указателя

	std::shared_ptr<Node> assigment_node = findAssignmentNodes(program_node);
	//Node::printTree(assigment_node, "", false);
	// Генерация кода для AST
	out<<generate_assembly(program_node);

	// Эпилог программы
	out << "\nmov rax, 60\n"; // syscall для выхода
	out << "xor rdi, rdi\n"; // Код возврата 0
	out << "syscall\n";

	// Закрытие файла
	out.close();
}

std::string ASMParser::add_data_assembly()
{
	std::ostringstream oss;
	oss << "section .data\n";

	for (const std::pair<int, Lexer::number>& pair: syner->lexer->table_numbers1)
	{
		const Lexer::number& num = pair.second;
		if (std::holds_alternative<int>(num)) {
			int value = std::get<int>(num);
			oss << "constant" << pair.first << " dd " << value << "\n";
			constant_types[pair.first] = 4;
		}
		else if (std::holds_alternative<double>(num)) {
			double value = std::get<double>(num);
			oss << "constant" << pair.first << " dq " << value << "\n";
			constant_types[pair.first] = 8;
		}
	}

	return oss.str();
}

std::string ASMParser::generate_assembly(std::shared_ptr<Node> root_node)
{
	std::ostringstream code;
	if (root_node->token.first == 2)
	{
		generate_assembly_1(root_node, code);
		return code.str();
	}
	if (root_node->children.size() > 0)
	{
		for (std::shared_ptr<Node> node : root_node->children) {
			generate_assembly_1(node, code);
		}
	}
	else
	{
		generate_assembly_1(root_node, code);
	}
	return code.str();
}

void ASMParser::generate_assembly_1(std::shared_ptr<Node> root_node, std::ostringstream& code)
{
	int temp_offset = 0;
	switch (root_node->type)
	{
	case NodeType::INT:
	case NodeType::DOUBLE:
	case NodeType::BOOL:
	{
		code << "; | declaration process |\n";
		std::string dec_str = generate_declaration(root_node);
		code << dec_str;
		break;
	}
	case NodeType::ASSIGNMENT:
	{
		code << "; | assigment process |\n";
		// Генерация кода для правой части выражения
		std::string rhs = generate_assembly(root_node->children[1]);
		code << rhs;
		int lhs = root_node->children[0]->token.second;
		// Если переменная еще не известна, выделяем под нее место
		if (variable_offsets.find(lhs) == variable_offsets.end()) {
			variable_offsets[lhs] = current_offset;
			current_offset += 4; // Для упрощения: по 4 байта на переменную
		}
		int offset = variable_types[lhs];
		std::string suf = offset <= 4 ? " dword " : " qword ";
		std::string reg_type = offset <= 4 ? "eax" : "rax";
		std::string mov = "mov";
		if (is_expression_double)
		{
			suf = " ";
			mov += "sd";
			reg_type = "xmm0";
		}
		code << mov << suf << "[rbp-" << variable_offsets[lhs] << "], " << reg_type << "; assigment\n\n";
		is_expression_double = false;
		break;
	}
	case NodeType::IDENTIFIER:
	{
		int num = root_node->token.second;
		if (variable_types[num] <= 4)
		{
			code << "mov eax, [rbp-" << variable_offsets[num] << "]\n";
		}
		else
		{
			code << "mov rax, [rbp-" << variable_offsets[num] << "]\n";
		}
		break;
	}
	case NodeType::NUMBER:
	{
		// Числовой литерал
		Lexer::number num = syner->lexer->table_numbers1[root_node->token.second];
		if (std::holds_alternative<int>(num)) {
			int value = std::get<int>(num);
			code << "mov eax, [constant" << root_node->token.second << "]; number ("<<value<<")\n";
		}
		else if (std::holds_alternative<double>(num)) {
			double value = std::get<double>(num);
			code << "mov rax, [constant" << root_node->token.second << "]; number (" << value << ")\n";
		}
		break;
	}
	case NodeType::NOT:
	{
		std::string rhs = generate_assembly(root_node->children[0]);
		code << rhs;
		code << "xor eax, 1; not\n";
		break;
	}
	case NodeType::EXPRESSION:
	{
		std::string expr_str = generate_expression(root_node);
		code << expr_str;
		break;
	}
	default:
		break;
	}
}

std::string ASMParser::generate_declaration(std::shared_ptr<Node> node)
{
	std::ostringstream declaration_code;
	int temp_offset = 0; // Размер переменной

	switch (node->type) {
	case NodeType::INT:
		declaration_code << "mov eax, 0; declaration(int)\n";
		temp_offset = 4;
		break;
	case NodeType::DOUBLE:
		declaration_code << "mov rax, 0; declaration(double)\n";
		temp_offset = 8;
		break;
	case NodeType::BOOL:
		declaration_code << "mov eax, 0; declaration(bool)\n";
		temp_offset = 1; // Для простоты выравниваем до 4 байт
		break;
	default:
		return std::string();
	}

	for (std::shared_ptr<Node> child : node->children) {
		current_offset += temp_offset; // Увеличиваем смещение
		variable_offsets[child->token.second] = current_offset;

		// Генерация ассемблерного кода для инициализации
		if (node->type == NodeType::INT) {
			variable_types[child->token.second] = 4;
			declaration_code << "mov dword [rbp-" << current_offset << "], eax; declaration(int,bool)\n"; // 4 байта
		}
		else if (node->type == NodeType::BOOL)
		{
			variable_types[child->token.second] = 1;
			declaration_code << "mov dword [rbp-" << current_offset << "], eax; declaration(int,bool)\n";
		}
		else if (node->type == NodeType::DOUBLE) {
			variable_types[child->token.second] = 8;
			declaration_code << "mov qword [rbp-" << current_offset << "], rax; declaration(double)\n"; // 8 байт
		}
	}
	declaration_code << "\n";
	return declaration_code.str();
}

std::string ASMParser::generate_expression(std::shared_ptr<Node> node)
{
	if (node->token.first == 1)
	{
		std::cout << "";
		if (node->token.second == 14)
		{
			return "mov eax, 1; expression (true)\n";
		}
		if (node->token.second == 15)
		{
			return "mov eax, 0; expression (false)\n";
		}
	}
	else if(node->token.first == 2)
	{
		if (node->token.second == 10)
		{
			std::ostringstream oss;
			oss << get_expression_operand(node->children[0], "mov");
			oss << get_expression_operand(node->children[1], "add");
			return oss.str();
		}
		else if (node->token.second == 11)
		{
			std::ostringstream oss;
			oss << get_expression_operand(node->children[0], "mov");
			oss << get_expression_operand(node->children[1], "sub");
			return oss.str();
		}
		else if (node->token.second == 12)
		{
			std::ostringstream oss;
			oss << get_expression_operand(node->children[0], "mov");
			oss << get_expression_operand(node->children[1], "mul");
			return oss.str();
		}
		else if (node->token.second == 13)
		{
			std::ostringstream oss;
			oss << get_expression_operand(node->children[0], "mov");
			oss << get_expression_operand(node->children[1], "div");
			return oss.str();
		}
	}
	return std::string();
}

std::string ASMParser::get_expression_operand(std::shared_ptr<Node> node, std::string operation)
{
	std::ostringstream oss;
	if (node->token.first == 4)
	{
		std::string reg_type = "eax";
		if (variable_types[node->token.second] > 4)
		{
			reg_type = "xmm0";
			operation += "sd";
			is_expression_double = true;
		}
		if (operation == "mul" && !is_expression_double)
			operation = "imul";
		if (operation == "div" && !is_expression_double)
		{
			oss << "cdq\n";
			oss << "idiv dword [rbp-" << variable_offsets[node->token.second] << "]; div operand\n";
			return oss.str();
		}
		oss << operation <<" "<< reg_type <<", [rbp - " << variable_offsets[node->token.second] << "]; load first operand\n";
		std::cout << oss.str();
	}
	else if (node->token.first == 3)
	{
		std::string reg_type = "eax";
		if(constant_types[node->token.second] > 4)
		{
			reg_type = "xmm0";
			operation += "sd";
			is_expression_double = true;
		}
		if (operation == "mul" && !is_expression_double)
			operation = "imul";
		if (operation == "div" && !is_expression_double)
		{
			oss << "cdq\n";
			oss << "idiv dword [constant" << node->token.second << "]; div constant";
			return oss.str();
		}
		oss << operation << " " << reg_type << ", [constant" << node->token.second << "]; load second operand\n";
		std::cout << oss.str();
	}
	else if (node->type == NodeType::EXPRESSION)
	{
		oss << generate_expression(node);
		std::cout << oss.str();
	}
	return oss.str();
}

std::shared_ptr<Node> ASMParser::findAssignmentNodes(const std::shared_ptr<Node>& node)
{
	if (!node)
	{
		return nullptr;
	}

	// Проверяем текущий узел
	if (node->type == NodeType::ASSIGNMENT) {
		return node;  // Возвращаем первый найденный узел ASSIGNMENT
	}

	// Рекурсивно обходим дочерние узлы
	for (const auto& child : node->children) {
		std::shared_ptr<Node> result = findAssignmentNodes(child);
		if (result) {
			return result;  // Возвращаем найденный узел из рекурсии
		}
	}

	return nullptr;  // Если узел ASSIGNMENT не найден, возвращаем nullptr
}

std::string ASMParser::get_number(std::shared_ptr<Node> node)
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
