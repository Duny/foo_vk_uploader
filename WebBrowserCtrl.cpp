#include "stdafx.h"
#include "WebBrowserCtrl.h"
#include "login_dialog.h"
#include <Exdispid.h> // platform SDK header

HRESULT __stdcall DWebBrowserEventsImpl::QueryInterface (REFIID riid, LPVOID* ppv)
{
	*ppv = NULL;

	if (IID_IUnknown == riid || __uuidof (SHDocVw::DWebBrowserEventsPtr) == riid) {
		*ppv = (LPUNKNOWN)(SHDocVw::DWebBrowserEventsPtr*)this;
		AddRef ();
		return NOERROR;
	}
	else if (IID_IOleClientSite == riid) {
		*ppv = (IOleClientSite*)this;
		AddRef ();
		return NOERROR;
	}
	else if (IID_IDispatch == riid) {
		*ppv = (IDispatch*)this;
		AddRef ();
		return NOERROR;
	}
	else
		return E_NOTIMPL;
}

ULONG __stdcall DWebBrowserEventsImpl::AddRef ()
{
    return 1;
}

ULONG __stdcall DWebBrowserEventsImpl::Release ()
{
    return 0;
}

// IDispatch methods
HRESULT __stdcall DWebBrowserEventsImpl::GetTypeInfoCount (UINT* pctinfo)
{
	return E_NOTIMPL;
}

HRESULT __stdcall DWebBrowserEventsImpl::GetTypeInfo (UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT __stdcall DWebBrowserEventsImpl::GetIDsOfNames (REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
	return E_NOTIMPL;
}
        
HRESULT __stdcall DWebBrowserEventsImpl::Invoke (DISPID dispIdMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS __RPC_FAR *pDispParams,
    VARIANT __RPC_FAR *pVarResult,
    EXCEPINFO __RPC_FAR *pExcepInfo,
    UINT __RPC_FAR *puArgErr)
{ 
	// proces OnBeforeNavigate
	if (dispIdMember == DISPID_BEFORENAVIGATE)
	    BeforeNavigate (_bstr_t (pDispParams->rgvarg[5].bstrVal),
						0,
						_bstr_t (pDispParams->rgvarg[3].bstrVal),
						NULL,
						_bstr_t (""),
						NULL);
	else if (dispIdMember == DISPID_NAVIGATECOMPLETE)
		NavigateComplete (_bstr_t (pDispParams->rgvarg[0].bstrVal));

	return NOERROR;
}

HRESULT DWebBrowserEventsImpl::BeforeNavigate (
    _bstr_t URL,
    long Flags,
    _bstr_t TargetFrameName,
    VARIANT * PostData,
    _bstr_t Headers,
    VARIANT_BOOL * Cancel)
{
	return S_OK;
}

HRESULT DWebBrowserEventsImpl::NavigateComplete (_bstr_t URL)
{
    return m_cpParent->NavigateComplete (URL);
}

HRESULT DWebBrowserEventsImpl::StatusTextChange (_bstr_t Text)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::ProgressChange (long Progress, long ProgressMax)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::DownloadComplete ()
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::CommandStateChange (long Command, VARIANT_BOOL Enable)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::DownloadBegin ()
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::NewWindow (
    _bstr_t URL,
    long Flags,
    _bstr_t TargetFrameName,
    VARIANT * PostData,
    _bstr_t Headers,
    VARIANT_BOOL * Processed)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::TitleChange (_bstr_t Text)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::FrameBeforeNavigate (
    _bstr_t URL,
    long Flags,
    _bstr_t TargetFrameName,
    VARIANT * PostData,
    _bstr_t Headers,
    VARIANT_BOOL * Cancel)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::FrameNavigateComplete (_bstr_t URL)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::FrameNewWindow (
    _bstr_t URL,
    long Flags,
    _bstr_t TargetFrameName,
    VARIANT * PostData,
    _bstr_t Headers,
    VARIANT_BOOL * Processed)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::Quit (VARIANT_BOOL * Cancel)
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::WindowMove ()
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::WindowResize ()
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::WindowActivate ()
{
    return S_OK;
}

HRESULT DWebBrowserEventsImpl::PropertyChange (_bstr_t Property)
{
    return S_OK;
}