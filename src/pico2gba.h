#ifndef PICO2GBA_H
#define PICO2GBA_H

void setPico8LocationGlobal(char* location);
char* getPico8LocationGlobal();

void createConversionFolder(char* cartPath);

int isPico8Cart(char* cartPath);

int cartExportLua(const char* path);
int cartExportGfx(const char* path);
int cartExportLabel(const char* path);

#endif