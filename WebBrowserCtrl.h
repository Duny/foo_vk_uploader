    #pragma once

// web browser type libraries
#import "shdocvw.dll" // web browser control
#import "mshtml.tlb" // web browser dom

class vk_login_dialog; // forward declaration


class DWebBrowserEventsImpl : public DWebBrowserEvents
{

	// IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

	// IDispatch methods
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo,
            LCID lcid,
            ITypeInfo** ppTInfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid,
            LPOLESTR* rgszNames,
            UINT cNames,
            LCID lcid,
            DISPID* rgDispId);
	STDMETHOD(Invoke)(DISPID dispIdMember,
            REFIID riid,
            LCID lcid,
            WORD wFlags,
            DISPPARAMS __RPC_FAR *pDispParams,
            VARIANT __RPC_FAR *pVarResult,
            EXCEPINFO __RPC_FAR *pExcepInfo,
            UINT __RPC_FAR *puArgErr);

    // Methods:
    HRESULT BeforeNavigate (
        _bstr_t URL,
        long Flags,
        _bstr_t TargetFrameName,
        VARIANT * PostData,
        _bstr_t Headers,
        VARIANT_BOOL * Cancel );

    HRESULT NavigateComplete ( _bstr_t URL );
    HRESULT StatusTextChange ( _bstr_t Text );
    HRESULT ProgressChange (
        long Progress,
        long ProgressMax );
    HRESULT DownloadComplete();
    HRESULT CommandStateChange (
        long Command,
        VARIANT_BOOL Enable );
    HRESULT DownloadBegin ();
    HRESULT NewWindow (
        _bstr_t URL,
        long Flags,
        _bstr_t TargetFrameName,
        VARIANT * PostData,
        _bstr_t Headers,
        VARIANT_BOOL * Processed );
    HRESULT TitleChange ( _bstr_t Text );
    HRESULT FrameBeforeNavigate (
        _bstr_t URL,
        long Flags,
        _bstr_t TargetFrameName,
        VARIANT * PostData,
        _bstr_t Headers,
        VARIANT_BOOL * Cancel );
    HRESULT FrameNavigateComplete (
        _bstr_t URL );
    HRESULT FrameNewWindow (
        _bstr_t URL,
        long Flags,
        _bstr_t TargetFrameName,
        VARIANT * PostData,
        _bstr_t Headers,
        VARIANT_BOOL * Processed );
    HRESULT Quit (
        VARIANT_BOOL * Cancel );
    HRESULT WindowMove ( );
    HRESULT WindowResize ( );
    HRESULT WindowActivate ( );
    HRESULT PropertyChange (
        _bstr_t Property );

	// members
	vk_login_dialog *m_cpParent; // any time a IWebBrowser instance is needed

public:
	void SetParent(vk_login_dialog *pParent) { m_cpParent = pParent; }

};