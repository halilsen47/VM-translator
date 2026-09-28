#include "parser.h"
#include <sstream>
#include <iostream>

parser::parser(std::string fileName)
{
    file.open(fileName);
}

parser::~parser()
{
    file.close();
}

bool parser::hasMoreLine()
{
    return file.peek() != EOF;
}

void parser::advance()
{
    command.clear();
    arg_1.clear();
    arg_2.clear();

    while (getline(file, stream))
    {
        size_t commentPos = stream.find("//");

        if (commentPos != std::string::npos)
        {
            stream = stream.substr(0, commentPos);
        }

        std::stringstream ss(stream);

        ss >> command;
        ss >> arg_1;
        ss >> arg_2;

        if (!command.empty())
        {
            stream = command;
            if (!arg_1.empty())
                stream += " " + arg_1;
            if (!arg_2.empty())
                stream += " " + arg_2;

            std::cout << stream + "\n"; // Konsola okunan satırı bas (Hata ayıklama için faydalı)
            break;
        }
    }
}

CommandType parser::command_type()
{
    // 1. Aritmetik ve Mantıksal Komutlar
    for (size_t i = 0; i < 9; i++)
    {
        if (command == c_commands[i])
        {
            return C_ARITHMATIC;
        }
    }

    // 2. Bellek Erişim Komutları
    if (command == "push")
        return C_PUSH;
    if (command == "pop")
        return C_POP;

    // 3. Dallanma / Branch Komutları
    if (command == "label")
        return C_LABEL;
    if (command == "goto")
        return C_GOTO;
    if (command == "if-goto")
        return C_IF;

    // 4. Fonksiyon Komutları
    if (command == "function")
        return C_FUNCTION;
    if (command == "call")
        return C_CALL;
    if (command == "return")
        return C_RETURN;

    return C_NULL;
}

std::string parser::arg1()
{
    CommandType type = command_type();

    // Aritmetik komutlarda arg1, komutun kendisidir (add, sub vs.)
    if (type == C_ARITHMATIC)
        return command;

    // return komutu arg1 almaz
    if (type == C_RETURN)
        return "";

    // PUSH, POP, LABEL, GOTO, IF, FUNCTION, CALL komutlarının tümü arg_1'i kullanır
    return arg_1;
}

int parser::arg2()
{
    CommandType type = command_type();

    // Sadece bu 4 komut arg2 (sayısal bir indeks veya argüman sayısı) alır
    if (type == C_PUSH || type == C_POP || type == C_FUNCTION || type == C_CALL)
    {
        if (arg_2.empty())
        {
            std::cerr << "Hata: '" << command << "' komutunda arguman/index eksik.\n";
            return -1;
        }
        try
        {
            return std::stoi(arg_2);
        }
        catch (const std::exception &)
        {
            std::cerr << "Hata: gecersiz index/arguman '" << arg_2 << "'.\n";
            return -1;
        }
    }

    return -1;
}