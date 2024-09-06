// WinData.c
#include "WinData.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_PATH_LENGTH 260

char dirPath[MAX_PATH_LENGTH];
char* GetUserDir(const char* dirName) {
  char userProfile[MAX_PATH_LENGTH];

  if (GetEnvironmentVariable("USERPROFILE", userProfile, MAX_PATH_LENGTH) == 0) {
    return NULL;
  }
  // 构建指定目录的完整路径
  snprintf(dirPath, sizeof(dirPath), "%s\\%s", userProfile, dirName);

  // 检查目录是否存在
  DWORD ftyp = GetFileAttributes(dirPath);
  if (ftyp == INVALID_FILE_ATTRIBUTES) {
    if (CreateDirectory(dirPath, NULL) ||
        GetLastError() == ERROR_ALREADY_EXISTS) {
      printf("目录 '%s' 创建成功或已经存在。\n", dirPath);
    } else {
      return NULL;
    }
  }
  return dirPath;
}

void saveWinData(const char *filename, const WinData *win) {
  char* dir = GetUserDir("lyvba");
  char bufile[MAX_PATH_LENGTH];
  sprintf(bufile, "%s\\%s", dir, filename);
  FILE *file = fopen(bufile, "wb");
  if (!file) {
    //  perror("无法打开文件进行写入");
    return;
  }
  fwrite(win, sizeof(WinData), 1, file);
  fclose(file);
}

WinData loadWinData(const char *filename, WinData win) {
  char* dir = GetUserDir("lyvba");
  char bufile[MAX_PATH_LENGTH];
  sprintf(bufile, "%s\\%s", dir, filename);

  FILE *file = fopen(bufile, "rb");
  if (!file) {
    return win; 
  }
  fread(&win, sizeof(WinData), 1, file);
  fclose(file);
  return win;
}