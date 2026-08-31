#include "StdAfx.h"
#include "UIButton.h"
#include "UISvgBox.h"
#include "UILoading.h"
#include "UIMenu.h"
#include "UIInputBox.h"
#include "UIHotKeyBox.h"
#include "UIHotKey.h"
#include <new>
#include <vector>

namespace DuiLib
{
	/////////////////////////////////////////////////////////////////////////////////////
	// 右键「修改文本」菜单点击：临时挂到 PaintManager，避免每个 Button 常驻 MessageFilter

	class CButtonEditTextMenuFilter : public IMessageFilterUI
	{
	public:
		static CButtonEditTextMenuFilter& Instance()
		{
			static CButtonEditTextMenuFilter s;
			return s;
		}

		void Arm(CButtonUI* pOwner)
		{
			Disarm();
			if( pOwner == NULL || pOwner->GetManager() == NULL ) return;
			m_pOwner = pOwner;
			m_pManager = pOwner->GetManager();
			m_pManager->AddMessageFilter(this);
		}

		void Disarm()
		{
			if( m_pManager != NULL )
				m_pManager->RemoveMessageFilter(this);
			m_pOwner = NULL;
			m_pManager = NULL;
		}

		void DisarmIf(CButtonUI* pOwner)
		{
			if( m_pOwner == pOwner )
				Disarm();
		}

		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled) override
		{
			bHandled = false;
			if( uMsg != WM_MENUCLICK || m_pOwner == NULL ) return 0;
			CButtonUI* p = m_pOwner;
			if( p->HandleEditTextMenuClick(wParam) ) {
				bHandled = true;
				Disarm();
			}
			return 0;
		}

	private:
		CButtonEditTextMenuFilter() : m_pOwner(NULL), m_pManager(NULL) {}
		CButtonUI* m_pOwner;
		CPaintManagerUI* m_pManager;
	};

	IMPLEMENT_DUICONTROL(CButtonUI)

	CButtonUI::CButtonUI()
		: m_uButtonState(0)
		, m_iHoverFont(-1)
		, m_iActiveFont(-1)
		, m_iFocusedFont(-1)
		, m_dwFocusedColor(0)
		, m_nStateCount(0)
		, m_iBindTabIndex(-1)
		, m_pIcon(NULL)
		, m_pRasterIcon(NULL)
		, m_pLoading(NULL)
		, m_eIconKind(IconNone)
		, m_bRasterMemKey(false)
		, m_pPendingIconData(NULL)
		, m_dwPendingIconSize(0)
		, m_hRasterTint(NULL)
		, m_dwRasterTintColor(0)
		, m_nRasterTintW(0)
		, m_nRasterTintH(0)
		, m_sLoadingType(_T("css"))
		, m_bLoading(false)
		, m_bLoadingDisable(true)
		, m_bEnabledBeforeLoading(true)
		, m_nIconSize(16)
		, m_nIconGap(4)
		, m_sIconPos(_T("left"))
		, m_dwIconTint(0)
		, m_dwIconTintHover(0)
		, m_dwIconTintActive(0)
		, m_dwIconTintDisabled(0)
		, m_dwIconTintFocus(0)
		, m_bIconTint(false)
		, m_bIconTintAuto(false)
		, m_bEditText(false)
		, m_bEditHotKey(false)
		, m_bEditTextMenuPending(false)
		, m_dwSubColor(0)
		, m_iSubFont(-1)
		, m_nSubGap(2)
		, m_wShortcutVk(0)
		, m_wShortcutMod(0)
		, m_nShortcutScope(HOTKEYBOX_SCOPE_APP)
	{
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		SetKind(CONTROLKIND_DEFAULT);
		// 可点按钮默认手型光标；皮肤可用 cursor="arrow" 覆盖
		SetCursor(DUI_HAND);
	}

	CButtonUI::~CButtonUI()
	{
		CButtonEditTextMenuFilter::Instance().DisarmIf(this);
		ClearPendingIconMemory();
		ReleaseMemIcon();
		if( m_pIcon != NULL ) {
			delete m_pIcon;
			m_pIcon = NULL;
		}
		if( m_pRasterIcon != NULL ) {
			delete m_pRasterIcon;
			m_pRasterIcon = NULL;
		}
		ClearRasterTintCache();
		if( m_pLoading != NULL ) {
			if( m_pManager != NULL ) m_pLoading->Stop();
			delete m_pLoading;
			m_pLoading = NULL;
		}
	}

	LPCTSTR CButtonUI::GetClass() const
	{
		return _T("ButtonUI");
	}

	LPVOID CButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_BUTTON) == 0 ) return static_cast<CButtonUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CButtonUI::GetControlFlags() const
	{
		return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	bool CButtonUI::PreferClientHit() const
	{
		if( !IsEnabled() ) return false;
		if( HasIcon() ) return true;
		if( m_dwFocusedColor != 0 ) return true;
		if( !m_sHoverImage.IsEmpty() || !m_sHoverForegroundImage.IsEmpty() ) return true;
		if( !m_sActiveImage.IsEmpty() || !m_sActiveForegroundImage.IsEmpty() ) return true;
		return CLabelUI::PreferClientHit();
	}

	void CButtonUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CLabelUI::SetManager(pManager, pParent, bInit);
		if( m_pIcon != NULL )
			m_pIcon->SetManager(pManager, this, bInit);
		if( m_pRasterIcon != NULL )
			m_pRasterIcon->SetManager(pManager, this, bInit);
		if( m_pLoading != NULL )
			m_pLoading->SetManager(pManager, this, bInit);
		if( m_pManager != NULL && m_pPendingIconData != NULL && m_dwPendingIconSize > 0 ) {
			BYTE* p = m_pPendingIconData;
			DWORD n = m_dwPendingIconSize;
			m_pPendingIconData = NULL;
			m_dwPendingIconSize = 0;
			ApplyIconFromMemory(p, n);
			delete[] p;
		}
	}

	void CButtonUI::SyncControlStateFromButton()
	{
		m_uControlState = m_uButtonState;
	}

	void CButtonUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KEYDOWN )
		{
			if (IsKeyboardEnabled()) {
				if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
					Activate();
					return;
				}
			}
		}		
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				SyncControlStateFromButton();
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);
			}
			return;
		}	
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
            if ((m_uButtonState & UISTATE_CAPTURED) != 0)
            {
                if (::PtInRect(&m_rcItem, event.ptMouse))
                    m_uButtonState |= UISTATE_PUSHED;
                else m_uButtonState &= ~UISTATE_PUSHED;
				SyncControlStateFromButton();
                Invalidate();
            }

			return;
		}
		if( event.Type == UIEVENT_BUTTONUP)
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				SyncControlStateFromButton();
				Invalidate();
				if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();				
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( (m_bEditText || m_bEditHotKey) && IsEnabled() && m_pManager != NULL ) {
				ShowEditTextMenu(event.ptMouse);
				return;
			}
			if( IsContextMenuUsed() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				SyncControlStateFromButton();
				Invalidate();

				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
			}
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				SyncControlStateFromButton();
				Invalidate();

				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
			}
		}
		CLabelUI::DoEvent(event);
	}

	bool CButtonUI::Activate()
	{
		if( !CControlUI::Activate() ) return false;
		if( m_pManager != NULL )
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
			BindTriggerTabSel();
		}
		return true;
	}

	void CButtonUI::SetEditTextEnabled(bool bEnable)
	{
		m_bEditText = bEnable;
		if( !m_bEditText && !m_bEditHotKey ) {
			m_bEditTextMenuPending = false;
			CButtonEditTextMenuFilter::Instance().DisarmIf(this);
		}
	}

	void CButtonUI::SetEditHotKeyEnabled(bool bEnable)
	{
		m_bEditHotKey = bEnable;
		if( !m_bEditText && !m_bEditHotKey ) {
			m_bEditTextMenuPending = false;
			CButtonEditTextMenuFilter::Instance().DisarmIf(this);
		}
	}

	void CButtonUI::SetSubText(LPCTSTR pstrText)
	{
		CDuiString s = pstrText ? pstrText : _T("");
		if( m_sSubText == s ) return;
		m_sSubText = s;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetSubColor(DWORD dwColor)
	{
		if( m_dwSubColor == dwColor ) return;
		m_dwSubColor = dwColor;
		Invalidate();
	}

	void CButtonUI::SetSubFont(int index)
	{
		if( m_iSubFont == index ) return;
		m_iSubFont = index;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetSubGap(int nGap)
	{
		if( nGap < 0 ) nGap = 0;
		if( m_nSubGap == nGap ) return;
		m_nSubGap = nGap;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetShortcutKey(WORD wVirtualKeyCode, WORD wModifiers, int scope)
	{
		m_wShortcutVk = wVirtualKeyCode;
		m_wShortcutMod = wModifiers;
		m_nShortcutScope = (scope == HOTKEYBOX_SCOPE_GLOBAL) ? HOTKEYBOX_SCOPE_GLOBAL : HOTKEYBOX_SCOPE_APP;
		if( wVirtualKeyCode == 0 && wModifiers == 0 ) {
			m_nShortcutScope = HOTKEYBOX_SCOPE_APP;
			SetSubText(_T(""));
		}
		else
			SetSubText(CHotKeyUI::FormatHotKeyName(wVirtualKeyCode, wModifiers).GetData());
	}

	void CButtonUI::GetShortcutKey(WORD& wVirtualKeyCode, WORD& wModifiers) const
	{
		wVirtualKeyCode = m_wShortcutVk;
		wModifiers = m_wShortcutMod;
	}

	void CButtonUI::GetShortcutKey(WORD& wVirtualKeyCode, WORD& wModifiers, int& scope) const
	{
		wVirtualKeyCode = m_wShortcutVk;
		wModifiers = m_wShortcutMod;
		scope = m_nShortcutScope;
	}

	void CButtonUI::ClearShortcutKey()
	{
		SetShortcutKey(0, 0, HOTKEYBOX_SCOPE_APP);
	}

	void CButtonUI::ShowEditTextMenu(POINT ptClient)
	{
		if( m_pManager == NULL ) return;
		if( !m_bEditText && !m_bEditHotKey ) return;

		// 根必须是 Window：Builder 只解析根的子节点
		static LPCTSTR sBuiltinMenuShell =
			_T("<Window>")
			_T("<Menu border-width=\"1\" border-radius=\"4,4\" ")
			_T("padding=\"4,4,4,4\" item-padding=\"0,14,0,32\" />")
			_T("</Window>");

		POINT ptScreen = ptClient;
		::ClientToScreen(m_pManager->GetPaintWindow(), &ptScreen);

		CMenuWnd* pMenuWnd = CMenuWnd::CreateMenu(NULL, STRINGorID(sBuiltinMenuShell), ptScreen, m_pManager, NULL,
			eMenuAlignment_Left | eMenuAlignment_Top);
		if( pMenuWnd == NULL ) return;

		CMenuUI* pMenu = pMenuWnd->GetMenuUI();
		if( pMenu == NULL ) {
			pMenuWnd->Close();
			return;
		}

		pMenu->RemoveAll();
		auto addItem = [&](LPCTSTR name, LPCTSTR text, LPCTSTR lucide) {
			CMenuElementUI* pItem = new CMenuElementUI;
			pItem->SetName(name);
			pItem->SetText(text);
			pItem->SetFixedHeight(30);
			pItem->SetAttribute(_T("lucide"), lucide);
			pItem->SetAttribute(_T("icon-size"), _T("16"));
			pItem->SetAttribute(_T("icon-tint"), _T("auto"));
			pMenu->Add(pItem);
		};

		if( m_bEditText )
			addItem(_T("button_edit_text"), _T("修改文本"), _T("pencil"));
		if( m_bEditHotKey ) {
			addItem(_T("button_edit_hotkey"), _T("设置快捷键"), _T("keyboard"));
			addItem(_T("button_clear_hotkey"), _T("清除快捷键"), _T("x"));
		}

		if( pMenu->GetCount() == 0 ) {
			pMenuWnd->Close();
			return;
		}

		pMenuWnd->ResizeMenu();
		m_bEditTextMenuPending = true;
		CButtonEditTextMenuFilter::Instance().Arm(this);
	}

	bool CButtonUI::HandleEditTextMenuClick(WPARAM wParam)
	{
		if( !m_bEditTextMenuPending ) return false;
		MenuCmd* pMenuCmd = (MenuCmd*)wParam;
		if( pMenuCmd == NULL ) return false;

		CDuiString sName = pMenuCmd->szName;
		const bool bEditText = m_bEditText && (sName.CompareNoCase(_T("button_edit_text")) == 0);
		const bool bEditHotKey = m_bEditHotKey && (sName.CompareNoCase(_T("button_edit_hotkey")) == 0);
		const bool bClearHotKey = m_bEditHotKey && (sName.CompareNoCase(_T("button_clear_hotkey")) == 0);
		if( !bEditText && !bEditHotKey && !bClearHotKey ) return false;

		if( m_pManager != NULL )
			m_pManager->DeletePtr(pMenuCmd);
		else
			delete pMenuCmd;

		m_bEditTextMenuPending = false;

		HWND hOwner = (m_pManager != NULL) ? m_pManager->GetPaintWindow() : NULL;
		if( bClearHotKey ) {
			ClearShortcutKey();
			return true;
		}
		if( bEditHotKey ) {
			WORD vk = m_wShortcutVk;
			WORD mod = m_wShortcutMod;
			int nScope = m_nShortcutScope;
			CDuiString sDisp;
			if( HOTKEYBOX_OK == CHotKeyBox::Show(hOwner,
					CHotKeyBoxOptions()
						.Title(_T("设置快捷键"))
						.Prompt(_T("请按下快捷键组合："))
						.HotKey(vk, mod)
						.Scope(nScope)
						.ConflictManager(m_pManager)
						.ExcludeControl(this),
					vk, mod, &sDisp, &nScope) ) {
				SetShortcutKey(vk, mod, nScope);
			}
			return true;
		}

		CDuiString sText = GetText();
		if( INPUTBOX_OK == CInputBox::Show(hOwner,
				CInputBoxOptions()
					.Title(_T("修改文本"))
					.Prompt(_T("请输入按钮文字："))
					.Value(sText.GetData()),
				sText) ) {
			SetText(sText.GetData());
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_TEXTCHANGED);
		}
		return true;
	}

	void CButtonUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState |= UISTATE_DISABLED;
		}
		else {
			m_uButtonState &= ~UISTATE_DISABLED;
		}
		SyncControlStateFromButton();
	}

	
	void CButtonUI::SetHoverFont(int index)
	{
		m_iHoverFont = index;
		Invalidate();
	}

	int CButtonUI::GetHoverFont() const
	{
		return m_iHoverFont;
	}

	void CButtonUI::SetActiveFont(int index)
	{
		m_iActiveFont = index;
		Invalidate();
	}

	int CButtonUI::GetActiveFont() const
	{
		return m_iActiveFont;
	}

	void CButtonUI::SetFocusedFont(int index)
	{
		m_iFocusedFont = index;
		Invalidate();
	}

	int CButtonUI::GetFocusedFont() const
	{
		return m_iFocusedFont;
	}

	void CButtonUI::SetFocusedColor(DWORD dwColor)
	{
		m_dwFocusedColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetFocusedColor() const
	{
		return m_dwFocusedColor;
	}

	LPCTSTR CButtonUI::GetImage()
	{
		return m_sImage.GetData();
	}

	void CButtonUI::SetImage(LPCTSTR pStrImage)
	{
		m_sImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHoverImage()
	{
		return m_sHoverImage.GetData();
	}

	void CButtonUI::SetHoverImage(LPCTSTR pStrImage)
	{
		m_sHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetActiveImage()
	{
		return m_sActiveImage.GetData();
	}

	void CButtonUI::SetActiveImage(LPCTSTR pStrImage)
	{
		m_sActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetFocusImage()
	{
		return m_sFocusImage.GetData();
	}

	void CButtonUI::SetFocusImage(LPCTSTR pStrImage)
	{
		m_sFocusImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetDisabledImage()
	{
		return m_sDisabledImage.GetData();
	}

	void CButtonUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHoverForegroundImage()
	{
		return m_sHoverForegroundImage.GetData();
	}

	void CButtonUI::SetHoverForegroundImage( LPCTSTR pStrImage )
	{
		m_sHoverForegroundImage = pStrImage;
		Invalidate();
	}

    LPCTSTR CButtonUI::GetActiveForegroundImage()
    {
        return m_sActiveForegroundImage.GetData();
    }

    void CButtonUI::SetActiveForegroundImage(LPCTSTR pStrImage)
    {
        m_sActiveForegroundImage = pStrImage;
        Invalidate();
    }

	void CButtonUI::SetStateCount(int nCount)
	{
		m_nStateCount = nCount;
		Invalidate();
	}

	int CButtonUI::GetStateCount() const
	{
		return m_nStateCount;
	}

	LPCTSTR CButtonUI::GetStateImage()
	{
		return m_sStateImage.GetData();
	}

	void CButtonUI::SetStateImage( LPCTSTR pStrImage )
	{
		m_sImage.Empty();
		m_sStateImage = pStrImage;
		Invalidate();
	}

	void CButtonUI::BindTabIndex(int _BindTabIndex )
	{
		if( _BindTabIndex >= 0)
			m_iBindTabIndex	= _BindTabIndex;
	}

	void CButtonUI::BindTabLayoutName( LPCTSTR _TabLayoutName )
	{
		if(_TabLayoutName)
			m_sBindTabLayoutName = _TabLayoutName;
	}

	void CButtonUI::BindTriggerTabSel( int _SetSelectIndex /*= -1*/ )
	{
		LPCTSTR pstrName = GetBindTabLayoutName();
		if(pstrName == NULL || (GetBindTabLayoutIndex() < 0 && _SetSelectIndex < 0))
			return;

		CTabLayoutUI* pTabLayout = static_cast<CTabLayoutUI*>(GetManager()->FindControl(pstrName));
		if(!pTabLayout) return;
		pTabLayout->SelectItem(_SetSelectIndex >=0?_SetSelectIndex:GetBindTabLayoutIndex());
	}

	void CButtonUI::RemoveBindTabIndex()
	{
		m_iBindTabIndex	= -1;
		m_sBindTabLayoutName.Empty();
	}

	int CButtonUI::GetBindTabLayoutIndex()
	{
		return m_iBindTabIndex;
	}

	LPCTSTR CButtonUI::GetBindTabLayoutName()
	{
		return m_sBindTabLayoutName.GetData();
	}

	void CButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("image")) == 0 ) SetImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-hover")) == 0 ) SetHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-active")) == 0 ) SetActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-focus")) == 0 ) SetFocusImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-disabled")) == 0 ) SetDisabledImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("foreground-image-hover")) == 0) SetHoverForegroundImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("foreground-image-active")) == 0) SetActiveForegroundImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("state-image")) == 0 ) SetStateImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("state-count")) == 0 ) SetStateCount(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("bind-tab-index")) == 0 ) BindTabIndex(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("bind-tab-layout-name")) == 0 ) BindTabLayoutName(pstrValue);
		else if( _tcsicmp(pstrName, _T("color-focus")) == 0 )
		{
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetFocusedColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("icon-size")) == 0 ) {
			SetIconSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("icon-gap")) == 0 ) {
			SetIconGap(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("icon-position")) == 0
			|| _tcsicmp(pstrName, _T("icon-pos")) == 0 ) {
			SetIconPosition(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint")) == 0
			|| _tcsicmp(pstrName, _T("icon-color")) == 0 ) {
			if( pstrValue == NULL || *pstrValue == _T('\0')
				|| _tcsicmp(pstrValue, _T("none")) == 0
				|| _tcsicmp(pstrValue, _T("false")) == 0
				|| _tcsicmp(pstrValue, _T("original")) == 0 ) {
				SetIconTintAuto(false);
				SetIconTint(0);
			}
			else if( _tcsicmp(pstrValue, _T("auto")) == 0
				|| _tcsicmp(pstrValue, _T("true")) == 0 ) {
				SetIconTintAuto(true);
			}
			else {
				DWORD clr = 0;
				if( ParseColorString(pstrValue, clr) ) SetIconTint(clr);
			}
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-hover")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintHover(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-active")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-active")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintActive(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-disabled")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintDisabled(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-focus")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-focus")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintFocus(clr);
		}
		else if( _tcsicmp(pstrName, _T("loading")) == 0 ) {
			SetLoading(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("loading-type")) == 0 ) {
			SetLoadingType(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("loading-disable")) == 0 ) {
			SetLoadingDisable(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0
				|| _tcsicmp(pstrValue, _T("yes")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("edit-text")) == 0 ) {
			SetEditTextEnabled(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0
				|| _tcsicmp(pstrValue, _T("yes")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("edit-hotkey")) == 0
			|| _tcsicmp(pstrName, _T("edit-shortcut")) == 0 ) {
			SetEditHotKeyEnabled(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0
				|| _tcsicmp(pstrValue, _T("yes")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("sub-text")) == 0
			|| _tcsicmp(pstrName, _T("subtitle")) == 0 ) {
			SetSubText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("sub-color")) == 0
			|| _tcsicmp(pstrName, _T("subtitle-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetSubColor(clr);
		}
		else if( _tcsicmp(pstrName, _T("sub-font")) == 0
			|| _tcsicmp(pstrName, _T("subtitle-font")) == 0 ) {
			SetSubFont(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("sub-gap")) == 0
			|| _tcsicmp(pstrName, _T("subtitle-gap")) == 0 ) {
			SetSubGap(_ttoi(pstrValue));
		}
		else if( IsIconAttr(pstrName) ) {
			if( pstrValue == NULL || *pstrValue == _T('\0') ) {
				ClearIcon();
				return;
			}
			if( _tcsicmp(pstrName, _T("icon-src")) == 0 || _tcsicmp(pstrName, _T("icon")) == 0 )
				SetIconSrc(pstrValue);
			else
				SetIconLib(pstrName, pstrValue);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CButtonUI::PaintText(IRenderContext& ctx)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;
		SyncControlStateFromButton();

		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();

		CDuiString sText = GetText();
		const bool bHasSub = HasSubText();
		// 纯图标按钮（无主/副文案）仍须绘制图标
		if( sText.IsEmpty() && !bHasSub && !HasIcon() && !IsLoading() ) return;

		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		GetManager()->GetDPIObj()->Scale(&rcTextPadding);
		RECT rcContent = m_rcItem;
		rcContent.left += rcPadding.left + rcTextPadding.left;
		rcContent.right -= rcPadding.right + rcTextPadding.right;
		rcContent.top += rcPadding.top + rcTextPadding.top;
		rcContent.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		RECT rcText = rcContent;
		if( HasIcon() ) {
			SyncIconAppearance();
			RECT rcIcon = { 0 };
			if( LayoutIconAndText(rcContent, rcIcon, rcText) ) {
				if( IsLoading() && m_pLoading != NULL ) {
					m_pLoading->SetPos(rcIcon, false);
					m_pLoading->Paint(ctx, m_rcPaint, NULL);
				}
				else if( m_eIconKind == IconRaster && m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() ) {
					PaintRasterIcon(ctx, rcIcon);
				}
				else if( m_pIcon != NULL && m_pIcon->IsVisible() ) {
					m_pIcon->SetPos(rcIcon, false);
					m_pIcon->PaintIcon(ctx, m_rcPaint);
				}
			}
		}

		DWORD clrColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetActiveColor() != 0) )
			clrColor = GetActiveColor();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHoverColor() != 0) )
			clrColor = GetHoverColor();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedColor() != 0) )
			clrColor = GetFocusedColor();

		int iFont = ResolvePaintMainFont();
		// DT_LEFT 值为 0，不能用 (style & LEFT) 判断；无 CENTER/RIGHT 即左对齐
		UINT uHAlign = DT_LEFT;
		if( (m_uTextStyle & DT_CENTER) != 0 ) uHAlign = DT_CENTER;
		else if( (m_uTextStyle & DT_RIGHT) != 0 ) uHAlign = DT_RIGHT;
		if( HasIcon() ) {
			const bool bVertical = (m_sIconPos.CompareNoCase(_T("top")) == 0
				|| m_sIconPos.CompareNoCase(_T("bottom")) == 0);
			uHAlign = bVertical ? DT_CENTER : DT_LEFT;
		}

		if( !bHasSub ) {
			if( sText.IsEmpty() ) return;
			UINT uStyle = (m_uTextStyle & ~(DT_CENTER | DT_RIGHT | DT_LEFT)) | uHAlign;
			if( (uStyle & (DT_VCENTER | DT_BOTTOM | DT_TOP)) == 0 )
				uStyle |= DT_VCENTER;
			int nLinks = 0;
			if( m_bShowHtml )
				ctx.DrawHtmlText(rcText, sText.GetData(), GetAdjustColor(clrColor), NULL, NULL, nLinks, iFont, uStyle);
			else
				ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(clrColor), iFont, uStyle);
			return;
		}

		const int iSubFont = ResolvePaintSubFont(iFont);
		SIZE szBlock = MeasureTitleBlock(iFont);
		int nGap = m_nSubGap;
		if( m_pManager != NULL )
			nGap = m_pManager->GetDPIObj()->Scale(m_nSubGap);

		SIZE szMain = { 0, 0 };
		SIZE szSub = { 0, 0 };
		if( m_pManager != NULL ) {
			UINT uMeas = DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT;
			if( !sText.IsEmpty() )
				szMain = RenderMeasureTextSize(m_pManager, sText.GetData(), iFont, uMeas);
			szSub = RenderMeasureTextSize(m_pManager, m_sSubText.GetData(), iSubFont, uMeas);
		}
		if( szMain.cy < 0 ) szMain.cy = 0;
		if( szSub.cy < 0 ) szSub.cy = 0;

		const int ch = rcText.bottom - rcText.top;
		int y = rcText.top;
		if( szBlock.cy < ch )
			y = rcText.top + (ch - szBlock.cy) / 2;

		UINT uLine = DT_SINGLELINE | DT_TOP | uHAlign;
		if( !sText.IsEmpty() ) {
			RECT rcMain = { rcText.left, y, rcText.right, y + szMain.cy };
			if( rcMain.bottom > rcText.bottom ) rcMain.bottom = rcText.bottom;
			int nLinks = 0;
			if( m_bShowHtml )
				ctx.DrawHtmlText(rcMain, sText.GetData(), GetAdjustColor(clrColor), NULL, NULL, nLinks, iFont, uLine);
			else
				ctx.DrawText(rcMain, sText.GetData(), GetAdjustColor(clrColor), iFont, uLine);
			y = rcMain.bottom + nGap;
		}

		RECT rcSub = { rcText.left, y, rcText.right, y + szSub.cy };
		if( rcSub.bottom > rcText.bottom ) rcSub.bottom = rcText.bottom;
		if( rcSub.top > rcSub.bottom ) rcSub.top = rcSub.bottom;
		DWORD clrSub = ResolveSubTextColor(clrColor);
		if( !IsEnabled() )
			clrSub = DuiColorSetA(m_dwDisabledColor != 0 ? m_dwDisabledColor : clrSub, 0x99);
		ctx.DrawText(rcSub, m_sSubText.GetData(), GetAdjustColor(clrSub), iSubFont, uLine);
	}

	void CButtonUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		SyncControlStateFromButton();
		CControlUI::PaintBackgroundColor(ctx);
	}

	void CButtonUI::PaintBackgroundImage(IRenderContext& ctx)
	{
		SyncControlStateFromButton();
		CControlUI::PaintBackgroundImage(ctx);
	}

	void CButtonUI::PaintStatusImage(IRenderContext& ctx)
	{
		if(!m_sStateImage.IsEmpty() && m_nStateCount > 0)
		{
			TDrawInfo info;
			info.Parse(m_sStateImage.GetData(), _T(""), m_pManager);
			const TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName.GetData(), info.sResType.GetData(), info.dwMask, info.bHSL, info.bGdiplus);
			if(m_sImage.IsEmpty() && pImage != NULL)
			{
				SIZE szImage = {pImage->nX, pImage->nY};
				SIZE szStatus = {pImage->nX / m_nStateCount, pImage->nY};
				if( szImage.cx > 0 && szImage.cy > 0 )
				{
					RECT rcSrc = {0, 0, szImage.cx, szImage.cy};
					if(m_nStateCount > 0) {
						int iLeft = rcSrc.left + 0 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 1) {
						int iLeft = rcSrc.left + 1 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sHoverImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
						m_sActiveImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 2) {
						int iLeft = rcSrc.left + 2 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sActiveImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 3) {
						int iLeft = rcSrc.left + 3 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sDisabledImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
				}
			}
		}

		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;
		// 窗口禁用时叠加 DISABLED；切勿 &=（会清掉 HOT，导致悬停底色有、图标着色无）
		if( m_pManager != NULL ) {
			HWND hPaint = m_pManager->GetPaintWindow();
			if( hPaint != NULL && !::IsWindowEnabled(hPaint) )
				m_uButtonState |= UISTATE_DISABLED;
		}
		SyncControlStateFromButton();
		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sDisabledImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sActiveImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sHoverImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sFocusImage.GetData()) ) {}
				else return;
			}
		}

		if( !m_sImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sImage.GetData()) ) {}
		}
	}

	void CButtonUI::PaintBorder(IRenderContext& ctx)
	{
		SyncControlStateFromButton();
		CControlUI::PaintBorder(ctx);
	}

	void CButtonUI::PaintForegroundImage(IRenderContext& ctx)
	{
		if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sActiveForegroundImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sActiveForegroundImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHoverForegroundImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sHoverForegroundImage.GetData()) ) {}
				else return;
			}
		}
		if(!m_sForegroundImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sForegroundImage.GetData()) ) {}
		}
	}

	void CButtonUI::SetKind(ControlKind kind)
	{
		InitKindColors();
		CControlUI::SetKind(kind);

		if (kind == CONTROLKIND_NONE) {
			// 清掉 ctor/上一 kind 残留的悬停·按下色，避免 none 仍带 default 灰底
			SetHoverBackgroundColor(0);
			SetHoverBorderColor(0);
			SetActiveBackgroundColor(0);
			SetActiveBorderColor(0);
			SetDisabledBackgroundColor(0);
			SetDisabledBorderColor(0);
			return;
		}
		if (m_bOutline) return;

		int idx = (int)kind;
		const KindStateColors& normal = g_kindColors[idx].Normal;
		const KindStateColors& hover = g_kindColors[idx].Hover;
		const KindStateColors& active = g_kindColors[idx].Active;

		SetColor(normal.dwColor);
		SetHoverBackgroundColor(hover.dwBackgroundColor);
		SetHoverColor(hover.dwColor);
		SetHoverBorderColor(hover.dwBorderColor);
		SetActiveBackgroundColor(active.dwBackgroundColor);
		SetActiveColor(active.dwColor);
		SetActiveBorderColor(active.dwBorderColor);
	}

	void CButtonUI::SetOutline(bool bOutline)
	{
		InitKindColors();
		m_bOutline = bOutline;
		int idx = (int)m_controlKind;

		if (bOutline && m_controlKind != CONTROLKIND_NONE) {
			DWORD outlineColor = g_kindColors[idx].Normal.dwBackgroundColor;
			if (outlineColor == 0) outlineColor = g_kindColors[idx].Normal.dwColor;
			if (m_controlKind == CONTROLKIND_LIGHT) outlineColor = 0x212529FF;

			SetBackgroundColor(0);
			SetForeColor(outlineColor);
			SetBorderColor(outlineColor);
			SetBorderWidth(1);
			SIZE round = {6, 6};
			SetBorderRadius(round);

			SetHoverBackgroundColor(g_kindColors[idx].Hover.dwBackgroundColor);
			SetHoverColor(0xFFFFFFFF);
			SetHoverBorderColor(g_kindColors[idx].Hover.dwBorderColor);
			SetActiveBackgroundColor(g_kindColors[idx].Active.dwBackgroundColor);
			SetActiveColor(0xFFFFFFFF);
			SetActiveBorderColor(g_kindColors[idx].Active.dwBorderColor);
		}
		else {
			SetKind(m_controlKind);
			return;
		}

		Invalidate();
	}

	void CButtonUI::EnsureIcon()
	{
		if( m_pIcon != NULL ) return;
		m_pIcon = new CSvgBoxUI;
		m_pIcon->SetMouseEnabled(false);
		m_pIcon->SetVisible(false);
		if( m_pManager != NULL )
			m_pIcon->SetManager(m_pManager, this, false);
	}

	void CButtonUI::EnsureRasterIcon()
	{
		if( m_pRasterIcon != NULL ) return;
		m_pRasterIcon = new CControlUI;
		m_pRasterIcon->SetMouseEnabled(false);
		m_pRasterIcon->SetVisible(false);
		if( m_pManager != NULL )
			m_pRasterIcon->SetManager(m_pManager, this, false);
	}

	void CButtonUI::EnsureLoading()
	{
		if( m_pLoading != NULL ) return;
		m_pLoading = new CLoadingUI;
		m_pLoading->SetMouseEnabled(false);
		m_pLoading->SetVisible(false);
		m_pLoading->SetAttribute(_T("type"), m_sLoadingType.IsEmpty() ? _T("css") : m_sLoadingType.GetData());
		m_pLoading->Stop();
		if( m_pManager != NULL )
			m_pLoading->SetManager(m_pManager, this, false);
	}

	bool CButtonUI::IsIconAttr(LPCTSTR pstrName) const
	{
		return _tcsicmp(pstrName, _T("bsicon")) == 0
			|| _tcsicmp(pstrName, _T("iconpark")) == 0
			|| _tcsicmp(pstrName, _T("lucide")) == 0
			|| _tcsicmp(pstrName, _T("tabler-outline")) == 0
			|| _tcsicmp(pstrName, _T("tabler-filled")) == 0
			|| _tcsicmp(pstrName, _T("remixicon")) == 0
			|| _tcsicmp(pstrName, _T("twicon")) == 0
			|| _tcsicmp(pstrName, _T("icon-src")) == 0
			|| _tcsicmp(pstrName, _T("icon")) == 0;
	}

	bool CButtonUI::IsRasterImagePath(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;
		CDuiString s(pstrPath);
		s.MakeLower();
		LPCTSTR pExt = NULL;
		for( LPCTSTR p = s.GetData(); *p != _T('\0'); ++p ) {
			if( *p == _T('.') ) pExt = p;
			else if( *p == _T('\'') || *p == _T('"') || *p == _T(' ') || *p == _T('\t') ) {
				if( pExt != NULL ) break;
			}
		}
		if( pExt == NULL ) return false;
		return _tcsncmp(pExt, _T(".bmp"), 4) == 0
			|| _tcsncmp(pExt, _T(".png"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpg"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpeg"), 5) == 0
			|| _tcsncmp(pExt, _T(".gif"), 4) == 0
			|| _tcsncmp(pExt, _T(".webp"), 5) == 0;
	}

	void CButtonUI::RefreshRasterIconImage()
	{
		if( m_pRasterIcon == NULL || m_sRasterPath.IsEmpty() ) return;
		if( !m_pRasterIcon->IsVisible() ) return;
		// dest 用逻辑尺寸，由 TDrawInfo::Parse 做一次 DPI Scale（勿预 Scale，否则会二次放大发糊）
		const int nSize = m_nIconSize > 0 ? m_nIconSize : 16;
		CDuiString sImg = m_sRasterPath;
		if( m_bRasterMemKey ) {
			// 必须用 file='key'：裸 key + dest= 时 TDrawInfo::Parse 会把整串当成图片名，
			// GetImage 找不到 AddImage 注册的 mem key → EXE/外壳图标空白（JPG file= 路径正常）。
			CDuiString sFmt;
			sFmt.Format(_T("file='%s' dest='0,0,%d,%d'"), m_sRasterPath.GetData(), nSize, nSize);
			sImg = sFmt;
		}
		else if( sImg.Find(_T("file=")) < 0 && sImg.Find(_T("res=")) < 0
			&& sImg.Find(_T("url(")) < 0 ) {
			CDuiString sFmt;
			sFmt.Format(_T("file='%s' dest='0,0,%d,%d'"), m_sRasterPath.GetData(), nSize, nSize);
			sImg = sFmt;
		}
		else if( sImg.Find(_T("dest=")) < 0 ) {
			CDuiString sFmt;
			sFmt.Format(_T("%s dest='0,0,%d,%d'"), m_sRasterPath.GetData(), nSize, nSize);
			sImg = sFmt;
		}
		m_pRasterIcon->SetBackgroundImage(sImg.GetData());
		ClearRasterTintCache();
	}

	void CButtonUI::ReleaseMemIcon()
	{
		if( !m_sMemIconKey.IsEmpty() && m_pManager != NULL )
			m_pManager->RemoveImage(m_sMemIconKey.GetData(), false);
		m_sMemIconKey.Empty();
		m_bRasterMemKey = false;
	}

	namespace {
		bool RasterizeHIconToPremul(HICON hIcon, int w, int h, BYTE* pOut)
		{
			if( hIcon == NULL || w < 1 || h < 1 || pOut == NULL ) return false;

			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = w;
			bmi.bmiHeader.biHeight = -h;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;

			void* pBlackBits = NULL;
			void* pWhiteBits = NULL;
			HBITMAP hbmBlack = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pBlackBits, NULL, 0);
			HBITMAP hbmWhite = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pWhiteBits, NULL, 0);
			if( hbmBlack == NULL || hbmWhite == NULL || pBlackBits == NULL || pWhiteBits == NULL ) {
				if( hbmBlack != NULL ) ::DeleteObject(hbmBlack);
				if( hbmWhite != NULL ) ::DeleteObject(hbmWhite);
				return false;
			}

			HDC hdc = ::CreateCompatibleDC(NULL);
			if( hdc == NULL ) {
				::DeleteObject(hbmBlack);
				::DeleteObject(hbmWhite);
				return false;
			}
			HGDIOBJ hOld = ::SelectObject(hdc, hbmBlack);
			::PatBlt(hdc, 0, 0, w, h, BLACKNESS);
			::DrawIconEx(hdc, 0, 0, hIcon, w, h, 0, NULL, DI_NORMAL);
			::SelectObject(hdc, hbmWhite);
			::PatBlt(hdc, 0, 0, w, h, WHITENESS);
			::DrawIconEx(hdc, 0, 0, hIcon, w, h, 0, NULL, DI_NORMAL);
			::SelectObject(hdc, hOld);
			::DeleteDC(hdc);

			BYTE* pB = static_cast<BYTE*>(pBlackBits);
			const BYTE* pW = static_cast<const BYTE*>(pWhiteBits);
			const int nPix = w * h;
			bool bHasAlpha = false;
			for( int i = 0; i < nPix; ++i ) {
				if( pB[i * 4 + 3] != 0 ) { bHasAlpha = true; break; }
			}
			if( !bHasAlpha ) {
				for( int i = 0; i < nPix; ++i ) {
					BYTE* d = pB + i * 4;
					const BYTE* s = pW + i * 4;
					int ar = 255 - ((int)s[2] - (int)d[2]);
					int ag = 255 - ((int)s[1] - (int)d[1]);
					int ab = 255 - ((int)s[0] - (int)d[0]);
					int a = (ar + ag + ab) / 3;
					if( a < 0 ) a = 0;
					if( a > 255 ) a = 255;
					d[3] = (BYTE)a;
				}
			}
			::DeleteObject(hbmWhite);

			for( int i = 0; i < nPix; ++i ) {
				BYTE* d = pB + i * 4;
				const BYTE a = d[3];
				if( a == 0 ) {
					d[0] = d[1] = d[2] = 0;
				}
				else if( a < 255 && (d[0] > a || d[1] > a || d[2] > a) ) {
					d[0] = (BYTE)((int)d[0] * a / 255);
					d[1] = (BYTE)((int)d[1] * a / 255);
					d[2] = (BYTE)((int)d[2] * a / 255);
				}
			}
			::CopyMemory(pOut, pB, (SIZE_T)nPix * 4);
			::DeleteObject(hbmBlack);
			return true;
		}

		bool HqScalePremul(const BYTE* pSrc, int srcW, int srcH,
			BYTE* pDst, int dstW, int dstH)
		{
			if( pSrc == NULL || pDst == NULL || srcW < 1 || srcH < 1 || dstW < 1 || dstH < 1 )
				return false;
			if( srcW == dstW && srcH == dstH ) {
				::CopyMemory(pDst, pSrc, (SIZE_T)dstW * dstH * 4);
				return true;
			}
			Gdiplus::Bitmap src(srcW, srcH, srcW * 4, PixelFormat32bppPARGB, (BYTE*)pSrc);
			if( src.GetLastStatus() != Gdiplus::Ok ) return false;
			Gdiplus::Bitmap dst(dstW, dstH, PixelFormat32bppPARGB);
			if( dst.GetLastStatus() != Gdiplus::Ok ) return false;
			{
				Gdiplus::Graphics g(&dst);
				g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
				g.Clear(Gdiplus::Color(0, 0, 0, 0));
				g.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
				g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
				g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
				g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
				g.DrawImage(&src, 0, 0, dstW, dstH);
			}
			Gdiplus::BitmapData bd = {};
			Gdiplus::Rect lockRc(0, 0, dstW, dstH);
			if( dst.LockBits(&lockRc, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bd) != Gdiplus::Ok )
				return false;
			BYTE* pOut = pDst;
			const BYTE* pIn = static_cast<const BYTE*>(bd.Scan0);
			for( int y = 0; y < dstH; ++y )
				::CopyMemory(pOut + y * dstW * 4, pIn + y * bd.Stride, (SIZE_T)dstW * 4);
			dst.UnlockBits(&bd);
			return true;
		}
	}

	HBITMAP CButtonUI::CreateBitmapFromHIcon(HICON hIcon, int cx, int cy)
	{
		if( hIcon == NULL || cx < 1 || cy < 1 ) return NULL;

		// 先按 HICON 固有尺寸栅格化，再用 Gdiplus 高品质缩到目标 —— DrawIconEx 直接缩 256→56 会发糊。
		int work = cx > cy ? cx : cy;
		ICONINFO ii = {};
		if( ::GetIconInfo(hIcon, &ii) ) {
			BITMAP bm = {};
			HBITMAP hbm = ii.hbmColor != NULL ? ii.hbmColor : ii.hbmMask;
			if( hbm != NULL && ::GetObject(hbm, sizeof(bm), &bm) && bm.bmWidth > 0 ) {
				int iw = bm.bmWidth;
				int ih = bm.bmHeight;
				if( ii.hbmColor == NULL ) ih /= 2;
				const int side = iw > ih ? iw : ih;
				if( side > work ) work = side;
			}
			if( ii.hbmColor != NULL ) ::DeleteObject(ii.hbmColor);
			if( ii.hbmMask != NULL ) ::DeleteObject(ii.hbmMask);
		}
		if( work > 512 ) work = 512;

		std::vector<BYTE> workBits((size_t)work * (size_t)work * 4);
		if( !RasterizeHIconToPremul(hIcon, work, work, workBits.data()) )
			return NULL;

		// 透明边裁剪（文本类型图标留白多）；EXE 图标几乎铺满则跳过，避免无谓重采样
		const BYTE kTrim = 12;
		int minX = work, minY = work, maxX = -1, maxY = -1;
		for( int y = 0; y < work; ++y ) {
			for( int x = 0; x < work; ++x ) {
				if( workBits[(size_t)(y * work + x) * 4 + 3] > kTrim ) {
					if( x < minX ) minX = x;
					if( y < minY ) minY = y;
					if( x > maxX ) maxX = x;
					if( y > maxY ) maxY = y;
				}
			}
		}
		int srcX = 0, srcY = 0, srcW = work, srcH = work;
		if( maxX >= minX && maxY >= minY ) {
			const int bw = maxX - minX + 1;
			const int bh = maxY - minY + 1;
			if( bw * 10 < work * 9 || bh * 10 < work * 9 ) {
				srcX = minX; srcY = minY; srcW = bw; srcH = bh;
			}
		}

		std::vector<BYTE> cropBits;
		const BYTE* pScaleSrc = workBits.data();
		int scaleW = work, scaleH = work;
		if( srcX != 0 || srcY != 0 || srcW != work || srcH != work ) {
			cropBits.resize((size_t)srcW * (size_t)srcH * 4);
			for( int y = 0; y < srcH; ++y ) {
				::CopyMemory(cropBits.data() + (size_t)y * srcW * 4,
					workBits.data() + ((size_t)(srcY + y) * work + srcX) * 4,
					(SIZE_T)srcW * 4);
			}
			pScaleSrc = cropBits.data();
			scaleW = srcW;
			scaleH = srcH;
		}

		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = cx;
		bmi.bmiHeader.biHeight = -cy;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		void* pBits = NULL;
		HBITMAP hbm = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
		if( hbm == NULL || pBits == NULL ) {
			if( hbm != NULL ) ::DeleteObject(hbm);
			return NULL;
		}
		::ZeroMemory(pBits, (SIZE_T)cx * cy * 4);

		// 等比放入目标格（contain）
		float sx = (float)cx / (float)scaleW;
		float sy = (float)cy / (float)scaleH;
		float scale = (sx < sy) ? sx : sy;
		int dw = (int)((float)scaleW * scale + 0.5f);
		int dh = (int)((float)scaleH * scale + 0.5f);
		if( dw < 1 ) dw = 1;
		if( dh < 1 ) dh = 1;
		if( dw > cx ) dw = cx;
		if( dh > cy ) dh = cy;
		const int ox = (cx - dw) / 2;
		const int oy = (cy - dh) / 2;

		if( dw == cx && dh == cy && scaleW == cx && scaleH == cy ) {
			::CopyMemory(pBits, pScaleSrc, (SIZE_T)cx * cy * 4);
			return hbm;
		}

		std::vector<BYTE> scaled((size_t)dw * (size_t)dh * 4);
		if( !HqScalePremul(pScaleSrc, scaleW, scaleH, scaled.data(), dw, dh) ) {
			::DeleteObject(hbm);
			return NULL;
		}
		BYTE* pDst = static_cast<BYTE*>(pBits);
		for( int y = 0; y < dh; ++y ) {
			::CopyMemory(pDst + ((size_t)(oy + y) * cx + ox) * 4,
				scaled.data() + (size_t)y * dw * 4, (SIZE_T)dw * 4);
		}
		return hbm;
	}

	void CButtonUI::ClearPendingIconMemory()
	{
		if( m_pPendingIconData != NULL ) {
			delete[] m_pPendingIconData;
			m_pPendingIconData = NULL;
		}
		m_dwPendingIconSize = 0;
	}

	namespace {
		bool LooksLikeSvgMemory(const BYTE* pData, DWORD dwSize)
		{
			if( pData == NULL || dwSize < 4 ) return false;
			DWORD i = 0;
			if( dwSize >= 3 && pData[0] == 0xEF && pData[1] == 0xBB && pData[2] == 0xBF ) i = 3;
			while( i < dwSize && (pData[i] == ' ' || pData[i] == '\t' || pData[i] == '\r' || pData[i] == '\n') )
				++i;
			if( i + 4 <= dwSize && _strnicmp((const char*)pData + i, "<svg", 4) == 0 ) return true;
			if( i + 5 <= dwSize && _strnicmp((const char*)pData + i, "<?xml", 5) == 0 ) return true;
			return false;
		}

		HBITMAP HIconToAlphaBitmap(HICON hIcon, int cx, int cy)
		{
			return CButtonUI::CreateBitmapFromHIcon(hIcon, cx, cy);
		}

		HICON CreateIconFromIcoMemory(const BYTE* pData, DWORD dwSize, int cx, int cy)
		{
			if( pData == NULL || dwSize < 6 ) return NULL;
			if( pData[0] != 0 || pData[1] != 0 || pData[2] != 1 || pData[3] != 0 )
				return NULL;
			int offset = ::LookupIconIdFromDirectoryEx((PBYTE)pData, TRUE, cx, cy, LR_DEFAULTCOLOR);
			if( offset == 0 ) return NULL;
			if( (DWORD)offset >= dwSize ) return NULL;
			return ::CreateIconFromResourceEx((PBYTE)pData + offset, dwSize - (DWORD)offset,
				TRUE, 0x00030000, cx, cy, LR_DEFAULTCOLOR);
		}
	}

	bool CButtonUI::ApplyIconFromMemory(const BYTE* pData, DWORD dwSize)
	{
		if( pData == NULL || dwSize == 0 || m_pManager == NULL ) return false;

		if( LooksLikeSvgMemory(pData, dwSize) ) {
			ReleaseMemIcon();
			EnsureIcon();
			char* psz = new (std::nothrow) char[(size_t)dwSize + 1];
			if( psz == NULL ) return false;
			::CopyMemory(psz, pData, dwSize);
			psz[dwSize] = '\0';
			m_pIcon->LoadFromUtf8Data(psz);
			delete[] psz;
			ShowSvgIcon();
			m_bNeedEstimateSize = true;
			Invalidate();
			return true;
		}

		TImageInfo* pInfo = CRenderEngine::LoadImageFromMemory(pData, dwSize, 0);
		if( pInfo != NULL && pInfo->hBitmap != NULL ) {
			HBITMAP hBmp = pInfo->hBitmap;
			int w = pInfo->nX;
			int h = pInfo->nY;
			bool bA = pInfo->bAlpha;
			pInfo->hBitmap = NULL;
			CRenderEngine::FreeImage(pInfo);
			return SetIconBitmap(hBmp, w, h, bA);
		}
		if( pInfo != NULL ) CRenderEngine::FreeImage(pInfo);

		int cx = m_nIconSize;
		if( m_pManager != NULL )
			cx = m_pManager->GetDPIObj()->Scale(m_nIconSize);
		if( cx < 16 ) cx = 16;
		HICON hIcon = CreateIconFromIcoMemory(pData, dwSize, cx, cx);
		if( hIcon == NULL ) return false;
		HBITMAP hbm = HIconToAlphaBitmap(hIcon, cx, cx);
		::DestroyIcon(hIcon);
		if( hbm == NULL ) return false;
		return SetIconBitmap(hbm, cx, cx, true);
	}

	bool CButtonUI::SetIconFromMemory(const BYTE* pData, DWORD dwSize)
	{
		ClearPendingIconMemory();
		if( pData == NULL || dwSize == 0 ) {
			ClearIcon();
			return false;
		}
		if( m_pManager == NULL ) {
			m_pPendingIconData = new (std::nothrow) BYTE[dwSize];
			if( m_pPendingIconData == NULL ) return false;
			::CopyMemory(m_pPendingIconData, pData, dwSize);
			m_dwPendingIconSize = dwSize;
			return true;
		}
		return ApplyIconFromMemory(pData, dwSize);
	}

	void CButtonUI::ClearRasterTintCache()
	{
		if( m_hRasterTint != NULL ) {
			IRenderDevice* pDev = GetRenderDevice();
			if( pDev != NULL ) pDev->InvalidateBitmapGpu(m_hRasterTint);
			::DeleteObject(m_hRasterTint);
			m_hRasterTint = NULL;
		}
		m_dwRasterTintColor = 0;
		m_nRasterTintW = 0;
		m_nRasterTintH = 0;
	}

	bool CButtonUI::EnsureRasterTintCache(DWORD dwColor)
	{
		if( m_pManager == NULL || m_sRasterPath.IsEmpty() || dwColor == 0 )
			return false;

		int nSize = m_nIconSize;
		nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
		if( nSize <= 0 ) return false;

		if( m_hRasterTint != NULL && m_dwRasterTintColor == dwColor
			&& m_nRasterTintW == nSize && m_nRasterTintH == nSize )
			return true;

		ClearRasterTintCache();

		CDuiString sName = m_sRasterPath;
		// file='xxx.png' … → 裸路径给 GetImageEx
		const int nFile = sName.Find(_T("file='"));
		if( nFile >= 0 ) {
			sName = sName.Mid(nFile + 6);
			const int nEnd = sName.Find(_T('\''));
			if( nEnd >= 0 ) sName = sName.Left(nEnd);
		}
		else {
			const int nUrl = sName.Find(_T("url("));
			if( nUrl >= 0 ) {
				CDuiString sPath;
				if( ParseCssUrlImage(m_sRasterPath.GetData(), sPath) )
					sName = sPath;
			}
		}

		const TImageInfo* pSrc = m_pManager->GetImageEx(sName.GetData());
		if( pSrc == NULL || pSrc->hBitmap == NULL || pSrc->nX <= 0 || pSrc->nY <= 0 )
			return false;

		BITMAP bm = { 0 };
		if( !::GetObject(pSrc->hBitmap, sizeof(bm), &bm) || bm.bmWidth <= 0 || bm.bmHeight <= 0 )
			return false;

		LPBYTE pSrcBits = NULL;
		BYTE* pTempBits = NULL;
		if( bm.bmBits != NULL ) {
			pSrcBits = (LPBYTE)bm.bmBits;
		}
		else if( pSrc->pBits != NULL ) {
			pSrcBits = pSrc->pBits;
		}
		else {
			pTempBits = new BYTE[pSrc->nX * pSrc->nY * 4];
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = pSrc->nX;
			bmi.bmiHeader.biHeight = -pSrc->nY;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			HDC hScreen = ::GetDC(NULL);
			int nCopied = ::GetDIBits(hScreen, pSrc->hBitmap, 0, pSrc->nY, pTempBits, &bmi, DIB_RGB_COLORS);
			::ReleaseDC(NULL, hScreen);
			if( nCopied == 0 ) {
				delete[] pTempBits;
				return false;
			}
			pSrcBits = pTempBits;
		}

		BITMAPINFO bmiOut = {};
		bmiOut.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiOut.bmiHeader.biWidth = nSize;
		bmiOut.bmiHeader.biHeight = -nSize;
		bmiOut.bmiHeader.biPlanes = 1;
		bmiOut.bmiHeader.biBitCount = 32;
		bmiOut.bmiHeader.biCompression = BI_RGB;
		LPBYTE pDest = NULL;
		HBITMAP hTint = ::CreateDIBSection(NULL, &bmiOut, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hTint == NULL || pDest == NULL ) {
			delete[] pTempBits;
			return false;
		}

		const BYTE tR = DuiColorR(dwColor);
		const BYTE tG = DuiColorG(dwColor);
		const BYTE tB = DuiColorB(dwColor);
		const int srcW = pSrc->nX;
		const int srcH = pSrc->nY;

		// 最近邻缩放到 icon-size，并按 alpha（无）重着色为 tint（预乘）
		for( int y = 0; y < nSize; ++y ) {
			const int sy = y * srcH / nSize;
			for( int x = 0; x < nSize; ++x ) {
				const int sx = x * srcW / nSize;
				const BYTE* pS = pSrcBits + (sy * srcW + sx) * 4;
				BYTE* pD = pDest + (y * nSize + x) * 4;
				BYTE a = pS[3];
				if( !pSrc->bAlpha ) {
					// 无 alpha（如 JPG）：用暗度作蒙版
					const int lum = (pS[2] * 30 + pS[1] * 59 + pS[0] * 11) / 100;
					a = (BYTE)(255 - lum);
				}
				pD[0] = (BYTE)((DWORD)tB * a / 255);
				pD[1] = (BYTE)((DWORD)tG * a / 255);
				pD[2] = (BYTE)((DWORD)tR * a / 255);
				pD[3] = a;
			}
		}

		delete[] pTempBits;
		m_hRasterTint = hTint;
		m_dwRasterTintColor = dwColor;
		m_nRasterTintW = nSize;
		m_nRasterTintH = nSize;
		return true;
	}

	void CButtonUI::PaintRasterIcon(IRenderContext& ctx, const RECT& rcIcon)
	{
		if( ShouldTintRasterIcon() ) {
			const DWORD paint = ResolvePaintIconColor();
			if( EnsureRasterTintCache(paint) && m_hRasterTint != NULL ) {
				RECT rcBmp = { 0, 0, m_nRasterTintW, m_nRasterTintH };
				RECT rcCorners = { 0, 0, 0, 0 };
				ctx.DrawImage(m_hRasterTint, rcIcon, m_rcPaint, rcBmp, rcCorners, true, ScaleImageFade());
				return;
			}
		}
		// 未请求着色 / 着色失败 → 原图
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetPos(rcIcon, false);
			m_pRasterIcon->Paint(ctx, m_rcPaint, NULL);
		}
	}

	bool CButtonUI::ShouldTintRasterIcon() const
	{
		if( m_eIconKind != IconRaster ) return false;
		if( m_bIconTintAuto || m_bIconTint ) return true;
		if( !IsEnabled() || (m_uButtonState & UISTATE_DISABLED) != 0 )
			return m_dwIconTintDisabled != 0;
		if( (m_uButtonState & UISTATE_PUSHED) != 0 )
			return m_dwIconTintActive != 0;
		if( (m_uButtonState & UISTATE_HOT) != 0 )
			return m_dwIconTintHover != 0;
		if( (m_uButtonState & UISTATE_FOCUSED) != 0 )
			return m_dwIconTintFocus != 0;
		return false;
	}

	void CButtonUI::ShowSvgIcon()
	{
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
		}
		m_sRasterPath.Empty();
		ClearRasterTintCache();
		m_eIconKind = IconSvg;
		if( m_pIcon != NULL && !m_bLoading )
			m_pIcon->SetVisible(true);
	}

	void CButtonUI::ShowRasterIcon(LPCTSTR pstrPath)
	{
		EnsureRasterIcon();
		if( m_pIcon != NULL )
			m_pIcon->SetVisible(false);
		ReleaseMemIcon();
		m_sRasterPath = pstrPath ? pstrPath : _T("");
		m_eIconKind = IconRaster;
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(!m_bLoading);
			if( !m_bLoading )
				RefreshRasterIconImage();
		}
	}

	bool CButtonUI::SetIconBitmap(HBITMAP hBitmap, int nWidth, int nHeight, bool bAlpha)
	{
		if( hBitmap == NULL || nWidth <= 0 || nHeight <= 0 ) {
			if( hBitmap != NULL ) ::DeleteObject(hBitmap);
			return false;
		}
		if( m_pManager == NULL ) {
			::DeleteObject(hBitmap);
			return false;
		}

		ReleaseMemIcon();
		static volatile LONG s_nMemIconSeq = 0;
		LONG nSeq = ::InterlockedIncrement(&s_nMemIconSeq);
		m_sMemIconKey.Format(_T("_dui_btn_icon_%p_%ld"), this, nSeq);
		if( m_pManager->AddImage(m_sMemIconKey.GetData(), hBitmap, nWidth, nHeight, bAlpha, false) == NULL ) {
			::DeleteObject(hBitmap);
			m_sMemIconKey.Empty();
			return false;
		}

		EnsureRasterIcon();
		if( m_pIcon != NULL )
			m_pIcon->SetVisible(false);
		m_bRasterMemKey = true;
		m_sRasterPath = m_sMemIconKey;
		m_eIconKind = IconRaster;
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(!m_bLoading);
			if( !m_bLoading )
				RefreshRasterIconImage();
		}
		m_bNeedEstimateSize = true;
		Invalidate();
		return true;
	}

	void CButtonUI::SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName)
	{
		if( pstrLib == NULL || *pstrLib == _T('\0')
			|| pstrName == NULL || *pstrName == _T('\0')
			|| !IsIconAttr(pstrLib) ) {
			ClearIcon();
			return;
		}
		ReleaseMemIcon();
		EnsureIcon();
		m_pIcon->SetAttribute(pstrLib, pstrName);
		ShowSvgIcon();
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconSrc(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) {
			ClearIcon();
			return;
		}
		ReleaseMemIcon();
		if( IsRasterImagePath(pstrPath) ) {
			ShowRasterIcon(pstrPath);
		}
		else {
			EnsureIcon();
			m_pIcon->SetAttribute(_T("src"), pstrPath);
			ShowSvgIcon();
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::ClearIcon()
	{
		ClearPendingIconMemory();
		ReleaseMemIcon();
		m_eIconKind = IconNone;
		m_sRasterPath.Empty();
		ClearRasterTintCache();
		if( m_pIcon != NULL ) {
			m_pIcon->SetVisible(false);
			m_pIcon->LoadFromUtf8Data("");
		}
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	bool CButtonUI::HasIcon() const
	{
		if( m_bLoading ) return true;
		if( m_eIconKind == IconSvg && m_pIcon != NULL && m_pIcon->IsVisible() ) return true;
		if( m_eIconKind == IconRaster && m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() ) return true;
		return false;
	}

	void CButtonUI::SetLoading(bool bLoading)
	{
		if( m_bLoading == bLoading ) return;
		EnsureLoading();
		if( bLoading ) {
			m_bEnabledBeforeLoading = IsEnabled();
			m_bLoading = true;
			if( m_pIcon != NULL ) m_pIcon->SetVisible(false);
			if( m_pRasterIcon != NULL ) m_pRasterIcon->SetVisible(false);
			SyncLoadingAppearance();
			m_pLoading->SetVisible(true);
			m_pLoading->Start();
			if( m_bLoadingDisable )
				SetEnabled(false);
		}
		else {
			m_bLoading = false;
			if( m_pLoading != NULL ) {
				m_pLoading->Stop();
				m_pLoading->SetVisible(false);
			}
			RestoreIconAfterLoading();
			if( m_bLoadingDisable )
				SetEnabled(m_bEnabledBeforeLoading);
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	bool CButtonUI::IsLoading() const
	{
		return m_bLoading && m_pLoading != NULL && m_pLoading->IsVisible();
	}

	void CButtonUI::SetLoadingType(LPCTSTR pstrType)
	{
		m_sLoadingType = pstrType ? pstrType : _T("css");
		if( m_sLoadingType.IsEmpty() ) m_sLoadingType = _T("css");
		if( IsLoading() )
			SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetLoadingDisable(bool bDisable)
	{
		if( m_bLoadingDisable == bDisable ) return;
		m_bLoadingDisable = bDisable;
		if( m_bLoading ) {
			if( bDisable )
				SetEnabled(false);
			else
				SetEnabled(m_bEnabledBeforeLoading);
		}
	}

	void CButtonUI::RestoreIconAfterLoading()
	{
		if( m_eIconKind == IconSvg && m_pIcon != NULL )
			m_pIcon->SetVisible(true);
		else if( m_eIconKind == IconRaster && m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(true);
			RefreshRasterIconImage();
		}
	}

	void CButtonUI::SyncLoadingAppearance()
	{
		if( m_pLoading == NULL || !m_bLoading ) return;
		LPCTSTR pType = m_sLoadingType.IsEmpty() ? _T("css") : m_sLoadingType.GetData();
		m_pLoading->SetAttribute(_T("type"), pType);
		DWORD dwColor = ResolvePaintIconColor();
		if( dwColor == 0 ) dwColor = 0x1677FFFF;
		CDuiString sClr;
		sClr.Format(_T("#%08X"), dwColor);
		m_pLoading->SetAttribute(_T("color"), sClr.GetData());
	}

	void CButtonUI::SetIconSize(int nSize)
	{
		if( nSize < 8 ) nSize = 8;
		if( nSize > 64 ) nSize = 64;
		if( m_nIconSize == nSize ) return;
		m_nIconSize = nSize;
		if( m_eIconKind == IconRaster ) {
			ClearRasterTintCache();
			RefreshRasterIconImage();
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconGap(int nGap)
	{
		if( nGap < 0 ) nGap = 0;
		if( m_nIconGap == nGap ) return;
		m_nIconGap = nGap;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconPosition(LPCTSTR pstrPos)
	{
		CDuiString s = pstrPos ? pstrPos : _T("left");
		if( s.CompareNoCase(_T("right")) != 0
			&& s.CompareNoCase(_T("top")) != 0
			&& s.CompareNoCase(_T("bottom")) != 0 )
			s = _T("left");
		if( m_sIconPos == s ) return;
		m_sIconPos = s;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconTint(DWORD dwColor)
	{
		m_bIconTint = (dwColor != 0);
		m_dwIconTint = dwColor;
		if( m_bIconTint ) m_bIconTintAuto = false;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintAuto(bool bAuto)
	{
		const bool bClearExplicit = bAuto && m_bIconTint;
		if( m_bIconTintAuto == bAuto && !bClearExplicit ) return;
		m_bIconTintAuto = bAuto;
		if( bAuto ) {
			m_bIconTint = false;
			m_dwIconTint = 0;
		}
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintHover(DWORD dwColor)
	{
		m_dwIconTintHover = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintActive(DWORD dwColor)
	{
		m_dwIconTintActive = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintDisabled(DWORD dwColor)
	{
		m_dwIconTintDisabled = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintFocus(DWORD dwColor)
	{
		m_dwIconTintFocus = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	DWORD CButtonUI::ResolveIconColor() const
	{
		if( m_bIconTint && m_dwIconTint != 0 )
			return m_dwIconTint;
		if( m_dwColor != 0 ) return m_dwColor;
		if( m_pManager != NULL ) return m_pManager->GetDefaultFontColor();
		return 0x000000E0;
	}

	DWORD CButtonUI::ResolvePaintIconColor() const
	{
		DWORD clr = ResolveIconColor();
		DWORD clrHover = m_dwIconTintHover != 0 ? m_dwIconTintHover
			: (GetHoverColor() != 0 ? GetHoverColor() : clr);
		DWORD clrActive = m_dwIconTintActive != 0 ? m_dwIconTintActive
			: (GetActiveColor() != 0 ? GetActiveColor() : clr);
		DWORD clrDisabled = m_dwIconTintDisabled != 0 ? m_dwIconTintDisabled
			: (m_dwDisabledColor != 0 ? m_dwDisabledColor : clr);
		DWORD clrFocus = m_dwIconTintFocus != 0 ? m_dwIconTintFocus
			: (GetFocusedColor() != 0 ? GetFocusedColor() : clr);

		if( !IsEnabled() || (m_uButtonState & UISTATE_DISABLED) != 0 )
			return clrDisabled;
		if( (m_uButtonState & UISTATE_PUSHED) != 0 )
			return clrActive;
		if( (m_uButtonState & UISTATE_HOT) != 0 )
			return clrHover;
		if( (m_uButtonState & UISTATE_FOCUSED) != 0 )
			return clrFocus;
		return clr;
	}

	void CButtonUI::SyncIconAppearance()
	{
		if( m_pIcon == NULL || m_eIconKind != IconSvg ) return;
		m_pIcon->SetEnabled(IsEnabled());

		// 嵌套 SvgBox 为 mouse=false，自身不会进 HOT。按父按钮当前态把绘制色直接写入
		//（与 PaintRasterIcon 用 ResolvePaintIconColor 同一路径），勿依赖 SvgBox 悬停态。
		const DWORD paint = ResolvePaintIconColor();
		m_pIcon->SetHoverColor(0, false);
		m_pIcon->SetActiveColor(0, false);
		m_pIcon->SetDisabledColor(0, false);
		m_pIcon->ApplyParentButtonState(0);
		m_pIcon->SetColor(paint, false);
	}

	SIZE CButtonUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = CLabelUI::EstimateSize(szAvailable);
		const bool bSub = HasSubText();
		if( !HasIcon() && !bSub ) return sz;
		// 宽高都写死时不改
		if( m_cxyFixed.cx > 0 && m_cxyFixed.cy > 0 ) return sz;

		int nSize = m_nIconSize;
		int nGap = m_nIconGap;
		int nSubGap = m_nSubGap;
		if( m_pManager != NULL ) {
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
			nGap = m_pManager->GetDPIObj()->Scale(m_nIconGap);
			nSubGap = m_pManager->GetDPIObj()->Scale(m_nSubGap);
		}

		RECT rcTextPadding = GetTextPadding();
		RECT rcPadding = GetPadding();
		const int padL = rcPadding.left + rcTextPadding.left;
		const int padR = rcPadding.right + rcTextPadding.right;
		const int padT = rcPadding.top + rcTextPadding.top;
		const int padB = rcPadding.bottom + rcTextPadding.bottom;

		const bool bHasText = !GetText().IsEmpty() || bSub;
		const bool bVertical = (m_sIconPos.CompareNoCase(_T("top")) == 0
			|| m_sIconPos.CompareNoCase(_T("bottom")) == 0);

		SIZE szBlock = { 0, 0 };
		if( bSub || HasIcon() )
			szBlock = MeasureTitleBlock(ResolvePaintMainFont());

		if( bSub && m_cxyFixed.cy == 0 ) {
			// Label 已按单行估高；补上副行
			int subH = 0;
			if( m_pManager != NULL ) {
				SIZE szSub = RenderMeasureTextSize(m_pManager, m_sSubText.GetData(),
					ResolvePaintSubFont(ResolvePaintMainFont()), DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT);
				subH = szSub.cy;
			}
			if( subH > 0 )
				sz.cy += nSubGap + subH;
			if( m_cxyFixed.cx == 0 && GetAutoCalcWidth() && szBlock.cx > 0 ) {
				const int want = szBlock.cx + padL + padR;
				if( sz.cx < want ) sz.cx = want;
			}
		}

		if( !HasIcon() ) {
			m_cxyFixedLast = sz;
			return sz;
		}

		if( m_cxyFixed.cx == 0 ) {
			// 未写 width / width=auto 时：仅在需要固有宽（auto-calc 或纯图标）时给出最小宽。
			// 有文字且非 auto：保持 cx=0，让父布局（如 TreeNode 内 HBox）撑满；
			// 否则原先无条件 += icon 会把控件估成 ~20px，再叠加 Option 左右 padding 图标被裁残。
			if( !bHasText ) {
				sz.cx = nSize + padL + padR;
			}
			else if( GetAutoCalcWidth() ) {
				if( bVertical ) {
					const int minW = (szBlock.cx > nSize ? szBlock.cx : nSize) + padL + padR;
					if( sz.cx < minW ) sz.cx = minW;
				}
				else {
					const int want = nSize + nGap + szBlock.cx + padL + padR;
					if( sz.cx < want ) sz.cx = want;
				}
			}
		}
		if( m_cxyFixed.cy == 0 ) {
			if( !bHasText ) {
				sz.cy = nSize + padT + padB;
			}
			else if( bVertical ) {
				sz.cy += nSize + nGap;
			}
			else {
				const int minH = (szBlock.cy > nSize ? szBlock.cy : nSize) + padT + padB;
				if( sz.cy < minH ) sz.cy = minH;
			}
		}

		m_cxyFixedLast = sz;
		return sz;
	}

	int CButtonUI::ResolvePaintMainFont() const
	{
		int iFont = GetFont();
		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetActiveFont() != -1) )
			iFont = GetActiveFont();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHoverFont() != -1) )
			iFont = GetHoverFont();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1) )
			iFont = GetFocusedFont();
		return iFont;
	}

	int CButtonUI::ResolvePaintSubFont(int iMainFont) const
	{
		return (m_iSubFont >= 0) ? m_iSubFont : iMainFont;
	}

	DWORD CButtonUI::ResolveSubTextColor(DWORD clrMain) const
	{
		if( m_dwSubColor != 0 )
			return m_dwSubColor;

		const ControlKind kind = GetKind();
		const bool bSolidKind = (kind != CONTROLKIND_NONE && kind != CONTROLKIND_DEFAULT
			&& kind != CONTROLKIND_LINK && kind != CONTROLKIND_LIGHT
			&& !IsOutline());
		if( bSolidKind )
			return DuiColorSetA(clrMain, 0xB3);

		CThemeManager* pTm = CThemeManager::GetInstance();
		if( pTm != NULL && pTm->IsEnabled() ) {
			DWORD sec = pTm->GetColor(_T("color-text-secondary"), 0);
			if( sec != 0 ) return sec;
		}
		return DuiColorSetA(clrMain != 0 ? clrMain : 0x000000FF, 0x99);
	}

	SIZE CButtonUI::MeasureTitleBlock(int iMainFont) const
	{
		SIZE sz = { 0, 0 };
		if( m_pManager == NULL ) return sz;
		const int iSubFont = ResolvePaintSubFont(iMainFont);
		UINT uMeas = DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT;
		CDuiString sText = GetText();
		SIZE szMain = { 0, 0 };
		SIZE szSub = { 0, 0 };
		if( !sText.IsEmpty() )
			szMain = RenderMeasureTextSize(const_cast<CPaintManagerUI*>(m_pManager),
				sText.GetData(), iMainFont, uMeas);
		if( HasSubText() )
			szSub = RenderMeasureTextSize(const_cast<CPaintManagerUI*>(m_pManager),
				m_sSubText.GetData(), iSubFont, uMeas);
		sz.cx = szMain.cx > szSub.cx ? szMain.cx : szSub.cx;
		sz.cy = szMain.cy;
		if( HasSubText() ) {
			int nGap = m_pManager->GetDPIObj()->Scale(m_nSubGap);
			if( szMain.cy > 0 && szSub.cy > 0 )
				sz.cy += nGap;
			sz.cy += szSub.cy;
		}
		if( sz.cx < 0 ) sz.cx = 0;
		if( sz.cy < 0 ) sz.cy = 0;
		return sz;
	}

	bool CButtonUI::LayoutIconAndText(const RECT& rcContent, RECT& rcIcon, RECT& rcText) const
	{
		rcText = rcContent;
		::ZeroMemory(&rcIcon, sizeof(rcIcon));
		if( !HasIcon() ) return false;

		int nSize = m_nIconSize;
		int nGap = m_nIconGap;
		if( m_pManager != NULL ) {
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
			nGap = m_pManager->GetDPIObj()->Scale(m_nIconGap);
		}
		const int cw = rcContent.right - rcContent.left;
		const int ch = rcContent.bottom - rcContent.top;
		if( cw <= 0 || ch <= 0 ) return false;
		if( nSize > cw ) nSize = cw;

		const bool bHasText = !GetText().IsEmpty() || HasSubText();
		const bool bTop = (m_sIconPos.CompareNoCase(_T("top")) == 0);
		const bool bBottom = (m_sIconPos.CompareNoCase(_T("bottom")) == 0);
		const bool bRight = (m_sIconPos.CompareNoCase(_T("right")) == 0);

		if( !bHasText ) {
			if( nSize > ch ) nSize = ch;
			rcIcon.left = rcContent.left + (cw - nSize) / 2;
			rcIcon.top = rcContent.top + (ch - nSize) / 2;
			rcIcon.right = rcIcon.left + nSize;
			rcIcon.bottom = rcIcon.top + nSize;
			rcText = rcContent;
			return true;
		}

		SIZE szText = MeasureTitleBlock(ResolvePaintMainFont());

		if( bTop || bBottom ) {
			const int nTextReserve = szText.cy + nGap;
			if( nSize > ch - nTextReserve && ch > nTextReserve )
				nSize = ch - nTextReserve;
			else if( nSize > ch )
				nSize = ch;

			int blockH = nSize + nGap + szText.cy;
			if( blockH > ch ) blockH = ch;
			int y = rcContent.top + (ch - blockH) / 2;
			const int yEnd = (y + blockH > rcContent.bottom) ? rcContent.bottom : (y + blockH);
			rcIcon.left = rcContent.left + (cw - nSize) / 2;
			rcIcon.right = rcIcon.left + nSize;
			rcText.left = rcContent.left;
			rcText.right = rcContent.right;
			if( bBottom ) {
				rcText.top = y;
				rcText.bottom = y + szText.cy;
				if( rcText.bottom > yEnd ) rcText.bottom = yEnd;
				if( rcText.top > rcText.bottom ) rcText.top = rcText.bottom;
				rcIcon.top = rcText.bottom + nGap;
				rcIcon.bottom = rcIcon.top + nSize;
				if( rcIcon.bottom > yEnd ) {
					rcIcon.bottom = yEnd;
					rcIcon.top = rcIcon.bottom - nSize;
					if( rcIcon.top < rcText.bottom ) rcIcon.top = rcText.bottom;
				}
			}
			else {
				rcIcon.top = y;
				rcIcon.bottom = rcIcon.top + nSize;
				rcText.top = rcIcon.bottom + nGap;
				rcText.bottom = yEnd;
				if( rcText.top > rcText.bottom ) rcText.top = rcText.bottom;
			}
			return true;
		}

		if( nSize > ch ) nSize = ch;

		int blockW = nSize + nGap + szText.cx;
		if( blockW > cw ) {
			if( bRight ) {
				rcIcon.right = rcContent.right;
				rcIcon.left = rcIcon.right - nSize;
				rcIcon.top = rcContent.top + (ch - nSize) / 2;
				rcIcon.bottom = rcIcon.top + nSize;
				rcText.left = rcContent.left;
				rcText.right = rcIcon.left - nGap;
				if( rcText.right < rcText.left ) rcText.right = rcText.left;
			}
			else {
				rcIcon.left = rcContent.left;
				rcIcon.right = rcIcon.left + nSize;
				rcIcon.top = rcContent.top + (ch - nSize) / 2;
				rcIcon.bottom = rcIcon.top + nSize;
				rcText.left = rcIcon.right + nGap;
				rcText.right = rcContent.right;
				if( rcText.left > rcText.right ) rcText.left = rcText.right;
			}
			rcText.top = rcContent.top;
			rcText.bottom = rcContent.bottom;
			return true;
		}

		// DT_LEFT==0：无 CENTER/RIGHT 时左对齐（TreeNode item 设了 text-align=left）
		int x = rcContent.left;
		if( (m_uTextStyle & DT_CENTER) != 0 )
			x = rcContent.left + (cw - blockW) / 2;
		else if( (m_uTextStyle & DT_RIGHT) != 0 )
			x = rcContent.right - blockW;
		if( bRight ) {
			rcText.left = x;
			rcText.right = x + szText.cx;
			rcIcon.left = rcText.right + nGap;
			rcIcon.right = rcIcon.left + nSize;
		}
		else {
			rcIcon.left = x;
			rcIcon.right = x + nSize;
			rcText.left = rcIcon.right + nGap;
			rcText.right = rcText.left + szText.cx;
		}
		rcIcon.top = rcContent.top + (ch - nSize) / 2;
		rcIcon.bottom = rcIcon.top + nSize;
		rcText.top = rcContent.top;
		rcText.bottom = rcContent.bottom;
		return true;
	}

}