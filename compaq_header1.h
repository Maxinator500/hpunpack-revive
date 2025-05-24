#ifndef COMPAQ_HEADER1_H_
#define COMPAQ_HEADER1_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitai/kaitaistruct.h"
#include <stdint.h>

#if KAITAI_STRUCT_VERSION < 9000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.9 or later is required"
#endif

class compaq_header1_t : public kaitai::kstruct {

public:
    class header_entry_t;

    compaq_header1_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, compaq_header1_t* p__root = 0);
    void _read();

private:
    void _clean_up();

public:
    ~compaq_header1_t();

    class header_entry_t : public kaitai::kstruct {

    public:

        header_entry_t(kaitai::kstream* p__io, compaq_header1_t* p__parent = 0, compaq_header1_t* p__root = 0);
        void _read();

    private:
        void _clean_up();

    public:
        ~header_entry_t();

    private:
        bool f_module_name;
        int32_t m_module_name;

    public:
        int32_t module_name();

    private:
        std::string m_signature;
        uint8_t m_more_blocks;
        std::string m_reserved;
        uint32_t m_unpacked_modulesize;
        uint32_t m_packed_modulesize;
        uint32_t m_target_address;
        std::string m_body;
        bool n_body;

    public:
        bool _is_null_body() { body(); return n_body; };

    private:
        compaq_header1_t* m__root;
        compaq_header1_t* m__parent;

    public:
        std::string signature() const { return m_signature; }

        /**
         * BOOLEAN value. If 0 (FALSE), there's no more compressed data.
         */
        uint8_t more_blocks() const { return m_more_blocks; }
        std::string reserved() const { return m_reserved; }
        uint32_t unpacked_modulesize() const { return m_unpacked_modulesize; }
        uint32_t packed_modulesize() const { return m_packed_modulesize; }
        uint32_t target_address() const { return m_target_address; }
        std::string body() const { return m_body; }
        compaq_header1_t* _root() const { return m__root; }
        compaq_header1_t* _parent() const { return m__parent; }
    };

private:
    header_entry_t* m_magic;
    compaq_header1_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Identifier of the file block. (01 00) and Total length of the Header including module name. (14)
     */
    header_entry_t* magic() const { return m_magic; }
    compaq_header1_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // COMPAQ_HEADER1_H_
