#include "StdAfx.h"
#include "UIHotKeyBox.h"
#include "UIHotKey.h"
#include "UIMessageBox.h"

namespace DuiLib {

namespace {

	LPCTSTR GetBuiltinHotKeyBoxSkin()
	{
		return LR"dui(<html theme="chrome">
  <VBox name="root" gap="0">
    <TitleBar name="titlebar" title="快捷键" height="36"
        show-min="false" show-max="false" show-close="true" action="title" />
    <VBox padding="16,16,16,16" gap="8">
      <Label name="prompt" theme="secondary" height="20" />
      <HotKey name="hotkey" height="30" border-width="1" border-radius="4" readonly="true" />
      <Label name="hint" theme="secondary" height="36" word-break="break-word"
          text="字母数字需修饰键；功能键可单独设置。&#10;Backspace / Delete 清除" />
      <Segmented name="scope_seg" height="32"
          options="程序快捷键:app|全局快捷键:global" selected="app" block="true" />
      <HBox height="36" gap="8" align-items="vcenter">
        <Control flexible="true" />
        <Button name="btn_ok" text="确定" kind="primary" width="72" height="30" />
        <Button name="btn_cancel" text="取消" kind="default" width="72" height="30" />
      </HBox>
    </VBox>
  </VBox>
</html>
<style>
  html {
    size: 440,280;
    caption: 0,0,0,36;
    min-size: 360,250;
  }
</style>
)dui";
	}

	CPaintManagerUI* FindPaintManagerByHwnd(HWND hWnd)
	{
		if( hWnd == NULL ) return NULL;
		CStdPtrArray* a = CPaintManagerUI::GetPaintManagers();
		if( a == NULL ) return NULL;
		HWND hWalk = hWnd;
		while( hWalk != NULL ) {
			for( int i = 0; i < a->GetSize(); ++i ) {
				CPaintManagerUI* pm = static_cast<CPaintManagerUI*>(a->GetAt(i));
				if( pm != NULL && pm->GetPaintWindow() == hWalk )
					return pm;
			}
			hWalk = ::GetParent(hWalk);
		}
		return NULL;
	}

} // namespace

/////////////////////////////////////////////////////////////////////////////////////
// CHotKeyBoxOptions

CHotKeyBoxOptions::CHotKeyBoxOptions()
	: m_wVk(0)
	, m_wModifiers(0)
	, m_nScope(HOTKEYBOX_SCOPE_APP)
	, m_bShowScope(true)
	, m_bAllowEmpty(true)
	, m_bRequireModifier(true)
	, m_bCheckConflict(true)
	, m_pConflictManager(NULL)
	, m_pExcludeControl(NULL)
	, m_pfnConflictCheck(NULL)
	, m_pConflictUser(NULL)
	, m_nReserved(0)
	, m_sOkText(_T("确定"))
	, m_sCancelText(_T("取消"))
	, m_nWidth(440)
	, m_nHeight(280)
	, m_hOwner(NULL)
{
	m_sTitle = _T("快捷键");
	m_sPrompt = _T("请按下快捷键组合：");
	ZeroMemory(m_aReservedVk, sizeof(m_aReservedVk));
	ZeroMemory(m_aReservedMod, sizeof(m_aReservedMod));
}

CHotKeyBoxOptions& CHotKeyBoxOptions::Title(LPCTSTR text)
{
	m_sTitle = text ? text : _T("");
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::Prompt(LPCTSTR text)
{
	m_sPrompt = text ? text : _T("");
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::HotKey(WORD wVirtualKeyCode, WORD wModifiers)
{
	m_wVk = wVirtualKeyCode;
	m_wModifiers = wModifiers;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::Scope(int scope)
{
	m_nScope = (scope == HOTKEYBOX_SCOPE_GLOBAL) ? HOTKEYBOX_SCOPE_GLOBAL : HOTKEYBOX_SCOPE_APP;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::ShowScope(bool show)
{
	m_bShowScope = show;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::AllowEmpty(bool allow)
{
	m_bAllowEmpty = allow;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::RequireModifier(bool require)
{
	m_bRequireModifier = require;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::CheckConflict(bool enable)
{
	m_bCheckConflict = enable;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::ConflictManager(CPaintManagerUI* pm)
{
	m_pConflictManager = pm;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::ExcludeControl(CControlUI* pControl)
{
	m_pExcludeControl = pControl;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::AddReserved(WORD vk, WORD mod, LPCTSTR name)
{
	if( m_nReserved >= kMaxReserved ) return *this;
	m_aReservedVk[m_nReserved] = vk;
	m_aReservedMod[m_nReserved] = mod;
	m_aReservedName[m_nReserved] = name ? name : _T("");
	++m_nReserved;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::ConflictCheck(LPHotKeyConflictCheck fn, LPVOID pUser)
{
	m_pfnConflictCheck = fn;
	m_pConflictUser = pUser;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::OkText(LPCTSTR text)
{
	m_sOkText = text ? text : _T("");
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::CancelText(LPCTSTR text)
{
	m_sCancelText = text ? text : _T("");
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::Width(int w)
{
	m_nWidth = w;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::Height(int h)
{
	m_nHeight = h;
	return *this;
}

CHotKeyBoxOptions& CHotKeyBoxOptions::Owner(HWND hOwner)
{
	m_hOwner = hOwner;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////
// CHotKeyBoxWnd

class CHotKeyBoxWnd : public WindowImplBase
{
public:
	CHotKeyBoxWnd(const CHotKeyBoxOptions& opts, WORD* pOutVk, WORD* pOutMod, CDuiString* pOutDisplay, int* pOutScope)
		: m_opts(opts)
		, m_pOutVk(pOutVk)
		, m_pOutMod(pOutMod)
		, m_pOutDisplay(pOutDisplay)
		, m_pOutScope(pOutScope)
		, m_pHotKey(NULL)
		, m_pPrompt(NULL)
		, m_pHint(NULL)
		, m_pScopeSeg(NULL)
		, m_pBtnOk(NULL)
		, m_pBtnCancel(NULL)
	{
	}

	virtual void OnFinalMessage(HWND hWnd)
	{
		WindowImplBase::OnFinalMessage(hWnd);
		delete this;
	}

	virtual CDuiString GetSkinFile()
	{
		return GetBuiltinHotKeyBoxSkin();
	}

	virtual LPCTSTR GetWindowClassName() const
	{
		return _T("DuiHotKeyBoxWnd");
	}

	virtual void InitWindow()
	{
		m_pPrompt = static_cast<CLabelUI*>(m_pm.FindControl(_T("prompt")));
		m_pHotKey = static_cast<CHotKeyUI*>(m_pm.FindControl(_T("hotkey")));
		m_pHint = static_cast<CLabelUI*>(m_pm.FindControl(_T("hint")));
		CControlUI* pSeg = m_pm.FindControl(_T("scope_seg"));
		m_pScopeSeg = pSeg
			? static_cast<CSegmentedUI*>(pSeg->GetInterface(DUI_CTR_SEGMENTED))
			: NULL;
		m_pBtnOk = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_ok")));
		m_pBtnCancel = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_cancel")));

		CControlUI* pTb = m_pm.FindControl(_T("titlebar"));
		CTitleBarUI* pTitleBar = pTb
			? static_cast<CTitleBarUI*>(pTb->GetInterface(DUI_CTR_TITLEBAR))
			: NULL;
		if( pTitleBar != NULL && !m_opts.m_sTitle.IsEmpty() )
			pTitleBar->SetTitle(m_opts.m_sTitle.GetData());

		if( m_pPrompt != NULL ) {
			if( m_opts.m_sPrompt.IsEmpty() ) {
				m_pPrompt->SetVisible(false);
				m_pPrompt->SetFixedHeight(0);
			}
			else {
				m_pPrompt->SetText(m_opts.m_sPrompt.GetData());
			}
		}

		if( m_pHint != NULL ) {
			if( !m_opts.m_bAllowEmpty && !m_opts.m_bRequireModifier ) {
				m_pHint->SetVisible(false);
				m_pHint->SetFixedHeight(0);
			}
			else {
				CDuiString tip;
				if( m_opts.m_bRequireModifier && m_opts.m_bAllowEmpty )
					tip = _T("字母数字需修饰键；功能键可单独设置。\nBackspace / Delete 清除");
				else if( m_opts.m_bRequireModifier )
					tip = _T("字母数字需修饰键；功能键可单独设置。");
				else
					tip = _T("Backspace / Delete 可清除快捷键");
				m_pHint->SetText(tip.GetData());
				m_pHint->SetAttribute(_T("word-break"), _T("break-word"));
				m_pHint->SetFixedHeight(m_opts.m_bAllowEmpty && m_opts.m_bRequireModifier ? 36 : 20);
			}
		}

		if( m_pHotKey != NULL ) {
			m_pHotKey->SetReadOnly(true);
			m_pHotKey->SetHotKey(m_opts.m_wVk, m_opts.m_wModifiers);
		}

		if( m_pScopeSeg != NULL ) {
			if( !m_opts.m_bShowScope ) {
				m_pScopeSeg->SetVisible(false);
				m_pScopeSeg->SetFixedHeight(0);
			}
			else {
				m_pScopeSeg->SetSelectedValue(
					m_opts.m_nScope == HOTKEYBOX_SCOPE_GLOBAL ? _T("global") : _T("app"),
					false);
			}
		}

		if( m_pBtnOk != NULL && !m_opts.m_sOkText.IsEmpty() )
			m_pBtnOk->SetText(m_opts.m_sOkText.GetData());
		if( m_pBtnCancel != NULL && !m_opts.m_sCancelText.IsEmpty() )
			m_pBtnCancel->SetText(m_opts.m_sCancelText.GetData());

		int nH = m_opts.m_nHeight;
		if( nH >= 280 ) {
			if( !m_opts.m_bShowScope ) nH -= 40;
			// 默认高度按双行提示；单行或隐藏时略矮
			if( !m_opts.m_bAllowEmpty && !m_opts.m_bRequireModifier ) nH -= 44;
			else if( !(m_opts.m_bAllowEmpty && m_opts.m_bRequireModifier) ) nH -= 16;
			if( nH < 220 ) nH = 220;
		}

		if( m_opts.m_nWidth > 0 && nH > 0 && m_hWnd != NULL ) {
			RECT rc = { 0 };
			::GetWindowRect(m_hWnd, &rc);
			int w = m_opts.m_nWidth;
			int h = nH;
			if( m_pm.GetDPIObj() != NULL ) {
				w = m_pm.GetDPIObj()->Scale(w);
				h = m_pm.GetDPIObj()->Scale(h);
			}
			::SetWindowPos(m_hWnd, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
	{
		if( (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && m_pHotKey != NULL ) {
			if( wParam == VK_ESCAPE ) {
				Close(HOTKEYBOX_CANCEL);
				bHandled = true;
				return 0;
			}
			if( wParam == VK_RETURN ) {
				CommitOk();
				bHandled = true;
				return 0;
			}
			if( ApplyDialogHotKey((UINT)wParam, lParam) ) {
				bHandled = true;
				return 0;
			}
		}
		return WindowImplBase::MessageHandler(uMsg, wParam, lParam, bHandled);
	}

	DUI_DECLARE_MESSAGE_MAP()

	virtual void OnClick(TNotifyUI& msg)
	{
		CDuiString sName = msg.pSender ? msg.pSender->GetName() : _T("");
		if( sName.CompareNoCase(_T("btn_ok")) == 0 ) {
			CommitOk();
			return;
		}
		if( sName.CompareNoCase(_T("btn_cancel")) == 0
			|| sName.CompareNoCase(_T("closebtn")) == 0 ) {
			Close(HOTKEYBOX_CANCEL);
			return;
		}
		WindowImplBase::OnClick(msg);
	}

private:
	static bool IsModifierKey(UINT vk)
	{
		switch( vk ) {
		case VK_SHIFT: case VK_CONTROL: case VK_MENU:
		case VK_LSHIFT: case VK_RSHIFT:
		case VK_LCONTROL: case VK_RCONTROL:
		case VK_LMENU: case VK_RMENU:
		case VK_LWIN: case VK_RWIN:
			return true;
		default:
			return false;
		}
	}

	bool ApplyDialogHotKey(UINT vk, LPARAM lParam)
	{
		if( m_pHotKey == NULL ) return false;
		if( IsModifierKey(vk) ) return false;
		if( vk == VK_TAB ) return false;

		if( vk == VK_BACK || vk == VK_DELETE ) {
			if( !m_opts.m_bAllowEmpty ) return false;
			m_pHotKey->SetHotKey(0, 0);
			return true;
		}

		WORD mod = 0;
		if( ::GetKeyState(VK_CONTROL) < 0 ) mod |= HOTKEYF_CONTROL;
		if( ::GetKeyState(VK_SHIFT) < 0 ) mod |= HOTKEYF_SHIFT;
		if( ::GetKeyState(VK_MENU) < 0 ) mod |= HOTKEYF_ALT;
		if( ::GetKeyState(VK_LWIN) < 0 || ::GetKeyState(VK_RWIN) < 0 ) mod |= HOTKEYF_WIN;
		if( (lParam & (1L << 24)) != 0 ) mod |= HOTKEYF_EXT;

		// 仅拦截字母/数字裸键；F1~F12、Home、End 等可单独设置
		if( m_opts.m_bRequireModifier && CHotKeyUI::IsBareLetterOrDigit((WORD)vk, mod) )
			return true; // 吞掉按键，不写入

		m_pHotKey->SetHotKey((WORD)vk, mod);
		return true;
	}

	int ReadScope() const
	{
		if( !m_opts.m_bShowScope )
			return m_opts.m_nScope;
		if( m_pScopeSeg != NULL ) {
			LPCTSTR v = m_pScopeSeg->GetSelectedValue();
			if( v != NULL && _tcsicmp(v, _T("global")) == 0 )
				return HOTKEYBOX_SCOPE_GLOBAL;
		}
		return HOTKEYBOX_SCOPE_APP;
	}

	void CommitOk()
	{
		WORD vk = 0, mod = 0;
		if( m_pHotKey != NULL )
			m_pHotKey->GetHotKey(vk, mod);

		const bool bEmpty = (vk == 0 && mod == 0);
		if( bEmpty && !m_opts.m_bAllowEmpty )
			return;

		if( !bEmpty && m_opts.m_bRequireModifier
			&& CHotKeyUI::IsBareLetterOrDigit(vk, mod) ) {
			CMessageBox::ShowInfo(m_hWnd, _T("无效快捷键"),
				_T("字母或数字请配合 Ctrl / Alt / Shift / Win 使用。"));
			Close(HOTKEYBOX_CANCEL);
			return;
		}

		if( !bEmpty && m_opts.m_bCheckConflict ) {
			CDuiString sReason;
			bool bConflict = false;
			if( m_opts.m_pfnConflictCheck != NULL
				&& m_opts.m_pfnConflictCheck(vk, mod, m_opts.m_pConflictUser, &sReason) ) {
				bConflict = true;
				if( sReason.IsEmpty() )
					sReason = _T("该快捷键已被占用，设置无效。");
			}

			if( !bConflict ) {
				for( int i = 0; i < m_opts.m_nReserved; ++i ) {
					if( CHotKeyUI::IsSameHotKey(vk, mod, m_opts.m_aReservedVk[i], m_opts.m_aReservedMod[i]) ) {
						CDuiString key = CHotKeyUI::FormatHotKeyName(vk, mod);
						CDuiString who = m_opts.m_aReservedName[i];
						if( who.IsEmpty() ) who = _T("系统保留");
						sReason.Format(_T("快捷键「%s」与「%s」冲突，设置无效。"),
							key.IsEmpty() ? _T("?") : key.GetData(), who.GetData());
						bConflict = true;
						break;
					}
				}
			}

			if( !bConflict ) {
				CPaintManagerUI* pm = m_opts.m_pConflictManager;
				if( pm == NULL )
					pm = FindPaintManagerByHwnd(m_opts.m_hOwner);
				if( pm != NULL ) {
					CDuiString tip;
					if( CHotKeyUI::FindShortcutConflict(pm, vk, mod, m_opts.m_pExcludeControl, &tip) != NULL ) {
						sReason = tip;
						bConflict = true;
					}
				}
			}

			if( bConflict ) {
				CMessageBox::ShowInfo(m_hWnd, _T("快捷键冲突"), sReason.GetData());
				Close(HOTKEYBOX_CANCEL); // 可关闭，但不写出 / 调用方不生效
				return;
			}
		}

		const int nScope = ReadScope();

		if( m_pOutVk != NULL ) *m_pOutVk = vk;
		if( m_pOutMod != NULL ) *m_pOutMod = mod;
		if( m_pOutScope != NULL ) *m_pOutScope = nScope;
		if( m_pOutDisplay != NULL ) {
			if( bEmpty )
				m_pOutDisplay->Empty();
			else
				*m_pOutDisplay = CHotKeyUI::FormatHotKeyName(vk, mod);
		}
		Close(HOTKEYBOX_OK);
	}

	CHotKeyBoxOptions m_opts;
	WORD* m_pOutVk;
	WORD* m_pOutMod;
	CDuiString* m_pOutDisplay;
	int* m_pOutScope;
	CHotKeyUI* m_pHotKey;
	CLabelUI* m_pPrompt;
	CLabelUI* m_pHint;
	CSegmentedUI* m_pScopeSeg;
	CButtonUI* m_pBtnOk;
	CButtonUI* m_pBtnCancel;
};

DUI_BEGIN_MESSAGE_MAP(CHotKeyBoxWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CHotKeyBoxWnd::OnClick)
DUI_END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////
// CHotKeyBox

int CHotKeyBox::Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt,
	WORD& outVk, WORD& outModifiers, CDuiString* pOutDisplay, int* pOutScope)
{
	return Show(hOwner,
		CHotKeyBoxOptions()
			.Title(title)
			.Prompt(prompt),
		outVk, outModifiers, pOutDisplay, pOutScope);
}

int CHotKeyBox::Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt,
	WORD wVk, WORD wModifiers,
	WORD& outVk, WORD& outModifiers, CDuiString* pOutDisplay, int* pOutScope)
{
	return Show(hOwner,
		CHotKeyBoxOptions()
			.Title(title)
			.Prompt(prompt)
			.HotKey(wVk, wModifiers),
		outVk, outModifiers, pOutDisplay, pOutScope);
}

int CHotKeyBox::Show(HWND hOwner, const CHotKeyBoxOptions& opts,
	WORD& outVk, WORD& outModifiers, CDuiString* pOutDisplay, int* pOutScope)
{
	CHotKeyBoxOptions o = opts;
	if( hOwner != NULL )
		o.Owner(hOwner);

	CHotKeyBoxWnd* pWnd = new CHotKeyBoxWnd(o, &outVk, &outModifiers, pOutDisplay, pOutScope);
	HWND hCreateOwner = o.m_hOwner;
	pWnd->Create(hCreateOwner, o.m_sTitle.IsEmpty() ? _T("快捷键") : o.m_sTitle.GetData(),
		WS_POPUP | WS_CLIPCHILDREN, WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME,
		0, 0, o.m_nWidth, o.m_nHeight);
	if( pWnd->GetHWND() == NULL ) {
		delete pWnd;
		return HOTKEYBOX_CANCEL;
	}
	pWnd->CenterWindow();
	return (int)pWnd->ShowModal();
}

} // namespace DuiLib
