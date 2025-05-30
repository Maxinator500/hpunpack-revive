meta:
  id: compaq_header2
  title: Header 2
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
   doc: Identifier of the file block. (01 00) and Total length of the Header including module name. (14)
   repeat: eos
   #For IDE tests
   #repeat: expr
   #repeat-expr: 22
    
types:
 header_entry:
  seq:
  - id: signature
    size: 3
    contents: [0x01, 0x00, 0x14]
  - id: more_blocks
    type: u1
    doc: Boolean value. If 0 (FALSE), there's no more compressed data.
  - id: unpacked_modulesize
    type: u4
  - id: packed_modulesize
    type: u4
  - id: target_address
    type: u4
  - id: module_name
    type: u4
  - id: body
    size: packed_modulesize
    #Skip body if size is 0
    if: packed_modulesize != 0