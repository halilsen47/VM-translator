#ifndef CODEWRITER_H
#define CODEWRITER_H
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include "parser.h"

enum SegmentType
{
    SEG_CONSTANT,
    SEG_LOCAL,
    SEG_ARGUMENT,
    SEG_THIS,
    SEG_THAT,
    SEG_TEMP,
    SEG_POINTER,
    SEG_STATIC
};

class codeWriter
{
private:
    std::ofstream file;
    std::string stream;
    int jumpCount;
    std::string fileName;
    std::unordered_map<std::string, SegmentType> segmentMap;
    int returnLabel;
    std::string currentFunction;

public:
    codeWriter(std::string outputfileName);
    ~codeWriter();

    void writeArithmetic(std::string command);
    void writePushPop(CommandType commandType, std::string segment, int index);
    void writeLabel(std::string label);
    void writeGoto(std::string label);
    void writeIf(std::string label);
    void writeFunction(std::string functionName, int nVars);
    void writeReturn();
    void writeCall(std::string functionName, int nArgs);
    void setFileName(std::string newFileName);
    std::string scopedLabel(std::string label);
    void pushD();
    void popD();
    void writeRuntime();
};

#endif