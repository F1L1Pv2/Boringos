#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"

const char* path_skip_one(const char* str) {
    const char* og = str;
    while(*str && *str != '/') str++;
    return *str ? str+1 : og;
}

int main(int argc, char** argv){
    NOB_GO_REBUILD_URSELF(argc,argv);
    char* build_dir = getenv_or_default("BUILDDIR", "./build");
    mkdir_if_not_exists(build_dir);

    Cmd cmd = { 0 };
    File_Paths objs = { 0 };
    String_Builder stb = { 0 };

    File_Paths dirs = { 0 };
    File_Paths c_sources = { 0 };
    File_Paths asm_sources = { 0 };
    File_Paths pathb = { 0 };

    if(!walk_directory("src",
            { &dirs, .file_type = FILE_DIRECTORY },
            { &c_sources, .ext = ".c" },
            { &asm_sources, .ext = ".s" },
       )) return 1;

    for(size_t i = 0; i < asm_sources.count; ++i) {
        const char* src = asm_sources.items[i];
        const char* out = temp_sprintf("%s/kernel/%s.o", build_dir, path_skip_one(src));
        da_append(&objs, out);
        if(!needs_rebuild1(out, src)) continue;
        cmd_append(&cmd, "x86_64-elf-gcc", "-c", src, "-o", out);
        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    for(size_t i = 0; i < c_sources.count; ++i) {
        const char* src = c_sources.items[i];
        const char* out = temp_sprintf("%s/kernel/%s.o", build_dir, path_skip_one(src));
        da_append(&objs, out);
        if(!c_needs_rebuild1(&stb, &pathb, out, src)) continue;
        cmd_append(&cmd, "x86_64-elf-gcc");
        cmd_append(&cmd,
            "-Wall", "-Wextra", 
            "-ffreestanding", "-fno-pie", "-mno-red-zone", "-fno-stack-protector",
            "-MMD", "-MP",
            "-I../thirdparty/Limine",
            "-c", src, "-o", out,
        );
        if(!cmd_run_sync_and_reset(&cmd)) {  
            char* str = temp_strdup(out);  
            size_t str_len = strlen(str);  
            assert(str_len);  
            str[str_len-1] = 'd';  
            delete_file(str);  
            return 1;  
        }
    }
    
    char* out = temp_sprintf("%s/boringos-kernel", build_dir);
    if(needs_rebuild(out, objs.items, objs.count)) {
        cmd_append(&cmd, "x86_64-elf-gcc", "-T", "linker.ld");
        cmd_append(&cmd, "-nostdlib", "-nostartfiles", "-ffreestanding", "-fno-pie", "-mno-red-zone");
        cmd_append(&cmd, "-o", out);
        da_append_many(&cmd, objs.items, objs.count);
        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    return 0;
}