#include "StdAfx.h"
#include "UISpin.h"
#include <stdio.h>
#include <math.h>

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSpinUI)

	static double Pow10(int n)
	{
		double r = 1.0;
		if( n > 0 ) { for( int i = 0; i < n; ++i ) r *= 10.0; }
		else if( n < 0 ) { for( int i = 0; i < -n; ++i ) r /= 10.0; }
		return r;
	}

	static double RoundToPrecision(double v, int precision)
	{
		if( precision < 0 ) precision = 0;
		double s = Pow10(precision);
		if( v >= 0 ) return floor(v * s + 0.5) / s;
		return ceil(v * s - 0.5) / s;
	}

	CSpinUI::CSpinUI()
		: m_dbValue(0)
		, m_dbMin(-1e100)
		, m_dbMax(1e100)
		, m_dbStep(1)
		, m_nPrecision(0)
		, m_bControls(true)
		, m_bUpdating(false)
		, m_nBtnWidth(22)
		, m_nHoverBtn(0)
	{
		::ZeroMemory(&m_rcBtnUp, sizeof(m_rcBtnUp));
		::ZeroMemory(&m_rcBtnDown, sizeof(m_rcBtnDown));
		SetKind(CONTROLKIND_NONE);
		SetNumberOnly(true);
		SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		SetColor(0x000000FF);
		SetBackgroundColor(0xFFFFFFFF);
		SyncTextFromValue(false);
	}

	LPCTSTR CSpinUI::GetClass() const
	{
		return _T("SpinUI");
	}

	LPVOID CSpinUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SPIN) == 0 ) return static_cast<CSpinUI*>(this);
		if( _tcsicmp(pstrName, DUI_CTR_NUMBER) == 0 ) return static_cast<CSpinUI*>(this);
		return CEditUI::GetInterface(pstrName);
	}

	UINT CSpinUI::GetControlFlags() const
	{
		return CEditUI::GetControlFlags() | UIFLAG_SETCURSOR;
	}

	bool CSpinUI::PreferClientHit() const
	{
		return true;
	}

	int CSpinUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CSpinUI::ApplyIntegerEditStyle()
	{
		// ES_NUMBER 不允许小数点和负号
		bool bIntOnly = (m_nPrecision <= 0 && m_dbMin >= 0);
		SetNumberOnly(bIntOnly);
	}

	double CSpinUI::ClampValue(double v) const
	{
		if( v < m_dbMin ) v = m_dbMin;
		if( v > m_dbMax ) v = m_dbMax;
		return RoundToPrecision(v, m_nPrecision);
	}

	CDuiString CSpinUI::FormatValue(double v) const
	{
		TCHAR buf[64] = { 0 };
		int prec = m_nPrecision;
		if( prec < 0 ) prec = 0;
		if( prec > 8 ) prec = 8;
		_stprintf_s(buf, _countof(buf), _T("%.*f"), prec, v);
		return CDuiString(buf);
	}

	void CSpinUI::SyncTextFromValue(bool /*bNotifyEdit*/)
	{
		m_bUpdating = true;
		CDuiString s = FormatValue(m_dbValue);
		CEditUI::SetText(s.GetData());
		m_bUpdating = false;
	}

	bool CSpinUI::ParseTextToValue(LPCTSTR pstr, double& out) const
	{
		if( pstr == NULL || *pstr == _T('\0') ) return false;
		TCHAR* end = NULL;
		double v = _tcstod(pstr, &end);
		if( end == pstr ) return false;
		while( end && (*end == _T(' ') || *end == _T('\t')) ) ++end;
		if( end && *end != _T('\0') ) return false;
		out = v;
		return true;
	}

	void CSpinUI::SetValue(double v)
	{
		v = ClampValue(v);
		if( m_dbValue == v ) {
			SyncTextFromValue(true);
			return;
		}
		m_dbValue = v;
		SyncTextFromValue(true);
		Invalidate();
		if( m_pManager ) m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
	}

	double CSpinUI::GetValue() const
	{
		return m_dbValue;
	}

	void CSpinUI::SetMin(double v)
	{
		m_dbMin = v;
		ApplyIntegerEditStyle();
		SetValue(m_dbValue);
	}

	double CSpinUI::GetMin() const { return m_dbMin; }

	void CSpinUI::SetMax(double v)
	{
		m_dbMax = v;
		SetValue(m_dbValue);
	}

	double CSpinUI::GetMax() const { return m_dbMax; }

	void CSpinUI::SetStep(double v)
	{
		if( v == 0 ) v = 1;
		m_dbStep = fabs(v);
	}

	double CSpinUI::GetStep() const { return m_dbStep; }

	void CSpinUI::SetPrecision(int n)
	{
		if( n < 0 ) n = 0;
		if( n > 8 ) n = 8;
		m_nPrecision = n;
		ApplyIntegerEditStyle();
		SetValue(m_dbValue);
	}

	int CSpinUI::GetPrecision() const { return m_nPrecision; }

	void CSpinUI::SetControls(bool bShow)
	{
		m_bControls = bShow;
		Invalidate();
		if( GetHWND() != NULL )
			SetPos(GetPos(), false);
	}

	bool CSpinUI::IsControls() const { return m_bControls; }

	int CSpinUI::GetNativeEditRightReserve() const
	{
		return m_bControls ? m_nBtnWidth : 0;
	}

	void CSpinUI::StepUp()
	{
		if( IsReadOnly() || !IsEnabled() ) return;
		SetValue(m_dbValue + m_dbStep);
	}

	void CSpinUI::StepDown()
	{
		if( IsReadOnly() || !IsEnabled() ) return;
		SetValue(m_dbValue - m_dbStep);
	}

	void CSpinUI::OnNativeEditChanged()
	{
		if( m_bUpdating ) return;
		double v = 0;
		if( ParseTextToValue(m_sText.GetData(), v) ) {
			v = ClampValue(v);
			bool bChanged = (v != m_dbValue);
			m_dbValue = v;
			// 编辑中不强制改写原生文本（夹紧/规范化放到失焦）；避免 Edit_SetText 触发二次 EN_CHANGE
			CEditUI::OnNativeEditChanged();
			if( bChanged && m_pManager )
				m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
		}
		else {
			// 空串且已失焦：多半是销毁 HWND 时的噪声，恢复显示
			if( m_sText.IsEmpty() && !IsFocused() ) {
				SyncTextFromValue(true);
				return;
			}
			// 输入中间态（如 "-"、"1."）不强制改回
			CEditUI::OnNativeEditChanged();
		}
	}

	void CSpinUI::SetText(LPCTSTR pstrText)
	{
		if( m_bUpdating ) {
			CEditUI::SetText(pstrText);
			return;
		}
		double v = 0;
		if( ParseTextToValue(pstrText, v) ) {
			m_dbValue = ClampValue(v);
			SyncTextFromValue(true);
		}
		else if( pstrText == NULL || *pstrText == _T('\0') ) {
			// 空文本不当作清值，保持当前数值显示
			SyncTextFromValue(true);
		}
		else {
			CEditUI::SetText(pstrText);
		}
	}

	void CSpinUI::LayoutButtons()
	{
		if( !m_bControls ) {
			::ZeroMemory(&m_rcBtnUp, sizeof(m_rcBtnUp));
			::ZeroMemory(&m_rcBtnDown, sizeof(m_rcBtnDown));
			return;
		}
		RECT rc = m_rcItem;
		CDuiBox pad = GetPadding();
		int w = ScaleValue(m_nBtnWidth);
		m_rcBtnUp.right = rc.right - pad.right;
		m_rcBtnUp.left = m_rcBtnUp.right - w;
		m_rcBtnUp.top = rc.top + pad.top;
		m_rcBtnUp.bottom = (rc.top + rc.bottom) / 2;
		m_rcBtnDown = m_rcBtnUp;
		m_rcBtnDown.top = m_rcBtnUp.bottom;
		m_rcBtnDown.bottom = rc.bottom - pad.bottom;
	}

	int CSpinUI::HitButton(POINT pt) const
	{
		if( !m_bControls ) return 0;
		if( ::PtInRect(&m_rcBtnUp, pt) ) return 1;
		if( ::PtInRect(&m_rcBtnDown, pt) ) return -1;
		return 0;
	}

	SIZE CSpinUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = CEditUI::EstimateSize(szAvailable);
		if( GetFixedHeight() <= 0 && sz.cy < ScaleValue(28) ) sz.cy = ScaleValue(28);
		if( GetFixedWidth() <= 0 && sz.cx < ScaleValue(100) ) sz.cx = ScaleValue(100);
		return sz;
	}

	void CSpinUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			CEditUI::DoEvent(event);
			return;
		}

		LayoutButtons();

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() ) {
			// 上下钮：手型；文本区交给 Edit（I 型）
			if( HitButton(event.ptMouse) != 0 ) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			}
		}

		if( event.Type == UIEVENT_SCROLLWHEEL && IsEnabled() && !IsReadOnly() ) {
			bool bDown = (LOWORD(event.wParam) == SB_LINEDOWN);
			if( bDown ) StepDown();
			else StepUp();
			return;
		}

		if( event.Type == UIEVENT_MOUSEMOVE ) {
			int h = HitButton(event.ptMouse);
			if( h != m_nHoverBtn ) {
				m_nHoverBtn = h;
				Invalidate();
			}
			if( h != 0 ) return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_nHoverBtn != 0 ) { m_nHoverBtn = 0; Invalidate(); }
		}

		if( (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
			&& IsEnabled() && !IsReadOnly() ) {
			int h = HitButton(event.ptMouse);
			if( h > 0 ) { StepUp(); return; }
			if( h < 0 ) { StepDown(); return; }
		}

		if( event.Type == UIEVENT_KEYDOWN && IsEnabled() && !IsReadOnly() ) {
			if( event.chKey == VK_UP ) { StepUp(); return; }
			if( event.chKey == VK_DOWN ) { StepDown(); return; }
		}

		if( event.Type == UIEVENT_KILLFOCUS ) {
			// 失焦：提交合法值并规范显示；非法/空则回退到当前值
			double v = 0;
			if( ParseTextToValue(m_sText.GetData(), v) )
				SetValue(v);
			else
				SyncTextFromValue(true);
		}

		CEditUI::DoEvent(event);
	}

	void CSpinUI::PaintStatusImage(IRenderContext& ctx)
	{
		CEditUI::PaintStatusImage(ctx);
		if( !m_bControls ) return;
		LayoutButtons();

		DWORD clrLine = 0xD9D9D9FF;
		DWORD clrTxt = IsEnabled() ? 0x595959FF : 0xBFBFBFFF;
		DWORD clrHover = 0xE6F4FFFF;

		if( m_nHoverBtn > 0 && IsEnabled() )
			ctx.DrawColor(m_rcBtnUp, GetAdjustColor(clrHover));
		if( m_nHoverBtn < 0 && IsEnabled() )
			ctx.DrawColor(m_rcBtnDown, GetAdjustColor(clrHover));

		// 分隔线
		RECT rcSplit = m_rcBtnUp;
		rcSplit.left = m_rcBtnUp.left;
		rcSplit.right = m_rcBtnUp.left + ScaleValue(1);
		rcSplit.top = m_rcItem.top;
		rcSplit.bottom = m_rcItem.bottom;
		ctx.DrawColor(rcSplit, GetAdjustColor(clrLine));

		RECT rcMid = m_rcBtnUp;
		rcMid.top = m_rcBtnUp.bottom - ScaleValue(1);
		rcMid.bottom = m_rcBtnUp.bottom;
		ctx.DrawColor(rcMid, GetAdjustColor(clrLine));

		ctx.DrawText(m_rcBtnUp, _T("▲"), GetAdjustColor(clrTxt), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		ctx.DrawText(m_rcBtnDown, _T("▼"), GetAdjustColor(clrTxt), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	void CSpinUI::PaintText(IRenderContext& ctx)
	{
		// 原生编辑框仍有焦点时由 WC_EDIT 绘制
		HWND hEdit = GetHWND();
		if( hEdit != NULL && ::GetFocus() == hEdit ) return;

		// 始终按数值绘制，避免 m_sText 被空串冲掉后失焦空白
		CDuiString sDraw = FormatValue(m_dbValue);
		if( sDraw != m_sText ) {
			m_bUpdating = true;
			m_sText = sDraw;
			m_bUpdating = false;
		}

		DWORD clr = m_dwColor;
		if( clr == 0 && m_pManager ) clr = m_pManager->GetDefaultFontColor();
		if( !IsEnabled() ) {
			clr = m_dwDisabledColor;
			if( clr == 0 && m_pManager ) clr = m_pManager->GetDefaultDisabledColor();
		}

		RECT rcPad = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		RECT rc = m_rcItem;
		rc.left += rcPad.left + rcTextPadding.left;
		rc.right -= rcPad.right + rcTextPadding.right + ScaleValue(GetNativeEditRightReserve());
		rc.top += rcPad.top + rcTextPadding.top;
		rc.bottom -= rcPad.bottom + rcTextPadding.bottom;
		if( rc.right < rc.left + 4 ) rc.right = rc.left + 4;

		ctx.DrawText(rc, sDraw.GetData(), GetAdjustColor(clr), m_iFont, DT_SINGLELINE | m_uTextStyle);
	}

	void CSpinUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("value")) == 0 ) {
			SetValue(_tcstod(pstrValue, NULL));
		}
		else if( _tcsicmp(pstrName, _T("min")) == 0 ) {
			SetMin(_tcstod(pstrValue, NULL));
		}
		else if( _tcsicmp(pstrName, _T("max")) == 0 ) {
			SetMax(_tcstod(pstrValue, NULL));
		}
		else if( _tcsicmp(pstrName, _T("step")) == 0 ) {
			SetStep(_tcstod(pstrValue, NULL));
		}
		else if( _tcsicmp(pstrName, _T("precision")) == 0 || _tcsicmp(pstrName, _T("digits")) == 0 ) {
			SetPrecision(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("controls")) == 0 ) {
			SetControls(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("button-width")) == 0 ) {
			m_nBtnWidth = _ttoi(pstrValue);
			if( m_nBtnWidth < 14 ) m_nBtnWidth = 14;
			Invalidate();
		}
		else {
			CEditUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
