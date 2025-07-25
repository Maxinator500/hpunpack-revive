#include "HPUnpack-revive.h"

int main(int argc, char* argv[])
{
    // Обработка аргументов, получение имения файла в fileName
    if (argc < 2 || argc > 5) {
        printf("\rUsage: Rom.bin [action] <module offset> <module file>\n\nActions:\n\t-r\t\tReplace a module\n\t-fix\t\tСhecksum check disable\n");
        return 1;
    }
    // Debug
    //printf("Args count: %d\n", argc);

    //
    // Вместо всех fileName можно временно указать Rom.bin
    //
    const std::string fileName = argv[1];
    uint32_t start = NULL;

    uint8_t headerSize = 0x14; // Не константа
    std::string pattern = "01001401"; // Pattern заголовка 2
    uint8_t modulesCount = countHexPattern(fileName, pattern); // Посчитать модули
    pattern = "01001400"; // Тоже Pattern заголовка 2
    modulesCount += countHexPattern(fileName, pattern); // Добавить исключения в виде модуля F0000 и последнего модуля

    // Проверка сколько модулей нашло. Если не нашло, тогда предполагаем сжатие по типу Header 1.
    if (modulesCount < 1) {
        printf("Assuming the BIOS is of Header 1 type...\n");

        start = separateMainBlockandGetStartSize(fileName);
        headerSize = 0x10;
        pattern = "00100100"; // Pattern заголовка 1
        modulesCount = countHexPattern(fileName, pattern); // Посчитать модули
        modulesCount += 1; // Добавить исключения в виде последнего модуля

        // Debug
        //printf("Modules: %d\n", modulesCount);
        if (modulesCount < 1) {
            printf("\nBIOS is corrupted or has an unknown format!\nModules count: %d\n", modulesCount);
            return 1;
        };
    }
    else
    {
        isHeader2 = TRUE;

        // Одновременно проверка правильности файла и разделение его на сост.
        start = separateMainBlockandGetStartSize(fileName);
    }

    // Debug
    // printf("Header Size: 0x%x\n", headerSize);
    const char* action = argv[2];

    headerOffsets[0] = start; // Первый модуль лежит здесь, start получен от separateMainBlockandGetStartSize()
    dataOffsets[0] = start + headerSize; // Данные первого модуля лежат здесь
    
    // Общие переменные
    uint32_t unpacked = NULL;
    uint32_t packed = NULL;
    uint32_t target = NULL;
    std::string moduleName;
    std::vector<uint8_t> biosData;

    // Переменные для Сhecksum check disable
    uint32_t patternOffset = NULL;
    uint8_t patternSize = NULL;
    std::string patternString = (std::string)"660BFF7405660BC07401F96661C3";
    std::vector<uint8_t> patchBytes = hexStringToBytes((std::string)"660BFF7400660BC0EB01F96661C3");

    // Переменные для Replace
    uint32_t replaceOffset = NULL;
    uint32_t packedModulesDiff = NULL;
    std::vector<uint8_t> compressedData;

    // Повторная проверка сколько модулей нашло, на случай если Header 1
    if (modulesCount > 0) {
        switch (argc) {
        case 3:
            // Процедура откл. защиты
            if (action != (std::string)"-fix") { action = NULL; break; }
            action = "Сhecksum check disable";

            if (isHeader2 == FALSE) { 
                patternString = (std::string)"E2F80BFF74040BC07401F9C3";
                patchBytes = hexStringToBytes((std::string)"E2F80BFF74000BC0EB01F9C3");
            };

            patternOffset = findHexPatternOffset(fileName, patternString, patternSize);
            if (patternOffset == NULL || patternSize == NULL) {
                printf("Couldn't find the checksum check, patch not applied!\n");
                return 1;
            }

            // Сохранение файла в буфер
            saveFileToBuffer(fileName, biosData);

            // Стереть байты
            for (uint32_t i = 0; i < patternSize; i++) {
                biosData.erase(biosData.begin() + patternOffset);
            }
            // Вставить байты
            for (size_t i = patternSize - 1; i > 0; i--) {
                biosData.insert(biosData.begin() + patternOffset, patchBytes[i]);
            }
            biosData.insert(biosData.begin() + patternOffset, patchBytes[0]);

            printf("Found and patched the checksum check at offset: 0x%X\n", patternOffset);
            printf("Saving patched Rom to newBios_ChkOff.rom file...\n");
            saveBufferToFile("newBios_ChkOff.rom", biosData, biosData.size());

            break;
        case 5:
            // Процедура запаковки и замены
            if (action != (std::string)"-r") { action = NULL; break; }

            
            if (isHeader2 == FALSE) {
                printf("\nHeader 1 BIOS type unsupported for compression.\nHold \"O\" to override and replace with 0%% compression rate.\n");
                
                // С осторожностью, Win only hack
                system("timeout 10");
                if (!(GetKeyState('O') & 0x8000))
                {
                    action = NULL; break;
                }
            };

            /*=================================================
                    Ниже идут вызовы функций замены
            =================================================*/
            for (uint8_t i = 0; i < modulesCount; i++) {
                // На первой итерации отдаём fileName и headerOffsets[0]
                Dispatch(fileName, headerOffsets[i], unpacked, packed, target, moduleName);

                headerOffsets[i + 1] = headerOffsets[i]; // Наполнение массива смещениями модулей, не пропуская заголовки
                dataOffsets[i + 1] = headerOffsets[i] + headerSize; // Наполнение массива смещениями модулей, пропуская заголовки

                if (dataOffsets[i] == stringToUint32(argv[3])) {
                    replaceOffset = dataOffsets[i];
                    printf("Replacing %x...\n", stringToUint32(argv[3]));
                    break; // Завершить
                }
            }
            if (replaceOffset != stringToUint32(argv[3]))
            {
                printf("Failed to find module at offset %s\n", argv[3]);
                return 1;
            }

            /* 
            * Запаковка файла в буфер
            * Нельзя передавать unpacked для размера buffer,
            * предполагается, что размер входного файла может быть любой.
            * Поэтому, "OUT uint32_t& unpackedSize"
            */
            compressedData = Compress(argv[4], packed, unpacked);
            if (compressedData.size() == 0) { printf("Failed to compress the file: empty data\n"); return 1; }

            // Сохранение файла в буфер
            saveFileToBuffer(fileName, biosData);

            // Стереть старый модуль из буфера
            for (uint32_t i = 0; i < packed; i++) {
                biosData.erase(biosData.begin() + replaceOffset);
            }
            // Вставить запакованный файл
            for (size_t i = compressedData.size() - 1; i > 0; i--) {
                biosData.insert(biosData.begin() + replaceOffset, compressedData[i]);
            }
            biosData.insert(biosData.begin() + replaceOffset, compressedData[0]);

            // Простое выявление большего значения и присвоение разницы - packedModulesDiff
            packedModulesDiff = packed > compressedData.size() ? (packed - compressedData.size()) : (compressedData.size() - packed);

            // Debug
            //printf("packedModulesDiff 0x%X\n", packedModulesDiff);

            // Убрать разницу в размерах. "(fileSize - romBlockSize)" это вычисление смещения romBlock. 4 - отступ для chksum bytes
            if (packed >= compressedData.size()) {
                //Увеличить Rom, т.к. размер ориг модуля (packed), был больше чем размер данных для вставки (compressedData.size)
                for (uint32_t i = 0; i < packedModulesDiff; i++) {
                    biosData.insert(biosData.begin() + ((uint32_t)fileSize - romBlockSize) - (packedModulesDiff + 4), 0xFF);
                }
            }
            else
            {   
                printf("Uh-oh, the compressed module got bigger!\nConsider checking if it overlaps with the start of RomBlock. Offset: 0x%X.\n", (uint32_t)fileSize - romBlockSize);
                
                // Уменьшить Rom
                for (uint32_t i = 0; i < packedModulesDiff; i++) {
                    biosData.erase(biosData.begin() + ((uint32_t)fileSize - romBlockSize) - (packedModulesDiff - 4));
                }
            }

            // Правка packed и unpacked в файле биоса
            replaceDataWithUint32(biosData, (replaceOffset - headerSize) + 0x8, (uint32_t)compressedData.size());
            replaceDataWithUint32(biosData, (replaceOffset - headerSize) + 0x4, unpacked);

            printf("Saving re-compressed Rom to newBios.rom file...\n");
            saveBufferToFile("newBios.rom", biosData, biosData.size());

            action = "Replace";
            break;
        case 2:
            // Если не Replace, значит Decompress
            action = "Decompress";

            // Процедура чтения и распаковки
            printf("Offset in file\tUnpacked  Packed    Target    Name\n");
            for (uint8_t i = 0; i < modulesCount; i++) {

                // Отдаёт данные заголовка и сообщает смещение нового
                Dispatch(fileName, headerOffsets[i], unpacked, packed, target, moduleName);

                printf("0x%08X;\t0x%06X; 0x%06X; 0x%06X; %s\n", (uint32_t)dataOffsets[i], unpacked, packed, target, moduleName.c_str());

                headerOffsets[i + 1] = headerOffsets[i]; // Наполнение массива смещениями модулей, не пропуская заголовки
                dataOffsets[i + 1] = headerOffsets[i] + headerSize; // Наполнение массива смещениями модулей, пропуская заголовки

                /*=================================================
                Ниже идут вызовы функций сохранения и распаковки
                =================================================*/
                char moduleFileName[24];
                sprintf_s(moduleFileName, "%X_%s.bin", target, moduleName.c_str());
                char decModuleFileName[24];
                sprintf_s(decModuleFileName, "%X_%s.dec.bin", target, moduleName.c_str());
                char newCompModuleFileName[24];
                sprintf_s(newCompModuleFileName, "%X_%s.newcomp.bin", target, moduleName.c_str());

                // Debug
                //printf("Module Name: %s", moduleName.c_str());

                // Создать файл запакованного модуля
                saveDataByOffset(fileName, (std::string)"DUMP/" + moduleFileName, dataOffsets[i], packed);

                // Создать .dec файл
                std::vector<uint8_t> decompressedData = Decompress(fileName, dataOffsets[i], packed, unpacked);
                saveBufferToFile((std::string)"DUMP/" + decModuleFileName, decompressedData, unpacked);

                // Создать .newcomp файл. Закомментировано чтобы не запаковывать все файлы
                //std::vector<uint8_t> compressedData = Compress((std::string)"DUMP/" + decModuleFileName, packed, unpacked);
                //saveBufferToFile(newCompModuleFileName, compressedData, compressedData.size());

                /* Debug
                std::cout << "Data (size: " << decompressedData.size() << "):" << std::endl;
                for (uint8_t byte : decompressedData) {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte) << " ";
                }
                std::cout << std::endl;*/
            }
            break;
        default:
            // Если кол-во аргументов ни одно из заявленных
            printf("Incorrect command!\n");
            return 1;
        }
        if (action == NULL) {
            printf("\nInvalid action: %s\n", argv[2]);
            return 1;
        }
    }
    else
    {
        printf("\nBIOS is corrupted or has an unknown format!\nModules count: %d\n", modulesCount);
        return 1;
    }

    printf("\n%s performed successfully!\n", action); //Если дошло до конца, вывод текста
    return 0;
}
