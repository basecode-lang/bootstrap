#!/bin/sh

# Uses portasm to generates MASM sources for intel platforms.
printf "; auto-generated by `basename $0`\r\n" > dyncall_call_x86_generic_masm.asm
printf "; auto-generated by `basename $0`\r\n" > dyncall_call_x64_generic_masm.asm
gcc -E -P -DGEN_MASM dyncall_call_x86.S | awk '{printf "%s\r\n", $0}' >> dyncall_call_x86_generic_masm.asm
gcc -E -P -DGEN_MASM dyncall_call_x64.S | awk '{printf "%s\r\n", $0}' >> dyncall_call_x64_generic_masm.asm
