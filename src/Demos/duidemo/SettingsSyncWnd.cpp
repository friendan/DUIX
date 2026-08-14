#include "StdAfx.h"
#include "SettingsSyncWnd.h"

DUI_BEGIN_MESSAGE_MAP(CSettingsSyncWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CSettingsSyncWnd::OnClick)
DUI_END_MESSAGE_MAP()

CSettingsSyncWnd::CSettingsSyncWnd()
{
}

CSettingsSyncWnd::~CSettingsSyncWnd()
{
}

void CSettingsSyncWnd::Open(HWND hOwner)
{
	if( hOwner == NULL || !::IsWindow(hOwner) ) return;

	// 屏幕坐标（副屏可为负）；与 Owner 同矩形铺满
	RECT rcOwner = { 0 };
	if( !::GetWindowRect(hOwner, &rcOwner) ) return;
	const int w = rcOwner.right - rcOwner.left;
	const int h = rcOwner.bottom - rcOwner.top;
	if( w < 1 || h < 1 ) return;

	CSettingsSyncWnd* pWnd = new CSettingsSyncWnd();
	// MINIMIZEBOX/MAXIMIZEBOX：标题栏最小/最大化可用；勿用 TOOLWINDOW，便于任务栏还原测联动
	pWnd->Create(hOwner, _T("设置（SyncOwner）"),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		0, rcOwner.left, rcOwner.top, w, h);
	if( pWnd->GetHWND() == NULL ) {
		delete pWnd;
		return;
	}

	// Create 后再钉一次，避免多屏/DPI 下首帧位置被校正偏移
	::SetWindowPos(pWnd->GetHWND(), NULL, rcOwner.left, rcOwner.top, w, h,
		SWP_NOZORDER | SWP_NOACTIVATE);

	pWnd->SetSyncOwnerMove(true);
	pWnd->SetSyncOwnerSize(true);
	pWnd->ShowModal();
}

void CSettingsSyncWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CSettingsSyncWnd::GetSkinFile()
{
	return _T("settings_sync.html");
}

LPCTSTR CSettingsSyncWnd::GetWindowClassName() const
{
	return _T("SettingsSyncWnd");
}

void CSettingsSyncWnd::InitWindow()
{
}

void CSettingsSyncWnd::Notify(TNotifyUI& msg)
{
	WindowImplBase::Notify(msg);
}

void CSettingsSyncWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sName = msg.pSender->GetName();
	if( sName.CompareNoCase(_T("closebtn2")) == 0 ) {
		Close(0);
		return;
	}
	WindowImplBase::OnClick(msg);
}
