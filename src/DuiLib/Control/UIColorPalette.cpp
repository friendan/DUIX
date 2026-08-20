#include "StdAfx.h"
#include "UIColorPalette.h"
#include <math.h>

namespace DuiLib {

	static float HueToRGB(float v1, float v2, float vH)
	{
		if (vH < 0.0f) vH += 1.0f;
		if (vH > 1.0f) vH -= 1.0f;
		if ((6.0f * vH) < 1.0f) return (v1 + (v2 - v1) * 6.0f * vH);
		if ((2.0f * vH) < 1.0f) return (v2);
		if ((3.0f * vH) < 2.0f) return (v1 + (v2 - v1) * ((2.0f / 3.0f) - vH) * 6.0f);
		return (v1);
	}

	static void RGBToHSL(BYTE R, BYTE G, BYTE B, float *pHue, float *pSat, float *pLue)
	{
		float r = R / 255.0f, g = G / 255.0f, b = B / 255.0f;
		float H = 0, S = 0, L = 0;
		float fMin = min(r, min(g, b));
		float fMax = max(r, max(g, b));
		float fDelta = fMax - fMin;
		L = (fMax + fMin) / 2.0f;
		if (fDelta == 0) {
			H = 0.0f;
			S = 0.0f;
		}
		else {
			if (L < 0.5f) S = fDelta / (fMax + fMin);
			else S = fDelta / (2.0f - fMax - fMin);
			float del_R = (((fMax - r) / 6.0f) + (fDelta / 2.0f)) / fDelta;
			float del_G = (((fMax - g) / 6.0f) + (fDelta / 2.0f)) / fDelta;
			float del_B = (((fMax - b) / 6.0f) + (fDelta / 2.0f)) / fDelta;
			if (r == fMax) H = del_B - del_G;
			else if (g == fMax) H = (1.0f / 3.0f) + del_R - del_B;
			else H = (2.0f / 3.0f) + del_G - del_R;
			if (H < 0.0f) H += 1.0f;
			if (H > 1.0f) H -= 1.0f;
		}
		*pHue = H;
		*pSat = S;
		*pLue = L;
	}

	// H/S/L in [0,1] → BGRA bytes (for DIB)
	static void HSLToBGR(float H, float S, float L, BYTE* pOut)
	{
		BYTE R, G, B;
		if (S <= 0.00001f) {
			R = G = B = (BYTE)(L * 255.0f + 0.5f);
		}
		else {
			float var_2 = (L < 0.5f) ? (L * (1.0f + S)) : ((L + S) - (S * L));
			float var_1 = 2.0f * L - var_2;
			R = (BYTE)(255.0f * HueToRGB(var_1, var_2, H + (1.0f / 3.0f)) + 0.5f);
			G = (BYTE)(255.0f * HueToRGB(var_1, var_2, H) + 0.5f);
			B = (BYTE)(255.0f * HueToRGB(var_1, var_2, H - (1.0f / 3.0f)) + 0.5f);
		}
		pOut[0] = B;
		pOut[1] = G;
		pOut[2] = R;
		pOut[3] = 0xFF;
	}

	// 经典刻度：H 0–360，S/L 0–200
	static void HSL200ToBGR(int h, int s, int l, BYTE* pOut)
	{
		HSLToBGR((float)h / 360.0f, (float)s / 200.0f, (float)l / 200.0f, pOut);
		// 避免纯黑被部分合成路径当透明
		if (pOut[0] == 0 && pOut[1] == 0 && pOut[2] == 0) {
			pOut[0] = pOut[1] = pOut[2] = 1;
		}
	}

	static DWORD HSL200ToDuiColor(int h, int s, int l)
	{
		BYTE bgr[4];
		HSL200ToBGR(h, s, l, bgr);
		return DuiColorFromRGB(bgr[2], bgr[1], bgr[0], 0xFF);
	}

	IMPLEMENT_DUICONTROL(CColorPaletteUI)

	CColorPaletteUI::CColorPaletteUI()
		: m_MemDc(NULL)
		, m_hMemBitmap(NULL)
		, m_hOldBitmap(NULL)
		, m_pBits(NULL)
		, m_uButtonState(0)
		, m_bIsInBar(false)
		, m_bIsInPallet(false)
		, m_nCurH(180)
		, m_nCurS(200)
		, m_nCurB(100)
		, m_nPalletHeight(200)
		, m_nBarHeight(16)
		, m_ptLastPalletMouse(0, 0)
		, m_ptLastBarMouse(0, 0)
	{
		memset(&m_bmInfo, 0, sizeof(BITMAP));
	}

	CColorPaletteUI::~CColorPaletteUI()
	{
		ReleaseOffscreen();
	}

	DWORD CColorPaletteUI::GetSelectColor()
	{
		return HSL200ToDuiColor(m_nCurH, m_nCurS, m_nCurB);
	}

	void CColorPaletteUI::SetSelectColor(DWORD dwColor)
	{
		float H = 0, S = 0, B = 0;
		RGBToHSL(DuiColorR(dwColor), DuiColorG(dwColor), DuiColorB(dwColor), &H, &S, &B);
		m_nCurH = (int)(H * 360.0f + 0.5f);
		m_nCurS = (int)(S * 200.0f + 0.5f);
		m_nCurB = (int)(B * 200.0f + 0.5f);
		if (m_nCurH < 0) m_nCurH = 0;
		if (m_nCurH > 360) m_nCurH = 360;
		if (m_nCurS < 0) m_nCurS = 0;
		if (m_nCurS > 200) m_nCurS = 200;
		if (m_nCurB < 0) m_nCurB = 0;
		if (m_nCurB > 200) m_nCurB = 200;

		SyncCursorFromValues();
		if (m_pBits != NULL) {
			UpdatePalletData();
			UpdateBarData();
		}
		Invalidate();
	}

	LPCTSTR CColorPaletteUI::GetClass() const
	{
		return _T("ColorPaletteUI");
	}

	LPVOID CColorPaletteUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_COLORPALETTE) == 0) return static_cast<CColorPaletteUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	UINT CColorPaletteUI::GetControlFlags() const
	{
		UINT uFlags = UIFLAG_TABSTOP;
		if (IsEnabled()) uFlags |= UIFLAG_SETCURSOR;
		return uFlags;
	}

	void CColorPaletteUI::SetPalletHeight(int nHeight)
	{
		if (nHeight < 1) nHeight = 1;
		if (m_nPalletHeight == nHeight) return;
		m_nPalletHeight = nHeight;
		NeedUpdate();
	}

	int CColorPaletteUI::GetPalletHeight() const
	{
		if (m_pManager != NULL) return m_pManager->GetDPIObj()->Scale(m_nPalletHeight);
		return m_nPalletHeight;
	}

	void CColorPaletteUI::SetBarHeight(int nHeight)
	{
		if (nHeight <= 0) return;
		if (nHeight > 150) nHeight = 150;
		if (m_nBarHeight == nHeight) return;
		m_nBarHeight = nHeight;
		NeedUpdate();
	}

	int CColorPaletteUI::GetBarHeight() const
	{
		if (m_pManager != NULL) return m_pManager->GetDPIObj()->Scale(m_nBarHeight);
		return m_nBarHeight;
	}

	void CColorPaletteUI::SetThumbImage(LPCTSTR pszImage)
	{
		if (m_strThumbImage == pszImage) return;
		m_strThumbImage = pszImage;
		Invalidate();
	}

	LPCTSTR CColorPaletteUI::GetThumbImage() const
	{
		return m_strThumbImage.GetData();
	}

	void CColorPaletteUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		SyncCursorFromValues();
		if (m_pBits != NULL) {
			UpdatePalletData();
			UpdateBarData();
		}
	}

	void CColorPaletteUI::DoInit()
	{
		EnsureOffscreen();
	}

	void CColorPaletteUI::EnsureOffscreen()
	{
		if (m_MemDc != NULL && m_pBits != NULL && m_hMemBitmap != NULL) return;
		ReleaseOffscreen();

		HDC hPaintDC = (m_pManager != NULL) ? m_pManager->GetPaintDC() : NULL;
		if (hPaintDC == NULL) hPaintDC = ::GetDC(NULL);
		bool bReleasePaint = (m_pManager == NULL || m_pManager->GetPaintDC() == NULL);

		BITMAPINFO bmi;
		memset(&bmi, 0, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = 400;
		bmi.bmiHeader.biHeight = -360; // top-down DIB，与逐行写像素一致
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		m_MemDc = ::CreateCompatibleDC(hPaintDC);
		if (m_MemDc == NULL) {
			if (bReleasePaint) ::ReleaseDC(NULL, hPaintDC);
			return;
		}
		m_hMemBitmap = ::CreateDIBSection(hPaintDC, &bmi, DIB_RGB_COLORS, (void**)&m_pBits, NULL, 0);
		if (bReleasePaint) ::ReleaseDC(NULL, hPaintDC);
		if (m_hMemBitmap == NULL || m_pBits == NULL) {
			ReleaseOffscreen();
			return;
		}
		m_hOldBitmap = (HBITMAP)::SelectObject(m_MemDc, m_hMemBitmap);
		::GetObject(m_hMemBitmap, sizeof(m_bmInfo), &m_bmInfo);
		memset(m_pBits, 0xFF, 400 * 360 * 4);

		SyncCursorFromValues();
		UpdatePalletData();
		UpdateBarData();
	}

	void CColorPaletteUI::ReleaseOffscreen()
	{
		if (m_MemDc != NULL) {
			if (m_hOldBitmap != NULL) {
				::SelectObject(m_MemDc, m_hOldBitmap);
				m_hOldBitmap = NULL;
			}
			::DeleteDC(m_MemDc);
			m_MemDc = NULL;
		}
		if (m_hMemBitmap != NULL) {
			::DeleteObject(m_hMemBitmap);
			m_hMemBitmap = NULL;
		}
		m_pBits = NULL; // CreateDIBSection 内存随 bitmap 释放
		memset(&m_bmInfo, 0, sizeof(BITMAP));
	}

	void CColorPaletteUI::SyncCursorFromValues()
	{
		int nClientWidth = m_rcItem.right - m_rcItem.left;
		int nPalletH = GetPalletHeight();
		int nBarH = GetBarHeight();
		if (nClientWidth <= 0) return;

		// 色板占满除饱和度条外的区域，避免中间死区
		int nItemH = m_rcItem.bottom - m_rcItem.top;
		int nPalletPaintH = nItemH - nBarH;
		if (nPalletPaintH < 1) nPalletPaintH = 1;
		if (nPalletH > 0 && nPalletH < nPalletPaintH) nPalletPaintH = nPalletH;

		m_ptLastPalletMouse.x = m_rcItem.left + m_nCurH * nClientWidth / 360;
		if (m_ptLastPalletMouse.x >= m_rcItem.right)
			m_ptLastPalletMouse.x = m_rcItem.right - 1;
		m_ptLastPalletMouse.y = m_rcItem.top + (200 - m_nCurB) * nPalletPaintH / 200;
		if (m_ptLastPalletMouse.y >= m_rcItem.top + nPalletPaintH)
			m_ptLastPalletMouse.y = m_rcItem.top + nPalletPaintH - 1;

		m_ptLastBarMouse.x = m_rcItem.left + m_nCurS * nClientWidth / 200;
		if (m_ptLastBarMouse.x >= m_rcItem.right)
			m_ptLastBarMouse.x = m_rcItem.right - 1;
		m_ptLastBarMouse.y = m_rcItem.bottom - nBarH / 2;
	}

	void CColorPaletteUI::GetPalletBarRect(RECT& rcPallet, RECT& rcBar) const
	{
		int nBarH = GetBarHeight();
		int nPalletH = GetPalletHeight();
		int nItemH = m_rcItem.bottom - m_rcItem.top;
		int nPalletPaintH = nItemH - nBarH;
		if (nPalletPaintH < 1) nPalletPaintH = 1;
		if (nPalletH > 0 && nPalletH < nPalletPaintH) nPalletPaintH = nPalletH;

		rcPallet.left = m_rcItem.left;
		rcPallet.top = m_rcItem.top;
		rcPallet.right = m_rcItem.right;
		rcPallet.bottom = m_rcItem.top + nPalletPaintH;

		rcBar.left = m_rcItem.left;
		rcBar.top = m_rcItem.bottom - nBarH;
		rcBar.right = m_rcItem.right;
		rcBar.bottom = m_rcItem.bottom;

		// 若布局有空隙，把空隙并入色板，保证点哪都能选
		if (rcPallet.bottom < rcBar.top)
			rcPallet.bottom = rcBar.top;
	}

	void CColorPaletteUI::NotifyColorChanging()
	{
		if (m_pManager == NULL) return;
		m_pManager->SendNotify(this, DUI_MSGTYPE_COLORCHANGING, GetSelectColor(), 0);
	}

	void CColorPaletteUI::NotifyColorChanged()
	{
		if (m_pManager == NULL) return;
		m_pManager->SendNotify(this, DUI_MSGTYPE_COLORCHANGED, GetSelectColor(), 0);
	}

	bool CColorPaletteUI::ApplyPalletPoint(POINT pt)
	{
		RECT rcPallet, rcBar;
		GetPalletBarRect(rcPallet, rcBar);
		int nClientWidth = m_rcItem.right - m_rcItem.left;
		int nPH = rcPallet.bottom - rcPallet.top;
		if (nClientWidth <= 0 || nPH < 1) return false;

		if (pt.x < rcPallet.left) pt.x = rcPallet.left;
		if (pt.x >= rcPallet.right) pt.x = rcPallet.right - 1;
		if (pt.y < rcPallet.top) pt.y = rcPallet.top;
		if (pt.y >= rcPallet.bottom) pt.y = rcPallet.bottom - 1;

		int x = (pt.x - m_rcItem.left) * 360 / nClientWidth;
		int y = (pt.y - rcPallet.top) * 200 / nPH;
		x = min(max(x, 0), 360);
		y = min(max(y, 0), 200);
		m_ptLastPalletMouse = pt;
		m_nCurH = x;
		m_nCurB = 200 - y;
		UpdateBarData();
		return true;
	}

	bool CColorPaletteUI::ApplyBarPoint(POINT pt)
	{
		RECT rcPallet, rcBar;
		GetPalletBarRect(rcPallet, rcBar);
		int nClientWidth = m_rcItem.right - m_rcItem.left;
		if (nClientWidth <= 0) return false;

		if (pt.x < rcBar.left) pt.x = rcBar.left;
		if (pt.x >= rcBar.right) pt.x = rcBar.right - 1;
		m_nCurS = (pt.x - m_rcItem.left) * 200 / nClientWidth;
		m_nCurS = min(max(m_nCurS, 0), 200);
		m_ptLastBarMouse.x = pt.x;
		m_ptLastBarMouse.y = (rcBar.top + rcBar.bottom) / 2;
		UpdatePalletData();
		return true;
	}

	bool CColorPaletteUI::NudgeByKey(WPARAM chKey)
	{
		const int kStep = (::GetKeyState(VK_CONTROL) < 0) ? 10 : 1;
		int nH = m_nCurH, nS = m_nCurS, nB = m_nCurB;
		const bool bShift = (::GetKeyState(VK_SHIFT) < 0) != 0;

		switch (chKey) {
		case VK_LEFT:
			if (bShift) nS -= kStep;
			else nH -= kStep;
			break;
		case VK_RIGHT:
			if (bShift) nS += kStep;
			else nH += kStep;
			break;
		case VK_UP:
			nB += kStep;
			break;
		case VK_DOWN:
			nB -= kStep;
			break;
		case VK_PRIOR: // PageUp
			nS += kStep;
			break;
		case VK_NEXT: // PageDown
			nS -= kStep;
			break;
		default:
			return false;
		}

		if (nH < 0) nH = 0;
		if (nH > 360) nH = 360;
		if (nS < 0) nS = 0;
		if (nS > 200) nS = 200;
		if (nB < 0) nB = 0;
		if (nB > 200) nB = 200;
		if (nH == m_nCurH && nS == m_nCurS && nB == m_nCurB) return true;

		const bool bSatChanged = (nS != m_nCurS);
		const bool bHBChanged = (nH != m_nCurH || nB != m_nCurB);
		m_nCurH = nH;
		m_nCurS = nS;
		m_nCurB = nB;
		SyncCursorFromValues();
		if (bSatChanged) UpdatePalletData();
		if (bHBChanged) UpdateBarData();
		return true;
	}

	void CColorPaletteUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			CControlUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETCURSOR) {
			if (IsEnabled())
				::SetCursor(::LoadCursor(NULL, IDC_CROSS));
			else
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS || event.Type == UIEVENT_KILLFOCUS) {
			Invalidate();
			return;
		}

		if (event.Type == UIEVENT_KEYDOWN) {
			if (!IsEnabled()) return;
			if (NudgeByKey(event.chKey)) {
				NotifyColorChanged();
				Invalidate();
			}
			return;
		}

		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK) {
			if (!IsEnabled() || m_pManager == NULL) return;
			if (!::PtInRect(&m_rcItem, event.ptMouse)) return;

			SetFocus();
			m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;

			RECT rcPallet, rcBar;
			GetPalletBarRect(rcPallet, rcBar);
			if (::PtInRect(&rcPallet, event.ptMouse)) {
				m_bIsInPallet = true;
				m_bIsInBar = false;
				ApplyPalletPoint(event.ptMouse);
				NotifyColorChanging();
			}
			else if (::PtInRect(&rcBar, event.ptMouse)) {
				m_bIsInBar = true;
				m_bIsInPallet = false;
				ApplyBarPoint(event.ptMouse);
				NotifyColorChanging();
			}
			Invalidate();
			return;
		}

		if (event.Type == UIEVENT_BUTTONUP) {
			const bool bWasPushed = (m_uButtonState & UISTATE_PUSHED) != 0;
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			m_bIsInPallet = false;
			m_bIsInBar = false;
			if (bWasPushed && IsEnabled())
				NotifyColorChanged();
			Invalidate();
			return;
		}

		if (event.Type == UIEVENT_MOUSEMOVE) {
			if ((m_uButtonState & UISTATE_CAPTURED) == 0 || !IsEnabled()) {
				CControlUI::DoEvent(event);
				return;
			}

			if (m_bIsInPallet) {
				ApplyPalletPoint(event.ptMouse);
				NotifyColorChanging();
				Invalidate();
			}
			else if (m_bIsInBar) {
				ApplyBarPoint(event.ptMouse);
				NotifyColorChanging();
				Invalidate();
			}
			return;
		}

		CControlUI::DoEvent(event);
	}

	void CColorPaletteUI::PaintThumb(IRenderContext& ctx, RECT rcThumb)
	{
		if (!m_strThumbImage.IsEmpty()) {
			ctx.DrawImageString(rcThumb, m_rcPaint, m_strThumbImage.GetData());
			return;
		}
		int cx = (rcThumb.left + rcThumb.right) / 2;
		int cy = (rcThumb.top + rcThumb.bottom) / 2;
		RECT rcOuter = { cx - 5, cy - 5, cx + 5, cy + 5 };
		RECT rcInner = { cx - 3, cy - 3, cx + 3, cy + 3 };
		ctx.DrawRect(rcOuter, 1, 0xFFFFFFFF);
		ctx.DrawRect(rcInner, 1, 0xFF000000);
	}

	void CColorPaletteUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		CControlUI::PaintBackgroundColor(ctx);
		PaintPallet(ctx);
	}

	void CColorPaletteUI::PaintPallet(IRenderContext& ctx)
	{
		EnsureOffscreen();
		if (m_MemDc == NULL || m_pBits == NULL) return;

		RECT rcPallet, rcBar;
		GetPalletBarRect(rcPallet, rcBar);
		int nPalletW = rcPallet.right - rcPallet.left;
		int nPalletH = rcPallet.bottom - rcPallet.top;
		int nBarW = rcBar.right - rcBar.left;
		int nBarH = rcBar.bottom - rcBar.top;
		if (nPalletW > 0 && nPalletH > 0)
			ctx.StretchBlit(m_MemDc, rcPallet.left, rcPallet.top, nPalletW, nPalletH, 0, 0, 360, 200);
		if (nBarW > 0 && nBarH > 0)
			ctx.StretchBlit(m_MemDc, rcBar.left, rcBar.top, nBarW, nBarH, 0, 210, 200, max(m_nBarHeight, 1));

		RECT rcPalletThumb = { m_ptLastPalletMouse.x - 5, m_ptLastPalletMouse.y - 5,
			m_ptLastPalletMouse.x + 5, m_ptLastPalletMouse.y + 5 };
		PaintThumb(ctx, rcPalletThumb);

		int nWidth = m_rcItem.right - m_rcItem.left;
		RECT rcBarThumb = {
			m_rcItem.left + m_nCurS * nWidth / 200 - 5,
			(rcBar.top + rcBar.bottom) / 2 - 5,
			m_rcItem.left + m_nCurS * nWidth / 200 + 5,
			(rcBar.top + rcBar.bottom) / 2 + 5 };
		PaintThumb(ctx, rcBarThumb);

		if (!IsEnabled()) {
			ctx.DrawColor(m_rcItem, 0xFFFFFF99); // 半透明白罩，表示禁用
		}
		else if (IsFocused()) {
			RECT rcFocus = m_rcItem;
			ctx.DrawRect(rcFocus, 1, 0xFF1677FF);
		}
	}

	void CColorPaletteUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcsicmp(pstrName, _T("palletheight")) == 0 ||
			_tcsicmp(pstrName, _T("palette-height")) == 0 ||
			_tcsicmp(pstrName, _T("paletteheight")) == 0) {
			SetPalletHeight(_ttoi(pstrValue));
		}
		else if (_tcsicmp(pstrName, _T("barheight")) == 0 ||
			_tcsicmp(pstrName, _T("bar-height")) == 0) {
			SetBarHeight(_ttoi(pstrValue));
		}
		else if (_tcsicmp(pstrName, _T("thumbimage")) == 0 ||
			_tcsicmp(pstrName, _T("thumb-image")) == 0) {
			SetThumbImage(pstrValue);
		}
		else if (_tcsicmp(pstrName, _T("selectcolor")) == 0 ||
			_tcsicmp(pstrName, _T("select-color")) == 0 ||
			_tcsicmp(pstrName, _T("color")) == 0) {
			DWORD dwColor = 0;
			if (ParseColorString(pstrValue, dwColor))
				SetSelectColor(dwColor);
			else if (*pstrValue == _T('#'))
				SetSelectColor((DWORD)_tcstoul(pstrValue + 1, NULL, 16));
			else
				SetSelectColor((DWORD)_tcstoul(pstrValue, NULL, 16));
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CColorPaletteUI::UpdatePalletData()
	{
		if (m_pBits == NULL) return;
		const int stride = m_bmInfo.bmWidthBytes > 0 ? m_bmInfo.bmWidthBytes : 400 * 4;
		for (int y = 0; y < 200; ++y) {
			BYTE* pRow = m_pBits + y * stride;
			for (int x = 0; x < 360; ++x) {
				// 上亮下暗：屏幕 y=0 → L=200
				HSL200ToBGR(x, m_nCurS, 200 - y, pRow + x * 4);
			}
		}
	}

	void CColorPaletteUI::UpdateBarData()
	{
		if (m_pBits == NULL) return;
		const int stride = m_bmInfo.bmWidthBytes > 0 ? m_bmInfo.bmWidthBytes : 400 * 4;
		const int barRows = max(m_nBarHeight, 1);
		for (int y = 0; y < barRows; ++y) {
			BYTE* pRow = m_pBits + (210 + y) * stride;
			for (int x = 0; x < 200; ++x) {
				HSL200ToBGR(m_nCurH, x, m_nCurB, pRow + x * 4);
			}
		}
	}

} // namespace DuiLib
