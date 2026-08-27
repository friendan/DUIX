#include "StdAfx.h"
#include "UIOption.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(COptionUI)
	COptionUI::COptionUI() : m_bSelected(false) ,m_iSelectedFont(-1), m_dwSelectedBackgroundColor(0), m_dwSelectedColor(0), m_nSelectedStateCount(0)
	{
		// 左右内边距：左对齐文字不贴边；皮肤可用 padding="0" 关掉
		SetPadding(CDuiBox(0, 12, 0, 12));
	}

	COptionUI::~COptionUI()
	{
		if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName.GetData(), this);
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
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName.GetData(), this);
		}
	}

	LPCTSTR COptionUI::GetGroup() const
	{
		return m_sGroupName.GetData();
	}

	void COptionUI::SetGroup(LPCTSTR pStrGroupName)
	{
		if( pStrGroupName == NULL ) {
			if( m_sGroupName.IsEmpty() ) return;
			m_sGroupName.Empty();
		}
		else {
			if( m_sGroupName == pStrGroupName ) return;
			if (!m_sGroupName.IsEmpty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName.GetData(), this);
			m_sGroupName = pStrGroupName;
		}

		if( !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName.GetData(), this);
		}
		else {
			if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName.GetData(), this);
		}

		Selected(m_bSelected);
	}

	LPCTSTR COptionUI::GetGroupType() const
	{
		return m_sGroupType.GetData();
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
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName.GetData());
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
		return m_sSelectedImage.GetData();
	}

	void COptionUI::SetSelectedImage(LPCTSTR pStrImage)
	{
		m_sSelectedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR COptionUI::GetSelectedHoverImage()
	{
		return m_sSelectedHoverImage.GetData();
	}

	void COptionUI::SetSelectedHoverImage( LPCTSTR pStrImage )
	{
		m_sSelectedHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR COptionUI::GetSelectedActiveImage()
	{
		return m_sSelectedActiveImage.GetData();
	}

	void COptionUI::SetSelectedActiveImage(LPCTSTR pStrImage)
	{
		m_sSelectedActiveImage = pStrImage;
		Invalidate();
	}

	void COptionUI::SetSelectedColor(DWORD dwColor)
	{
		m_dwSelectedColor = dwColor;
	}

	DWORD COptionUI::GetSelectedColor()
	{
		if (m_dwSelectedColor == 0) m_dwSelectedColor = m_pManager->GetDefaultFontColor();
		return m_dwSelectedColor;
	}

	void COptionUI::SetSelectedBackgroundColor( DWORD dwBackgroundColor )
	{
		m_dwSelectedBackgroundColor = dwBackgroundColor;
	}

	DWORD COptionUI::GetSelectedBackgroundColor()
	{
		return m_dwSelectedBackgroundColor;
	}

	LPCTSTR COptionUI::GetSelectedForegroundImage()
	{
		return m_sSelectedForegroundImage.GetData();
	}

	void COptionUI::SetSelectedForegroundImage(LPCTSTR pStrImage)
	{
		m_sSelectedForegroundImage = pStrImage;
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
		return m_sSelectedStateImage.GetData();
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
		else if( _tcsicmp(pstrName, _T("group-type")) == 0 ) SetGroupType(pstrValue);
		else if( _tcsicmp(pstrName, _T("selected")) == 0 || _tcsicmp(pstrName, _T("checked")) == 0 )
			Selected(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("image-selected")) == 0 ) SetSelectedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-selected-hover")) == 0 ) SetSelectedHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-selected-active")) == 0 ) SetSelectedActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("foreground-image-selected")) == 0 ) SetSelectedForegroundImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("selected-state-image")) == 0 ) SetSelectedStateImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("selected-state-count")) == 0 ) SetSelectedStateCount(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("background-color-selected")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetSelectedBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-selected")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetSelectedColor(clrColor);
		}
		else CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	void COptionUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		if(IsSelected()) {
			if(m_dwSelectedBackgroundColor != 0) {
				ctx.DrawColor(m_rcPaint, GetAdjustColor(m_dwSelectedBackgroundColor));
			}
		}
		else {
			return CButtonUI::PaintBackgroundColor(ctx);
		}
	}

	void COptionUI::PaintStatusImage(IRenderContext& ctx)
	{
		if(IsSelected()) {
			if(!m_sSelectedStateImage.IsEmpty() && m_nSelectedStateCount > 0)
			{
				TDrawInfo info;
				info.Parse(m_sSelectedStateImage.GetData(), _T(""), m_pManager);
				const TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName.GetData(), info.sResType.GetData(), info.dwMask, info.bHSL, info.bGdiplus);
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
							m_sSelectedHoverImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d' corner='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom, info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
							m_sSelectedActiveImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d' corner='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom, info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
						if(m_nSelectedStateCount > 2) {
							int iLeft = rcSrc.left + 2 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sSelectedActiveImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d' corner='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom, info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
					}
				}
			}

			if( (m_uButtonState & UISTATE_PUSHED) != 0 && !m_sSelectedActiveImage.IsEmpty()) {
				if( !DrawImage(ctx, m_sSelectedActiveImage.GetData()) ) {}
				else return;
			}
			else if( (m_uButtonState & UISTATE_HOT) != 0 && !m_sSelectedHoverImage.IsEmpty()) {
				if( !DrawImage(ctx, m_sSelectedHoverImage.GetData()) ) {}
				else return;
			}

			if( !m_sSelectedImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sSelectedImage.GetData()) ) {}
			}
		}
		else {
			CButtonUI::PaintStatusImage(ctx);
		}
	}

	void COptionUI::PaintForegroundImage(IRenderContext& ctx)
	{
		if(IsSelected()) {
			if( !m_sSelectedForegroundImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sSelectedForegroundImage.GetData()) ) {}
				else return;
			}
		}

		return CButtonUI::PaintForegroundImage(ctx);
	}

	void COptionUI::PaintText(IRenderContext& ctx)
	{
		if( (m_uButtonState & UISTATE_SELECTED) != 0 )
		{
			DWORD oldTextColor = m_dwColor;
			int iOldFont = GetFont();
			if( m_dwSelectedColor != 0 ) m_dwColor = m_dwSelectedColor;
			if( GetSelectedFont() != -1 )
				SetFont(GetSelectedFont());
			// 与未选中共用 Button 绘制，避免选中/未选中对齐不一致
			CButtonUI::PaintText(ctx);
			m_dwColor = oldTextColor;
			SetFont(iOldFont);
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
		, m_nBoxBorderWidth(1)
		, m_dwBoxBackgroundColor(0xFFFFFFFF)
		, m_dwBoxBorderColor(0xBFBFBFFF)
		, m_dwBoxHoverBackgroundColor(0xFFFFFFFF)
		, m_dwBoxHoverBorderColor(0x1677FFFF)
		, m_dwSelectedBoxBackgroundColor(0x1677FFFF)
		, m_dwSelectedBoxBorderColor(0x1677FFFF)
		, m_dwSelectedBoxHoverBackgroundColor(0x0958D9FF)
		, m_dwSelectedBoxHoverBorderColor(0x0958D9FF)
		, m_dwCheckMarkColor(0xFFFFFFFF)
		, m_dwDisabledBoxBackgroundColor(0xF5F5F5FF)
		, m_dwDisabledBoxBorderColor(0xD9D9D9FF)
	{
		m_szBox.cx = m_szBox.cy = 16;
		m_szBoxRound.cx = m_szBoxRound.cy = 2;
		// Button 默认 kind=default 会带按钮色，这里清掉并改为内置勾选框样式
		SetKind(CONTROLKIND_NONE);
		SetBackgroundColor(0);
		SetHoverBackgroundColor(0);
		SetActiveBackgroundColor(0);
		SetDisabledBackgroundColor(0);
		SetHoverColor(0);
		SetActiveColor(0);
		SetHoverBorderColor(0);
		SetActiveBorderColor(0);
		SetBorderWidth(0);
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_LEFT;
		m_dwColor = 0x333333FF;
		m_dwSelectedColor = 0; // 原生样式下标签色不随选中变化
		SetCursor(DUI_HAND);
		SetAutoCalcWidth(true);
		// 勾选框用 box-gap 与文字间距；不要继承 Option 的左右 padding
		SetPadding(CDuiBox(0));
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
		return m_sImage.IsEmpty() && m_sSelectedImage.IsEmpty()
			&& m_sSelectedStateImage.IsEmpty() && m_sStateImage.IsEmpty();
	}

	RECT CCheckBoxUI::GetCheckBoxRect() const
	{
		SIZE sz = GetBoxSize();
		RECT rcPad = GetPadding();
		RECT rc = m_rcItem;
		rc.top = m_rcItem.top + (m_rcItem.bottom - m_rcItem.top - sz.cy) / 2;
		if( rc.top < m_rcItem.top ) rc.top = m_rcItem.top;
		rc.bottom = rc.top + sz.cy;
		rc.left = m_rcItem.left + rcPad.left;
		rc.right = rc.left + sz.cx;
		return rc;
	}

	void CCheckBoxUI::PaintNativeCheckBox(IRenderContext& ctx)
	{
		RECT rcBox = GetCheckBoxRect();
		DWORD dwBk = m_dwBoxBackgroundColor;
		DWORD dwBorder = m_dwBoxBorderColor;
		bool bSelected = IsSelected();
		bool bHot = (m_uButtonState & UISTATE_HOT) != 0;
		bool bPushed = (m_uButtonState & UISTATE_PUSHED) != 0;

		if( !IsEnabled() ) {
			dwBk = m_dwDisabledBoxBackgroundColor;
			dwBorder = m_dwDisabledBoxBorderColor;
		}
		else if( bSelected ) {
			dwBk = m_dwSelectedBoxBackgroundColor;
			dwBorder = m_dwSelectedBoxBorderColor;
			if( bHot || bPushed ) {
				dwBk = m_dwSelectedBoxHoverBackgroundColor;
				dwBorder = m_dwSelectedBoxHoverBorderColor;
			}
		}
		else if( bHot || bPushed ) {
			dwBk = m_dwBoxHoverBackgroundColor;
			dwBorder = m_dwBoxHoverBorderColor;
		}

		SIZE szRound = m_szBoxRound;
		if( m_pManager != NULL ) m_pManager->GetDPIObj()->Scale(&szRound);
		int nBorder = m_nBoxBorderWidth;
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

		DWORD dwMark = IsEnabled() ? m_dwCheckMarkColor : 0xBFBFBFFF;
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
					RenderMeasureHtmlText(m_pManager, rcText, sText.GetData(), 0, GetFont(), uStyle);
				else
					RenderMeasureText(m_pManager, rcText, sText.GetData(), 0, GetFont(), uStyle);
				sz.cx += nGap + (rcText.right - rcText.left);
				RECT rcPad = GetTextPadding();
				sz.cx += rcPad.left + rcPad.right;
			}
			RECT rcPadding = GetPadding();
			sz.cx += rcPadding.left + rcPadding.right;
		}
		return sz;
	}

	void CCheckBoxUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		if( IsNativeCheckStyle() ) return; // 背景只画在方框上
		COptionUI::PaintBackgroundColor(ctx);
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

		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();

		SIZE szBox = GetBoxSize();
		int nGap = GetBoxGap();
		RECT rcPadding = GetPadding();
		RECT rcPad = GetTextPadding();
		RECT rc = m_rcItem;
		rc.left += rcPadding.left + szBox.cx + nGap + rcPad.left;
		rc.right -= rcPadding.right + rcPad.right;
		rc.top += rcPadding.top + rcPad.top;
		rc.bottom -= rcPadding.bottom + rcPad.bottom;

		DWORD clrColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		int nLinks = 0;
		UINT uStyle = m_uTextStyle;
		if( (uStyle & (DT_CENTER | DT_RIGHT)) == 0 )
			uStyle |= DT_LEFT;
		if( m_bShowHtml )
			ctx.DrawHtmlText(rc, sText.GetData(), GetAdjustColor(clrColor), NULL, NULL, nLinks, GetFont(), uStyle);
		else
			ctx.DrawText(rc, sText.GetData(), GetAdjustColor(clrColor), GetFont(), uStyle);
	}

	static bool ParseCheckColorAttr(LPCTSTR pstrValue, DWORD& dwColor)
	{
		return ParseColorString(pstrValue, dwColor);
	}

	void CCheckBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("auto-check")) == 0 ) {
			SetAutoCheck(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("box-size")) == 0 ) {
			SIZE sz = { 16, 16 };
			LPTSTR pstr = NULL;
			sz.cx = _tcstol(pstrValue, &pstr, 10);
			sz.cy = sz.cx;
			if( pstr && *pstr == _T(',') ) sz.cy = _tcstol(pstr + 1, &pstr, 10);
			SetBoxSize(sz);
		}
		else if( _tcsicmp(pstrName, _T("box-gap")) == 0 ) {
			SetBoxGap(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("box-border-width")) == 0 ) {
			m_nBoxBorderWidth = _ttoi(pstrValue);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-border-radius")) == 0 ) {
			SIZE szRound = { 0 };
			if( ParseBorderRadiusValue(pstrValue, szRound) ) {
				m_szBoxRound = szRound;
				Invalidate();
			}
		}
		else if( _tcsicmp(pstrName, _T("box-background-color")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxBackgroundColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-border-color")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-background-color-hover")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxHoverBackgroundColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-border-color-hover")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwBoxHoverBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-background-color-selected")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwSelectedBoxBackgroundColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-border-color-selected")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwSelectedBoxBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-background-color-selected-hover")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwSelectedBoxHoverBackgroundColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-border-color-selected-hover")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwSelectedBoxHoverBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-background-color-disabled")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwDisabledBoxBackgroundColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("box-border-color-disabled")) == 0 ) {
			ParseCheckColorAttr(pstrValue, m_dwDisabledBoxBorderColor); Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("checkmark-color")) == 0 || _tcsicmp(pstrName, _T("accent-color")) == 0 ) {
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
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName.GetData());
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