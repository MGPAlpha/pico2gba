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
    free(luaPath);
    // fputs("", luaFile);

    return 0;
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