#include <iostream>
#include "codeWriter.h"

codeWriter::codeWriter(std::string outputfileName)
{
    // Dosyayı aç
    file.open(outputfileName);
    jumpCount = 0;
    returnLabel = 0;
    currentFunction = "";

    segmentMap = {
        {"constant", SEG_CONSTANT},
        {"local", SEG_LOCAL},
        {"argument", SEG_ARGUMENT},
        {"this", SEG_THIS},
        {"that", SEG_THAT},
        {"temp", SEG_TEMP},
        {"pointer", SEG_POINTER},
        {"static", SEG_STATIC}};

    // Bootstrap: SP = 256, sonra Sys.init'i cagir
    file << "@256\n"
         << "D=A\n"
         << "@SP\n"
         << "M=D\n";
    writeCall("Sys.init", 0);

    // Sys.init geri donerse buraya duser; sonsuz dongu (ayrica asagidaki
    // ortak alt yordamlara "dusmeyi" de engeller)
    file << "(GLOBAL_HALT)\n"
         << "@GLOBAL_HALT\n"
         << "0;JMP\n";

    writeRuntime();
}

codeWriter::~codeWriter()
{
    file.close();
}

void codeWriter::setFileName(std::string newFileName)
{
    fileName = newFileName;
    size_t slashPos = fileName.find_last_of("/\\");
    if (slashPos != std::string::npos)
        fileName = fileName.substr(slashPos + 1);
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos)
        fileName = fileName.substr(0, dotPos);
}

// VM etiketleri fonksiyon kapsamlidir: FonksiyonAdi$etiket
std::string codeWriter::scopedLabel(std::string label)
{
    if (currentFunction.empty())
        return label;
    return currentFunction + "$" + label;
}

// D'yi yigina it
void codeWriter::pushD()
{
    file << "@SP\n"
         << "M=M+1\n"
         << "A=M-1\n"
         << "M=D\n";
}

// Yigindan D'ye cek
void codeWriter::popD()
{
    file << "@SP\n"
         << "AM=M-1\n"
         << "D=M\n";
}

// -------------------------------------------------------------------------
// Ortak alt yordamlar. call / return / eq / gt / lt her cagri yerinde
// tekrar tekrar uretilmek yerine tek bir kez yazilir; cagri yerleri
// buraya atlar. ROM'a sigmak icin gerekli.
// -------------------------------------------------------------------------
void codeWriter::writeRuntime()
{
    // ---- GLOBAL_CALL ----
    // Giris: D = donus adresi, R14 = hedef fonksiyon adresi, R13 = nArgs
    file << "(GLOBAL_CALL)\n";
    pushD(); // donus adresi
    const char *ptrs[] = {"LCL", "ARG", "THIS", "THAT"};
    for (int i = 0; i < 4; i++)
    {
        file << "@" << ptrs[i] << "\n"
             << "D=M\n";
        pushD();
    }
    // ARG = SP - 5 - nArgs
    file << "@SP\n"
         << "D=M\n"
         << "@5\n"
         << "D=D-A\n"
         << "@R13\n"
         << "D=D-M\n"
         << "@ARG\n"
         << "M=D\n";
    // LCL = SP
    file << "@SP\n"
         << "D=M\n"
         << "@LCL\n"
         << "M=D\n";
    // goto fonksiyon
    file << "@R14\n"
         << "A=M\n"
         << "0;JMP\n";

    // ---- GLOBAL_RETURN ----
    file << "(GLOBAL_RETURN)\n";
    // R13 = FRAME = LCL
    file << "@LCL\n"
         << "D=M\n"
         << "@R13\n"
         << "M=D\n";
    // R14 = *(FRAME-5)  (donus adresi, ARG'a yazmadan ONCE alinmali)
    file << "@5\n"
         << "A=D-A\n"
         << "D=M\n"
         << "@R14\n"
         << "M=D\n";
    // *ARG = pop()
    file << "@SP\n"
         << "A=M-1\n"
         << "D=M\n"
         << "@ARG\n"
         << "A=M\n"
         << "M=D\n";
    // SP = ARG+1
    file << "@ARG\n"
         << "D=M+1\n"
         << "@SP\n"
         << "M=D\n";
    // THAT/THIS/ARG/LCL = *(FRAME-1..4)
    const char *restore[] = {"THAT", "THIS", "ARG", "LCL"};
    for (int i = 0; i < 4; i++)
    {
        file << "@R13\n"
             << "AM=M-1\n"
             << "D=M\n"
             << "@" << restore[i] << "\n"
             << "M=D\n";
    }
    // goto R14
    file << "@R14\n"
         << "A=M\n"
         << "0;JMP\n";

    // ---- GLOBAL_EQ / GLOBAL_GT / GLOBAL_LT ----
    // Giris: R15 = donus adresi
    const char *names[] = {"EQ", "GT", "LT"};
    const char *jumps[] = {"JEQ", "JGT", "JLT"};
    for (int i = 0; i < 3; i++)
    {
        file << "(GLOBAL_" << names[i] << ")\n"
             << "@SP\n"
             << "AM=M-1\n"
             << "D=M\n"
             << "A=A-1\n"
             << "D=M-D\n"
             << "@GLOBAL_" << names[i] << "_T\n"
             << "D;" << jumps[i] << "\n"
             << "@SP\n"
             << "A=M-1\n"
             << "M=0\n"
             << "@R15\n"
             << "A=M\n"
             << "0;JMP\n"
             << "(GLOBAL_" << names[i] << "_T)\n"
             << "@SP\n"
             << "A=M-1\n"
             << "M=-1\n"
             << "@R15\n"
             << "A=M\n"
             << "0;JMP\n";
    }
}

void codeWriter::writeArithmetic(std::string command)
{
    if (command == "add")
        file << "@SP\nAM=M-1\nD=M\nA=A-1\nM=D+M\n";
    else if (command == "sub")
        file << "@SP\nAM=M-1\nD=M\nA=A-1\nM=M-D\n";
    else if (command == "and")
        file << "@SP\nAM=M-1\nD=M\nA=A-1\nM=D&M\n";
    else if (command == "or")
        file << "@SP\nAM=M-1\nD=M\nA=A-1\nM=D|M\n";
    else if (command == "not")
        file << "@SP\nA=M-1\nM=!M\n";
    else if (command == "neg")
        file << "@SP\nA=M-1\nM=-M\n";
    else if (command == "eq" || command == "gt" || command == "lt")
    {
        std::string sub = (command == "eq") ? "GLOBAL_EQ" : (command == "gt") ? "GLOBAL_GT"
                                                                             : "GLOBAL_LT";
        file << "@CMP_RET_" << jumpCount << "\n"
             << "D=A\n"
             << "@R15\n"
             << "M=D\n"
             << "@" << sub << "\n"
             << "0;JMP\n"
             << "(CMP_RET_" << jumpCount << ")\n";
        jumpCount++;
    }
    else
    {
        std::cerr << "UYARI: bilinmeyen aritmetik komut: " << command << "\n";
    }
}

void codeWriter::writePushPop(CommandType commandType, std::string segment, int index)
{
    if (segmentMap.find(segment) == segmentMap.end())
    {
        std::cerr << "UYARI: bilinmeyen segment: " << segment << "\n";
        return;
    }
    SegmentType type = segmentMap[segment];

    std::string segSymbol = "";
    if (type == SEG_LOCAL)
        segSymbol = "LCL";
    else if (type == SEG_ARGUMENT)
        segSymbol = "ARG";
    else if (type == SEG_THIS)
        segSymbol = "THIS";
    else if (type == SEG_THAT)
        segSymbol = "THAT";

    if (commandType == C_PUSH)
    {
        switch (type)
        {
        case SEG_LOCAL:
        case SEG_ARGUMENT:
        case SEG_THIS:
        case SEG_THAT:
        {
            if (index == 0)
                file << "@" << segSymbol << "\nA=M\nD=M\n";
            else if (index == 1)
                file << "@" << segSymbol << "\nA=M+1\nD=M\n";
            else
                file << "@" << index << "\nD=A\n@" << segSymbol << "\nA=D+M\nD=M\n";
            pushD();
            break;
        }
        case SEG_POINTER:
        {
            std::string ptrSymbol = (index == 0) ? "THIS" : "THAT";
            file << "@" << ptrSymbol << "\nD=M\n";
            pushD();
            break;
        }
        case SEG_CONSTANT:
        {
            if (index == 0)
                file << "@SP\nM=M+1\nA=M-1\nM=0\n";
            else if (index == 1)
                file << "@SP\nM=M+1\nA=M-1\nM=1\n";
            else
            {
                file << "@" << index << "\nD=A\n";
                pushD();
            }
            break;
        }
        case SEG_TEMP:
            file << "@" << (5 + index) << "\nD=M\n";
            pushD();
            break;
        case SEG_STATIC:
            file << "@" << fileName << "." << index << "\nD=M\n";
            pushD();
            break;
        default:
            break;
        }
    }
    else if (commandType == C_POP)
    {
        switch (type)
        {
        case SEG_LOCAL:
        case SEG_ARGUMENT:
        case SEG_THIS:
        case SEG_THAT:
        {
            if (index == 0)
            {
                popD();
                file << "@" << segSymbol << "\nA=M\nM=D\n";
            }
            else if (index == 1)
            {
                popD();
                file << "@" << segSymbol << "\nA=M+1\nM=D\n";
            }
            else
            {
                // hedef adresi R13'te sakla
                file << "@" << index << "\nD=A\n@" << segSymbol << "\nD=D+M\n@R13\nM=D\n";
                popD();
                file << "@R13\nA=M\nM=D\n";
            }
            break;
        }
        case SEG_POINTER:
        {
            std::string ptrSymbol = (index == 0) ? "THIS" : "THAT";
            popD();
            file << "@" << ptrSymbol << "\nM=D\n";
            break;
        }
        case SEG_TEMP:
            popD();
            file << "@" << (5 + index) << "\nM=D\n";
            break;
        case SEG_STATIC:
            popD();
            file << "@" << fileName << "." << index << "\nM=D\n";
            break;
        case SEG_CONSTANT:
            break;
        default:
            break;
        }
    }
}

void codeWriter::writeLabel(std::string label)
{
    if (!label.empty())
        file << "(" << scopedLabel(label) << ")\n";
}

void codeWriter::writeGoto(std::string label)
{
    if (!label.empty())
    {
        file << "@" << scopedLabel(label) << "\n"
             << "0;JMP\n";
    }
}

void codeWriter::writeIf(std::string label)
{
    if (!label.empty())
    {
        file << "@SP\n"
             << "AM=M-1\n"
             << "D=M\n"
             << "@" << scopedLabel(label) << "\n"
             << "D;JNE\n";
    }
}

void codeWriter::writeFunction(std::string functionName, int nVars)
{
    currentFunction = functionName;
    file << "(" << functionName << ")\n";

    for (int i = 0; i < nVars; i++)
        file << "@SP\nM=M+1\nA=M-1\nM=0\n";
}

void codeWriter::writeReturn()
{
    file << "@GLOBAL_RETURN\n"
         << "0;JMP\n";
}

void codeWriter::writeCall(std::string functionName, int nArgs)
{
    // R14 = hedef fonksiyon adresi
    file << "@" << functionName << "\nD=A\n@R14\nM=D\n";
    // R13 = nArgs
    file << "@" << nArgs << "\nD=A\n@R13\nM=D\n";
    // D = donus adresi, sonra ortak call yordamina atla
    file << "@returnLabel_" << returnLabel << "\nD=A\n"
         << "@GLOBAL_CALL\n0;JMP\n"
         << "(returnLabel_" << returnLabel << ")\n";
    returnLabel++;
}
