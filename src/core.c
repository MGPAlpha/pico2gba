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
    printf("Dir path: %s\n", dirPath);
    int result = mkdir(dirPath, 0700);
}
