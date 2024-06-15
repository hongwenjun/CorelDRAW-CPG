// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <tchar.h>
#import "vgcoreauto.tlb"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

class CVGAppPlugin : public VGCore::IVGAppPlugin
{
private:
    VGCore::IVGApplication *m_pApp;
    ULONG m_ulRefCount;
    long m_lCookie;
    bool m_bEnabled;

public:
    CVGAppPlugin();

    // IUnknown
public:
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    STDMETHOD_(ULONG, AddRef)(void)
    {
        return ++m_ulRefCount;
    }
    STDMETHOD_(ULONG, Release)(void)
    {
        ULONG ulCount = --m_ulRefCount;
        if (ulCount == 0)
        {
            delete this;
        }
        return ulCount;
    }

    // IDispatch
public:
    STDMETHOD(GetTypeInfoCount)(UINT *pctinfo)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

    // IVGAppPlugin
public:
    STDMETHOD(raw_OnLoad)(VGCore::IVGApplication *Application);
    STDMETHOD(raw_StartSession)();
    STDMETHOD(raw_StopSession)();
    STDMETHOD(raw_OnUnload)();
};


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
    if (riid == IID_IUnknown)
    {
        *ppvObject = (IUnknown *)this;
    }
    else if (riid == IID_IDispatch)
    {
        *ppvObject = (IDispatch *)this;
    }
    else if (riid == __uuidof(VGCore::IVGAppPlugin))
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
    return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_OnLoad(VGCore::IVGApplication *Application)
{
    m_pApp = Application;
    if (m_pApp)
    {
        m_pApp->AddRef();
    }
    return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_StartSession()
{
    MessageBox(NULL, _T("插件加载到内存，本CPG使用VC2022 X64编译"), _T("蘭雅CPG64"), MB_ICONSTOP);
    return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_StopSession()
{
    MessageBox(NULL, _T("插件结束"), _T("蘭雅CPG64"), MB_ICONSTOP);
    return S_OK;
}

STDMETHODIMP CVGAppPlugin::raw_OnUnload()
{
    if (m_pApp)
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
