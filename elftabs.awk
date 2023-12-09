#!/usr/bin/awk -f

BEGIN {
  class_map = make_map("unsigned char", "std::string", "e_classes")
  data_map = make_map("unsigned char", "std::string", "e_datas")
  osabi_map = make_map("unsigned char", "std::string", "e_osabis")
  type_map = make_map("uint16_t", "std::string", "e_types")
  machine_map = make_map("uint16_t", "std::string", "e_machines")
}

{
  gsub(/""/, "\"")
}

/#define ELFCLASS/ && $2 != "ELFCLASSNUM" {
  class_map = class_map make_kvp()
}

/#define ELFDATA/ && $2 != "ELFDATANUM" {
  data_map = data_map make_kvp()
}

/#define ELFOSABI/ {
  osabi_map = osabi_map make_kvp()
}

/#define ET_/ {
  type_map = type_map make_kvp()
}

/#define EM_/ && $2 !~ /(EM_ARC_A5|EM_ALPHA|EM_NUM)/ {
  machine_map = machine_map make_kvp()
}

END {
  print "" >"elftabs.h"

  elftab_print("#ifndef ELF_TABS_H")
  elftab_print("#define ELF_TABS_H\n")
  elftab_print("#include <unordered_map>")
  elftab_print("#include <string>\n")
  elftab_print("// Automatically generated, do NOT modify\n")

  print_map(class_map)
  print_map(data_map)
  print_map(osabi_map)
  print_map(type_map)
  print_map(machine_map)

  elftab_print("#endif")
}

function make_map(key_t, val_t, name) {
  return "static const std::unordered_map<" key_t "," val_t "> " name " {\n"
}

function make_kvp() {
  return "\t{ " $2 ", \"" parse_comment() "\" },\n"
}

function parse_comment() {
  match($0, /\/\*.*\*\//);
  comment = substr($0, RSTART + 3, RLENGTH - 6)
  gsub(/"/, "", comment)
}

function elftab_print(s) {
  print s >>"elftabs.h"
}

function print_map(map) {
  map = map "};\n"
  elftab_print(map)
}
