#!/bin/bash

echo -e "\n\033[1mWhatsAScript Compiler (FOR DEVELOPER)\033[0m"

read -n1 -p "Also compile to Windows? [ ]" confirm
confirm=${confirm,,}
echo -e "\b\b\b$confirm] "

if [ $confirm != "y" ] && [ $confirm != "n" ]; then
    echo -e "  Error: You need to provide a y/n value.\nNo compilation was done.\n"
    exit 1
fi

echo -n "  Compiling to ELF (Linux)..."
g++ src/was.cpp -o release/was-linux -lm -std=c++23

if [ ${confirm,,} == "y" ]; then
    echo -e -n " done\n  Compiling to PE (Windows)..."
    i686-w64-mingw32-g++ src/was.cpp -o release/was-win32.exe -lm -std=c++23
    x86_64-w64-mingw32-g++ src/was.cpp -o release/was-win64.exe -lm -std=c++23
fi

echo -e " done\n"
exit 0

