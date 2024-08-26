#include "cdrapp.h"

// sr.ApplyUniformFill CreateCMYKColor(0, 100, 100, 0)
bool fill_red(corel *cdr)
{
    auto sr = cdr->ActiveSelectionRange;
    auto red = cdr->CreateCMYKColor(0, 100, 100, 0);
    sr->ApplyUniformFill(red);

    return true;
}

bool cql_OutlineColor(corel *cdr)
{
    auto col = cdr->CreateCMYKColor(0, 100, 100, 0);
    auto s = cdr->ActiveShape;
    col-> CopyAssign(s->Outline->Color);
    col->ConvertToRGB();

    auto r = col->RGBRed;
    auto g = col->RGBGreen;
    auto b = col->RGBBlue;

    char buf[256] = { 0 };
    sprintf(buf, "@Outline.Color.rgb[.r='%d' And .g='%d' And .b='%d']", r, g, b);
    auto cql = _bstr_t(buf);
    // MessageBox(NULL, cql, "cql 轮廓颜色", MB_ICONSTOP);
    auto sr = cdr->ActivePage->Shapes->FindShapes(_bstr_t(), cdrNoShape, VARIANT_TRUE, cql);
    sr->CreateSelection();

    strcpy(infobuf , buf);
    return true;
}

bool cql_FillColor(corel *cdr)
{
    auto col = cdr->CreateCMYKColor(0, 100, 100, 0);
    auto s = cdr->ActiveShape;
    col-> CopyAssign(s->Fill->UniformColor);
    col->ConvertToRGB();

    auto r = col->RGBRed;
    auto g = col->RGBGreen;
    auto b = col->RGBBlue;

    char buf[256] = { 0 };
    sprintf(buf, "@Fill.Color.rgb[.r='%d' And .g='%d' And .b='%d']", r, g, b);
    auto cql = _bstr_t(buf);

    auto sr = cdr->ActivePage->Shapes->FindShapes(_bstr_t(), cdrNoShape, VARIANT_TRUE, cql);
    sr->CreateSelection();

    strcpy(infobuf , buf);
    return true;
}

bool cql_SameSize(corel *cdr)
{
    cdr->ActiveDocument->Unit = cdrMillimeter;
    auto s = cdr->ActiveShape;

    char buf[256] = { 0 };
    sprintf(buf, "@width = {%lf mm} and @height = {%lf mm}", s->SizeWidth, s->SizeHeight);
    auto cql = _bstr_t(buf);

    //  MessageBox(NULL, cql, "cql 尺寸相同", MB_ICONSTOP);
    auto sr = cdr->ActivePage->Shapes->FindShapes(_bstr_t(), cdrNoShape, VARIANT_TRUE, cql);
    sr->CreateSelection();

    strcpy(infobuf , buf);
    return true;
}

bool Shapes_Filp(corel *cdr)
{
    BeginOpt(cdr);
    auto sr = cdr->ActiveSelectionRange;
    // CorelDRAW Shapes 物件 Item 编号从1开始
    for (auto i = 0; i != sr->Count; i++)
        sr->Shapes->Item[i + 1]->Flip(VGCore::cdrFlipHorizontal);

    EndOpt(cdr);
    return true;
}

bool  Clear_Fill(corel *cdr)
{
    cdr->ActiveSelection->Fill->ApplyNoFill();
    return true;
}
