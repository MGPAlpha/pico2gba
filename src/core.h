#ifndef CORE_H
#define CORE_H

#include <json-c/json.h>

json_object* openOrCreateJsonFile(const char* path);

char* getConfigPath();

#endif