#include <bits/stdint-uintn.h>
#include <sys/types.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
const TGAColor white = TGAColor{255, 255, 255, 255};
const TGAColor red = TGAColor{255, 0, 0, 255};
void DrawLine(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
void DrawGeometry();
void DrawTriangle(vec2 v0, vec2 v1, vec2 v2, TGAImage &image, TGAColor color);
void DrawTriangle(vec2 (&vecList)[3], TGAImage &image, TGAColor color);
void DrawVec(vec2, TGAImage &, TGAColor);
vec3 CrossProduct(const vec3 v1, const vec3 v2);
vec3 Barycentric(const vec2 (&triangleVecList)[3], const vec2 p);
void logTime();
int main(int argc, char **argv) {
  DrawGeometry();
  return 0;
}
// Bresenhamm's line algorithm
void DrawLine(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
  bool steep = false;
  //如果线段是陡峭的就转置线段
  if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
    steep = true;
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  //确定从左向右画
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  int y = y0;
  int detlax = x1 - x0;
  int detlay = y1 - y0;
  int dError = std::abs(detlay) * 2;
  int error = 0;
  for (int x = x0; x < x1; x++) {
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
    error += dError;
    if (error > detlax) {
      y += (y1 > y0 ? 1 : -1);
      error -= detlax * 2;
    }
  }
}
void DrawGeometry() {
  Model model{"obj/african_head.obj"};
  int width = 1000;
  int height = 1000;
  TGAImage image(width, height, TGAImage::RGB);
  vec2 screenCroods[3];
  vec3 worldCroods[3];
  vec3 lightDir{0,0,-1};
  for (int i = 0; i < model.nfaces(); i++) {
    for (int j = 0; j < 3; j++) {
      vec3 v0 = model.vert(i, j);
      double x0 = (v0.x + 1.) * width / 2.;
      double y0 = (v0.y + 1.) * height / 2.;
      screenCroods[j] = vec2{x0, y0};
      worldCroods[j]=v0;
    }
    vec3 n = CrossProduct(worldCroods[2] - worldCroods[0], worldCroods[1] - worldCroods[0]).normalized();
    auto intensity = n * lightDir;
    if (intensity > 0) {
    uint8_t rgbValue = intensity * 255;
    DrawTriangle(screenCroods, image,TGAColor{rgbValue,rgbValue,rgbValue});
    }
  }
  image.write_tga_file("test/geometry.tga");
}

void DrawTriangle(vec2 v0, vec2 v1, vec2 v2, TGAImage &image, TGAColor color) {
  if (v0.y > v1.y) std::swap(v0, v1);
  if (v0.y > v2.y) std::swap(v0, v2);
  if (v1.y > v2.y) std::swap(v1, v2);
  for (int y = v0.y; y <= v2.y; y++) {
    bool second_half = y > v1.y;
    int total_height = v2.y - v0.y;
    int segment_height = second_half ? v2.y - v1.y : v1.y - v0.y;
    float alpha = 1 - (v2.y - y) / total_height;
    float beta = 1 - (second_half ? v2.y - y : v1.y - y) / segment_height;
    vec2 v_left = v0 + (v2 - v0) * alpha;
    vec2 v_right = second_half ? v1 + (v2 - v1) * beta : v0 + (v1 - v0) * beta;
    if (v_left.x > v_right.x) std::swap(v_left, v_right);
    for (int x = v_left.x; x <= v_right.x; x++) {
      image.set(x, y, color);
    }
  }
}
void DrawTriangle(vec2 (&vecList)[3], TGAImage &image, TGAColor color) {
  vec2 boundingBoxMin{image.width() - 1., image.height() - 1.};
  vec2 boundingBoxMax{0, 0};
  vec2 detla{image.width() - 1., image.height() - 1.};
  // get bounding box
  for (int i = 0; i < 3; i++) {
    boundingBoxMin.x = std::max(0, (int)std::min(vecList[i].x, boundingBoxMin.x));
    boundingBoxMin.y = std::max(0, (int)std::min(vecList[i].y, boundingBoxMin.y));
    boundingBoxMax.x = std::min(detla.x, std::max(vecList[i].x, boundingBoxMax.x));
    boundingBoxMax.y = std::min(detla.y, std::max(vecList[i].y, boundingBoxMax.y));
  }
  vec2 p;
  for (int y = boundingBoxMax.y; y >= boundingBoxMin.y; y--) {
    for (int x = boundingBoxMin.x; x <= boundingBoxMax.x; x++) {
      p.x = x;
      p.y = y;
      vec3 u = Barycentric(vecList, p);
      if (u.z < 0 || u.x < 0 || u.y < 0) continue;
      image.set(p.x, p.y, color);
    }
  }
}
void DrawVec(vec2 vec, TGAImage &image, TGAColor color) { image.set(vec.x, vec.y, color); }
vec3 Barycentric(const vec2 (&triangleVecList)[3], const vec2 p) {
  vec3 pa{triangleVecList[1].x - triangleVecList[0].x, triangleVecList[2].x - triangleVecList[0].x,
          triangleVecList[0].x - p.x};
  vec3 pb{triangleVecList[1].y - triangleVecList[0].y, triangleVecList[2].y - triangleVecList[0].y,
          triangleVecList[0].y - p.y};
  vec3 u = CrossProduct(pa, pb);
  if (std::abs(u.z) < 1) return vec3{-1, 1, 1};
  return vec3{u.x / u.z, u.y / u.z, 1 - (u.x + u.y) / u.z};
}
vec3 CrossProduct(const vec3 v1, const vec3 v2) {
  // 判断两个向量是否为三维向量
  if (std::isnan(v1.z) || std::isnan(v2.z)) {
    throw std::invalid_argument("Invalid input");
  }

  vec3 result;
  result.x = v1.y * v2.z - v1.z * v2.y;
  result.y = v1.z * v2.x - v1.x * v2.z;
  result.z = v1.x * v2.y - v1.y * v2.x;
  return result;
}
void logTime() {
  // 获取当前时间
  auto now = std::chrono::system_clock::now();

  // 将时间转换为时间点（可选）
  auto now_tp = std::chrono::system_clock::to_time_t(now);

  // 打印时间
  std::cout << "当前时间: " << std::ctime(&now_tp);
}