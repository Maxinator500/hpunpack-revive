// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "compaq_header1.h"
#include "kaitai/exceptions.h"

compaq_header1_t::compaq_header1_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, compaq_header1_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = this;
    m_magic = 0;
}

void compaq_header1_t::_read() {
    m_magic = new header_entry_t(m__io, this, m__root);
    m_magic->_read();
}

compaq_header1_t::~compaq_header1_t() {
    _clean_up();
}

void compaq_header1_t::_clean_up() {
    if (m_magic) {
        delete m_magic; m_magic = 0;
    }
}

compaq_header1_t::header_entry_t::header_entry_t(kaitai::kstream* p__io, compaq_header1_t* p__parent, compaq_header1_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_module_name = false;
}

void compaq_header1_t::header_entry_t::_read() {
    m_signature = m__io->read_bytes(2);
    if (!(signature() == std::string("\x00\x10", 2))) {
        throw kaitai::validation_not_equal_error<std::string>(std::string("\x00\x10", 2), signature(), _io(), std::string("/types/header_entry/seq/0"));
    }
    m_more_blocks = m__io->read_u1();
    m_reserved = m__io->read_bytes(1);
    if (!(reserved() == std::string("\x00", 1))) {
        throw kaitai::validation_not_equal_error<std::string>(std::string("\x00", 1), reserved(), _io(), std::string("/types/header_entry/seq/2"));
    }
    m_unpacked_modulesize = m__io->read_u4le();
    m_packed_modulesize = m__io->read_u4le();
    m_target_address = m__io->read_u4le();
    n_body = true;
    if (packed_modulesize() != 0) {
        n_body = false;
        m_body = m__io->read_bytes(packed_modulesize());
    }
}

compaq_header1_t::header_entry_t::~header_entry_t() {
    _clean_up();
}

void compaq_header1_t::header_entry_t::_clean_up() {
    if (!n_body) {
    }
}

int32_t compaq_header1_t::header_entry_t::module_name() {
    if (f_module_name)
        return m_module_name;
    m_module_name = 1162690894;
    f_module_name = true;
    return m_module_name;
}
