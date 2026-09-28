#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "parser.h"
#include "codeWriter.h"

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    // 1. Konsoldan argüman girildi mi kontrol et
    if (argc != 2)
    {
        std::cout << "Kullanim: VMTranslator <dosya.vm | klasor_yolu>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::vector<std::string> vmFiles;
    std::string outputFile;

    // 2. Girdinin tek bir dosya mı yoksa klasör mü olduğunu belirle
    if (fs::is_regular_file(inputPath) && fs::path(inputPath).extension() == ".vm")
    {
        vmFiles.push_back(inputPath);
        // ".vm" uzantısını atıp ".asm" ekle
        outputFile = inputPath.substr(0, inputPath.find_last_of('.')) + ".asm";
    }
    else if (fs::is_directory(inputPath))
    {
        fs::path p(inputPath);
        // Klasör adıyla aynı isimde bir .asm dosyası oluştur (Örn: FibonacciElement/FibonacciElement.asm)
        outputFile = p.string() + "/" + p.filename().string() + ".asm";

        // Klasördeki tüm .vm dosyalarını listeye ekle
        for (const auto &entry : fs::directory_iterator(inputPath))
        {
            if (entry.path().extension() == ".vm")
            {
                vmFiles.push_back(entry.path().string());
            }
        }
    }
    else
    {
        std::cout << "Gecersiz girdi. Lutfen bir .vm dosyasi veya klasor yolu verin." << std::endl;
        return 1;
    }

    // 3. Tek bir codeWriter nesnesi oluştur ve çıktı dosyasını bağla
    codeWriter codeWriter(outputFile);

    // 4. Bulunan tüm .vm dosyalarını sırayla çevir
    for (const std::string &vmFile : vmFiles)
    {

        codeWriter.setFileName(vmFile); // Statik değişkenlerin karışmaması için dosya adını güncelle
        parser parser(vmFile);

        while (parser.hasMoreLine())
        {
            parser.advance();
            CommandType type = parser.command_type();

            if (type == C_ARITHMATIC)
            {
                codeWriter.writeArithmetic(parser.arg1());
            }
            else if (type == C_POP || type == C_PUSH)
            {
                codeWriter.writePushPop(type, parser.arg1(), parser.arg2());
            }
            // Yeni eklenen Bölüm 8 komutları
            else if (type == C_LABEL)
            {
                codeWriter.writeLabel(parser.arg1());
            }
            else if (type == C_GOTO)
            {
                codeWriter.writeGoto(parser.arg1());
            }
            else if (type == C_IF)
            {
                codeWriter.writeIf(parser.arg1());
            }
            else if (type == C_FUNCTION)
            {
                codeWriter.writeFunction(parser.arg1(), parser.arg2());
            }
            else if (type == C_RETURN)
            {
                codeWriter.writeReturn();
            }
            else if (type == C_CALL)
            {
                codeWriter.writeCall(parser.arg1(), parser.arg2());
            }
        }
    }

    return 0;
}