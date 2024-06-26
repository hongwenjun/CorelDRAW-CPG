﻿#include <cstdio>
#include <windows.h>
#include "resource.h"

#import "VGCoreAuto.tlb" \
rename("GetCommandLine", "VGGetCommandLine") \
rename("CopyFile", "VGCore") \
rename("FindWindow", "VGFindWindow")

using namespace VGCore;

static HINSTANCE g_hResource = NULL;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hResource = (HINSTANCE)hinstDLL;
    }
    return TRUE;
}

class ToolsBoxPlugin : public VGCore::IVGAppPlugin
{
private:

    volatile ULONG m_ulRefCount;
    long m_lCookie;

    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OpenToolsBox();


public:
    ToolsBoxPlugin();
    VGCore::IVGApplication* m_pApp;

// IUnknown
public:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
    STDMETHOD_(ULONG, AddRef)(void) { return ++m_ulRefCount; }
    STDMETHOD_(ULONG, Release)(void)
    {
        ULONG ulCount = --m_ulRefCount;
        if (ulCount == 0) {
            delete this;
        }
        return ulCount;
    }

// IDispatch
public:
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) { return E_NOTIMPL; }
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) { return E_NOTIMPL; }
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) { return E_NOTIMPL; }
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

// IVGAppPlugin
public:
    STDMETHOD(raw_OnLoad)(VGCore::IVGApplication* Application);
    STDMETHOD(raw_StartSession)();
    STDMETHOD(raw_StopSession)();
    STDMETHOD(raw_OnUnload)();

};

ToolsBoxPlugin::ToolsBoxPlugin()
{
    m_pApp = NULL;
    m_lCookie = 0;
    m_ulRefCount = 1;
}

STDMETHODIMP ToolsBoxPlugin::QueryInterface(REFIID riid, void** ppvObject)
{
    HRESULT hr = S_OK;
    m_ulRefCount++;
    if (riid == IID_IUnknown) {
        *ppvObject = (IUnknown*)this;
    } else if (riid == IID_IDispatch) {
        *ppvObject = (IDispatch*)this;
    } else if (riid == __uuidof(VGCore::IVGAppPlugin)) {
        *ppvObject = (VGCore::IVGAppPlugin*)this;
    } else {
        m_ulRefCount--;
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDMETHODIMP ToolsBoxPlugin::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    switch (dispIdMember) {

    case 0x0014: // DISPID_APP_ONPLUGINCMD
        if (pDispParams != NULL && pDispParams->cArgs == 1) {
            _bstr_t strCmd(pDispParams->rgvarg[0].bstrVal);
            if (strCmd == _bstr_t("OpenToolsBox")) {
           //   MessageBox(NULL, _bstr_t("OpenToolsBox"), _bstr_t("OpenToolsBox"), MB_ICONSTOP);
                OpenToolsBox();
            }
        }
        break;

    case 0x0015: // DISPID_APP_ONPLUGINCMDSTATE
        if (pDispParams != NULL && pDispParams->cArgs == 3) {
            _bstr_t strCmd(pDispParams->rgvarg[2].bstrVal);
            if (strCmd == _bstr_t("OpenToolsBox")) {
                *pDispParams->rgvarg[1].pboolVal = VARIANT_TRUE;
            }
        }
        break;
    }
    return S_OK;
}

STDMETHODIMP ToolsBoxPlugin::raw_OnLoad(VGCore::IVGApplication* Application)
{
    m_pApp = Application;
    if (m_pApp) {
        m_pApp->AddRef();
    }
    return S_OK;
}

STDMETHODIMP ToolsBoxPlugin::raw_StartSession()
{
    try {
        m_pApp->AddPluginCommand(_bstr_t("OpenToolsBox"), _bstr_t("Tools Box"), _bstr_t("打开工具窗口"));

        VGCore::ICUIControlPtr ctl = m_pApp->CommandBars->Item[_bstr_t("Standard")]->Controls->AddCustomButton(VGCore::cdrCmdCategoryPlugins, _bstr_t("OpenToolsBox"), 10, VARIANT_FALSE);
        ctl->SetIcon2(_bstr_t("guid://d2fdc0d9-09f8-4948-944c-4297395c05b7"));
        m_lCookie = m_pApp->AdviseEvents(this);
    } catch (_com_error &e) {
        MessageBox(NULL, e.Description(), _bstr_t("Error"), MB_ICONSTOP);
    }
    return S_OK;
}

STDMETHODIMP ToolsBoxPlugin::raw_StopSession()
{
    try {
        m_pApp->UnadviseEvents(m_lCookie);
        m_pApp->RemovePluginCommand(_bstr_t("OpenToolsBox"));
    } catch (_com_error &e) {
        MessageBox(NULL, e.Description(), _bstr_t("Error"), MB_ICONSTOP);
    }
    return S_OK;
}

STDMETHODIMP ToolsBoxPlugin::raw_OnUnload()
{
    if (m_pApp) {
        m_pApp->Release();
        m_pApp = NULL;
    }
    return S_OK;
}

void ToolsBoxPlugin::OpenToolsBox()
{
    m_pApp->StartupMode = VGCore::cdrStartupDoNothing;

    INT_PTR nHandle = m_pApp->AppWindow->Handle;
    HWND hAppWnd = reinterpret_cast<HWND>(nHandle);

    // 创建非模态对话框
    HWND hDlgWnd = CreateDialogParam(g_hResource, MAKEINTRESOURCE(IDD_TOOLS_BOX), hAppWnd, DlgProc, (LPARAM)m_pApp);
    // 在创建对话框之前存储 m_pApp 指针
    SetWindowLongPtr(hDlgWnd, DWLP_USER, (LONG_PTR)m_pApp);

    // 获取屏幕的宽度和高度
    RECT rect;
    GetWindowRect(GetDesktopWindow(), &rect);
    int screenWidth = rect.right - rect.left;
    int screenHeight = rect.bottom - rect.top;

    // 计算对话框窗口的宽度和高度
    RECT dlgRect;
    GetWindowRect(hDlgWnd, &dlgRect);
    int dlgWidth = dlgRect.right - dlgRect.left;
    int dlgHeight = dlgRect.bottom - dlgRect.top;

    // 计算对话框窗口的左上角坐标,使其居中显示
    int x = (screenWidth - dlgWidth) / 2;
    int y = (screenHeight - dlgHeight) / 2;

    // 设置对话框窗口的位置
    SetWindowPos(hDlgWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    // 设置对话框窗口的父窗口  // #define GWL_HWNDPARENT      (-8)
    SetWindowLong(hDlgWnd, -8, (LONG)hAppWnd);
    // 显示对话框窗口
    ShowWindow(hDlgWnd, SW_SHOW);
}


INT_PTR CALLBACK ToolsBoxPlugin::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 从附加数据中获取 m_pApp 指针
    VGCore::IVGApplication* cdr = reinterpret_cast<VGCore::IVGApplication*>(GetWindowLongPtr(hDlg, DWLP_USER));

    if (uMsg == WM_COMMAND) {
        try {
            switch (LOWORD(wParam)) {
            case IDC_RED : {
                    // sr.ApplyUniformFill CreateCMYKColor(0, 100, 100, 0)
                    auto sr = cdr->ActiveSelectionRange;
                    auto red = cdr->CreateCMYKColor(0, 100, 100, 0);
                    sr->ApplyUniformFill(red);
                    MessageBox(NULL, "选择物件填充红色", "填充颜色测试", MB_ICONSTOP);
                }

                break;

            case IDC_CQL_OUTLINE: {
                    auto col = cdr->CreateCMYKColor(0, 100, 100, 0);
                    auto s = cdr->ActiveShape;
                    col-> CopyAssign (s->Outline->Color);
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
                }

                // 将焦点返回到父窗口 关闭对话框窗口
                SetFocus(GetParent(hDlg));
                break;

            case IDC_CQL_FILL: {
                    auto col = cdr->CreateCMYKColor(0, 100, 100, 0);
                    auto s = cdr->ActiveShape;
                    col-> CopyAssign (s->Fill->UniformColor);
                    col->ConvertToRGB();

                    auto r = col->RGBRed;
                    auto g = col->RGBGreen;
                    auto b = col->RGBBlue;

                    char buf[256] = { 0 };
                    sprintf(buf, "@Fill.Color.rgb[.r='%d' And .g='%d' And .b='%d']", r, g, b);
                    auto cql = _bstr_t(buf);

                    auto sr = cdr->ActivePage->Shapes->FindShapes(_bstr_t(), cdrNoShape, VARIANT_TRUE, cql);
                    sr->CreateSelection();
                }

                // 将焦点返回到父窗口 关闭对话框窗口
                SetFocus(GetParent(hDlg));
                break;


            case IDC_CQL_SIZE: {
                    cdr->ActiveDocument->Unit = cdrMillimeter;
                    auto s = cdr->ActiveShape;

                    char buf[256] = { 0 };
                    sprintf(buf, "@width = {%lf mm} and @height = {%lf mm}", s->SizeWidth, s->SizeHeight);
                    auto cql = _bstr_t(buf);

                //  MessageBox(NULL, cql, "cql 尺寸相同", MB_ICONSTOP);
                    auto sr = cdr->ActivePage->Shapes->FindShapes(_bstr_t(), cdrNoShape, VARIANT_TRUE, cql);
                    sr->CreateSelection();
                }

                // 复制dll改名到CorelDRAW 插件目录
                // copy /y lycpg64.dll  "C:\Program Files\Corel\CorelDRAW Technical Suite X6\Draw\Plugins64\lycpg64.cpg"

                SetFocus(GetParent(hDlg));
                break;

            case IDC_CLEAR_FILL:
                cdr->ActiveSelection->Fill->ApplyNoFill();
                break;

            case IDC_SR_FLIP: {
                    auto sr = cdr->ActiveSelectionRange;
                    // CorelDRAW Shapes 物件 Item 编号从1开始
                    for (auto i = 0; i != sr->Count; i++)
                        sr->Shapes->Item[i + 1]->Flip(VGCore::cdrFlipHorizontal);
                }
                break;

            case IDOK:
            case IDCANCEL:
                EndDialog(hDlg, 0);
                break;
            }

        } catch (_com_error &e) {
            MessageBox(NULL, e.Description(), "Error", MB_ICONSTOP);
        }

    } else if (uMsg == WM_INITDIALOG) {
        return 1;
    }
    return 0;
}

extern "C" __declspec(dllexport) DWORD APIENTRY AttachPlugin(VGCore::IVGAppPlugin** ppIPlugin)
{
    *ppIPlugin = new ToolsBoxPlugin;
    return 0x100;
}
