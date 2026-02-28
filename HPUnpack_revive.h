#pragma once
#include "Helpers.h"

class HPUnpack_revive
{
public:
	static uint32_t *DecatenateBlocks(
		std::string fileName,
		bool &isHeader2
	);
	static std::vector<uint8_t> DisableCHKSUM(
		std::string fileName,
		std::vector<uint8_t> biosData,
		bool isHeader2
	);
	static bool Unpack(
		std::vector<uint8_t> biosData,
		std::string fileName,
		std::string newFileName,
		uint32_t *headerOffsets,
		uint8_t moduleId,
		bool isUnpackedReq,
		bool isHeader2
	);
	static std::vector<uint8_t> CompressAndReplace(
		std::vector<uint8_t> biosData,
		std::string newModuleName,
		uint8_t moduleId,
		bool isHeader2
	);
	static void CreateReport(
		std::vector<uint8_t> biosData,
		uint32_t *headerOffsets,
		uint8_t headerSize,
		bool isHeader2
	);
};