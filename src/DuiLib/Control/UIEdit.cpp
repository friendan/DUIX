#include "StdAfx.h"
#include "UIEdit.h"
#include "UIEditBox.h"
#include <Imm.h>
#include <vector>
#pragma comment(lib, "Imm32.lib")

namespace DuiLib
{
	enum { EDIT_CARET_BLINK_TIMERID = 0xFFF2 };

	static UINT GetEditCaretBlinkInterval()
	{
		UINT u = ::GetCaretBlinkTime();
		return (u == 0) ? 530u : u;
	}

	struct EditTimerCtx
	{
		CEditUI* pSelf;
		UINT idTimer;
	};

	VOID CALLBACK EditQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		EditTimerCtx* pCtx = static_cast<EditTimerCtx*>(lpParameter);
		if( pCtx == NULL || pCtx->pSelf == NULL ) return;
		CPaintManagerUI* pm = pCtx->pSelf->GetManager();
		if( pm == NULL ) return;
		HWND hWnd = pm->GetPaintWindow();
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, UIMSG_EDIT_TICK, (WPARAM)pCtx->pSelf, pCtx->idTimer);
	}

	struct CEditUI::EditQueueTimers
	{
		struct Entry
		{
			UINT idTimer;
			HANDLE hTimer;
			EditTimerCtx* pCtx;
		};
		std::vector<Entry> items;

		Entry* Find(UINT idTimer)
		{
			for( size_t i = 0; i < items.size(); ++i ) {
				if( items[i].idTimer == idTimer )
					return &items[i];
			}
			return NULL;
		}

		void StopEntry(Entry& e)
		{
			if( e.hTimer != NULL ) {
				::DeleteTimerQueueTimer(NULL, e.hTimer, INVALID_HANDLE_VALUE);
				e.hTimer = NULL;
			}
			if( e.pCtx != NULL ) {
				delete e.pCtx;
				e.pCtx = NULL;
			}
		}

		void Stop(UINT idTimer)
		{
			for( size_t i = 0; i < items.size(); ) {
				if( items[i].idTimer == idTimer ) {
					StopEntry(items[i]);
					items.erase(items.begin() + i);
				}
				else {
					++i;
				}
			}
		}

		void StopAll()
		{
			for( size_t i = 0; i < items.size(); ++i )
				StopEntry(items[i]);
			items.clear();
		}
	};

	class CEditWnd : public CWindowWnd
	{
	public:
		CEditWnd();

		void Init(CEditUI* pOwner);
		RECT CalPos();
		/// 控件析构前同步拆掉原生窗，避免异步 WM_CLOSE 访问已释放的 owner
		void CloseAndDetach();
		void OnCaretBlinkTick();
		void ApplyReadOnlyCaretPolicy();
		void RestartSoftCaretBlink();

		LPCTSTR GetWindowClassName() const;
		LPCTSTR GetSuperClassName() const;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	protected:
		enum { 
			DEFAULT_TIMERID = 20,
		};

		int GetCaretLineHeight() const;
		int GetCaretCharIndex() const;
		bool GetCaretPoint(POINT& pt) const;
		bool GetCaretRect(RECT& rc) const;
		void SuppressSystemCaret();
		void InvalidateCaretRect();
		void DrawSoftCaret(HDC hdc) const;
		void RefreshSoftCaret();

		CEditUI* m_pOwner;
		HBRUSH m_hBkBrush;
		DWORD m_dwBrushColor;
		bool m_bInit;
		bool m_bDrawCaret;
		RECT m_rcLastSoftCaret;
	};


	CEditWnd::CEditWnd() : m_pOwner(NULL), m_hBkBrush(NULL), m_dwBrushColor(0), m_bInit(false), m_bDrawCaret(false)
	{
		::SetRectEmpty(&m_rcLastSoftCaret);
	}

	int CEditWnd::GetCaretLineHeight() const
	{
		if( m_pOwner == NULL || m_pOwner->GetManager() == NULL ) return 16;
		HFONT hFont = m_pOwner->GetManager()->GetFont(m_pOwner->GetFont());
		if( hFont == NULL )
			hFont = m_pOwner->GetManager()->GetDefaultFontInfo()->hFont;
		if( hFont == NULL ) return 16;
		HDC hdc = ::GetDC(m_hWnd);
		if( hdc == NULL ) return 16;
		HFONT hOld = (HFONT)::SelectObject(hdc, hFont);
		TEXTMETRIC tm = { 0 };
		::GetTextMetrics(hdc, &tm);
		::SelectObject(hdc, hOld);
		::ReleaseDC(m_hWnd, hdc);
		return tm.tmHeight > 0 ? tm.tmHeight : 16;
	}

	int CEditWnd::GetCaretCharIndex() const
	{
		if( !::IsWindow(m_hWnd) ) return 0;
		DWORD dwStart = 0;
		DWORD dwEnd = 0;
		// 直接交给 Edit 默认过程，避免 SendMessage 再进 HandleMessage 拿到旧 sel
		::CallWindowProc(m_OldWndProc, m_hWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		return (int)((dwEnd >= dwStart) ? dwEnd : dwStart);
	}

	bool CEditWnd::GetCaretPoint(POINT& pt) const
	{
		pt.x = 0;
		pt.y = 0;
		if( !::IsWindow(m_hWnd) || m_pOwner == NULL ) return false;

		const int iChar = GetCaretCharIndex();
		const int cchLen = ::GetWindowTextLength(m_hWnd);

		HDC hdc = ::GetDC(m_hWnd);
		if( hdc == NULL ) return false;
		HFONT hFont = NULL;
		if( m_pOwner->GetManager() != NULL ) {
			hFont = m_pOwner->GetManager()->GetFont(m_pOwner->GetFont());
			if( hFont == NULL )
				hFont = m_pOwner->GetManager()->GetDefaultFontInfo()->hFont;
		}
		HFONT hOld = hFont != NULL ? (HFONT)::SelectObject(hdc, hFont) : NULL;

		auto MeasureChars = [&](LPCTSTR psz, int nChars) -> int {
			if( nChars <= 0 || psz == NULL ) return 0;
			SIZE sz = { 0 };
			if( !::GetTextExtentPoint32(hdc, psz, nChars, &sz) ) return 0;
			return sz.cx;
		};

		int nPrefixCx = 0;
		int nFullCx = 0;
		if( cchLen > 0 ) {
			LPTSTR pszText = static_cast<LPTSTR>(_alloca((cchLen + 1) * sizeof(TCHAR)));
			if( pszText == NULL ) {
				if( hOld != NULL ) ::SelectObject(hdc, hOld);
				::ReleaseDC(m_hWnd, hdc);
				return false;
			}
			::GetWindowText(m_hWnd, pszText, cchLen + 1);
			if( m_pOwner->IsPasswordMode() ) {
				const TCHAR chPwd = m_pOwner->GetPasswordChar();
				TCHAR szPrefix[512] = { 0 };
				const int nPrefix = (iChar < (int)_countof(szPrefix)) ? iChar : (int)_countof(szPrefix) - 1;
				for( int i = 0; i < nPrefix; ++i ) szPrefix[i] = chPwd;
				nPrefixCx = MeasureChars(szPrefix, nPrefix);
				TCHAR szFull[512] = { 0 };
				const int nFull = (cchLen < (int)_countof(szFull)) ? cchLen : (int)_countof(szFull) - 1;
				for( int i = 0; i < nFull; ++i ) szFull[i] = chPwd;
				nFullCx = MeasureChars(szFull, nFull);
			}
			else {
				LPCTSTR pEnd = pszText;
				for( int i = 0; i < iChar && *pEnd != _T('\0'); ++i )
					pEnd = ::CharNext(pEnd);
				const int nPrefixLen = (int)(pEnd - pszText);
				nPrefixCx = MeasureChars(pszText, nPrefixLen);
				nFullCx = MeasureChars(pszText, cchLen);
			}
		}

		if( hOld != NULL ) ::SelectObject(hdc, hOld);
		::ReleaseDC(m_hWnd, hdc);

		DWORD dwMargins = (DWORD)::SendMessage(m_hWnd, EM_GETMARGINS, 0, 0);
		const int nLeftMg = (int)LOWORD(dwMargins);
		const int nRightMg = (int)HIWORD(dwMargins);
		RECT rcClient = { 0 };
		::GetClientRect(m_hWnd, &rcClient);
		const int nClientW = rcClient.right - rcClient.left;
		const LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
		int x = nLeftMg + nPrefixCx;
		if( style & ES_CENTER ) {
			x = nLeftMg + (nClientW - nLeftMg - nRightMg - nFullCx) / 2 + nPrefixCx;
		}
		else if( style & ES_RIGHT ) {
			x = nClientW - nRightMg - nFullCx + nPrefixCx;
		}
		else {
			const int nScroll = ::GetScrollPos(m_hWnd, SB_HORZ);
			if( nScroll > 0 ) x -= nScroll;
		}
		if( x < 0 ) x = 0;
		pt.x = x;
		pt.y = 0;
		return true;
	}

	bool CEditWnd::GetCaretRect(RECT& rc) const
	{
		POINT pt = { 0 };
		if( !GetCaretPoint(pt) ) return false;
		const int nH = GetCaretLineHeight();
		rc.left = pt.x;
		rc.top = pt.y;
		rc.right = pt.x + 1;
		rc.bottom = pt.y + nH;
		return true;
	}

	void CEditWnd::SuppressSystemCaret()
	{
		// 仅 HideCaret；DestroyCaret 会把 Edit 插入点重置到开头
		::HideCaret(m_hWnd);
	}

	void CEditWnd::InvalidateCaretRect()
	{
		if( !::IsWindow(m_hWnd) ) return;
		RECT rc = { 0 };
		if( !GetCaretRect(rc) ) return;
		::InflateRect(&rc, 0, 1);
		::InvalidateRect(m_hWnd, &rc, FALSE);
	}

	void CEditWnd::DrawSoftCaret(HDC hdc) const
	{
		if( !m_bDrawCaret || m_pOwner == NULL ) return;
		RECT rc = { 0 };
		if( !GetCaretRect(rc) ) return;
		DWORD dwColor = m_pOwner->GetNativeEditColor();
		if( dwColor == 0 && m_pOwner->GetManager() != NULL )
			dwColor = m_pOwner->GetManager()->GetDefaultFontColor();
		dwColor = m_pOwner->GetAdjustColor(dwColor);
		HBRUSH hBrush = ::CreateSolidBrush(DuiColorToCOLORREF(dwColor));
		if( hBrush == NULL ) return;
		::FillRect(hdc, &rc, hBrush);
		::DeleteObject(hBrush);
	}

	void CEditWnd::RefreshSoftCaret()
	{
		if( !m_bInit || !::IsWindow(m_hWnd) || ::GetFocus() != m_hWnd ) return;
		if( m_pOwner != NULL && m_pOwner->IsReadOnly() ) return;
		RECT rcNew = { 0 };
		if( !GetCaretRect(rcNew) ) return;
		::InflateRect(&rcNew, 0, 1);
		RECT rcPaint = rcNew;
		if( !::IsRectEmpty(&m_rcLastSoftCaret) )
			::UnionRect(&rcPaint, &rcPaint, &m_rcLastSoftCaret);
		SuppressSystemCaret();
		::RedrawWindow(m_hWnd, &rcPaint, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		m_rcLastSoftCaret = rcNew;
	}

	void CEditWnd::OnCaretBlinkTick()
	{
		if( !m_bInit || !::IsWindow(m_hWnd) || ::GetFocus() != m_hWnd ) return;
		if( m_pOwner != NULL && m_pOwner->IsReadOnly() ) return;
		m_bDrawCaret = !m_bDrawCaret;
		RefreshSoftCaret();
	}

	void CEditWnd::ApplyReadOnlyCaretPolicy()
	{
		if( m_pOwner == NULL || !m_pOwner->IsReadOnly() ) return;
		m_pOwner->StopAllQueueTimers();
		m_bDrawCaret = false;
		::SetRectEmpty(&m_rcLastSoftCaret);
		SuppressSystemCaret();
		if( ::IsWindow(m_hWnd) )
			::InvalidateRect(m_hWnd, NULL, FALSE);
	}

	void CEditWnd::RestartSoftCaretBlink()
	{
		if( !m_bInit || m_pOwner == NULL || m_pOwner->IsReadOnly() ) return;
		if( ::GetFocus() != m_hWnd ) return;
		m_bDrawCaret = true;
		m_pOwner->StartCaretBlinkTimer();
		RefreshSoftCaret();
	}

	void CEditWnd::Init(CEditUI* pOwner)
	{
		m_pOwner = pOwner;
		RECT rcPos = CalPos();
		UINT uStyle = 0;
		if(m_pOwner->GetManager()->IsLayered()) {
			uStyle = WS_POPUP | ES_AUTOHSCROLL | WS_VISIBLE;
			RECT rcWnd={0};
			::GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWnd);
			rcPos.left += rcWnd.left;
			rcPos.right += rcWnd.left;
			rcPos.top += rcWnd.top - 1;
			rcPos.bottom += rcWnd.top - 1;
		}
		else {
			uStyle = WS_CHILD | ES_AUTOHSCROLL;
		}
		UINT uTextStyle = m_pOwner->GetTextStyle();
		if(uTextStyle & DT_LEFT) uStyle |= ES_LEFT;
		else if(uTextStyle & DT_CENTER) uStyle |= ES_CENTER;
		else if(uTextStyle & DT_RIGHT) uStyle |= ES_RIGHT;
		if( m_pOwner->IsPasswordMode() ) uStyle |= ES_PASSWORD;
		Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
		// 关掉视觉样式，避免主题重绘盖住系统插入符（同 IPAddress）
		{
			typedef HRESULT (WINAPI *PFNSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
			HMODULE hUx = ::GetModuleHandle(_T("uxtheme.dll"));
			if( hUx == NULL ) hUx = ::LoadLibrary(_T("uxtheme.dll"));
			if( hUx != NULL ) {
				PFNSetWindowTheme pfn = (PFNSetWindowTheme)::GetProcAddress(hUx, "SetWindowTheme");
				if( pfn != NULL ) pfn(m_hWnd, L"", L"");
			}
		}
		HFONT hFont=NULL;
		int iFontIndex=m_pOwner->GetFont();
		if (iFontIndex!=-1)
			hFont = m_pOwner->GetManager()->GetFont(iFontIndex);
		if (hFont == NULL)
			hFont = m_pOwner->GetManager()->GetDefaultFontInfo()->hFont;

		SetWindowFont(m_hWnd, hFont, TRUE);
		Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
		if( m_pOwner->IsPasswordMode() ) Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());
		Edit_SetText(m_hWnd, m_pOwner->GetText().GetData());
		Edit_SetModify(m_hWnd, FALSE);
		SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
		Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
		Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);

		//Styls
		LONG styleValue = ::GetWindowLong(m_hWnd, GWL_STYLE);
		styleValue |= pOwner->GetWindowStyls();
		::SetWindowLong(GetHWND(), GWL_STYLE, styleValue);
		//Styls
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		::SetFocus(m_hWnd);
		if (m_pOwner->IsAutoSelAll()) {
			int nSize = GetWindowTextLength(m_hWnd);
			if( nSize == 0 ) nSize = 1;
			Edit_SetSel(m_hWnd, 0, nSize);
		}
		else {
			int nSize = GetWindowTextLength(m_hWnd);
			Edit_SetSel(m_hWnd, nSize, nSize);
		}

		m_bInit = true;
		SuppressSystemCaret();
		if( !m_pOwner->IsReadOnly() ) {
			m_bDrawCaret = true;
			m_pOwner->StartCaretBlinkTimer();
			RefreshSoftCaret();
		}
		else {
			m_bDrawCaret = false;
		}
	}

	RECT CEditWnd::CalPos()
	{
		CDuiRect rcPos = m_pOwner->GetPos();
		RECT rcPad = m_pOwner->GetPadding();
		RECT rcTextPad = m_pOwner->GetTextPadding();
		rcPos.left += rcPad.left + rcTextPad.left;
		rcPos.top += rcPad.top + rcTextPad.top;
		rcPos.right -= rcPad.right + rcTextPad.right;
		rcPos.bottom -= rcPad.bottom + rcTextPad.bottom;
		int nReserve = m_pOwner->GetNativeEditRightReserve();
		if( nReserve > 0 && m_pOwner->GetManager() )
			nReserve = m_pOwner->GetManager()->GetDPIObj()->Scale(nReserve);
		if( nReserve > 0 ) rcPos.right -= nReserve;
		if( rcPos.right < rcPos.left + 4 ) rcPos.right = rcPos.left + 4;
		LONG lEditHeight = m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->tm.tmHeight;
		if( lEditHeight < rcPos.GetHeight() ) {
			rcPos.top += (rcPos.GetHeight() - lEditHeight) / 2;
			rcPos.bottom = rcPos.top + lEditHeight;
		}

		CControlUI* pParent = m_pOwner;
		RECT rcParent;
		while( (pParent = pParent->GetParent()) ) {
			if( !pParent->IsVisible() ) {
				rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
				break;
			}
			rcParent = pParent->GetClientPos();
			if( !::IntersectRect(&rcPos, &rcPos, &rcParent) ) {
				rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
				break;
			}
		}

		return rcPos;
	}

	LPCTSTR CEditWnd::GetWindowClassName() const
	{
		return _T("EditWnd");
	}

	LPCTSTR CEditWnd::GetSuperClassName() const
	{
		return WC_EDIT;
	}

	void CEditWnd::CloseAndDetach()
	{
		m_bInit = false;
		if( m_pOwner != NULL && m_pOwner->m_pWindow == this )
			m_pOwner->m_pWindow = NULL;
		m_pOwner = NULL;
		if( ::IsWindow(m_hWnd) )
			::DestroyWindow(m_hWnd); // 同步 OnFinalMessage → delete this
		else
			delete this;
	}

	void CEditWnd::OnFinalMessage(HWND hWnd)
	{
		if( m_hBkBrush != NULL ) {
			::DeleteObject(m_hBkBrush);
			m_hBkBrush = NULL;
		}
		if( m_pOwner != NULL ) {
			m_pOwner->Invalidate();
			if( m_pOwner->GetManager() != NULL && m_pOwner->GetManager()->IsLayered() )
				m_pOwner->GetManager()->RemoveNativeWindow(hWnd);
			// 可能已在 OnKillFocus / CloseAndDetach 里提前摘掉
			if( m_pOwner->m_pWindow == this )
				m_pOwner->m_pWindow = NULL;
		}
		delete this;
	}

	LRESULT CEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if( uMsg == WM_CREATE ) {
			bHandled = FALSE;
		}
		else if( uMsg == WM_SETFOCUS ) {
			CPaintManagerUI* pm = (m_pOwner != NULL) ? m_pOwner->GetManager() : NULL;
			if( m_pOwner != NULL && pm != NULL && pm->GetFocus() != m_pOwner )
				pm->SetFocus(m_pOwner);
			SuppressSystemCaret();
			if( m_bInit && m_pOwner != NULL ) {
				if( m_pOwner->IsReadOnly() )
					ApplyReadOnlyCaretPolicy();
				else
					RestartSoftCaretBlink();
			}
			bHandled = FALSE;
		}
		else if( uMsg == WM_KILLFOCUS ) {
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		}
		else if( uMsg == OCM_COMMAND ) {
			if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
			else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
				RECT rcClient;
				::GetClientRect(m_hWnd, &rcClient);
				::InvalidateRect(m_hWnd, &rcClient, FALSE);
				if( m_bInit && ::GetFocus() == m_hWnd )
					RefreshSoftCaret();
			}
		}
		else if( uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_RETURN ){
			// 回车前先把原生文本写回 m_sText（IME 确认等场景可能尚未 EN_CHANGE）
			if( m_bInit && m_pOwner != NULL ) {
				int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
				LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
				if( pstr != NULL ) {
					::GetWindowText(m_hWnd, pstr, cchLen);
					m_pOwner->m_sText = pstr;
					m_pOwner->OnNativeEditChanged();
				}
			}
			// 异步通知：避免在 Edit WM_KEYDOWN 里同步 Navigate/SetText 重入
			m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_RETURN, 0, 0, true);
		}
		else if( uMsg == WM_KEYDOWN && wParam == VK_TAB ){
			// 分层 Edit 为 WS_POPUP，进不了 PreMessageHandler，这里负责切焦点。
			// SetNextTabControl 内会同步 WM_KILLFOCUS → m_pOwner=NULL，之后勿再解引用。
			if( m_pOwner != NULL && m_pOwner->GetManager() != NULL )
				m_pOwner->GetManager()->SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
		}
		else if( uMsg == WM_CHAR
			|| (uMsg == WM_KEYDOWN && wParam != VK_RETURN && wParam != VK_TAB) ) {
			lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
			if( m_bInit && ::GetFocus() == m_hWnd )
				RefreshSoftCaret();
			return lRes;
		}
		else if( uMsg == WM_IME_STARTCOMPOSITION || uMsg == WM_IME_COMPOSITION ) {
			// 把候选/拼写窗钉在光标旁。EditBox 内嵌 Edit 在外壳 Invalidate 后，
			// 系统默认位置常飘到错误处或看不见；与 RichEdit 同样显式设置。
			HIMC hImc = ::ImmGetContext(m_hWnd);
			if( hImc != NULL ) {
				POINT pt = { 0 };
				GetCaretPoint(pt);
				COMPOSITIONFORM cf = {};
				cf.dwStyle = CFS_POINT;
				cf.ptCurrentPos = pt;
				::ImmSetCompositionWindow(hImc, &cf);
				CANDIDATEFORM cand = {};
				cand.dwIndex = 0;
				cand.dwStyle = CFS_CANDIDATEPOS;
				cand.ptCurrentPos = pt;
				::ImmSetCandidateWindow(hImc, &cand);
				if( m_pOwner != NULL && m_pOwner->GetManager() != NULL ) {
					HFONT hFont = m_pOwner->GetManager()->GetFont(m_pOwner->GetFont());
					if( hFont == NULL )
						hFont = m_pOwner->GetManager()->GetDefaultFontInfo()->hFont;
					if( hFont != NULL ) {
						LOGFONT lf = {};
						if( ::GetObject(hFont, sizeof(lf), &lf) != 0 )
							::ImmSetCompositionFont(hImc, &lf);
					}
				}
				::ImmReleaseContext(m_hWnd, hImc);
			}
			bHandled = FALSE; // 仍交 Edit 默认处理组字
		}
		else if( uMsg == OCM__BASE + WM_CTLCOLOREDIT  || uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) {
			::SetBkMode((HDC)wParam, OPAQUE);
			DWORD dwColor = m_pOwner->GetAdjustColor(m_pOwner->GetNativeEditColor());
			::SetTextColor((HDC)wParam, DuiColorToCOLORREF(dwColor));
			DWORD clrColor = m_pOwner->GetNativeEditBackgroundColor();
			if( !DuiColorIsOpaque(clrColor) ) {
				if (m_hBkBrush != NULL) {
					::DeleteObject(m_hBkBrush);
					m_hBkBrush = NULL;
				}
				m_dwBrushColor = 0;
				RECT rcWnd = m_pOwner->GetManager()->GetNativeWindowRect(m_hWnd);
				HBITMAP hBmpEditBk = CRenderEngine::GenerateBitmap(m_pOwner->GetManager(), rcWnd, m_pOwner, clrColor);
				m_hBkBrush = ::CreatePatternBrush(hBmpEditBk);
				::DeleteObject(hBmpEditBk);
				::SetBkColor((HDC)wParam, DuiColorToCOLORREF(m_pOwner->GetAdjustColor(clrColor)));
			}
			else {
				DWORD adj = m_pOwner->GetAdjustColor(clrColor);
				::SetBkColor((HDC)wParam, DuiColorToCOLORREF(adj));
				if( m_hBkBrush == NULL || m_dwBrushColor != adj ) {
					if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
					m_hBkBrush = ::CreateSolidBrush(DuiColorToCOLORREF(adj));
					m_dwBrushColor = adj;
				}
			}
			return (LRESULT)m_hBkBrush;
		}
		else if( uMsg == WM_PAINT) {
			lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
			SuppressSystemCaret();
			if( m_bInit && m_bDrawCaret && ::GetFocus() == m_hWnd
				&& m_pOwner != NULL && !m_pOwner->IsReadOnly() ) {
				HDC hdc = ::GetDC(m_hWnd);
				if( hdc != NULL ) {
					DrawSoftCaret(hdc);
					::ReleaseDC(m_hWnd, hdc);
				}
			}
			return lRes;
		}
		else if( uMsg == WM_PRINT ) {
			bHandled = FALSE;
		}
		else if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) {
			lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
			if( m_pOwner != NULL && m_pOwner->GetManager() != NULL
				&& m_pOwner->GetManager()->GetFocus() != m_pOwner ) {
				m_pOwner->GetManager()->SetFocus(m_pOwner);
			}
			// 已有焦点时再点输入框不会走 Dui SETFOCUS/BUTTONDOWN，在此通知 EditBox 弹历史
			if( m_pOwner != NULL ) {
				for( CControlUI* p = m_pOwner; p != NULL; p = p->GetParent() ) {
					CEditBoxUI* pBox = static_cast<CEditBoxUI*>(p->GetInterface(DUI_CTR_EDITBOX));
					if( pBox != NULL ) {
						pBox->OnInnerEditNativeClick();
						break;
					}
				}
			}
			if( m_bInit && ::GetFocus() == m_hWnd )
				RefreshSoftCaret();
			return lRes;
		}
		else bHandled = FALSE;

		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CEditWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( m_pOwner != NULL )
			m_pOwner->StopAllQueueTimers();
		m_bDrawCaret = false;
		::SetRectEmpty(&m_rcLastSoftCaret);
		// 销毁前先回写文本；随后忽略 EN_CHANGE。
		// 文本相对上次 EN_CHANGE 未变时不要再发 TEXTCHANGED：否则点列表项时
		// KillFocus 会同步重建列表并删掉正在点击的控件 → 崩溃。
		if( m_bInit && m_pOwner != NULL ) {
			int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
			LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
			if( pstr != NULL ) {
				::GetWindowText(m_hWnd, pstr, cchLen);
				if( m_pOwner->m_sText != pstr ) {
					m_pOwner->m_sText = pstr;
					m_pOwner->OnNativeEditChanged();
				}
			}
		}
		m_bInit = false;

		// 立刻隐藏并摘掉指针，让失焦自绘马上能画（不要等异步 WM_CLOSE）
		::ShowWindow(m_hWnd, SW_HIDE);
		if( m_pOwner != NULL ) {
			if( m_pOwner->m_pWindow == this )
				m_pOwner->m_pWindow = NULL;
			m_pOwner->Invalidate();
			// 异步 WM_CLOSE 前断开 owner：列表可能已删掉 CEditUI
			m_pOwner = NULL;
		}

		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		PostMessage(WM_CLOSE);
		return lRes;
	}

	LRESULT CEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if( !m_bInit ) return 0;
		if( m_pOwner == NULL ) return 0;
		// Copy text back
		int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
		LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
		ASSERT(pstr);
		if( pstr == NULL ) return 0;
		::GetWindowText(m_hWnd, pstr, cchLen);
		m_pOwner->m_sText = pstr;
		m_pOwner->OnNativeEditChanged();
		if( m_pOwner->GetManager()->IsLayered() ) m_pOwner->Invalidate();
		return 0;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CEditUI)

		CEditUI::CEditUI() : m_pWindow(NULL), m_uMaxChar(255), m_bReadOnly(false), 
		m_bPasswordMode(false), m_bAutoSelAll(false), m_cPasswordChar(_T('*')), m_uButtonState(0), 
		m_dwPlaceholderColor(0xBAC0C5FF), m_dwEditbkColor(0), m_dwEditTextColor(0),
		m_bNativeBkColorCustom(false), m_bNativeTextColorCustom(false), m_iWindowStyls(0),
		m_pQueueTimers(NULL)
	{
		SetPadding(CDuiBox(4, 10, 4, 10)); // 默认左右内边距，圆角时文字不贴边
		SetBackgroundColor(0xFFFFFFFF);
	}

	CEditUI::~CEditUI()
	{
		StopAllQueueTimers();
		// DestroyWindow 前确保 manager 不再认为本控件有焦点，避免 paint WM_SETFOCUS 回打重建。
		if( m_pManager != NULL && m_pManager->GetFocus() == this )
			m_pManager->ReapObjects(this);
		if( m_pWindow != NULL ) {
			CEditWnd* pWnd = m_pWindow;
			m_pWindow = NULL;
			pWnd->CloseAndDetach();
		}
	}

	LPCTSTR CEditUI::GetClass() const
	{
		return _T("EditUI");
	}

	LPVOID CEditUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_EDIT) == 0 ) return static_cast<CEditUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	bool CEditUI::CanHostNativeEdit() const
	{
		return GetEffectiveOpacity() >= 255;
	}

	void CEditUI::DismissNativeEdit()
	{
		if( m_pWindow == NULL ) return;
		CEditWnd* pWnd = m_pWindow;
		// 回写文本（与 KillFocus 一致）
		HWND hWnd = pWnd->GetHWND();
		if( hWnd != NULL && ::IsWindow(hWnd) ) {
			int cchLen = ::GetWindowTextLength(hWnd) + 1;
			LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
			if( pstr != NULL ) {
				::GetWindowText(hWnd, pstr, cchLen);
				if( m_sText != pstr ) {
					m_sText = pstr;
					OnNativeEditChanged();
				}
			}
		}
		m_pWindow = NULL;
		pWnd->CloseAndDetach();
		Invalidate();
	}

	void CEditUI::SetOpacity(BYTE nOpacity)
	{
		CControlUI::SetOpacity(nOpacity);
		if( !CanHostNativeEdit() )
			DismissNativeEdit();
	}

	UINT CEditUI::GetControlFlags() const
	{
		if( !IsEnabled() ) return CControlUI::GetControlFlags();

		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CEditUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
			return;
		}
		if( event.Type == UIEVENT_WINDOWSIZE )
		{
			if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if( m_pWindow != NULL ) return;
		}
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
		{
			if( m_pWindow ) return;
			// 已摘树 / 非当前焦点时不创建原生窗（删除获焦 Edit 时的重入保护）。
			if( m_pParent == NULL || m_pManager == NULL || m_pManager->GetFocus() != this ) {
				Invalidate();
				return;
			}
			if( !CanHostNativeEdit() ) {
				Invalidate();
				return;
			}
			m_pWindow = new CEditWnd();
			ASSERT(m_pWindow);
			m_pWindow->Init(this);
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
		{
			if( IsEnabled() ) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL )
				{
					if( CanHostNativeEdit() ) {
						m_pWindow = new CEditWnd();
						ASSERT(m_pWindow);
						m_pWindow->Init(this);

						if( PtInRect(&m_rcItem, event.ptMouse) )
						{
							int nSize = GetWindowTextLength(*m_pWindow);
							if( nSize == 0 ) nSize = 1;
							Edit_SetSel(*m_pWindow, 0, nSize);
						}
					}
					else {
						Invalidate();
					}
				}
				else if( m_pWindow != NULL )
				{
					if (!m_bAutoSelAll) {
						RECT rcPad = GetPadding();
						RECT rcTextPadding = GetTextPadding();
						POINT pt = event.ptMouse;
						pt.x -= m_rcItem.left + rcPad.left + rcTextPadding.left;
						pt.y -= m_rcItem.top + rcPad.top + rcTextPadding.top;
						Edit_SetSel(*m_pWindow, 0, 0);
						::SendMessage(*m_pWindow, WM_LBUTTONDOWN, event.wParam, MAKELPARAM(pt.x, pt.y));
					}
				}
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse ) ) {
				if( IsEnabled() ) {
					if( (m_uButtonState & UISTATE_HOT) == 0  ) {
						m_uButtonState |= UISTATE_HOT;
						Invalidate();
					}
				}
			}
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CLabelUI::DoEvent(event);
	}

	void CEditUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	void CEditUI::SetText(LPCTSTR pstrText)
	{
		if( pstrText == NULL ) pstrText = _T("");
		// 相同内容不要 Edit_SetText：Windows 会把光标打回开头
		if( m_sText == pstrText ) {
			if( m_pWindow != NULL ) {
				int cch = ::GetWindowTextLength(*m_pWindow) + 1;
				LPTSTR pstr = static_cast<LPTSTR>(_alloca(cch * sizeof(TCHAR)));
				if( pstr != NULL ) {
					::GetWindowText(*m_pWindow, pstr, cch);
					if( m_sText == pstr ) return;
				}
			}
			else return;
		}
		m_sText = pstrText;
		if( m_pWindow != NULL ) {
			Edit_SetText(*m_pWindow, m_sText.GetData());
			int nLen = ::GetWindowTextLength(*m_pWindow);
			Edit_SetSel(*m_pWindow, nLen, nLen);
		}
		Invalidate();
	}

	void CEditUI::SetMaxChar(UINT uMax)
	{
		m_uMaxChar = uMax;
		if( m_pWindow != NULL ) Edit_LimitText(*m_pWindow, m_uMaxChar);
	}

	UINT CEditUI::GetMaxChar()
	{
		return m_uMaxChar;
	}

	void CEditUI::SetReadOnly(bool bReadOnly)
	{
		if( m_bReadOnly == bReadOnly ) return;

		m_bReadOnly = bReadOnly;
		if( m_pWindow != NULL ) {
			Edit_SetReadOnly(*m_pWindow, m_bReadOnly);
			if( m_bReadOnly )
				m_pWindow->ApplyReadOnlyCaretPolicy();
			else if( ::GetFocus() == m_pWindow->GetHWND() )
				m_pWindow->RestartSoftCaretBlink();
		}
		Invalidate();
	}

	bool CEditUI::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	void CEditUI::SetNumberOnly(bool bNumberOnly)
	{
		if( bNumberOnly )
		{
			m_iWindowStyls |= ES_NUMBER;
		}
		else
		{
			m_iWindowStyls &= ~ES_NUMBER;
		}
	}

	bool CEditUI::IsNumberOnly() const
	{
		return (m_iWindowStyls & ES_NUMBER) ? true:false;
	}

	int CEditUI::GetNativeEditRightReserve() const
	{
		return 0;
	}

	void CEditUI::OnNativeEditChanged()
	{
		if( m_pManager ) m_pManager->SendNotify(this, DUI_MSGTYPE_TEXTCHANGED);
	}

	int CEditUI::GetWindowStyls() const 
	{
		return m_iWindowStyls;
	}

	void CEditUI::SetPasswordMode(bool bPasswordMode)
	{
		if( m_bPasswordMode == bPasswordMode ) return;
		m_bPasswordMode = bPasswordMode;
		Invalidate();
		if( m_pWindow != NULL ) {
			LONG styleValue = ::GetWindowLong(*m_pWindow, GWL_STYLE);
			bPasswordMode ? styleValue |= ES_PASSWORD : styleValue &= ~ES_PASSWORD;
			::SetWindowLong(*m_pWindow, GWL_STYLE, styleValue);
		}
	}

	bool CEditUI::IsPasswordMode() const
	{
		return m_bPasswordMode;
	}

	void CEditUI::SetPasswordChar(TCHAR cPasswordChar)
	{
		if( m_cPasswordChar == cPasswordChar ) return;
		m_cPasswordChar = cPasswordChar;
		if( m_pWindow != NULL ) Edit_SetPasswordChar(*m_pWindow, m_cPasswordChar);
		Invalidate();
	}

	TCHAR CEditUI::GetPasswordChar() const
	{
		return m_cPasswordChar;
	}

	LPCTSTR CEditUI::GetImage()
	{
		return m_sImage.GetData();
	}

	void CEditUI::SetImage(LPCTSTR pStrImage)
	{
		m_sImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditUI::GetHoverImage()
	{
		return m_sHoverImage.GetData();
	}

	void CEditUI::SetHoverImage(LPCTSTR pStrImage)
	{
		m_sHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditUI::GetFocusImage()
	{
		return m_sFocusImage.GetData();
	}

	void CEditUI::SetFocusImage(LPCTSTR pStrImage)
	{
		m_sFocusImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditUI::GetDisabledImage()
	{
		return m_sDisabledImage.GetData();
	}

	void CEditUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	void CEditUI::SetNativeEditBackgroundColor(DWORD dwBackgroundColor)
	{
		m_dwEditbkColor = dwBackgroundColor;
		m_bNativeBkColorCustom = true;
		SyncNativeEditColors();
	}

	DWORD CEditUI::GetNativeEditBackgroundColor() const
	{
		// 未写 native-background-color 时跟控件背景（主题/默认属性），避免聚焦 WC_EDIT 冒出白条
		if( m_bNativeBkColorCustom ) return m_dwEditbkColor;
		return GetBackgroundColor();
	}

	void CEditUI::SetNativeEditColor( LPCTSTR pStrColor )
	{
		DWORD clrColor = 0;
		if( ParseColorString(pStrColor, clrColor) ) {
			m_dwEditTextColor = clrColor;
			m_bNativeTextColorCustom = true;
			SyncNativeEditColors();
		}
	}

	DWORD CEditUI::GetNativeEditColor() const
	{
		if( m_bNativeTextColorCustom ) return m_dwEditTextColor;
		DWORD c = GetColor();
		if( c != 0 ) return c;
		if( m_pManager != NULL ) return m_pManager->GetDefaultFontColor();
		return 0x000000E0;
	}

	void CEditUI::SyncNativeEditColors()
	{
		if( m_pWindow == NULL ) return;
		HWND hWnd = m_pWindow->GetHWND();
		if( hWnd != NULL && ::IsWindow(hWnd) )
			::InvalidateRect(hWnd, NULL, TRUE);
	}

	bool CEditUI::IsAutoSelAll()
	{
		return m_bAutoSelAll;
	}

	void CEditUI::SetAutoSelAll(bool bAutoSelAll)
	{
		m_bAutoSelAll = bAutoSelAll;
	}

	void CEditUI::SetSel(long nStartChar, long nEndChar)
	{
		if( m_pWindow != NULL ) Edit_SetSel(*m_pWindow, nStartChar,nEndChar);
	}

	void CEditUI::SetSelAll()
	{
		SetSel(0,-1);
	}

	void CEditUI::SetReplaceSel(LPCTSTR lpszReplace)
	{
		if( m_pWindow != NULL ) Edit_ReplaceSel(*m_pWindow, lpszReplace);
	}

	void CEditUI::SetPlaceholder( LPCTSTR pStrPlaceholder )
	{
		m_sPlaceholder	= pStrPlaceholder;
	}

	LPCTSTR CEditUI::GetPlaceholder()
	{
		if (!IsResourceText()) return m_sPlaceholder.GetData();
		return CResourceManager::GetInstance()->GetText(m_sPlaceholder.GetData()).GetData();
	}

	void CEditUI::SetPlaceholderColor( LPCTSTR pStrColor )
	{
		DWORD clrColor = 0;
		if( ParseColorString(pStrColor, clrColor) )
			m_dwPlaceholderColor = clrColor;
	}

	DWORD CEditUI::GetPlaceholderColor()
	{
		return m_dwPlaceholderColor;
	}

	HWND CEditUI::GetHWND()
	{
		if(m_pWindow != NULL) {
			return m_pWindow->GetHWND();
		}
		return NULL;
	}

	void CEditUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		if( m_pWindow != NULL ) {
			RECT rcPos = m_pWindow->CalPos();
			::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left, 
				rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE);        
		}
	}

	void CEditUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CControlUI::Move(szOffset, bNeedInvalidate);
		if( m_pWindow != NULL ) {
			RECT rcPos = m_pWindow->CalPos();
			::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left, 
				rcPos.bottom - rcPos.top, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);        
		}
	}

	void CEditUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	void CEditUI::SetInternVisible(bool bVisible)
	{
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	SIZE CEditUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 ) return CDuiSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 6);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("readonly")) == 0 ) SetReadOnly(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("type")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("number")) == 0 ) SetNumberOnly(true);
			else if( _tcsicmp(pstrValue, _T("password")) == 0 ) SetPasswordMode(true);
			else if( _tcsicmp(pstrValue, _T("text")) == 0 ) {
				SetNumberOnly(false);
				SetPasswordMode(false);
			}
		}
		else if( _tcsicmp(pstrName, _T("select-on-focus")) == 0 ) SetAutoSelAll(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("password")) == 0 ) SetPasswordMode(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("password-char")) == 0 ) SetPasswordChar(*pstrValue);
		else if( _tcsicmp(pstrName, _T("maxlength")) == 0 ) SetMaxChar(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("image")) == 0 ) SetImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-hover")) == 0 ) SetHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-focus")) == 0 ) SetFocusImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-disabled")) == 0 ) SetDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("placeholder")) == 0 ) SetPlaceholder(pstrValue);
		else if( _tcsicmp(pstrName, _T("placeholder-color")) == 0 ) SetPlaceholderColor(pstrValue);
		else if( _tcsicmp(pstrName, _T("native-color")) == 0 ) SetNativeEditColor(pstrValue);
		else if( _tcsicmp(pstrName, _T("native-background-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetNativeEditBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("value")) == 0 ) SetText(pstrValue);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CEditUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sDisabledImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sFocusImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sHoverImage.GetData()) ) {}
				else return;
			}
		}

		if( !m_sImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sImage.GetData()) ) {}
			else return;
		}
	}

	void CEditUI::PaintText(IRenderContext& ctx)
	{
		// 祖先/自身 opacity 变半透明时拆掉原生窗，改自绘
		if( m_pWindow != NULL && !CanHostNativeEdit() )
			DismissNativeEdit();

		// 聚焦且原生 Edit 仍在时由 WC_EDIT 绘制；否则自绘。
		// 走 ctx.DrawText（与 Label 一致），避免 GetDC+圆角裁剪栈导致文字画不上。
		if( m_pWindow != NULL ) return;

		DWORD mCurTextColor = m_dwColor;

		if( m_dwColor == 0 ) mCurTextColor = m_dwColor = m_pManager->GetDefaultFontColor();		
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();

		CDuiString sDrawText = GetText();
		CDuiString sPlaceholder = GetPlaceholder();
		if(sDrawText == sPlaceholder || sDrawText == _T("")) {
			mCurTextColor = m_dwPlaceholderColor;
			sDrawText = sPlaceholder;
		}
		else {
			CDuiString sTemp = sDrawText;
			if( m_bPasswordMode ) {
				sDrawText.Empty();
				LPCTSTR pStr = sTemp.GetData();
				while( *pStr != _T('\0') ) {
					sDrawText += m_cPasswordChar;
					pStr = ::CharNext(pStr);
				}
			}
		}

		RECT rcPad = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		RECT rc = m_rcItem;
		rc.left += rcPad.left + rcTextPadding.left;
		rc.right -= rcPad.right + rcTextPadding.right;
		rc.top += rcPad.top + rcTextPadding.top;
		rc.bottom -= rcPad.bottom + rcTextPadding.bottom;
		int nReserve = GetNativeEditRightReserve();
		if( nReserve > 0 && m_pManager )
			rc.right -= m_pManager->GetDPIObj()->Scale(nReserve);
		if( rc.right < rc.left + 4 ) rc.right = rc.left + 4;

		DWORD clrColor = IsEnabled() ? mCurTextColor : m_dwDisabledColor;
		ctx.DrawText(rc, sDrawText.GetData(), GetAdjustColor(clrColor), m_iFont, DT_SINGLELINE | m_uTextStyle);
	}

	bool CEditUI::StartQueueTimer(UINT idTimer, UINT uElapse)
	{
		if( uElapse == 0 ) uElapse = 530;
		if( m_pQueueTimers == NULL )
			m_pQueueTimers = new EditQueueTimers;
		if( m_pManager == NULL ) return false;
		StopQueueTimer(idTimer);
		EditTimerCtx* pCtx = new EditTimerCtx;
		pCtx->pSelf = this;
		pCtx->idTimer = idTimer;
		HANDLE hTimer = NULL;
		if( !::CreateTimerQueueTimer(&hTimer, NULL, EditQueueTimerProc,
			reinterpret_cast<PVOID>(pCtx), uElapse, uElapse, WT_EXECUTEDEFAULT) ) {
			delete pCtx;
			return false;
		}
		EditQueueTimers::Entry e;
		e.idTimer = idTimer;
		e.hTimer = hTimer;
		e.pCtx = pCtx;
		m_pQueueTimers->items.push_back(e);
		return true;
	}

	void CEditUI::StopQueueTimer(UINT idTimer)
	{
		if( m_pQueueTimers != NULL )
			m_pQueueTimers->Stop(idTimer);
	}

	void CEditUI::StopAllQueueTimers()
	{
		if( m_pQueueTimers != NULL ) {
			m_pQueueTimers->StopAll();
			delete m_pQueueTimers;
			m_pQueueTimers = NULL;
		}
	}

	void CEditUI::StartCaretBlinkTimer()
	{
		StartQueueTimer(EDIT_CARET_BLINK_TIMERID, GetEditCaretBlinkInterval());
	}

	void CEditUI::OnQueueTimerTick(UINT idTimer)
	{
		if( !IsFocused() ) return;
		if( idTimer == EDIT_CARET_BLINK_TIMERID && m_pWindow != NULL )
			m_pWindow->OnCaretBlinkTick();
	}

	void DuiLib_EditOnQueueTick(CEditUI* pEdit, UINT idTimer)
	{
		if( pEdit != NULL )
			pEdit->OnQueueTimerTick(idTimer);
	}
}
