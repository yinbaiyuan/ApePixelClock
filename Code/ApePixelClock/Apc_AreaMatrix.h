#ifndef __APC_AREAMATRIX_H
#define __APC_AREAMATRIX_H


#include <Adafruit_NeoMatrix.h>

class Apc_AreaMatrix: public Adafruit_NeoMatrix
{
  public:
    int offsetX = 0;
    int offsetY = 0;
    int areaWidth = 0;
    int areaHeight = 0;
    Apc_AreaMatrix(uint8_t matrixW, uint8_t matrixH, uint8_t tX,
                   uint8_t tY, uint8_t pin = 6,
                   uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS +
                                        NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS,
                   neoPixelType ledType = NEO_GRB + NEO_KHZ800): Adafruit_NeoMatrix(matrixW, matrixH, tX,
                         tY, pin, matrixType, ledType) {}
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawColorIndexFrame(unsigned long* colorMap,
                             unsigned char width, unsigned char height, unsigned char* pixels);
};



#endif // __APC_AREAMATRIX_H
