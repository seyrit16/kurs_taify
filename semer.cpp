
/// НЕ НУЖЕН

#include "semer.h"

Semer::Semer(const std::string& filename, Lexer* lexer)
	:lexeme_file(filename), lexer(lexer)
{
    parse_lexems();
}

void Semer::run()
{
    for (int i = 0; i < lexems.size(); i++)
    {
        //std::cout << lexems[i].first << " " << lexems[i].second << std::endl;
        ProcessType pt = defining_process(lexems[i]);
        if (pt == ProcessType::DESCRIPTION)
        {
            int temp = process_description(i);
            if (!(temp < 0))
            {
                i = temp;
            }
            else
            {
                // описать ошибку
            }
        }
        else if (pt == ProcessType::MB_UNDESCRIPTION)
        {
            int temp = process_mb_undescription(i);
            if(temp == -1)
            {
                auto it = lexer->table_identifiers1.find(lexems[i].second);
                if (it != lexer->table_identifiers1.end())
                {
                    std::cout << "Error: Undeclared identifier: ( " << it->first << " ) " << it->second << std::endl;
                    record_error(SemanticErrorType::UNDECLARED_IDENTIFIER, it->second);
                    continue;
                }
                else
                {
                    std::cout << "Error: unknown identifier" << std::endl;
                }
            }
            else
            {
                // описать другие
            }
        }
        else if (pt == ProcessType::ASSIGNMENT)
        {
            int temp = process_assignment(i);
        }
    }

    for (std::pair<int, int> p : symbol_table)
    {
        std::cout<<"|1|" << p.first << " " << p.second << std::endl;
    }
}

void Semer::print_errors() const
{

}

ProcessType Semer::defining_process(std::pair<int, int> pair)
{
    if (pair.first == 1 && (pair.second == 11 || pair.second == 12 || pair.second == 13))
    {
        return ProcessType::DESCRIPTION;
    }
    else if (pair.first == 4)
    {
        return ProcessType::MB_UNDESCRIPTION;
    }
    else if (pair.first == 2 && pair.second == 20)
    {
        return ProcessType::ASSIGNMENT;
    }
    else
    {
        return ProcessType::IS_NOT_PROCESS;
    }

    return ProcessType();
}

bool Semer::parse_lexems()
{
    if (lexeme_file.is_open())
    {
        std::string line;
        while (std::getline(lexeme_file, line))
        {
            std::stringstream ss(line);
            std::pair<int, int> p;
            char c;
            ss >> c >> p.first >> c >> p.second;
            lexems.push_back(p);
        }
        lexeme_file.close();
    }
    else
    {
        std::cerr << "Failed to open lexems file" << std::endl;
        return false;
    }
    return true;
}

int Semer::process_mb_undescription(int index)
{
    std::pair<int, int> lexem = lexems[index];
    if (symbol_table.count(lexem.second))
    {
        return index;
    }
    else
    {
        return -1;
    }
}

int Semer::process_description(int indexStart)
{
    std::pair<int, int> dataType = lexems[indexStart];
    for (int i = indexStart + 1; i < lexems.size(); i++)
    {
        if (lexems[i].first == 4)
        {
            if (symbol_table.count(lexems[i].second))
            {
                auto it = lexer->table_identifiers1.find(lexems[i].second);
                if (it != lexer->table_identifiers1.end())
                {
                    std::cout << "Error: duplicate declaration: ( " << it->first << " ) " << it->second << std::endl;
                    record_error(SemanticErrorType::DUPLICATE_DECLARATION, it->second);
                    continue;
                }
                else
                {
                    std::cout << "Error: unknown identifier" << std::endl;
                }
            }
            else
            {
                symbol_table[lexems[i].second] = dataType.second;
            }
        }
        else if (lexems[i].first == 2 && lexems[i].second == 9)
        {
            continue;
        }
        else
        {
            return i;
        }
    }
    return 0;
}

int Semer::process_assignment(int index)
{
    if (lexems[index - 1].first != 4)
    {
        record_error(SemanticErrorType::INCORRECT_ASSIGNMENT,  "not identificator");
    }
    return 0;
}

bool Semer::process_expression(Token& token)
{
	return false;
}

bool Semer::process_operator(Token& token)
{
	return false;
}

void Semer::record_error(SemanticErrorType error_type, const std::string& details)
{
    switch (error_type) {
    case SemanticErrorType::DUPLICATE_DECLARATION:
        errors.push_back("Duplicate declaration: " + details);
        break;
    case SemanticErrorType::UNDECLARED_IDENTIFIER:
        errors.push_back("Undeclared identifier: " + details);
        break;
    case SemanticErrorType::TYPE_MISMATCH:
        errors.push_back("Type mismatch: " + details);
        break;
    case SemanticErrorType::INVALID_EXPRESSION:
        errors.push_back("Invalid expression: " + details);
        break;
    case SemanticErrorType::INVALID_OPERATOR:
        errors.push_back("Invalid operator usage: " + details);
        break;
    }
}

