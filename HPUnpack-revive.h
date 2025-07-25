#pragma once
#define KS_STR_ENCODING_WIN32API

#include "Utils.h"

const uint16_t MaxWordbookSize = 0xfee;
uint32_t maxCompressedWordbookOffset;
uint32_t maxCompressedLength;
uint32_t maxReCompressedWordbookOffset;

BOOLEAN isHeader2 = FALSE;
uint32_t romBlockSize;
std::streampos fileSize;
uint32_t headerOffsets[255];
uint32_t dataOffsets[255];

static int numberOfSetBits(IN uint32_t i) {
    i -= (i >> 1) & 0x55555555;
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return ((((i + (i >> 4)) & 0x0f0f0f0f) * 0x1010101) >> 0x18);
}

// Функция определяет кол-во байт между соседними Instruction bytes, функ. numberOfSetBits помогает
static int bytesToNextInstruction(IN int instruction) {
    return 0x11 - numberOfSetBits(instruction); // 0x11 это константа, никакое не смещение
}

// Функция распаковывает модуль в буфер
static std::vector<uint8_t> Decompress(
    IN const std::string& sourceFileName,
    IN uint64_t dataOffset,
    IN uint32_t packedSize,
    IN uint32_t unpackedSize
) {
    
    std::ifstream sourceFile(sourceFileName, std::ios::binary);
    if (!sourceFile.is_open()) {
        std::cerr << "Error opening source file on decompression: " << sourceFileName << std::endl; exit(1);
    }

    sourceFile.seekg(dataOffset);
    std::vector<uint8_t> sourceBuffer(packedSize); // Это сжатые данные
    sourceFile.read((char*)sourceBuffer.data(), packedSize);

    std::vector<uint8_t> buffer(unpackedSize); // Это распакованные данные
    uint8_t instruction = 0; // Текущая инструкция, изменяемое значение

    int offset = 0; // Смещение до нового Instruction byte в sourceFile
    uint32_t index = 0; // Текущее смещение в sourceFile
    uint32_t WordBook_byte_write_ptr = 0; // Смещение записи в buffer
    uint32_t num4 = 0; // Нужно для обработки исключения, учавствует в подсчете ожидаемого расстояния м-ду Instruction bytes
    uint32_t num7 = 0; // Смещение байта в sourceBuffer для записи в buffer
    uint32_t num8 = 0; // Индекс байта после Instruction byte. Если 0, то index это смещение Instruction byte
    maxCompressedWordbookOffset = 0;
    maxCompressedLength = 0;

    // Пропуск функции, но создание файла, если размер модуля на входе равен 0
    if (packedSize == 0) { buffer = { 0 }; goto SkipDeco; }

    // Алгоритм распаковки
    // I hardly understand WGO
    while (true) {
        if (num7 >= packedSize)
        {
            break;
        }

        num8 = num8 % 8;
        if (num8 == 0)
        {
            num4 = index;
            index = num7++;
            offset = bytesToNextInstruction(instruction); //Получаем значение offset

            if ((index != 0) && ((index - num4) != offset)) {
                /*Debug
                std::cerr << "Ошибка соответствия смещения инструкции!" << std::endl;
                std::cerr << "index: " << index << std::endl;
                std::cerr << "num4: " << num4 << std::endl;
                std::cerr << "offset (BytesToNextInstruction): " << offset << std::endl;
                std::cerr << "Instruction: 0x" << std::hex << static_cast<int>(instruction) << std::dec << std::endl;
                */
                std::cerr << "Instruction byte offset mismatch" << std::endl; exit(1);
            }
            instruction = sourceBuffer[index];
        }
        if ((instruction & (1 << (num8 & 0x1f))) != 0) // Пример: (0xFF & (1 << (0x0 & 0x1f))) = 1; (0xFB & (1 << (0x2 & 0x1f))) = 0 (else);
        {
            if (num7 == packedSize)
            {
                break;
            }
            buffer[WordBook_byte_write_ptr++] = sourceBuffer[num7]; // Байт не сжат, записываем как есть
        }
        else // Распаковка сжатых байт
        {
            if (num7 == (packedSize - 1))
            {
                std::cerr << "Unexpected end of input buffer" << std::endl; exit(1);
            }
            uint32_t WordBook_byte_read_ptr = (sourceBuffer[num7 + 1] << 4) | ((sourceBuffer[num7] & 240) >> 4);
            uint32_t num6 = sourceBuffer[num7] & 15;
            if (WordBook_byte_read_ptr > maxCompressedWordbookOffset)
            {
                maxCompressedWordbookOffset = WordBook_byte_read_ptr;
            }
            if (num6 > maxCompressedLength)
            {
                maxCompressedLength = num6;
            }
            num7++;
            if ((WordBook_byte_write_ptr - WordBook_byte_read_ptr) < 0)
            {   
                std::cerr << "Negative offset encountered" << std::endl; exit(1);
            }
            if (WordBook_byte_read_ptr <= 0 && isHeader2 != FALSE) // Negative offset для Header 1 это типа нормально
            {
                /*Debug
                std::cerr << "num7: " << num7 << std::endl;
                std::cerr << "num5: " << num5 << std::endl;
                std::cerr << "Current byte (sourceBuffer[num7]): 0x" << std::hex << static_cast<int>(sourceBuffer[num7]) << std::endl;
                std::cerr << "Next byte (sourceBuffer[num7 + 1]): 0x" << std::hex << static_cast<int>(sourceBuffer[num7 + 1]) << std::endl;
                */
                std::cerr << "Zero or negative offset encountered" << std::endl; exit(1);
            }
            uint32_t WordBook_Loop_Counter = 0;
            if (isHeader2 == FALSE) {

                /*
                *   "Add the value 0x1012 to the three Nibbles and subtract 0x1000 until the Wordbook Read Pointer is smaller than the Wordbook Write Pointer"
                *
                *   Eg: 10E3 это 10E, представленное как WordBook_byte_read_ptr.
                *   10E + 1012 = 1120
                *   1120 - 1000 x Write Pointer = 120h (dec file offset)
                */
                WordBook_byte_read_ptr += 0x1012;
                for (uint32_t i = WordBook_byte_write_ptr; WordBook_byte_read_ptr > i;) {
                    WordBook_byte_read_ptr -= 0x1000;
                }

                while (true)
                {
                    if (WordBook_Loop_Counter >= (num6 + 3)) // Цикл должен пройти минимум 3 раза. num6 это длина последовательности decompressed байт для копирования
                    {
                        WordBook_byte_write_ptr += num6 + 3; // Защитить распакованные данные от перезаписи, пропустив байты
                        break;
                    }
                    buffer[WordBook_byte_write_ptr + WordBook_Loop_Counter] = buffer[WordBook_Loop_Counter + WordBook_byte_read_ptr]; // Взять decompressed байт, который находится на смещении WordBook_Loop_Counter + WordBook_byte_read_ptr
                    WordBook_Loop_Counter++; // Счетчик для байт после Instruction byte
                }
            }
            else
            {
                while (true)
                {
                    if (WordBook_Loop_Counter >= (num6 + 3)) // Цикл должен пройти минимум 3 раза. num6 это длина последовательности decompressed байт для копирования
                    {
                        WordBook_byte_write_ptr += num6 + 3; // Защитить распакованные данные от перезаписи, пропустив байты
                        break;
                    }
                    buffer[WordBook_byte_write_ptr + WordBook_Loop_Counter] = buffer[(WordBook_byte_write_ptr + WordBook_Loop_Counter) - WordBook_byte_read_ptr]; // Взять decompressed байт, который находится num5 байт позади
                    WordBook_Loop_Counter++; // Счетчик для байт после Instruction byte
                }
            } // isHeader2
        }
        num7++; // Тек. смещение обработано, увеличение счетчика
        num8++;
    } // while (true)

    SkipDeco:
    sourceFile.close();
    return buffer;
}


static uint32_t MatchIt(
    IN uint8_t* a,
    IN uint8_t* b,
    IN uint32_t len
)
{
    uint32_t index = 0;
    while (true)
    {
        bool flag = index < len;
        if (!flag || (a[index] != b[index]))
        {
            return index;
        }
        index++;
    }
}

static void MatchBest(
    IN uint8_t* buffer,
    IN OUT uint32_t& backward,
    IN OUT uint32_t& forward
) {
    uint32_t num4 = 1;
    uint32_t num2 = 0;
    uint32_t num3 = 0;
    while (true)
    {
        if (num4 > backward)  // Negative offset не предусмотрен. Если смещение равно 0h, ничего лучше, чем данные здесь, не найти.
        {
            break;
        }
        uint32_t num = MatchIt(buffer - num4, buffer, forward); // Просмотреть forward байт вперёд и вернуть кол-во совпавших байт
        if (num > num2) // Определить если это совпадение лучше прошлого и сохранить смещение
        {
            num2 = num;
            num3 = num4;
        }
        if (forward == num2) // Совпали байты на всей длине forward, завершить этот instruction byte
        {
            break;
        }
        num4++; // Подсчёт итераций
    }
    backward = num3;
    forward = num2;
}

// Функция заспаковывает модуль в буфер, MatchBest и MathIt помогают
static std::vector<uint8_t> Compress(
    IN const std::string& sourceFileName,
    IN uint32_t packedSize,
    OUT uint32_t& unpackedSize
)
{
    std::ifstream sourceFile(sourceFileName, std::ios::binary | std::ios::ate); // Модификатор ate is crucial
    if (!sourceFile.is_open()) {
        std::cerr << "Error opening source file on compression: " << sourceFileName << std::endl; exit(1);
    }

    std::vector<uint8_t> sourceBuffer;
    std::streamsize fileSize = sourceFile.tellg();
    if (fileSize <= 0) {
        // Если файл пустой или ошибка чтения размера
        sourceBuffer.clear();
        std::vector<uint8_t> dst(0);
        return dst;
    }
    unpackedSize = fileSize;
    sourceBuffer.resize(static_cast<uint32_t>(fileSize));
    sourceFile.seekg(0, std::ios::beg);

    sourceFile.read((char*)sourceBuffer.data(), fileSize);

    //Debug
    //printf("fileSize: 0x%X, unpackedSize: 0x%X", (int)fileSize, unpackedSize);

    uint32_t length = fileSize;
    uint8_t* numPtr = nullptr;

    // Пропуск функции, если размер модуля на входе равен 0
    if (packedSize == 0) { sourceBuffer = { 0 }; goto SkipComp; }
    else
    {
        uint8_t* numRef = &sourceBuffer[0];
        std::vector<uint8_t> src(static_cast<uint32_t>(std::ceil(static_cast<double>(length) * 1.125)), 0);
        {
            uint8_t* numRef2 = &src[0];
            uint32_t num = 0;   // Смещение в sourceBuffer
            uint32_t num2 = 0;  // Индекс байта после instruction byte
            uint32_t index = 0; // Смещение в сжатом буфере

            while (true) {
                /*
                *  Сохранить когда num достигнет значения length, но буфер src может расти сверх length.
                *  Если пересжатый модуль сохраняется размером packedSize или в нём появились мусорные значения,
                *  проверь вызов saveBufferToFile.
                */
                if (num >= length) {
                    //Debug
                    //numRef2[0x50] = 0x1;
                    std::vector<uint8_t> dst(index, 0);
                    std::memcpy(dst.data(), src.data(), index);
                    return dst;
                }

                int64_t num1 = num2 % 8;
                num2 = (uint32_t)num1;
                if (num2 == 0) { // Новый instruction byte
                    //printf("instruction%02X\n", index);
                    numPtr = numRef2 + index++;
                    numPtr[0] = 0;
                }
                uint32_t backward = (num < MaxWordbookSize) ? num : MaxWordbookSize; // Выбор MAX_WINDOWSIZE
                uint32_t forward = ((length - num) < 0x12) ? (length - num) : 0x12; // (length - num) чтобы не получить unexpected EOF

                // Подбираем лучшее совпадение в буфере
                MatchBest(numRef + num, backward, forward);
                if (maxReCompressedWordbookOffset < backward) {
                    maxReCompressedWordbookOffset = backward;
                }
                if (forward < 3 || isHeader2 == FALSE) { // Если итераций MatchIt было менее 3...
                    // Записать data bytes без сжатия
                    //printf("data%02X ", index);
                    numRef2[index] = numRef[num++];
                    numPtr[0] = (numPtr[0] | ((uint8_t)(1 << (num2 & 0x1f))));
                }
                else {
                    if (isHeader2 == FALSE){ // Mustn reach here because of line 297
                        printf("cypher%02X ", index);
                        printf("back%02X ", backward);
                        uint32_t WordBook_byte_read_ptr = index - backward - forward - 0x12; // Надо сделать нормально, но хрен знает как

                        uint16_t val = static_cast<uint16_t>(((WordBook_byte_read_ptr) << 4) | (((forward - 3) << 4) / 0x10));
                        *reinterpret_cast<uint16_t*>(numRef2 + index) = val;
                        index++;
                        num += forward;
                    }
                    else
                    {
                        // Записываем кодированное значение (2 байта)
                        //printf("cypher%02X ", index);
                        uint16_t val = static_cast<uint16_t>((backward << 4) | (forward - 3));
                        *reinterpret_cast<uint16_t*>(numRef2 + index) = val;
                        index++;
                        num += forward;
                    }
                }
                num2++;
                index++;
            }
        }
    }
    SkipComp:
    std::vector<uint8_t> dst(0);
    return dst;
}

//Функция отдаёт данные об одном модуле
static void Dispatch(
    IN const std::string& sourceFileName,
    IN OUT uint32_t& headerOffset,
    OUT uint32_t& unpacked, uint32_t& packed, uint32_t& target, std::string& moduleName
){
    std::ifstream ifs(sourceFileName, std::ifstream::binary); // Объявление переменной ifs класса ifstream (input file stream), входящий файловый поток
    kaitai::kstream ks(&ifs); // Объявление переменной ks класса kstream (kaitai stream), поток от указателя на "ifs" понятный API kaitai
    compaq_header1_t::header_entry_t data_header1 = compaq_header1_t::header_entry_t(&ks);
    compaq_header2_t::header_entry_t data_header2 = compaq_header2_t::header_entry_t(&ks); // Объявление переменной data класса compaq_header2_t от указателя на "ks", даёт только headers

    data_header1._io()->seek(headerOffset); // Выбор смещения с начала файла, работает и для data_header2

    if (isHeader2 == FALSE) {
        // Чтение ifs
        data_header1._read();

        // Данные заголовка
        unpacked = data_header1.unpacked_modulesize();
        packed = data_header1.packed_modulesize();
        target = data_header1.target_address();
        moduleName = uint32ToAsciiBytes(data_header1.module_name());
        headerOffset = data_header1._io()->pos(); // Смещение конца данных это смещение нового заголовка
    }
    else
    {
        // Чтение ifs
        data_header2._read();

        // Данные заголовка
        unpacked = data_header2.unpacked_modulesize();
        packed = data_header2.packed_modulesize();
        target = data_header2.target_address();
        moduleName = uint32ToAsciiBytes(data_header2.module_name());
        headerOffset = data_header2._io()->pos(); // Смещение конца данных это смещение нового заголовка
    }

    return;
}

// Функция отделяет запакованные данные и возвращает смещение Main блока
static uint32_t separateMainBlockandGetStartSize(IN const std::string& filePath) {
    int mkdirResult = _mkdir("DUMP");
    if (mkdirResult != 0) {
        if (errno != EEXIST) {
            std::cerr << "Couldn't create DUMP dir" << std::endl << std::endl;
            exit(1);
        }
    }
    std::ifstream inputFile(filePath, std::ios::binary);
    std::vector<char> tempBuffer;
    uint32_t start = 0;;

    if (inputFile.is_open()) {
        // Получаем размер файла
        inputFile.seekg(0, std::ios::end);
        fileSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);

        /* Отвергнуть файл если:
        *  Его размер не кратен 4
        *  Его размер менее 512 кбайт
        */
        if ((fileSize % 0x4 == 0) && (fileSize >= 0x80000)) {
            // Выделяем память под буфер
            tempBuffer.resize(fileSize);
        }
        else {
            std::cerr << "Unsupported BIOS" << std::endl;
            exit(1);
        }

        // Читаем файл в буфер
        inputFile.read(tempBuffer.data(), fileSize);
        inputFile.close();

        // Размер Start.bin
        uint32_t startSize;

        fileSize == 0x100000 ? startSize = fileSize / 0x10 : startSize = fileSize / 0x80; // Поддерка 512 кбайт ромов

        if (isHeader2 == FALSE) {
            startSize = 0x70000;
            if (fileSize <= startSize + 0x10000) { std::cerr << "Unsupported 512kb Rom! Report the device model for support." << std::endl; exit(1); }
        }
        start = startSize; // Смещение Main block является размером Start.bin

        // Размер RomBlock.bin
        romBlockSize = fileSize / 0x8;

        // Создаем файл для блока start (переменных и DMI)
        std::ofstream outputFileSt((std::string)"DUMP/" + "Start.bin", std::ios::binary);
        if (outputFileSt.is_open()) {
            // Записываем первую часть в файл
            outputFileSt.write(tempBuffer.data(), startSize);
            outputFileSt.close();
        }
        else { std::cout << "Unable to create Start.bin, continuing nevertheless..." << std::endl; }

        // Создаем файл для Rom Block
        std::ofstream outputFileRB((std::string)"DUMP/" + "RomBlock.bin", std::ios::binary);
        if (outputFileRB.is_open()) {
            // Записываем остальную часть в файл
            outputFileRB.write(tempBuffer.data() + (size_t)fileSize - romBlockSize, romBlockSize);
            outputFileRB.close();
        }
        else { std::cout << "Unable to create RomBlock.bin, continuing nevertheless..." << std::endl; }

        // Создаем файл для остальной части
        std::ofstream outputFileMain((std::string)"DUMP/" + "Main.bin", std::ios::binary);
        if (outputFileMain.is_open()) {
            // Записываем остальную часть в файл
            outputFileMain.write(tempBuffer.data() + startSize, (size_t)fileSize - startSize - romBlockSize);
            outputFileMain.close();
        }
        else { std::cout << "Unable to create Main.bin, continuing nevertheless..." << std::endl; }

    }
    else { std::cerr << "Unable to open input file" << std::endl; exit(1); }
    return start;
}
