#include <mach-o/loader.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "page.h"
#include "macho.h"


static inline struct section_64* get_section_pointer(struct segment_command_64* segment, uint32_t sect)
{
    return ((struct section_64*) (((char*) segment) + sizeof(struct segment_command_64))) + sect;
}


static struct mach_header_64* create_header()
{
    struct mach_header_64* header = (struct mach_header_64*) malloc(sizeof(struct mach_header_64));
    
    if (header != NULL)
    {
        header->magic = MH_MAGIC_64;
        header->cputype = CPU_TYPE_X86_64;
        header->cpusubtype = CPU_SUBTYPE_LIB64 | CPU_SUBTYPE_I386_ALL;
        header->filetype = MH_EXECUTE;
        header->ncmds = 0;
        header->sizeofcmds = 0;
        header->flags = MH_NOUNDEFS;
        header->reserved = 0;
    }

    return header;
}


static void init_segment(struct segment_command_64* segment, const char* segname)
{
    segment->cmd = LC_SEGMENT_64;
    segment->cmdsize = sizeof(struct segment_command_64);
    strcpy(segment->segname, segname);
    segment->vmaddr = 0;
    segment->vmsize = 0;
    segment->fileoff = 0;
    segment->filesize = 0;
    segment->maxprot = VM_PROT_NONE;
    segment->initprot = VM_PROT_NONE;
    segment->nsects = 0;
    segment->flags = 0;
}


static void add_section(struct segment_command_64* segment, struct section_64* section, const char* sectname)
{
    segment->cmdsize += sizeof(struct section_64);
    segment->nsects += 1;

    strcpy(section->sectname, sectname);
    strcpy(section->segname, segment->segname);
    section->addr = segment->vmaddr;
    section->size = segment->vmsize;
    section->offset = 0;
    section->align = 4;
    section->reloff = 0;
    section->nreloc = 0;
    section->flags = 0;
    section->reserved1 = 0;
    section->reserved2 = 0;
    section->reserved3 = 0;
}


static struct segment_command_64* create_segment(struct mach_header_64* mh, const char* segname, const char* sectname)
{
    size_t total_size = sizeof(struct segment_command_64);
    if (sectname != NULL)
    {
        total_size += sizeof(struct section_64);
    }

    struct segment_command_64* segment = (struct segment_command_64*) malloc(total_size);
    
    if (segment != NULL)
    {
        init_segment(segment, segname);

        if (sectname != NULL)
        {
            struct section_64* section = get_section_pointer(segment, 0);
            add_section(segment, section, sectname);
        }

        mh->ncmds++;
        mh->sizeofcmds += segment->cmdsize;

    }

    return segment;
}


static size_t count_pages(struct page* page_list)
{
    size_t pages = 0;

    while (page_list != NULL)
    {
        page_list = page_list->next;
        ++pages;
    }

    return pages;
}


static size_t count_size(struct page* page_list)
{
    size_t size = 0;

    while (page_list != NULL)
    {
        size += page_list->size;
        page_list = page_list->next;
    }

    return size;
}


static size_t count_load_commands(struct mach_header_64* header, uint32_t ncmds, ...)
{
    // TODO: use this function instead of passing mh around 
    
    va_list commands;
    header->ncmds = ncmds;

    va_start(commands, ncmds);
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        struct load_command* cmd = va_arg(commands, struct load_command*);
        header->sizeofcmds += cmd->cmdsize;
    }
    va_end(commands);

    return sizeof(struct mach_header_64) + header->sizeofcmds;
}


static struct dyld_info_command* create_dyld_info(struct mach_header_64* mh)
{
    struct dyld_info_command* ldinfo = (struct dyld_info_command*) malloc(sizeof(struct dyld_info_command));

    if (ldinfo != NULL)
    {
        memset(ldinfo, 0, sizeof(struct dyld_info_command));
        ldinfo->cmd = LC_DYLD_INFO_ONLY;
        ldinfo->cmdsize = sizeof(struct dyld_info_command);

        mh->ncmds++;
        mh->sizeofcmds += ldinfo->cmdsize;
    }

    return ldinfo;
}


static struct dylinker_command* create_dyld(struct mach_header_64* mh)
{
    struct dylinker_command* dyld = (struct dylinker_command*) malloc(sizeof(struct dylinker_command) + 20);

    if (dyld != NULL)
    {
        dyld->cmd = LC_LOAD_DYLINKER;
        dyld->cmdsize = sizeof(struct dylinker_command) + 20;
        dyld->name.offset = 12;
        strcpy(((char*) dyld) + sizeof(struct dylinker_command), "/usr/lib/dyld");

        mh->ncmds++;
        mh->sizeofcmds += dyld->cmdsize;
    }

    return dyld;
}


static struct dylib_command* create_dylib(struct mach_header_64* mh)
{
    struct dylib_command* dylib = (struct dylib_command*) malloc(sizeof(struct dylib_command) + 32);

    if (dylib != NULL)
    {
        dylib->cmd = LC_LOAD_DYLIB;
        dylib->cmdsize = sizeof(struct dylib_command) + 32;
        dylib->dylib.name.offset = 24;
        dylib->dylib.timestamp = 2;
        dylib->dylib.current_version = 0x4ca0a01;
        dylib->dylib.compatibility_version = 0x10000;
        strcpy(((char*) dylib) + sizeof(struct dylib_command), "/usr/lib/libSystem.B.dylib");

        mh->ncmds++;
        mh->sizeofcmds += dylib->cmdsize;
    }

    return dylib;
}


static size_t write_load_command(const void* load_command, FILE* stream)
{
    return fwrite(load_command, ((const struct load_command*) load_command)->cmdsize, 1, stream);
}


int write_executable(FILE* output_file, struct page* page_list, size_t page_size, uint64_t data_addr, uint64_t text_addr)
{
    // TODO: handle errors

    uint32_t pages = count_pages(page_list);
    uint32_t code_size = count_size(page_list);

    // Create Mach-O header
    struct mach_header_64* header = create_header();

    // Create null segment
    struct segment_command_64* null_segment = create_segment(header, SEG_PAGEZERO, NULL);
    null_segment->vmaddr = 0x0;
    null_segment->vmsize = data_addr;

    // Create data segment
    struct segment_command_64* data_segment = create_segment(header, SEG_DATA, SECT_DATA);
    data_segment->vmaddr = data_addr;
    data_segment->vmsize = text_addr - data_addr;
    data_segment->maxprot = VM_PROT_READ | VM_PROT_WRITE;
    data_segment->initprot = VM_PROT_READ | VM_PROT_WRITE;

    struct section_64* data_section = get_section_pointer(data_segment, 0);
    data_section->addr = data_addr;
    data_section->size = data_segment->vmsize;
    data_section->flags = S_ZEROFILL;

    // FIXME: We probably want another 4 GB of VM_PROT_NONE data in case cell pointer goes out of bounds
    //        This will require a rewrite of create_segment

    // Create text segment
    struct segment_command_64* text_segment = create_segment(header, SEG_TEXT, SECT_TEXT);
    text_segment->vmaddr = text_addr;
    text_segment->vmsize = pages * page_size; 
    text_segment->fileoff = 0;
    text_segment->filesize = pages * page_size;
    text_segment->maxprot = VM_PROT_ALL;
    text_segment->initprot = VM_PROT_READ | VM_PROT_EXECUTE;
    
    struct section_64* text_section = get_section_pointer(text_segment, 0);
    text_section->addr = text_addr;
    text_section->size = code_size;
    text_section->align = 4;
    text_section->reloff = 0;
    text_section->nreloc = 0;
    text_section->flags = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;

    // Create linkedit segment
    struct segment_command_64* linkedit = create_segment(header, SEG_LINKEDIT, NULL);
    linkedit->fileoff = text_segment->fileoff + text_segment->filesize;
    linkedit->filesize = 0;

    // Create dynamic loader stuff
    struct dyld_info_command* dyldinfo = create_dyld_info(header);
    struct dylinker_command* dyld = create_dyld(header);
    struct dylib_command* dylib = create_dylib(header);

    struct dysymtab_command dysymtab;
    memset(&dysymtab, 0, sizeof(dysymtab));
    dysymtab.cmd = LC_DYSYMTAB;
    dysymtab.cmdsize = sizeof(dysymtab);
    header->ncmds++;
    header->sizeofcmds += dysymtab.cmdsize;

    struct symtab_command symtab;
    memset(&symtab, 0, sizeof(symtab));
    symtab.cmd = LC_SYMTAB;
    symtab.cmdsize = sizeof(symtab);
    header->ncmds++;
    header->sizeofcmds += symtab.cmdsize;

    struct entry_point_command entry_point;
    entry_point.cmd = LC_MAIN;
    entry_point.cmdsize = sizeof(struct entry_point_command);
    entry_point.stacksize = 0;
    header->ncmds++;
    header->sizeofcmds += entry_point.cmdsize;
    
    // Set correct offsets
    entry_point.entryoff = text_section->offset = header->sizeofcmds + sizeof(*header);

    // Write headers to file
    fwrite(header, sizeof(struct mach_header_64), 1, output_file);
    write_load_command(null_segment, output_file);
    write_load_command(data_segment, output_file);
    write_load_command(text_segment, output_file);
    write_load_command(linkedit, output_file);
    write_load_command(dyldinfo, output_file);
    write_load_command(&dysymtab, output_file);
    write_load_command(dyld, output_file);
    write_load_command(dylib, output_file);
    write_load_command(&symtab, output_file);
    write_load_command(&entry_point, output_file);

    // Free resources
    free(header);
    free(null_segment);
    free(text_segment);
    free(linkedit);
    free(data_segment);
    free(dyldinfo);
    free(dyld);
    free(dylib);

    // Write code to file
    while (page_list != NULL)
    {
        fwrite(page_list->data, 1, page_size, output_file);
        page_list = page_list->next;
    }
    
    return 0;
}
