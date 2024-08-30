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

bool AutoMakeSelection(corel *cdr);
bool Box_AutoGroup(corel *cdr, double exp = 0.0);
bool BBox_DrawRectangle(corel *cdr, double exp = 0.0);

void run_BoundaryGroup(corel *cdr);



extern char infobuf[];

#define GET_BOUNDING_BOX(box)                                                  \
  GetBoundingBox(&(box).x, &(box).y, &(box).w, &(box).h, false)

#define ZERO_4PC 0, 0, 0, 0
 // 左下角坐标 x,y  w,h 宽高
typedef struct {
  double x;
  double y; 
  double w; 
  double h; 
} BoundingBox;

bool isOverlapped(const BoundingBox &a, const BoundingBox &b);
bool BoxGrouping(corel *cdr, IVGShapeRange *sr, IVGShapeRange *srs, double exp );

#endif // CDRAPP_H_INCLUDED
