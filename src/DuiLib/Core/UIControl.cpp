#include "StdAfx.h"

namespace DuiLib {

	// Bootstrap 5.3.8 全局颜色表
	UILIB_API KindColors g_kindColors[11] = {};
	static bool s_kindColorsInited = false;

	UILIB_API void InitKindColors()
	{
		if (s_kindColorsInited) return;
		s_kindColorsInited = true;

		// None（索引0）— 无样式，全零
		g_kindColors[0] = {
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
		};
		// Default（索引1）— 浅色背景 + 灰边框；hover/active 加深以便辨认
		g_kindColors[1] = {
			{0xEEEEEEFF, 0xDEE2E6FF, 0x212529FF},
			{0xD3D4D5FF, 0xC6C7C8FF, 0x212529FF},
			{0xC6C7C8FF, 0xBABBBCFF, 0x212529FF}
		};
		// Primary
		g_kindColors[2] = {
			{0x0D6EFDFF, 0x0D6EFDFF, 0xFFFFFFFF},
			{0x0B5ED7FF, 0x0A58CAFF, 0xFFFFFFFF},
			{0x0A58CAFF, 0x0A53BEFF, 0xFFFFFFFF}
		};
		// Secondary
		g_kindColors[3] = {
			{0x6C757DFF, 0x6C757DFF, 0xFFFFFFFF},
			{0x5C636AFF, 0x565E64FF, 0xFFFFFFFF},
			{0x565E64FF, 0x51585EFF, 0xFFFFFFFF}
		};
		// Success
		g_kindColors[4] = {
			{0x198754FF, 0x198754FF, 0xFFFFFFFF},
			{0x157347FF, 0x146C43FF, 0xFFFFFFFF},
			{0x146C43FF, 0x13653FFF, 0xFFFFFFFF}
		};
		// Danger
		g_kindColors[5] = {
			{0xDC3545FF, 0xDC3545FF, 0xFFFFFFFF},
			{0xBB2D3BFF, 0xB02A37FF, 0xFFFFFFFF},
			{0xB02A37FF, 0xA52834FF, 0xFFFFFFFF}
		};
		// Warning — hover/active 加深，避免 Bootstrap 5 提亮在黄色上几乎看不出变化
		g_kindColors[6] = {
			{0xFFC107FF, 0xFFC107FF, 0x000000FF},
			{0xE0A800FF, 0xD39E00FF, 0x000000FF},
			{0xD39E00FF, 0xC69500FF, 0x000000FF}
		};
		// Info — hover/active 加深，避免提亮在青色上几乎看不出变化
		g_kindColors[7] = {
			{0x0DCAF0FF, 0x0DCAF0FF, 0x000000FF},
			{0x0BA5C7FF, 0x0A98B8FF, 0x000000FF},
			{0x0A98B8FF, 0x098BA8FF, 0x000000FF}
		};
		// Light
		g_kindColors[8] = {
			{0xF8F9FAFF, 0xF8F9FAFF, 0x000000FF},
			{0xD3D4D5FF, 0xC6C7C8FF, 0x000000FF},
			{0xC6C7C8FF, 0xBABBBCFF, 0x000000FF}
		};
		// Dark
		g_kindColors[9] = {
			{0x212529FF, 0x212529FF, 0xFFFFFFFF},
			{0x424649FF, 0x373B3EFF, 0xFFFFFFFF},
			{0x4D5154FF, 0x373B3EFF, 0xFFFFFFFF}
		};
		// Link
		g_kindColors[10] = {
			{0, 0, 0x0D6EFDFF},
			{0, 0, 0x0A58CAFF},
			{0, 0, 0x0A58CAFF}
		};
	}

	IMPLEMENT_DUICONTROL(CControlUI)

		CControlUI::CControlUI()
		:m_pManager(NULL), 
		m_pParent(NULL), 
		m_bUpdateNeeded(true),
		m_bMenuUsed(false),
		m_bVisible(true), 
		m_bInternVisible(true),
		m_bFocused(false),
		m_bEnabled(true),
		m_bMouseEnabled(true),
		m_bKeyboardEnabled(true),
		m_bAbsolute(false),
		m_uAbsoluteAlign(0),
		m_iTextAlign(-1),
		m_iVerticalAlign(-1),
		m_bSetPos(false),
		m_bRichEvent(false),
		m_bDragEnabled(false),
		m_bDropEnabled(false),
		m_bResourceText(false),
		m_chShortcut('\0'),
		m_pTag(NULL),
		m_uAction(UIACTION_NONE),
		m_controlKind(CONTROLKIND_NONE),
		m_bOutline(false),
		m_bGradientVertical(true),
		m_dwBackColor(0),
		m_dwBackColor2(0),
		m_dwBackColor3(0),
		m_dwForeColor(0),
		m_dwBorderColor(0),
		m_dwFocusBorderColor(0),
		m_dwHoverBackgroundColor(0),
		m_dwActiveBackgroundColor(0),
		m_dwDisabledBackgroundColor(0),
		m_dwFocusBackgroundColor(0),
		m_dwHoverBorderColor(0),
		m_dwActiveBorderColor(0),
		m_dwDisabledBorderColor(0),
		m_uControlState(0),
		m_bColorHSL(false),
		m_nOpacity(255),
		m_nBorderWidth(0),
		m_nBorderStyle(PS_SOLID),
		m_nTooltipWidth(300),
		m_wCursor(0),
		m_instance(NULL)
	{
		m_cXY.cx = m_cXY.cy = 0;
		m_cxyFixed.cx = m_cxyFixed.cy = 0;
		m_fWidthPercent = 0.0f;
		m_fHeightPercent = 0.0f;
		m_cxyMin.cx = m_cxyMin.cy = 0;
		m_cxyMax.cx = m_cxyMax.cy = 9999;
		m_cxyBorderRadius.cx = m_cxyBorderRadius.cy = 0;

		::ZeroMemory(&m_rcMargin, sizeof(m_rcMargin));
		::ZeroMemory(&m_rcPadding, sizeof(m_rcPadding));
		::ZeroMemory(&m_rcItem, sizeof(RECT));
		::ZeroMemory(&m_rcPaint, sizeof(RECT));
		::ZeroMemory(&m_rcBorderWidth,sizeof(RECT));
		m_piAbsolutePercent.left = m_piAbsolutePercent.top = m_piAbsolutePercent.right = m_piAbsolutePercent.bottom = 0.0f;
	}

	CControlUI::~CControlUI()
	{
		if( OnDestroy ) OnDestroy(this);
		RemoveAllCustomAttribute();	
		if( m_pManager != NULL ) m_pManager->ReapObjects(this);
	}

	CDuiString CControlUI::GetName() const
	{
		return m_sName;
	}

	void CControlUI::SetName(LPCTSTR pstrName)
	{
		m_sName = pstrName;
	}

	LPVOID CControlUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_CONTROL) == 0 ) return this;
		return NULL;
	}

	LPCTSTR CControlUI::GetClass() const
	{
		return _T("ControlUI");
	}

	UINT CControlUI::GetControlFlags() const
	{
		return 0;
	}

	bool CControlUI::IsCaptionDragHit(POINT /*pt*/) const
	{
		UIAction a = GetAction();
		return (a == UIACTION_TITLE || a == UIACTION_MOVEWINDOW);
	}

	bool CControlUI::PreferClientHit() const
	{
		if( !IsEnabled() ) return false;
		// 勿在重写 PreferClientHit 的子类里再让 GetControlFlags 回调 PreferClientHit，以免循环
		if( (GetControlFlags() & UIFLAG_SETCURSOR) != 0 ) return true;
		if( m_wCursor != 0 ) return true;
		// 已配状态视觉时 DoEvent 会跟踪 HOT；若被当成 HTCAPTION 则悬停永不生效
		return HasStateVisual();
	}

	bool CControlUI::Activate()
	{
		if( !IsVisible() ) return false;
		if( !IsEnabled() ) return false;
		return true;
	}

	CPaintManagerUI* CControlUI::GetManager() const
	{
		return m_pManager;
	}

	void CControlUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		m_pManager = pManager;
		m_pParent = pParent;
		if( bInit && m_pParent ) Init();
	}

	CControlUI* CControlUI::GetParent() const
	{
		return m_pParent;
	}

	bool CControlUI::SetTimer(UINT nTimerID, UINT nElapse)
	{
		if(m_pManager == NULL) return false;

		return m_pManager->SetTimer(this, nTimerID, nElapse);
	}

	void CControlUI::KillTimer(UINT nTimerID)
	{
		if(m_pManager == NULL) return;

		m_pManager->KillTimer(this, nTimerID);
	}

	CDuiString CControlUI::GetText() const
	{
		if (!IsResourceText()) return m_sText;
		return CResourceManager::GetInstance()->GetText(m_sText);
	}

	void CControlUI::SetText(LPCTSTR pstrText)
	{
		if( m_sText == pstrText ) return;

		m_sText = pstrText;
		// 瑙ｆ瀽xml鎹㈣绗?
		m_sText.Replace(_T("{\\n}"), _T("\n"));
		Invalidate();
	}

	bool CControlUI::IsResourceText() const
	{
		return m_bResourceText;
	}

	void CControlUI::SetResourceText(bool bResource)
	{
		if( m_bResourceText == bResource ) return;
		m_bResourceText = bResource;
		Invalidate();
	}

	bool CControlUI::IsDragEnabled() const
	{
		return m_bDragEnabled;
	}

	void CControlUI::SetDragEnable(bool bDrag)
	{
		m_bDragEnabled = bDrag;
	}

	bool CControlUI::IsDropEnabled() const
	{
		return m_bDropEnabled;
	}

	void CControlUI::SetDropEnable(bool bDrop)
	{
		m_bDropEnabled = bDrop;
	}


	bool CControlUI::IsRichEvent() const
	{
		return m_bRichEvent;
	}

	void CControlUI::SetRichEvent(bool bEnable)
	{
		m_bRichEvent = bEnable;
	}

	static bool ParseLinearGradient(LPCTSTR pstr, bool& bVertical, DWORD& c1, DWORD& c2, DWORD& c3)
	{
		bVertical = true;
		c1 = c2 = c3 = 0;
		if( pstr == NULL ) return false;
		while( *pstr && *pstr <= _T(' ') ) ++pstr;
		if( _tcsnicmp(pstr, _T("linear-gradient"), 15) != 0 ) return false;
		pstr += 15;
		while( *pstr && *pstr <= _T(' ') ) ++pstr;
		if( *pstr != _T('(') ) return false;
		++pstr;
		// 取括号内内容
		CDuiString sInner;
		int depth = 1;
		for( ; *pstr && depth > 0; ++pstr ) {
			if( *pstr == _T('(') ) ++depth;
			else if( *pstr == _T(')') ) {
				--depth;
				if( depth == 0 ) break;
			}
			if( depth > 0 ) sInner += *pstr;
		}
		if( sInner.IsEmpty() ) return false;

		// 按顶层逗号切分
		CDuiString parts[8];
		int nParts = 0;
		CDuiString cur;
		int d = 0;
		for( int i = 0; i < sInner.GetLength(); ++i ) {
			TCHAR ch = sInner[i];
			if( ch == _T('(') ) ++d;
			else if( ch == _T(')') ) --d;
			if( ch == _T(',') && d == 0 ) {
				cur.Trim();
				if( !cur.IsEmpty() && nParts < 8 ) parts[nParts++] = cur;
				cur.Empty();
			}
			else cur += ch;
		}
		cur.Trim();
		if( !cur.IsEmpty() && nParts < 8 ) parts[nParts++] = cur;
		if( nParts < 2 ) return false;

		int iColor = 0;
		bool bReverse = false;
		int iStart = 0;
		CDuiString s0 = parts[0];
		s0.Trim();
		bool bDir = false;
		if( _tcsnicmp(s0.GetData(), _T("to "), 3) == 0 ) {
			bDir = true;
			CDuiString dir = s0.Mid(3);
			dir.Trim();
			dir.MakeLower();
			if( dir.Find(_T("left")) >= 0 ) { bVertical = false; bReverse = true; }
			else if( dir.Find(_T("right")) >= 0 ) { bVertical = false; bReverse = false; }
			else if( dir.Find(_T("top")) >= 0 ) { bVertical = true; bReverse = true; }
			else if( dir.Find(_T("bottom")) >= 0 ) { bVertical = true; bReverse = false; }
			else bDir = false;
		}
		else {
			// 90deg → to right；180deg → to bottom；0deg → to top；270deg → to left
			LPTSTR pEnd = NULL;
			double deg = _tcstod(s0.GetData(), &pEnd);
			if( pEnd != s0.GetData() ) {
				while( *pEnd && *pEnd <= _T(' ') ) ++pEnd;
				if( _tcsnicmp(pEnd, _T("deg"), 3) == 0 ) {
					bDir = true;
					while( deg < 0 ) deg += 360;
					while( deg >= 360 ) deg -= 360;
					if( deg >= 45 && deg < 135 ) { bVertical = false; bReverse = false; }      // → right
					else if( deg >= 135 && deg < 225 ) { bVertical = true; bReverse = false; } // → bottom
					else if( deg >= 225 && deg < 315 ) { bVertical = false; bReverse = true; } // → left
					else { bVertical = true; bReverse = true; }                                 // → top
				}
			}
		}
		if( bDir ) iStart = 1;
		if( nParts - iStart < 2 ) return false;

		DWORD colors[3] = { 0, 0, 0 };
		int nColors = 0;
		for( int i = iStart; i < nParts && nColors < 3; ++i ) {
			CDuiString s = parts[i];
			s.Trim();
			// 去掉可选的 stop 百分比：#fff 50%
			int sp = s.ReverseFind(_T(' '));
			if( sp > 0 ) {
				CDuiString tail = s.Mid(sp + 1);
				tail.Trim();
				if( !tail.IsEmpty() && (tail[tail.GetLength()-1] == _T('%') || _istdigit(tail[0]) ) ) {
					s = s.Left(sp);
					s.Trim();
				}
			}
			DWORD clr = 0;
			if( !ParseColorString(s.GetData(), clr) ) return false;
			colors[nColors++] = clr;
		}
		if( nColors < 2 ) return false;
		if( bReverse ) {
			if( nColors == 2 ) { DWORD t = colors[0]; colors[0] = colors[1]; colors[1] = t; }
			else { DWORD t = colors[0]; colors[0] = colors[2]; colors[2] = t; }
		}
		c1 = colors[0];
		c2 = colors[1];
		c3 = (nColors >= 3) ? colors[2] : 0;
		return true;
	}

	void CControlUI::SetBackground(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL ) return;
		while( *pstrValue && *pstrValue <= _T(' ') ) ++pstrValue;
		if( *pstrValue == _T('\0') ) return;

		bool bVertical = true;
		DWORD c1 = 0, c2 = 0, c3 = 0;
		if( ParseLinearGradient(pstrValue, bVertical, c1, c2, c3) ) {
			m_sBackground = pstrValue;
			m_bGradientVertical = bVertical;
			m_dwBackColor = c1;
			m_dwBackColor2 = c2;
			m_dwBackColor3 = c3;
			Invalidate();
			return;
		}

		DWORD clrColor = 0;
		if( ParseColorString(pstrValue, clrColor) ) {
			m_sBackground = pstrValue;
			m_dwBackColor = clrColor;
			m_dwBackColor2 = 0;
			m_dwBackColor3 = 0;
			Invalidate();
		}
	}

	LPCTSTR CControlUI::GetBackground() const
	{
		return m_sBackground;
	}

	DWORD CControlUI::GetBackgroundColor() const
	{
		return m_dwBackColor;
	}

	void CControlUI::SetBackgroundColor(DWORD dwBackColor)
	{
		if( m_dwBackColor == dwBackColor && m_dwBackColor2 == 0 && m_dwBackColor3 == 0 ) return;

		m_dwBackColor = dwBackColor;
		m_dwBackColor2 = 0;
		m_dwBackColor3 = 0;
		m_sBackground.Empty();
		Invalidate();
	}

	DWORD CControlUI::GetForeColor() const
	{
		return m_dwForeColor;
	}

	void CControlUI::SetForeColor(DWORD dwForeColor)
	{
		if( m_dwForeColor == dwForeColor ) return;

		m_dwForeColor = dwForeColor;
		Invalidate();
	}

	LPCTSTR CControlUI::GetBackgroundImage()
	{
		return m_sBackgroundImage;
	}

	void CControlUI::SetBackgroundImage(LPCTSTR pStrImage)
	{
		CDuiString sUrl;
		if( ParseCssUrlImage(pStrImage, sUrl) ) pStrImage = sUrl.GetData();
		if( m_sBackgroundImage == pStrImage ) return;

		m_sBackgroundImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CControlUI::GetHoverBackgroundImage() const
	{
		return m_sBackgroundImageHover;
	}

	void CControlUI::SetHoverBackgroundImage(LPCTSTR pStrImage)
	{
		CDuiString sUrl;
		if( ParseCssUrlImage(pStrImage, sUrl) ) pStrImage = sUrl.GetData();
		if( m_sBackgroundImageHover == pStrImage ) return;
		m_sBackgroundImageHover = pStrImage;
		Invalidate();
	}

	LPCTSTR CControlUI::GetActiveBackgroundImage() const
	{
		return m_sBackgroundImageActive;
	}

	void CControlUI::SetActiveBackgroundImage(LPCTSTR pStrImage)
	{
		CDuiString sUrl;
		if( ParseCssUrlImage(pStrImage, sUrl) ) pStrImage = sUrl.GetData();
		if( m_sBackgroundImageActive == pStrImage ) return;
		m_sBackgroundImageActive = pStrImage;
		Invalidate();
	}

	LPCTSTR CControlUI::GetDisabledBackgroundImage() const
	{
		return m_sBackgroundImageDisabled;
	}

	void CControlUI::SetDisabledBackgroundImage(LPCTSTR pStrImage)
	{
		CDuiString sUrl;
		if( ParseCssUrlImage(pStrImage, sUrl) ) pStrImage = sUrl.GetData();
		if( m_sBackgroundImageDisabled == pStrImage ) return;
		m_sBackgroundImageDisabled = pStrImage;
		Invalidate();
	}

	LPCTSTR CControlUI::GetFocusBackgroundImage() const
	{
		return m_sBackgroundImageFocus;
	}

	void CControlUI::SetFocusBackgroundImage(LPCTSTR pStrImage)
	{
		CDuiString sUrl;
		if( ParseCssUrlImage(pStrImage, sUrl) ) pStrImage = sUrl.GetData();
		if( m_sBackgroundImageFocus == pStrImage ) return;
		m_sBackgroundImageFocus = pStrImage;
		Invalidate();
	}

	LPCTSTR CControlUI::GetSelectedBackgroundImage() const
	{
		return m_sBackgroundImageSelected;
	}

	void CControlUI::SetSelectedBackgroundImage(LPCTSTR pStrImage)
	{
		CDuiString sUrl;
		if( ParseCssUrlImage(pStrImage, sUrl) ) pStrImage = sUrl.GetData();
		if( m_sBackgroundImageSelected == pStrImage ) return;
		m_sBackgroundImageSelected = pStrImage;
		Invalidate();
	}
	
	LPCTSTR CControlUI::GetForegroundImage() const
	{
		return m_sForegroundImage;
	}

	void CControlUI::SetForegroundImage(LPCTSTR pStrImage)
	{
		if( m_sForegroundImage == pStrImage ) return;

		m_sForegroundImage = pStrImage;
		Invalidate();
	}

	DWORD CControlUI::GetBorderColor() const
	{
		return m_dwBorderColor;
	}

	void CControlUI::SetBorderColor(DWORD dwBorderColor)
	{
		if( m_dwBorderColor == dwBorderColor ) return;

		m_dwBorderColor = dwBorderColor;
		Invalidate();
	}

	DWORD CControlUI::GetHoverBackgroundColor() const
	{
		return m_dwHoverBackgroundColor;
	}

	void CControlUI::SetHoverBackgroundColor(DWORD dwColor)
	{
		if( m_dwHoverBackgroundColor == dwColor ) return;
		m_dwHoverBackgroundColor = dwColor;
		Invalidate();
	}

	DWORD CControlUI::GetActiveBackgroundColor() const
	{
		return m_dwActiveBackgroundColor;
	}

	void CControlUI::SetActiveBackgroundColor(DWORD dwColor)
	{
		if( m_dwActiveBackgroundColor == dwColor ) return;
		m_dwActiveBackgroundColor = dwColor;
		Invalidate();
	}

	DWORD CControlUI::GetDisabledBackgroundColor() const
	{
		return m_dwDisabledBackgroundColor;
	}

	void CControlUI::SetDisabledBackgroundColor(DWORD dwColor)
	{
		if( m_dwDisabledBackgroundColor == dwColor ) return;
		m_dwDisabledBackgroundColor = dwColor;
		Invalidate();
	}

	DWORD CControlUI::GetFocusBackgroundColor() const
	{
		return m_dwFocusBackgroundColor;
	}

	void CControlUI::SetFocusBackgroundColor(DWORD dwColor)
	{
		if( m_dwFocusBackgroundColor == dwColor ) return;
		m_dwFocusBackgroundColor = dwColor;
		Invalidate();
	}

	DWORD CControlUI::GetHoverBorderColor() const
	{
		return m_dwHoverBorderColor;
	}

	void CControlUI::SetHoverBorderColor(DWORD dwColor)
	{
		if( m_dwHoverBorderColor == dwColor ) return;
		m_dwHoverBorderColor = dwColor;
		Invalidate();
	}

	DWORD CControlUI::GetActiveBorderColor() const
	{
		return m_dwActiveBorderColor;
	}

	void CControlUI::SetActiveBorderColor(DWORD dwColor)
	{
		if( m_dwActiveBorderColor == dwColor ) return;
		m_dwActiveBorderColor = dwColor;
		Invalidate();
	}

	DWORD CControlUI::GetDisabledBorderColor() const
	{
		return m_dwDisabledBorderColor;
	}

	void CControlUI::SetDisabledBorderColor(DWORD dwColor)
	{
		if( m_dwDisabledBorderColor == dwColor ) return;
		m_dwDisabledBorderColor = dwColor;
		Invalidate();
	}

	bool CControlUI::HasStateVisual() const
	{
		return m_dwHoverBackgroundColor != 0 || m_dwActiveBackgroundColor != 0 || m_dwDisabledBackgroundColor != 0
			|| m_dwFocusBackgroundColor != 0
			|| m_dwHoverBorderColor != 0 || m_dwActiveBorderColor != 0 || m_dwDisabledBorderColor != 0
			|| !m_sBackgroundImageHover.IsEmpty() || !m_sBackgroundImageActive.IsEmpty()
			|| !m_sBackgroundImageDisabled.IsEmpty() || !m_sBackgroundImageFocus.IsEmpty()
			|| !m_sBackgroundImageSelected.IsEmpty();
	}

	DWORD CControlUI::GetPaintBackgroundColor() const
	{
		if( !IsEnabled() || (m_uControlState & UISTATE_DISABLED) != 0 ) {
			if( m_dwDisabledBackgroundColor != 0 ) return m_dwDisabledBackgroundColor;
		}
		else if( (m_uControlState & UISTATE_PUSHED) != 0 && m_dwActiveBackgroundColor != 0 ) {
			return m_dwActiveBackgroundColor;
		}
		else if( (m_uControlState & UISTATE_HOT) != 0 && m_dwHoverBackgroundColor != 0 ) {
			return m_dwHoverBackgroundColor;
		}
		else if( IsFocused() && m_dwFocusBackgroundColor != 0 ) {
			return m_dwFocusBackgroundColor;
		}
		return m_dwBackColor;
	}

	DWORD CControlUI::GetPaintBorderColor() const
	{
		if( !IsEnabled() || (m_uControlState & UISTATE_DISABLED) != 0 ) {
			if( m_dwDisabledBorderColor != 0 ) return m_dwDisabledBorderColor;
		}
		else if( (m_uControlState & UISTATE_PUSHED) != 0 && m_dwActiveBorderColor != 0 ) {
			return m_dwActiveBorderColor;
		}
		else if( (m_uControlState & UISTATE_HOT) != 0 && m_dwHoverBorderColor != 0 ) {
			return m_dwHoverBorderColor;
		}
		if( IsFocused() && m_dwFocusBorderColor != 0 ) return m_dwFocusBorderColor;
		return m_dwBorderColor;
	}

	DWORD CControlUI::GetFocusBorderColor() const
	{
		return m_dwFocusBorderColor;
	}

	void CControlUI::SetFocusBorderColor(DWORD dwBorderColor)
	{
		if( m_dwFocusBorderColor == dwBorderColor ) return;

		m_dwFocusBorderColor = dwBorderColor;
		Invalidate();
	}

	bool CControlUI::IsColorHSL() const
	{
		return m_bColorHSL;
	}

	void CControlUI::SetColorHSL(bool bColorHSL)
	{
		if( m_bColorHSL == bColorHSL ) return;

		m_bColorHSL = bColorHSL;
		Invalidate();
	}

	int CControlUI::GetBorderWidth() const
	{
		if(m_pManager != NULL) return m_pManager->GetDPIObj()->Scale(m_nBorderWidth);
		return m_nBorderWidth;
	}

	void CControlUI::SetBorderWidth(int nSize)
	{
		if( m_nBorderWidth == nSize ) return;

		m_nBorderWidth = nSize;
		Invalidate();
	}

	RECT CControlUI::GetBorderRectWidth() const
	{
		RECT rcBorderWidth = m_rcBorderWidth;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderWidth);
		return rcBorderWidth;
	}

	void CControlUI::SetBorderWidth( RECT rc )
	{
		m_rcBorderWidth = rc;
		Invalidate();
	}

	SIZE CControlUI::GetBorderRadius() const
	{
		SIZE cxyBorderRadius = m_cxyBorderRadius;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyBorderRadius);
		return cxyBorderRadius;
	}

	void CControlUI::SetBorderRadius(SIZE cxyRound)
	{
		m_cxyBorderRadius = cxyRound;
		Invalidate();
	}

	bool CControlUI::DrawImage(IRenderContext& ctx, LPCTSTR pStrImage, LPCTSTR pStrModify)
	{
		return ctx.DrawImageString(m_rcItem, m_rcPaint, pStrImage, pStrModify, m_instance);
	}

	const RECT& CControlUI::GetPos() const
	{
		return m_rcItem;
	}

	RECT CControlUI::GetRelativePos() const
	{
		CControlUI* pParent = GetParent();
		if( pParent != NULL ) {
			RECT rcParentPos = pParent->GetPos();
			CDuiRect rcRelativePos(m_rcItem);
			rcRelativePos.Offset(-rcParentPos.left, -rcParentPos.top);
			return rcRelativePos;
		}
		else {
			return CDuiRect(0, 0, 0, 0);
		}
	}

	RECT CControlUI::GetClientPos() const 
	{
		RECT rc = m_rcItem;
		RECT rcPadding = GetPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;
		return rc;
	}

	void CControlUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		if( rc.right < rc.left ) rc.right = rc.left;
		if( rc.bottom < rc.top ) rc.bottom = rc.top;

		CDuiRect invalidateRc = m_rcItem;
		if( ::IsRectEmpty(&invalidateRc) ) invalidateRc = rc;

		m_rcItem = rc;
		if( m_pManager == NULL ) return;

		if( !m_bSetPos ) {
			m_bSetPos = true;
			if( OnSize ) OnSize(this);
			m_bSetPos = false;
		}

		m_bUpdateNeeded = false;

		if( bNeedInvalidate && IsVisible() ) {
			invalidateRc.Join(m_rcItem);
			CControlUI* pParent = this;
			RECT rcTemp;
			RECT rcParent;
			while( pParent = pParent->GetParent() ) {
				if( !pParent->IsVisible() ) return;
				rcTemp = invalidateRc;
				rcParent = pParent->GetPos();
				if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) return;
			}
			m_pManager->Invalidate(invalidateRc);
		}
	}

	void CControlUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		m_cXY.cx += szOffset.cx;
		m_cXY.cy += szOffset.cy;
		NeedParentUpdate();
	}

	int CControlUI::GetWidth() const
	{
		return m_rcItem.right - m_rcItem.left;
	}

	int CControlUI::GetHeight() const
	{
		return m_rcItem.bottom - m_rcItem.top;
	}

	int CControlUI::GetX() const
	{
		return m_rcItem.left;
	}

	int CControlUI::GetY() const
	{
		return m_rcItem.top;
	}

	CDuiBox CControlUI::GetMargin() const
	{
		CDuiBox rcMargin = m_rcMargin;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcMargin);
		return rcMargin;
	}

	void CControlUI::SetMargin(CDuiBox rcMargin)
	{
		m_rcMargin = rcMargin;
		NeedParentUpdate();
	}

	CDuiBox CControlUI::GetPadding() const
	{
		CDuiBox rcPadding = m_rcPadding;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcPadding);
		return rcPadding;
	}

	void CControlUI::SetPadding(CDuiBox rcPadding)
	{
		m_rcPadding = rcPadding;
		NeedParentUpdate();
	}

	SIZE CControlUI::GetFixedXY() const
	{
		SIZE cXY = m_cXY;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cXY);
		return cXY;
	}

	void CControlUI::SetFixedXY(SIZE szXY)
	{
		m_cXY.cx = szXY.cx;
		m_cXY.cy = szXY.cy;
		NeedParentUpdate();
	}

	SIZE CControlUI::GetFixedSize() const
	{
		SIZE cxyFixed = m_cxyFixed;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyFixed);
		return cxyFixed;
	}

	int CControlUI::GetFixedWidth() const
	{
		SIZE cxyFixed = m_cxyFixed;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyFixed);

		return cxyFixed.cx;
	}

	void CControlUI::SetFixedWidth(int cx)
	{
		if( cx < 0 ) return;
		m_fWidthPercent = 0.0f;
		m_cxyFixed.cx = cx;
		NeedParentUpdate();
	}

	int CControlUI::GetFixedHeight() const
	{
		SIZE cxyFixed = m_cxyFixed;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyFixed);

		return cxyFixed.cy;
	}

	void CControlUI::SetFixedHeight(int cy)
	{
		if( cy < 0 ) return;
		m_fHeightPercent = 0.0f;
		m_cxyFixed.cy = cy;
		NeedParentUpdate();
	}

	float CControlUI::GetWidthPercent() const
	{
		return m_fWidthPercent;
	}

	void CControlUI::SetWidthPercent(float fPercent)
	{
		if( fPercent < 0.0f ) fPercent = 0.0f;
		m_fWidthPercent = fPercent;
		if( fPercent > 0.0f ) m_cxyFixed.cx = 0;
		NeedParentUpdate();
	}

	float CControlUI::GetHeightPercent() const
	{
		return m_fHeightPercent;
	}

	void CControlUI::SetHeightPercent(float fPercent)
	{
		if( fPercent < 0.0f ) fPercent = 0.0f;
		m_fHeightPercent = fPercent;
		if( fPercent > 0.0f ) m_cxyFixed.cy = 0;
		NeedParentUpdate();
	}

	bool CControlUI::IsWidthPercent() const
	{
		return m_fWidthPercent > 0.0f;
	}

	bool CControlUI::IsHeightPercent() const
	{
		return m_fHeightPercent > 0.0f;
	}

	int CControlUI::GetMinWidth() const
	{
		SIZE cxyMin = m_cxyMin;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyMin);
		return cxyMin.cx;
	}

	void CControlUI::SetMinWidth(int cx)
	{
		if( m_cxyMin.cx == cx ) return;

		if( cx < 0 ) return; 
		m_cxyMin.cx = cx;
		NeedParentUpdate();
	}

	int CControlUI::GetMaxWidth() const
	{
		SIZE cxyMax = m_cxyMax;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyMax);
		return cxyMax.cx;
	}

	void CControlUI::SetMaxWidth(int cx)
	{
		if( m_cxyMax.cx == cx ) return;

		if( cx < 0 ) return; 
		m_cxyMax.cx = cx;
		NeedParentUpdate();
	}

	int CControlUI::GetMinHeight() const
	{
		SIZE cxyMin = m_cxyMin;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyMin);
		
		return cxyMin.cy;
	}

	void CControlUI::SetMinHeight(int cy)
	{
		if( m_cxyMin.cy == cy ) return;

		if( cy < 0 ) return; 
		m_cxyMin.cy = cy;
		NeedParentUpdate();
	}

	int CControlUI::GetMaxHeight() const
	{
		SIZE cxyMax = m_cxyMax;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyMax);
		return cxyMax.cy;
	}

	void CControlUI::SetMaxHeight(int cy)
	{
		if( m_cxyMax.cy == cy ) return;

		if( cy < 0 ) return; 
		m_cxyMax.cy = cy;
		NeedParentUpdate();
	}

	TPercentInfo CControlUI::GetAbsolutePercent() const
	{
		return m_piAbsolutePercent;
	}
	
	void CControlUI::SetAbsolutePercent(TPercentInfo piAbsolutePercent)
	{
		m_piAbsolutePercent = piAbsolutePercent;
		NeedParentUpdate();
	}

	void CControlUI::SetAbsoluteAlign(UINT uAlign)
	{
		m_uAbsoluteAlign = uAlign;
		NeedParentUpdate();
	}

	UINT CControlUI::GetAbsoluteAlign() const
	{
		return m_uAbsoluteAlign;
	}

	void CControlUI::SetTextAlign(int iAlign)
	{
		if( m_iTextAlign == iAlign ) return;
		m_iTextAlign = iAlign;
		NeedParentUpdate();
	}

	int CControlUI::GetTextAlign() const
	{
		return m_iTextAlign;
	}

	void CControlUI::SetVerticalAlign(int iAlign)
	{
		if( m_iVerticalAlign == iAlign ) return;
		m_iVerticalAlign = iAlign;
		NeedParentUpdate();
	}

	int CControlUI::GetVerticalAlign() const
	{
		return m_iVerticalAlign;
	}

	CDuiString CControlUI::GetToolTip() const
	{
		if (!IsResourceText()) return m_sToolTip;
		return CResourceManager::GetInstance()->GetText(m_sToolTip);
	}

	void CControlUI::SetToolTip(LPCTSTR pstrText)
	{
		CDuiString strTemp(pstrText);
		strTemp.Replace(_T("<n>"),_T("\r\n"));
		m_sToolTip = strTemp;
	}

	void CControlUI::SetToolTipWidth( int nWidth )
	{
		m_nTooltipWidth = nWidth;
	}

	int CControlUI::GetToolTipWidth( void )
	{
		if(m_pManager != NULL) return m_pManager->GetDPIObj()->Scale(m_nTooltipWidth);
		return m_nTooltipWidth;
	}
	
	WORD CControlUI::GetCursor()
	{
		return m_wCursor;
	}

	void CControlUI::SetCursor(WORD wCursor)
	{
		m_wCursor = wCursor;
		Invalidate();
	}

	TCHAR CControlUI::GetShortcut() const
	{
		return m_chShortcut;
	}

	void CControlUI::SetShortcut(TCHAR ch)
	{
		m_chShortcut = ch;
	}

	bool CControlUI::IsContextMenuUsed() const
	{
		return m_bMenuUsed;
	}

	void CControlUI::SetContextMenuUsed(bool bMenuUsed)
	{
		m_bMenuUsed = bMenuUsed;
	}

	const CDuiString& CControlUI::GetUserData()
	{
		return m_sUserData;
	}

	void CControlUI::SetUserData(LPCTSTR pstrText)
	{
		m_sUserData = pstrText;
	}

	UINT_PTR CControlUI::GetTag() const
	{
		return m_pTag;
	}

	void CControlUI::SetTag(UINT_PTR pTag)
	{
		m_pTag = pTag;
	}

	UIAction CControlUI::GetAction() const
	{
		return m_uAction;
	}

	void CControlUI::SetAction(UIAction action)
	{
		m_uAction = action;
	}

	bool CControlUI::IsVisible() const
	{

		return m_bVisible && m_bInternVisible;
	}

	void CControlUI::SetVisible(bool bVisible)
	{
		if( m_bVisible == bVisible ) return;

		bool v = IsVisible();
		m_bVisible = bVisible;
		if( m_bFocused ) m_bFocused = false;
		if (!bVisible && m_pManager && m_pManager->GetFocus() == this) {
			m_pManager->SetFocus(NULL) ;
		}
		if( IsVisible() != v ) {
			NeedParentUpdate();
		}
	}

	void CControlUI::SetInternVisible(bool bVisible)
	{
		m_bInternVisible = bVisible;
		if (!bVisible && m_pManager && m_pManager->GetFocus() == this) {
			m_pManager->SetFocus(NULL) ;
		}
	}

	bool CControlUI::IsEnabled() const
	{
		return m_bEnabled;
	}

	void CControlUI::SetEnabled(bool bEnabled)
	{
		if( m_bEnabled == bEnabled ) return;

		m_bEnabled = bEnabled;
		if( bEnabled ) m_uControlState &= ~UISTATE_DISABLED;
		else m_uControlState |= UISTATE_DISABLED;
		Invalidate();
	}

	bool CControlUI::IsMouseEnabled() const
	{
		return m_bMouseEnabled;
	}

	void CControlUI::SetMouseEnabled(bool bEnabled)
	{
		m_bMouseEnabled = bEnabled;
	}

	bool CControlUI::IsKeyboardEnabled() const
	{
		return m_bKeyboardEnabled ;
	}
	void CControlUI::SetKeyboardEnabled(bool bEnabled)
	{
		m_bKeyboardEnabled = bEnabled ; 
	}

	bool CControlUI::IsFocused() const
	{
		return m_bFocused;
	}

	void CControlUI::SetFocus()
	{
		if( m_pManager != NULL ) m_pManager->SetFocus(this);
	}

	bool CControlUI::IsAbsolute() const
	{
		return m_bAbsolute;
	}

	void CControlUI::SetAbsolute(bool bAbsolute)
	{
		if( m_bAbsolute == bAbsolute ) return;

		m_bAbsolute = bAbsolute;
		NeedParentUpdate();
	}

	CControlUI* CControlUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
		if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
		if( (uFlags & UIFIND_HITTEST) != 0 && (!m_bMouseEnabled || !::PtInRect(&m_rcItem, * static_cast<LPPOINT>(pData))) ) return NULL;
		return Proc(this, pData);
	}

	void CControlUI::Invalidate()
	{
		if( !IsVisible() ) return;

		RECT invalidateRc = m_rcItem;

		CControlUI* pParent = this;
		RECT rcTemp;
		RECT rcParent;
		while( pParent = pParent->GetParent() )
		{
			rcTemp = invalidateRc;
			rcParent = pParent->GetPos();
			if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
			{
				return;
			}
		}

		if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
	}

	bool CControlUI::IsUpdateNeeded() const
	{
		return m_bUpdateNeeded;
	}

	void CControlUI::NeedUpdate()
	{
		if( !IsVisible() ) return;
		m_bUpdateNeeded = true;
		Invalidate();

		if( m_pManager != NULL ) m_pManager->NeedUpdate();
	}

	void CControlUI::NeedParentUpdate()
	{
		if( GetParent() ) {
			GetParent()->NeedUpdate();
			GetParent()->Invalidate();
		}
		else {
			NeedUpdate();
		}

		if( m_pManager != NULL ) m_pManager->NeedUpdate();
	}

	DWORD CControlUI::GetAdjustColor(DWORD dwColor)
	{
		DWORD c = dwColor;
		if( m_bColorHSL ) {
			short H, S, L;
			CPaintManagerUI::GetHSL(&H, &S, &L);
			c = CRenderEngine::AdjustColor(c, H, S, L);
		}
		if( m_nOpacity < 255 ) {
			BYTE a = (BYTE)((DuiColorA(c) * (UINT)m_nOpacity) / 255u);
			c = DuiColorSetA(c, a);
		}
		return c;
	}

	BYTE CControlUI::GetOpacity() const
	{
		return m_nOpacity;
	}

	void CControlUI::SetOpacity(BYTE nOpacity)
	{
		if( m_nOpacity == nOpacity ) return;
		m_nOpacity = nOpacity;
		Invalidate();
	}

	void CControlUI::Init()
	{
		DoInit();
		if( OnInit ) OnInit(this);
	}

	void CControlUI::DoInit()
	{

	}

	void CControlUI::Event(TEventUI& event)
	{
		CControlUI* ancestors[512];
		int depth = 0;
		for( CControlUI* p = m_pParent; p != NULL && depth < 512; p = p->GetParent() )
			ancestors[depth++] = p;

		event.ePhase = PHASE_CAPTURE;
		for( int i = depth - 1; i >= 0; --i ) {
			event.pCurrentTarget = ancestors[i];
			ancestors[i]->DoCaptureEvent(event);
			if( event.IsPropagationStopped() ) return;
		}

		event.ePhase = PHASE_TARGET;
		event.pCurrentTarget = this;
		if( OnEvent(&event) ) DoEvent(event);
	}

	void CControlUI::DoCaptureEvent(TEventUI& /*event*/)
	{
	}

	void CControlUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_SETCURSOR ) {
			if( GetCursor() ) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(GetCursor())));
			}
			else {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
			}
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			m_bFocused = true;
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			m_bFocused = false;
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_TIMER )
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_TIMER, event.wParam, event.lParam);
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( IsContextMenuUsed() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
				return;
			}
		}

		// 仅当配置了状态色时跟踪热态，避免无样式控件改变冒泡行为
		if( HasStateVisual() && IsMouseEnabled() ) {
			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
					m_uControlState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_MOUSEMOVE ) {
				if( (m_uControlState & UISTATE_CAPTURED) != 0 ) {
					if( ::PtInRect(&m_rcItem, event.ptMouse) )
						m_uControlState |= UISTATE_PUSHED;
					else
						m_uControlState &= ~UISTATE_PUSHED;
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_BUTTONUP ) {
				if( (m_uControlState & UISTATE_CAPTURED) != 0 ) {
					m_uControlState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_MOUSEENTER ) {
				if( IsEnabled() ) {
					m_uControlState |= UISTATE_HOT;
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE ) {
				m_uControlState &= ~UISTATE_HOT;
				Invalidate();
				return;
			}
		}

		BubbleEvent(event);
	}

	bool CControlUI::BubbleEvent(TEventUI& event)
	{
		if( m_pParent != NULL && !event.IsPropagationStopped() ) {
			event.ePhase = PHASE_BUBBLE;
			event.pCurrentTarget = m_pParent;
			m_pParent->DoEvent(event);
			return true;
		}
		return false;
	}


	void CControlUI::SetVirtualWnd(LPCTSTR pstrValue)
	{
		m_sVirtualWnd = pstrValue;
		m_pManager->UsedVirtualWnd(true);
	}

	CDuiString CControlUI::GetVirtualWnd() const
	{
		CDuiString str;
		if( !m_sVirtualWnd.IsEmpty() ){
			str = m_sVirtualWnd;
		}
		else{
			CControlUI* pParent = GetParent();
			if( pParent != NULL){
				str = pParent->GetVirtualWnd();
			}
			else{
				str = _T("");
			}
		}
		return str;
	}

	void CControlUI::AddCustomAttribute(LPCTSTR pstrName, LPCTSTR pstrAttr)
	{
		if( pstrName == NULL || pstrName[0] == _T('\0') || pstrAttr == NULL || pstrAttr[0] == _T('\0') ) return;

		if (m_mCustomAttrHash.Find(pstrName) == NULL) {
			CDuiString* pCostomAttr = new CDuiString(pstrAttr);
			if (pCostomAttr != NULL) {
				m_mCustomAttrHash.Set(pstrName, (LPVOID)pCostomAttr);
			}
		}
	}

	LPCTSTR CControlUI::GetCustomAttribute(LPCTSTR pstrName) const
	{
		if( pstrName == NULL || pstrName[0] == _T('\0') ) return NULL;
		CDuiString* pCostomAttr = static_cast<CDuiString*>(m_mCustomAttrHash.Find(pstrName));
		if( pCostomAttr ) return pCostomAttr->GetData();
		return NULL;
	}

	bool CControlUI::RemoveCustomAttribute(LPCTSTR pstrName)
	{
		if( pstrName == NULL || pstrName[0] == _T('\0') ) return NULL;
		CDuiString* pCostomAttr = static_cast<CDuiString*>(m_mCustomAttrHash.Find(pstrName));
		if( !pCostomAttr ) return false;

		delete pCostomAttr;
		return m_mCustomAttrHash.Remove(pstrName);
	}

	void CControlUI::RemoveAllCustomAttribute()
	{
		CDuiString* pCostomAttr;
		for( int i = 0; i< m_mCustomAttrHash.GetSize(); i++ ) {
			if(LPCTSTR key = m_mCustomAttrHash.GetAt(i)) {
				pCostomAttr = static_cast<CDuiString*>(m_mCustomAttrHash.Find(key));
				delete pCostomAttr;
			}
		}
		m_mCustomAttrHash.Resize();
	}

	namespace {

	bool MapBorderStyleKeyword(LPCTSTR tok, int& nStyle, bool& bNone)
	{
		bNone = false;
		if( tok == NULL || *tok == _T('\0') ) return false;
		if( _tcsicmp(tok, _T("none")) == 0 || _tcsicmp(tok, _T("hidden")) == 0 ) {
			bNone = true;
			nStyle = PS_SOLID;
			return true;
		}
		if( _tcsicmp(tok, _T("solid")) == 0 ) { nStyle = PS_SOLID; return true; }
		if( _tcsicmp(tok, _T("dashed")) == 0 || _tcsicmp(tok, _T("dash")) == 0 ) { nStyle = PS_DASH; return true; }
		if( _tcsicmp(tok, _T("dotted")) == 0 || _tcsicmp(tok, _T("dot")) == 0 ) { nStyle = PS_DOT; return true; }
		if( _tcsicmp(tok, _T("dashdot")) == 0 ) { nStyle = PS_DASHDOT; return true; }
		if( _tcsicmp(tok, _T("dashdotdot")) == 0 ) { nStyle = PS_DASHDOTDOT; return true; }
		return false;
	}

	bool ParseBorderWidthToken(LPCTSTR tok, int& nWidth)
	{
		if( tok == NULL || *tok == _T('\0') ) return false;
		if( *tok < _T('0') || *tok > _T('9') ) return false;
		LPTSTR pEnd = NULL;
		long v = _tcstol(tok, &pEnd, 10);
		if( pEnd == tok ) return false;
		if( *pEnd == _T('\0') || _tcsicmp(pEnd, _T("px")) == 0 ) {
			nWidth = (int)v;
			return true;
		}
		return false;
	}

	bool ParseBorderColorToken(LPCTSTR tok, DWORD& dwColor)
	{
		return ParseColorString(tok, dwColor);
	}

	void ApplyBorderShorthand(CControlUI* pControl, LPCTSTR pstrValue)
	{
		if( pControl == NULL || pstrValue == NULL ) return;
		while( *pstrValue == _T(' ') || *pstrValue == _T('\t') ) ++pstrValue;
		if( *pstrValue == _T('\0') ) return;

		if( _tcsicmp(pstrValue, _T("none")) == 0 || _tcsicmp(pstrValue, _T("0")) == 0 ) {
			RECT rcClear = { 0, 0, 0, 0 };
			pControl->SetBorderWidth(0);
			pControl->SetBorderWidth(rcClear);
			pControl->SetBorderColor(0);
			return;
		}

		int nWidth = -1;
		int nStyle = PS_SOLID;
		bool bHasStyle = false;
		bool bNoneStyle = false;
		DWORD dwColor = 0;
		bool bHasColor = false;

		LPCTSTR p = pstrValue;
		while( *p != _T('\0') ) {
			while( *p == _T(' ') || *p == _T('\t') ) ++p;
			if( *p == _T('\0') ) break;
			TCHAR tok[64];
			int n = 0;
			while( *p != _T('\0') && *p != _T(' ') && *p != _T('\t') && n < 63 )
				tok[n++] = *p++;
			tok[n] = _T('\0');

			bool isNone = false;
			int style = PS_SOLID;
			int w = 0;
			DWORD c = 0;
			if( MapBorderStyleKeyword(tok, style, isNone) ) {
				if( isNone ) bNoneStyle = true;
				else { nStyle = style; bHasStyle = true; }
			}
			else if( ParseBorderWidthToken(tok, w) ) {
				nWidth = w;
			}
			else if( ParseBorderColorToken(tok, c) ) {
				dwColor = c;
				bHasColor = true;
			}
		}

		if( bNoneStyle || nWidth == 0 ) {
			RECT rcClear = { 0, 0, 0, 0 };
			pControl->SetBorderWidth(0);
			pControl->SetBorderWidth(rcClear);
			if( bNoneStyle ) pControl->SetBorderColor(0);
			return;
		}

		// CSS：省略宽度时默认 1；省略样式默认 solid
		if( nWidth < 0 ) nWidth = bHasColor ? 1 : 0;
		if( nWidth <= 0 ) return;

		if( bHasColor ) pControl->SetBorderColor(dwColor);
		pControl->SetBorderStyle(bHasStyle ? nStyle : PS_SOLID);
		pControl->SetBorderWidth(nWidth);
		RECT rcClear = { 0, 0, 0, 0 };
		pControl->SetBorderWidth(rcClear);
	}

	} // namespace

	void CControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		// 样式表名：style / class（HTML class ≈ Default/style 名）
		if(m_pManager != NULL && ( _tcsicmp(pstrName, _T("style")) == 0 || _tcsicmp(pstrName, _T("class")) == 0 )) {
			LPCTSTR pStyle = m_pManager->GetStyle(pstrValue);
			if( pStyle != NULL) {
				ApplyAttributeList(pStyle);
				return;
			}
		}
		// 灞炴€?
		if( _tcsicmp(pstrName, _T("inner-style")) == 0 ) {
			ApplyAttributeList(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("position")) == 0 ) {
			// CSS position：absolute 绝对定位；static/relative 参与流式布局
			// 兼容：true→absolute；false→static；也可写相对父级百分比 left,top,right,bottom
			CDuiString nValue = pstrValue;
			if(nValue.Find(',') < 0) {
				bool bAbs = (_tcsicmp(pstrValue, _T("absolute")) == 0
					|| _tcsicmp(pstrValue, _T("true")) == 0);
				SetAbsolute(bAbs);
			}
			else {
				TPercentInfo piAbsolutePercent = { 0 };
				LPTSTR pstr = NULL;
				piAbsolutePercent.left = _tcstod(pstrValue, &pstr);  ASSERT(pstr);
				piAbsolutePercent.top = _tcstod(pstr + 1, &pstr);    ASSERT(pstr);
				piAbsolutePercent.right = _tcstod(pstr + 1, &pstr);  ASSERT(pstr);
				piAbsolutePercent.bottom = _tcstod(pstr + 1, &pstr); ASSERT(pstr);
				SetAbsolutePercent(piAbsolutePercent);
				SetAbsolute(true);
			}
		}
		else if( _tcsicmp(pstrName, _T("position-align")) == 0) {
			UINT uAlign = GetAbsoluteAlign();
			// 瑙ｆ瀽鏂囧瓧灞炴€?
			while( *pstrValue != _T('\0') ) {
				CDuiString sValue;
				while( *pstrValue == _T(',') || *pstrValue == _T(' ') ) pstrValue = ::CharNext(pstrValue);

				while( *pstrValue != _T('\0') && *pstrValue != _T(',') && *pstrValue != _T(' ') ) {
					LPTSTR pstrTemp = ::CharNext(pstrValue);
					while( pstrValue < pstrTemp) {
						sValue += *pstrValue++;
					}
				}
				if(sValue.CompareNoCase(_T("null")) == 0) {
					uAlign = 0;
				}
				if( sValue.CompareNoCase(_T("left")) == 0 ) {
					uAlign &= ~(DT_CENTER | DT_RIGHT);
					uAlign |= DT_LEFT;
				}
				else if( sValue.CompareNoCase(_T("center")) == 0 ) {
					uAlign &= ~(DT_LEFT | DT_RIGHT);
					uAlign |= DT_CENTER;
				}
				else if( sValue.CompareNoCase(_T("right")) == 0 ) {
					uAlign &= ~(DT_LEFT | DT_CENTER);
					uAlign |= DT_RIGHT;
				}
				else if( sValue.CompareNoCase(_T("top")) == 0 ) {
					uAlign &= ~(DT_BOTTOM | DT_VCENTER);
					uAlign |= DT_TOP;
				}
				else if( sValue.CompareNoCase(_T("vcenter")) == 0 ) {
					uAlign &= ~(DT_TOP | DT_BOTTOM);
					uAlign |= DT_VCENTER;
				}
				else if( sValue.CompareNoCase(_T("bottom")) == 0 ) {
					uAlign &= ~(DT_TOP | DT_VCENTER);
					uAlign |= DT_BOTTOM;
				}
			}
			SetAbsoluteAlign(uAlign);
		}
		else if( _tcsicmp(pstrName, _T("text-align")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("left")) == 0 ) SetTextAlign(DT_LEFT);
			else if( _tcsicmp(pstrValue, _T("center")) == 0 ) SetTextAlign(DT_CENTER);
			else if( _tcsicmp(pstrValue, _T("right")) == 0 ) SetTextAlign(DT_RIGHT);
		}
		else if( _tcsicmp(pstrName, _T("vertical-align")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("top")) == 0 ) SetVerticalAlign(DT_TOP);
			else if( _tcsicmp(pstrValue, _T("vcenter")) == 0 || _tcsicmp(pstrValue, _T("middle")) == 0 ) SetVerticalAlign(DT_VCENTER);
			else if( _tcsicmp(pstrValue, _T("bottom")) == 0 ) SetVerticalAlign(DT_BOTTOM);
		}
		else if( _tcsicmp(pstrName, _T("margin")) == 0 ) {
			// CSS margin 简写：top[,right[,bottom[,left]]]
			CDuiBox rcMargin;
			if( ParseCssBox(pstrValue, rcMargin) )
				SetMargin(rcMargin);
		}
		else if( _tcsicmp(pstrName, _T("padding")) == 0 ) {
			// CSS padding 简写：top[,right[,bottom[,left]]]
			CDuiBox rcPadding;
			if( ParseCssBox(pstrValue, rcPadding) )
				SetPadding(rcPadding);
		}
		else if( _tcsnicmp(pstrName, _T("margin-"), 7) == 0
			|| _tcsnicmp(pstrName, _T("padding-"), 8) == 0 ) {
			// 单边 / 轴向：margin-top、padding-left、padding-x 等（值支持 30 或 30px）
			LPTSTR pEnd = NULL;
			long v = _tcstol(pstrValue, &pEnd, 10);
			if( pEnd != pstrValue ) {
				const bool bMargin = (_tcsnicmp(pstrName, _T("margin-"), 7) == 0);
				LPCTSTR pSide = pstrName + (bMargin ? 7 : 8);
				CDuiBox rc = bMargin ? m_rcMargin : m_rcPadding;
				bool bApplied = true;
				if( _tcsicmp(pSide, _T("left")) == 0 ) rc.left = (int)v;
				else if( _tcsicmp(pSide, _T("top")) == 0 ) rc.top = (int)v;
				else if( _tcsicmp(pSide, _T("right")) == 0 ) rc.right = (int)v;
				else if( _tcsicmp(pSide, _T("bottom")) == 0 ) rc.bottom = (int)v;
				else if( _tcsicmp(pSide, _T("x")) == 0 ) { rc.left = rc.right = (int)v; }
				else if( _tcsicmp(pSide, _T("y")) == 0 ) { rc.top = rc.bottom = (int)v; }
				else bApplied = false;
				if( bApplied ) {
					if( bMargin ) SetMargin(rc);
					else SetPadding(rc); // 虚函数：容器走 CContainerUI::SetPadding→NeedUpdate
				}
			}
		}
		else if( _tcsicmp(pstrName, _T("background")) == 0 ) {
			// background: 纯色或 linear-gradient(...)
			SetBackground(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-image")) == 0 ) {
			// background-image: linear-gradient → 渐变；否则 url(...) / 裸路径 / file='...'
			bool bVertical = true;
			DWORD c1 = 0, c2 = 0, c3 = 0;
			if( ParseLinearGradient(pstrValue, bVertical, c1, c2, c3) )
				SetBackground(pstrValue);
			else
				SetBackgroundImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-image-hover")) == 0 ) {
			SetHoverBackgroundImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-image-active")) == 0 ) {
			SetActiveBackgroundImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-image-disabled")) == 0 ) {
			SetDisabledBackgroundImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-image-focus")) == 0 ) {
			SetFocusBackgroundImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-image-selected")) == 0 ) {
			SetSelectedBackgroundImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("background-color-hover")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetHoverBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("background-color-active")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetActiveBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("background-color-disabled")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetDisabledBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("background-color-focus")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetFocusBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("border-color-hover")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetHoverBorderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("border-color-active")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetActiveBorderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("border-color-disabled")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetDisabledBorderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("fore-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetForeColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("border-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetBorderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("border-color-focus")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetFocusBorderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-hsl")) == 0 ) SetColorHSL(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("border")) == 0 ) {
			// HTML/CSS 简写：border="1px solid #DDD"；顺序可任意；none/0 清除
			ApplyBorderShorthand(this, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("border-width")) == 0 ) {
			CDuiString nValue = pstrValue;
			if(nValue.Find(',') < 0) {
				SetBorderWidth(_ttoi(pstrValue));
				RECT rcBorder = {0};
				SetBorderWidth(rcBorder);
			}
			else {
				RECT rcBorder = { 0 };
				LPTSTR pstr = NULL;
				rcBorder.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
				rcBorder.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				rcBorder.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				rcBorder.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
				SetBorderWidth(rcBorder);
			}
		}
		else if( _tcsicmp(pstrName, _T("border-left-width")) == 0 ) SetLeftBorderWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("border-top-width")) == 0 ) SetTopBorderWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("border-right-width")) == 0 ) SetRightBorderWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("border-bottom-width")) == 0 ) SetBottomBorderWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("border-style")) == 0 ) {
			int nStyle = PS_SOLID;
			bool bNone = false;
			if( MapBorderStyleKeyword(pstrValue, nStyle, bNone) ) {
				if( bNone ) {
					RECT rcClear = { 0, 0, 0, 0 };
					SetBorderWidth(0);
					SetBorderWidth(rcClear);
				}
				else SetBorderStyle(nStyle);
			}
			else SetBorderStyle(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("border-radius")) == 0 ) {
			SIZE cxyRound = { 0 };
			if( ParseBorderRadiusValue(pstrValue, cxyRound) )
				SetBorderRadius(cxyRound);
		}
		else if( _tcsicmp(pstrName, _T("foreground-image")) == 0 ) SetForegroundImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("width")) == 0 ) {
			// "120" 像素；"50%" / "100%" 相对父级可用宽
			LPCTSTR p = pstrValue;
			while( p && (*p == _T(' ') || *p == _T('\t')) ) ++p;
			LPTSTR pEnd = NULL;
			double v = _tcstod(p, &pEnd);
			if( pEnd != p ) {
				while( *pEnd == _T(' ') || *pEnd == _T('\t') ) ++pEnd;
				if( *pEnd == _T('%') ) SetWidthPercent((float)(v / 100.0));
				else SetFixedWidth((int)v);
			}
		}
		else if( _tcsicmp(pstrName, _T("height")) == 0 ) {
			LPCTSTR p = pstrValue;
			while( p && (*p == _T(' ') || *p == _T('\t')) ) ++p;
			LPTSTR pEnd = NULL;
			double v = _tcstod(p, &pEnd);
			if( pEnd != p ) {
				while( *pEnd == _T(' ') || *pEnd == _T('\t') ) ++pEnd;
				if( *pEnd == _T('%') ) SetHeightPercent((float)(v / 100.0));
				else SetFixedHeight((int)v);
			}
		}
		else if( _tcsicmp(pstrName, _T("min-width")) == 0 ) SetMinWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("min-height")) == 0 ) SetMinHeight(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("max-width")) == 0 ) SetMaxWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("max-height")) == 0 ) SetMaxHeight(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("name")) == 0 || _tcsicmp(pstrName, _T("id")) == 0 ) SetName(pstrValue);
		else if( _tcsicmp(pstrName, _T("drag")) == 0 || _tcsicmp(pstrName, _T("draggable")) == 0 )
			SetDragEnable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("drop")) == 0 ) SetDropEnable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("resource-text")) == 0 ) SetResourceText(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("rich-event")) == 0 ) SetRichEvent(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("text")) == 0 ) SetText(pstrValue);
		else if( _tcsicmp(pstrName, _T("tooltip")) == 0 || _tcsicmp(pstrName, _T("title")) == 0 )
			SetToolTip(pstrValue);
		else if( _tcsicmp(pstrName, _T("user-data")) == 0 ) SetUserData(pstrValue);
		else if( _tcsicmp(pstrName, _T("enabled")) == 0 ) SetEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("disabled")) == 0 ) SetEnabled(_tcsicmp(pstrValue, _T("true")) != 0);
		else if( _tcsicmp(pstrName, _T("mouse")) == 0 ) SetMouseEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("pointer-events")) == 0 ) {
			bool bEnabled = true;
			if( ParseCssPointerEventsEnabled(pstrValue, bEnabled) )
				SetMouseEnabled(bEnabled);
		}
		else if( _tcsicmp(pstrName, _T("opacity")) == 0 ) {
			BYTE nOpacity = 255;
			if( ParseCssOpacity(pstrValue, nOpacity) )
				SetOpacity(nOpacity);
		}
		else if( _tcsicmp(pstrName, _T("keyboard")) == 0 ) SetKeyboardEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("visible")) == 0 ) SetVisible(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("visibility")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("hidden")) == 0 || _tcsicmp(pstrValue, _T("collapse")) == 0 )
				SetVisible(false);
			else
				SetVisible(true);
		}
		else if( _tcsicmp(pstrName, _T("display")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("none")) == 0 )
				SetVisible(false);
			else
				SetVisible(true);
		}
		else if( _tcsicmp(pstrName, _T("shortcut")) == 0 || _tcsicmp(pstrName, _T("accesskey")) == 0 )
			SetShortcut(pstrValue[0]);
		else if( _tcsicmp(pstrName, _T("menu")) == 0 || _tcsicmp(pstrName, _T("contextmenu")) == 0 )
			SetContextMenuUsed(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("cursor")) == 0 && pstrValue) {
			// Win32 名 + CSS 常用关键字
			if( _tcsicmp(pstrValue, _T("arrow")) == 0 || _tcsicmp(pstrValue, _T("default")) == 0 )
				SetCursor(DUI_ARROW);
			else if( _tcsicmp(pstrValue, _T("ibeam")) == 0 || _tcsicmp(pstrValue, _T("text")) == 0 )
				SetCursor(DUI_IBEAM);
			else if( _tcsicmp(pstrValue, _T("wait")) == 0 || _tcsicmp(pstrValue, _T("progress")) == 0 )
				SetCursor(DUI_WAIT);
			else if( _tcsicmp(pstrValue, _T("cross")) == 0 || _tcsicmp(pstrValue, _T("crosshair")) == 0 )
				SetCursor(DUI_CROSS);
			else if( _tcsicmp(pstrValue, _T("uparrow")) == 0 )	SetCursor(DUI_UPARROW);
			else if( _tcsicmp(pstrValue, _T("size")) == 0 )		SetCursor(DUI_SIZE);
			else if( _tcsicmp(pstrValue, _T("icon")) == 0 )		SetCursor(DUI_ICON);
			else if( _tcsicmp(pstrValue, _T("sizenwse")) == 0 || _tcsicmp(pstrValue, _T("nwse-resize")) == 0
				|| _tcsicmp(pstrValue, _T("nw-resize")) == 0 || _tcsicmp(pstrValue, _T("se-resize")) == 0 )
				SetCursor(DUI_SIZENWSE);
			else if( _tcsicmp(pstrValue, _T("sizenesw")) == 0 || _tcsicmp(pstrValue, _T("nesw-resize")) == 0
				|| _tcsicmp(pstrValue, _T("ne-resize")) == 0 || _tcsicmp(pstrValue, _T("sw-resize")) == 0 )
				SetCursor(DUI_SIZENESW);
			else if( _tcsicmp(pstrValue, _T("sizewe")) == 0 || _tcsicmp(pstrValue, _T("ew-resize")) == 0
				|| _tcsicmp(pstrValue, _T("e-resize")) == 0 || _tcsicmp(pstrValue, _T("w-resize")) == 0
				|| _tcsicmp(pstrValue, _T("col-resize")) == 0 )
				SetCursor(DUI_SIZEWE);
			else if( _tcsicmp(pstrValue, _T("sizens")) == 0 || _tcsicmp(pstrValue, _T("ns-resize")) == 0
				|| _tcsicmp(pstrValue, _T("n-resize")) == 0 || _tcsicmp(pstrValue, _T("s-resize")) == 0
				|| _tcsicmp(pstrValue, _T("row-resize")) == 0 )
				SetCursor(DUI_SIZENS);
			else if( _tcsicmp(pstrValue, _T("sizeall")) == 0 || _tcsicmp(pstrValue, _T("move")) == 0
				|| _tcsicmp(pstrValue, _T("all-scroll")) == 0 )
				SetCursor(DUI_SIZEALL);
			else if( _tcsicmp(pstrValue, _T("no")) == 0 || _tcsicmp(pstrValue, _T("not-allowed")) == 0
				|| _tcsicmp(pstrValue, _T("no-drop")) == 0 )
				SetCursor(DUI_NO);
			else if( _tcsicmp(pstrValue, _T("hand")) == 0 || _tcsicmp(pstrValue, _T("pointer")) == 0 )
				SetCursor(DUI_HAND);
		}
		else if( _tcsicmp(pstrName, _T("virtual-wnd")) == 0 ) SetVirtualWnd(pstrValue);
		else if( _tcsicmp(pstrName, _T("action")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("close")) == 0 )         SetAction(UIACTION_CLOSE);
			else if( _tcsicmp(pstrValue, _T("min")) == 0
			      || _tcsicmp(pstrValue, _T("mini")) == 0 )     SetAction(UIACTION_MIN);
			else if( _tcsicmp(pstrValue, _T("max")) == 0 )      SetAction(UIACTION_MAX);
			else if( _tcsicmp(pstrValue, _T("title")) == 0 )    SetAction(UIACTION_TITLE);
			else if( _tcsicmp(pstrValue, _T("move")) == 0
			      || _tcsicmp(pstrValue, _T("movewindow")) == 0) SetAction(UIACTION_MOVEWINDOW);
			else if( _tcsicmp(pstrValue, _T("copy")) == 0 )     SetAction(UIACTION_COPY);
			else                                                SetAction(UIACTION_NONE);
		}
		else if( _tcsicmp(pstrName, _T("kind")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("none")) == 0 )         SetKind(CONTROLKIND_NONE);
			else if( _tcsicmp(pstrValue, _T("default")) == 0 ) SetKind(CONTROLKIND_DEFAULT);
			else if( _tcsicmp(pstrValue, _T("primary")) == 0 )        SetKind(CONTROLKIND_PRIMARY);
			else if( _tcsicmp(pstrValue, _T("secondary")) == 0 ) SetKind(CONTROLKIND_SECONDARY);
			else if( _tcsicmp(pstrValue, _T("success")) == 0 )   SetKind(CONTROLKIND_SUCCESS);
			else if( _tcsicmp(pstrValue, _T("danger")) == 0 )    SetKind(CONTROLKIND_DANGER);
			else if( _tcsicmp(pstrValue, _T("warning")) == 0 )   SetKind(CONTROLKIND_WARNING);
			else if( _tcsicmp(pstrValue, _T("info")) == 0 )      SetKind(CONTROLKIND_INFO);
			else if( _tcsicmp(pstrValue, _T("light")) == 0 )     SetKind(CONTROLKIND_LIGHT);
			else if( _tcsicmp(pstrValue, _T("dark")) == 0 )      SetKind(CONTROLKIND_DARK);
			else if( _tcsicmp(pstrValue, _T("link")) == 0 )      SetKind(CONTROLKIND_LINK);
			else                                                  SetKind(CONTROLKIND_NONE);
		}
		else if( _tcsicmp(pstrName, _T("outline")) == 0 ) {
			SetOutline(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else {
			AddCustomAttribute(pstrName, pstrValue);
		}
	}

	CControlUI* CControlUI::ApplyAttributeList(LPCTSTR pstrValue)
	{
		// 瑙ｆ瀽鏍峰紡琛?
		if(m_pManager != NULL) {
			LPCTSTR pStyle = m_pManager->GetStyle(pstrValue);
			if( pStyle != NULL) {
				return ApplyAttributeList(pStyle);
			}
		}
		CDuiString sXmlData = pstrValue;
        sXmlData.Replace(_T("&quot;"), _T("\""));
        sXmlData.Replace(_T("\r"), _T(" "));
        sXmlData.Replace(_T("\n"), _T(" "));
        sXmlData.Replace(_T("\t"), _T(" "));
        sXmlData.Trim();

		LPCTSTR pstrList = sXmlData.GetData();
		// 瑙ｆ瀽鏍峰紡灞炴€?
		CDuiString sItem;
		CDuiString sValue;
		while( *pstrList != _T('\0') ) {
			sItem.Empty();
			sValue.Empty();
            while (*pstrList != _T('\0')  && (*pstrList == _T(' ')) )
            {
				pstrList++;
            }
			while( *pstrList != _T('\0') && *pstrList != _T('=') ) {
				LPTSTR pstrTemp = ::CharNext(pstrList);
				while( pstrList < pstrTemp) {
					sItem += *pstrList++;
				}
			}			
			ASSERT( *pstrList == _T('=') );
			if( *pstrList++ != _T('=') ) return this;
			ASSERT( *pstrList == _T('\"') );
			if( *pstrList++ != _T('\"') ) return this;
			while( *pstrList != _T('\0') && *pstrList != _T('\"') ) {
				LPTSTR pstrTemp = ::CharNext(pstrList);
				while( pstrList < pstrTemp) {
					sValue += *pstrList++;
				}
			}
			ASSERT( *pstrList == _T('\"') );
			if( *pstrList++ != _T('\"') ) return this;
			SetAttribute(sItem, sValue);
			if( *pstrList != _T(' ') && *pstrList != _T(',') )
			{
				return this;
			}else
			{
                ++pstrList;
			}
		}
		return this;
	}

	SIZE CControlUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = { 0, 0 };
		if( m_fWidthPercent > 0.0f ) {
			if( szAvailable.cx > 0 )
				sz.cx = (int)(szAvailable.cx * (double)m_fWidthPercent + 0.5);
		}
		else {
			sz.cx = GetFixedWidth();
		}
		if( m_fHeightPercent > 0.0f ) {
			if( szAvailable.cy > 0 )
				sz.cy = (int)(szAvailable.cy * (double)m_fHeightPercent + 0.5);
		}
		else {
			sz.cy = GetFixedHeight();
		}
		return sz;
	}

	bool CControlUI::Paint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if (pStopControl == this) return false;
		if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return true;
		if (!DoPaint(ctx, m_rcPaint, pStopControl)) return false;
		return true;
	}

	bool CControlUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		// 缁樺埗寰簭锛氳儗鏅鑹?>鑳屾櫙鍥?>鐘舵€佸浘->鏂囨湰->杈规
		SIZE cxyBorderRadius = GetBorderRadius();
		RECT rcBorderWidth = GetBorderRectWidth();

		if( cxyBorderRadius.cx > 0 || cxyBorderRadius.cy > 0 ) {
			CRenderClipScope roundClip(ctx, m_rcPaint, m_rcItem, cxyBorderRadius.cx, cxyBorderRadius.cy);
			PaintBackgroundColor(ctx);
			PaintBackgroundImage(ctx);
			PaintStatusImage(ctx);
			PaintForeColor(ctx);
			PaintForegroundImage(ctx);
			PaintText(ctx);
			PaintBorder(ctx);
		}
		else {
			PaintBackgroundColor(ctx);
			PaintBackgroundImage(ctx);
			PaintStatusImage(ctx);
			PaintForeColor(ctx);
			PaintForegroundImage(ctx);
			PaintText(ctx);
			PaintBorder(ctx);
		}
		return true;
	}

	void CControlUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		if( !IsEnabled() ) m_uControlState |= UISTATE_DISABLED;
		else m_uControlState &= ~UISTATE_DISABLED;

		const DWORD dwPaintBk = GetPaintBackgroundColor();
		if( dwPaintBk == 0 ) return;

		// 状态色（非常态）时忽略渐变，直接铺纯色
		const bool bStateFill = (dwPaintBk != m_dwBackColor);
		bool bVer = m_bGradientVertical;
		if( !bStateFill && m_dwBackColor2 != 0 ) {
			if( m_dwBackColor3 != 0 ) {
				RECT rc = m_rcItem;
				if( bVer ) {
					rc.bottom = (rc.bottom + rc.top) / 2;
					ctx.DrawGradient(rc, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), true, 8);
					rc.top = rc.bottom;
					rc.bottom = m_rcItem.bottom;
					ctx.DrawGradient(rc, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), true, 8);
				}
				else {
					rc.right = (rc.right + rc.left) / 2;
					ctx.DrawGradient(rc, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), false, 8);
					rc.left = rc.right;
					rc.right = m_rcItem.right;
					ctx.DrawGradient(rc, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), false, 8);
				}
			}
			else {
				ctx.DrawGradient(m_rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 16);
			}
		}
		else {
			DWORD color = GetAdjustColor(dwPaintBk);
			// 有 BorderRadius 时 DoPaint 已 PushRoundClip：这里用直角填充，
			// 避免 FillRoundRect 与 clip 几何不一致在角上漏出灰/透明底。
			// 按钮等自行 override PaintBackgroundColor 仍可用 FillRoundRect。
			if( DuiColorIsOpaque(dwPaintBk) ) ctx.DrawColor(m_rcPaint, color);
			else ctx.DrawColor(m_rcItem, color);
		}
	}

	void CControlUI::PaintBackgroundImage(IRenderContext& ctx)
	{
		LPCTSTR pImage = m_sBackgroundImage;
		if( !IsEnabled() && !m_sBackgroundImageDisabled.IsEmpty() )
			pImage = m_sBackgroundImageDisabled;
		else if( (m_uControlState & UISTATE_SELECTED) != 0 && !m_sBackgroundImageSelected.IsEmpty() )
			pImage = m_sBackgroundImageSelected;
		else if( (m_uControlState & UISTATE_PUSHED) != 0 && !m_sBackgroundImageActive.IsEmpty() )
			pImage = m_sBackgroundImageActive;
		else if( (m_uControlState & UISTATE_HOT) != 0 && !m_sBackgroundImageHover.IsEmpty() )
			pImage = m_sBackgroundImageHover;
		else if( IsFocused() && !m_sBackgroundImageFocus.IsEmpty() )
			pImage = m_sBackgroundImageFocus;
		if( pImage == NULL || *pImage == _T('\0') ) return;
		DrawImage(ctx, pImage);
	}

	void CControlUI::PaintStatusImage(IRenderContext& ctx)
	{
		return;
	}

	void CControlUI::PaintForeColor(IRenderContext& ctx)
	{
		if( m_dwForeColor == 0 ) return;
		ctx.DrawColor(m_rcItem, GetAdjustColor(m_dwForeColor));
	}
	
	void CControlUI::PaintForegroundImage(IRenderContext& ctx)
	{
		if( m_sForegroundImage.IsEmpty() ) return;
		DrawImage(ctx, (LPCTSTR)m_sForegroundImage);
	}

	void CControlUI::PaintText(IRenderContext& ctx)
	{
		return;
	}

	void CControlUI::PaintBorder(IRenderContext& ctx)
	{
		int nBorderWidth = GetBorderWidth();
		SIZE cxyBorderRadius = GetBorderRadius();
		RECT rcBorderWidth = GetBorderRectWidth();
		const DWORD dwBorder = GetPaintBorderColor();

		if(dwBorder != 0) {
			//画圆角边框
			if(nBorderWidth > 0 && ( cxyBorderRadius.cx > 0 || cxyBorderRadius.cy > 0 )) {
				ctx.DrawRoundRect(m_rcItem, nBorderWidth, cxyBorderRadius.cx, cxyBorderRadius.cy, GetAdjustColor(dwBorder), m_nBorderStyle);
			}
			else {
				if(rcBorderWidth.left > 0 || rcBorderWidth.top > 0 || rcBorderWidth.right > 0 || rcBorderWidth.bottom > 0) {
					RECT rcBorder;

					if(rcBorderWidth.left > 0){
						rcBorder		= m_rcItem;
						rcBorder.right	= rcBorder.left;
						ctx.DrawLine(rcBorder,rcBorderWidth.left,GetAdjustColor(dwBorder),m_nBorderStyle);
					}
					if(rcBorderWidth.top > 0){
						rcBorder		= m_rcItem;
						rcBorder.bottom	= rcBorder.top;
						ctx.DrawLine(rcBorder,rcBorderWidth.top,GetAdjustColor(dwBorder),m_nBorderStyle);
					}
					if(rcBorderWidth.right > 0){
						rcBorder		= m_rcItem;
						rcBorder.right -= 1;
						rcBorder.left	= rcBorder.right;
						ctx.DrawLine(rcBorder,rcBorderWidth.right,GetAdjustColor(dwBorder),m_nBorderStyle);
					}
					if(rcBorderWidth.bottom > 0){
						rcBorder		= m_rcItem;
						rcBorder.bottom -= 1;
						rcBorder.top	= rcBorder.bottom;
						ctx.DrawLine(rcBorder,rcBorderWidth.bottom,GetAdjustColor(dwBorder),m_nBorderStyle);
					}
				}
				else if(nBorderWidth > 0) {
					ctx.DrawRect(m_rcItem, nBorderWidth, GetAdjustColor(dwBorder), m_nBorderStyle);
				}
			}
		}
	}

	void CControlUI::DoPostPaint(IRenderContext& ctx, const RECT& rcPaint)
	{
		return;
	}

	int CControlUI::GetLeftBorderWidth() const
	{
		RECT rcBorderWidth = m_rcBorderWidth;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderWidth);

		return rcBorderWidth.left;
	}

	void CControlUI::SetLeftBorderWidth( int nSize )
	{
		m_rcBorderWidth.left = nSize;
		Invalidate();
	}

	int CControlUI::GetTopBorderWidth() const
	{
		RECT rcBorderWidth = m_rcBorderWidth;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderWidth);

		return rcBorderWidth.top;
	}

	void CControlUI::SetTopBorderWidth( int nSize )
	{
		m_rcBorderWidth.top = nSize;
		Invalidate();
	}

	int CControlUI::GetRightBorderWidth() const
	{
		RECT rcBorderWidth = m_rcBorderWidth;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderWidth);

		return rcBorderWidth.right;
	}

	void CControlUI::SetRightBorderWidth( int nSize )
	{
		m_rcBorderWidth.right = nSize;
		Invalidate();
	}

	int CControlUI::GetBottomBorderWidth() const
	{
		RECT rcBorderWidth = m_rcBorderWidth;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderWidth);

		return rcBorderWidth.bottom;
	}

	void CControlUI::SetBottomBorderWidth( int nSize )
	{
		m_rcBorderWidth.bottom = nSize;
		Invalidate();
	}

	int CControlUI::GetBorderStyle() const
	{
		return m_nBorderStyle;
	}

	void CControlUI::SetBorderStyle( int nStyle )
	{
		m_nBorderStyle = nStyle;
		Invalidate();
	}

	void CControlUI::SetKind(ControlKind kind)
	{
		InitKindColors();
		m_controlKind = kind;

		if (kind == CONTROLKIND_NONE) {
			SetBackgroundColor(0);
			SetBorderColor(0);
			SetForeColor(0);
			SetBorderWidth(0);
			SIZE round = {0, 0};
			SetBorderRadius(round);
			Invalidate();
			return;
		}

		if (m_bOutline) {
			SetOutline(true);
			return;
		}

		int idx = (int)kind;
		const KindStateColors& normal = g_kindColors[idx].Normal;

		SetBackgroundColor(normal.dwBackgroundColor);
		SetBorderColor(normal.dwBorderColor);

		// kind 默认圆角：CSS 半径 6px（约 Bootstrap --bs-border-radius）
		if (kind != CONTROLKIND_LINK) {
			SIZE round = {6, 6};
			SetBorderRadius(round);
		}
		if (normal.dwBorderColor != 0) {
			SetBorderWidth(1);
		}

		Invalidate();
	}

	ControlKind CControlUI::GetKind() const
	{
		return m_controlKind;
	}

	void CControlUI::SetOutline(bool bOutline)
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
		}
		else {
			SetKind(m_controlKind);
			return;
		}

		Invalidate();
	}

	bool CControlUI::IsOutline() const
	{
		return m_bOutline;
	}

} // namespace DuiLib
