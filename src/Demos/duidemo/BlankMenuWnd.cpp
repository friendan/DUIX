#include "StdAfx.h"
#include "BlankMenuWnd.h"

DUI_BEGIN_MESSAGE_MAP(CBlankMenuWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CBlankMenuWnd::OnClick)
DUI_END_MESSAGE_MAP()

CBlankMenuWnd::CBlankMenuWnd()
	: m_pStatus(NULL)
	, m_pMenu(NULL)
{
}

CBlankMenuWnd::~CBlankMenuWnd()
{
	if( m_pMenu != NULL ) {
		delete m_pMenu;
		m_pMenu = NULL;
	}
}

void CBlankMenuWnd::Open(HWND hParent)
{
	CBlankMenuWnd* pWnd = new CBlankMenuWnd();
	pWnd->Create(hParent, _T("空白右键菜单测试"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 520, 480);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CBlankMenuWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CBlankMenuWnd::GetSkinFile()
{
	return _T("blankmenu.html");
}

LPCTSTR CBlankMenuWnd::GetWindowClassName() const
{
	return _T("BlankMenuWnd");
}

	void CBlankMenuWnd::InitWindow()
	{
		// 开空白右键菜单：总开关 + target（默认最内层覆盖点容器，这里显式设置让它更直观）
		m_pm.SetBlankContextMenuEnabled(true);
		m_pm.SetBlankContextMenuUseDeepestContainer(true);

		// 空白 MENU 是定向派发：给目标容器挂 OnNotify 回调接收。
		// 本 demo 皮肤里，空白在 sitePage(VBox) 内，故挂到 sitePage（而非根）。
		// 若空白可能在多个容器里，可给要用到的容器分别挂（这里是演示最简：只挂 sitePage）。
		CControlUI* pSite = m_pm.FindControl(_T("sitePage"));
		if( pSite != NULL )
			pSite->OnNotify += MakeDelegate(this, &CBlankMenuWnd::OnBlankMenu);
		else {
			CControlUI* pRoot = m_pm.GetRootPtr();
			if( pRoot != NULL )
				pRoot->OnNotify += MakeDelegate(this, &CBlankMenuWnd::OnBlankMenu);
		}

		m_pStatus = static_cast<CLabelUI*>(m_pm.FindControl(_T("status")));
		SetStatus(_T("就绪：请在页面下方的空白区域右键（容器未铺满处）"));
	}

void CBlankMenuWnd::SetStatus(LPCTSTR pstrText)
{
	if( m_pStatus != NULL )
		m_pStatus->SetText(pstrText ? pstrText : _T(""));
}

	void CBlankMenuWnd::Notify(TNotifyUI& msg)
	{
		// 空白 MENU 已改定向派发（走目标容器 OnNotify → OnBlankMenu），
		// 窗口层 Notify 不再收它，其余消息交回基类。
		WindowImplBase::Notify(msg);
	}

	bool CBlankMenuWnd::OnBlankMenu(void* p)
	{
		if( p == NULL ) return false;
		TNotifyUI* msg = static_cast<TNotifyUI*>(p);
		if( msg->sType != DUI_MSGTYPE_MENU || msg->pSender == NULL ) return false;

		// 空白右键（定向派发到目标容器）：pSender 为最内层容器或根，ptScreen/ptMouse 为坐标
		CDuiString s;
		s.Format(_T("空白右键：sender=%s(%s)，屏幕(%d,%d)，客户端(%d,%d)"),
			msg->pSender->GetName().IsEmpty() ? _T("(no-name)") : msg->pSender->GetName().GetData(),
			msg->pSender->GetClass(),
			(int)msg->ptScreen.x, (int)msg->ptScreen.y,
			(int)msg->ptMouse.x, (int)msg->ptMouse.y);
		SetStatus(s.GetData());
		ShowBlankMenu(msg->pSender, msg->ptScreen);
		return true;
	}

void CBlankMenuWnd::ShowBlankMenu(CControlUI* pSender, POINT ptScreen)
{
	CDuiPoint point(ptScreen.x, ptScreen.y);
	CThemeManager* tm = CThemeManager::GetInstance();
	CMenuWnd::GetGlobalContextMenuObserver().SetMenuCheckInfo(NULL);
	if( m_pMenu != NULL ) {
		delete m_pMenu;
		m_pMenu = NULL;
	}
	m_pMenu = new CMenuWnd();
	m_pMenu->Init(NULL, _T("blankmenu_ctx.html"), point, &m_pm);
	if( tm != NULL && m_pMenu->GetMenuUI() != NULL )
		tm->ApplyMenuChrome(m_pMenu->GetMenuUI());
	m_pMenu->ResizeMenu();
}

void CBlankMenuWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sName = msg.pSender ? msg.pSender->GetName() : CDuiString();
	if( sName == _T("closebtn") ) {
		Close();
		return;
	}
	if( sName == _T("act_hello") || sName == _T("act_clear") ) {
		SetStatus(sName == _T("act_hello") ? _T("点了『你好』菜单项") : _T("点了『清空状态』菜单项"));
		return;
	}
	WindowImplBase::OnClick(msg);
}
