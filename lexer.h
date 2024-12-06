#pragma once
#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <unordered_map>
#include <variant>
#include <iostream>
#include <fstream>
#include <cctype>
#include <cstring>
#include <string_view>
#include <bitset>
#include <cmath>
#include <sstream>

// Хеш-функция для const char*
struct CStringHash {
	std::size_t operator()(const std::string& s) const noexcept {
		return std::hash<std::string>()(s);
	}
};

// Компаратор для const char* (сравнивает строки по содержимому)
struct CStringEqual {
	bool operator()(const std::string& lhs, const std::string& rhs) const noexcept {
		return lhs == rhs;
	}
};

enum class State {
	START,
	IDENTIFIER,
	NUMBER,
	COMMENT,
	LIMITER,
	KEYWORD,
	ERROR,
	END
};


enum class TokenType {
	KEYWORD,
	IDENTIFIER,
	NUMBER,
	COMMENT,
	LIMITER,
	END,
	UNKNOWN
};

struct Token {
	TokenType type;
	std::string value;
	int number_in_table;
};

// Функция для преобразования значений enum в строку
int token_type_to_int(TokenType type);
std::string token_type_to_string(TokenType type);
std::string state_to_string(State state);

class Lexer
{
public:
	using number = std::variant<int, double>;
	//std::holds_alternative<int>(value) - проверка типов
	//std::get<int>(value) - получение числа с данным типом, если такое тип совпадает
	
	//возвращает если 1 - int, 2 - double, 3 - ни один из них
	int variant_type(number num); 

	Lexer(std::string input_text,std::string out_filename);
	~Lexer();

	std::unordered_map<std::string, int, CStringHash, CStringEqual> get_table_identifiers();
	std::unordered_map<Lexer::number, int> get_table_numbers();

	Token get_next_token();

	void run();
	std::string out_filename;
	std::unordered_map<std::string, int, CStringHash, CStringEqual> table_words;
	std::unordered_map<std::string, int, CStringHash, CStringEqual> table_limiters;
	std::vector<std::string> table_words1;
	std::vector<std::string> table_limiters1;
	std::unordered_map<std::string, int, CStringHash, CStringEqual> table_identifiers;
	std::unordered_map<int, std::string> table_identifiers1;
	//std::unordered_map<std::bitset<64>, int> table_numbers;
	//std::unordered_map<int, std::bitset<64>> table_numbers1;
	std::unordered_map<number, int> table_numbers;
	std::unordered_map<int, number> table_numbers1;
	std::string variant_to_string(const std::variant<int, double>& var);
private:
	// преобразование числа в виде строки в целочисленный тип
	int parse_to_int(const std::string& str);
	// проверка, что буква входит в диапазон символов для шестнадцатиричной системы
	bool is_hex_letter(char ch);
	// преобразование числа в виде строки в тип с плавющей точкой
	double parse_to_double(const std::string& str);

	Token process_final_number_token(const std::string& token_value,bool is_real, bool possibility_exp_error);
	//запись токенов в output в виде (n,k)
	void write_to_output(Token token);
	void flush_output(std::ofstream& out);

	std::string input;
	std::ostringstream output_buffer;
	size_t pos;
	State current_state;

	int counter_identifiers = 1;
	int counter_numbers = 1;
	bool end_miss=false;

	//std::unordered_set<char> letters = {
	//	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	//	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	//	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	//	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
	//};

	//std::unordered_set<char> numbers = { '0','1','2','3','4','5','6','7','8','9' };
};

#endif // !LEXER_H



