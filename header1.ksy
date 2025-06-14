meta:
  id: compaq_header1
  title: Header 1
  application: HP Compaq BIOS
  file-extension: bin
  tags:
    - firmware
  license: CC0-1.0
  ks-version: 0.9
  endian: le
  

seq:
 - id: magic
   type: header_entry
   doc: Identifier of the file block.
   repeat: eos
   #For IDE tests
   #repeat: expr
   #repeat-expr: 17
    
types:
 header_entry:
  seq:
  - id: signature
    size: 2
    contents: [0x00, 0x10]
  - id: more_blocks
    type: u1
    doc: BOOLEAN value. If 0 (FALSE), there's no more compressed data.
  - id: reserved
    size: 1
    contents: [0x00]
  - id: unpacked_modulesize
    type: u4
  - id: packed_modulesize
    type: u4
  - id: target_address
    type: u4
  - id: body
    size: packed_modulesize
    #Skip body if size is 0
    if: packed_modulesize != 0
  instances:
   module_name:
    value: 0x454D414E
    doc: Hack. This header type doesn't have name, but module_name is needed by HPUnpack