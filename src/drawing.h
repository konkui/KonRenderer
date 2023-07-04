#include "geometry.h"
#include "tgaimage.h"
class Drawing {
public:
/**
 * @brief Draw Trianle via sweep line
 * 
 * @param v0 
 * @param v1 
 * @param v2 
 * @param image   
 * @param color The color of triangle
 */
  void DrawTriangle(vec2 v0, vec2 v1, vec2 v2, TGAImage &image, TGAColor color);
  void DrawLine(int x0, int y0, int x1, int y1, TGAImage &image,TGAColor color);
};