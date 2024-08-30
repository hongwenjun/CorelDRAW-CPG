#ifndef CDRAPI_H_INCLUDED
#define CDRAPI_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <algorithm>
#include <cstdio>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>

#import "VGCoreAuto.tlb" \
rename("GetCommandLine", "VGGetCommandLine") \
rename("CopyFile", "VGCore") \
rename("FindWindow", "VGFindWindow")

#define corel VGCore::IVGApplication
using namespace VGCore;



void BeginOpt(corel *cdr);
void EndOpt(corel *cdr);
void Active_CorelWindows(HWND hDlg);
double GetTextValue(HWND hDlg, int IDITEM);
void PutTextValue(HWND hDlg, int IDITEM, char *buf);


#endif // CDRAPI_H_INCLUDED
