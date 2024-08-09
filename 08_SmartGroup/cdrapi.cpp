#include <windows.h>
#include <stdio.h>
#include "cdrapi.h"

void BeginOpt(corel *cdr)
{
  auto name = _bstr_t("Undo");
  cdr->EventsEnabled = false;
  cdr->ActiveDocument->BeginCommandGroup(name);
  cdr->ActiveDocument->Unit = cdrMillimeter;
  cdr->Optimization = true;
}

void EndOpt(corel *cdr)
{
  cdr->EventsEnabled = true;
  cdr->Optimization = false;
  cdr->EventsEnabled = true;
  cdr->ActiveDocument->ReferencePoint = cdrBottomLeft;
  cdr->Application->Refresh();
  cdr->ActiveDocument->EndCommandGroup();
}

void Active_CorelWindows(HWND hDlg)
{                
  // 将焦点返回到父窗口 关闭对话框窗口
  SetFocus(GetParent(hDlg));
}

double GetTextValue(HWND hDlg, int IDITEM) {
    char ibuf[64];
    memset(ibuf, 0, sizeof(ibuf));
    GetWindowText(::GetDlgItem(hDlg, IDITEM), ibuf, sizeof(ibuf));
    double exp = 0.0;
    sscanf_s(ibuf, "%lf", &exp);
    return exp;
}