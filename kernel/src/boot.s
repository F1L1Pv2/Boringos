.global _start
_start:
    cli                     # disable interrupts
    mov $0x200000, %rsp     # set stack to 2 MiB

    call kernel_main        # call C entry
halt:
    hlt
    jmp halt