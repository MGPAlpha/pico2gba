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

int cartSectionToFile(char* path, char* section) {
    FILE* cart = fopen(path, "r");
    char line[256];
    char sectionHeader[16] = "__";
    strcat(sectionHeader, section);
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
        printf("Failed to find section %s in cartridge %s\n", section, path);
        return 1;
    }
    
    char* dirPath = getConversionFolderPath(path);
    // printf("dir path: %s\n", dirPath);
    char* sectionPath = malloc(strlen(dirPath) + 10);
    strcpy(sectionPath, dirPath);
    strcat(sectionPath, section);
    strcat(sectionPath, ".txt");
    printf("output file %s\n", sectionPath);
    FILE* sectionFile = fopen(sectionPath, "w");
    
    while (fgets(line, 255, cart)) {
        int isHeader = isSectionHeader(line);
        if (isHeader) {
            break;
        }
        fputs(line, sectionFile);
    }
    fclose(sectionFile);
    fclose(cart);
    return 0;
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