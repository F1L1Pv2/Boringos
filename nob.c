#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

int main(int argc, char** argv){
    NOB_GO_REBUILD_URSELF(argc, argv);

    char* program = shift_args(&argc, &argv);

    bool run = false;
    while(argc){
        char* arg = shift_args(&argc, &argv);
        if(strcmp(arg, "run") == 0) {
            run = true;
            continue;
        }
        nob_log(NOB_ERROR, "Unknown arg %s", arg);
        return 1;
    }

    Cmd cmd = {0};

    //creating necessary build folders
    mkdir_if_not_exists("build");
    mkdir_if_not_exists("build/kernel");
    mkdir_if_not_exists("build/iso");
    mkdir_if_not_exists("build/iso/boot");

    setenv("BUILDDIR", nob_temp_realpath("./build"), 0);

    //building kernel
    if(!go_run_nob_inside(&cmd, "kernel")) return 1;
    if(needs_rebuild1("build/iso/boringos-kernel", "build/boringos-kernel")){
        if(!copy_file("build/boringos-kernel","build/iso/boringos-kernel")) return 1;
    }

    //creating iso
    if(needs_rebuild1("build/iso/boot/limine.conf", "limine.conf")){
        if(!copy_file("limine.conf", "build/iso/boot/limine.conf")) return 1;
    }

    if(needs_rebuild1("build/iso/boot/limine-uefi-cd.bin", "thirdparty/Limine/limine-uefi-cd.bin")){
        if(!copy_file("thirdparty/Limine/limine-uefi-cd.bin", "build/iso/boot/limine-uefi-cd.bin")) return 1;
    }

    if(needs_rebuild1("build/iso/boot/limine-bios.sys", "thirdparty/Limine/limine-bios.sys")){
        if(!copy_file("thirdparty/Limine/limine-bios.sys", "build/iso/boot/limine-bios.sys")) return 1;
    }

    if(needs_rebuild1("build/iso/boot/limine-bios-cd.bin", "thirdparty/Limine/limine-bios-cd.bin")){
        if(!copy_file("thirdparty/Limine/limine-bios-cd.bin", "build/iso/boot/limine-bios-cd.bin")) return 1;
    }

    if(needs_rebuild1("build/iso/boot/BOOTX64.EFI", "thirdparty/Limine/BOOTX64.EFI")){
        if(!copy_file("thirdparty/Limine/BOOTX64.EFI", "build/iso/boot/BOOTX64.EFI")) return 1;
    }
    
    bool iso_needs_rebuild = needs_rebuild1("build/boringos.iso", "build/iso/boringos-kernel")
                        || needs_rebuild1("build/boringos.iso", "build/iso/boot/limine.conf")
                        || needs_rebuild1("build/boringos.iso", "build/iso/boot/limine-uefi-cd.bin")
                        || needs_rebuild1("build/boringos.iso", "build/iso/boot/limine-bios.sys")
                        || needs_rebuild1("build/boringos.iso", "build/iso/boot/limine-bios-cd.bin")
                        || needs_rebuild1("build/boringos.iso", "build/iso/boot/BOOTX64.EFI");
    
    if(iso_needs_rebuild){
        cmd_append(&cmd,
            "xorriso",
            "-as", "mkisofs",
            "-R", "-r", "-J",
            "-b", "boot/limine-bios-cd.bin",
            "-no-emul-boot",
            "-boot-load-size", "4",
            "-boot-info-table",
            "-hfsplus",
            "-apm-block-size", "2048",
            "--efi-boot", "boot/limine-uefi-cd.bin",
            "-efi-boot-part",
            "--efi-boot-image",
            "--protective-msdos-label",
            "build/iso",
            "-o", "build/boringos.iso"
        );
    
        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    if(run){
        cmd_append(&cmd, "qemu-system-x86_64", "-cdrom", "build/boringos.iso");
        return cmd_run_sync_and_reset(&cmd);
    }

    return 0;
}