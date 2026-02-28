#pragma once
#include "Utils.h"

class Helpers
{
public:
	static int numberOfSetBits(uint32_t i);
	static int bytesToNextInstruction(int instruction);
    static std::vector<uint8_t> Decompress(
        const std::string &sourceFileName,
        uint64_t dataOffset,
        uint32_t packedSize,
        uint32_t unpackedSize,
        bool isHeader2
    );
    static uint32_t MatchIt(
        uint8_t *a,
        uint8_t *b,
        uint32_t len
    );
    static void MatchBest(
        uint8_t *buffer,
        uint32_t &backward,
        uint32_t &forward
    );
    static std::vector<uint8_t> Compress(
        const std::string &sourceFileName,
        uint32_t packedSize,
        uint32_t &unpackedSize,
        bool isHeader2
    );
    static uint32_t Dispatch(
        std::vector<uint8_t> biosData,
        uint32_t &headerOffset,
        uint32_t &unpacked, uint32_t &packed, uint32_t &target, std::string &moduleName,
        bool isHeader2
    );
    static void separateMainBlockandGetStartSize(
        const std::string &filePath,
        uint32_t *romBlockSize,
        std::streampos *fileSize,
        bool isHeader2
    );
};