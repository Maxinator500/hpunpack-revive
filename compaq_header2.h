#ifndef COMPAQ_HEADER2_H_
#define COMPAQ_HEADER2_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include <vector>

#if KAITAI_STRUCT_VERSION < 9000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.9 or later is required"
#endif

class compaq_header2_t : public kaitai::kstruct {

public:
    class header_entry_t;

    compaq_header2_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, compaq_header2_t* p__root = 0);
    void _read();

private:
    void _clean_up();

public:
    ~compaq_header2_t();

    class header_entry_t : public kaitai::kstruct {

    public:

        header_entry_t(kaitai::kstream* p__io, compaq_header2_t* p__parent = 0, compaq_header2_t* p__root = 0);
        void _read();

    private:
        void _clean_up();

    public:
        ~header_entry_t();

    private:
        std::string m_signature;
        uint8_t m_more_blocks;
        uint32_t m_unpacked_modulesize;
        uint32_t m_packed_modulesize;
        uint32_t m_target_address;
        uint32_t m_module_name;
        std::string m_body;
        bool n_body;

    public:
        bool _is_null_body() { body(); return n_body; };

    private:
        compaq_header2_t* m__root;
        compaq_header2_t* m__parent;

    public:
        std::string signature() const { return m_signature; }

        /**
         * Boolean value. If 0 (FALSE), there's no more compressed data.
         */
        uint8_t more_blocks() const { return m_more_blocks; }
        uint32_t unpacked_modulesize() const { return m_unpacked_modulesize; }
        uint32_t packed_modulesize() const { return m_packed_modulesize; }
        uint32_t target_address() const { return m_target_address; }
        uint32_t module_name() const { return m_module_name; }
        std::string body() const { return m_body; }
        compaq_header2_t* _root() const { return m__root; }
        compaq_header2_t* _parent() const { return m__parent; }
    };

private:
    std::vector<header_entry_t*>* m_magic;
    compaq_header2_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Identifier of the file block. (01 00) and Total length of the Header including module name. (14)
     */
    std::vector<header_entry_t*>* magic() const { return m_magic; }
    compaq_header2_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // COMPAQ_HEADER2_H_
