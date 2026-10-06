#ifndef DISPLAY_HUB75_H
#define DISPLAY_HUB75_H

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>
#include "Config_Pins.h"
#include "Config_Params.h"

// Objek Display Matrix DMA & Virtual Canvas
extern MatrixPanel_I2S_DMA *dma_display;
extern VirtualMatrixPanel_T<VIRTUAL_CHAIN_TYPE> *matrix;

// Prototipe Fungsi Display
void initDisplay();
void setMatrixBrightness(uint8_t bright);
void clearMatrix();

#endif // DISPLAY_HUB75_H
