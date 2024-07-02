#ifndef CDRAPI_H_INCLUDED
#define CDRAPI_H_INCLUDED

#import "VGCoreAuto.tlb" \
rename("GetCommandLine", "VGGetCommandLine") \
rename("CopyFile", "VGCore") \
rename("FindWindow", "VGFindWindow")

#define corel VGCore::IVGApplication
using namespace VGCore;


void BeginOpt(corel *cdr);
void EndOpt(corel *cdr);
void Active_CorelWindows(HWND hDlg);

#endif // CDRAPI_H_INCLUDED
