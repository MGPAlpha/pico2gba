#include "pico2gba.h"
#include "cli.h"
#include <stdio.h>

int main() {
    setPico8LocationGlobal("/home/pico-8/pico8");
    char* path1 = "/home/mitchell/.lexaloffle/pico-8/carts/converttest/voidprotocol.p8";
    printf("Checking if file %s is a cartridge\n", path1);
    printf("Result: %d\n", isPico8Cart(path1));
    char* path2 = "/home/mitchell/.lexaloffle/pico-8/carts/demos/drippy.p812312";
    printf("Checking if file %s is a cartridge\n", path2);
    printf("Result: %d\n", isPico8Cart(path2));
    char* path3 = "/home/mitchell/Documents/pico2gba/src/pico2gba.h";
    printf("Checking if file %s is a cartridge\n", path3);
    printf("Result: %d\n", isPico8Cart(path3));
    createConversionFolder(path1);
    cartExportLua(path1);
    cartExportLabel(path1);
}