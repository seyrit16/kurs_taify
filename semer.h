
// �� �����

#pragma once
#ifndef SEMER_H
#define SEMER_H

#include <fstream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>

#include "lexer.h"

enum class SemanticErrorType
{
    DUPLICATE_DECLARATION, // ����������� ����������
    UNDECLARED_IDENTIFIER, // ������������� �������������
    TYPE_MISMATCH, // �������������� ����
    INCORRECT_ASSIGNMENT, //������������ ������������
    INVALID_EXPRESSION, // ������������ ���������
    INVALID_OPERATOR, // ������������ ��������
};

enum class ProcessType
{
    IS_NOT_PROCESS,
    MB_UNDESCRIPTION,
    DESCRIPTION,
    ASSIGNMENT,
    EXPRESSION,
    OPERATOR,
};

class Semer
{
public:
    Semer(const std::string& filename, Lexer* lexer);
    void run();
    void print_errors() const;

private:
    Lexer* lexer;
    std::ifstream lexeme_file;
    std::unordered_map<int, int> symbol_table; // ������������� -> ���
    std::vector<std::pair<int, int>> lexems;
    std::vector<std::string> errors;

    ProcessType defining_process(std::pair<int,int> pair);
    bool parse_lexems();

    int process_mb_undescription(int index);
    int process_description(int indexStart);
    int process_assignment(int index);
    bool process_expression(Token& token);
    bool process_operator(Token& token);
    void record_error(SemanticErrorType error_type, const std::string& details);
};

#endif // !SEMER_H