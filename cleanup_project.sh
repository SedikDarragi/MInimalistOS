#!/bin/bash
# Cleanup script to remove unnecessary and redundant files

echo "Cleaning up project folder..."

rm -f kernel/process_simple.c
rm -f kernel/syscall_simple.c
rm -f kernel/simple_kmain.c
rm -f kernel/shell_new.c
rm -f kernel/syscall_wrap.c

# Remove redundant build scripts and text files
rm -f quick_build
rm -f quick_build.sh
rm -f make.txt

echo "Cleanup complete."