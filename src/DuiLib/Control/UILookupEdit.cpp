#include "StdAfx.h"
#include "UILookupEdit.h"
#include <vector>

namespace DuiLib {

	namespace {

		int LookupScale(CPaintManagerUI* pManager, int nLogic)
		{
			if( pManager == NULL ) return nLogic;
			return pManager->GetDPIObj()->Scale(nLogic);
		}

		bool LookupContainsI(LPCTSTR pHay, LPCTSTR pNeedle)
		{
			if( pNeedle == NULL || pNeedle[0] == _T('\0') ) return true;
			if( pHay == NULL ) pHay = _T("");
			CDuiString sHay(pHay);
			CDuiString sNeedle(pNeedle);
			sHay.MakeLower();
			sNeedle.MakeLower();
			return sHay.Find(sNeedle.GetData()) >= 0;
		}

		void LookupApplyDefault(CControlUI* pControl, CPaintManagerUI* pManager, LPCTSTR pstrClass)
		{
			if( pControl == NULL || pManager == NULL || pstrClass == NULL ) return;
			LPCTSTR pDefault = pManager->GetDefaultAttributeList(pstrClass);
			if( pDefault != NULL ) pControl->ApplyAttributeList(pDefault);
		}

		bool LookupImeComposing(HWND hWnd)
		{
			if( hWnd == NULL || !::IsWindow(hWnd) ) return false;
			HIMC hImc = ::ImmGetContext(hWnd);
			if( hImc == NULL ) return false;
			LONG nComp = ::ImmGetCompositionString(hImc, GCS_COMPSTR, NULL, 0);
			::ImmReleaseContext(hWnd, hImc);
			return nComp > 0;
		}

		void LookupClampRect(RECT& rc, const RECT& rcWork)
		{
			const int w = rc.right - rc.left;
			const int h = rc.bottom - rc.top;
			if( rc.left < rcWork.left ) {
				rc.left = rcWork.left;
				rc.right = rc.left + w;
			}
			if( rc.right > rcWork.right ) {
				rc.right = rcWork.right;
				rc.left = rc.right - w;
				if( rc.left < rcWork.left ) {
					rc.left = rcWork.left;
					rc.right = rcWork.right;
				}
			}
			if( rc.top < rcWork.top ) {
				rc.top = rcWork.top;
				rc.bottom = rc.top + h;
			}
			if( rc.bottom > rcWork.bottom ) {
				rc.bottom = rcWork.bottom;
				rc.top = rc.bottom - h;
				if( rc.top < rcWork.top ) {
					rc.top = rcWork.top;
					rc.bottom = rcWork.bottom;
				}
			}
		}

		RECT LookupPlacePopup(const RECT& rcOwn, int cx, int cy,
			CLookupEditUI::DropPosition ePos, const RECT& rcWork)
		{
			RECT rc = { 0, 0, cx, cy };
			switch( ePos ) {
			case CLookupEditUI::DropTop:
				rc.left = rcOwn.left;
				rc.right = rc.left + cx;
				rc.bottom = rcOwn.top;
				rc.top = rc.bottom - cy;
				if( rc.top < rcWork.top && (rcOwn.bottom + cy) <= rcWork.bottom ) {
					rc.top = rcOwn.bottom;
					rc.bottom = rc.top + cy;
				}
				break;
			case CLookupEditUI::DropLeft:
				rc.right = rcOwn.left;
				rc.left = rc.right - cx;
				rc.top = rcOwn.top;
				rc.bottom = rc.top + cy;
				if( rc.left < rcWork.left && (rcOwn.right + cx) <= rcWork.right ) {
					rc.left = rcOwn.right;
					rc.right = rc.left + cx;
				}
				break;
			case CLookupEditUI::DropRight:
				rc.left = rcOwn.right;
				rc.right = rc.left + cx;
				rc.top = rcOwn.top;
				rc.bottom = rc.top + cy;
				if( rc.right > rcWork.right && (rcOwn.left - cx) >= rcWork.left ) {
					rc.right = rcOwn.left;
					rc.left = rc.right - cx;
				}
				break;
			case CLookupEditUI::DropCenter:
				rc.left = rcOwn.left + (rcOwn.right - rcOwn.left - cx) / 2;
				rc.top = rcOwn.top + (rcOwn.bottom - rcOwn.top - cy) / 2;
				rc.right = rc.left + cx;
				rc.bottom = rc.top + cy;
				break;
			case CLookupEditUI::DropBottom:
			default:
				rc.left = rcOwn.left;
				rc.right = rc.left + cx;
				rc.top = rcOwn.bottom;
				rc.bottom = rc.top + cy;
				if( rc.bottom > rcWork.bottom && (rcOwn.top - cy) >= rcWork.top ) {
					rc.bottom = rcOwn.top;
					rc.top = rc.bottom - cy;
				}
				break;
			}
			LookupClampRect(rc, rcWork);
			return rc;
		}

		bool LookupParseDropPosition(LPCTSTR pstrValue, CLookupEditUI::DropPosition& ePos)
		{
			if( pstrValue == NULL || pstrValue[0] == _T('\0') ) return false;
			if( _tcsicmp(pstrValue, _T("top")) == 0 || _tcsicmp(pstrValue, _T("up")) == 0
				|| _tcscmp(pstrValue, _T("上")) == 0 ) {
				ePos = CLookupEditUI::DropTop;
				return true;
			}
			if( _tcsicmp(pstrValue, _T("left")) == 0 || _tcscmp(pstrValue, _T("左")) == 0 ) {
				ePos = CLookupEditUI::DropLeft;
				return true;
			}
			if( _tcsicmp(pstrValue, _T("right")) == 0 || _tcscmp(pstrValue, _T("右")) == 0 ) {
				ePos = CLookupEditUI::DropRight;
				return true;
			}
			if( _tcsicmp(pstrValue, _T("center")) == 0 || _tcsicmp(pstrValue, _T("middle")) == 0
				|| _tcscmp(pstrValue, _T("中")) == 0 || _tcscmp(pstrValue, _T("居中")) == 0 ) {
				ePos = CLookupEditUI::DropCenter;
				return true;
			}
			if( _tcsicmp(pstrValue, _T("bottom")) == 0 || _tcsicmp(pstrValue, _T("down")) == 0
				|| _tcscmp(pstrValue, _T("下")) == 0 ) {
				ePos = CLookupEditUI::DropBottom;
				return true;
			}
			return false;
		}

		bool LookupParseTextAlign(LPCTSTR pstrValue, UINT& uAlign)
		{
			if( pstrValue == NULL || pstrValue[0] == _T('\0') ) return false;
			if( _tcsicmp(pstrValue, _T("left")) == 0 || _tcscmp(pstrValue, _T("左")) == 0 ) {
				uAlign = DT_LEFT;
				return true;
			}
			if( _tcsicmp(pstrValue, _T("center")) == 0 || _tcsicmp(pstrValue, _T("middle")) == 0
				|| _tcscmp(pstrValue, _T("中")) == 0 || _tcscmp(pstrValue, _T("居中")) == 0 ) {
				uAlign = DT_CENTER;
				return true;
			}
			if( _tcsicmp(pstrValue, _T("right")) == 0 || _tcscmp(pstrValue, _T("右")) == 0 ) {
				uAlign = DT_RIGHT;
				return true;
			}
			if( _tcsstr(pstrValue, _T("left")) != NULL ) {
				uAlign = DT_LEFT;
				return true;
			}
			if( _tcsstr(pstrValue, _T("center")) != NULL ) {
				uAlign = DT_CENTER;
				return true;
			}
			if( _tcsstr(pstrValue, _T("right")) != NULL ) {
				uAlign = DT_RIGHT;
				return true;
			}
			return false;
		}

	} // namespace

	class CLookupWnd : public CWindowWnd, public INotifyUI, public IVirtualListCallback, public IMessageFilterUI
	{
	public:
		CLookupWnd();
		void Init(CLookupEditUI* pOwner);
		void Confirm();
		void Cancel();
		void RebuildVisible();

		LPCTSTR GetWindowClassName() const override;
		UINT GetClassStyle() const override;
		void OnFinalMessage(HWND hWnd) override;
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
		void Notify(TNotifyUI& msg) override;

		LPCTSTR GetItemText(CControlUI* pList, int iIndex) override;
		bool PaintItem(CControlUI* pList, IRenderContext& ctx, int iIndex, const RECT& rcItem, UINT uState) override;

	public:
		CPaintManagerUI m_pm;
		CLookupEditUI* m_pOwner;
		CVirtualListUI* m_pList;
		std::vector<CEditUI*> m_aFilters;
		std::vector<int> m_aColWidth;
		std::vector<int> m_aVisible;
	};

	CLookupWnd::CLookupWnd()
		: m_pOwner(NULL)
		, m_pList(NULL)
	{
	}

	LPCTSTR CLookupWnd::GetWindowClassName() const
	{
		return _T("LookupEditWnd");
	}

	UINT CLookupWnd::GetClassStyle() const
	{
		return CS_DBLCLKS | CS_DROPSHADOW;
	}

	void CLookupWnd::OnFinalMessage(HWND /*hWnd*/)
	{
		m_pm.RemovePreMessageFilter(this);
		if( m_pOwner != NULL ) {
			m_pOwner->m_pWindow = NULL;
			m_pOwner->m_uButtonState &= ~UISTATE_PUSHED;
			m_pOwner->Invalidate();
		}
		delete this;
	}

	void CLookupWnd::Init(CLookupEditUI* pOwner)
	{
		m_pOwner = pOwner;
		if( pOwner == NULL || pOwner->GetManager() == NULL ) return;

		CPaintManagerUI* pOwnerPm = pOwner->GetManager();
		SIZE szDrop = pOwner->GetDropBoxSize();
		RECT rcOwner = pOwner->GetPos();

		m_aColWidth.clear();
		int cxCols = 0;
		const int nColCount = pOwner->GetCount();
		if( nColCount <= 0 ) {
			int w = LookupScale(pOwnerPm, 160);
			m_aColWidth.push_back(160);
			cxCols = w;
		}
		else {
			for( int i = 0; i < nColCount; ++i ) {
				int nLogic = 80;
				CLookupColumnUI* pCol = pOwner->GetColumn(i);
				if( pCol != NULL ) nLogic = pCol->GetLogicWidth();
				if( nLogic <= 0 ) nLogic = 80;
				m_aColWidth.push_back(nLogic);
				cxCols += LookupScale(pOwnerPm, nLogic);
			}
		}

		const int cxScroll = LookupScale(pOwnerPm, 18);
		int cxAuto = rcOwner.right - rcOwner.left;
		if( cxCols + cxScroll > cxAuto ) cxAuto = cxCols + cxScroll;
		int cx = cxAuto;
		if( szDrop.cx > 0 )
			cx = LookupScale(pOwnerPm, szDrop.cx);

		int cyLogic = (szDrop.cy > 0) ? szDrop.cy : 280;
		int cy = LookupScale(pOwnerPm, cyLogic);
		const int cyHead = LookupScale(pOwnerPm, 26) + LookupScale(pOwnerPm, 28)
			+ LookupScale(pOwnerPm, 32) + LookupScale(pOwnerPm, 8);
		if( cy < cyHead + LookupScale(pOwnerPm, 80) )
			cy = cyHead + LookupScale(pOwnerPm, 80);

		RECT rcOwnScreen = rcOwner;
		::MapWindowRect(pOwnerPm->GetPaintWindow(), HWND_DESKTOP, &rcOwnScreen);

		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromRect(&rcOwnScreen, MONITOR_DEFAULTTONEAREST), &oMonitor);

		RECT rc = LookupPlacePopup(rcOwnScreen, cx, cy, pOwner->GetDropPosition(), oMonitor.rcWork);
		Create(pOwnerPm->GetPaintWindow(), NULL, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW, rc);
	}

	void CLookupWnd::Confirm()
	{
		if( m_pOwner == NULL || m_pList == NULL ) {
			Cancel();
			return;
		}
		const int iVis = m_pList->GetCurSel();
		if( iVis < 0 || iVis >= (int)m_aVisible.size() ) {
			Cancel();
			return;
		}
		m_pOwner->m_iPendingPick = m_aVisible[iVis];
		Close(IDOK);
	}

	void CLookupWnd::Cancel()
	{
		if( m_pOwner != NULL )
			m_pOwner->m_iPendingPick = -1;
		Close(IDCANCEL);
	}

	void CLookupWnd::RebuildVisible()
	{
		m_aVisible.clear();
		ILookupEditCallback* pCb = (m_pOwner != NULL) ? m_pOwner->GetCallback() : NULL;
		const int nRows = (pCb != NULL) ? pCb->GetRowCount() : 0;
		const int nCols = (int)m_aColWidth.size();

		std::vector<CDuiString> aFilter;
		aFilter.resize((size_t)MAX(nCols, 0));
		for( int c = 0; c < nCols && c < (int)m_aFilters.size(); ++c ) {
			if( m_aFilters[c] != NULL )
				aFilter[c] = m_aFilters[c]->GetText();
		}

		for( int r = 0; r < nRows; ++r ) {
			bool bHit = true;
			for( int c = 0; c < nCols; ++c ) {
				if( aFilter[c].IsEmpty() ) continue;
				LPCTSTR pCell = (pCb != NULL) ? pCb->GetCellText(r, c) : NULL;
				if( !LookupContainsI(pCell, aFilter[c].GetData()) ) {
					bHit = false;
					break;
				}
			}
			if( bHit ) m_aVisible.push_back(r);
		}

		if( m_pList != NULL ) {
			const int nNew = (int)m_aVisible.size();
			if( m_pList->GetItemCount() != nNew )
				m_pList->SetItemCount(nNew);
			else
				m_pList->Invalidate();
			int iSel = -1;
			const int iCur = (m_pOwner != NULL) ? m_pOwner->GetCurSel() : -1;
			if( iCur >= 0 ) {
				for( int i = 0; i < (int)m_aVisible.size(); ++i ) {
					if( m_aVisible[i] == iCur ) {
						iSel = i;
						break;
					}
				}
			}
			if( iSel < 0 && !m_aVisible.empty() ) {
				bool bAnyFilter = false;
				for( int c = 0; c < (int)aFilter.size(); ++c ) {
					if( !aFilter[c].IsEmpty() ) {
						bAnyFilter = true;
						break;
					}
				}
				if( bAnyFilter ) iSel = 0;
			}
			m_pList->SelectItem(iSel, false);
			if( iSel >= 0 ) m_pList->EnsureVisible(iSel);
		}
	}

	LPCTSTR CLookupWnd::GetItemText(CControlUI* /*pList*/, int iIndex)
	{
		if( iIndex < 0 || iIndex >= (int)m_aVisible.size() || m_pOwner == NULL )
			return _T("");
		ILookupEditCallback* pCb = m_pOwner->GetCallback();
		if( pCb == NULL ) return _T("");
		LPCTSTR p = pCb->GetCellText(m_aVisible[iIndex], 0);
		return (p != NULL) ? p : _T("");
	}

	bool CLookupWnd::PaintItem(CControlUI* pList, IRenderContext& ctx, int iIndex, const RECT& rcItem, UINT uState)
	{
		if( pList == NULL || m_pOwner == NULL ) return false;
		CVirtualListUI* pVList = static_cast<CVirtualListUI*>(pList->GetInterface(DUI_CTR_VIRTUALLIST));
		if( pVList == NULL ) return false;
		(void)pVList;
		if( iIndex < 0 || iIndex >= (int)m_aVisible.size() ) return true;

		ILookupEditCallback* pCb = m_pOwner->GetCallback();
		const int nFull = m_aVisible[iIndex];
		const int nCols = (int)m_aColWidth.size();

		DWORD dwBk = 0;
		DWORD dwText = 0x333333FF;
		if( m_pOwner->GetManager() != NULL )
			dwText = m_pOwner->GetManager()->GetDefaultFontColor();

		const bool bSelected = (uState & UISTATE_SELECTED) != 0;
		const bool bHot = (uState & UISTATE_HOT) != 0;
		if( bSelected ) dwBk = 0xBAE0FFFF;
		else if( bHot ) dwBk = 0xE6F4FFFF;
		else if( (iIndex % 2) == 1 ) dwBk = 0xFAFAFAFF;

		if( dwBk != 0 )
			ctx.DrawColor(rcItem, pList->GetAdjustColor(dwBk));

		RECT rcLine = { rcItem.left, rcItem.bottom - 1, rcItem.right, rcItem.bottom };
		ctx.DrawColor(rcLine, pList->GetAdjustColor(0xF0F0F0FF));

		int x = rcItem.left;
		const int nPad = LookupScale(&m_pm, 6);
		for( int c = 0; c < nCols; ++c ) {
			int nW = LookupScale(&m_pm, m_aColWidth[c]);
			if( nW < 8 ) nW = 8;
			RECT rcCell = rcItem;
			rcCell.left = x;
			if( c == nCols - 1 ) rcCell.right = rcItem.right;
			else rcCell.right = x + nW;
			if( rcCell.right > rcItem.right ) rcCell.right = rcItem.right;

			if( c > 0 && rcCell.left < rcItem.right ) {
				RECT rcCol = { rcCell.left, rcItem.top, rcCell.left + 1, rcItem.bottom };
				ctx.DrawColor(rcCol, pList->GetAdjustColor(0xF0F0F0FF));
			}

			RECT rcText = rcCell;
			rcText.left += nPad;
			rcText.right -= nPad;
			if( rcText.right > rcText.left ) {
				LPCTSTR pCell = (pCb != NULL) ? pCb->GetCellText(nFull, c) : NULL;
				if( pCell != NULL && pCell[0] != _T('\0') ) {
					ctx.DrawText(rcText, pCell, pList->GetAdjustColor(dwText), -1,
						DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
				}
			}
			x = rcCell.right;
			if( x >= rcItem.right ) break;
		}
		return true;
	}

	void CLookupWnd::Notify(TNotifyUI& msg)
	{
		if( msg.sType == _T("windowinit") ) {
			RebuildVisible();
			if( !m_aFilters.empty() && m_aFilters[0] != NULL )
				m_pm.SetFocus(m_aFilters[0]);
		}
		else if( msg.sType == DUI_MSGTYPE_TEXTCHANGED ) {
			RebuildVisible();
		}
		else if( msg.sType == DUI_MSGTYPE_ITEMACTIVATE ) {
			if( msg.pSender != NULL && msg.pSender->GetInterface(DUI_CTR_VIRTUALLIST) != NULL )
				Confirm();
		}
		else if( msg.sType == DUI_MSGTYPE_CLICK && msg.pSender != NULL ) {
			CDuiString sName = msg.pSender->GetName();
			if( sName.CompareNoCase(_T("lookup_cancel")) == 0 )
				Cancel();
			else if( sName.CompareNoCase(_T("lookup_ok")) == 0 )
				Confirm();
		}
	}

	LRESULT CLookupWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
	{
		if( uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN ) {
			if( wParam == VK_ESCAPE || wParam == VK_RETURN ) {
				if( LookupImeComposing(::GetFocus()) ) {
					bHandled = false;
					return 0;
				}
				bHandled = true;
				if( wParam == VK_ESCAPE )
					Cancel();
				else
					Confirm();
				return 0;
			}
		}
		bHandled = false;
		return 0;
	}

	LRESULT CLookupWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if( uMsg == WM_CREATE ) {
			m_pm.SetForceUseSharedRes(true);
			m_pm.Init(m_hWnd);
			m_pm.SetLayered(false);

			CVerticalLayoutUI* pRoot = new CVerticalLayoutUI;
			pRoot->SetAttribute(_T("theme"), _T("chrome"));
			LookupApplyDefault(pRoot, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("VerticalLayout"));

			DWORD dropBg = 0xFFFFFFFF;
			DWORD dropBd = 0xC6C7D2FF;
			CThemeManager* tm = CThemeManager::GetInstance();
			if( tm != NULL ) {
				CTheme* th = tm->GetCurrentTheme();
				if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
				if( th != NULL ) {
					dropBg = th->GetToken(_T("color-control-bg"), th->GetToken(_T("color-bg"), dropBg));
					dropBd = th->GetToken(_T("color-control-border"), th->GetToken(_T("color-border"), dropBd));
				}
			}
			pRoot->SetBackgroundColor(dropBg);
			pRoot->SetBorderColor(dropBd);
			pRoot->SetBorderWidth(1);
			pRoot->SetPadding(CDuiBox(4, 4, 4, 4));
			pRoot->SetAttribute(_T("gap"), _T("2"));

			const int nCols = (int)m_aColWidth.size();

			CHorizontalLayoutUI* pHead = new CHorizontalLayoutUI;
			pHead->SetFixedHeight(26);
			LookupApplyDefault(pHead, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("HorizontalLayout"));

			CHorizontalLayoutUI* pFilter = new CHorizontalLayoutUI;
			pFilter->SetFixedHeight(28);
			LookupApplyDefault(pFilter, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("HorizontalLayout"));

			m_aFilters.clear();
			m_aFilters.reserve((size_t)nCols);
			for( int i = 0; i < nCols; ++i ) {
				CLabelUI* pLabel = new CLabelUI;
				LookupApplyDefault(pLabel, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("Label"));
				if( m_pOwner != NULL && i < m_pOwner->GetCount() ) {
					CLookupColumnUI* pCol = m_pOwner->GetColumn(i);
					if( pCol != NULL ) {
						CDuiString sTitle = pCol->GetText();
						pLabel->SetText(sTitle.GetData());
					}
				}
				pLabel->SetAttribute(_T("text-overflow"), _T("ellipsis"));
				pLabel->SetPadding(CDuiBox(0, 6, 0, 6));
				if( i < nCols - 1 )
					pLabel->SetFixedWidth(m_aColWidth[i]);
				pHead->Add(pLabel);

				CEditUI* pEdit = new CEditUI;
				LookupApplyDefault(pEdit, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("Edit"));
				if( i < nCols - 1 )
					pEdit->SetFixedWidth(m_aColWidth[i]);
				pEdit->SetPlaceholder(_T("筛选"));
				pEdit->SetMaxChar(64);
				pEdit->SetAttribute(_T("border"), _T("1px solid"));
				pEdit->SetAttribute(_T("padding"), _T("0,6,0,6"));
				pFilter->Add(pEdit);
				m_aFilters.push_back(pEdit);
			}

			m_pList = new CVirtualListUI;
			LookupApplyDefault(m_pList, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("VirtualList"));
			m_pList->SetCallback(this);
			if( m_pOwner != NULL )
				m_pList->SetItemHeight(m_pOwner->GetItemHeight());
			m_pList->SetAttribute(_T("overflow"), _T("auto"));
			m_pList->SetAttribute(_T("item-show-row-line"), _T("true"));
			m_pList->SetAttribute(_T("item-alternate-background"), _T("true"));

			pRoot->Add(pHead);
			pRoot->Add(pFilter);
			pRoot->Add(m_pList);

			CHorizontalLayoutUI* pBar = new CHorizontalLayoutUI;
			pBar->SetFixedHeight(30);
			LookupApplyDefault(pBar, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("HorizontalLayout"));
			pBar->SetAttribute(_T("gap"), _T("8"));
			pBar->SetAttribute(_T("align-items"), _T("vcenter"));
			CSpacerUI* pGap = new CSpacerUI;
			pBar->Add(pGap);

			CButtonUI* pCancel = new CButtonUI;
			LookupApplyDefault(pCancel, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("Button"));
			pCancel->SetName(_T("lookup_cancel"));
			pCancel->SetText(_T("取消"));
			pCancel->SetFixedWidth(72);
			pCancel->SetAttribute(_T("kind"), _T("default"));
			pBar->Add(pCancel);

			CButtonUI* pOk = new CButtonUI;
			LookupApplyDefault(pOk, m_pOwner != NULL ? m_pOwner->GetManager() : NULL, _T("Button"));
			pOk->SetName(_T("lookup_ok"));
			pOk->SetText(_T("确定"));
			pOk->SetFixedWidth(72);
			pOk->SetAttribute(_T("kind"), _T("primary"));
			pBar->Add(pOk);

			pRoot->Add(pBar);

			m_pm.AttachDialog(pRoot);
			m_pm.AddPreMessageFilter(this);
			m_pm.AddNotifier(this);
			return 0;
		}
		else if( uMsg == WM_KEYDOWN ) {
			if( wParam == VK_ESCAPE ) {
				if( !LookupImeComposing(::GetFocus()) )
					Cancel();
				return 0;
			}
			if( wParam == VK_RETURN ) {
				Confirm();
				return 0;
			}
		}

		LRESULT lRes = 0;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	IMPLEMENT_DUICONTROL(CLookupColumnUI)
	IMPLEMENT_DUICONTROL(CLookupEditUI)

	CLookupColumnUI::CLookupColumnUI()
	{
		SetVisible(false);
		SetFixedWidth(80);
	}

	LPCTSTR CLookupColumnUI::GetClass() const
	{
		return _T("LookupColumnUI");
	}

	LPVOID CLookupColumnUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_LOOKUPCOLUMN) == 0 ) return static_cast<CLookupColumnUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	int CLookupColumnUI::GetLogicWidth() const
	{
		return (m_cxyFixed.cx > 0) ? m_cxyFixed.cx : 80;
	}

	void CLookupColumnUI::SetLogicWidth(int nWidth)
	{
		SetFixedWidth(nWidth > 0 ? nWidth : 80);
	}

	CLookupEditUI::CLookupEditUI()
		: m_pWindow(NULL)
		, m_pCallback(NULL)
		, m_iCurSel(-1)
		, m_iPendingPick(-1)
		, m_dwColor(0)
		, m_dwDisabledColor(0)
		, m_iFont(-1)
		, m_uTextStyle(DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS)
		, m_dwPlaceholderColor(0x8A8A8AFF)
		, m_eDropPos(DropBottom)
		, m_uButtonState(0)
		, m_nItemHeight(28)
	{
		m_rcTextPadding.left = 6;
		m_rcTextPadding.top = 0;
		m_rcTextPadding.right = 6;
		m_rcTextPadding.bottom = 0;
		m_szDropBox.cx = 0;
		m_szDropBox.cy = 280;
		SetPadding(CDuiBox(0, 28, 0, 8));
		SetCursor(DUI_HAND);
	}

	LPCTSTR CLookupEditUI::GetClass() const
	{
		return _T("LookupEditUI");
	}

	LPVOID CLookupEditUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_LOOKUPEDIT) == 0 ) return static_cast<CLookupEditUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CLookupEditUI::GetControlFlags() const
	{
		UINT f = UIFLAG_TABSTOP;
		if( IsEnabled() ) f |= UIFLAG_SETCURSOR;
		return f;
	}

	void CLookupEditUI::SetCallback(ILookupEditCallback* pCallback)
	{
		m_pCallback = pCallback;
	}

	ILookupEditCallback* CLookupEditUI::GetCallback() const
	{
		return m_pCallback;
	}

	int CLookupEditUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	void CLookupEditUI::SetCurSel(int iIndex)
	{
		if( iIndex == m_iCurSel ) return;
		m_iCurSel = iIndex;
		Invalidate();
	}

	SIZE CLookupEditUI::GetDropBoxSize() const
	{
		return m_szDropBox;
	}

	void CLookupEditUI::SetDropBoxSize(SIZE szDropBox)
	{
		m_szDropBox = szDropBox;
	}

	CLookupEditUI::DropPosition CLookupEditUI::GetDropPosition() const
	{
		return m_eDropPos;
	}

	void CLookupEditUI::SetDropPosition(DropPosition ePos)
	{
		m_eDropPos = ePos;
	}

	int CLookupEditUI::GetItemHeight() const
	{
		return m_nItemHeight;
	}

	void CLookupEditUI::SetItemHeight(int nHeight)
	{
		if( nHeight < 16 ) nHeight = 16;
		m_nItemHeight = nHeight;
	}

	void CLookupEditUI::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
		Invalidate();
	}

	DWORD CLookupEditUI::GetColor() const
	{
		return m_dwColor;
	}

	void CLookupEditUI::SetDisabledColor(DWORD dwColor)
	{
		m_dwDisabledColor = dwColor;
		Invalidate();
	}

	DWORD CLookupEditUI::GetDisabledColor() const
	{
		return m_dwDisabledColor;
	}

	void CLookupEditUI::SetFont(int index)
	{
		m_iFont = index;
		Invalidate();
	}

	int CLookupEditUI::GetFont() const
	{
		return m_iFont;
	}

	void CLookupEditUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	UINT CLookupEditUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	RECT CLookupEditUI::GetTextPadding() const
	{
		RECT rc = m_rcTextPadding;
		if( m_pManager != NULL ) m_pManager->GetDPIObj()->Scale(&rc);
		return rc;
	}

	void CLookupEditUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	void CLookupEditUI::SetPlaceholder(LPCTSTR pstrText)
	{
		m_sPlaceholder = (pstrText != NULL) ? pstrText : _T("");
		Invalidate();
	}

	LPCTSTR CLookupEditUI::GetPlaceholder() const
	{
		return m_sPlaceholder.GetData();
	}

	void CLookupEditUI::SetPlaceholderColor(DWORD dwColor)
	{
		m_dwPlaceholderColor = dwColor;
		Invalidate();
	}

	DWORD CLookupEditUI::GetPlaceholderColor() const
	{
		return m_dwPlaceholderColor;
	}

	CLookupColumnUI* CLookupEditUI::GetColumn(int iIndex) const
	{
		CControlUI* p = GetItemAt(iIndex);
		if( p == NULL ) return NULL;
		return static_cast<CLookupColumnUI*>(p->GetInterface(DUI_CTR_LOOKUPCOLUMN));
	}

	int CLookupEditUI::GetColumnCount() const
	{
		return GetCount();
	}

	void CLookupEditUI::AddColumn(LPCTSTR pstrName, LPCTSTR pstrText, int nWidth)
	{
		CLookupColumnUI* pCol = new CLookupColumnUI;
		if( pstrName != NULL ) pCol->SetName(pstrName);
		if( pstrText != NULL ) pCol->SetText(pstrText);
		pCol->SetLogicWidth(nWidth);
		Add(pCol);
	}

	bool CLookupEditUI::Add(CControlUI* pControl)
	{
		if( pControl == NULL ) return false;
		if( pControl->GetInterface(DUI_CTR_LOOKUPCOLUMN) == NULL ) return false;
		pControl->SetVisible(false);
		return CContainerUI::Add(pControl);
	}

	bool CLookupEditUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( pControl == NULL ) return false;
		if( pControl->GetInterface(DUI_CTR_LOOKUPCOLUMN) == NULL ) return false;
		pControl->SetVisible(false);
		return CContainerUI::AddAt(pControl, iIndex);
	}

	bool CLookupEditUI::Activate()
	{
		if( !CControlUI::Activate() ) return false;
		if( m_pWindow != NULL ) return true;
		if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_DROPDOWN);

		m_iPendingPick = -1;
		CLookupWnd* pWnd = new CLookupWnd();
		m_pWindow = pWnd;
		pWnd->Init(this);
		if( pWnd->GetHWND() == NULL ) {
			m_pWindow = NULL;
			delete pWnd;
			return false;
		}
		pWnd->ShowModal();

		if( m_iPendingPick >= 0 ) {
			const int iOld = m_iCurSel;
			m_iCurSel = m_iPendingPick;
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, (WPARAM)m_iCurSel, (LPARAM)iOld);
			Invalidate();
		}
		return true;
	}

	SIZE CLookupEditUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 && m_pManager != NULL && m_pManager->GetDefaultFontInfo() != NULL )
			return CDuiSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 12);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CLookupEditUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		RECT rcNull = { 0 };
		for( int i = 0; i < m_items.GetSize(); ++i )
			static_cast<CControlUI*>(m_items[i])->SetPos(rcNull);
	}

	void CLookupEditUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CControlUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR ) {
			CControlUI::DoEvent(event);
			return;
		}
		if( event.Type == UIEVENT_SETFOCUS || event.Type == UIEVENT_KILLFOCUS ) {
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			if( IsEnabled() ) {
				Activate();
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) {
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~UISTATE_CAPTURED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
			return;
		if( event.Type == UIEVENT_KEYDOWN ) {
			if( !IsKeyboardEnabled() ) return;
			switch( event.chKey ) {
			case VK_F4:
			case VK_DOWN:
			case VK_SPACE:
			case VK_RETURN:
				Activate();
				return;
			default:
				break;
			}
		}
		if( event.Type == UIEVENT_MOUSEENTER ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) {
				if( (m_uButtonState & UISTATE_HOT) == 0 )
					m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( (m_uButtonState & UISTATE_HOT) != 0 ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CLookupEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("color")) == 0 ) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-disabled")) == 0 ) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetDisabledColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("text-padding")) == 0 ) {
			RECT rc = { 0 };
			if( ParseCssBoxToRect(pstrValue, rc) )
				SetTextPadding(rc);
		}
		else if( _tcsicmp(pstrName, _T("placeholder")) == 0 ) SetPlaceholder(pstrValue);
		else if( _tcsicmp(pstrName, _T("placeholder-color")) == 0 ) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetPlaceholderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("drop-box-size")) == 0 ) {
			SIZE sz = { 0 };
			LPTSTR pstr = NULL;
			sz.cx = _tcstol(pstrValue, &pstr, 10);
			if( pstr != NULL )
				sz.cy = _tcstol(pstr + 1, &pstr, 10);
			SetDropBoxSize(sz);
		}
		else if( _tcsicmp(pstrName, _T("drop-position")) == 0
			|| _tcsicmp(pstrName, _T("popup-position")) == 0
			|| _tcsicmp(pstrName, _T("placement")) == 0 ) {
			DropPosition ePos = DropBottom;
			if( LookupParseDropPosition(pstrValue, ePos) )
				SetDropPosition(ePos);
		}
		else if( _tcsicmp(pstrName, _T("item-height")) == 0 ) {
			SetItemHeight(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("text-align")) == 0
			|| _tcsicmp(pstrName, _T("align")) == 0 ) {
			UINT uAlign = DT_LEFT;
			if( LookupParseTextAlign(pstrValue, uAlign) ) {
				m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_RIGHT);
				m_uTextStyle |= uAlign;
				Invalidate();
			}
		}
		else if( _tcsicmp(pstrName, _T("text-overflow")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("ellipsis")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
			else m_uTextStyle &= ~DT_END_ELLIPSIS;
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	bool CLookupEditUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		return CControlUI::DoPaint(ctx, rcPaint, pStopControl);
	}

	void CLookupEditUI::PaintStatusImage(IRenderContext& ctx)
	{
		RECT rcPad = GetPadding();
		RECT rcArrow = m_rcItem;
		rcArrow.left = rcArrow.right - rcPad.right;
		if( rcArrow.left < m_rcItem.left ) rcArrow.left = m_rcItem.left;
		DWORD dwColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		if( dwColor == 0 && m_pManager != NULL )
			dwColor = IsEnabled() ? m_pManager->GetDefaultFontColor() : m_pManager->GetDefaultDisabledColor();
		ctx.DrawText(rcArrow, _T("▾"), GetAdjustColor(dwColor), m_iFont,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	void CLookupEditUI::PaintText(IRenderContext& ctx)
	{
		if( m_pManager != NULL ) {
			if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
			if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();
		}

		RECT rc = m_rcItem;
		RECT rcPad = GetPadding();
		RECT rcTextPad = GetTextPadding();
		rc.left += rcPad.left + rcTextPad.left;
		rc.right -= rcPad.right + rcTextPad.right;
		rc.top += rcPad.top + rcTextPad.top;
		rc.bottom -= rcPad.bottom + rcTextPad.bottom;

		CDuiString sText = GetText();
		DWORD clrColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		LPCTSTR pDraw = sText.GetData();
		if( sText.IsEmpty() ) {
			pDraw = m_sPlaceholder.GetData();
			clrColor = m_dwPlaceholderColor;
			if( pDraw == NULL || pDraw[0] == _T('\0') ) return;
		}
		ctx.DrawText(rc, pDraw, GetAdjustColor(clrColor), m_iFont, m_uTextStyle);
	}

} // namespace DuiLib
