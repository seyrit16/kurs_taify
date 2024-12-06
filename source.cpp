#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "syntax_analyzer.h"
#include "semer.h"
#include "asm_parser.h"
#include "parser.h"

std::string read_file(const std::string& filename);
// lexer
int main() {
	std::string filename = "test";
	std::string out_filename = "otest";
	Lexer* lexer = new Lexer(read_file(filename),out_filename);
	lexer->run();
	std::cout << "\n__________________________________________\n";

	std::shared_ptr<SyntaxAnalyzer> syner = std::make_shared<SyntaxAnalyzer>(out_filename,lexer);
	syner->run();

	if (!syner->error_detected)
	{
		std::shared_ptr<Parser> parser = std::make_shared<Parser>(syner);
		parser->generate();

		std::system("gcc -c program.c -o program.o");
		std::cout << "objdump -d program.o\n";
	}
	
	delete lexer;

	return 0;
}

std::string read_file(const std::string& filename)
{
	std::ifstream file(filename); 
	if (!file.is_open()) {       
		std::cerr << "Error opening file: " << filename << std::endl;
		return "";               
	}

	std::stringstream buffer;  
	buffer << file.rdbuf();        
	return buffer.str();         
}
