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
			{0xFFEEEEEE, 0xFFDEE2E6, 0xFF212529},
			{0xFFD3D4D5, 0xFFC6C7C8, 0xFF212529},
			{0xFFC6C7C8, 0xFFBABBBC, 0xFF212529}
		};
		// Primary
		g_kindColors[2] = {
			{0xFF0D6EFD, 0xFF0D6EFD, 0xFFFFFFFF},
			{0xFF0B5ED7, 0xFF0A58CA, 0xFFFFFFFF},
			{0xFF0A58CA, 0xFF0A53BE, 0xFFFFFFFF}
		};
		// Secondary
		g_kindColors[3] = {
			{0xFF6C757D, 0xFF6C757D, 0xFFFFFFFF},
			{0xFF5C636A, 0xFF565E64, 0xFFFFFFFF},
			{0xFF565E64, 0xFF51585E, 0xFFFFFFFF}
		};
		// Success
		g_kindColors[4] = {
			{0xFF198754, 0xFF198754, 0xFFFFFFFF},
			{0xFF157347, 0xFF146C43, 0xFFFFFFFF},
			{0xFF146C43, 0xFF13653F, 0xFFFFFFFF}
		};
		// Danger
		g_kindColors[5] = {
			{0xFFDC3545, 0xFFDC3545, 0xFFFFFFFF},
			{0xFFBB2D3B, 0xFFB02A37, 0xFFFFFFFF},
			{0xFFB02A37, 0xFFA52834, 0xFFFFFFFF}
		};
		// Warning — hover/active 加深，避免 Bootstrap 5 提亮在黄色上几乎看不出变化
		g_kindColors[6] = {
			{0xFFFFC107, 0xFFFFC107, 0xFF000000},
			{0xFFE0A800, 0xFFD39E00, 0xFF000000},
			{0xFFD39E00, 0xFFC69500, 0xFF000000}
		};
		// Info — hover/active 加深，避免提亮在青色上几乎看不出变化
		g_kindColors[7] = {
			{0xFF0DCAF0, 0xFF0DCAF0, 0xFF000000},
			{0xFF0BA5C7, 0xFF0A98B8, 0xFF000000},
			{0xFF0A98B8, 0xFF098BA8, 0xFF000000}
		};
		// Light
		g_kindColors[8] = {
			{0xFFF8F9FA, 0xFFF8F9FA, 0xFF000000},
			{0xFFD3D4D5, 0xFFC6C7C8, 0xFF000000},
			{0xFFC6C7C8, 0xFFBABBBC, 0xFF000000}
		};
		// Dark
		g_kindColors[9] = {
			{0xFF212529, 0xFF212529, 0xFFFFFFFF},
			{0xFF424649, 0xFF373B3E, 0xFFFFFFFF},
			{0xFF4D5154, 0xFF373B3E, 0xFFFFFFFF}
		};
		// Link
		g_kindColors[10] = {
			{0, 0, 0xFF0D6EFD},
			{0, 0, 0xFF0A58CA},
			{0, 0, 0xFF0A58CA}
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
		m_bFloat(false),
		m_uFloatAlign(0),
		m_iHAlign(-1),
		m_iVAlign(-1),
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
		m_dwBackColor(0),
		m_dwBackColor2(0),
		m_dwBackColor3(0),
		m_dwForeColor(0),
		m_dwBorderColor(0),
		m_dwFocusBorderColor(0),
		m_bColorHSL(false),
		m_nBorderSize(0),
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
		m_cxyBorderRound.cx = m_cxyBorderRound.cy = 0;

		::ZeroMemory(&m_rcPadding, sizeof(RECT));
		::ZeroMemory(&m_rcInset, sizeof(RECT));
		::ZeroMemory(&m_rcItem, sizeof(RECT));
		::ZeroMemory(&m_rcPaint, sizeof(RECT));
		::ZeroMemory(&m_rcBorderSize,sizeof(RECT));
		m_piFloatPercent.left = m_piFloatPercent.top = m_piFloatPercent.right = m_piFloatPercent.bottom = 0.0f;
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

	LPCTSTR CControlUI::GetGradient()
	{
		return m_sGradient;
	}

	void CControlUI::SetGradient(LPCTSTR pStrImage)
	{
		if( m_sGradient == pStrImage ) return;

		m_sGradient = pStrImage;
		Invalidate();
	}

	DWORD CControlUI::GetBkColor() const
	{
		return m_dwBackColor;
	}

	void CControlUI::SetBkColor(DWORD dwBackColor)
	{
		if( m_dwBackColor == dwBackColor ) return;

		m_dwBackColor = dwBackColor;
		Invalidate();
	}

	DWORD CControlUI::GetBkColor2() const
	{
		return m_dwBackColor2;
	}

	void CControlUI::SetBkColor2(DWORD dwBackColor)
	{
		if( m_dwBackColor2 == dwBackColor ) return;

		m_dwBackColor2 = dwBackColor;
		Invalidate();
	}

	DWORD CControlUI::GetBkColor3() const
	{
		return m_dwBackColor3;
	}

	void CControlUI::SetBkColor3(DWORD dwBackColor)
	{
		if( m_dwBackColor3 == dwBackColor ) return;

		m_dwBackColor3 = dwBackColor;
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

	LPCTSTR CControlUI::GetBkImage()
	{
		return m_sBkImage;
	}

	void CControlUI::SetBkImage(LPCTSTR pStrImage)
	{
		if( m_sBkImage == pStrImage ) return;

		m_sBkImage = pStrImage;
		Invalidate();
	}
	
	LPCTSTR CControlUI::GetForeImage() const
	{
		return m_sForeImage;
	}

	void CControlUI::SetForeImage(LPCTSTR pStrImage)
	{
		if( m_sForeImage == pStrImage ) return;

		m_sForeImage = pStrImage;
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

	int CControlUI::GetBorderSize() const
	{
		if(m_pManager != NULL) return m_pManager->GetDPIObj()->Scale(m_nBorderSize);
		return m_nBorderSize;
	}

	void CControlUI::SetBorderSize(int nSize)
	{
		if( m_nBorderSize == nSize ) return;

		m_nBorderSize = nSize;
		Invalidate();
	}

	RECT CControlUI::GetBorderRectSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderSize);
		return rcBorderSize;
	}

	void CControlUI::SetBorderSize( RECT rc )
	{
		m_rcBorderSize = rc;
		Invalidate();
	}

	SIZE CControlUI::GetBorderRound() const
	{
		SIZE cxyBorderRound = m_cxyBorderRound;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&cxyBorderRound);
		return cxyBorderRound;
	}

	void CControlUI::SetBorderRound(SIZE cxyRound)
	{
		m_cxyBorderRound = cxyRound;
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
		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;
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

	RECT CControlUI::GetPadding() const
	{
		RECT rcPadding = m_rcPadding;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcPadding);
		return rcPadding;
	}

	void CControlUI::SetPadding(RECT rcPadding)
	{
		m_rcPadding = rcPadding;
		NeedParentUpdate();
	}

	RECT CControlUI::GetInset() const
	{
		RECT rcInset = m_rcInset;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcInset);
		return rcInset;
	}

	void CControlUI::SetInset(RECT rcInset)
	{
		m_rcInset = rcInset;
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

	TPercentInfo CControlUI::GetFloatPercent() const
	{
		return m_piFloatPercent;
	}
	
	void CControlUI::SetFloatPercent(TPercentInfo piFloatPercent)
	{
		m_piFloatPercent = piFloatPercent;
		NeedParentUpdate();
	}

	void CControlUI::SetFloatAlign(UINT uAlign)
	{
		m_uFloatAlign = uAlign;
		NeedParentUpdate();
	}

	UINT CControlUI::GetFloatAlign() const
	{
		return m_uFloatAlign;
	}

	void CControlUI::SetHAlign(int iAlign)
	{
		if( m_iHAlign == iAlign ) return;
		m_iHAlign = iAlign;
		NeedParentUpdate();
	}

	int CControlUI::GetHAlign() const
	{
		return m_iHAlign;
	}

	void CControlUI::SetVAlign(int iAlign)
	{
		if( m_iVAlign == iAlign ) return;
		m_iVAlign = iAlign;
		NeedParentUpdate();
	}

	int CControlUI::GetVAlign() const
	{
		return m_iVAlign;
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

	bool CControlUI::IsFloat() const
	{
		return m_bFloat;
	}

	void CControlUI::SetFloat(bool bFloat)
	{
		if( m_bFloat == bFloat ) return;

		m_bFloat = bFloat;
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
		if( !m_bColorHSL ) return dwColor;
		short H, S, L;
		CPaintManagerUI::GetHSL(&H, &S, &L);
		return CRenderEngine::AdjustColor(dwColor, H, S, L);
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
			pControl->SetBorderSize(0);
			pControl->SetBorderSize(rcClear);
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
			pControl->SetBorderSize(0);
			pControl->SetBorderSize(rcClear);
			if( bNoneStyle ) pControl->SetBorderColor(0);
			return;
		}

		// CSS：省略宽度时默认 1；省略样式默认 solid
		if( nWidth < 0 ) nWidth = bHasColor ? 1 : 0;
		if( nWidth <= 0 ) return;

		if( bHasColor ) pControl->SetBorderColor(dwColor);
		pControl->SetBorderStyle(bHasStyle ? nStyle : PS_SOLID);
		pControl->SetBorderSize(nWidth);
		RECT rcClear = { 0, 0, 0, 0 };
		pControl->SetBorderSize(rcClear);
	}

	} // namespace

	void CControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		// 鏍峰紡琛?
		if(m_pManager != NULL &&  _tcsicmp(pstrName, _T("style")) == 0) {
			LPCTSTR pStyle = m_pManager->GetStyle(pstrValue);
			if( pStyle != NULL) {
				ApplyAttributeList(pStyle);
				return;
			}
		}
		// 灞炴€?
		if( _tcsicmp(pstrName, _T("innerstyle")) == 0 ) {
			ApplyAttributeList(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("pos")) == 0 ) {
			RECT rcPos = { 0 };
			LPTSTR pstr = NULL;
			rcPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			rcPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
			rcPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
			rcPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
			SIZE szXY = {rcPos.left >= 0 ? rcPos.left : rcPos.right, rcPos.top >= 0 ? rcPos.top : rcPos.bottom};
			SetFixedXY(szXY);
			SetFixedWidth(abs(rcPos.right - rcPos.left));
			SetFixedHeight(abs(rcPos.bottom - rcPos.top));
		}
		else if( _tcsicmp(pstrName, _T("float")) == 0 ) {
			CDuiString nValue = pstrValue;
			// 鍔ㄦ€佽绠楃浉瀵规瘮渚?
			if(nValue.Find(',') < 0) {
				SetFloat(_tcsicmp(pstrValue, _T("true")) == 0);
			}
			else {
				TPercentInfo piFloatPercent = { 0 };
				LPTSTR pstr = NULL;
				piFloatPercent.left = _tcstod(pstrValue, &pstr);  ASSERT(pstr);
				piFloatPercent.top = _tcstod(pstr + 1, &pstr);    ASSERT(pstr);
				piFloatPercent.right = _tcstod(pstr + 1, &pstr);  ASSERT(pstr);
				piFloatPercent.bottom = _tcstod(pstr + 1, &pstr); ASSERT(pstr);
				SetFloatPercent(piFloatPercent);
				SetFloat(true);
			}
		}
		else if( _tcsicmp(pstrName, _T("floatalign")) == 0) {
			UINT uAlign = GetFloatAlign();
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
			SetFloatAlign(uAlign);
		}
		else if( _tcsicmp(pstrName, _T("halign")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("left")) == 0 ) SetHAlign(DT_LEFT);
			else if( _tcsicmp(pstrValue, _T("center")) == 0 ) SetHAlign(DT_CENTER);
			else if( _tcsicmp(pstrValue, _T("right")) == 0 ) SetHAlign(DT_RIGHT);
		}
		else if( _tcsicmp(pstrName, _T("valign")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("top")) == 0 ) SetVAlign(DT_TOP);
			else if( _tcsicmp(pstrValue, _T("vcenter")) == 0 ) SetVAlign(DT_VCENTER);
			else if( _tcsicmp(pstrValue, _T("bottom")) == 0 ) SetVAlign(DT_BOTTOM);
		}
		else if( _tcsicmp(pstrName, _T("margin")) == 0 ) {
			// CSS margin → 相对父级外边距（根节点相对窗口客户区）；RECT 顺序 left,top,right,bottom
			RECT rcMargin = { 0 };
			LPTSTR pstr = NULL;
			long v0 = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			if( pstr && (*pstr == _T(',') || *pstr == _T(' ')) ) {
				rcMargin.left = v0;
				rcMargin.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				rcMargin.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				rcMargin.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			}
			else {
				rcMargin.left = rcMargin.top = rcMargin.right = rcMargin.bottom = v0;
			}
			SetPadding(rcMargin);
		}
		else if( _tcsicmp(pstrName, _T("padding")) == 0 || _tcsicmp(pstrName, _T("inset")) == 0 ) {
			// CSS padding / inset → 内边距（内容区相对控件边框）
			RECT rcInset = { 0 };
			LPTSTR pstr = NULL;
			long v0 = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			if( pstr && (*pstr == _T(',') || *pstr == _T(' ')) ) {
				rcInset.left = v0;
				rcInset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				rcInset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				rcInset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			}
			else {
				rcInset.left = rcInset.top = rcInset.right = rcInset.bottom = v0;
			}
			SetInset(rcInset);
		}
		else if( _tcsicmp(pstrName, _T("gradient")) == 0 ) SetGradient(pstrValue);
		else if( _tcsicmp(pstrName, _T("bkcolor")) == 0 || _tcsicmp(pstrName, _T("bkcolor1")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetBkColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("bkcolor2")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetBkColor2(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("bkcolor3")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetBkColor3(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("forecolor")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetForeColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("bordercolor")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetBorderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("focusbordercolor")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetFocusBorderColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("colorhsl")) == 0 ) SetColorHSL(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("border")) == 0 ) {
			// HTML/CSS 简写：border="1px solid #DDD"；顺序可任意；none/0 清除
			ApplyBorderShorthand(this, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("bordersize")) == 0 ) {
			CDuiString nValue = pstrValue;
			if(nValue.Find(',') < 0) {
				SetBorderSize(_ttoi(pstrValue));
				RECT rcPadding = {0};
				SetBorderSize(rcPadding);
			}
			else {
				RECT rcPadding = { 0 };
				LPTSTR pstr = NULL;
				rcPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
				rcPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				rcPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				rcPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
				SetBorderSize(rcPadding);
			}
		}
		else if( _tcsicmp(pstrName, _T("leftbordersize")) == 0 ) SetLeftBorderSize(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("topbordersize")) == 0 ) SetTopBorderSize(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("rightbordersize")) == 0 ) SetRightBorderSize(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("bottombordersize")) == 0 ) SetBottomBorderSize(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("borderstyle")) == 0 ) {
			int nStyle = PS_SOLID;
			bool bNone = false;
			if( MapBorderStyleKeyword(pstrValue, nStyle, bNone) ) {
				if( bNone ) {
					RECT rcClear = { 0, 0, 0, 0 };
					SetBorderSize(0);
					SetBorderSize(rcClear);
				}
				else SetBorderStyle(nStyle);
			}
			else SetBorderStyle(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("borderround")) == 0 ) {
			SIZE cxyRound = { 0 };
			LPTSTR pstr = NULL;
			cxyRound.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			cxyRound.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			SetBorderRound(cxyRound);
		}
		else if( _tcsicmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
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
		else if( _tcsicmp(pstrName, _T("minwidth")) == 0 ) SetMinWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("minheight")) == 0 ) SetMinHeight(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("maxwidth")) == 0 ) SetMaxWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("maxheight")) == 0 ) SetMaxHeight(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("name")) == 0 || _tcsicmp(pstrName, _T("id")) == 0 ) SetName(pstrValue);
		else if( _tcsicmp(pstrName, _T("drag")) == 0 ) SetDragEnable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("drop")) == 0 ) SetDropEnable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("resourcetext")) == 0 ) SetResourceText(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("richevent")) == 0 ) SetRichEvent(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("text")) == 0 ) SetText(pstrValue);
		else if( _tcsicmp(pstrName, _T("tooltip")) == 0 ) SetToolTip(pstrValue);
		else if( _tcsicmp(pstrName, _T("userdata")) == 0 ) SetUserData(pstrValue);
		else if( _tcsicmp(pstrName, _T("enabled")) == 0 ) SetEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("mouse")) == 0 ) SetMouseEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("keyboard")) == 0 ) SetKeyboardEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("visible")) == 0 ) SetVisible(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("float")) == 0 ) SetFloat(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("shortcut")) == 0 ) SetShortcut(pstrValue[0]);
		else if( _tcsicmp(pstrName, _T("menu")) == 0 ) SetContextMenuUsed(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("cursor")) == 0 && pstrValue) {
			if( _tcsicmp(pstrValue, _T("arrow")) == 0 )			SetCursor(DUI_ARROW);
			else if( _tcsicmp(pstrValue, _T("ibeam")) == 0 )	SetCursor(DUI_IBEAM);
			else if( _tcsicmp(pstrValue, _T("wait")) == 0 )		SetCursor(DUI_WAIT);
			else if( _tcsicmp(pstrValue, _T("cross")) == 0 )	SetCursor(DUI_CROSS);
			else if( _tcsicmp(pstrValue, _T("uparrow")) == 0 )	SetCursor(DUI_UPARROW);
			else if( _tcsicmp(pstrValue, _T("size")) == 0 )		SetCursor(DUI_SIZE);
			else if( _tcsicmp(pstrValue, _T("icon")) == 0 )		SetCursor(DUI_ICON);
			else if( _tcsicmp(pstrValue, _T("sizenwse")) == 0 )	SetCursor(DUI_SIZENWSE);
			else if( _tcsicmp(pstrValue, _T("sizenesw")) == 0 )	SetCursor(DUI_SIZENESW);
			else if( _tcsicmp(pstrValue, _T("sizewe")) == 0 )	SetCursor(DUI_SIZEWE);
			else if( _tcsicmp(pstrValue, _T("sizens")) == 0 )	SetCursor(DUI_SIZENS);
			else if( _tcsicmp(pstrValue, _T("sizeall")) == 0 )	SetCursor(DUI_SIZEALL);
			else if( _tcsicmp(pstrValue, _T("no")) == 0 )		SetCursor(DUI_NO);
			else if( _tcsicmp(pstrValue, _T("hand")) == 0 )		SetCursor(DUI_HAND);
		}
		else if( _tcsicmp(pstrName, _T("virtualwnd")) == 0 ) SetVirtualWnd(pstrValue);
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
		SIZE cxyBorderRound = GetBorderRound();
		RECT rcBorderSize = GetBorderRectSize();

		if( cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0 ) {
			CRenderClipScope roundClip(ctx, m_rcPaint, m_rcItem, cxyBorderRound.cx, cxyBorderRound.cy);
			PaintBkColor(ctx);
			PaintBkImage(ctx);
			PaintStatusImage(ctx);
			PaintForeColor(ctx);
			PaintForeImage(ctx);
			PaintText(ctx);
			PaintBorder(ctx);
		}
		else {
			PaintBkColor(ctx);
			PaintBkImage(ctx);
			PaintStatusImage(ctx);
			PaintForeColor(ctx);
			PaintForeImage(ctx);
			PaintText(ctx);
			PaintBorder(ctx);
		}
		return true;
	}

	void CControlUI::PaintBkColor(IRenderContext& ctx)
	{
		if( m_dwBackColor == 0 ) return;


		bool bVer = (m_sGradient.CompareNoCase(_T("hor")) != 0);
		if( m_dwBackColor2 != 0 ) {
			if( m_dwBackColor3 != 0 ) {
				RECT rc = m_rcItem;
				rc.bottom = (rc.bottom + rc.top) / 2;
				ctx.DrawGradient(rc, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 8);
				rc.top = rc.bottom;
				rc.bottom = m_rcItem.bottom;
				ctx.DrawGradient(rc, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), bVer, 8);
			}
			else {
				ctx.DrawGradient(m_rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 16);
			}
		}
		else {
			DWORD color = GetAdjustColor(m_dwBackColor);
			// 有 BorderRound 时 DoPaint 已 PushRoundClip：这里用直角填充，
			// 避免 FillRoundRect 与 clip 几何不一致在角上漏出灰/透明底。
			// 按钮等自行 override PaintBkColor 仍可用 FillRoundRect。
			if( m_dwBackColor >= 0xFF000000 ) ctx.DrawColor(m_rcPaint, color);
			else ctx.DrawColor(m_rcItem, color);
		}
	}

	void CControlUI::PaintBkImage(IRenderContext& ctx)
	{
		if( m_sBkImage.IsEmpty() ) return;
		DrawImage(ctx, (LPCTSTR)m_sBkImage);
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
	
	void CControlUI::PaintForeImage(IRenderContext& ctx)
	{
		if( m_sForeImage.IsEmpty() ) return;
		DrawImage(ctx, (LPCTSTR)m_sForeImage);
	}

	void CControlUI::PaintText(IRenderContext& ctx)
	{
		return;
	}

	void CControlUI::PaintBorder(IRenderContext& ctx)
	{
		int nBorderSize = GetBorderSize();
		SIZE cxyBorderRound = GetBorderRound();
		RECT rcBorderSize = GetBorderRectSize();
		
		if(m_dwBorderColor != 0 || m_dwFocusBorderColor != 0) {

			//画圆角边框
			if(nBorderSize > 0 && ( cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0 )) {
				if (IsFocused() && m_dwFocusBorderColor != 0)
					ctx.DrawRoundRect(m_rcItem, nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(m_dwFocusBorderColor), m_nBorderStyle);
				else
					ctx.DrawRoundRect(m_rcItem, nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
			}
			else {
				if (IsFocused() && m_dwFocusBorderColor != 0 && nBorderSize > 0) { 
					ctx.DrawRect(m_rcItem, nBorderSize, GetAdjustColor(m_dwFocusBorderColor), m_nBorderStyle);
				}
				else if(rcBorderSize.left > 0 || rcBorderSize.top > 0 || rcBorderSize.right > 0 || rcBorderSize.bottom > 0) {
					RECT rcBorder;

					if(rcBorderSize.left > 0){
						rcBorder		= m_rcItem;
						rcBorder.right	= rcBorder.left;
						ctx.DrawLine(rcBorder,rcBorderSize.left,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
					if(rcBorderSize.top > 0){
						rcBorder		= m_rcItem;
						rcBorder.bottom	= rcBorder.top;
						ctx.DrawLine(rcBorder,rcBorderSize.top,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
					if(rcBorderSize.right > 0){
						rcBorder		= m_rcItem;
						rcBorder.right -= 1;
						rcBorder.left	= rcBorder.right;
						ctx.DrawLine(rcBorder,rcBorderSize.right,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
					if(rcBorderSize.bottom > 0){
						rcBorder		= m_rcItem;
						rcBorder.bottom -= 1;
						rcBorder.top	= rcBorder.bottom;
						ctx.DrawLine(rcBorder,rcBorderSize.bottom,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
				}
				else if(nBorderSize > 0) {
					ctx.DrawRect(m_rcItem, nBorderSize, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
				}
			}
		}
	}

	void CControlUI::DoPostPaint(IRenderContext& ctx, const RECT& rcPaint)
	{
		return;
	}

	int CControlUI::GetLeftBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderSize);

		return rcBorderSize.left;
	}

	void CControlUI::SetLeftBorderSize( int nSize )
	{
		m_rcBorderSize.left = nSize;
		Invalidate();
	}

	int CControlUI::GetTopBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderSize);

		return rcBorderSize.top;
	}

	void CControlUI::SetTopBorderSize( int nSize )
	{
		m_rcBorderSize.top = nSize;
		Invalidate();
	}

	int CControlUI::GetRightBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderSize);

		return rcBorderSize.right;
	}

	void CControlUI::SetRightBorderSize( int nSize )
	{
		m_rcBorderSize.right = nSize;
		Invalidate();
	}

	int CControlUI::GetBottomBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcBorderSize);

		return rcBorderSize.bottom;
	}

	void CControlUI::SetBottomBorderSize( int nSize )
	{
		m_rcBorderSize.bottom = nSize;
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
			SetBkColor(0);
			SetBorderColor(0);
			SetForeColor(0);
			SetBorderSize(0);
			SIZE round = {0, 0};
			SetBorderRound(round);
			Invalidate();
			return;
		}

		if (m_bOutline) {
			SetOutline(true);
			return;
		}

		int idx = (int)kind;
		const KindStateColors& normal = g_kindColors[idx].Normal;

		SetBkColor(normal.dwBkColor);
		SetBorderColor(normal.dwBorderColor);

		// kind 按钮默认圆角（link 无背景框，不加）
		// GDI RoundRect 的 width/height 为椭圆直径；12 → 半径 6px，接近 Bootstrap --bs-border-radius
		if (kind != CONTROLKIND_LINK) {
			SIZE round = {12, 12};
			SetBorderRound(round);
		}
		if (normal.dwBorderColor != 0) {
			SetBorderSize(1);
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
			DWORD outlineColor = g_kindColors[idx].Normal.dwBkColor;
			if (outlineColor == 0) outlineColor = g_kindColors[idx].Normal.dwTextColor;
			if (m_controlKind == CONTROLKIND_LIGHT) outlineColor = 0xFF212529;

			SetBkColor(0);
			SetForeColor(outlineColor);
			SetBorderColor(outlineColor);
			SetBorderSize(1);
			SIZE round = {12, 12};
			SetBorderRound(round);
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
