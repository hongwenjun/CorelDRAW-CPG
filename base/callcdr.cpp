#import "VGCoreAuto.tlb"
#include <Windows.h>

#define corel VGCore::IVGApplication
using namespace VGCore;

void Call_CorelDRAW();
bool fill_red(corel *cdr);

corel *cdr = NULL;
int main() {
  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    try {
      Call_CorelDRAW();

    } catch (_com_error &e) {
      MessageBox(NULL, e.Description(), "Error", MB_ICONSTOP);
    }
    CoUninitialize();
  }
  return 0;
}
void Call_CorelDRAW() {
  IVGApplicationPtr app(L"CorelDRAW.Application.16");  // 我的电脑 X6 正常 ; 毛子版 CorelDRAW 2020  绑定不上，没法使用
  app->Visible = VARIANT_TRUE;

  auto doc = app->ActiveDocument;
  if (!doc)
    doc = app->CreateDocument();

  cdr = app;
  fill_red(cdr);
}

bool fill_red(corel *cdr) {
  auto sr = cdr->ActiveSelectionRange;
  auto red = cdr->CreateCMYKColor(0, 100, 100, 0);
  sr->ApplyUniformFill(red);
  return true;
}