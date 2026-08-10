#include "StdAfx.h"
#include "UISwitch.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSwitchUI)

	CSwitchUI::CSwitchUI()
		: m_nTrackGap(8)
		, m_nThumbInset(2)
		, m_dwTrackColor(0x00000040)
		, m_dwTrackColorChecked(0x1677FFFF)
		, m_dwTrackColorHover(0x00000059)
		, m_dwTrackColorCheckedHover(0x0958D9FF)
		, m_dwTrackColorDisabled(0x00000026)
		, m_dwTrackColorCheckedDisabled(0x1677FF80)
		, m_dwThumbColor(0xFFFFFFFF)
		, m_dwThumbColorDisabled(0xF5F5F5FF)
		, m_dwInnerTextColor(0xFFFFFFFF)
		, m_dwInnerTextColorUnchecked(0xFFFFFFFF)
	{
		m_szTrack.cx = 44;
		m_szTrack.cy = 22;
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
		m_dwSelectedColor = 0;
		SetCursor(DUI_HAND);
		SetAutoCalcWidth(true);
	}

	LPCTSTR CSwitchUI::GetClass() const
	{
		return _T("SwitchUI");
	}

	LPVOID CSwitchUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SWITCH) == 0 ) return static_cast<CSwitchUI*>(this);
		return COptionUI::GetInterface(pstrName);
	}

	void CSwitchUI::SetCheck(bool bCheck)
	{
		Selected(bCheck);
	}

	bool CSwitchUI::GetCheck() const
	{
		return IsSelected();
	}

	void CSwitchUI::SetTrackSize(SIZE sz)
	{
		m_szTrack = sz;
		m_bNeedEstimateSize = true;
		NeedParentUpdate();
		Invalidate();
	}

	SIZE CSwitchUI::GetTrackSize() const
	{
		SIZE sz = m_szTrack;
		if( m_pManager != NULL ) m_pManager->GetDPIObj()->Scale(&sz);
		return sz;
	}

	void CSwitchUI::SetTrackGap(int nGap)
	{
		m_nTrackGap = nGap;
		m_bNeedEstimateSize = true;
		NeedParentUpdate();
		Invalidate();
	}

	int CSwitchUI::GetTrackGap() const
	{
		return (m_pManager != NULL) ? m_pManager->GetDPIObj()->Scale(m_nTrackGap) : m_nTrackGap;
	}

	void CSwitchUI::SetThumbInset(int nInset)
	{
		m_nThumbInset = nInset;
		Invalidate();
	}

	int CSwitchUI::GetThumbInset() const
	{
		return (m_pManager != NULL) ? m_pManager->GetDPIObj()->Scale(m_nThumbInset) : m_nThumbInset;
	}

	void CSwitchUI::SetCheckedText(LPCTSTR pstrText)
	{
		m_sCheckedText = pstrText ? pstrText : _T("");
		Invalidate();
	}

	LPCTSTR CSwitchUI::GetCheckedText() const
	{
		return m_sCheckedText.GetData();
	}

	void CSwitchUI::SetUncheckedText(LPCTSTR pstrText)
	{
		m_sUncheckedText = pstrText ? pstrText : _T("");
		Invalidate();
	}

	LPCTSTR CSwitchUI::GetUncheckedText() const
	{
		return m_sUncheckedText.GetData();
	}

	bool CSwitchUI::IsNativeSwitchStyle() const
	{
		return m_sImage.IsEmpty() && m_sSelectedImage.IsEmpty()
			&& m_sSelectedStateImage.IsEmpty() && m_sStateImage.IsEmpty();
	}

	RECT CSwitchUI::GetTrackRect() const
	{
		SIZE sz = GetTrackSize();
		RECT rc = m_rcItem;
		rc.top = m_rcItem.top + (m_rcItem.bottom - m_rcItem.top - sz.cy) / 2;
		if( rc.top < m_rcItem.top ) rc.top = m_rcItem.top;
		rc.bottom = rc.top + sz.cy;
		rc.left = m_rcItem.left;
		rc.right = rc.left + sz.cx;
		return rc;
	}

	RECT CSwitchUI::GetThumbRect(const RECT& rcTrack) const
	{
		int nInset = GetThumbInset();
		if( nInset < 1 ) nInset = 1;
		int nThumb = (rcTrack.bottom - rcTrack.top) - nInset * 2;
		if( nThumb < 4 ) nThumb = 4;

		RECT rc = { 0 };
		rc.top = rcTrack.top + nInset;
		rc.bottom = rc.top + nThumb;
		if( IsSelected() )
			rc.left = rcTrack.right - nInset - nThumb;
		else
			rc.left = rcTrack.left + nInset;
		rc.right = rc.left + nThumb;
		return rc;
	}

	void CSwitchUI::PaintNativeSwitch(IRenderContext& ctx)
	{
		RECT rcTrack = GetTrackRect();
		bool bOn = IsSelected();
		bool bHot = (m_uButtonState & UISTATE_HOT) != 0 || (m_uButtonState & UISTATE_PUSHED) != 0;
		bool bEnabled = IsEnabled();

		DWORD dwTrack = m_dwTrackColor;
		if( !bEnabled )
			dwTrack = bOn ? m_dwTrackColorCheckedDisabled : m_dwTrackColorDisabled;
		else if( bOn )
			dwTrack = bHot ? m_dwTrackColorCheckedHover : m_dwTrackColorChecked;
		else if( bHot )
			dwTrack = m_dwTrackColorHover;

		int nRadius = (rcTrack.bottom - rcTrack.top + 1) / 2;
		if( dwTrack != 0 )
			ctx.FillRoundRect(rcTrack, nRadius, nRadius, GetAdjustColor(dwTrack));

		// 轨道内开/关文案（画在滑块对侧）
		LPCTSTR pInner = bOn ? m_sCheckedText.GetData() : m_sUncheckedText.GetData();
		if( pInner != NULL && pInner[0] != _T('\0') ) {
			RECT rcThumb = GetThumbRect(rcTrack);
			RECT rcText = rcTrack;
			int nPad = GetThumbInset() + 2;
			if( m_pManager != NULL ) nPad = m_pManager->GetDPIObj()->Scale(nPad);
			if( bOn ) {
				rcText.left += nPad;
				rcText.right = rcThumb.left - 1;
			}
			else {
				rcText.left = rcThumb.right + 1;
				rcText.right -= nPad;
			}
			if( rcText.right > rcText.left ) {
				DWORD dwText = bOn ? m_dwInnerTextColor : m_dwInnerTextColorUnchecked;
				if( !bEnabled ) dwText = 0xFFFFFFB3;
				ctx.DrawText(rcText, pInner, GetAdjustColor(dwText), GetFont(),
					DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
			}
		}

		RECT rcThumb = GetThumbRect(rcTrack);
		DWORD dwThumb = bEnabled ? m_dwThumbColor : m_dwThumbColorDisabled;
		int nThumbR = (rcThumb.bottom - rcThumb.top + 1) / 2;
		if( dwThumb != 0 )
			ctx.FillRoundRect(rcThumb, nThumbR, nThumbR, GetAdjustColor(dwThumb));
	}

	SIZE CSwitchUI::EstimateSize(SIZE szAvailable)
	{
		if( !IsNativeSwitchStyle() )
			return COptionUI::EstimateSize(szAvailable);

		SIZE szTrack = GetTrackSize();
		int nGap = GetTrackGap();
		CDuiString sText = GetText();

		SIZE sz = GetFixedSize();
		if( sz.cx > 0 && sz.cy > 0 ) return sz;

		if( sz.cy == 0 ) {
			int nFontH = 16;
			if( m_pManager != NULL && m_pManager->GetFontInfo(GetFont()) != NULL )
				nFontH = m_pManager->GetFontInfo(GetFont())->tm.tmHeight;
			sz.cy = (szTrack.cy > nFontH) ? szTrack.cy : nFontH;
		}

		if( sz.cx == 0 ) {
			sz.cx = szTrack.cx;
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
		}
		return sz;
	}

	void CSwitchUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		if( IsNativeSwitchStyle() ) return;
		COptionUI::PaintBackgroundColor(ctx);
	}

	void CSwitchUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( IsNativeSwitchStyle() ) {
			PaintNativeSwitch(ctx);
			return;
		}
		COptionUI::PaintStatusImage(ctx);
	}

	void CSwitchUI::PaintBorder(IRenderContext& ctx)
	{
		if( IsNativeSwitchStyle() ) return;
		COptionUI::PaintBorder(ctx);
	}

	void CSwitchUI::PaintText(IRenderContext& ctx)
	{
		if( !IsNativeSwitchStyle() ) {
			COptionUI::PaintText(ctx);
			return;
		}

		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();

		SIZE szTrack = GetTrackSize();
		int nGap = GetTrackGap();
		RECT rcPadding = GetPadding();
		RECT rcPad = GetTextPadding();
		RECT rc = m_rcItem;
		rc.left += rcPadding.left + szTrack.cx + nGap + rcPad.left;
		rc.right -= rcPadding.right + rcPad.right;
		rc.top += rcPadding.top + rcPad.top;
		rc.bottom -= rcPadding.bottom + rcPad.bottom;

		DWORD clrColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		int nLinks = 0;
		UINT uStyle = m_uTextStyle;
		if( (uStyle & (DT_CENTER | DT_RIGHT)) == 0 )
			uStyle |= DT_LEFT;
		if( m_bShowHtml )
			ctx.DrawHtmlText(rc, sText.GetData(), clrColor, NULL, NULL, nLinks, GetFont(), uStyle);
		else
			ctx.DrawText(rc, sText.GetData(), clrColor, GetFont(), uStyle);
	}

	void CSwitchUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("track-size")) == 0 || _tcsicmp(pstrName, _T("switch-size")) == 0 ) {
			SIZE sz = { 44, 22 };
			LPTSTR pstr = NULL;
			sz.cx = _tcstol(pstrValue, &pstr, 10);
			sz.cy = sz.cx / 2;
			if( pstr && (*pstr == _T(',') || *pstr == _T('x') || *pstr == _T('X')) )
				sz.cy = _tcstol(pstr + 1, &pstr, 10);
			if( sz.cx < 16 ) sz.cx = 16;
			if( sz.cy < 10 ) sz.cy = 10;
			SetTrackSize(sz);
		}
		else if( _tcsicmp(pstrName, _T("track-gap")) == 0 ) {
			SetTrackGap(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("thumb-inset")) == 0 ) {
			SetThumbInset(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("checked-text")) == 0 ) {
			SetCheckedText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("unchecked-text")) == 0 ) {
			SetUncheckedText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("track-color")) == 0
			|| _tcsicmp(pstrName, _T("track-color-unchecked")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwTrackColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("track-color-checked")) == 0
			|| _tcsicmp(pstrName, _T("accent-color")) == 0
			|| _tcsicmp(pstrName, _T("checked-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwTrackColorChecked = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("track-color-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwTrackColorHover = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("track-color-checked-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwTrackColorCheckedHover = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("track-color-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwTrackColorDisabled = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("track-color-checked-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwTrackColorCheckedDisabled = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("thumb-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwThumbColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("thumb-color-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwThumbColorDisabled = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("inner-color")) == 0
			|| _tcsicmp(pstrName, _T("inner-color-checked")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwInnerTextColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("inner-color-unchecked")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwInnerTextColorUnchecked = clr; Invalidate(); }
		}
		else {
			COptionUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
