#include <windows.h>
#include <tchar.h>

#import "VGCoreAuto.tlb" \
      rename("GetCommandLine", "VGGetCommandLine") \
      rename("CopyFile", "VGCore") \
      rename("FindWindow", "VGFindWindow")

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
  switch (fdwReason)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

class CVGAppPlugin : public VGCore::IVGAppPlugin
{
private:
  VGCore::IVGApplication *m_pApp;
  ULONG m_ulRefCount;
  long m_lCookie;
  bool m_bEnabled;

  bool CheckSelection();
  void OnClearFill();

public:
  CVGAppPlugin();

// IUnknown
public:
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
  STDMETHOD_(ULONG, AddRef)(void) { return ++m_ulRefCount; }
  STDMETHOD_(ULONG, Release)(void)
  {
    ULONG ulCount = --m_ulRefCount;
    if(ulCount == 0)
    {
      delete this;
    }
    return ulCount;
  }

// IDispatch
public:
  STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return E_NOTIMPL; }
  STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
  STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return E_NOTIMPL; }
  STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

// IVGAppPlugin
public:
  STDMETHOD(raw_OnLoad)(VGCore::IVGApplication *Application);
  STDMETHOD(raw_StartSession)();
  STDMETHOD(raw_StopSession)();
  STDMETHOD(raw_OnUnload)();
};

bool CVGAppPlugin::CheckSelection()
{
  bool bRet = false;
  if(m_pApp->Documents->Count > 0)
  {
    bRet = (m_pApp->ActiveSelection->Shapes->Count > 0);
  }
  return bRet;
}

void CVGAppPlugin::OnClearFill()
{
  if(CheckSelection())
  {
    m_pApp->ActiveSelection->Fill->ApplyNoFill();
  }
}

CVGAppPlugin::CVGAppPlugin() :
  m_pApp(NULL),
  m_lCookie(0),
  m_ulRefCount(1),
  m_bEnabled(false)
{
}

STDMETHODIMP CVGAppPlugin::QueryInterface(REFIID riid, void **ppvObject)
{
  HRESULT hr = S_OK;
  m_ulRefCount++;
  if(riid == IID_IUnknown)
  {
    *ppvObject = (IUnknown *)this;
  }
  else if(riid == IID_IDispatch)
  {
    *ppvObject = (IDispatch *)this;
  }
  else if(riid == __uuidof(VGCore::IVGAppPlugin))
  {
    *ppvObject = (VGCore::IVGAppPlugin *)this;
  }
  else
  {
    m_ulRefCount--;
    hr = E_NOINTERFACE;
  }
  return hr;
}

STDMETHODIMP CVGAppPlugin::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
  switch(dispIdMember)
  {
    case 0x0011: //DISPID_APP_SELCHANGE
      m_bEnabled = CheckSelection();
      break;

    case 0x0014: // DISPID_APP_ONPLUGINCMD
      if(pDispParams != NULL && pDispParams->cArgs == 1)
      {
        _bstr_t strCmd(pDispParams->rgvarg[0].bstrVal);
        if(strCmd == _bstr_t("ClearFill"))
        {
          OnClearFill();
        }
      }
      break;

    case 0x0015: // DISPID_APP_ONPLUGINCMDSTATE
      if(pDispParams != NULL && pDispParams->cArgs == 3)
      {
        _bstr_t strCmd(pDispParams->rgvarg[2].bstrVal);
        if(strCmd == _bstr_t("ClearFill"))
        {
          *pDispParams->rgvarg[1].pboolVal = m_bEnabled ? VARIANT_TRUE : VARIANT_FALSE;
        }
      }
      break;
  }
  return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_OnLoad(VGCore::IVGApplication *Application)
{
  m_pApp = Application;
  if(m_pApp)
  {
    m_pApp->AddRef();
  }
  return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_StartSession()
{
  try
  {
    m_pApp->AddPluginCommand(_bstr_t("ClearFill"), _bstr_t("Clear Fill"), _bstr_t("Clears fill from selected objects"));

    // CorelDRAW X6 使用 VGCore::CommandBarControlPtr ctl
    // CorelDRAW 2020 SDK 使用  VGCore::ICUIControlPtr ctl

    // https://github.com/fersatgit/SmartDepart/issues/1    // 感谢 fersatgit 修复CorelDRAW 2020 使用错误
    VGCore::ICUIControlPtr ctl = m_pApp->CommandBars->Item[_bstr_t("Standard")]->Controls->AddCustomButton(VGCore::cdrCmdCategoryPlugins, _bstr_t("ClearFill"), 1, VARIANT_FALSE);
//  _bstr_t bstrPath(m_pApp->Path + _bstr_t("Plugins64\\ClearFill.bmp"));
    ctl->SetIcon2(_bstr_t("guid://d2fdc0d9-09f8-4948-944c-4297395c05b7"));

    m_lCookie = m_pApp->AdviseEvents(this);
  }
  catch(_com_error &e)
  {
    MessageBox(NULL, e.Description(), _T("Error"), MB_ICONSTOP);
  }
  return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_StopSession()
{
  try
  {
    m_pApp->UnadviseEvents(m_lCookie);
    m_pApp->RemovePluginCommand(_bstr_t("ClearFill"));
  }
  catch(_com_error &e)
  {
    MessageBox(NULL, e.Description(), _T("Error"), MB_ICONSTOP);
  }
  return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_OnUnload()
{
  if(m_pApp)
  {
    m_pApp->Release();
    m_pApp = NULL;
  }
  return S_OK;
}

extern "C" __declspec(dllexport) DWORD APIENTRY AttachPlugin(VGCore::IVGAppPlugin **ppIPlugin)
{
  *ppIPlugin = new CVGAppPlugin;
  return 0x100;
}
