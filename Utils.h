#pragma once
#include <iostream> // Нужно для cout, cerr
#include <fstream>  // Нужно для создания файлов
#include <direct.h> // Нужно для _mkdir
#include <cmath> // Нужно для ceil
#include "kaitai/kaitaistream.cpp" // Подключение kaitai API
#include "compaq_header2.cpp" // kaitai generated исходники из .ksy для header2 type, compaq_header2.cpp ссылается на compaq_header2.h
#include "compaq_header1.cpp"

//Debug
//#include <iomanip> // Нужно для setfill и setw в HPUnpack-revive.cpp
//#include <bitset>

static void saveBufferToFile(
    IN const std::string& destinationFileName,
    IN std::vector<uint8_t> buffer,
    IN uint32_t size
) {
    std::ofstream outputFile(destinationFileName, std::ios::binary);

    // Пропуск функции, если размер модуля на входе равен 0
    if (size == 0) { buffer = { 0 }; goto SkipSave; }

    if (!outputFile.is_open()) {
        std::cerr << "Error creating output file: " << destinationFileName << std::endl; exit(1);
    }

    outputFile.write((char*)buffer.data(), size);

    if (outputFile.fail()) {
        std::cerr << "Error writing data to " << destinationFileName << std::endl;
        outputFile.close();
        exit(1);
    }

    SkipSave:
    outputFile.close();
    return;
};

static void saveFileToBuffer(
    IN const std::string& sourceFileName,
    OUT std::vector<uint8_t>& buffer
) {
    std::ifstream inputFile(sourceFileName, std::ios::binary | std::ios::ate);  // Модификатор ate is crucial
    if (!inputFile.is_open()) {
        std::cerr << "Error opening inputFile file: " + sourceFileName << std::endl; exit(1);
    }

    std::streamsize fileSize = inputFile.tellg();
    if (fileSize <= 0) {
        // Если файл пустой или ошибка чтения размера
        buffer.clear();
        return;
    }

    buffer.resize(static_cast<uint32_t>(fileSize));
    inputFile.seekg(0, std::ios::beg);

    if (!inputFile.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        std::cerr << "Error reading file: " << sourceFileName << std::endl; exit(1);
    }
}


// Функция перевода char* в uint32_t
static uint32_t stringToUint32(IN const std::string& str) {
    try {
        return std::stoul(str, nullptr, 0x10); // Преобразование строки в uint32_t с основанием 10h
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Invalid string format on replacement" << std::endl; exit(1);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Number is out of uint32_t range" << std::endl; exit(1);
    }
}

// Функция замены данных в uint8_t буфере
static void replaceDataWithUint32(
    IN OUT std::vector<uint8_t>& buffer,
    IN uint32_t offset,
    IN uint32_t newData) {
    if (offset + 4 > buffer.size()) {
        std::cerr << "Replacement exceeds buffer size" << std::endl; exit(1);
    }
    buffer[offset] = static_cast<uint8_t>(newData & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>((newData >> 8) & 0xFF);
    buffer[offset + 2] = static_cast<uint8_t>((newData >> 16) & 0xFF);
    buffer[offset + 3] = static_cast<uint8_t>((newData >> 24) & 0xFF);
}

// Функция перевода LE UINTN в ASCII
static std::string uint32ToAsciiBytes(IN uint32_t value) {
    std::string result = "";
    for (int i = 0; i < 4; ++i) {
        uint8_t byte = (value >> (i * 8)) & 0xFF; // Извлекаем байты по порядку
        if (byte >= 0 && byte <= 127) {          // Проверка на валидность ASCII
            result += static_cast<char>(byte);
        }
        else {
            result += '?'; // Заменяем невалидные ASCII символы на '?'
        }
    }
    return result;
}


static std::vector<uint8_t> hexStringToBytes(IN const std::string& hex) {
    std::vector<uint8_t> bytes;
    if (hex.length() % 2 != 0) { std::cerr << "Invalid HEX String: not even length" << std::endl; exit(1); }
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        uint8_t byte;
        try {
            byte = std::stoul(byteStr, nullptr, 16);
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid HEX String: " << std::string(e.what()) << std::endl; exit(1);
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Invalid HEX String: char out of range" << std::endl; exit(1);
        }
        bytes.push_back(byte);
    }
    return bytes;
}

// Функция подсчитывает количество вхождений шаблона в бинарном файле, функ. hexStringToBytes помогает
static uint8_t countHexPattern(
    IN const std::string& sourceFileName,
    IN const std::string& hexPattern
)
{
    std::ifstream file(sourceFileName, std::ios::binary);
    if (!file.is_open()) { std::cerr << "Couldn't open: " << sourceFileName << std::endl; exit(1); }

    std::vector<uint8_t> pattern = hexStringToBytes(hexPattern);
    uint8_t count = 0;
    std::vector<uint8_t> buffer(pattern.size());

    file.seekg(0, std::ios::end);
    uint32_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // файл меньше шаблона
    if (fileSize < pattern.size()) { std::cerr << "BIOS is corrupted or too small" << std::endl; exit(1); }

    for (uint32_t i = 0; i <= fileSize - pattern.size(); ++i) {
        file.seekg(i, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), pattern.size());
        if (buffer == pattern) {
            count++;
        }
    }

    file.close();
    return count;
}

static uint32_t findHexPatternOffset(
    IN const std::string& sourceFileName,
    IN const std::string& hexPattern,
    OUT uint8_t& patternSize
) {
    std::ifstream file(sourceFileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Couldn't open: " << sourceFileName << std::endl; exit(1);
    }

    std::vector<uint8_t> pattern = hexStringToBytes(hexPattern);
    std::vector<uint8_t> buffer(pattern.size());
    patternSize = pattern.size();

    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // файл меньше шаблона
    if (fileSize < pattern.size()) {
        std::cerr << "BIOS is corrupted or too small" << std::endl;
        file.close();
        exit(1);
    }

    for (long long i = 0; i <= fileSize - pattern.size(); ++i) {
        file.seekg(i, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), pattern.size());
        if (buffer == pattern) {
            file.close();
            return i; // Возвращаем смещение
        }
    }

    file.close();
    return NULL; // Возвращаем NULL, если шаблон не найден
}

// Функция сохраняет данные из файла по смещению
static void saveDataByOffset(
    IN const std::string& sourceFileName,
    IN const std::string& destinationFileName,
    IN uint32_t dataOffset,
    IN uint32_t dataSize
)
{

    std::ifstream sourceFile(sourceFileName, std::ios::binary);
    std::ofstream destinationFile(destinationFileName, std::ios::binary);

    if (!sourceFile.is_open()) {
        std::cerr << "Error opening source file on saveDataByOffset function: " << sourceFileName << std::endl;
        exit(1);
    }

    if (!destinationFile.is_open()) {
        std::cerr << "Error creating output file on saveDataByOffset function: " << destinationFileName << std::endl;
        sourceFile.close();
        exit(1);
    }

    sourceFile.seekg(0, std::ios::end);
    std::streampos fileSize = sourceFile.tellg();
    sourceFile.seekg(0, std::ios::beg);

    if (dataOffset >= fileSize || dataOffset + dataSize > fileSize) {
        std::cerr << "Offset " << dataOffset << " is beyond the file size." << std::endl;
        sourceFile.close();
        destinationFile.close();
        exit(1);
    }


    sourceFile.seekg(dataOffset);
    std::vector<char> buffer(dataSize);
    sourceFile.read(buffer.data(), dataSize);


    if (sourceFile.fail()) {
        std::cerr << "Error reading data from: " << sourceFileName << std::endl;
        sourceFile.close();
        destinationFile.close();
        exit(1);
    }

    destinationFile.write(buffer.data(), dataSize);

    if (destinationFile.fail()) {
        std::cerr << "Error writing data to " << destinationFileName << std::endl;
        sourceFile.close();
        destinationFile.close();
        exit(1);
    }

    sourceFile.close();
    destinationFile.close();
    return;
}