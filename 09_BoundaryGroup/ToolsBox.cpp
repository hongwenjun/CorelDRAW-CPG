#include "cdrapi.h"
#include "cdrapp.h"
#include "resource.h"
#include <stdio.h>
#include <windows.h>


corel *cdr = NULL;
static HINSTANCE g_hResource = NULL;
HICON  g_hIcon;         // 窗口图标句柄
bool debug_flg = false; // 调试->高级模式
char infobuf[256] = {0};
BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    g_hResource = (HINSTANCE)hinstDLL;
  }
  return TRUE;
}

class ToolsBoxPlugin : public VGCore::IVGAppPlugin {
private:
  volatile ULONG m_ulRefCount;
  long m_lCookie;
  static intptr_t CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
  ToolsBoxPlugin();
  VGCore::IVGApplication *m_pApp;
  void OpenToolsBox();

  // IUnknown
public:
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
  STDMETHOD_(ULONG, AddRef)(void) { return ++m_ulRefCount; }
  STDMETHOD_(ULONG, Release)(void) {
    ULONG ulCount = --m_ulRefCount;
    if (ulCount == 0) {
      delete this;
    }
    return ulCount;
  }

  // IDispatch
public:
  STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return E_NOTIMPL; }
  STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) {
    return E_NOTIMPL;
  }
  STDMETHOD(GetIDsOfNames)
  (REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) {
    return E_NOTIMPL;
  }
  STDMETHOD(Invoke)
  (DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

  // IVGAppPlugin
public:
  STDMETHOD(raw_OnLoad)(VGCore::IVGApplication *Application);
  STDMETHOD(raw_StartSession)();
  STDMETHOD(raw_StopSession)();
  STDMETHOD(raw_OnUnload)();
};

ToolsBoxPlugin::ToolsBoxPlugin() {
  m_pApp = NULL;
  m_lCookie = 0;
  m_ulRefCount = 1;
}

STDMETHODIMP ToolsBoxPlugin::QueryInterface(REFIID riid, void **ppvObject) {
  HRESULT hr = S_OK;
  m_ulRefCount++;
  if (riid == IID_IUnknown) {
    *ppvObject = (IUnknown *)this;
  } else if (riid == IID_IDispatch) {
    *ppvObject = (IDispatch *)this;
  } else if (riid == __uuidof(VGCore::IVGAppPlugin)) {
    *ppvObject = (VGCore::IVGAppPlugin *)this;
  } else {
    m_ulRefCount--;
    hr = E_NOINTERFACE;
  }
  return hr;
}

STDMETHODIMP ToolsBoxPlugin::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) {
  switch (dispIdMember) {

  case 0x0014: // DISPID_APP_ONPLUGINCMD
    if (pDispParams != NULL && pDispParams->cArgs == 1) {
      _bstr_t strCmd(pDispParams->rgvarg[0].bstrVal);
      if (strCmd == _bstr_t("OpenToolsBox")) {
        //   MessageBox(NULL, _bstr_t("OpenToolsBox"), _bstr_t("OpenToolsBox"),
        //   MB_ICONSTOP);
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

STDMETHODIMP ToolsBoxPlugin::raw_OnLoad(VGCore::IVGApplication *Application) {
  m_pApp = Application;
  if (m_pApp) {
    m_pApp->AddRef();
  }
  return S_OK;
}

ToolsBoxPlugin* lycpg = nullptr;
STDMETHODIMP ToolsBoxPlugin::raw_StartSession() {
  // 接口转交给cdr
  cdr = m_pApp;
  lycpg = this;

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

STDMETHODIMP ToolsBoxPlugin::raw_StopSession() {
  try {
    m_pApp->UnadviseEvents(m_lCookie);
    m_pApp->RemovePluginCommand(_bstr_t("OpenToolsBox"));
  } catch (_com_error &e) {
    MessageBox(NULL, e.Description(), _bstr_t("Error"), MB_ICONSTOP);
  }
  return S_OK;
}

STDMETHODIMP ToolsBoxPlugin::raw_OnUnload() {
  if (m_pApp) {
    m_pApp->Release();
    m_pApp = NULL;
  }
  return S_OK;
}

void ToolsBoxPlugin::OpenToolsBox() {
  m_pApp->StartupMode = VGCore::cdrStartupDoNothing;

  intptr_t nHandle = m_pApp->AppWindow->Handle;
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
  int w = dlgRect.right - dlgRect.left;
  int h = dlgRect.bottom - dlgRect.top;

  // 计算对话框窗口的左上角坐标,使其居中显示
  int x = (screenWidth - w) / 2;
  int y = (screenHeight - h) / 2;

  double exp = 0.0; // 默认初始容差值

  // 创建窗口数据实例      // 从文件加载
  WinData win = {x, y, w, h, exp};
  win = loadWinData("window.dat", win);

  // 设置对话框窗口的位置
  SetWindowPos(hDlgWnd, NULL, win.x, win.y, win.w, win.h, SWP_NOZORDER | SWP_NOACTIVATE);
  // 设置对话框窗口的父窗口  // #define GWL_HWNDPARENT      (-8)
  SetWindowLong(hDlgWnd, -8, (LONG)hAppWnd);

  // 设置容差值 文本框的数值
  char expbuf[64] = {0};
  sprintf(expbuf, "%.1f", win.exp);
  SetWindowText(::GetDlgItem(hDlgWnd, EXP_TEXT), expbuf); // 设置为需要的数值

  // 显示对话框窗口
  ShowWindow(hDlgWnd, SW_SHOW);

  // 使用 g_hResource 作为 HINSTANCE
  g_hIcon = ::LoadIcon(g_hResource, MAKEINTRESOURCE(IDI_ICON1));
  // 小图标：就是窗口左上角对应的那个图标
  ::SendMessage(hDlgWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hIcon);

}

// MessageBox(NULL, "更新提示信息: 激活CorelDRAW窗口", "CPG代码测试", MB_ICONSTOP);
#define UPDATE_INFO_ACTIVE_CDRWND                                              \
  PutTextValue(hDlg, INFO_TEXT, infobuf);                                      \
  Active_CorelWindows(hDlg);
intptr_t CALLBACK ToolsBoxPlugin::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  // 从附加数据中获取 m_pApp 指针
  VGCore::IVGApplication *cdr = reinterpret_cast<VGCore::IVGApplication *>(
      GetWindowLongPtr(hDlg, DWLP_USER));

  if (uMsg == WM_COMMAND) {
    try {
      switch (LOWORD(wParam)) {

      case ID_BOUNDARY_GROUP: {

        if (BST_CHECKED == IsDlgButtonChecked(hDlg, DEBUG_FLG))
          debug_flg = true;
        else
          debug_flg = false;
        double exp = GetTextValue(hDlg, EXP_TEXT);

        run_BoundaryGroup(cdr);
        UPDATE_INFO_ACTIVE_CDRWND
      } break;

      case IDC_BOX_GROUP: {
        double exp = GetTextValue(hDlg, EXP_TEXT);
        AutoMakeSelection(cdr);
        Box_AutoGroup(cdr, exp);
        UPDATE_INFO_ACTIVE_CDRWND

      } break;

      case IDC_CQL_OUTLINE:
        cql_OutlineColor(cdr);

        UPDATE_INFO_ACTIVE_CDRWND
        break;

      case IDC_CQL_FILL:
        cql_FillColor(cdr);

        UPDATE_INFO_ACTIVE_CDRWND
        break;

      case IDC_CQL_SIZE:
        cql_SameSize(cdr);

        UPDATE_INFO_ACTIVE_CDRWND
        break;

      case IDC_CLEAR_FILL: {
        double exp = GetTextValue(hDlg, EXP_TEXT);
        AutoMakeSelection(cdr);
        BBox_DrawRectangle(cdr, exp);

        UPDATE_INFO_ACTIVE_CDRWND
      } break;

      case IDC_SR_FLIP:
        Shapes_Filp(cdr);
      break;

      case IDC_CDR2AI: {
        CdrCopy_to_AdobeAI(cdr);
        sprintf(infobuf, "把CorelDRAW软件中选择物件复制到剪贴板，请切换到AI软件粘贴");
        UPDATE_INFO_ACTIVE_CDRWND
      } break;

      case IDC_AI2CDR: {
        AdobeAI_Copy_ImportCdr(cdr);
        sprintf(infobuf, "请先在AI软件选择物件复制，再切换到CorelDRAW软件点执行本功能");
        UPDATE_INFO_ACTIVE_CDRWND
      } break;

//////////// 窗口扩展、最小化、恢复按钮按钮////////////////////////////////////////////////
      case EXPAND_TOOLS: {
        // 获取当前窗口的句柄
        HWND hwnd = GetActiveWindow();
        // 获取当前窗口的矩形区域
        RECT rect;
        GetWindowRect(hwnd, &rect);
        // 计算新的宽度
        int newWidth = rect.right - rect.left + 120; // 增加120单位
        // 移动窗口到新的大小
        SetWindowPos(hwnd, NULL, rect.left, rect.top, newWidth, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
        // 隐藏按钮 (假设按钮的句柄为 buttonHandle)
        ShowWindow(::GetDlgItem(hDlg, EXPAND_TOOLS), SW_HIDE);
      } break;

      case MIN_TOOLS: {
        RECT rect;
        GetWindowRect(hDlg, &rect);
        int currentWidth = rect.right - rect.left;
        int currentHeight = rect.bottom - rect.top;
        int h = 98;
        SetWindowPos(hDlg, NULL, rect.left, rect.top, currentWidth, h, SWP_NOZORDER | SWP_NOACTIVATE);
        ShowWindow(::GetDlgItem(hDlg, MIN_TOOLS), SW_HIDE);
        double exp = GetTextValue(hDlg, EXP_TEXT);

        int x = rect.left;
        int y = rect.top;
        int w = currentWidth;
        // 保存窗口位置
        WinData win = {x, y, w, h, exp};
        saveWinData("window.dat", &win);
      } break;

      case RENEW_TOOLS: {
        RECT rect;
        GetWindowRect(hDlg, &rect);
        int x = rect.left;
        int y = rect.top;
        int h = 240; // 恢复宽高
        int w = 220;
        SetWindowPos(hDlg, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
        ShowWindow(::GetDlgItem(hDlg, MIN_TOOLS), !SW_HIDE);
        ShowWindow(::GetDlgItem(hDlg, EXPAND_TOOLS), !SW_HIDE);
        double exp = GetTextValue(hDlg, EXP_TEXT);

        // 保存窗口位置
        WinData win = {x, y, w, h, exp};
        saveWinData("window.dat", &win);

      } break;

      case IDOK:
        break;

       // 关闭CPG插件，保存窗口位置
      case IDCANCEL:{

        RECT rect;
        GetWindowRect(hDlg, &rect);
        int w = rect.right - rect.left;
        int h = rect.bottom - rect.top;
        int x = rect.left;
        int y = rect.top;
        double exp = GetTextValue(hDlg, EXP_TEXT);

        // 保存窗口位置
        WinData win = {x, y, w, h, exp};
        saveWinData("window.dat", &win);

        EndDialog(hDlg, 0);
      } break;

      }

    } catch (_com_error &e) {
      MessageBox(NULL, e.Description(), "Error", MB_ICONSTOP);
      EndOpt(cdr);
    }

  } else if (uMsg == WM_INITDIALOG) {
    SetWindowText(::GetDlgItem(hDlg, EXP_TEXT), "0");
    return 1;
  }
  return 0;
}

extern "C" __declspec(dllexport) DWORD APIENTRY AttachPlugin(VGCore::IVGAppPlugin **ppIPlugin) {
  *ppIPlugin = new ToolsBoxPlugin;
  return 0x100;
}

void open_lycpg() {
    if (lycpg) {
        lycpg->OpenToolsBox(); // 使用类的实例调用成员函数
    }
}