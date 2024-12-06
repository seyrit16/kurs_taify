#include "syntax_analyzer.h"
#include <stdexcept>

SyntaxAnalyzer::SyntaxAnalyzer(std::string fileName, Lexer* lexer)
    :lexeme_file(fileName), lexer(lexer)
{
    if (!parseLexem())
        throw std::runtime_error("Error read file with lexems");

    semer = new SemanticAnalyzer(lexer);
    //for (std::pair<int, int> p : lexems)
    //    std::cout << p.first << " " << p.second<<std::endl;
}

void SyntaxAnalyzer::run()
{
    try
    {
        program_node = parse_program();
        Node::printTree(program_node, "", false);
    }
    catch (const std::exception& e)
    {
        error_detected = true;
        std::cerr << "\033[31m" << "Error: " << e.what() << "\033[0m" << std::endl;
    }
}

void SyntaxAnalyzer::createTree()
{

}

std::shared_ptr<Node> SyntaxAnalyzer::parse_program()
{
    std::shared_ptr<Node> root = std::make_shared<Node>(NodeType::PROGRAM, std::make_pair(0, 0),"Programm");

    while (pos < lexems.size())
    {
        std::shared_ptr<Node> child = parse_statement();
        if(child)
        {
            root->addChild(child);
            if (child->type == NodeType::END)
            {
                break;
            }
        }
    }
    if (!root)
    {
        throw std::runtime_error("Error (syntax (program_node)): unknown error on create program_node");
    }
    return root;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_statement()
{
    std::pair<int, int> token = lexems[pos];
    if (token.first == 1)
    {
        if (token.second == 2)//if
        {
            return parse_condition();
        }
        if (token.second == 3)
        {
            throw std::runtime_error("Error (syntax (if)): found \"then\" outside condition");
        }
        if (token.second == 4)
        {
            throw std::runtime_error("Error (syntax (if)): found \"else\" outside condition");
        }
        else if (token.second == 5)//for
        {
            return parse_loop_for();
        }
        else if (token.second == 8)//while
        {
            return parse_loop_while();
        }
        else if (token.second == 6) //to
        {
            throw std::runtime_error("Error (syntax (for)): found \"to\" outside loop");
        }
        else if (token.second == 7) //do
        {
            throw std::runtime_error("Error (syntax (loop)): found \"do\" outside loop");
        }
        else if (token.second == 9)//read
        {
            return parse_read();
        }
        else if (token.second == 10)//write
        {
            return parse_write();
        }
        else if (token.second >= 11 && token.second <= 13) // int double bool
        {
            return parse_variable_declaration();
        }
        else if (token.second == 1)//end
        {
            return std::make_shared<Node>(NodeType::END, lexems[pos], "end");
        }
        else if (token.second == 14 || token.second == 15)
        {
            std::shared_ptr<Node> operandNode = std::make_shared<Node>(NodeType::EXPRESSION, lexems[pos++], "Expression (t, f)");
            operandNode->typeOperation = NodeType::BOOL;
            return operandNode;
        }
        else
        {
            throw std::runtime_error("Error (syntax (keyword)): unknown keyword");
        }
        //pos++;
    }
    else if (token.first == 2)
    {
        if (token.second == 20) //as
        {
            return parse_assignment();
        }
        if (token.second == 1 ||
            token.second == 2 || token.second == 3 || token.second == 5 ||
            token.second == 6 || token.second == 7 || token.second == 10 ||
            token.second == 11 || token.second == 12 || token.second == 13 ||
            token.second == 17 || token.second == 18 || token.second == 19)
        {
            return parse_expression();
        }
        else if (token.second == 21)
        {
            throw std::runtime_error("Error (syntax (block)): unknown block(no loop < no condition)");
        }
        else if (token.second == 9) // ,
        {
            throw std::runtime_error("Error (syntax (keyword)): invalid position for \",\"");
        }
        else
        {
            pos++;
        }
    }
    else if (token.first == 3 || token.first == 4)
    {
        return parse_expression();
    }
    else
    {
        pos++;
        throw std::runtime_error("Error syntax: Unexpected token");
    }
    return nullptr;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_expression()
{
    return parse_expression_with_precedence();
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_expression_with_precedence(int min_precedence)
{
    std::shared_ptr<Node> leftNode = parse_operand();
    if (lexems[pos].second == 20)
    {
        return parse_assignment();
    }
    if (leftNode)
    {
        if (lexems[pos] == std::pair<int, int>(2, 14))
        {
            std::string s = leftNode->type == NodeType::IDENTIFIER ? "identifier" : "number";
            throw std::runtime_error("Error (syntax (" + s + ")): " + "unknown structure \"" + s + "(\"");
        }
    }
    while (lexems[pos].first == 2 && get_precedence(lexems[pos].second) >= min_precedence) {
        int current_op = lexems[pos].second;
        int precedence = get_precedence(current_op);
        pos++;

        if (current_op == 19) {  // not
            std::shared_ptr<Node> operatorNode = std::make_shared<Node>(NodeType::NOT, lexems[pos - 1], "Not");
            std::shared_ptr<Node> rightNode = parse_expression_with_precedence(precedence);
            if (rightNode->token.first == 4)
            {
                if (bool_identifier.count(rightNode->token.second))
                {
                    rightNode->typeOperation = NodeType::BOOL;
                }
            }
            if (!rightNode)
            {
                throw std::runtime_error("Error (syntax (expression)): lost right operand");
            }
            operatorNode->addChild(rightNode);
            leftNode = operatorNode;
            leftNode->typeOperation = NodeType::BOOL;
            semer->check_type_matching(leftNode);
            continue;
        }

        std::shared_ptr<Node> operatorNode = std::make_shared<Node>(NodeType::EXPRESSION, lexems[pos-1], lexer->table_limiters1.at(current_op));


        if (current_op == 1 || current_op == 2 || current_op == 6 || current_op == 7
            || current_op == 5 || current_op == 17 || current_op == 18)
        {
            operatorNode->typeOperation = NodeType::BOOL;
        }

        std::shared_ptr<Node> rightNode = parse_expression_with_precedence(precedence + 1);
        if (rightNode->token.first == 4)
        {
            if (bool_identifier.count(rightNode->token.second))
            {
                rightNode->typeOperation = NodeType::BOOL;
            }
        }
        if (!rightNode)
        {
            throw std::runtime_error("Error (syntax (expression)): lost right operand");
        }

        if (!leftNode)
        {
            throw std::runtime_error("Error (syntax (expression)): lost left operand");
        }
        operatorNode->addChild(leftNode);
        operatorNode->addChild(rightNode);

        leftNode = operatorNode;
        if (leftNode->children.size() != 0)
        {
            semer->check_type_matching(leftNode);
        }
        if (rightNode->children.size() != 0)
        {
            semer->check_type_matching(rightNode);
        }
    }
    return leftNode;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_operand(NodeType typeOperation)
{
    std::shared_ptr<Node> operandNode;
    if (lexems[pos].first == 4)
    {
        auto it = lexer->table_identifiers1.find(lexems[pos].second);
        operandNode = std::make_shared<Node>(NodeType::IDENTIFIER, lexems[pos++], "Operand(" + it->second + ")");
        if (!(typeOperation == NodeType::INT || typeOperation == NodeType::DOUBLE || typeOperation == NodeType::BOOL))
        {
            semer->check_declared(operandNode);
        }
        else
        {
            operandNode->typeOperation = typeOperation;
        }
    }
    else if (lexems[pos].first == 3)
    {
        operandNode = std::make_shared<Node>(NodeType::NUMBER, lexems[pos++], "Operand");
    }
    else if (lexems[pos].first == 1 && (lexems[pos].second == 14 || lexems[pos].second == 15))
    {
        operandNode = std::make_shared<Node>(NodeType::EXPRESSION, lexems[pos++], "Expression (t, f)");
        operandNode->typeOperation = NodeType::BOOL;
    }
    else if (lexems[pos].second)
    {
        return std::shared_ptr<Node>();
    }
    else
    {
        throw std::runtime_error("Error (syntax (operand)): unknown operand");
    }

    if (!operandNode)
    {
        throw std::runtime_error("Error (syntax (operand)): unknown operand");
    }
    return operandNode;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_loop_for()
{
    std::shared_ptr<Node> forNode = std::make_shared<Node>(NodeType::LOOP_FOR, lexems[pos++], "for");
    std::shared_ptr<Node> expressionNode = parse_statement();
    if (expressionNode)
    {
        forNode->addChild(expressionNode);
    }
    else
    {
        throw std::runtime_error("Error (syntax (for)): lost expression assigment after \"for\"");
    }

    if (lexems[pos].first == 1 && lexems[pos].second == 6)// to
    {
        std::shared_ptr<Node> toNode = std::make_shared<Node>(NodeType::TO, lexems[pos++], "to");
        std::shared_ptr<Node> nextNode  = parse_statement();
        if (!nextNode)
        {
            throw std::runtime_error("Error (syntax (for)): lost expression condition after \"to\"");
        }
        toNode->addChild(nextNode);
        semer->check_type_matching(toNode);
        forNode->addChild(toNode);
    }
    else
    {
        throw std::runtime_error("Error (syntax (for)): not found \"to\" block");
    }

    if (lexems[pos].first == 1 && lexems[pos].second == 7) // do
    {
        std::shared_ptr<Node> doNode = std::make_shared<Node>(NodeType::DO, lexems[pos++], "do");
        if (lexems[pos].first == 2 && lexems[pos].second == 21)
        {
            current_indent_level++;
            std::shared_ptr<Node> blockNode = parse_block();
            doNode->addChild(blockNode);
        }
        else
        {
            std::shared_ptr<Node> statementNode = parse_statement();
            doNode->addChild(statementNode);
        }
        forNode->addChild(doNode);
    }
    else
    {
        throw std::runtime_error("Error (syntax (for)): not found \"do\" block");
    }

    return forNode;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_loop_while()
{
    std::shared_ptr<Node> whileNode = std::make_shared<Node>(NodeType::LOOP_WHILE, lexems[pos++], "while");
    std::shared_ptr<Node> expressionNode = parse_statement();
    if (expressionNode)
    {
        whileNode->addChild(expressionNode);
        semer->check_type_matching(whileNode);
    }
    else
    {
        throw std::runtime_error("Error (syntax (while)): lost expression assigment after \"while\"");
    }

    if (lexems[pos].first == 1 && lexems[pos].second == 7) // do
    {
        std::shared_ptr<Node> doNode = std::make_shared<Node>(NodeType::DO, lexems[pos++], "do");
        if (lexems[pos].first == 2 && lexems[pos].second == 21)
        {
            current_indent_level++;
            std::shared_ptr<Node> blockNode = parse_block();
            doNode->addChild(blockNode);
        }
        else
        {
            std::shared_ptr<Node> statementNode = parse_statement();
            doNode->addChild(statementNode);
        }
        whileNode->addChild(doNode);
    }
    else
    {
        throw std::runtime_error("Error (syntax (while)): not found \"do\" block");
    }

    return whileNode;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_condition()
{
    std::shared_ptr<Node> ifNode = std::make_shared<Node>(NodeType::IF, lexems[pos++], "if");
    std::shared_ptr<Node> conditionNode = parse_expression();
    ifNode->addChild(conditionNode);
    semer->check_type_matching(ifNode);

    bool isThen = lexems[pos].first == 1 && lexems[pos].second == 3;
    if (isThen) // then
    {
        std::shared_ptr<Node> thenNode = std::make_shared<Node>(NodeType::THEN, lexems[pos++], "then");
        if (lexems[pos].first == 2 && lexems[pos].second == 21)
        {
            current_indent_level++;
            std::shared_ptr<Node> blockNode = parse_block();
            thenNode->addChild(blockNode);
        }
        else
        {
            std::shared_ptr<Node> expressionNode = parse_statement();
            thenNode->addChild(expressionNode);
        }
        ifNode->addChild(thenNode);
    }
    else
    {
        throw std::runtime_error("Error (syntax (if)): not found \"then\" block");
    }
    
    if (lexems[pos].first == 1 && lexems[pos].second == 4) //else
    {
        if (!isThen) throw std::runtime_error("Error (syntax (if)): else block was found without specifying the then block");

        std::shared_ptr<Node> elseNode = std::make_shared<Node>(NodeType::ELSE, lexems[pos++], "else");
        bool isAfterBlock = false;
        if (lexems[pos].first == 2 && lexems[pos].second == 21)
        {
            current_indent_level++;
            isAfterBlock = true;
            std::shared_ptr<Node> blockNode = parse_block();
            elseNode->addChild(blockNode);
        }
        else
        {
            std::shared_ptr<Node> expressionNode = parse_statement();
            elseNode->addChild(expressionNode);
        }
        ifNode->addChild(elseNode);
        if(isAfterBlock) pos--;
    }


    return ifNode;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_block()
{
    std::shared_ptr<Node> blockNode = std::make_shared<Node>(NodeType::BLOCK, lexems[pos], "Block");

    while (pos < lexems.size() && lexems[pos] == std::pair<int, int>(2, 21)) {
        int indent_level = getIndentLevel();

        if (indent_level < current_indent_level)
        {
            break;
        }

        if (indent_level > current_indent_level)
        {
            std::shared_ptr<Node> nestedBlock = parse_block();
            blockNode->addChild(nestedBlock);
            continue;
        }

        std::shared_ptr<Node> block_item = parse_block_item();
        if (block_item)
        {
            blockNode->addChild(block_item);
        }
    }

    if (!blockNode)
    {
        throw std::runtime_error("Error (syntax (block)): error to create block");
    }
    current_indent_level--;
    return blockNode;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_block_item()
{
    std::shared_ptr<Node> block_item = std::make_shared<Node>(NodeType::BLOCK_ITEM, lexems[pos-1], "Block_Item");

    std::shared_ptr<Node> statementNode = parse_statement(); // parse first statement
    if (statementNode)
    {
        block_item->addChild(statementNode);
    }

    if (lexems[pos] == std::pair<int, int>(2, 8))
    {
        while (pos < lexems.size() && lexems[pos] == std::pair<int, int>(2, 8) 
            && lexems[pos + 1] != std::pair<int, int>(2, 21)
            )
        {
            if (lexems[pos] == std::pair<int, int>(2, 8))
            {
                pos++;
            }
            std::shared_ptr<Node> newStatementNode = parse_statement();
            if (newStatementNode)
            {
                block_item->addChild(newStatementNode);
            }
            else
            {
                throw std::runtime_error("Error (syntax (block)): error to create block");
            }
        }
    }
    return block_item;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_read()
{
    std::shared_ptr<Node> read = std::make_shared<Node>(NodeType::READ, lexems[pos++], "read");

    if (lexems[pos++] == std::pair<int, int>(2, 14))
    {
        std::shared_ptr<Node> firstIdNode = parse_operand();
        if (firstIdNode) 
        {
            read->addChild(firstIdNode);
            while (pos < lexems.size() && lexems[pos] == std::pair<int, int>(2, 9))
            {
                pos++;
                std::shared_ptr<Node> nextIdNode = parse_operand();
                if (!nextIdNode)
                {
                    throw std::runtime_error("Error (syntax (read)): lost operand");
                }
                read->addChild(nextIdNode);
            }
            if (lexems[pos++] != std::pair<int, int>(2, 15))
            {
                throw std::runtime_error("Error (syntax (read)): not found close bracket");
            }
        }
        else
        {
            throw std::runtime_error("Error (syntax (read)): lost operand");
        }
    }
    else
    {
        throw std::runtime_error("Error (syntax (read)): not found open bracket");
    }
    return read;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_write()
{
    std::shared_ptr<Node> write = std::make_shared<Node>(NodeType::WRITE, lexems[pos++], "write");

    if (lexems[pos++] == std::pair<int, int>(2, 14))
    {
        std::shared_ptr<Node> firstIdNode = parse_expression();
        if (firstIdNode)
        {
            write->addChild(firstIdNode);
            while (pos < lexems.size() && lexems[pos] == std::pair<int, int>(2, 9))
            {
                pos++;
                std::shared_ptr<Node> nextIdNode = parse_expression();
                if (!nextIdNode)
                {
                    throw std::runtime_error("Error (syntax (write)): lost operand");
                }
                write->addChild(nextIdNode);
            }
            if (lexems[pos++] != std::pair<int, int>(2, 15))
            {
                throw std::runtime_error("Error (syntax (write)): not found close bracket");
            }
        }
        else
        {
            throw std::runtime_error("Error (syntax (write)): lost operand");
        }
    }
    else
    {
        throw std::runtime_error("Error (syntax (write)): not found open bracket");
    }
    return write;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_variable_declaration()
{
    std::shared_ptr<Node> typeNode;
    if (lexems[pos] == std::pair<int, int>(1, 11))
    {
        typeNode = std::make_shared<Node>(NodeType::INT, lexems[pos++], "% (int)");
        typeNode->typeOperation = NodeType::INT;
    }
    else if (lexems[pos] == std::pair<int, int>(1, 12))
    {
        typeNode = std::make_shared<Node>(NodeType::DOUBLE, lexems[pos++], "! (double)");
        typeNode->typeOperation = NodeType::DOUBLE;
    }
    else if (lexems[pos] == std::pair<int, int>(1, 13))
    {
        typeNode = std::make_shared<Node>(NodeType::BOOL, lexems[pos++], "$ (bool)");
        typeNode->typeOperation = NodeType::BOOL;
    }
    else
    {
        throw std::runtime_error("Error (syntax (declaration)): unknown type");
    }

    if (typeNode)
    {
        std::shared_ptr<Node> firstIdNode = parse_operand(typeNode->type);

        if (firstIdNode)
        {
            typeNode->addChild(firstIdNode);
            if (typeNode->typeOperation == NodeType::BOOL)
            {
                bool_identifier[firstIdNode->token.second] = 0;
            }
            while (pos < lexems.size() && lexems[pos] == std::pair<int, int>(2, 9))
            {
                pos++;
                std::shared_ptr<Node> nextIdNode = parse_operand(typeNode->type);
                if (!nextIdNode)
                {
                    throw std::runtime_error("Error (syntax (declaration)): lost operand");
                }
                typeNode->addChild(nextIdNode);
                if (typeNode->typeOperation == NodeType::BOOL)
                {
                    bool_identifier[nextIdNode->token.second] = 0;
                }
            }
        }
        else
        {
            throw std::runtime_error("Error (syntax (declaration)): lost operand");
        }
    }
    else
    {
        throw std::runtime_error("Error (syntax (declaration)): unknown type");
    }

    semer->process_declared(typeNode);

    return typeNode;
}

int SyntaxAnalyzer::get_precedence(int op)
{
    switch (op) {
    case 1:  // '<'
    case 2:  // '>'
        return 1;
    case 10:  // '+'
    case 11:  // '-'
        return 2;
    case 12:  // '*'
    case 13:  // '/'
        return 3;
    case 17: // '&&' (AND)
    case 18: // '||' (OR)
        return 4;
    case 19: // 'NOT'
        return 5;
    default:
        return -1; // Non-operator or unsupported operator
    }
    return 0;
}

int SyntaxAnalyzer::getIndentLevel()
{
    int oldPos = pos;
    int level = 0;
    while (pos < lexems.size() && lexems[pos].first == 2 && lexems[pos].second == 21) {
        ++level;
        ++pos;
    }
    //pos = oldPos;
    return level;
}

std::shared_ptr<Node> SyntaxAnalyzer::parse_assignment()
{
    std::shared_ptr<Node> asNode= std::make_shared<Node>(NodeType::ASSIGNMENT, lexems[pos],"Assigment"); // узел присваивания

    auto it = lexer->table_identifiers1.find(lexems[pos-1].second);
    std::shared_ptr<Node> identifierNode = std::make_shared<Node>(NodeType::EXPRESSION, lexems[pos - 1], "Operand(" + it->second + ")"); // узел идентификатора
    asNode->addChild(identifierNode);
    pos++;//переход на следующий

    std::shared_ptr<Node> expressionNode = parse_expression();
    asNode->addChild(expressionNode);

    semer->check_type_matching(asNode);
    return asNode;
}


bool SyntaxAnalyzer::parseLexem()
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
        return false;
    }
    return true;
}
