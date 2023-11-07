#ifndef PICO2GBA_H
#define PICO2GBA_H

void setPico8LocationGlobal(char* location);

void createConversionFolder(char* cartPath);

int isPico8Cart(char* cartPath);

#endif