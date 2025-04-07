// WinData.h
#ifndef WINDATA_H
#define WINDATA_H

#include <stdio.h>

typedef struct {
    int x;  // x 坐标
    int y;  // y 坐标
    int w;  // 宽度
    int h;  // 高度
    double exp; // 容差值
} WinData;

// 函数声明
void saveWinData(const char* filename, const WinData* win);
WinData loadWinData(const char *filename, WinData win);

#endif // WINDATA_H