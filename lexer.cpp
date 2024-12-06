#include "lexer.h"
#include <algorithm>


int Lexer::variant_type(number num)
{
	if (std::holds_alternative<int>(num))
	{
		return 1;
	}
	else if (std::holds_alternative<double>(num))
	{
		return 2;
	}
	return 0;
}

Lexer::Lexer(std::string input_text, std::string out_filename)
	: input(input_text += " "), out_filename(out_filename), pos(0), current_state(State::START),
	table_words{
			{"end", 1}, {"if", 2}, {"then", 3}, {"else", 4}, {"for", 5},
			{"to", 6}, {"do", 7}, {"while", 8}, {"read", 9}, {"write", 10},
			{"%", 11}, {"!", 12}, {"$", 13}, {"true", 14}, {"false", 15}
	},
	table_limiters{
	  {"<", 1}, {">", 2}, {"=", 3}, {"\n", 4}, {"<>", 5},
	  {"<=", 6}, {">=", 7}, {":", 8}, {",", 9}, {"+", 10},
	  {"-", 11}, {"*", 12}, {"/", 13}, {"(", 14}, {")", 15},
	  {"#", 16}, {"or", 17}, {"and", 18}, {"not", 19}, {"as", 20}, {"\t", 21}//, {"\"",21}
	}
{
	table_words1.resize(table_words.size()+1);
	table_words1[0] = "";
	for (std::pair<std::string, int> p : table_words)
		table_words1[p.second] = p.first;

	table_limiters1.resize(table_limiters.size()+1);
	table_limiters1[0] = "";
	for (std::pair<std::string, int> p : table_limiters)
		table_limiters1[p.second] = p.first;
}

Lexer::~Lexer() = default;

std::unordered_map<std::string, int, CStringHash, CStringEqual> Lexer::get_table_identifiers()
{
	return table_identifiers;
}

std::unordered_map<Lexer::number, int> Lexer::get_table_numbers()
{
	return table_numbers;
}

Token Lexer::get_next_token()
{
	current_state = State::START;
	std::string token_value;
	bool comment_is_closed = true;
	bool possibility_hex_error = false;
	bool possibility_exp_num = false, possibility_exp_error = false;;
	bool is_hex=false;
	bool is_real = false;
	bool default_to_decimal = false;
	bool end_found = false;

	while (pos < input.size())
	{
		char current_char = input[pos];
		//std::cout << current_char << std::endl;
		switch (current_state)
		{
		case State::START:
			if (std::isalpha(current_char))
			{
				current_state = State::IDENTIFIER;
				token_value += current_char;
			}
			else if (current_char == '\n')
			{
				auto it = table_words.find(std::string(1,current_char));
				if (it != table_words.end())
				{
					return Token{ TokenType::KEYWORD,it->first , it->second };
				}
			}
			else if (current_char == '\t')
			{
				auto it = table_limiters.find(std::string(1, current_char));
				if (it != table_limiters.end())
				{
					pos++;
					return Token{ TokenType::LIMITER, it->first , it->second };
				}
			}
			else if (std::isspace(current_char))
			{
				// Пропуск пробела
			}
			else if (std::isdigit(current_char))
			{
				current_state = State::NUMBER;
				token_value += current_char;
			}
			else if (table_words.count(std::string(1, current_char)))
			{
				current_state = State::KEYWORD;
				token_value += current_char;
			}
			else if (current_char == '#')
			{
				current_state = State::COMMENT;
				comment_is_closed = false;
				token_value += current_char;
			}
			else if (table_limiters.count(std::string(1, current_char)))
			{
				current_state = State::LIMITER;
				token_value += current_char;
			}
			else
			{
				current_state = State::ERROR;
				return Token{ TokenType::UNKNOWN,"unknown symbol( " + std::string(1, current_char) + " )" , -1 };
			}
			break;

		case State::IDENTIFIER:
			if (std::isalnum(current_char))
			{
				token_value += current_char;
			}
			else
			{
				Token token;
				auto it = table_words.find((token_value));
				if (it != table_words.end())
				{
					if (table_words[token_value] == 1)
					{
						current_state = State::END;
					}
					token = Token{ TokenType::KEYWORD, it->first,it->second };
				}
				else
				{
					it = table_limiters.find(token_value);
					if (it != table_limiters.end())
					{
						token = Token{ TokenType::LIMITER, it->first , it->second };
					}
					else
					{
						if (table_identifiers.count(token_value))
						{
							it = table_identifiers.find(token_value);
							token = Token{ TokenType::IDENTIFIER, it->first,it->second };
						}
						else
						{
							table_identifiers[token_value] = counter_identifiers++;
							table_identifiers1[counter_identifiers - 1] = token_value;
							it = table_identifiers.find(token_value);
							token = Token{ TokenType::IDENTIFIER, it->first,it->second };
						}
					}
				}
				token_value.clear();
				return token;
			}
			break;
		case State::NUMBER:
			if (std::isdigit(current_char)) {
				// Накопление цифр числа
				token_value += current_char;
			}
			else if (current_char == '.') {
				if (is_real || possibility_exp_num) {
					// Ошибка, если '.' повторяется или идет после экспоненты
					current_state = State::ERROR;
					return Token{ TokenType::NUMBER, "invalid floating-point syntax", -1 };
				}
				is_real = true;
				token_value += current_char;
			}
			else if (current_char == 'e' || current_char == 'E') {
				if (possibility_exp_num) {
					possibility_exp_error = true;
				}
				possibility_exp_num = true;
				is_real = true;
				token_value += current_char;
			}
			else if (current_char == '+' || current_char == '-') {
				// Обработка знака после экспоненты
				if (possibility_exp_num && (token_value.back() == 'e' || token_value.back() == 'E')) {
					token_value += current_char;
				}
				else {
					return process_final_number_token(token_value, is_real, possibility_exp_error);
				}
			}
			else if (std::isalpha(current_char)) {
				// Обработка шестнадцатеричных букв
				char last_char = std::tolower(current_char);
				if (is_hex_letter(current_char)|| last_char == 'b' || last_char == 'o' || last_char == 'd' || last_char == 'h') {
					is_hex = true;
					if (last_char == 'h')
					{
						is_real = false;
					}
					token_value += current_char;
				}
				else {
					current_state = State::ERROR;
					return Token{ TokenType::NUMBER, "invalid character in number:2", -1 };
				}
			}
			else {
				return process_final_number_token(token_value, is_real, possibility_exp_error);
			}
			break;
		case State::COMMENT:
			if (!comment_is_closed) {
				token_value += current_char;
				if (current_char == '#' && token_value.size() != 1)
				{
					token_value.clear();
					pos++;
					return Token{ TokenType::COMMENT, "comment is closed" ,16 };
				}
				if (current_char != '#' && pos == input.size() - 1)
				{
					current_state = State::ERROR;
					token_value.clear();
					return Token{ TokenType::COMMENT, "comment is not closed", -1 };
				}
			}
			break;
		case State::LIMITER:
			if (!std::isalnum(current_char))
			{
				if (current_char != token_value[0] && !std::isspace(current_char) && token_value.size() == 1)
				{
					token_value += current_char;
				}
				else {
					auto it = table_limiters.find(token_value);
					if (it != table_limiters.end())
					{
						token_value.clear();
						return Token{ TokenType::LIMITER, it->first ,it->second };
					}
					else
					{
						pos--;
						it = table_limiters.find(token_value.substr(0,token_value.size()-1));
						if (it != table_limiters.end())
						{
							token_value.clear();
							return Token{ TokenType::LIMITER, it->first ,it->second };
						}
						else
						{
							return Token{ TokenType::LIMITER, "limiter is not found", -1 };
						}
					}
				}
			}
			else
			{
				auto it = table_limiters.find(token_value);
				if (it != table_limiters.end())
				{
					token_value.clear();
					return Token{ TokenType::LIMITER, it->first ,it->second };
				}
			}

			break;
		case State::KEYWORD:
			if (true)
			{
				auto it = table_words.find(token_value);
				if (it != table_words.end())
				{
					token_value.clear();
					return Token{ TokenType::KEYWORD,it->first,it->second };
				}
			}
			break;
		case State::ERROR:
			break;
		case State::END:
			break;
		default:
			break;
		}

		pos++;
	}

	// Проверка на отсутствие "end" после завершения цикла
	if (!end_found) {
		current_state == State::ERROR;
		end_miss = true;
		return Token{ TokenType::UNKNOWN, "missing 'end' at the end of input", -1 };
	}

	return Token();
}

void Lexer::run()
{
	Token token;
	//int i = 0;
	std::ofstream out;     
	out.open(out_filename);

	while (true) {
		token = get_next_token();
		std::cout << "\033[32m" << "Token: " << token.value << ", Type: " << token_type_to_string(token.type) << ", Number: " << token.number_in_table << ", State: " << state_to_string(current_state)<< "\033[0m";
		std::cout << std::endl;
		if (out.is_open())
		{
			write_to_output(token);
		}
		if (current_state == State::END)
			break;
		if (end_miss || current_state == State::ERROR)
		{
			std::cout << "===== ERROR =====\n";
			break;
		}
		//i++;
	}

	flush_output(out);
	out.close();
}

std::string Lexer::variant_to_string(const std::variant<int, double>& var)
{
	return std::visit([](auto&& value) -> std::string {
		return std::to_string(value);
		}, var);
}

int Lexer::parse_to_int(const std::string& str)
{
	char baseChar = std::tolower(str.back());
	std::string numberPart = str.substr(0, str.size() - 1);

	int base;
	if (baseChar == 'b') base = 2;
	else if (baseChar == 'o') base = 8;
	else if (baseChar == 'd') base = 10;
	else if (baseChar == 'h') base = 16;

	// Конвертация строки в целое число с заданной базой
	size_t idx;
	int result = std::stoi(numberPart, &idx, base);

	return result;
}

bool Lexer::is_hex_letter(char ch)
{
	return (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
}

double Lexer::parse_to_double(const std::string& str)
{
	size_t idx;
	double result = std::stod(str, &idx);
	return result;
}

//std::bitset<64> Lexer::to_binary(int number)
//{
//	return std::bitset<64>(number);
//}
//
//std::bitset<64> Lexer::to_binary(double number)
//{
//	// Преобразуем указатель на double к указателю на 64-битное целое число
//	uint64_t binaryRepresentation = *reinterpret_cast<uint64_t*>(&number);
//	// Преобразуем целое число в битовую строку
//	std::bitset<64> bits(binaryRepresentation);
//	return bits;
//}

Token Lexer::process_final_number_token(const std::string& token_value, bool is_real, bool possibility_exp_error)
{
	// Если это конец числа и суффикс не указан, подразумеваем десятичную систему
	if (is_real) {
		if (possibility_exp_error)
		{
			current_state = State::ERROR;
			return Token{ TokenType::NUMBER, "invalid exp syntax", -1 };
		}
		double num = parse_to_double(token_value);
		//std::bitset<64> num2 = to_binary(num);
		auto it = table_numbers.find(num);
		if (it != table_numbers.end())
		{
			return Token{ TokenType::NUMBER, variant_to_string(it->first), it->second };
		}
		else {
			table_numbers[num] = counter_numbers++;
			table_numbers1[counter_numbers-1] = num;
			it = table_numbers.find(num);
			if (it != table_numbers.end())
			{
				return Token{ TokenType::NUMBER, variant_to_string(it->first), it->second };
			}
		}
	}
	else {
		// Проверка на наличие суффикса
		if (token_value.empty()) {
			current_state = State::ERROR;
			return Token{ TokenType::NUMBER, "empty number", -1 };
		}

		// Обработка шестнадцатеричного числа
		char last_char = std::tolower(token_value.back());
		if (last_char == 'b' || last_char == 'o' || last_char == 'd' || last_char == 'h') 
		{
			if (last_char == 'b') {
				// Проверка на бинарное число (только 0 и 1)
				for (int i = 0; i < token_value.size() - 1; i++) {
					if (std::tolower(token_value[i]) != '0' && std::tolower(token_value[i]) != '1') {
						current_state = State::ERROR;
						std::string str = "uncorrect binary number: " + token_value;
						return Token{ TokenType::NUMBER, str, -1 };
					}
				}
			}
			else if (last_char == 'o') {
				// Проверка на восьмеричное число (только 0-7)
				for (int i = 0; i < token_value.size() - 1; i++) {
					if (std::tolower(token_value[i]) < '0' || std::tolower(token_value[i]) > '7') {
						current_state = State::ERROR;
						std::string str = "uncorrect octal number: " + token_value;
						return Token{ TokenType::NUMBER, str, -1 };
					}
				}
			}
			else if (last_char == 'd') {
				// Проверка на десятичное число (только 0-9)
				for (int i = 0; i < token_value.size() - 1; i++) {
					if (std::tolower(token_value[i]) < '0' || std::tolower(token_value[i]) > '9') {
						current_state = State::ERROR;
						std::string str = "uncorrect demical number: " + token_value;
						return Token{ TokenType::NUMBER, str, -1 };
					}
				}
			}
			else if (last_char == 'h') {
				// Проверка на шестнадцатеричное число (0-9, a-f)
				for (int i = 0; i < token_value.size() - 1; i++) {
					char ch = std::tolower(token_value[i]);
					if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))) {
						current_state = State::ERROR;
						std::string str = "uncorrect hex number: " + token_value;
						return Token{ TokenType::NUMBER, str, -1 };
					}
				}
			}

			int num = parse_to_int(token_value); // Парсинг с учетом системы счисления
			//std::bitset<64> num2 = to_binary(num);
			auto it = table_numbers.find(num);
			if (it != table_numbers.end())
			{
				return Token{ TokenType::NUMBER, variant_to_string(it->first), it->second };
			}
			else 
			{
				table_numbers[num] = counter_numbers++;
				table_numbers1[counter_numbers-1] = num;
				it = table_numbers.find(num);
				if (it != table_numbers.end())
				{
					return Token{ TokenType::NUMBER, variant_to_string(it->first), it->second };
				}
			}
		}
		else 
		{
			// Подразумеваем десятичную систему
			int num = parse_to_int(token_value + "d"); // Парсинг как десятичного
			for (int i = 0; i < token_value.size() - 1; i++)
				if (std::tolower(token_value[i]) > '9')
				{
					current_state = State::ERROR;
					std::string str= "uncorrect demical number: " + token_value;
					return Token{ TokenType::NUMBER,  str, -1 };
				}
			//std::bitset<64> num2 = to_binary(num);
			auto it = table_numbers.find(num);
			if (it != table_numbers.end())
			{
				return Token{ TokenType::NUMBER, variant_to_string(it->first), it->second };
			}
			else {
				table_numbers[num] = counter_numbers++;
				table_numbers1[counter_numbers-1] = num;
				it = table_numbers.find(num);
				if (it != table_numbers.end())
				{
					return Token{ TokenType::NUMBER, variant_to_string(it->first), it->second };
				}
			}
		}
	}
}

int token_type_to_int(TokenType type)
{
	switch (type) {
	case TokenType::KEYWORD: return 1;
	case TokenType::IDENTIFIER: return 4;
	case TokenType::NUMBER: return 3;
	case TokenType::COMMENT: return 2;
	case TokenType::LIMITER: return 2;
	case TokenType::UNKNOWN: return -1;
	default: return 0;
	}
}

void Lexer::write_to_output(Token token)
{
	output_buffer << "(" << token_type_to_int(token.type) << "," << token.number_in_table << ")\n";
}

void Lexer::flush_output(std::ofstream& out)
{
	if (out.is_open()) {
		out << output_buffer.str();
		output_buffer.str("");
		output_buffer.clear();
	}
}

std::string token_type_to_string(TokenType type)
{
	switch (type) {
	case TokenType::KEYWORD: return "KEYWORD";
	case TokenType::IDENTIFIER: return "IDENTIFIER";
	case TokenType::NUMBER: return "NUMBER";
	case TokenType::COMMENT: return "COMMENT";
	case TokenType::LIMITER: return "LIMITER";
	case TokenType::UNKNOWN: return "UNKNOWN";
	default: return "UNKNOWN_TYPE";
	}
}

std::string state_to_string(State state)
{
	switch (state) {
	case State::START: return "START";
	case State::IDENTIFIER: return "IDENTIFIER";
	case State::NUMBER: return "NUMBER";
	case State::COMMENT: return "COMMENT";
	case State::LIMITER: return "LIMITER";
	case State::KEYWORD: return "KEYWORD";
	case State::ERROR: return "ERROR";
	case State::END: return "END";
	default: return "UNKNOWN_STATE";
	}
}