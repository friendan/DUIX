#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"
#include "SkinFrame.h"
#include "IconBrowserWnd.h"
#include "TabBarTestWnd.h"
#include "BrowserWnd.h"
#include "CarouselTestWnd.h"
#include "LayoutTestWnd.h"
#include "SettingsSyncWnd.h"
	#include "ShapeDemoWnd.h"
	#include "BlankMenuWnd.h"
	#include "Icons/BootstrapIconsData.h"
#include "Icons/LucideIconsIconsData.h"
#include "Icons/IconParkIconsData.h"
#include "Icons/TablerOutlineIconsData.h"
#include "Icons/TablerFilledIconsData.h"
#include "Icons/RemixIconIconsData.h"
#include "Icons/TwemojiIconsData.h"
#include "Core/UITheme.h"
#include <commdlg.h>

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
	CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.html"));
	CSkinManager::GetSkinManager()->AddReceiver(this);

	m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
	m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("maxbtn")));
	m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));
	m_pSkinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("skinbtn")));

	m_trayIcon.CreateTrayIcon(m_hWnd, IDR_MAINFRAME, _T("Duilib演示大全"));
	// 应用自行 Create → 不会走 WindowImplBase 自动托盘/默认菜单，仍用下方自定义右键菜单

	CControlUI* pVListCtrl = m_pm.FindControl(_T("vlist"));
	CVirtualListUI* pVList = pVListCtrl
		? static_cast<CVirtualListUI*>(pVListCtrl->GetInterface(DUI_CTR_VIRTUALLIST))
		: NULL;
	if( pVList != NULL ) {
		pVList->SetCallback(&m_vlistCallback);
		if( pVList->GetItemCount() <= 0 ) pVList->SetItemCount(100000);
	}
}

BOOL CMainWnd::Receive(SkinChangedParam param)
{
	CControlUI* pRoot = m_pm.FindControl(_T("root"));
	if( pRoot != NULL ) {
		if( param.bColor ) {
			pRoot->SetBackgroundColor(param.backgroundColor);
			pRoot->SetBackgroundImage(_T(""));
			CLabelUI* pPath = static_cast<CLabelUI*>(m_pm.FindControl(_T("wallpaper_path")));
			if( pPath != NULL ) pPath->SetText(_T("未设置（纯色）"));
		}
		else {
			ApplyWallpaperImage(param.bgimage.GetData());
		}
	}
	return TRUE;
}

void CMainWnd::ApplyWallpaperImage(LPCTSTR path)
{
	if( path == NULL || *path == _T('\0') ) {
		ClearWallpaperImage();
		return;
	}
	m_pm.SetWindowBackgroundColor(0);
	m_pm.SetWindowBackgroundImage(path);
	CControlUI* pRoot = m_pm.FindControl(_T("root"));
	if( pRoot != NULL )
		pRoot->SetBackgroundColor(0);
	CLabelUI* pPath = static_cast<CLabelUI*>(m_pm.FindControl(_T("wallpaper_path")));
	if( pPath != NULL ) {
		LPCTSTR pShow = path;
		LPCTSTR pSlash = _tcsrchr(path, _T('\\'));
		if( pSlash == NULL ) pSlash = _tcsrchr(path, _T('/'));
		if( pSlash != NULL && *(pSlash + 1) != _T('\0') ) pShow = pSlash + 1;
		pPath->SetText(pShow);
		pPath->SetToolTip(path);
	}
}

bool CMainWnd::ApplyWallpaperFromSvg(const char* utf8Svg, size_t nBytes, LPCTSTR pstrLabel)
{
	if( utf8Svg == NULL || nBytes == 0 ) return false;
	RECT rc = { 0 };
	::GetClientRect(m_hWnd, &rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	if( w < 256 ) w = 256;
	if( h < 256 ) h = 256;
	if( w > 1280 ) w = 1280;
	if( h > 1280 ) h = 1280;

	m_pm.SetWindowBackgroundColor(0);
	if( !m_pm.SetWindowBackgroundImageFromSvg(utf8Svg, nBytes, w, h, 0) )
		return false;

	CControlUI* pRoot = m_pm.FindControl(_T("root"));
	if( pRoot != NULL )
		pRoot->SetBackgroundColor(0);
	CLabelUI* pPath = static_cast<CLabelUI*>(m_pm.FindControl(_T("wallpaper_path")));
	if( pPath != NULL ) {
		CDuiString sShow = (pstrLabel != NULL && *pstrLabel != _T('\0')) ? pstrLabel : _T("(SVG)");
		pPath->SetText(sShow.GetData());
		pPath->SetToolTip(sShow.GetData());
	}
	return true;
}

void CMainWnd::ClearWallpaperImage()
{
	m_pm.SetWindowBackgroundImage(_T(""));
	CControlUI* pRoot = m_pm.FindControl(_T("root"));
	if( pRoot != NULL )
		pRoot->SetBackgroundColor(0);
	CLabelUI* pPath = static_cast<CLabelUI*>(m_pm.FindControl(_T("wallpaper_path")));
	if( pPath != NULL ) {
		pPath->SetText(_T("未设置"));
		pPath->SetToolTip(_T(""));
	}
}

void CMainWnd::PickWallpaperImage()
{
	TCHAR szFile[MAX_PATH] = { 0 };
	static TCHAR sFilter[] =
		_T("Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.gif)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif\0")
		_T("All Files (*.*)\0*.*\0");
	OPENFILENAME ofn;
	::ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = sFilter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrTitle = _T("选择窗口背景图");
	if( !::GetOpenFileName(&ofn) ) return;
	ApplyWallpaperImage(szFile);
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
	CDuiString name = msg.pSender ? msg.pSender->GetName() : CDuiString();
	if(msg.sType == _T("windowinit")) {
	}
	else if( msg.sType == DUI_MSGTYPE_TIMER )
	{
		if( msg.pSender != NULL && msg.pSender->GetName() == _T("btn_icon_loading")
			&& msg.wParam == 9101 ) {
			CButtonUI* pBtn = static_cast<CButtonUI*>(msg.pSender->GetInterface(DUI_CTR_BUTTON));
			if( pBtn != NULL ) {
				m_pm.KillTimer(pBtn, 9101);
				pBtn->SetLoading(false);
				pBtn->SetText(_T("提交"));
			}
			return;
		}
	}
	else if( msg.sType == DUI_MSGTYPE_TITLEBARCLOSING )
	{
		CTitleBarUI* pBar = static_cast<CTitleBarUI*>(msg.pSender->GetInterface(DUI_CTR_TITLEBAR));
		// close-to-tray：只是藏到托盘，不退出，无需确认
		if( pBar != NULL && pBar->IsCloseToTray() )
			return;
		if(MSGID_OK != CMsgWnd::MessageBox(m_hWnd, _T("提示"), _T("确定退出程序？")))
		{
			if( pBar != NULL ) pBar->CancelNotify();
		}
		return;
	}
	else if( msg.sType == DUI_MSGTYPE_ITEMSELECT )
	{
		if( msg.pSender && msg.pSender->GetName() == _T("vlist") ) {
			CLabelUI* pStatus = static_cast<CLabelUI*>(m_pm.FindControl(_T("vlist_status")));
			if( pStatus != NULL ) {
				CDuiString s;
				s.Format(_T("选中: %d"), (int)msg.wParam + 1);
				pStatus->SetText(s.GetData());
			}
		}
		else if( msg.pSender && msg.pSender->GetName() == _T("listview") ) {
			CLabelUI* pStatus = static_cast<CLabelUI*>(m_pm.FindControl(_T("listview_status")));
			if( pStatus != NULL ) {
				CDuiString s;
				s.Format(_T("选中: %d"), (int)msg.wParam + 1);
				pStatus->SetText(s.GetData());
			}
		}
	}
	else if( msg.sType == _T("textchanged") )
	{
		CEditUI* pEdit = (CEditUI*)msg.pSender;
	}
	else if( msg.sType == _T("colorchanging") || msg.sType == _T("colorchanged") )
	{
		DWORD dwColor = (DWORD)msg.wParam;
		if( dwColor == 0 && msg.pSender != NULL ) {
			CColorPaletteUI* pPal = static_cast<CColorPaletteUI*>(
				msg.pSender->GetInterface(DUI_CTR_COLORPALETTE));
			if( pPal != NULL ) dwColor = pPal->GetSelectColor();
		}
		CControlUI* pPreview = m_pm.FindControl(_T("palette_preview"));
		if( pPreview != NULL ) pPreview->SetBackgroundColor(dwColor);
		CLabelUI* pHex = static_cast<CLabelUI*>(m_pm.FindControl(_T("palette_hex")));
		if( pHex != NULL ) {
			CDuiString s;
			s.Format(_T("#%08X"), dwColor);
			pHex->SetText(s.GetData());
		}
	}
	else if(msg.sType == DUI_MSGTYPE_ITEMACTIVATE) {
		if(MSGID_OK == CMsgWnd::MessageBox(m_hWnd, _T("提示"), _T("确定退出程序？")))
		{
			::DestroyWindow(m_hWnd);
		}
	}
	else if(msg.sType == DUI_MSGTYPE_ITEMCLICK) {
		// listview / vlist 选中态由 itemselect 更新状态栏
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
		// TitleBar 系统按钮由控件自身处理（可取消 titlebarclosing 等）
		if( name.CompareNoCase(_T("closebtn")) == 0
			|| name.CompareNoCase(_T("minbtn")) == 0
			|| name.CompareNoCase(_T("maxbtn")) == 0
			|| name.CompareNoCase(_T("restorebtn")) == 0 )
		{
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
		if( name.CompareNoCase(_T("themeSwitch")) == 0 ) {
			CThemeManager* tm = CThemeManager::GetInstance();
			CLabelUI* pCur = static_cast<CLabelUI*>(m_pm.FindControl(_T("theme_current")));
			if( tm != NULL && pCur != NULL ) {
				CTheme* pTheme = tm->GetCurrentTheme();
				CDuiString s;
				s.Format(_T("当前: %s"), pTheme != NULL ? pTheme->GetDisplayName() : _T("default"));
				pCur->SetText(s.GetData());
				pCur->SetColor(tm->GetColor(_T("color-primary"), 0x1677FFFF));
			}
		}
		else {
			// 左侧大类导航 → 右侧 TabLayout
			CTabLayoutUI* pTabDemo = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("tab_demo")));
			if( pTabDemo != NULL ) {
				if(name.CompareNoCase(_T("nav_basic")) == 0) pTabDemo->SelectItem(0);
				else if(name.CompareNoCase(_T("nav_form")) == 0) pTabDemo->SelectItem(1);
				else if(name.CompareNoCase(_T("nav_layout")) == 0) pTabDemo->SelectItem(2);
				else if(name.CompareNoCase(_T("nav_feedback")) == 0) pTabDemo->SelectItem(3);
				else if(name.CompareNoCase(_T("nav_list")) == 0) pTabDemo->SelectItem(4);
				else if(name.CompareNoCase(_T("nav_misc")) == 0) pTabDemo->SelectItem(5);
			}
		}
	}
	else if(msg.sType == _T("valuechanged"))
	{
		// 仅同步主页 Slider→Progress；DateTime 等也会发 valuechanged
		if( msg.pSender != NULL && name.CompareNoCase(_T("slider")) == 0 )
		{
			CProgressUI* pSlider = static_cast<CProgressUI*>(msg.pSender->GetInterface(DUI_CTR_SLIDER));
			if( pSlider == NULL ) pSlider = static_cast<CProgressUI*>(msg.pSender->GetInterface(DUI_CTR_PROGRESS));
			CProgressUI* pPro1 = static_cast<CProgressUI*>(m_pm.FindControl(_T("progress")));
			CProgressUI* pPro2 = static_cast<CProgressUI*>(m_pm.FindControl(_T("circle_progress")));
			if( pSlider != NULL && pPro1 != NULL ) pPro1->SetValue(pSlider->GetValue());
			if( pSlider != NULL && pPro2 != NULL ) pPro2->SetValue(pSlider->GetValue());
		}
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
		m_pMenu->Init(NULL, _T("menu.html"), point, &m_pm);
	}

	return WindowImplBase::Notify(msg);
}
void CMainWnd::OnLClick(CControlUI *pControl)
{
	CDuiString sName = pControl->GetName();
	if(sName.CompareNoCase(_T("fonticon_click")) == 0
		|| sName.CompareNoCase(_T("fonticon_click2")) == 0)
	{
		CToast::ShowInfo(_T("FontIcon click"), 2000);
		return;
	}
	if(sName.CompareNoCase(_T("homepage_btn")) == 0)
	{
		//
		//CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("listview")));
		//CListContainerElementUI* pListItem  = new CListContainerElementUI();
		//pListItem->SetAlignItems(DT_VCENTER);
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
	else if(sName.CompareNoCase(_T("btn_browser_test")) == 0)
	{
		CBrowserWnd::Open(m_hWnd);
	}
	else if(sName.CompareNoCase(_T("btn_shape_wnd")) == 0)
	{
		CShapeDemoWnd* pWnd = new CShapeDemoWnd();
		pWnd->Create(m_hWnd, _T("异形窗口"), WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW, 0, 0, 800, 600);
		// 最终尺寸 / 居中在 InitWindow（按 apple.png + 工作区）
	}
	else if(sName.CompareNoCase(_T("shape_btn_circle")) == 0
		|| sName.CompareNoCase(_T("shape_btn_star")) == 0)
	{
		CToast::ShowSuccess(_T("点中了异形按钮（外形内）"), 2000);
	}
	else if(sName.CompareNoCase(_T("btn_behind_shape")) == 0)
	{
		CToast::ShowInfo(_T("点到了底下按钮（外形外穿透）"), 2000);
	}
	else if(sName.CompareNoCase(_T("btn_theme_toggle")) == 0)
	{
		CThemeManager* tm = CThemeManager::GetInstance();
		CLabelUI* pCur = static_cast<CLabelUI*>(m_pm.FindControl(_T("theme_current")));
		CButtonUI* pToggle = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_theme_toggle")));
		tm->SetEnabled(!tm->IsEnabled());
		if( pToggle != NULL )
			pToggle->SetText(tm->IsEnabled() ? _T("禁用主题") : _T("启用主题"));
		if( pCur != NULL ) {
			if( !tm->IsEnabled() ) {
				pCur->SetText(_T("当前: 已禁用（kind=Bootstrap，chrome=default）"));
				pCur->SetColor(0x0D6EFDFF);
			}
			else {
				CTheme* pTheme = tm->GetCurrentTheme();
				CDuiString s;
				s.Format(_T("当前: %s"), pTheme != NULL ? pTheme->GetDisplayName() : _T("default"));
				pCur->SetText(s.GetData());
				pCur->SetColor(tm->GetColor(_T("color-primary"), 0x1677FFFF));
			}
		}
		CThemeSwitcherUI* pSw = static_cast<CThemeSwitcherUI*>(m_pm.FindControl(_T("themeSwitch")));
		if( pSw != NULL ) pSw->SyncFromManager();
	}
	else if(sName.CompareNoCase(_T("btn_wallpaper")) == 0
		|| sName.CompareNoCase(_T("btn_wallpaper_pick")) == 0)
	{
		PickWallpaperImage();
	}
	else if(sName.CompareNoCase(_T("btn_wallpaper_clear")) == 0)
	{
		ClearWallpaperImage();
	}
	else if(sName.CompareNoCase(_T("btn_carousel_test")) == 0)
	{
		CCarouselTestWnd::Open(m_hWnd);
	}
	else if(sName.CompareNoCase(_T("btn_blankmenu_test")) == 0)
	{
		CBlankMenuWnd::Open(m_hWnd);
	}
	else if(sName.CompareNoCase(_T("btn_layout_test")) == 0)
	{
		CLayoutTestWnd::Open(m_hWnd);
	}
	else if(sName.CompareNoCase(_T("btn_loading_start")) == 0)
	{
		CLoadingUI* p = static_cast<CLoadingUI*>(m_pm.FindControl(_T("loading_demo")));
		if( p ) p->Start();
	}
	else if(sName.CompareNoCase(_T("btn_loading_stop")) == 0)
	{
		CLoadingUI* p = static_cast<CLoadingUI*>(m_pm.FindControl(_T("loading_demo")));
		if( p ) p->Stop();
	}
	else if(sName.CompareNoCase(_T("btn_icon_loading")) == 0)
	{
		CButtonUI* pBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_icon_loading")));
		if( pBtn && !pBtn->IsLoading() ) {
			pBtn->SetText(_T("提交中"));
			pBtn->SetLoading(true);
			m_pm.SetTimer(pBtn, 9101, 2000);
		}
	}
	else if(sName.CompareNoCase(_T("btn_list_empty_fill")) == 0)
	{
		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("list_empty_nested")));
		if( pList != NULL ) {
			CListLabelElementUI* pItem = new CListLabelElementUI;
			CDuiString s;
			s.Format(_T("新项 %d"), pList->GetCount() + 1);
			pItem->SetText(s.GetData());
			pItem->SetFixedHeight(28);
			pList->Add(pItem);
		}
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
	else if(sName.CompareNoCase(_T("btn_modal_nosync")) == 0)
	{
		CModal::Show(_T("不同步主窗"), _T("SyncOwnerMove(false)：拖动本对话框时主窗口留在原地。"),
			CModalOptions()
				.Kind(CONTROLKIND_SECONDARY)
				.Owner(m_hWnd)
				.SyncOwnerMove(false)
				.OnResult(OnModalResult));
	}
	else if(sName.CompareNoCase(_T("btn_settings_sync")) == 0)
	{
		CSettingsSyncWnd::Open(m_hWnd);
	}
	else if(sName.CompareNoCase(_T("btn_sidepanel_right")) == 0
		|| sName.CompareNoCase(_T("btn_sidepanel_left")) == 0
		|| sName.CompareNoCase(_T("btn_sidepanel_top")) == 0
		|| sName.CompareNoCase(_T("btn_sidepanel_bottom")) == 0)
	{
		CSidePanelUI* pSp = static_cast<CSidePanelUI*>(m_pm.FindControl(_T("demo_sidepanel")));
		if( pSp != NULL ) {
			pSp->SetFillHost(false);
			pSp->SetMaskEnabled(true);
			pSp->SetPanelWidthPercent(0.40f);
			pSp->SetPanelHeightPercent(0.45f);
			pSp->SetTitle(_T("侧滑面板"));
			CSidePanelUI::Placement e = CSidePanelUI::PlacementRight;
			if( sName.CompareNoCase(_T("btn_sidepanel_left")) == 0 )
				e = CSidePanelUI::PlacementLeft;
			else if( sName.CompareNoCase(_T("btn_sidepanel_top")) == 0 )
				e = CSidePanelUI::PlacementTop;
			else if( sName.CompareNoCase(_T("btn_sidepanel_bottom")) == 0 )
				e = CSidePanelUI::PlacementBottom;
			pSp->SetPlacement(e);
			pSp->Show(true);
		}
	}
	else if(sName.CompareNoCase(_T("btn_sidepanel_fill")) == 0
		|| sName.CompareNoCase(_T("btn_sidepanel_fill_left")) == 0
		|| sName.CompareNoCase(_T("btn_sidepanel_fill_top")) == 0
		|| sName.CompareNoCase(_T("btn_sidepanel_fill_bottom")) == 0)
	{
		CSidePanelUI* pSp = static_cast<CSidePanelUI*>(m_pm.FindControl(_T("demo_sidepanel")));
		if( pSp != NULL ) {
			CSidePanelUI::Placement e = CSidePanelUI::PlacementRight;
			LPCTSTR pszTitle = _T("设置（铺满·右）");
			if( sName.CompareNoCase(_T("btn_sidepanel_fill_left")) == 0 ) {
				e = CSidePanelUI::PlacementLeft;
				pszTitle = _T("设置（铺满·左）");
			}
			else if( sName.CompareNoCase(_T("btn_sidepanel_fill_top")) == 0 ) {
				e = CSidePanelUI::PlacementTop;
				pszTitle = _T("设置（铺满·上）");
			}
			else if( sName.CompareNoCase(_T("btn_sidepanel_fill_bottom")) == 0 ) {
				e = CSidePanelUI::PlacementBottom;
				pszTitle = _T("设置（铺满·下）");
			}
			pSp->SetFillHost(true);
			pSp->SetHostResize(true);
			pSp->SetPlacement(e);
			pSp->SetTitle(pszTitle);
			pSp->Show(true);
		}
	}
	else if(sName.CompareNoCase(_T("btn_sidepanel_close")) == 0
		|| sName.CompareNoCase(_T("btn_sidepanel_inner_close")) == 0)
	{
		CSidePanelUI* pSp = static_cast<CSidePanelUI*>(m_pm.FindControl(_T("demo_sidepanel")));
		if( pSp != NULL ) pSp->Hide(true);
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
		ShellExecute(NULL, _T("open"), sIEPath.GetData(), _T("http://jq.qq.com/?_wv=1027&k=cDTUzr"), NULL, SW_SHOW);
	}
	else if(sName.CompareNoCase(_T("qq_btn")) == 0)
	{
		TCHAR szPath[MAX_PATH] ={0};
		SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		CDuiString sIEPath;
		sIEPath.Format(_T("%s\\Internet Explorer\\iexplore.exe"), szPath);
		ShellExecute(NULL, _T("open"), sIEPath.GetData(), _T("tencent://Message/?Uin=656067418&Menu=yes"), NULL, SW_SHOW);
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
		m_pMenu->Init(NULL, _T("menu.html"), point, &m_pm);
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
		int nDPI = _ttoi(pControl->GetUserData().GetData());
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
		CControlUI* pControl = m_pm.FindControl(sUserData.GetData());
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
					CResourceManager::GetInstance()->LoadLanguage(_T("lan_en.html"));
				}
				else {
					CResourceManager::GetInstance()->SetLanguage(_T("cn_zh"));
					CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.html"));
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
				ForceClose(0);
			}
			else
			{
				CMsgWnd::MessageBox(m_hWnd, NULL, sText.GetData());
			}
		}
		bHandled = TRUE;
		return 0;
	}
	else if(uMsg == UIMSG_TRAYICON)
	{
		UINT uIconMsg = CTrayIcon::DecodeNotifyMsg(wParam, lParam, m_trayIcon.GetNotifyVersion());
		if(uIconMsg == WM_LBUTTONUP) {
			if( !::IsWindowVisible(m_hWnd) || CTrayIcon::IsWindowHiddenFromTaskbar(m_hWnd) )
				CTrayIcon::ShowWindowOnTaskbar(m_hWnd, true);
			else
				CTrayIcon::HideWindowFromTaskbar(m_hWnd);
		}
		else if(uIconMsg == WM_RBUTTONUP) {
			if(m_pMenu != NULL) {
				delete m_pMenu;
				m_pMenu = NULL;
			}
			m_pMenu = new CMenuWnd();
			CDuiPoint point = CTrayIcon::DecodeNotifyPos(wParam, lParam, m_trayIcon.GetNotifyVersion());
			m_pMenu->Init(NULL, _T("menu.html"), point, &m_pm);
			// 动态添加后重新设置菜单的大小
			m_pMenu->ResizeMenu();
		}
	}
	else if(uMsg == CTrayIcon::GetTaskbarCreatedMsg())
	{
		m_trayIcon.Recreate();
		bHandled = TRUE;
		return 0;
	}
	// WM_DPICHANGED 已由 WindowImplBase::OnDPIChanged 统一处理（含 SyncOwner）
	bHandled = FALSE;
	return 0;
}
//
//LRESULT CMainWnd::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	bHandled = FALSE;
//	return 0;
//}
