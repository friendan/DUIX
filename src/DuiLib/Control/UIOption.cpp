#include "StdAfx.h"
#include "UIOption.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(COptionUI)
	COptionUI::COptionUI() : m_bSelected(false) ,m_iSelectedFont(-1), m_dwSelectedTextColor(0), m_dwSelectedBkColor(0), m_nSelectedStateCount(0)
	{
	}

	COptionUI::~COptionUI()
	{
		if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
	}

	LPCTSTR COptionUI::GetClass() const
	{
		return _T("OptionUI");
	}

	LPVOID COptionUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_OPTION) == 0 ) return static_cast<COptionUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void COptionUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( bInit && !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
	}

	LPCTSTR COptionUI::GetGroup() const
	{
		return m_sGroupName;
	}

	void COptionUI::SetGroup(LPCTSTR pStrGroupName)
	{
		if( pStrGroupName == NULL ) {
			if( m_sGroupName.IsEmpty() ) return;
			m_sGroupName.Empty();
		}
		else {
			if( m_sGroupName == pStrGroupName ) return;
			if (!m_sGroupName.IsEmpty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
			m_sGroupName = pStrGroupName;
		}

		if( !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
		else {
			if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
		}

		Selected(m_bSelected);
	}

	LPCTSTR COptionUI::GetGroupType() const
	{
		return m_sGroupType;
	}

	void COptionUI::SetGroupType(LPCTSTR pStrGroupType)
	{
		if( pStrGroupType == NULL ) {
			if( m_sGroupType.IsEmpty() ) return;
			m_sGroupType.Empty();
		}
		else {
			m_sGroupType = pStrGroupType;
		}
	}

	bool COptionUI::IsSelected() const
	{
		return m_bSelected;
	}

	void COptionUI::Selected(bool bSelected, bool bMsg/* = true*/)
	{
		if(m_bSelected == bSelected) return;

		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.IsEmpty() ) {
				if( m_bSelected ) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this && (m_sGroupType.IsEmpty() || m_sGroupType.CompareNoCase(pControl->GetGroupType()) != 0)) {
							pControl->Selected(false, bMsg);
						}
					}
					if(bMsg) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
					}
				}
				else {
					if(bMsg) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
					}
				}
			}
			else {
				if(bMsg) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
				}
			}
		}

		Invalidate();
	}

	bool COptionUI::Activate()
	{
		if( !CButtonUI::Activate() ) return false;
		if( !m_sGroupName.IsEmpty() ) {
			Selected(true);
		}
		else Selected(!m_bSelected);
		
		Invalidate();
		return true;
	}

	LPCTSTR COptionUI::GetSelectedImage()
	{
		return m_sSelectedImage;
	}

	void COptionUI::SetSelectedImage(LPCTSTR pStrImage)
	{
		m_sSelectedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR COptionUI::GetSelectedHotImage()
	{
		return m_sSelectedHotImage;
	}

	void COptionUI::SetSelectedHotImage( LPCTSTR pStrImage )
	{
		m_sSelectedHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR COptionUI::GetSelectedPushedImage()
	{
		return m_sSelectedPushedImage;
	}

	void COptionUI::SetSelectedPushedImage(LPCTSTR pStrImage)
	{
		m_sSelectedPushedImage = pStrImage;
		Invalidate();
	}

	void COptionUI::SetSelectedTextColor(DWORD dwTextColor)
	{
		m_dwSelectedTextColor = dwTextColor;
	}

	DWORD COptionUI::GetSelectedTextColor()
	{
		if (m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
		return m_dwSelectedTextColor;
	}

	void COptionUI::SetSelectedBkColor( DWORD dwBkColor )
	{
		m_dwSelectedBkColor = dwBkColor;
	}

	DWORD COptionUI::GetSelectBkColor()
	{
		return m_dwSelectedBkColor;
	}

	LPCTSTR COptionUI::GetSelectedForedImage()
	{
		return m_sSelectedForeImage;
	}

	void COptionUI::SetSelectedForedImage(LPCTSTR pStrImage)
	{
		m_sSelectedForeImage = pStrImage;
		Invalidate();
	}

	void COptionUI::SetSelectedStateCount(int nCount)
	{
		m_nSelectedStateCount = nCount;
		Invalidate();
	}

	int COptionUI::GetSelectedStateCount() const
	{
		return m_nSelectedStateCount;
	}

	LPCTSTR COptionUI::GetSelectedStateImage()
	{
		return m_sSelectedStateImage;
	}

	void COptionUI::SetSelectedStateImage( LPCTSTR pStrImage )
	{
		m_sSelectedStateImage = pStrImage;
		Invalidate();
	}
	void COptionUI::SetSelectedFont(int index)
	{
		m_iSelectedFont = index;
		Invalidate();
	}

	int COptionUI::GetSelectedFont() const
	{
		return m_iSelectedFont;
	}
	void COptionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("group")) == 0 ) SetGroup(pstrValue);
		else if( _tcsicmp(pstrName, _T("grouptype")) == 0 ) SetGroupType(pstrValue);
		else if( _tcsicmp(pstrName, _T("selected")) == 0 ) Selected(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("selectedimage")) == 0 ) SetSelectedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("selectedhotimage")) == 0 ) SetSelectedHotImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("selectedpushedimage")) == 0 ) SetSelectedPushedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("selectedforeimage")) == 0 ) SetSelectedForedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("selectedstateimage")) == 0 ) SetSelectedStateImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("selectedstatecount")) == 0 ) SetSelectedStateCount(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("selectedbkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedBkColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("selectedtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedTextColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("selectedfont")) == 0 ) SetSelectedFont(_ttoi(pstrValue));
		else CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	void COptionUI::PaintBkColor(IRenderContext& ctx)
	{
		if(IsSelected()) {
			if(m_dwSelectedBkColor != 0) {
				ctx.DrawColor(m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
			}
		}
		else {
			return CButtonUI::PaintBkColor(ctx);
		}
	}

	void COptionUI::PaintStatusImage(IRenderContext& ctx)
	{
		if(IsSelected()) {
			if(!m_sSelectedStateImage.IsEmpty() && m_nSelectedStateCount > 0)
			{
				TDrawInfo info;
				info.Parse(m_sSelectedStateImage, _T(""), m_pManager);
				const TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName, info.sResType, info.dwMask, info.bHSL, info.bGdiplus);
				if(m_sSelectedImage.IsEmpty() && pImage != NULL)
				{
					SIZE szImage = {pImage->nX, pImage->nY};
					SIZE szStatus = {pImage->nX / m_nSelectedStateCount, pImage->nY};
					if( szImage.cx > 0 && szImage.cy > 0 )
					{
						RECT rcSrc = {0, 0, szImage.cx, szImage.cy};
						if(m_nSelectedStateCount > 0) {
							int iLeft = rcSrc.left + 0 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sSelectedImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d' corner='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom, info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
						if(m_nSelectedStateCount > 1) {
							int iLeft = rcSrc.left + 1 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sSelectedHotImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d' corner='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom, info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
							m_sSelectedPushedImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d' corner='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom, info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
						if(m_nSelectedStateCount > 2) {
							int iLeft = rcSrc.left + 2 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sSelectedPushedImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d' corner='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom, info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
					}
				}
			}

			if( (m_uButtonState & UISTATE_PUSHED) != 0 && !m_sSelectedPushedImage.IsEmpty()) {
				if( !DrawImage(ctx, (LPCTSTR)m_sSelectedPushedImage) ) {}
				else return;
			}
			else if( (m_uButtonState & UISTATE_HOT) != 0 && !m_sSelectedHotImage.IsEmpty()) {
				if( !DrawImage(ctx, (LPCTSTR)m_sSelectedHotImage) ) {}
				else return;
			}

			if( !m_sSelectedImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sSelectedImage) ) {}
			}
		}
		else {
			CButtonUI::PaintStatusImage(ctx);
		}
	}

	void COptionUI::PaintForeImage(IRenderContext& ctx)
	{
		if(IsSelected()) {
			if( !m_sSelectedForeImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sSelectedForeImage) ) {}
				else return;
			}
		}

		return CButtonUI::PaintForeImage(ctx);
	}

	void COptionUI::PaintText(IRenderContext& ctx)
	{
		if( (m_uButtonState & UISTATE_SELECTED) != 0 )
		{
			DWORD oldTextColor = m_dwTextColor;
			if( m_dwSelectedTextColor != 0 ) m_dwTextColor = m_dwSelectedTextColor;

			if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
			if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

			int iFont = GetFont();
			if(GetSelectedFont() != -1) {
				iFont = GetSelectedFont();
			}
			CDuiString sText = GetText();
			if( sText.IsEmpty() ) return;
			int nLinks = 0;
			RECT rc = m_rcItem;
			RECT rcInset = GetInset();
			RECT rcTextPadding = GetTextPadding();
			rc.left += rcInset.left + rcTextPadding.left;
			rc.right -= rcInset.right + rcTextPadding.right;
			rc.top += rcInset.top + rcTextPadding.top;
			rc.bottom -= rcInset.bottom + rcTextPadding.bottom;

			DWORD clrColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;
			
			if( m_bShowHtml )
				ctx.DrawHtmlText(rc, sText, clrColor, NULL, NULL, nLinks, iFont, m_uTextStyle);
			else
				ctx.DrawText(rc, sText, clrColor, iFont, m_uTextStyle);

			m_dwTextColor = oldTextColor;
		}
		else
			CButtonUI::PaintText(ctx);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	IMPLEMENT_DUICONTROL(CCheckBoxUI)

	CCheckBoxUI::CCheckBoxUI()
		: m_bAutoCheck(false)
		, m_nBoxGap(6)
		, m_nBoxBorderSize(1)
		, m_dwBoxBkColor(0xFFFFFFFF)
		, m_dwBoxBorderColor(0xFFBFBFBF)
		, m_dwBoxHotBkColor(0xFFFFFFFF)
		, m_dwBoxHotBorderColor(0xFF1677FF)
		, m_dwSelectedBoxBkColor(0xFF1677FF)
		, m_dwSelectedBoxBorderColor(0xFF1677FF)
		, m_dwCheckMarkColor(0xFFFFFFFF)
		, m_dwDisabledBoxBkColor(0xFFF5F5F5)
		, m_dwDisabledBoxBorderColor(0xFFD9D9D9)
	{
		m_szBox.cx = m_szBox.cy = 16;
		m_szBoxRound.cx = m_szBoxRound.cy = 3;
		// Button 默认 kind=default 会带按钮色，这里清掉并改为内置勾选框样式
		SetKind(CONTROLKIND_NONE);
		SetBkColor(0);
		SetHotBkColor(0);
		SetPushedBkColor(0);
		SetDisabledBkColor(0);
		SetHotTextColor(0);
		SetPushedTextColor(0);
		SetHotBorderColor(0);
		SetPushedBorderColor(0);
		SetBorderSize(0);
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_LEFT;
		m_dwTextColor = 0xFF333333;
		m_dwSelectedTextColor = 0; // 原生样式下标签色不随选中变化
		SetCursor(DUI_HAND);
		SetAutoCalcWidth(true);
	}

	LPCTSTR CCheckBoxUI::GetClass() const
	{
		return _T("CheckBoxUI");
	}

	LPVOID CCheckBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_CHECKBOX) == 0 ) return static_cast<CCheckBoxUI*>(this);
		return COptionUI::GetInterface(pstrName);
	}

	void CCheckBoxUI::SetCheck(bool bCheck)
	{
		Selected(bCheck);
	}

	bool CCheckBoxUI::GetCheck() const
	{
		return IsSelected();
	}

	void CCheckBoxUI::SetAutoCheck(bool bEnable)
	{
		m_bAutoCheck = bEnable;
	}

	void CCheckBoxUI::SetBoxSize(SIZE sz)
	{
		m_szBox = sz;
		m_bNeedEstimateSize = true;
		NeedParentUpdate();
		Invalidate();
	}

	SIZE CCheckBoxUI::GetBoxSize() const
	{
		SIZE sz = m_szBox;
		if( m_pManager != NULL ) m_pManager->GetDPIObj()->Scale(&sz);
		return sz;
	}

	void CCheckBoxUI::SetBoxGap(int nGap)
	{
		m_nBoxGap = nGap;
		m_bNeedEstimateSize = true;
		NeedParentUpdate();
		Invalidate();
	}

	int CCheckBoxUI::GetBoxGap() const
	{
		return (m_pManager != NULL) ? m_pManager->GetDPIObj()->Scale(m_nBoxGap) : m_nBoxGap;
	}

	bool CCheckBoxUI::IsNativeCheckStyle() const
	{
		return m_sNormalImage.IsEmpty() && m_sSelectedImage.IsEmpty()
			&& m_sSelectedStateImage.IsEmpty() && m_sStateImage.IsEmpty();
	}

	RECT CCheckBoxUI::GetCheckBoxRect() const
	{
		SIZE sz = GetBoxSize();
		RECT rc = m_rcItem;
		rc.top = m_rcItem.top + (m_rcItem.bottom - m_rcItem.top - sz.cy) / 2;
		if( rc.top < m_rcItem.top ) rc.top = m_rcItem.top;
		rc.bottom = rc.top + sz.cy;
		rc.left = m_rcItem.left;
		rc.right = rc.left + sz.cx;
		return rc;
	}

	void CCheckBoxUI::PaintNativeCheckBox(IRenderContext& ctx)
	{
		RECT rcBox = GetCheckBoxRect();
		DWORD dwBk = m_dwBoxBkColor;
		DWORD dwBorder = m_dwBoxBorderColor;
		bool bSelected = IsSelected();
		bool bHot = (m_uButtonState & UISTATE_HOT) != 0;
		bool bPushed = (m_uButtonState & UISTATE_PUSHED) != 0;

		if( !IsEnabled() ) {
			dwBk = m_dwDisabledBoxBkColor;
			dwBorder = m_dwDisabledBoxBorderColor;
		}
		else if( bSelected ) {
			dwBk = m_dwSelectedBoxBkColor;
			dwBorder = m_dwSelectedBoxBorderColor;
			if( bHot || bPushed ) {
				// 选中悬停略加深
				dwBk = 0xFF0958D9;
				dwBorder = dwBk;
			}
		}
		else if( bHot || bPushed ) {
			dwBk = m_dwBoxHotBkColor;
			dwBorder = m_dwBoxHotBorderColor;
		}

		SIZE szRound = m_szBoxRound;
		if( m_pManager != NULL ) m_pManager->GetDPIObj()->Scale(&szRound);
		int nBorder = m_nBoxBorderSize;
		if( m_pManager != NULL ) nBorder = m_pManager->GetDPIObj()->Scale(nBorder);
		if( nBorder < 1 ) nBorder = 1;

		if( dwBk != 0 ) {
			if( szRound.cx > 0 || szRound.cy > 0 )
				ctx.FillRoundRect(rcBox, szRound.cx, szRound.cy, GetAdjustColor(dwBk));
			else
				ctx.DrawColor(rcBox, GetAdjustColor(dwBk));
		}
		if( dwBorder != 0 && nBorder > 0 ) {
			if( szRound.cx > 0 || szRound.cy > 0 )
				ctx.DrawRoundRect(rcBox, nBorder, szRound.cx, szRound.cy, GetAdjustColor(dwBorder));
			else
				ctx.DrawRect(rcBox, nBorder, GetAdjustColor(dwBorder));
		}

		if( !bSelected ) return;

		DWORD dwMark = IsEnabled() ? m_dwCheckMarkColor : 0xFFBFBFBF;
		int w = rcBox.right - rcBox.left;
		int h = rcBox.bottom - rcBox.top;
		int stroke = (m_pManager != NULL) ? m_pManager->GetDPIObj()->Scale(2) : 2;
		if( stroke < 1 ) stroke = 1;

		// ✓：两段折线
		RECT rc1 = {
			rcBox.left + w * 22 / 100,
			rcBox.top + h * 50 / 100,
			rcBox.left + w * 42 / 100,
			rcBox.top + h * 70 / 100
		};
		RECT rc2 = {
			rcBox.left + w * 42 / 100,
			rcBox.top + h * 70 / 100,
			rcBox.left + w * 78 / 100,
			rcBox.top + h * 28 / 100
		};
		ctx.DrawLine(rc1, stroke, GetAdjustColor(dwMark));
		ctx.DrawLine(rc2, stroke, GetAdjustColor(dwMark));
	}

	SIZE CCheckBoxUI::EstimateSize(SIZE szAvailable)
	{
		if( !IsNativeCheckStyle() )
			return COptionUI::EstimateSize(szAvailable);

		SIZE szBox = GetBoxSize();
		int nGap = GetBoxGap();
		CDuiString sText = GetText();

		SIZE sz = GetFixedSize();
		if( sz.cx > 0 && sz.cy > 0 ) return sz;

		if( sz.cy == 0 ) {
			int nFontH = 16;
			if( m_pManager != NULL && m_pManager->GetFontInfo(GetFont()) != NULL )
				nFontH = m_pManager->GetFontInfo(GetFont())->tm.tmHeight;
			sz.cy = (szBox.cy > nFontH) ? szBox.cy : nFontH;
		}

		if( sz.cx == 0 ) {
			sz.cx = szBox.cx;
			if( !sText.IsEmpty() ) {
				RECT rcText = { 0, 0, 9999, sz.cy };
				UINT uStyle = DT_CALCRECT | DT_SINGLELINE | DT_LEFT | DT_VCENTER;
				if( m_bShowHtml )
					RenderMeasureHtmlText(m_pManager, rcText, sText, 0, GetFont(), uStyle);
				else
					RenderMeasureText(m_pManager, rcText, sText, 0, GetFont(), uStyle);
				sz.cx += nGap + (rcText.right - rcText.left);
				RECT rcPad = GetTextPadding();
				sz.cx += rcPad.left + rcPad.right;
			}
		}
		return sz;
	}

	void CCheckBoxUI::PaintBkColor(IRenderContext& ctx)
	{
		if( IsNativeCheckStyle() ) return; // 背景只画在方框上
		COptionUI::PaintBkColor(ctx);
	}

	void CCheckBoxUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( IsNativeCheckStyle() ) {
			PaintNativeCheckBox(ctx);
			return;
		}
		COptionUI::PaintStatusImage(ctx);
	}

	void CCheckBoxUI::PaintBorder(IRenderContext& ctx)
	{
		if( IsNativeCheckStyle() ) return;
		COptionUI::PaintBorder(ctx);
	}

	void CCheckBoxUI::PaintText(IRenderContext& ctx)
	{
		if( !IsNativeCheckStyle() ) {
			COptionUI::PaintText(ctx);
			return;
		}

		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		SIZE szBox = GetBoxSize();
		int nGap = GetBoxGap();
		RECT rcInset = GetInset();
		RECT rcPad = GetTextPadding();
		RECT rc = m_rcItem;
		rc.left += rcInset.left + szBox.cx + nGap + rcPad.left;
		rc.right -= rcInset.right + rcPad.right;
		rc.top += rcInset.top + rcPad.top;
		rc.bottom -= rcInset.bottom + rcPad.bottom;

		DWORD clrColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;
		int nLinks = 0;
		UINT uStyle = m_uTextStyle;
		if( (uStyle & (DT_CENTER | DT_RIGHT)) == 0 )
			uStyle |= DT_LEFT;
		if( m_bShowHtml )
			ctx.DrawHtmlText(rc, sText, clrColor, NULL, NULL, nLinks, GetFont(), uStyle);
		else
			ctx.DrawText(rc, sText, clrColor, GetFont(), uStyle);
	}

	static bool ParseCheckColorAttr(LPCTSTR pstrValue, DWORD& dwColor)
	{
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return false;
		if( *pstrValue == _T('#') ) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		dwColor = _tcstoul(pstrValue, &pstr, 16);
		return true;
	}

	void CCheckBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("EnableAutoCheck")) == 0 || _tcsicmp(pstrName, _T("autocheck")) == 0 ) {
			SetAutoCheck(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("boxsize")) == 0 ) {
			SIZE sz = { 16, 16 };
			LPTSTR pstr = NULL;
			sz.cx = _tcstol(pstrValue, &pstr, 10);
			sz.cy = sz.cx;
			if( pstr && *pstr == _T(',') ) sz.cy = _tcstol(pstr + 1, &pstr, 10);
			SetBoxSize(sz);
		}
		else if( _tcsicmp(pstrName, _T("boxgap")) == 0 ) {
			SetBoxGap(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("boxbordersize")) == 0 ) {
			m_nBoxBorderSize = _ttoi(pstrValue);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("boxborderround")) == 0 ) {
			LPTSTR pstr = NULL;
			m_szBoxRound.cx = _tcstol(pstrValue, &pstr, 10);
			m_szBoxRound.cy = m_szBoxRound.cx;
			if( pstr && *pstr == _T(',') ) m_szBoxRound.cy = _tcstol(pstr + 1, &pstr, 10);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("boxbkcolor")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxBkColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("boxbordercolor")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("boxhotbkcolor")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxHotBkColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("boxhotbordercolor")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxHotBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("selectedboxbkcolor")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwSelectedBoxBkColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("selectedboxbordercolor")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwSelectedBoxBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("checkmarkcolor")) == 0 || _tcsicmp(pstrName, _T("checkcolor")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwCheckMarkColor); Invalidate();
		}
		else {
			COptionUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CCheckBoxUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) COptionUI::DoEvent(event);
			return;
		}

		if( m_bAutoCheck && (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				SetCheck(!GetCheck());
				m_pManager->SendNotify(this, DUI_MSGTYPE_CHECKCLICK, 0, 0);
				Invalidate();
			}
			return;
		}
		COptionUI::DoEvent(event);
	}

	void CCheckBoxUI::Selected(bool bSelected, bool bMsg/* = true*/)
	{
		if( m_bSelected == bSelected ) return;

		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.IsEmpty() ) {
				if( m_bSelected ) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this ) {
							pControl->Selected(false, bMsg);
						}
					}
					if(bMsg) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED, m_bSelected, 0);
					}
				}
			}
			else {
				if(bMsg) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED, m_bSelected, 0);
				}
			}
		}

		Invalidate();
	}
}