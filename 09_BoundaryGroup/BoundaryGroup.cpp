#include "cdrapp.h"
#include <cerrno>
#include <cmath>
#include <cstddef>

bool isDPCurve(IVGShapePtr s) {
  auto dpc = (s->Type == cdrRectangleShape) || (s->Type == cdrEllipseShape) ||
             (s->Type == cdrCurveShape) || (s->Type == cdrPolygonShape) ||
             (s->Type == cdrBitmapShape);
  return dpc;
}
IVGShapePtr CreateBoundary(corel *cdr, IVGShapePtr s) {
  IVGShapePtr scp;
  if (cdr->VersionMajor < 17) {
    s->CreateSelection();
    cdr->FrameWork->Automation->InvokeItem( _bstr_t("b0491566-5ffe-450a-b17e-f2f496b4eb22"));
    scp = cdr->ActiveSelectionRange->Shapes->Item[1];
  } else {
    //  这个 API X7 以上才支持，所以现在直接画矩形
    scp = s->CreateBoundary(0, 0, true, false);    
  }
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
void calculate_center(const BoundingBox &box, double &cx, double &cy) {
  cx = box.x + (box.w / 2);
  cy = box.y + (box.h / 2);
}

// VGCore::cdrPositionOfPointOverShape VGCore::IVGShape::IsOnShape ( double x,
// double y, double HotArea ); VGCore::cdrPositionOfPointOverShape
// VGCore::IVGCurve::IsOnCurve ( double x, double y, double HotArea );

bool BoundaryGroup(corel *cdr, IVGShapeRange *sr, IVGShapeRange *srs) {

  if (sr->Count < 2)
    return false;

  BoundingBox box, bound_box;
  double x, y;
  int OnSh = 0;
  auto red = cdr->CreateCMYKColor(0, 100, 100, 0);

  // 处理文字和影响的物件
  auto txtbox = cdr->CreateShapeRange();
  auto sr_text =
      sr->Shapes->FindShapes(_bstr_t(), cdrTextShape, VARIANT_TRUE, _bstr_t());
  if (sr_text->Count > 0) {
    auto al = cdr->ActiveLayer;
    for (auto i = 0; i != sr_text->Count; i++) {
      sr_text->Shapes->Item[i + 1]->GET_BOUNDING_BOX(box);
      auto s = al->CreateRectangle2(box.x, box.y, box.w, box.h, ZERO_4PC);
      txtbox->Add(s);
    }
    sr->AddRange(txtbox);
  }


  // auto stmp = sr->CreateBoundary(0, 0, true, false); // 建立异性边界物件
  // auto ef1 = stmp->CreateContour(
  //     cdrContourOutside, 0.2, 1, cdrDirectFountainFillBlend, red, NULL, NULL, 0,
  //     0, cdrContourSquareCap, cdrContourCornerMiteredOffsetBevel, 15);

  // auto bounds = ef1->Separate();
  // stmp->Delete();
  // // bounds->SetOutlineProperties(0.076, NULL, cdr->CreateCMYKColor(100, 50, 0,
  // // 0), NULL, NULL, cdrUndefined,  cdrUndefined, cdrOutlineUndefinedLineCaps,
  // // cdrOutlineUndefinedLineJoin, NULL, NULL, NULL, NULL, NULL);

  // auto sbox = bounds->BreakApartEx();
  // sbox->ApplyUniformFill(red);


  // 建立辅助的异性边界物件，需要填充颜色，搞了半天才搞定
  IVGShapePtr bounds;
  if (cdr->VersionMajor < 17) {
    sr->CreateSelection();
    cdr->FrameWork->Automation->InvokeItem( _bstr_t("b0491566-5ffe-450a-b17e-f2f496b4eb22"));
    bounds = cdr->ActiveSelectionRange->Shapes->Item[1];
    bounds->OrderToFront();
  } else {
    bounds = sr->CreateBoundary(0, 0, true, false); // 建立异性边界物件  
  }

  bounds->Fill->UniformColor->RGBAssign(255, 0, 0);
  auto sbox = bounds->BreakApartEx(); // 把边界 拆分为多个边界 用来分组

  if (sbox->Count > 2) {
    //  VGCore::IVGShapeRange::Sort ( _bstr_t CompareExpression, long
    //  StartIndex, long EndIndex, const _variant_t & Data );
    sbox->Sort(bstr_t( "@shape1.width * @shape1.height > @shape2.width * @shape2.height"),  1, sbox->Count, _variant_t());
  }

  // 删除文字添加的方框
  if (sr_text->Count > 0) {
    sr->RemoveRange(txtbox);
    txtbox->Delete();
  }

  // 按照边界框异形范围进行分组群组
  auto srgp = cdr->CreateShapeRange();

  for (int k = 0; k < sbox->Count && sr->Count; k++) {
    sbox->Shapes->Item[k + 1]->GET_BOUNDING_BOX(bound_box);

    for (int i = 0; i < sr->Count; i++) {
      auto sh = sr->Shapes->Item[i + 1];
      sh->GET_BOUNDING_BOX(box);   // 获得物件矩形边界坐标
      calculate_center(box, x, y); // 获得物件中心坐标
      OnSh = sbox->Shapes->Item[k + 1]->IsOnShape(x, y, -1);

      if (OnSh) {
        srgp->Add(sh);
      } else if (isOverlapped(box, bound_box)) {
        if (isIntWith(cdr, sbox->Shapes->Item[k + 1], sh))
          srgp->Add(sh);
      }
    }

    // 从Range中移除已分组的图形
    sr->RemoveRange(srgp);
    if (srgp->Count > 1) {
      srs->Add(srgp->Group());
    } else {
      srs->AddRange(srgp);
    }
    srgp->RemoveAll();
  }

  // 删除辅助的异性边界物件
  if(!debug_flg)
    sbox->Delete();
  return true;
}

// 测试运行 异形群组
void run_BoundaryGroup(corel *cdr) {
  auto start = std::chrono::high_resolution_clock::now(); // 开始时间
  // if (cdr->VersionMajor < 17) {
  //   sprintf(infobuf, "异形群组目前只支持X7以上版本！");  return; }

  BeginOpt(cdr);

  auto sr = cdr->ActiveSelectionRange;
  auto srs = cdr->CreateShapeRange();
  auto sr_box = cdr->CreateShapeRange();

  int cnt = sr->Count;

  // 取消选择,速度优化
  cdr->ActiveDocument->ClearSelection();

  if (cnt > 300) {
    // 调用矩形分组，分步执行异形群组
    if (BoxGrouping(cdr, sr, sr_box, 0.1)) {
      for (int i = 0; i < sr_box->Count; i++) {
        auto s = sr_box->Shapes->Item[i + 1];
        if (!s->IsSimpleShape) {
          auto sr2 = s->UngroupEx();
          BoundaryGroup(cdr, sr2, srs);
        }
      }
    }
  } else {
    BoundaryGroup(cdr, sr, srs);
  }

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