#include "StdAfx.h"
#include <algorithm>
namespace DuiLib
{
	//////////////////////////////////////////////////////////////////////////
	//
	DUI_BEGIN_MESSAGE_MAP(WindowImplBase, CNotifyPump)
		DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,WindowImplBase::OnClick)
	DUI_END_MESSAGE_MAP()

	void WindowImplBase::OnFinalMessage( HWND hWnd )
	{
		m_pm.RemovePreMessageFilter(this);
		m_pm.RemoveNotifier(this);
		m_pm.ReapObjects(m_pm.GetRoot());
	}

	LRESULT WindowImplBase::ResponseDefaultKeyEvent(WPARAM wParam)
	{
		if (wParam == VK_RETURN)
		{
			return FALSE;
		}
		else if (wParam == VK_ESCAPE)
		{
			return TRUE;
		}

		return FALSE;
	}

	UINT WindowImplBase::GetClassStyle() const
	{
		return CS_DBLCLKS;
	}

	CControlUI* WindowImplBase::CreateControl(LPCTSTR pstrClass)
	{
		return NULL;
	}

	LPCTSTR WindowImplBase::QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType)
	{
		return NULL;
	}

	LRESULT WindowImplBase::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/)
	{
        if (uMsg == WM_KEYDOWN)
        {
            switch (wParam)
            {
            case VK_RETURN:
            case VK_ESCAPE:
                return ResponseDefaultKeyEvent(wParam);
            default:
                break;
            }
        }
		return FALSE;
	}

	LRESULT WindowImplBase::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

#if defined(WIN32) && !defined(UNDER_CE)
	LRESULT WindowImplBase::OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsIconic(*this) ) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT WindowImplBase::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT WindowImplBase::OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}


	BOOL WindowImplBase::IsInStaticControl(CControlUI *pControl)
	{
		BOOL bRet = FALSE;
		if (!pControl)
		{
			return bRet;
		}

		CDuiString strClassName;
		std::vector<CDuiString> vctStaticName;

		strClassName = pControl->GetClass();
		strClassName.MakeLower();
		vctStaticName.push_back(_T("controlui"));
		vctStaticName.push_back(_T("textui"));
		vctStaticName.push_back(_T("labelui"));
		vctStaticName.push_back(_T("containerui"));
		vctStaticName.push_back(_T("horizontallayoutui"));
		vctStaticName.push_back(_T("verticallayoutui"));
		vctStaticName.push_back(_T("tablayoutui"));
		vctStaticName.push_back(_T("childlayoutui"));
		vctStaticName.push_back(_T("dialoglayoutui"));
		vctStaticName.push_back(_T("progresscontainerui"));
		std::vector<CDuiString>::iterator it = std::find(vctStaticName.begin(), vctStaticName.end(), strClassName);
		if (vctStaticName.end() != it)
		{
			CControlUI* pParent = pControl->GetParent();
			while (pParent)
			{
				strClassName = pParent->GetClass();
				strClassName.MakeLower();
				it = std::find(vctStaticName.begin(), vctStaticName.end(), strClassName);
				if (vctStaticName.end() == it)
				{
					return bRet;
				}

				pParent = pParent->GetParent();
			}

			bRet = TRUE;
		}

		return bRet;
	}

	LRESULT WindowImplBase::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		// 注意：不要对 maxbtn/restorebtn 返回 HTMAXBUTTON。
		// 返回 HTMAXBUTTON 后鼠标消息变成 WM_NCMOUSE*，客户区 TrackMouseEvent
		// 会在进入该区域时触发 WM_MOUSELEAVE 并用 (-1,-1) 清掉 hover，
		// 导致最大化按钮 hotimage 不生效（最小化/关闭仍为 HTCLIENT 则正常）。
		// Win11 Snap Layouts 若需要，应另做非客户区 hover 同步，而不是牺牲热态绘制。

		if (!::IsZoomed(*this))
		{
			RECT rcSizeBox = m_pm.GetSizeBox();
			if (pt.y < rcClient.top + rcSizeBox.top)
			{
				if (pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;
				return HTTOP;
			}
			else if (pt.y > rcClient.bottom - rcSizeBox.bottom)
			{
				if (pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;
				return HTBOTTOM;
			}

			if (pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
		}

		// action 属性驱动的拖拽：不受 caption rect 限制；IsCaptionDragHit 区分空白/交互区
		{
			CControlUI* pHitCtrl = m_pm.FindControl(pt);
			if (pHitCtrl != NULL) {
				if (pHitCtrl->IsCaptionDragHit(pt))
					return HTCAPTION;

				UIAction leafAct = pHitCtrl->GetAction();
				// 自身有 title 但点在交互区（如 TabBar 标签/+）：保持 HTCLIENT，不向上/窗口级拖拽
				if (leafAct == UIACTION_NONE && !(pHitCtrl->GetControlFlags() & UIFLAG_SETCURSOR)) {
					CControlUI* pWalk = pHitCtrl->GetParent();
					while (pWalk != NULL) {
						if (pWalk->IsCaptionDragHit(pt))
							return HTCAPTION;
						UIAction parentAct = pWalk->GetAction();
						// 父级是 title 但该点不可拖（交互区）→ 停止向上，留给客户区点击
						if (parentAct == UIACTION_TITLE || parentAct == UIACTION_MOVEWINDOW)
							break;
						if (parentAct != UIACTION_NONE) break;
						pWalk = pWalk->GetParent();
					}
					UIAction winAct = m_pm.GetWindowAction();
					if (winAct == UIACTION_TITLE || winAct == UIACTION_MOVEWINDOW)
						return HTCAPTION;
				}
			}
			else {
				UIAction winAct = m_pm.GetWindowAction();
				if (winAct == UIACTION_TITLE || winAct == UIACTION_MOVEWINDOW)
					return HTCAPTION;
			}
		}

		RECT rcCaption = m_pm.GetCaptionRect();
		if (0 > rcCaption.bottom)
		{
			rcCaption.bottom = rcClient.bottom;
		}

		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom)
		{
			if (IsInStaticControl(m_pm.FindControl(pt)))
			{
				return HTCAPTION;
			}
		}

		return HTCLIENT;
	}

	LRESULT WindowImplBase::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MONITORINFO mi = {};
		mi.cbSize = sizeof(mi);
		// 用窗口所在屏（多显示器）；勿用 PRIMARY，否则副屏最大化会套主屏工作区
		::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi);
		const RECT& rcWork = mi.rcWork;
		const RECT& rcMon = mi.rcMonitor;
		const int cxWork = rcWork.right - rcWork.left;
		const int cyWork = rcWork.bottom - rcWork.top;
		// ptMaxPosition 相对当前显示器左上角；保留任务栏占用（含顶部任务栏）
		// 旧逻辑对非主屏 OffsetRect 到 (0,0) 会抹掉 rcWork.top 偏移，标题栏被挡

		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
		lpMMI->ptMaxPosition.x = rcWork.left - rcMon.left;
		lpMMI->ptMaxPosition.y = rcWork.top - rcMon.top;
		lpMMI->ptMaxSize.x = cxWork;
		lpMMI->ptMaxSize.y = cyWork;
		lpMMI->ptMaxTrackSize.x = m_pm.GetMaxInfo().cx == 0 ? cxWork : m_pm.GetMaxInfo().cx;
		lpMMI->ptMaxTrackSize.y = m_pm.GetMaxInfo().cy == 0 ? cyWork : m_pm.GetMaxInfo().cy;
		lpMMI->ptMinTrackSize.x = m_pm.GetMinInfo().cx;
		lpMMI->ptMinTrackSize.y = m_pm.GetMinInfo().cy;

		bHandled = TRUE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}
#endif

	LRESULT WindowImplBase::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
#if defined(WIN32) && !defined(UNDER_CE)
		if( !::IsIconic(*this) ) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);

			if (m_pm.IsValid() && wParam == SIZE_RESTORED) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
				if (pControl) pControl->SetVisible(true);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
				if (pControl) pControl->SetVisible(false);
			}
		}
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == SC_CLOSE)
		{
			bHandled = TRUE;
			SendMessage(WM_CLOSE);
			return 0;
		}
#if defined(WIN32) && !defined(UNDER_CE)
		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if( ::IsZoomed(*this) != bZoomed ) {
			if( !bZoomed ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
				if( pControl ) pControl->SetVisible(false);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
				if( pControl ) pControl->SetVisible(true);
			}
			else {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
				if( pControl ) pControl->SetVisible(true);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
				if( pControl ) pControl->SetVisible(false);
			}
		}
#else
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
#endif
		return lRes;
	}

	LRESULT WindowImplBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 调整窗口样式
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		// 关联UI管理器
		m_pm.Init(m_hWnd, GetManagerName());
		// 注册PreMessage回调
		m_pm.AddPreMessageFilter(this);

		// 创建主窗口
		CControlUI* pRoot=NULL;
		CDialogBuilder builder;
		CDuiString sSkinType = GetSkinType();
		if (!sSkinType.IsEmpty()) {
			STRINGorID xml(_ttoi(GetSkinFile().GetData()));
			pRoot = builder.Create(xml, sSkinType, this, &m_pm);
		}
		else {
			pRoot = builder.Create(GetSkinFile().GetData(), (UINT)0, this, &m_pm);
		}

		if (pRoot == NULL) {
			CDuiString sError = _T("加载资源文件失败：");
			sError += GetSkinFile();
			MessageBox(NULL, sError, _T("Duilib") ,MB_OK|MB_ICONERROR);
			ExitProcess(1);
			return 0;
		}
		m_pm.AttachDialog(pRoot);
		// 添加Notify事件接口
		m_pm.AddNotifier(this);
		// 皮肤 size 属性：创建后按 InitSize 调整客户区（未设置则保持 Create 尺寸）
		{
			SIZE szInit = m_pm.GetInitSize();
			if( szInit.cx > 0 && szInit.cy > 0 )
				ResizeClient(szInit.cx, szInit.cy);
		}
		// 窗口初始化完毕
		InitWindow();
		return 0;
	}

	LRESULT WindowImplBase::OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch (uMsg)
		{
		case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:		lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
#if defined(WIN32) && !defined(UNDER_CE)
		case WM_NCACTIVATE:		lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:		lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:		lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:		lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCLBUTTONDBLCLK: {
			if (wParam == HTMAXBUTTON) {
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(m_hWnd, &pt);
				LPARAM param = MAKELPARAM(pt.x, pt.y);
				if (uMsg == WM_NCLBUTTONDOWN) {
					::SendMessage(m_hWnd, WM_LBUTTONDOWN, wParam, param);
				}
				else if (uMsg == WM_NCLBUTTONUP) {
					::SendMessage(m_hWnd, WM_LBUTTONUP, wParam, param);
				}
				return 0;
			}
			bHandled = FALSE;
			break;
		}
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSELEAVE:
		case WM_NCMOUSEMOVE: {
			if (wParam == HTMAXBUTTON) {
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(m_hWnd, &pt);
				LPARAM param = MAKELPARAM(pt.x, pt.y);
				LRESULT lr = 0;
				if (uMsg == WM_NCMOUSELEAVE) {
					m_pm.MessageHandler(WM_MOUSELEAVE, 0, 0, lr);
				}
				else if (uMsg == WM_NCMOUSEMOVE) {
					TRACKMOUSEEVENT tme = { 0 };
					tme.cbSize = sizeof(tme);
					tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
					tme.hwndTrack = m_hWnd;
					::TrackMouseEvent(&tme);
					// wParam 必须是按键标志，不能传 HTMAXBUTTON
					m_pm.MessageHandler(WM_MOUSEMOVE, 0, param, lr);
				}
				return 0;
			}
			bHandled = FALSE;
			break;
		}
		case WM_GETMINMAXINFO:	lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEWHEEL:		lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
#endif
		case WM_SIZE:			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_CHAR:		lRes = OnChar(uMsg, wParam, lParam, bHandled); break;
		case WM_SYSCOMMAND:		lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		case WM_KILLFOCUS:		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_SETFOCUS:		lRes = OnSetFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONUP:		lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONDOWN:	lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEMOVE:		lRes = OnMouseMove(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEHOVER:	lRes = OnMouseHover(uMsg, wParam, lParam, bHandled); break;
		default:				bHandled = FALSE; break;
		}
		if (bHandled) return lRes;

		lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled);
		if (bHandled) return lRes;

		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes))
			return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT WindowImplBase::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LONG WindowImplBase::GetStyle()
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;

		return styleValue;
	}

	void WindowImplBase::OnClick(TNotifyUI& msg)
	{
		CControlUI* pSender = msg.pSender;
		if (pSender != NULL) {
			switch (pSender->GetAction())
			{
			case UIACTION_CLOSE:
				Close();
				return;
			case UIACTION_MIN:
				SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return;
			case UIACTION_MAX:
				SendMessage(WM_SYSCOMMAND, ::IsZoomed(m_hWnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
				return;
			case UIACTION_COPY:
				{
					CDuiString text = pSender->GetCustomAttribute(_T("copy-text"));
					if (text.IsEmpty()) text = pSender->GetText();
					if (!text.IsEmpty() && ::OpenClipboard(m_hWnd)) {
						::EmptyClipboard();
						size_t nBytes = (text.GetLength() + 1) * sizeof(TCHAR);
						HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, nBytes);
						if (hMem) {
							void* p = ::GlobalLock(hMem);
							if (p) {
								memcpy(p, text.GetData(), nBytes);
								::GlobalUnlock(hMem);
#ifdef _UNICODE
								::SetClipboardData(CF_UNICODETEXT, hMem);
#else
								::SetClipboardData(CF_TEXT, hMem);
#endif
							}
						}
						::CloseClipboard();
					}
				}
				return;
			default:
				break;
			}
		}

		CDuiString sCtrlName = msg.pSender->GetName();
		if( sCtrlName == _T("closebtn") ) {
			Close();
			return; 
		}
		else if( sCtrlName == _T("minbtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
			return; 
		}
		else if( sCtrlName == _T("maxbtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
			return; 
		}
		else if( sCtrlName == _T("restorebtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
			return; 
		}
		return;
	}

	void WindowImplBase::Notify(TNotifyUI& msg)
	{
		return CNotifyPump::NotifyPump(msg);
	}
}