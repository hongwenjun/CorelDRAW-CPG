﻿#include "cdrapp.h"

#define GET_BOUNDING_BOX(box)                                                  \
  GetBoundingBox(&(box).x, &(box).y, &(box).w, &(box).h, false)

#define ZERO_4PC 0, 0, 0, 0

typedef struct {
  double x; // 左下角 x 坐标
  double y; // 左下角 y 坐标
  double w; // 宽度
  double h; // 高度
} BoundingBox;
bool isDPCurve(IVGShapePtr s) {
  auto dpc = (s->Type == cdrRectangleShape) || (s->Type == cdrEllipseShape) ||
             (s->Type == cdrCurveShape) || (s->Type == cdrPolygonShape) ||
             (s->Type == cdrBitmapShape);
  return dpc;
}
IVGShapePtr CreateBoundary(corel *cdr, IVGShapePtr s) {
  auto scp = s->CreateBoundary(0, 0, true, false);

  //  这个 API X7 以上才支持，所以现在直接画矩形
  // BoundingBox box;
  // s->GET_BOUNDING_BOX(box);
  // auto scp = cdr->ActiveLayer->CreateRectangle2(box.x, box.y, box.w, box.h,
  // ZERO_4PC);

  return scp;
}

// VGCore::IVGShapePtr VGCore::IVGShape::CreateBoundary ( double x, double y,
// VARIANT_BOOL PlaceOnTop, VARIANT_BOOL DeleteSource ); VGCore::IVGShapePtr
// VGCore::IVGShapeRange::CreateBoundary ( double x, double y, VARIANT_BOOL
// PlaceOnTop, VARIANT_BOOL DeleteSource ); VARIANT_BOOL
// VGCore::IVGCurve::IntersectsWith ( struct IVGCurve * Curve )
bool isIntWith(corel *cdr, IVGShape *s1, IVGShape *s2) {
  bool isIn = false;
  if (isDPCurve(s1) && isDPCurve(s2)) {
    isIn = s1->GetDisplayCurve()->IntersectsWith(s2->GetDisplayCurve());
  } else if (isDPCurve(s1)) {
    // 群组文字和OLE等其他类型，创建一个临时边界范围
    auto scp = CreateBoundary(cdr, s2);
    isIn = s1->GetDisplayCurve()->IntersectsWith(scp->GetDisplayCurve());
    scp->Delete();

  } else if (isDPCurve(s2)) {
    auto scp = CreateBoundary(cdr, s1);
    isIn = scp->GetDisplayCurve()->IntersectsWith(s2->GetDisplayCurve());
    scp->Delete();
  } else {
    auto scp = CreateBoundary(cdr, s1);
    auto scp2 = CreateBoundary(cdr, s2);
    isIn = scp->GetDisplayCurve()->IntersectsWith(scp2->GetDisplayCurve());
    scp->Delete();
    scp2->Delete();
  }
  return isIn;
}

// 从矩形边界坐标 获得中心坐标
void calculate_center(const BoundingBox *box, double *cx, double *cy) {
  *cx = box->x + (box->w / 2);
  *cy = box->y + (box->h / 2);
}

// VGCore::cdrPositionOfPointOverShape VGCore::IVGShape::IsOnShape ( double x,
// double y, double HotArea ); VGCore::cdrPositionOfPointOverShape
// VGCore::IVGCurve::IsOnCurve ( double x, double y, double HotArea );

bool BoundaryGroup(corel *cdr, IVGShapeRange *sr, IVGShapeRange *srs) {

  if (sr->Count < 2)
    return false;

  BoundingBox box;
  double x, y;
  int OnSh = 0;
  auto bounds = sr->CreateBoundary(0, 0, true, false); // 建立异性边界物件
  bounds->Fill->UniformColor->RGBAssign(255, 0, 0);    // 填充红色

  auto sbox = bounds->BreakApartEx(); // 把边界 拆分为多个边界 用来分组

  auto srgp = cdr->CreateShapeRange();

  for (int k = 0; k < sbox->Count; k++) {

    for (int i = 0; i < sr->Count; i++) {
      auto sh = sr->Shapes->Item[i + 1];
      sh->GET_BOUNDING_BOX(box);      // 获得物件矩形边界坐标
      calculate_center(&box, &x, &y); // 获得物件中心坐标
      OnSh = sbox->Shapes->Item[k + 1]->IsOnShape(x, y, -1);

      if (OnSh) {
        srgp->Add(sh);
      } else if (isIntWith(cdr, sbox->Shapes->Item[k + 1], sh)) {
        srgp->Add(sh);
      }
    }
    // 从Range中移除已分组的图形
    sr->RemoveRange(srgp);
    srs->Add(srgp->Group());
    srgp->RemoveAll();
  }
  sbox->Delete();
  
  return true;
}

// 测试运行 异形群组
void run_BoundaryGroup(corel *cdr) {
  auto start = std::chrono::high_resolution_clock::now(); // 开始时间
  BeginOpt(cdr);

  auto sr = cdr->ActiveSelectionRange;
  auto srs = cdr->CreateShapeRange();
  int cnt = sr->Count;

  cdr->ActiveDocument->ClearSelection();
  BoundaryGroup(cdr, sr, srs);
  srs->CreateSelection();

  // 计算持续时间
  double runtime = 0.0;
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  runtime = duration.count();

  sprintf(infobuf, "选择物件: %d 个进行异形群组\n群组: %d 组, 时间: %.2f秒",
          cnt, srs->Count, runtime);
  EndOpt(cdr);
}