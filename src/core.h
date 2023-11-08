#ifndef CORE_H
#define CORE_H

#include <json-c/json.h>
#include <stdio.h>

json_object* openOrCreateJsonFile(const char* path);

char* getConfigPath();

char* getConversionFolderPath(const char* path);

int isSectionHeader(const char* line);

#endif