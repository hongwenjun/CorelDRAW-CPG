#ifndef CDRAPP_H_INCLUDED
#define CDRAPP_H_INCLUDED
#include "cdrapi.h"

bool fill_red(corel *cdr);
bool cql_OutlineColor(corel *cdr);
bool cql_FillColor(corel *cdr);
bool cql_SameSize(corel *cdr);
bool Shapes_Filp(corel *cdr);
bool Shapes_Filp(corel *cdr);
bool Clear_Fill(corel *cdr);

void CdrCopy_to_AdobeAI(corel *cdr);
void AdobeAI_Copy_ImportCdr(corel *cdr);

bool Box_AutoGroup(corel *cdr, double exp = 0.0);
bool BBox_DrawRectangle(corel *cdr, double exp = 0.0);

#endif // CDRAPP_H_INCLUDED
