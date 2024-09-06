
#include "cdrapi.h"
#include "cdrapp.h"

extern corel *cdr;
void open_lycpg();
extern "C" __declspec(dllexport)
int __stdcall vbadll(int code, double x =0.0){

try {
    switch(code){
        case 1:
            AdobeAI_Copy_ImportCdr(cdr);
        break;
        
        case 2:
            CdrCopy_to_AdobeAI(cdr);
        break;

        case 3:
            Clear_Fill(cdr);
        break;

        case 4:
            Shapes_Filp(cdr);
        break;

        case 5:
            cql_FillColor(cdr);
        break;

        case 6:
            cql_OutlineColor(cdr);
        break;

        case 7:
            cql_SameSize(cdr);
        break;

        case 8:
            fill_red(cdr);
        break;

        case 9:
            BBox_DrawRectangle(cdr, x);
        break;

        case 10:
            Box_AutoGroup(cdr, x);
        break;

        case 11:
            run_BoundaryGroup(cdr);
        break;

        case 888:
            open_lycpg();
        break;

        default:
            return 0;
    }
} catch (_com_error &e) {
    MessageBox(NULL, e.Description(), "Error", MB_ICONSTOP);
    EndOpt(cdr);
}
    return code;
}