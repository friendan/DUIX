#include "StdAfx.h"
#include "CarouselTestWnd.h"

DUI_BEGIN_MESSAGE_MAP(CCarouselTestWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CCarouselTestWnd::OnClick)
DUI_END_MESSAGE_MAP()

CCarouselTestWnd::CCarouselTestWnd()
	: m_pStatus(NULL)
{
}

CCarouselTestWnd::~CCarouselTestWnd()
{
}

void CCarouselTestWnd::Open(HWND hParent)
{
	CCarouselTestWnd* pWnd = new CCarouselTestWnd();
	pWnd->Create(hParent, _T("Carousel Test"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 720, 640);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CCarouselTestWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CCarouselTestWnd::GetSkinFile()
{
	return _T("carouseltest.html");
}

LPCTSTR CCarouselTestWnd::GetWindowClassName() const
{
	return _T("CarouselTestWnd");
}

void CCarouselTestWnd::InitWindow()
{
	m_pStatus = static_cast<CLabelUI*>(m_pm.FindControl(_T("status")));
	SetStatus(_T("就绪：自动播放 / 悬停暂停 / 控制栏切换"));
}

void CCarouselTestWnd::SetStatus(LPCTSTR pstrText)
{
	if( m_pStatus != NULL )
		m_pStatus->SetText(pstrText ? pstrText : _T(""));
}

void CCarouselTestWnd::Notify(TNotifyUI& msg)
{
	if( msg.sType == DUI_MSGTYPE_SLIDECHANGED && msg.pSender != NULL ) {
		CDuiString s;
		s.Format(_T("slidechanged: %s %d→%d"), msg.pSender->GetName().GetData(),
			(int)msg.lParam, (int)msg.wParam);
		SetStatus(s.GetData());
	}
	WindowImplBase::Notify(msg);
}

void CCarouselTestWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sName = msg.pSender->GetName();
	if( sName == _T("closebtn") ) {
		Close();
		return;
	}
	WindowImplBase::OnClick(msg);
}
