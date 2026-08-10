#pragma once
#include "PopWnd.h"
#include "ShlObj.h"
#include "MsgWnd.h"
#include "ControlEx.h"
#include "SkinManager.h"

//////////////////////////////////////////////////////////////////////////
///

class CMainPage : public CNotifyPump
{
public:
	CMainPage();

public:
	void SetPaintMagager(CPaintManagerUI* pPaintMgr);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged( TNotifyUI &msg );
	virtual void OnItemClick( TNotifyUI &msg );

private:
	CPaintManagerUI* m_pPaintManager;
};

//////////////////////////////////////////////////////////////////////////
///

class CVirtualListDemoCallback : public IVirtualListCallback
{
public:
	virtual LPCTSTR GetItemText(CControlUI* /*pList*/, int iIndex)
	{
		m_sText.Format(_T("虚拟行 #%d  —  仅绘制可见项，滚动不会创建 10 万个子控件"), iIndex + 1);
		return m_sText.GetData();
	}
private:
	CDuiString m_sText;
};

class CMainWnd : public WindowImplBase, public CWebBrowserEventHandler, public SkinChangedReceiver
{
public:
	CMainWnd();
	~CMainWnd();

public:// UI初始化
	DuiLib::CDuiString GetSkinFile();
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle() const;
	void InitWindow();
	void OnFinalMessage(HWND hWnd);

public:// 接口回调
	CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual BOOL Receive(SkinChangedParam param);
	LPCTSTR QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType);

public:// UI通知消息
	void Notify(TNotifyUI& msg);
	void OnLClick(CControlUI *pControl);

	DUI_DECLARE_MESSAGE_MAP()

public:// 系统消息
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:// WebBrowser
	virtual HRESULT STDMETHODCALLTYPE UpdateUI(void);
	virtual HRESULT STDMETHODCALLTYPE GetHostInfo(CWebBrowserUI* pWeb, DOCHOSTUIINFO __RPC_FAR *pInfo);
	virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(CWebBrowserUI* pWeb, DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);

private:// UI变量
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	CButtonUI* m_pSkinBtn;
	CMenuWnd* m_pMenu;
	CStdStringPtrMap m_MenuInfos;
	CTrayIcon m_trayIcon;
	CVirtualListDemoCallback m_vlistCallback;

public:
	CMainPage m_MainPage;
};
