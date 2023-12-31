#define _GNU_SOURCE

#include "pico2gba.h"
#include "core.h"

#include <json-c/json.h>
#include <stdio.h>
#include <errno.h>
#include "../lib/cfgpath/cfgpath.h"


json_object* openOrCreateJsonFile(const char* path) {
    json_object* root = json_object_from_file(path);
    if (!root) {
        root = json_object_new_object();
        json_object_to_file(path, root);
    }
    return root;
}

void setPico8LocationGlobal(char* location) {
    char* configPath = getConfigPath();
    json_object* configRoot = openOrCreateJsonFile(configPath);
    printf("Global Config located at %s\n", configPath);

    json_object* locationJson = json_object_new_string(location);
    json_object_object_add(configRoot, "pico8_location", locationJson);
    json_object_to_file(configPath, configRoot);
    
    // Cleanup
    json_object_put(configRoot);
    free(configPath);
}

char* getConfigPath() {
    char dir[256];
    get_user_config_folder(dir, 255, "pico2gba");
    char* path = malloc(256);
    sprintf(path, "%sconfig.json", dir);
    return path;
}

int isPico8Cart(char* cartPath) {
    FILE* cart = fopen(cartPath, "r");
    if (!cart) {
        return 0;
    }
    char line[256];
    fgets(line, 256, cart);
    fclose(cart);
    line[16] = 0;
    if (strcmp(line, "pico-8 cartridge")) {
        return 0;
    }
    return 1;
}

char* getConversionFolderPath(const char* path) {
    char* dirPath = malloc(strlen(path) + 5);
    strcpy(dirPath, path);
    strcat(dirPath, ".gba/");
    return dirPath;
}

void createConversionFolder(char* cartPath) {
    int isCart = isPico8Cart(cartPath);
    if (!isCart) {
        printf("Cannot create conversion folder for invalid cartridge at %s\n", cartPath);
    }
    char* dirPath = getConversionFolderPath(cartPath);
    // printf("Dir path: %s\n", dirPath);
    int result = mkdir(dirPath, 0700);
}

int processCartSection(const char* path, const FILE* output, const char* name, int (*process)(const FILE* output, const char* line)) {
    FILE* cart = fopen(path, "r");
    char line[256];
    
    // Find Section Header
    char sectionHeader[16] = "__";
    strcat(sectionHeader, name);
    strcat(sectionHeader, "__\n");
    int sectionFound = 0;
    while(fgets(line, 255, cart)) {
        if (!strcmp(line, sectionHeader)) {
            sectionFound = 1;
            break;
        }
    }
    if (!sectionFound) {
        fclose(cart);
        printf("Failed to find section %s in cartridge %s\n", name, path);
        return 1;
    }
    
    
    while (fgets(line, 255, cart)) {
        int isHeader = isSectionHeader(line);
        if (isHeader) {
            break;
        }
        (*process)(output, line);
    }
    fclose(cart);
    return 0;
}

int directTextProcess(FILE* file, const char* line) {
    fputs(line, file);
}

int charArrayProcess(FILE* file, const char* line) {
    char item[10];
    fputs("    ", file);
    for (char* i = line; *i != 0; i++) {
        snprintf(item, 9, "%d, ", *i);
        fputs(item, file);
    }
    fputs("\n", file);
}

int cartExportLua(const char* path) {

    char* dirPath = getConversionFolderPath(path);
    // printf("dir path: %s\n", dirPath);
    char* luaPath = malloc(strlen(dirPath) + 10);
    strcpy(luaPath, dirPath);
    strcat(luaPath, "code.lua");
    printf("output file %s\n", luaPath);
    FILE* luaFile = fopen(luaPath, "w");

    processCartSection(path, luaFile, "lua", directTextProcess);

    fclose(luaFile);

    strcat(luaPath, ".c");

    FILE* cLuaFile = fopen(luaPath, "w");
    fputs("#include \"code.lua.h\"\n", cLuaFile);
    fputs("\n", cLuaFile);
    fputs("char luaData[] = {\n", cLuaFile);

    processCartSection(path, cLuaFile, "lua", charArrayProcess);

    fputs("    0\n", cLuaFile);
    fputs("};\n", cLuaFile);
    fputs("\n", cLuaFile);
    fputs("AbstractPicoData lua = {luaData, sizeof(luaData)};\n", cLuaFile);
    fclose(cLuaFile);

    strcpy(luaPath, dirPath);
    strcat(luaPath, "code.lua.h");

    FILE* hLuaFile = fopen(luaPath, "w");

    fputs("#include \"picodata.h\"\n", hLuaFile);
    fputs("\n", hLuaFile);
    fputs("AbstractPicoData lua;\n", hLuaFile);

    fclose(hLuaFile);

    free(luaPath);


    return 0;
}

int gfxDataProcess(FILE* file, const char* line) {
    char item[10];
    fputs("    ", file);
    for (char* i = line; *i != 0 && *i != '\n'; i+=2) {
        snprintf(item, 9, "0x%c%c, ", *i, *(i+1));
        fputs(item, file);
    }
    fputs("\n", file);
}

int cartExportLabel(const char* path) {
    char* dirPath = getConversionFolderPath(path);

    char* labelPath = malloc(strlen(dirPath) + 10);
    strcpy(labelPath, dirPath);
    strcat(labelPath, "label.c");

    FILE* labelFile = fopen(labelPath, "w");


    fputs("#include \"label.h\"\n", labelFile);
    fputs("\n", labelFile);
    fputs("char labelData[] = {\n", labelFile);

    processCartSection(path, labelFile, "label", gfxDataProcess);

    // fputs("    0\n", labelFile);
    fputs("};\n", labelFile);
    fputs("\n", labelFile);
    fputs("AbstractPicoData label = {labelData, sizeof(labelData)};\n", labelFile);
    fclose(labelFile);

    labelPath[strlen(labelPath)-1] = 'h';
    FILE* labelHeader = fopen(labelPath, "w");

    fputs("#include \"picodata.h\"\n", labelHeader);
    fputs("\n", labelHeader);
    fputs("AbstractPicoData label;\n", labelHeader);

    fclose(labelHeader);

}

int cartSectionToFile(char* path, char* section) {
    
}

int isSectionHeader(const char* line) {
    int i;
    for (i = 0; i < 2; i++) {
        if (line[i] != '_') return 0;
    }
    while (line[i] && line[i] != '_') {
        i++;
    }
    if (!line[i]) return 0;
    for (int j = 0; j < 2; j++) {
        if (line[i+j] != '_') return 0;
    }
    return 1;

}

char* getPico8LocationGlobal() {
    char* configPath = getConfigPath();
    json_object* configRoot = openOrCreateJsonFile(configPath);

    json_object* pico8LocationJson = json_object_object_get(configRoot, "pico8_location");
    if (pico8LocationJson) {
        char* pico8Location = malloc(json_object_get_string_len(pico8LocationJson)+1);
        strcpy(pico8Location, json_object_get_string(pico8LocationJson));
    } else {
        return NULL;
    }
}