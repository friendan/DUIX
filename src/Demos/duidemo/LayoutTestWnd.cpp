#include "StdAfx.h"
#include "LayoutTestWnd.h"

DUI_BEGIN_MESSAGE_MAP(CLayoutTestWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CLayoutTestWnd::OnClick)
DUI_END_MESSAGE_MAP()

CLayoutTestWnd::CLayoutTestWnd()
{
}

CLayoutTestWnd::~CLayoutTestWnd()
{
}

void CLayoutTestWnd::Open(HWND hParent)
{
	CLayoutTestWnd* pWnd = new CLayoutTestWnd();
	pWnd->Create(hParent, _T("Layout Test"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 720, 480);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CLayoutTestWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CLayoutTestWnd::GetSkinFile()
{
	return _T("layouttest.htm");
}

LPCTSTR CLayoutTestWnd::GetWindowClassName() const
{
	return _T("LayoutTestWnd");
}

void CLayoutTestWnd::InitWindow()
{
}

void CLayoutTestWnd::Notify(TNotifyUI& msg)
{
	WindowImplBase::Notify(msg);
}

void CLayoutTestWnd::OnClick(TNotifyUI& msg)
{
	WindowImplBase::OnClick(msg);
}
