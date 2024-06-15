#include <windows.h>
#include "resource.h"

#import "VGCoreAuto.tlb" \
			rename("GetCommandLine", "VGGetCommandLine") \
			rename("CopyFile", "VGCore") \
			rename("FindWindow", "VGFindWindow")


static HINSTANCE g_hResource = NULL;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  if(fdwReason == DLL_PROCESS_ATTACH)
  {
    g_hResource = (HINSTANCE)hinstDLL;
  }
  return TRUE;
}

class CWelcomeScreenPlugin : public VGCore::IVGAppPlugin
{
private:
  VGCore::IVGApplication *m_pApp;
  volatile ULONG m_ulRefCount;
  long m_lCookie;

  static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
  void OnAppStart();

public:
  CWelcomeScreenPlugin();

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

CWelcomeScreenPlugin::CWelcomeScreenPlugin()
{
  m_pApp = NULL;
  m_lCookie = 0;
  m_ulRefCount = 1;
}

STDMETHODIMP CWelcomeScreenPlugin::QueryInterface(REFIID riid, void **ppvObject)
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

STDMETHODIMP CWelcomeScreenPlugin::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
  switch(dispIdMember)
  {
    case 0x0012: //DISPID_APP_START
      OnAppStart();
      break;
  }
  return S_OK;
}

STDMETHODIMP CWelcomeScreenPlugin::raw_OnLoad(VGCore::IVGApplication *Application)
{
  m_pApp = Application;
  if(m_pApp)
  {
    m_pApp->AddRef();
  }
  return S_OK;
}

STDMETHODIMP CWelcomeScreenPlugin::raw_StartSession()
{
  try
  {
    m_lCookie = m_pApp->AdviseEvents(this);
  }
  catch(_com_error &e)
  {
    MessageBox(NULL, e.Description(), "Error", MB_ICONSTOP);
  }
  return S_OK;
}

STDMETHODIMP CWelcomeScreenPlugin::raw_StopSession()
{
  try
  {
    m_pApp->UnadviseEvents(m_lCookie);
  }
  catch(_com_error &e)
  {
    MessageBox(NULL, e.Description(), "Error", MB_ICONSTOP);
  }
  return S_OK;
}

STDMETHODIMP CWelcomeScreenPlugin::raw_OnUnload()
{
  if(m_pApp)
  {
    m_pApp->Release();
    m_pApp = NULL;
  }
  return S_OK;
}

void CWelcomeScreenPlugin::OnAppStart()
{
  try
  {
    m_pApp->StartupMode = VGCore::cdrStartupDoNothing;
	// To avoid 64 bit portability warning, store the long handle value into an INT_PTR
	// before casting it to HWND:

	INT_PTR nHandle = m_pApp->AppWindow->Handle;
	HWND hAppWnd = reinterpret_cast<HWND>(nHandle);
    INT_PTR nRet = DialogBoxParam(g_hResource, MAKEINTRESOURCE(IDD_WELCOME), hAppWnd, DlgProc, (LPARAM)this);
    switch(nRet)
    {
    case IDC_NEWDOC:
      m_pApp->CreateDocument();
      break;

    case IDC_LASTDOC:
      if(m_pApp->RecentFiles->Count > 0)
      {
        m_pApp->OpenDocument(m_pApp->RecentFiles->Item[1]->FullName, 0);
      }
      else
      {
        MessageBox(NULL, "No documents were editied yet.", "Error", MB_ICONSTOP);
      }
      break;
    }
  }
  catch(_com_error &e)
  {
    MessageBox(NULL, e.Description(), "Error", MB_ICONSTOP);
  }
}

INT_PTR CALLBACK CWelcomeScreenPlugin::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND)
  {
    switch(LOWORD(wParam))
    {
      case IDC_NEWDOC:
        EndDialog(hDlg, IDC_NEWDOC);
        break;

      case IDC_LASTDOC:
        EndDialog(hDlg, IDC_LASTDOC);
        break;

      case IDOK:
      case IDCANCEL:
        EndDialog(hDlg, 0);
        break;
    }
  }
  else if(uMsg == WM_INITDIALOG)
  {
    return 1;
  }
  return 0;
}

extern "C" __declspec(dllexport) DWORD APIENTRY AttachPlugin(VGCore::IVGAppPlugin **ppIPlugin)
{
  *ppIPlugin = new CWelcomeScreenPlugin;
  return 0x100;
}
