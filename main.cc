#include <unordered_map>
#include <string>

using std::unordered_map;
using std::string;

#include <elf.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include "elftabs.h"

static void Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
static void Fseek(FILE *stream, long offset, int whence);
static void disasm_32(FILE *fp);
static void disasm_64(FILE *fp);

int main(int argc, char **argv)
{
        if (argc != 2)
                errx(EX_USAGE, "usage: a.out elf-binary");

        auto fp = fopen(argv[1], "r");
        if (fp == nullptr)
                err(EX_SOFTWARE, "could not open %s", argv[1]);

        unsigned char ident[EI_NIDENT];
        Fread(ident, sizeof(*ident), EI_NIDENT, fp);

        unsigned char magic[4] = { 0x7f, 'E', 'L', 'F' };
        if (memcmp(ident, magic, 4) != 0)
                errx(EX_USAGE, "ELF magic not found");

        auto c = e_classes.find(ident[EI_CLASS]);
        if (c == e_classes.end())
                errx(EX_USAGE, "invalid ELF class");

        auto d = e_datas.find(ident[EI_DATA]);
        if (d == e_datas.end())
                errx(EX_USAGE, "invalid data format");

        if (ident[EI_VERSION] == EV_NONE)
                errx(EX_USAGE, "invalid version");

        auto o = e_osabis.find(ident[EI_OSABI]);
        if (o == e_osabis.end())
                errx(EX_USAGE, "invalid ABI");

        printf("ident[EI_CLASS]   = %s\n", c->second.c_str());
        printf("ident[EI_DATA]    = %s\n", d->second.c_str());
        printf("ident[EI_OSABI]   = %s\n", o->second.c_str());

        Fseek(fp, 0, SEEK_SET);
        if (ident[EI_CLASS] == ELFCLASS32)
                disasm_32(fp);
        else if (ident[EI_CLASS] == ELFCLASS64)
                disasm_64(fp);
}

static void Fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
        auto n = fread(ptr, size, nmemb, stream);
        if (n != nmemb || ferror(stream))
                err(EX_SOFTWARE, "could not read file");
}

static void Fseek(FILE *stream, long offset, int whence)
{
        if (fseek(stream, offset, whence) < 0)
                err(EX_SOFTWARE, "could not seek in file");
}

static void disasm_32(FILE *fp)
{
        // TODO
}

static void disasm_64(FILE *fp)
{
        Elf64_Ehdr hdr;

        Fread(&hdr, sizeof(hdr), 1, fp);

        auto t = e_types.find(hdr.e_type);
        if (t == e_types.end())
                errx(EX_USAGE, "invalid type");

        auto m = e_machines.find(hdr.e_machine);
        if (m == e_machines.end())
                errx(EX_USAGE, "invalid machine");

        printf("e_type            = %s\n", t->second.c_str());
        printf("e_machine         = %s\n", m->second.c_str());
        printf("e_entry           = %#lx\n", hdr.e_entry);
        printf("e_phoff           = %#lx\n", hdr.e_phoff);
        printf("e_shoff           = %#lx\n", hdr.e_shoff);
        printf("e_flags           = %#x\n", hdr.e_flags);
        printf("e_ehsize          = %u\n", hdr.e_ehsize);
        printf("e_phentsize       = %u\n", hdr.e_phentsize);
        printf("e_phnum           = %u\n", hdr.e_phnum);
        printf("e_shentsize       = %u\n", hdr.e_shentsize);
        printf("e_shnum           = %u\n", hdr.e_shnum);
        printf("e_shstrndx        = %u\n\n", hdr.e_shstrndx);

        Fseek(fp, hdr.e_phoff, SEEK_SET);
        printf("program header table = [\n");
        for (uint16_t i = 0; i < hdr.e_phnum; i++) {
                Elf64_Phdr phdr;

                Fread(&phdr, sizeof(phdr), 1, fp);
                printf("\t[%u] = {\n", i);
                printf("\t\tp_type   = %u,\n", phdr.p_type);
                printf("\t\tp_flags  = %u,\n", phdr.p_flags);
                printf("\t\tp_offset = %lu,\n", phdr.p_offset);
                printf("\t\tp_vaddr  = %lu,\n", phdr.p_vaddr);
                printf("\t\tp_paddr  = %lu,\n", phdr.p_paddr);
                printf("\t\tp_filesz = %lu,\n", phdr.p_filesz);
                printf("\t\tp_memsz  = %lu,\n", phdr.p_memsz);
                printf("\t\tp_align  = %lu\n", phdr.p_align);
                printf("\t},\n");
        }
        printf("]\n");
}
