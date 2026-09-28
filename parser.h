#ifndef PARSER_H
#define PARSER_H
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
enum CommandType
{
    C_ARITHMATIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
    C_NULL
};

class parser
{
private:
    std::string stream;
    std::ifstream file;
    std::string command;
    std::string arg_1;
    std::string arg_2;
    std::string c_commands[9] = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};

public:
    parser(std::string fileName);
    ~parser();

    bool hasMoreLine();
    void advance();
    CommandType command_type();
    std::string arg1();
    int arg2();
};

#endif