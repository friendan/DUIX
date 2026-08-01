#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"
#include "SkinFrame.h"
#include "IconBrowserWnd.h"
#include "TabBarTestWnd.h"
#include "LayoutTestWnd.h"
#include "Icons/BootstrapIconsData.h"
#include "Icons/LucideIconsIconsData.h"
#include "Icons/IconParkIconsData.h"
#include "Icons/TablerOutlineIconsData.h"
#include "Icons/TablerFilledIconsData.h"
#include "Icons/RemixIconIconsData.h"
#include "Icons/TwemojiIconsData.h"

namespace {
	HWND g_hLastToast = NULL;

	HWND RememberToast(HWND h)
	{
		if( h ) g_hLastToast = h;
		return h;
	}

	void CALLBACK OnToastDetailClick(HWND /*hToast*/, LPCTSTR pUserData, void* /*pUser*/)
	{
		CDuiString s;
		s.Format(_T("跳转详情：%s"), pUserData && *pUserData ? pUserData : _T("(无 UserData)"));
		CToast::ShowInfo(s.GetData(), 3000);
	}

	void CALLBACK OnToastDismissed(HWND /*hToast*/, ToastDismissReason reason, LPCTSTR pUserData, void* /*pUser*/)
	{
		LPCTSTR sReason = _T("手动");
		if( reason == ToastDismiss_Timeout ) sReason = _T("超时");
		else if( reason == ToastDismiss_Evicted ) sReason = _T("被顶掉");
		CDuiString s;
		s.Format(_T("OnDismiss：%s%s%s"), sReason,
			(pUserData && *pUserData) ? _T(" · ") : _T(""),
			(pUserData && *pUserData) ? pUserData : _T(""));
		CToast::ShowInfo(s.GetData(), 3000);
	}

	void CALLBACK OnModalResult(bool ok, LPCTSTR pUserData, void* /*pUser*/)
	{
		CDuiString s;
		s.Format(_T("Modal %s%s%s"),
			ok ? _T("确定") : _T("取消"),
			(pUserData && *pUserData) ? _T(" · ") : _T(""),
			(pUserData && *pUserData) ? pUserData : _T(""));
		if( ok ) CToast::ShowSuccess(s.GetData(), 3000);
		else CToast::ShowInfo(s.GetData(), 3000);
	}
}

//////////////////////////////////////////////////////////////////////////
///

DUI_BEGIN_MESSAGE_MAP(CMainPage, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,CMainPage::OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,CMainPage::OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK,CMainPage::OnItemClick)
DUI_END_MESSAGE_MAP()

CMainPage::CMainPage()
{
	m_pPaintManager = NULL;
}

void CMainPage::SetPaintMagager(CPaintManagerUI* pPaintMgr)
{
	m_pPaintManager = pPaintMgr;
}

void CMainPage::OnClick(TNotifyUI& msg)
{

}

void CMainPage::OnSelectChanged( TNotifyUI &msg )
{

}

void CMainPage::OnItemClick( TNotifyUI &msg )
{

}

//////////////////////////////////////////////////////////////////////////
///
DUI_BEGIN_MESSAGE_MAP(CMainWnd, WindowImplBase)
DUI_END_MESSAGE_MAP()

CMainWnd::CMainWnd() 
{
	m_pMenu = NULL;

	m_MainPage.SetPaintMagager(&m_pm);
	AddVirtualWnd(_T("mainpage"),&m_MainPage);
}

CMainWnd::~CMainWnd()
{
	CMenuWnd::DestroyMenu();
	if(m_pMenu != NULL) {
		delete m_pMenu;
		m_pMenu = NULL;
	}
	RemoveVirtualWnd(_T("mainpage"));
}

CControlUI* CMainWnd::CreateControl(LPCTSTR pstrClass)
{
	if(lstrcmpi(pstrClass, _T("CircleProgress" )) == 0) {
		return new CCircleProgressUI();
	}
	return NULL;
}

void CMainWnd::InitWindow() 
{
	SetIcon(IDR_MAINFRAME);
	CResourceManager::GetInstance()->SetTextQueryInterface(this);
	CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.xml"));
	CSkinManager::GetSkinManager()->AddReceiver(this);

	m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
	m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("maxbtn")));
	m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));
	m_pSkinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("skinbtn")));

	m_trayIcon.CreateTrayIcon(m_hWnd, IDR_MAINFRAME, _T("Duilib演示大全"));
}

BOOL CMainWnd::Receive(SkinChangedParam param)
{
	CControlUI* pRoot = m_pm.FindControl(_T("root"));
	if( pRoot != NULL ) {
		if( param.bColor ) {
			pRoot->SetBkColor(param.bkcolor);
			pRoot->SetBkImage(_T(""));
		}
		else {
			pRoot->SetBkColor(0);
			pRoot->SetBkImage(param.bgimage);
			//m_pm.SetLayeredImage(param.bgimage);
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CMainWnd::UpdateUI( void)
{
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CMainWnd::GetHostInfo(CWebBrowserUI* pWeb, 
	/* [out][in] */ DOCHOSTUIINFO __RPC_FAR *pInfo)
{
	if (pInfo != NULL) {
		pInfo->dwFlags |= DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_NO3DOUTERBORDER;
	}
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CMainWnd::ShowContextMenu(CWebBrowserUI* pWeb, 
	/* [in] */ DWORD dwID,
	/* [in] */ POINT __RPC_FAR *ppt,
	/* [in] */ IUnknown __RPC_FAR *pcmdtReserved,
	/* [in] */ IDispatch __RPC_FAR *pdispReserved)
{
	return E_NOTIMPL;
	//返回 E_NOTIMPL 正常弹出系统右键菜单
	//返回S_OK 则可屏蔽系统右键菜单
}

DuiLib::CDuiString CMainWnd::GetSkinFile()
{
	return _T("XML_MAIN");
}

LPCTSTR CMainWnd::GetWindowClassName() const
{ 
	return _T("MainWnd");
}

UINT CMainWnd::GetClassStyle() const
{ 
	return CS_DBLCLKS; 
}

void CMainWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
}

LPCTSTR CMainWnd::QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType)
{
	CDuiString sLanguage = CResourceManager::GetInstance()->GetLanguage();
	if(sLanguage == _T("en")){
		if(lstrcmpi(lpstrId, _T("titletext")) == 0) {
			return _T("Duilib Demo v1.1");
		}
		else if(lstrcmpi(lpstrId, _T("hometext")) == 0) {
			return _T("{a}Home Page{/a}");
		}
	}
	else{
		if(lstrcmpi(lpstrId, _T("titletext")) == 0) {
			return _T("Duilib 使用演示 v1.1");
		}
		else if(lstrcmpi(lpstrId, _T("hometext")) == 0) {
			return _T("{a}开源官网{/a}");
		}
	}

	return NULL;
}

void CMainWnd::Notify(TNotifyUI& msg)
{
	CDuiString name = msg.pSender->GetName();
	if(msg.sType == _T("windowinit")) {
	}
	else if( msg.sType == _T("textchanged") )
	{
		CEditUI* pEdit = (CEditUI*)msg.pSender;
	}
	else if( msg.sType == _T("colorchanged") )
	{
		CControlUI* pRoot = m_pm.FindControl(_T("root"));
		if( pRoot != NULL ) {
			CColorPaletteUI* pColorPalette = (CColorPaletteUI*)m_pm.FindControl(_T("Pallet"));
			pRoot->SetBkColor(pColorPalette->GetSelectColor());
			pRoot->SetBkImage(_T(""));
		}
	}
	else if(msg.sType == DUI_MSGTYPE_ITEMACTIVATE) {
		if(MSGID_OK == CMsgWnd::MessageBox(m_hWnd, _T("Duilib旗舰版"), _T("确定退出duidemo演示程序？")))
		{
			::DestroyWindow(m_hWnd);
		}
	}
	else if(msg.sType == DUI_MSGTYPE_ITEMCLICK) {
		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("listview")));
	}
	else if( msg.sType == _T("showactivex") ) 
	{
		//if( name.CompareNoCase(_T("ani_flash")) == 0 ) {
		//	IShockwaveFlash* pFlash = NULL;
		//	CActiveXUI* pActiveX = static_cast<CActiveXUI*>(msg.pSender);
		//	pActiveX->GetControl(__uuidof(IShockwaveFlash), (void**)&pFlash);
		//	if( pFlash != NULL )  {
		//		pFlash->put_WMode( _bstr_t(_T("Transparent") ) );
		//		pFlash->put_Movie( _bstr_t(CPaintManagerUI::GetInstancePath() + _T("\\skin\\duidemo\\other\\waterdrop.swf")) );
		//		pFlash->DisableLocalSecurity();
		//		pFlash->put_AllowScriptAccess(L"always");
		//		BSTR response;
		//		pFlash->CallFunction(L"<invoke name=\"setButtonText\" returntype=\"xml\"><arguments><string>Click me!</string></arguments></invoke>", &response);
		//		pFlash->Release();
		//	}  
		//}
	}
	else if( msg.sType == _T("click") )
	{
		if( name.CompareNoCase(_T("closebtn")) == 0 ) 
		{
			if(MSGID_OK == CMsgWnd::MessageBox(m_hWnd, _T("Duilib旗舰版"), _T("确定退出duidemo演示程序？")))
			{
				::DestroyWindow(m_hWnd);
			}
			return; 
		}
		else if( msg.pSender == m_pMinBtn ) {
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return;
		}
		else if( msg.pSender == m_pMaxBtn ) { 
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
			return; 
		}
		else if( msg.pSender == m_pRestoreBtn ) { 
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
			return;
		}
		else if( msg.pSender == m_pSkinBtn ) {
			new CSkinFrame(m_hWnd, m_pSkinBtn);
		}
		// 按钮消息
		OnLClick(msg.pSender);
	}
	
	else if(msg.sType==_T("selectchanged"))
	{
		CTabLayoutUI* pTabSwitch = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("tab_switch")));
		if(name.CompareNoCase(_T("basic_tab")) == 0) pTabSwitch->SelectItem(0);
		if(name.CompareNoCase(_T("rich_tab")) == 0) pTabSwitch->SelectItem(1);
		if(name.CompareNoCase(_T("ex_tab")) == 0) pTabSwitch->SelectItem(2);
		if(name.CompareNoCase(_T("ani_tab")) == 0) pTabSwitch->SelectItem(3);
		if(name.CompareNoCase(_T("split_tab")) == 0) pTabSwitch->SelectItem(4);
		if(name.CompareNoCase(_T("layout_tab")) == 0) pTabSwitch->SelectItem(5);
	}
	else if(msg.sType == _T("valuechanged"))
	{
		CProgressUI* pSlider = static_cast<CProgressUI*>(m_pm.FindControl(_T("slider")));
		CProgressUI* pPro1 = static_cast<CProgressUI*>(m_pm.FindControl(_T("progress")));
		CProgressUI* pPro2 = static_cast<CProgressUI*>(m_pm.FindControl(_T("circle_progress")));
		pPro1->SetValue(pSlider->GetValue());
		pPro2->SetValue(pSlider->GetValue());
	}
	else if(msg.sType == _T("predropdown") && name == _T("font_size"))
	{
		CComboUI* pFontSize = static_cast<CComboUI*>(m_pm.FindControl(_T("font_size")));
		if(pFontSize)
		{
			pFontSize->RemoveAll();
			for(int i = 0; i < 10; i++) {
				CListLabelElementUI * pElement = new CListLabelElementUI();
				pElement->SetText(_T("测试长文字"));
				pElement->SetFixedHeight(30);
				pFontSize->Add(pElement);
			}
			pFontSize->SelectItem(0);
		}
	}
	else if(msg.sType == _T("menu"))
	{
		CMenuWnd::GetGlobalContextMenuObserver().SetMenuCheckInfo(&m_MenuInfos);

		if(m_pMenu != NULL) {
			delete m_pMenu;
			m_pMenu = NULL;
		}
		m_pMenu = new CMenuWnd();
		CDuiPoint point;
		::GetCursorPos(&point);
		m_pMenu->Init(NULL, _T("menu.xml"), point, &m_pm);
	}

	return WindowImplBase::Notify(msg);
}
void CMainWnd::OnLClick(CControlUI *pControl)
{
	CDuiString sName = pControl->GetName();
	if(sName.CompareNoCase(_T("homepage_btn")) == 0)
	{
		//
		//CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("listview")));
		//CListContainerElementUI* pListItem  = new CListContainerElementUI();
		//pListItem->SetChildVAlign(DT_VCENTER);
		//pListItem->SetFixedHeight(30);
		//pListItem->SetManager(&m_pm, NULL, false);
		//pListItem->SetFixedWidth(100);
		//pList->Add(pListItem);
		//pList->EndDown();
		//return;
		// 动态创建Combo
		//CComboUI* pFontSize = static_cast<CComboUI*>(m_pm.FindControl(_T("mycombo")));
		//if(pFontSize)
		//{
		//	pFontSize->RemoveAll();
		//	CListLabelElementUI * pElement = new CListLabelElementUI();
		//	pElement->SetText(_T("测试长文字"));
		//	pElement->SetFixedHeight(30);
		//	pElement->SetFixedWidth(120);
		//	pFontSize->Add(pElement);
		//	pFontSize->NeedParentUpdate();
		//}
		//CComboUI* pFontSize = static_cast<CComboUI*>(m_pm.FindControl(_T("mycombo")));
		//if(pFontSize)
		//{
		//	pFontSize->SetFixedXY(CDuiSize(pFontSize->GetFixedXY().cx + 5, pFontSize->GetFixedXY().cy));
		//}

		//CRichEditUI* wordedit = (CRichEditUI*)m_pm.FindControl(_T("wordedit"));
		//if(wordedit) {
		//	wordedit->SetAttribute(_T("autovscroll"), _T("true"));
		//	wordedit->SetAttribute(_T("vscrollbar"), _T("true"));
		//}
		ShellExecute(NULL, _T("open"), _T("https://github.com/qdtroy"), NULL, NULL, SW_SHOW);
	}
	else if(sName.CompareNoCase(_T("button1")) == 0)
	{
		//CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("edit3")));
		//TCHAR* pstrText = (TCHAR*)pEdit->GetText().GetData();
		//if(pstrText != NULL && lstrlen(pstrText) > 0) {
		//	double fEdit = _ttof(pstrText);
		//	MessageBox(m_hWnd, pstrText, _T(""), 0);
		//}
	}
	else if(sName.CompareNoCase(_T("popwnd_btn")) == 0)
	{
		CPopWnd* pPopWnd = new CPopWnd();
		pPopWnd->Create(m_hWnd, NULL, WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 0, 0, 800, 572);
		pPopWnd->CenterWindow();
	}
	else if(sName.CompareNoCase(_T("btn_bsicon")) == 0)
	{
		CIconBrowserWnd::Open(m_hWnd, g_bootstrapIcons, g_bootstrapIconCount, _T("bsicon"), _T("Bootstrap Icons"));
	}
	else if(sName.CompareNoCase(_T("btn_iconpark")) == 0)
	{
		CIconBrowserWnd::Open(m_hWnd, g_iconParkIcons, g_iconParkIconCount, _T("iconpark"), _T("IconPark"));
	}
	else if(sName.CompareNoCase(_T("btn_lucide")) == 0)
	{
		CIconBrowserWnd::Open(m_hWnd, g_lucideIconsIcons, g_lucideIconsIconCount, _T("lucide"), _T("Lucide Icons"));
	}
	else if(sName.CompareNoCase(_T("btn_tabler_filled")) == 0)
	{
		CIconBrowserWnd::Open(m_hWnd, g_tablerFilledIcons, g_tablerFilledIconCount, _T("tabler-filled"), _T("Tabler Filled"));
	}
	else if(sName.CompareNoCase(_T("btn_tabler_outline")) == 0)
	{
		CIconBrowserWnd::Open(m_hWnd, g_tablerOutlineIcons, g_tablerOutlineIconCount, _T("tabler-outline"), _T("Tabler Outline"));
	}
	else if(sName.CompareNoCase(_T("btn_remixicon")) == 0)
	{
		CIconBrowserWnd::Open(m_hWnd, g_remixIconIcons, g_remixIconIconCount, _T("remixicon"), _T("Remix Icon"));
	}
	else if(sName.CompareNoCase(_T("btn_twicon")) == 0)
	{
		CIconBrowserWnd::Open(m_hWnd, g_twemojiIcons, g_twemojiIconCount, _T("twicon"), _T("Twemoji"));
	}
	else if(sName.CompareNoCase(_T("btn_tabbar_test")) == 0)
	{
		CTabBarTestWnd::Open(m_hWnd);
	}
	else if(sName.CompareNoCase(_T("btn_layout_test")) == 0)
	{
		CLayoutTestWnd::Open(m_hWnd);
	}
	else if(sName.CompareNoCase(_T("btn_toast_success")) == 0)
	{
		RememberToast(CToast::ShowSuccess(_T("操作成功"), 3000));
	}
	else if(sName.CompareNoCase(_T("btn_toast_info")) == 0)
	{
		RememberToast(CToast::ShowInfo(_T("这是一条提示信息"), 3000));
	}
	else if(sName.CompareNoCase(_T("btn_toast_warning")) == 0)
	{
		RememberToast(CToast::ShowWarning(_T("请注意检查输入内容"), 4000));
	}
	else if(sName.CompareNoCase(_T("btn_toast_danger")) == 0)
	{
		RememberToast(CToast::ShowDanger(_T("操作失败，请稍后重试"), 5000));
	}
	else if(sName.CompareNoCase(_T("btn_toast_pause")) == 0)
	{
		RememberToast(CToast::Show(_T("悬停暂停测试"), _T("把鼠标移到这条 Toast 上，右侧秒数应停住；移开后继续倒计时"),
			CToastOptions()
				.Kind(CONTROLKIND_WARNING)
				.Duration(15000)
				.Owner(m_hWnd)
				.PauseOnHover(true)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_click")) == 0)
	{
		RememberToast(CToast::Show(_T("订单已支付"), _T("点击文字跳转详情；空白处可拖动"),
			CToastOptions()
				.Kind(CONTROLKIND_PRIMARY)
				.Duration(12000)
				.Owner(m_hWnd)
				.UserData(_T("order:A1024"))
				.OnClick(OnToastDetailClick)
				.ClickDismiss(true)
				.PauseOnHover(true)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_icon")) == 0)
	{
		RememberToast(CToast::Show(_T("自定义图标"), _T("Icon(\"lucide\", \"bell\") 覆盖 kind 默认图标"),
			CToastOptions()
				.Kind(CONTROLKIND_INFO)
				.Icon(_T("lucide"), _T("bell"))
				.Duration(5000)
				.Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_dismiss_cb")) == 0)
	{
		RememberToast(CToast::Show(_T("OnDismiss 测试"), _T("等超时 / 点× / 或点「关最近」，看下一条回调原因"),
			CToastOptions()
				.Kind(CONTROLKIND_SECONDARY)
				.Duration(5000)
				.Owner(m_hWnd)
				.UserData(_T("dismiss-demo"))
				.OnDismiss(OnToastDismissed)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_max2")) == 0)
	{
		// 每 Align 组上限 2：右下连弹 3 条应顶掉最早 1 条；左上 2 条不受影响
		CToast::SetMaxCount(2);
		RememberToast(CToast::Show(_T("右下 #1"), _T("同组将被顶掉"),
			CToastOptions().Kind(CONTROLKIND_INFO).Duration(15000).Owner(m_hWnd)
				.Align(ToastAlign_ScreenBottomRight)
				.UserData(_T("BR#1")).OnDismiss(OnToastDismissed)));
		RememberToast(CToast::Show(_T("右下 #2"), _T("同组应保留"),
			CToastOptions().Kind(CONTROLKIND_WARNING).Duration(15000).Owner(m_hWnd)
				.Align(ToastAlign_ScreenBottomRight)));
		RememberToast(CToast::Show(_T("左上 #1"), _T("另一 Align 组，不受右下顶掉影响"),
			CToastOptions().Kind(CONTROLKIND_PRIMARY).Duration(15000).Owner(m_hWnd)
				.Align(ToastAlign_ScreenTopLeft)));
		RememberToast(CToast::Show(_T("左上 #2"), _T("另一 Align 组"),
			CToastOptions().Kind(CONTROLKIND_SUCCESS).Duration(15000).Owner(m_hWnd)
				.Align(ToastAlign_ScreenTopLeft)));
		RememberToast(CToast::Show(_T("右下 #3"), _T("同组第 3 条 → 顶掉右下 #1"),
			CToastOptions().Kind(CONTROLKIND_DANGER).Duration(15000).Owner(m_hWnd)
				.Align(ToastAlign_ScreenBottomRight)));
		CToast::SetMaxCount(0);
	}
	else if(sName.CompareNoCase(_T("btn_toast_dual_success")) == 0)
	{
		RememberToast(CToast::Show(_T("同步完成"), _T("已将 12 个文件上传到服务器"),
			CToastOptions().Kind(CONTROLKIND_SUCCESS).Duration(4000).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_dual_info")) == 0)
	{
		RememberToast(CToast::Show(_T("新消息"), _T("你有 3 条未读通知，点击查看详情"),
			CToastOptions().Kind(CONTROLKIND_INFO).Duration(4000).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_dual_warning")) == 0)
	{
		RememberToast(CToast::Show(_T("磁盘空间不足"), _T("剩余空间低于 10%，请及时清理"),
			CToastOptions().Kind(CONTROLKIND_WARNING).Duration(4000).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_dual_danger")) == 0)
	{
		RememberToast(CToast::Show(_T("连接失败"), _T("无法连接到服务器，请检查网络后重试"),
			CToastOptions().Kind(CONTROLKIND_DANGER).Duration(5000).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_dual_primary")) == 0)
	{
		RememberToast(CToast::Show(_T("任务已排队"), _T("导出任务已加入队列，完成后将通知你"),
			CToastOptions().Kind(CONTROLKIND_PRIMARY).Duration(4000).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_dismiss_last")) == 0)
	{
		CToast::Dismiss(g_hLastToast);
		g_hLastToast = NULL;
	}
	else if(sName.CompareNoCase(_T("btn_toast_dismiss")) == 0)
	{
		CToast::DismissAll();
		g_hLastToast = NULL;
	}
	else if(sName.CompareNoCase(_T("btn_toast_scr_tl")) == 0)
	{
		RememberToast(CToast::Show(_T("屏幕左上"),
			CToastOptions().Kind(CONTROLKIND_PRIMARY).Duration(3000)
				.Align(ToastAlign_ScreenTopLeft).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_scr_tr")) == 0)
	{
		RememberToast(CToast::Show(_T("屏幕右上"),
			CToastOptions().Kind(CONTROLKIND_PRIMARY).Duration(3000)
				.Align(ToastAlign_ScreenTopRight).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_scr_bl")) == 0)
	{
		RememberToast(CToast::Show(_T("屏幕左下"),
			CToastOptions().Kind(CONTROLKIND_PRIMARY).Duration(3000)
				.Align(ToastAlign_ScreenBottomLeft).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_scr_br")) == 0)
	{
		RememberToast(CToast::Show(_T("屏幕右下"),
			CToastOptions().Kind(CONTROLKIND_PRIMARY).Duration(3000)
				.Align(ToastAlign_ScreenBottomRight).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_wnd_tl")) == 0)
	{
		RememberToast(CToast::Show(_T("窗口左上（拖主窗跟随）"),
			CToastOptions().Kind(CONTROLKIND_INFO).Duration(10000)
				.Align(ToastAlign_WindowTopLeft).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_wnd_tr")) == 0)
	{
		RememberToast(CToast::Show(_T("窗口右上（拖主窗跟随）"),
			CToastOptions().Kind(CONTROLKIND_INFO).Duration(10000)
				.Align(ToastAlign_WindowTopRight).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_wnd_bl")) == 0)
	{
		RememberToast(CToast::Show(_T("窗口左下（拖主窗跟随）"),
			CToastOptions().Kind(CONTROLKIND_INFO).Duration(10000)
				.Align(ToastAlign_WindowBottomLeft).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_toast_wnd_br")) == 0)
	{
		RememberToast(CToast::Show(_T("窗口右下（拖主窗跟随）"),
			CToastOptions().Kind(CONTROLKIND_INFO).Duration(10000)
				.Align(ToastAlign_WindowBottomRight).Owner(m_hWnd)));
	}
	else if(sName.CompareNoCase(_T("btn_modal_info")) == 0)
	{
		CModal::ShowInfo(_T("这是一条 Info 提示。"), OnModalResult);
	}
	else if(sName.CompareNoCase(_T("btn_modal_success")) == 0)
	{
		CModal::ShowSuccess(_T("操作已成功完成。"), OnModalResult);
	}
	else if(sName.CompareNoCase(_T("btn_modal_warning")) == 0)
	{
		CModal::ShowWarning(_T("请注意检查输入内容后再继续。"), OnModalResult);
	}
	else if(sName.CompareNoCase(_T("btn_modal_danger")) == 0)
	{
		CModal::ShowDanger(_T("发生错误，请稍后重试。"), OnModalResult);
	}
	else if(sName.CompareNoCase(_T("btn_modal_confirm")) == 0)
	{
		CModal::Confirm(_T("删除确认"), _T("确定删除该文件吗？此操作不可恢复。"),
			OnModalResult);
	}
	else if(sName.CompareNoCase(_T("btn_modal_custom")) == 0)
	{
		CModal::Show(_T("保存失败"), _T("磁盘空间不足，请清理后重试。"),
			CModalOptions()
				.Kind(CONTROLKIND_DANGER)
				.ShowCancel(true)
				.OkText(_T("重试"))
				.CancelText(_T("放弃"))
				.Owner(m_hWnd)
				.UserData(_T("save-retry"))
				.OnResult(OnModalResult));
	}
	else if(sName.CompareNoCase(_T("btn_modal_nobackdrop")) == 0)
	{
		CModal::Show(_T("点遮罩不关"), _T("ClickBackdropToClose(false)：只能点确定/Esc 关闭。"),
			CModalOptions()
				.Kind(CONTROLKIND_WARNING)
				.ClickBackdropToClose(false)
				.Owner(m_hWnd)
				.OnResult(OnModalResult));
	}
	else if(sName.CompareNoCase(_T("modal_popwnd_btn")) == 0)
	{
		CPopWnd* pPopWnd = new CPopWnd();
		pPopWnd->Create(m_hWnd, _T("透明窗口演示"), WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW, 0, 0, 800, 572);
		pPopWnd->CenterWindow();
		pPopWnd->ShowModal();
	}

	else if(sName.CompareNoCase(_T("qqgroup_btn")) == 0)
	{
		TCHAR szPath[MAX_PATH] ={0};
		SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		CDuiString sIEPath;
		sIEPath.Format(_T("%s\\Internet Explorer\\iexplore.exe"), szPath);
		ShellExecute(NULL, _T("open"), sIEPath, _T("http://jq.qq.com/?_wv=1027&k=cDTUzr"), NULL, SW_SHOW);
	}
	else if(sName.CompareNoCase(_T("qq_btn")) == 0)
	{
		TCHAR szPath[MAX_PATH] ={0};
		SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		CDuiString sIEPath;
		sIEPath.Format(_T("%s\\Internet Explorer\\iexplore.exe"), szPath);
		ShellExecute(NULL, _T("open"), sIEPath, _T("tencent://Message/?Uin=656067418&Menu=yes"), NULL, SW_SHOW);
	}
	else if(sName.CompareNoCase(_T("menubtn")) == 0)
	{
		CMenuWnd::GetGlobalContextMenuObserver().SetMenuCheckInfo(&m_MenuInfos);

		if(m_pMenu != NULL) {
			delete m_pMenu;
			m_pMenu = NULL;
		}
		m_pMenu = new CMenuWnd();
		CDuiPoint point;
		::GetCursorPos(&point);
		m_pMenu->Init(NULL, _T("menu.xml"), point, &m_pm);
		// 设置状态
		CMenuWnd::SetMenuItemInfo(_T("qianting"), true);

		CMenuUI* rootMenu = m_pMenu->GetMenuUI();
		if (rootMenu != NULL)
		{
			CMenuElementUI* pNew = new CMenuElementUI;
			pNew->SetName(_T("Menu_Dynamic"));
			pNew->SetText(_T("动态一级菜单"));
			pNew->SetShowExplandIcon(true);
			pNew->SetIcon(_T("WebSit.png"));
			pNew->SetIconSize(16,16);
			rootMenu->Add(pNew);

			//CMenuElementUI* pTempMenu = (CMenuElementUI*)rootMenu->GetItemAt(0);
			//CMenuElementUI* pSubNew = new CMenuElementUI;
			//pSubNew->SetText(_T("动态二级菜单"));
			//pSubNew->SetName(_T("Menu_Dynamic"));
			//pSubNew->SetIcon(_T("Virus.png"));
			//pSubNew->SetIconSize(16,16);
			//pSubNew->SetOwner((CControlUI*)pTempMenu->GetOwner());
			//pTempMenu->Add(pSubNew);

			CMenuElementUI* pNew2 = new CMenuElementUI;
			pNew2->SetName(_T("Menu_Dynamic"));
			pNew2->SetText(_T("动态一级菜单2"));
			rootMenu->AddAt(pNew2,2);
		}

		// 动态添加后重新设置菜单的大小
		m_pMenu->ResizeMenu();
	}
	else if(sName.CompareNoCase(_T("dpi_btn")) == 0)
	{
		int nDPI = _ttoi(pControl->GetUserData());
		m_pm.SetDPI(nDPI);
	}
	else if(sName.CompareNoCase(_T("combo_closebtn")) == 0 ) 
	{
		CMsgWnd::ShowMessageBox(m_hWnd, _T("Combo按钮点击"), _T("Combo列表项-按钮点击"));

		return; 
	}
	else if(sName.CompareNoCase(_T("move_btn")) == 0 ) 
	{
		CDuiString sUserData = pControl->GetUserData();
		CControlUI* pControl = m_pm.FindControl(sUserData);
		if(pControl != nullptr) {
			SIZE pt = pControl->GetFixedXY();
			pt.cx += 20;
			pControl->SetFixedXY(pt);
		}
		return; 
	}
}

LRESULT CMainWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	m_trayIcon.DeleteTrayIcon();
	bHandled = FALSE;
	// 退出程序
	PostQuitMessage(0);
	return 0;
}

LRESULT CMainWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(uMsg == WM_TIMER)
	{
		bHandled = FALSE;
	}
	else if(uMsg == WM_SHOWWINDOW)
	{
		bHandled = FALSE;
		if(m_pMinBtn) m_pMinBtn->NeedParentUpdate();
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
	else if(uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN) {
		int a = 0;
	}
	else if (uMsg == WM_MENUCLICK)
	{
		MenuCmd* pMenuCmd = (MenuCmd*)wParam;
		if(pMenuCmd != NULL)
		{
			BOOL bChecked = pMenuCmd->bChecked;
			CDuiString sMenuName = pMenuCmd->szName;
			CDuiString sUserData = pMenuCmd->szUserData;
			CDuiString sText = pMenuCmd->szText;
			m_pm.DeletePtr(pMenuCmd);

			if(sMenuName.CompareNoCase(_T("lan")) == 0)
			{
				static bool bEn = false;
				if(!bEn) {
					CResourceManager::GetInstance()->SetLanguage(_T("en"));
					CResourceManager::GetInstance()->LoadLanguage(_T("lan_en.xml"));
				}
				else {
					CResourceManager::GetInstance()->SetLanguage(_T("cn_zh"));
					CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.xml"));
				}
				bEn = !bEn;
				CResourceManager::GetInstance()->ReloadText();
				InvalidateRect(m_hWnd, NULL, TRUE);
				m_pm.NeedUpdate();
			}
			else if (sMenuName == _T("qianting"))
			{
				if (bChecked)
				{
					CMsgWnd::MessageBox(m_hWnd, NULL, _T("你预定修潜艇服务"));
				} 
				else
				{
					CMsgWnd::MessageBox(m_hWnd, NULL, _T("你取消修潜艇服务"));
				}			 
			}
			else if(sMenuName == _T("exit")) {
				Close(0);
			}
			else
			{
				CMsgWnd::MessageBox(m_hWnd, NULL, sText);
			}
		}
		bHandled = TRUE;
		return 0;
	}
	else if(uMsg == UIMSG_TRAYICON)
	{
		UINT uIconMsg = (UINT)lParam;
		if(uIconMsg == WM_LBUTTONUP) {
			BOOL bVisible = IsWindowVisible(m_hWnd);
			::ShowWindow(m_hWnd, !bVisible ?  SW_SHOW : SW_HIDE);
		}
		else if(uIconMsg == WM_RBUTTONUP) {
			if(m_pMenu != NULL) {
				delete m_pMenu;
				m_pMenu = NULL;
			}
			m_pMenu = new CMenuWnd();
			CDuiPoint point;
			::GetCursorPos(&point);
			point.y -= 100;
			m_pMenu->Init(NULL, _T("menu.xml"), point, &m_pm);
			// 动态添加后重新设置菜单的大小
			m_pMenu->ResizeMenu();
		}
	}
	else if (uMsg == WM_DPICHANGED) {
		// 系统建议矩形为准，避免 SetDPI 再按比例缩放一次
		m_pm.SetDPI(LOWORD(wParam), false);
		RECT* const prcNewWindow = (RECT*)lParam;
		if( prcNewWindow != NULL ) {
			SetWindowPos(m_hWnd, NULL, prcNewWindow->left, prcNewWindow->top, prcNewWindow->right - prcNewWindow->left, prcNewWindow->bottom - prcNewWindow->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		if (m_pm.GetRoot() != NULL) m_pm.GetRoot()->NeedUpdate();
	}
	bHandled = FALSE;
	return 0;
}
//
//LRESULT CMainWnd::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	bHandled = FALSE;
//	return 0;
//}
