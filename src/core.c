#include "pico2gba.h"
#include "core.h"

#include <json-c/json.h>
#include <stdio.h>
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
