#include <tchar.h>
#import "vgcoreauto.tlb"

// DllMain是动态链接库的入口函数
// hModule: 动态链接库的模块句柄
// ul_reason_for_call: 调用原因，指示动态链接库被加载或卸载的原因
// lpReserved: 保留参数，通常设为NULL
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    // 根据调用原因进行不同的操作
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: // 动态链接库被加载到进程地址空间时调用
    case DLL_THREAD_ATTACH: // 创建线程时调用
    case DLL_THREAD_DETACH: // 线程退出时调用
    case DLL_PROCESS_DETACH: // 动态链接库被卸载时调用
        break;
    }
    // 成功处理所有调用原因，返回TRUE
    return TRUE;
}

// CVGAppPlugin类实现了VGCore::IVGAppPlugin接口，用于插件开发
class CVGAppPlugin : public VGCore::IVGAppPlugin
{
private:
    VGCore::IVGApplication *m_pApp; // 应用程序接口指针
    ULONG m_ulRefCount; // 对象引用计数
    long m_lCookie; // 用于标识插件的cookie
    bool m_bEnabled; // 插件是否启用的标志

public:
    // 默认构造函数
    CVGAppPlugin();

    // IUnknown  接口实现
    // QueryInterface用于获取接口指针
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    // AddRef增加对象的引用计数
    STDMETHOD_(ULONG, AddRef)(void)
    {
        return ++m_ulRefCount;
    }
    // Release减少对象的引用计数，当计数为0时释放对象
    STDMETHOD_(ULONG, Release)(void)
    {
        ULONG ulCount = --m_ulRefCount;
        if (ulCount == 0)
        {
            delete this;
        }
        return ulCount;
    }

    // IDispatch 接口实现
    // GetTypeInfoCount返回类型信息的数量
    STDMETHOD(GetTypeInfoCount)(UINT *pctinfo)
    {
        return E_NOTIMPL;
    }
    // GetTypeInfo获取类型信息
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
    {
        return E_NOTIMPL;
    }
    // GetIDsOfNames获取指定名称的成员或变量的ID
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
    {
        return E_NOTIMPL;
    }
    // Invoke调用对象的成员函数
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

    // IVGAppPlugin接口实现
    STDMETHOD(raw_OnLoad)(VGCore::IVGApplication *Application);
    STDMETHOD(raw_StartSession)();
    STDMETHOD(raw_StopSession)();
    STDMETHOD(raw_OnUnload)();
};

// 默认构造函数
CVGAppPlugin::CVGAppPlugin() :
    m_pApp(NULL),
    m_lCookie(0),
    m_ulRefCount(1),
    m_bEnabled(false)
{

}

// QueryInterface实现，用于获取接口指针
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

// Invoke方法的实现，目前仅返回S_OK
STDMETHODIMP CVGAppPlugin::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    return S_OK;
}

// raw_OnLoad方法，在插件加载时被调用
STDMETHODIMP CVGAppPlugin::raw_OnLoad(VGCore::IVGApplication *Application)
{
    m_pApp = Application;
    if (m_pApp)
    {
        m_pApp->AddRef();
    }
    return S_OK;
}

// raw_StartSession方法，启动会话时被调用
STDMETHODIMP CVGAppPlugin::raw_StartSession()
{
    MessageBox(NULL, _T("插件加载到内存，本CPG使用VC2022 X64编译"), _T("蘭雅CPG64"), MB_ICONSTOP);
    return S_OK;
}

// raw_StopSession方法，结束会话时被调用
STDMETHODIMP CVGAppPlugin::raw_StopSession()
{
    MessageBox(NULL, _T("插件结束"), _T("蘭雅CPG64"), MB_ICONSTOP);
    return S_OK;
}

// raw_OnUnload方法，在插件卸载时被调用
STDMETHODIMP CVGAppPlugin::raw_OnUnload()
{
    if (m_pApp)
    {
        m_pApp->Release();
        m_pApp = NULL;
    }
    return S_OK;
}

// AttachPlugin函数用于将插件附加到应用程序中
// ppIPlugin: 插件接口指针的地址
// 返回值: 附加操作的结果，这里固定返回0x100
extern "C" __declspec(dllexport) DWORD APIENTRY AttachPlugin(VGCore::IVGAppPlugin **ppIPlugin)
{
    *ppIPlugin = new CVGAppPlugin;
    return 0x100;
}