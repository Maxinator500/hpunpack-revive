// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "compaq_header2.h"
#include "kaitai/exceptions.h"

compaq_header2_t::compaq_header2_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, compaq_header2_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = this;
    m_magic = 0;
}

void compaq_header2_t::_read() {
    m_magic = new std::vector<header_entry_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            header_entry_t* _t_magic = new header_entry_t(m__io, this, m__root);
            _t_magic->_read();
            m_magic->push_back(_t_magic);
            i++;
        }
    }
}

compaq_header2_t::~compaq_header2_t() {
    _clean_up();
}

void compaq_header2_t::_clean_up() {
    if (m_magic) {
        for (std::vector<header_entry_t*>::iterator it = m_magic->begin(); it != m_magic->end(); ++it) {
            delete *it;
        }
        delete m_magic; m_magic = 0;
    }
}

compaq_header2_t::header_entry_t::header_entry_t(kaitai::kstream* p__io, compaq_header2_t* p__parent, compaq_header2_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
}

void compaq_header2_t::header_entry_t::_read() {
    m_signature = m__io->read_bytes(3);
    if (!(signature() == std::string("\x01\x00\x14", 3))) {
        throw kaitai::validation_not_equal_error<std::string>(std::string("\x01\x00\x14", 3), signature(), _io(), std::string("/types/header_entry/seq/0"));
    }
    m_more_blocks = m__io->read_u1();
    m_unpacked_modulesize = m__io->read_u4le();
    m_packed_modulesize = m__io->read_u4le();
    m_target_address = m__io->read_u4le();
    m_module_name = m__io->read_u4le();
    n_body = true;
    if (packed_modulesize() != 0) {
        n_body = false;
        m_body = m__io->read_bytes(packed_modulesize());
    }
}

compaq_header2_t::header_entry_t::~header_entry_t() {
    _clean_up();
}

void compaq_header2_t::header_entry_t::_clean_up() {
    if (!n_body) {
    }
}
