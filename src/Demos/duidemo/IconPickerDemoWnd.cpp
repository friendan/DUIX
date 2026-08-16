#include "StdAfx.h"
#include "IconPickerDemoWnd.h"

DUI_BEGIN_MESSAGE_MAP(CIconPickerDemoWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CIconPickerDemoWnd::OnClick)
DUI_END_MESSAGE_MAP()

using namespace DuiLib;

CIconPickerDemoWnd::CIconPickerDemoWnd()
	: m_pPickerFilter(NULL)
	, m_pPickerAll(NULL)
	, m_pPickerFixed(NULL)
	, m_pStatusFilter(NULL)
	, m_pStatusAll(NULL)
	, m_pStatusFixed(NULL)
{
}

CIconPickerDemoWnd::~CIconPickerDemoWnd()
{
}

void CIconPickerDemoWnd::Open(HWND hParent)
{
	CIconPickerDemoWnd* pWnd = new CIconPickerDemoWnd();
	pWnd->Create(hParent, _T("图标选择控件测试"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 560, 340);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CIconPickerDemoWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CIconPickerDemoWnd::GetSkinFile()
{
	return _T("iconpicker_wnd.html");
}

LPCTSTR CIconPickerDemoWnd::GetWindowClassName() const
{
	return _T("IconPickerDemoWnd");
}

void CIconPickerDemoWnd::InitWindow()
{
	m_pPickerFilter = static_cast<CIconPickerUI*>(m_pm.FindControl(_T("pic_filter")));
	m_pPickerAll = static_cast<CIconPickerUI*>(m_pm.FindControl(_T("pic_all")));
	m_pPickerFixed = static_cast<CIconPickerUI*>(m_pm.FindControl(_T("pic_fixed")));
	m_pStatusFilter = static_cast<CLabelUI*>(m_pm.FindControl(_T("status_filter")));
	m_pStatusAll = static_cast<CLabelUI*>(m_pm.FindControl(_T("status_all")));
	m_pStatusFixed = static_cast<CLabelUI*>(m_pm.FindControl(_T("status_fixed")));
	UpdateStatus();
}

void CIconPickerDemoWnd::UpdateStatus()
{
	if( m_pStatusFilter != NULL ) {
		CDuiString s;
		LPCTSTR lib = m_pPickerFilter ? m_pPickerFilter->GetLibrary() : _T("");
		LPCTSTR name = m_pPickerFilter ? m_pPickerFilter->GetSelectedIcon() : _T("");
		s.Format(_T("白名单选中：库=%s 图标=%s"), (lib && *lib) ? lib : _T("(无)"), (name && *name) ? name : _T("(无)"));
		m_pStatusFilter->SetText(s.GetData());
	}
	if( m_pStatusAll != NULL ) {
		CDuiString s;
		LPCTSTR lib = m_pPickerAll ? m_pPickerAll->GetLibrary() : _T("");
		LPCTSTR name = m_pPickerAll ? m_pPickerAll->GetSelectedIcon() : _T("");
		DWORD clr = m_pPickerAll ? m_pPickerAll->GetIconColor() : 0;
		s.Format(_T("全部库选中：库=%s 图标=%s 颜色=#%06X"),
			(lib && *lib) ? lib : _T("(无)"),
			(name && *name) ? name : _T("(无)"),
			(unsigned)(clr & 0xFFFFFF));
		m_pStatusAll->SetText(s.GetData());
	}
	if( m_pStatusFixed != NULL ) {
		CDuiString s;
		LPCTSTR lib = m_pPickerFixed ? m_pPickerFixed->GetLibrary() : _T("");
		LPCTSTR name = m_pPickerFixed ? m_pPickerFixed->GetSelectedIcon() : _T("");
		int w = m_pPickerFixed ? m_pPickerFixed->GetIconWidth() : 0;
		int h = m_pPickerFixed ? m_pPickerFixed->GetIconHeight() : 0;
		s.Format(_T("固定选中：库=%s 图标=%s 尺寸=%dx%d"),
			(lib && *lib) ? lib : _T("(无)"),
			(name && *name) ? name : _T("(无)"),
			w, h);
		m_pStatusFixed->SetText(s.GetData());
	}
}

void CIconPickerDemoWnd::Notify(TNotifyUI& msg)
{
	// 图标选择控件确定后发 selectchanged（pSender = 对应 picker）
	if( msg.sType == DUI_MSGTYPE_SELECTCHANGED ) {
		if( msg.pSender == m_pPickerFilter || msg.pSender == m_pPickerAll || msg.pSender == m_pPickerFixed ) {
			UpdateStatus();
			return;
		}
	}
	WindowImplBase::Notify(msg);
}

void CIconPickerDemoWnd::OnClick(TNotifyUI& msg)
{
	if( msg.pSender == NULL ) {
		WindowImplBase::OnClick(msg);
		return;
	}
	CDuiString sName = msg.pSender->GetName();
	if( sName == _T("closebtn") ) {
		Close();
		return;
	}
	WindowImplBase::OnClick(msg);
}
