#include "Apc_AreaMatrix.h"

void Apc_AreaMatrix::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (y < 0) || (x >= areaWidth) || (y >= areaHeight)) return;
  Adafruit_NeoMatrix::drawPixel(offsetX + x, offsetY + y, color);
}

void Apc_AreaMatrix::drawColorIndexFrame(unsigned long* colorMap,
    unsigned char width, unsigned char height, unsigned char* pixels)
{
  for (int x = 0; x < width; x++)
  {
    for  (int y = 0; y < height; y++)
    {
      int count = y * width + x;
      unsigned long color = colorMap[pixels[count]];
      this->drawPixel(x, y, this->Color(color >> 16 & 0xFF, color >> 8 & 0xFF, color & 0xFF));
    }
  }
}
