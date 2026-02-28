#pragma once

#include <cmath>	// Нужно для ceil
#include <direct.h> // Нужно для _mkdir
#include <fstream>  // Нужно для создания файлов
#include <iostream> // Нужно для cout, cerr
#include <string>
#include <vector>
#include <algorithm>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

class Utils
{
public:
	static void saveBufferToFile(
		const std::string &destinationFileName,
		std::vector<uint8_t> buffer,
		uint32_t size
	);
	static void saveFileToBuffer(
		const std::string &sourceFileName,
		std::vector<uint8_t> &buffer
	);
	static uint32_t stringToUint32(const std::string &str);
	static void replaceDataWithUint32(
		std::vector<uint8_t> &buffer,
		uint32_t offset,
		uint32_t newData
	);
	static std::string uint32ToAsciiBytes(uint32_t value);
	static std::vector<uint8_t> hexStringToBytes(const std::string &hex);
	static uint8_t countHexPattern(
		const std::string &sourceFileName,
		const std::string &hexPattern,
		uint32_t *startOffset
	);
	static uint32_t findHexPatternOffset(
		const std::string &sourceFileName,
		const std::string &hexPattern,
		uint8_t &patternSize
	);
	static void saveDataByOffset(
		const std::string &sourceFileName,
		const std::string &destinationFileName,
		uint32_t dataOffset,
		uint32_t dataSize
	);
};

