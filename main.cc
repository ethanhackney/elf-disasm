#include <unordered_map>
#include <string>

using std::unordered_map;
using std::string;

#include <elf.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

// The fifth byte identifiers the architecture for this binary
static const unordered_map<unsigned char,string> e_classes {
        { ELFCLASSNONE, "This class is invalid." },
        { ELFCLASS32,   "This defines the 32-bit architecture. It supports machines with files and virtual address spaces up to 4 Gigabytes" },
        { ELFCLASS64,   "This defines the 64-bit architecture" },
};

// The sixth byte specifies the data encoding of the processor-
// specific data in the file. Currently, these encodings are
// supported:
static const unordered_map<unsigned char,string> e_datas {
        { ELFDATANONE,  "Unknown data format" },
        { ELFDATA2LSB,  "Two's complement, little-endian" },
        { ELFDATA2MSB,  "Two's complement, big-endian" },
};

// The seventh byte is the version number of the ELF specification
static const unordered_map<unsigned char,string> e_versions {
        { EV_NONE,      "Invalid version" },
        { EV_CURRENT,   "Current version" },
};

// The eighth byte identifies the operating system and ABI to
// which the object is targeted. Some fields in other ELF
// structures have flags and values that have platform-specific
// meanings; the interpretation of those fields is determined by
// the value of this byte
static const unordered_map<unsigned char,string> e_osabis {
        { ELFOSABI_NONE,        "Unix System V ABI" },
        { ELFOSABI_SYSV,        "Unix System V ABI" },
        { ELFOSABI_HPUX,        "HP-UX ABI" },
        { ELFOSABI_NETBSD,      "NetBSD ABI" },
        { ELFOSABI_LINUX,       "Linux ABI" },
        { ELFOSABI_SOLARIS,     "Solaris ABI" },
        { ELFOSABI_IRIX,        "IRIX ABI" },
        { ELFOSABI_FREEBSD,     "FreeBSD ABI" },
        { ELFOSABI_TRU64,       "TRU64 UNIX ABI" },
        { ELFOSABI_ARM,         "ARM architecture ABI" },
        { ELFOSABI_STANDALONE,  "Stand-alone (embedded) ABI" },
};

// This member of the struct identifies the object file type
static const unordered_map<uint16_t,string> e_types {
        { ET_NONE,      "An unknown type" },
        { ET_REL,       "A relocatable file" },
        { ET_EXEC,      "An executable file" },
        { ET_DYN,       "A shared object" },
        { ET_CORE,      "A core file" },
};

// This member specifies the required architecture for an individual file
static const unordered_map<uint16_t,string> e_machines {
        { EM_NONE,              "An unknown machine" },
        { EM_M32,               "AT&T WE 32100" },
        { EM_SPARC,             "Sun Microsystems SPARC" },
        { EM_386,               "Intel 80386" },
        { EM_68K,               "Motorola 68000" },
        { EM_88K,               "Motorola 88000" },
        { EM_860,               "Intel 80860" },
        { EM_MIPS,              "MIPS RS3000 (big-endian only)" },
        { EM_PARISC,            "HP/PA" },
        { EM_SPARC32PLUS,       "SPARC with enhanced instruction set" },
        { EM_PPC,               "PowerPC" },
        { EM_PPC64,             "PowerPC 64-bit" },
        { EM_S390,              "IBM S/390" },
        { EM_ARM,               "Advanced RISC Machines" },
        { EM_SH,                "Renesas SuperH" },
        { EM_SPARCV9,           "SPARC v9 64-bit" },
        { EM_IA_64,             "Intel Itanium" },
        { EM_X86_64,            "AMD x86-64" },
        { EM_VAX,               "DEC Vax" },
};

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

        auto v = e_versions.find(ident[EI_VERSION]);
        if (v == e_versions.end())
                errx(EX_USAGE, "invalid version");

        auto o = e_osabis.find(ident[EI_OSABI]);
        if (o == e_osabis.end())
                errx(EX_USAGE, "invalid ABI");

        printf("ident[EI_CLASS]   = %s.\n", c->second.c_str());
        printf("ident[EI_DATA]    = %s.\n", d->second.c_str());
        printf("ident[EI_VERSION] = %s.\n", v->second.c_str());
        printf("ident[EI_OSABI]   = %s.\n", o->second.c_str());

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

        printf("e_type            = %s.\n", t->second.c_str());
        printf("e_machine         = %s.\n", m->second.c_str());
}
