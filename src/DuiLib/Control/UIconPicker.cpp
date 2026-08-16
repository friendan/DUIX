#include "StdAfx.h"
#include "UIconPicker.h"
#include "Icons/UIconLibrary.h"
#include "Control/UISvgBox.h"

namespace DuiLib
{

namespace {

	/// 工程固定 UNICODE，只用宽字面量（见 StdAfx.h / AGENTS.md）。
	/// 内嵌皮肤：使用者无需再附带 iconpicker_popup.html；以 '<' 开头，DialogBuilder 按内联 XML 加载。
	LPCTSTR GetBuiltinIconPickerSkin()
	{
		return LR"dui(<html theme="chrome">
  <VBox name="root" gap="0">
    <TitleBar name="titlebar" title="选择图标" height="36"
        show-min="true" show-max="true" show-close="true" action="title" />
    <VBox padding="10,10,10,10" gap="8" flexible="true">
      <HBox height="34" gap="8" align-items="vcenter">
        		<Edit name="search" placeholder="搜索图标名…" width="168" height="26"
            border-width="1" border-radius="4" padding="4,2,4,2" />
        <HBox name="sizeGrp" gap="4" align-items="vcenter">
          <HBox name="comboSizeGrp" width="212" gap="4" align-items="vcenter">
            <Label text="宽" theme="secondary" width="20" align="right" />
            <Combo name="sizeComboW" width="76" height="26" itemtextcolor="#333333"
                itemheight="26" text-align="center" item-text-align="center" />
            <Label text="高" theme="secondary" width="20" align="right" />
            <Combo name="sizeComboH" width="76" height="26" itemtextcolor="#333333"
                itemheight="26" text-align="center" item-text-align="center" />
          </HBox>
          <HBox name="editSizeGrp" width="164" gap="4" align-items="vcenter" visible="false">
            <Label text="宽" theme="secondary" width="20" align="right" />
            <Edit name="sizeW" width="56" height="26" align="center"
                border-width="1" border-radius="4" padding="4,0,4,0" />
            <Label text="高" theme="secondary" width="20" align="right" />
            <Edit name="sizeH" width="56" height="26" align="center"
                border-width="1" border-radius="4" padding="4,0,4,0" />
          </HBox>
          <Button name="btnSizeMode" text="自定义宽高" width="80" height="26" kind="default" font-size="12" />
        </HBox>
        <Control flexible="true" />
        <Button name="btn_ok" text="确定" kind="primary" width="72" height="28" />
        <Button name="btn_cancel" text="取消" kind="default" width="72" height="28" />
      </HBox>
      <HBox name="colorGrp" height="30" gap="6" align-items="vcenter">
        <Label text="颜色" theme="secondary" width="34" />
        <Button name="color_none" text="无" width="40" height="24" kind="default" font-size="12" />
        <Button name="color_green" theme="none" kind="none" width="26" height="24" background-color="#00FF00" border-width="1" border-color="#00000044" round="3" />
        <Button name="color_blue" theme="none" kind="none" width="26" height="24" background-color="#0000FF" border-width="1" border-color="#00000044" round="3" />
        <Button name="color_cyan" theme="none" kind="none" width="26" height="24" background-color="#00FFFF" border-width="1" border-color="#00000044" round="3" />
        <Button name="color_purple" theme="none" kind="none" width="26" height="24" background-color="#FF00FF" border-width="1" border-color="#00000044" round="3" />
        <Button name="color_orange" theme="none" kind="none" width="26" height="24" background-color="#FF8000" border-width="1" border-color="#00000044" round="3" />
        <Button name="color_red" theme="none" kind="none" width="26" height="24" background-color="#FF0000" border-width="1" border-color="#00000044" round="3" />
        <Button name="color_custom" text="自定义" width="56" height="24" kind="default" font-size="12" />
        <Control flexible="true" />
      </HBox>
      <HBox name="customColorPanel" height="96" gap="8" theme="panel" padding="8,8,8,8" align-items="vcenter" visible="false">
        <ColorPalette name="colorPicker" width="150" height="80" palette-height="60" bar-height="14"
            select-color="#1677FF" />
        <VBox gap="4" flexible="true">
          <Label text="自定义图标颜色：下方选色或直接输入" theme="secondary" height="18" align="vcenter" />
          <HBox gap="6" align-items="vcenter">
            <Label name="colorPreview" width="40" height="24" theme="panel" border-width="1" border-color="#44000000" round="3" background-color="#1677FF" />
            <Edit name="colorEdit" width="96" height="24" align="center" placeholder="#RRGGBB"
                border-width="1" border-radius="4" padding="4,0,4,0" font-size="12" />
            <Button name="colorApply" text="应用" kind="primary" width="48" height="24" font-size="12" />
            <Button name="colorClose" text="收起" kind="default" width="48" height="24" font-size="12" />
            <Control flexible="true" />
          </HBox>
        </VBox>
      </HBox>
      <HBox gap="10" flexible="true">
        <VBox name="libList" width="168" overflow="auto" theme="panel" padding="6,6,6,6" gap="4" />
        <VBox flexible="true" gap="6" theme="panel" padding="8,8,8,8">
          <FlowLayout name="iconGrid" overflow="auto" flexible="true" gap="6" />
        </VBox>
      </HBox>
    </VBox>
    <HBox name="statusbar" height="32" padding="10,2,10,2" gap="8" align-items="vcenter"
        border-width="1,0,0,0" border-color="var(--color-titlebar-border)"
        background-color="var(--color-titlebar-bg)">
      <Label name="status" flex="1" align="vcenter" font-size="12"
          color="var(--color-titlebar-text)" />
    </HBox>
  </VBox>
</html>
<style>
  html {
    size: 960,530;
    caption: 0,0,0,36;
    min-size: 640,420;
  }
</style>
)dui";
	}

}

/////////////////////////////////////////////////////////////////////////////////////
// CIconPickerUI

IMPLEMENT_DUICONTROL(CIconPickerUI)

CIconPickerUI::CIconPickerUI()
	: m_bModal(false)
	, m_bShowSize(true)
	, m_bShowColor(true)
	, m_nSizeMin(8)
	, m_nSizeMax(256)
	, m_nIconW(18)
	, m_nIconH(18)
	, m_dwIconColor(0)
{
	// 默认：lucide 网格图标；无字，靠 tooltip 提示
	SetText(_T(""));
	SetKind(CONTROLKIND_NONE);
	SetBackgroundColor(0);
	SetToolTip(_T("选择图标"));
	SetIconLib(_T("lucide"), _T("grid-2x2"));
	CButtonUI::SetIconSize(18);
}

LPCTSTR CIconPickerUI::GetClass() const
{
	return _T("IconPickerUI");
}

LPVOID CIconPickerUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcsicmp(pstrName, DUI_CTR_ICONPICKER) == 0 ) return static_cast<CIconPickerUI*>(this);
	if( _tcsicmp(pstrName, _T("IconPicker")) == 0 ) return static_cast<CIconPickerUI*>(this);
	return CButtonUI::GetInterface(pstrName);
}

void CIconPickerUI::SetLibrary(LPCTSTR pstrLib)
{
	m_sIconLib = pstrLib ? pstrLib : _T("");
	SyncIcon();
}

LPCTSTR CIconPickerUI::GetLibrary() const
{
	return m_sIconLib.GetData();
}

void CIconPickerUI::SetSelectedIcon(LPCTSTR pstrName)
{
	m_sIconName = pstrName ? pstrName : _T("");
	SyncIcon();
}

	LPCTSTR CIconPickerUI::GetSelectedIcon() const
	{
		return m_sIconName.GetData();
	}

	void CIconPickerUI::SetDefaultLibrary(LPCTSTR pstrLib)
	{
		m_sDefaultLib = pstrLib ? pstrLib : _T("");
	}

	LPCTSTR CIconPickerUI::GetDefaultLibrary() const
	{
		return m_sDefaultLib.GetData();
	}

	void CIconPickerUI::SetShowSizeSettings(bool bShow)
	{
		m_bShowSize = bShow;
	}

	bool CIconPickerUI::IsShowSizeSettings() const
	{
		return m_bShowSize;
	}

	void CIconPickerUI::SetShowColorSettings(bool bShow)
	{
		m_bShowColor = bShow;
	}

	bool CIconPickerUI::IsShowColorSettings() const
	{
		return m_bShowColor;
	}

void CIconPickerUI::SetLibrariesFilter(LPCTSTR pstrLibs)
{
	m_sLibsFilter = pstrLibs ? pstrLibs : _T("");
}

LPCTSTR CIconPickerUI::GetLibrariesFilter() const
{
	return m_sLibsFilter.GetData();
}

bool CIconPickerUI::IsLibListed(LPCTSTR lib) const
{
	if( lib == NULL || *lib == _T('\0') ) return false;
	if( m_sLibsFilter.IsEmpty() ) return true;
	CDuiString filter = m_sLibsFilter;
	filter.MakeLower();
	CDuiString one = lib;
	one.MakeLower();
	int start = 0;
	while( start <= filter.GetLength() ) {
		int comma = filter.Find(_T(','), start);
		CDuiString part = (comma < 0) ? filter.Mid(start) : filter.Mid(start, comma - start);
		part.TrimLeft();
		part.TrimRight();
		if( part == one ) return true;
		if( comma < 0 ) break;
		start = comma + 1;
	}
	return false;
}

void CIconPickerUI::SetModal(bool bModal)
{
	m_bModal = bModal;
}

bool CIconPickerUI::IsModal() const
{
	return m_bModal;
}

void CIconPickerUI::SetSizeRange(int nMin, int nMax)
{
	if( nMin < 1 ) nMin = 1;
	if( nMax > 1024 ) nMax = 1024;   // 库层面再放宽：硬上限防明显误配置
	if( nMax < nMin ) nMax = nMin;
	m_nSizeMin = nMin;
	m_nSizeMax = nMax;
}

int CIconPickerUI::GetSizeMin() const
{
	return m_nSizeMin;
}

int CIconPickerUI::GetSizeMax() const
{
	return m_nSizeMax;
}

void CIconPickerUI::SetIconColor(DWORD dwColor)
{
	m_dwIconColor = dwColor;
	SyncIcon();
}

DWORD CIconPickerUI::GetIconColor() const
{
	return m_dwIconColor;
}

void CIconPickerUI::SetIconSize(int nW, int nH)
{
	m_nIconW = nW;
	m_nIconH = nH;
	// 触发按钮以正方形渲染 SVG，取较宽一侧为主尺寸
	CButtonUI::SetIconSize(nW > nH ? nW : nH);
}

int CIconPickerUI::GetIconWidth() const
{
	return m_nIconW;
}

int CIconPickerUI::GetIconHeight() const
{
	return m_nIconH;
}

void CIconPickerUI::SyncIcon()
{
	if( !m_sIconLib.IsEmpty() && !m_sIconName.IsEmpty() && IsIconAttr(m_sIconLib.GetData()) ) {
		SetIconLib(m_sIconLib.GetData(), m_sIconName.GetData());
		CButtonUI::SetIconSize(m_nIconW > m_nIconH ? m_nIconW : m_nIconH);
		if( m_dwIconColor != 0 ) {
			SetIconTintAuto(false);
			SetIconTint(m_dwIconColor);
		}
		else {
			SetIconTintAuto(true);
		}
	}
	else {
		ClearIcon();
	}
}

void CIconPickerUI::OpenPicker()
{
	HWND hOwner = NULL;
	if( m_pManager != NULL ) hOwner = m_pManager->GetPaintWindow();
	CIconPickerWnd::Open(hOwner, this, m_nSizeMin, m_nSizeMax, m_dwIconColor, m_bModal);
}

bool CIconPickerUI::Activate()
{
	OpenPicker();
	return true;
}

void CIconPickerUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcsicmp(pstrName, _T("iconlib")) == 0 ) {
		SetLibrary(pstrValue);
		return;
	}
	if( _tcsicmp(pstrName, _T("default-lib")) == 0 || _tcsicmp(pstrName, _T("default-library")) == 0 ) {
		SetDefaultLibrary(pstrValue);
		return;
	}
	if( _tcsicmp(pstrName, _T("icon")) == 0 || _tcsicmp(pstrName, _T("icon-name")) == 0 ) {
		SetSelectedIcon(pstrValue);
		return;
	}
	if( _tcsicmp(pstrName, _T("libs")) == 0 || _tcsicmp(pstrName, _T("libraries")) == 0 ) {
		SetLibrariesFilter(pstrValue);
		return;
	}
	if( _tcsicmp(pstrName, _T("library")) == 0 ) {
		SetLibrary(pstrValue);
		return;
	}
	if( _tcsicmp(pstrName, _T("modal")) == 0 ) {
		SetModal(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		return;
	}
	if( _tcsicmp(pstrName, _T("show-size")) == 0 || _tcsicmp(pstrName, _T("show-size-settings")) == 0 ) {
		SetShowSizeSettings(_tcsicmp(pstrValue, _T("false")) != 0 && _tcscmp(pstrValue, _T("0")) != 0);
		return;
	}
	if( _tcsicmp(pstrName, _T("show-color")) == 0 || _tcsicmp(pstrName, _T("show-color-settings")) == 0 ) {
		SetShowColorSettings(_tcsicmp(pstrValue, _T("false")) != 0 && _tcscmp(pstrValue, _T("0")) != 0);
		return;
	}
	if( _tcsicmp(pstrName, _T("size-min")) == 0 ) {
		int v = _ttoi(pstrValue);
		if( v > 0 ) SetSizeRange(v, m_nSizeMax);
		return;
	}
	if( _tcsicmp(pstrName, _T("size-max")) == 0 ) {
		int v = _ttoi(pstrValue);
		if( v > 0 ) SetSizeRange(m_nSizeMin, v);
		return;
	}
	if( _tcsicmp(pstrName, _T("icon-tint-color")) == 0 || _tcsicmp(pstrName, _T("icon-color")) == 0 ) {
		DWORD clr = 0;
		if( ParseColorString(pstrValue, clr) )
			SetIconColor(clr);
		return;
	}
	if( _tcsicmp(pstrName, _T("icon-width")) == 0 ) {
		int v = _ttoi(pstrValue);
		if( v > 0 ) SetIconSize(v, m_nIconH);
		return;
	}
	if( _tcsicmp(pstrName, _T("icon-height")) == 0 ) {
		int v = _ttoi(pstrValue);
		if( v > 0 ) SetIconSize(m_nIconW, v);
		return;
	}
	if( _tcsicmp(pstrName, _T("icon-size")) == 0 ) {
		int v = _ttoi(pstrValue);
		if( v > 0 ) SetIconSize(v, v);
		return;
	}
	CButtonUI::SetAttribute(pstrName, pstrValue);
}

void CIconPickerUI::DoInit()
{
	CButtonUI::DoInit();
	SyncIcon();
}

/////////////////////////////////////////////////////////////////////////////////////
// CIconPickerWnd

CIconPickerWnd* CIconPickerWnd::s_pActive = NULL;

DUI_BEGIN_MESSAGE_MAP(CIconPickerWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CIconPickerWnd::OnClick)
DUI_END_MESSAGE_MAP()

	CIconPickerWnd::CIconPickerWnd(CIconPickerUI* pOwner, LPCTSTR pstrLibsFilter, int nSizeMin, int nSizeMax, DWORD dwIconColor)
	: m_pOwner(pOwner)
	, m_sLibsFilter(pstrLibsFilter ? pstrLibsFilter : _T(""))
	, m_nIconW(32)
	, m_nIconH(32)
	, m_nSizeMin(nSizeMin)
	, m_nSizeMax(nSizeMax)
	, m_bModal(false)
	, m_pLibList(NULL)
	, m_pIconGrid(NULL)
	, m_pSearch(NULL)
	, m_pSizeComboW(NULL)
	, m_pSizeComboH(NULL)
	, m_pSizeW(NULL)
	, m_pSizeH(NULL)
	, m_pComboSizeGrp(NULL)
	, m_pEditSizeGrp(NULL)
	, m_pBtnSizeMode(NULL)
	, m_bCustomSize(false)
	, m_dwIconColor(dwIconColor)
	, m_pColorEdit(NULL)
	, m_pColorPicker(NULL)
	, m_pColorPreview(NULL)
	, m_pCustomColorPanel(NULL)
	, m_bCustomActive(false)
	, m_pStatus(NULL)
	, m_bShowSize(true)
	, m_bShowColor(true)
{
	if( m_nSizeMin < 1 ) m_nSizeMin = 1;
	if( m_nSizeMax < m_nSizeMin ) m_nSizeMax = m_nSizeMin;
	if( pOwner != NULL ) {
		// 打开时默认选中的分类：default-lib 指定的库；空 = 交由 RebuildLibList 回落第一个可见库。
		// 仅保留已选图标名用于网格高亮，不强制记忆触发按钮当前库。
		m_sCurLib = pOwner->GetDefaultLibrary();
		m_sSelectedName = pOwner->GetSelectedIcon();
		m_bShowSize = pOwner->IsShowSizeSettings();
		m_bShowColor = pOwner->IsShowColorSettings();
		// 用 owner 已存的图标尺寸作为初始值（若没有特别设置则为 32 默认）
		if( pOwner->GetIconWidth() > 0 ) m_nIconW = pOwner->GetIconWidth();
		if( pOwner->GetIconHeight() > 0 ) m_nIconH = pOwner->GetIconHeight();
		m_nIconW = ClampSize(m_nIconW);
		m_nIconH = ClampSize(m_nIconH);
	}
}

CIconPickerWnd::~CIconPickerWnd()
{
	if( s_pActive == this ) s_pActive = NULL;
}

void CIconPickerWnd::Open(HWND hOwner, CIconPickerUI* pPicker, int nSizeMin, int nSizeMax, DWORD dwIconColor, bool bModal)
{
	if( s_pActive != NULL && ::IsWindow(s_pActive->GetHWND()) ) {
		HWND hExist = s_pActive->GetHWND();
		if( ::IsIconic(hExist) )
			::ShowWindow(hExist, SW_RESTORE);
		::ShowWindow(hExist, SW_SHOW);
		::SetForegroundWindow(hExist);
		return;
	}
	CIconPickerWnd* pWnd = new CIconPickerWnd(pPicker, pPicker ? pPicker->GetLibrariesFilter() : _T(""), nSizeMin, nSizeMax, dwIconColor);
	pWnd->m_bModal = bModal;
	s_pActive = pWnd;
	// 非模态勿设 owner：Windows 规定 owned 窗永远压在 owner 之上，主窗无法前置对照预览
	HWND hCreateOwner = bModal ? hOwner : NULL;
	pWnd->Create(hCreateOwner, _T("选择图标"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 960, 530);
	if( !bModal && hOwner != NULL && ::IsWindow(hOwner) ) {
		RECT rcOwner = { 0 };
		RECT rcDlg = { 0 };
		::GetWindowRect(hOwner, &rcOwner);
		::GetWindowRect(pWnd->GetHWND(), &rcDlg);
		const int w = rcDlg.right - rcDlg.left;
		const int h = rcDlg.bottom - rcDlg.top;
		const int x = (rcOwner.left + rcOwner.right) / 2 - w / 2;
		const int y = (rcOwner.top + rcOwner.bottom) / 2 - h / 2;
		::SetWindowPos(pWnd->GetHWND(), NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else {
		pWnd->CenterWindow();
	}
	if( bModal )
		pWnd->ShowModal();
	else
		pWnd->ShowWindow(true, true);
}

void CIconPickerWnd::OnFinalMessage(HWND hWnd)
{
	if( s_pActive == this ) s_pActive = NULL;
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CIconPickerWnd::GetSkinFile()
{
	return GetBuiltinIconPickerSkin();
}

LPCTSTR CIconPickerWnd::GetWindowClassName() const
{
	return _T("IconPickerWnd");
}

static bool IsPresetSize(int v, int nMin, int nMax);   // 前向声明（定义在下方 ClampSize 之后）

void CIconPickerWnd::InitWindow()
{
	m_pLibList = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("libList")));
	m_pIconGrid = static_cast<CFlowLayoutUI*>(m_pm.FindControl(_T("iconGrid")));
	m_pSearch = static_cast<CEditUI*>(m_pm.FindControl(_T("search")));
	m_pSizeComboW = static_cast<CComboUI*>(m_pm.FindControl(_T("sizeComboW")));
	m_pSizeComboH = static_cast<CComboUI*>(m_pm.FindControl(_T("sizeComboH")));
	m_pSizeW = static_cast<CEditUI*>(m_pm.FindControl(_T("sizeW")));
	m_pSizeH = static_cast<CEditUI*>(m_pm.FindControl(_T("sizeH")));
	m_pComboSizeGrp = m_pm.FindControl(_T("comboSizeGrp"));
	m_pEditSizeGrp = m_pm.FindControl(_T("editSizeGrp"));
	m_pBtnSizeMode = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnSizeMode")));
	m_pColorEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("colorEdit")));
	m_pColorPicker = static_cast<CColorPaletteUI*>(m_pm.FindControl(_T("colorPicker")));
	m_pColorPreview = static_cast<CLabelUI*>(m_pm.FindControl(_T("colorPreview")));
	m_pCustomColorPanel = m_pm.FindControl(_T("customColorPanel"));
	m_pStatus = static_cast<CLabelUI*>(m_pm.FindControl(_T("status")));

	// 是否隐藏“大小设置”区 / “颜色自定义”区（固定宽高 / 固定颜色时不给使用者改）
	if( !m_bShowSize ) {
		CControlUI* pSize = m_pm.FindControl(_T("sizeGrp"));
		if( pSize ) pSize->SetVisible(false);
	}
	if( !m_bShowColor ) {
		CControlUI* pColor = m_pm.FindControl(_T("colorGrp"));
		if( pColor ) pColor->SetVisible(false);
	}

	BuildSizeCombos();
	ApplySwatchColors();
	SyncSizeCombos();
	SyncSizeEdits();
	// 初始模式：宽/高是预设值→预设(combo)，否则自定义(edit)
	SetSizeMode(!IsPresetPair());
	if( m_dwIconColor == 0 ) {
		SyncColorSwatches();
	} else {
		// 存在颜色：视为自定义色，展开面板并填入（用户可再选预设/无收起）
		m_bCustomActive = true;
		SetCustomColorPanel(true);
		SelectColor(m_dwIconColor);
	}
	RebuildLibList();
	RebuildIconGrid();
}

// 给两个尺寸下拉填预设列表（纯预设，不含自定义项）
void CIconPickerWnd::BuildSizeCombos()
{
	static const int kPresets[] = { 16, 24, 32, 48, 64, 96, 128 };
	CComboUI* combos[2] = { m_pSizeComboW, m_pSizeComboH };
	for( int c = 0; c < 2; ++c ) {
		CComboUI* pCombo = combos[c];
		if( pCombo == NULL ) continue;
		pCombo->RemoveAll();
		for( int i = 0; i < (int)(sizeof(kPresets) / sizeof(kPresets[0])); ++i ) {
			if( kPresets[i] < m_nSizeMin || kPresets[i] > m_nSizeMax ) continue;
			CListLabelElementUI* pEl = new CListLabelElementUI;
			CDuiString s;
			s.Format(_T("%d"), kPresets[i]);
			pEl->SetText(s.GetData());
			pEl->SetFixedHeight(26);
			pCombo->Add(pEl);
		}
	}
}

// 当前宽/高是否都是预设值
bool CIconPickerWnd::IsPresetPair() const
{
	return IsPresetSize(m_nIconW, m_nSizeMin, m_nSizeMax)
		&& IsPresetSize(m_nIconH, m_nSizeMin, m_nSizeMax);
}

// 让两个下拉选中与当前 W/H 对应的预设
void CIconPickerWnd::SyncSizeCombos()
{
	CComboUI* combos[2] = { m_pSizeComboW, m_pSizeComboH };
	const int vals[2] = { m_nIconW, m_nIconH };
	for( int c = 0; c < 2; ++c ) {
		CComboUI* pCombo = combos[c];
		if( pCombo == NULL ) continue;
		int n = pCombo->GetCount();
		int best = -1;
		for( int i = 0; i < n; ++i ) {
			CControlUI* it = pCombo->GetItemAt(i);
			if( it == NULL ) continue;
			if( _ttoi(it->GetText().GetData()) == vals[c] ) { best = i; break; }
		}
		pCombo->SelectItem(best >= 0 ? best : 0, false);
	}
}

// 在 预设(combo) 与 自定义(edit) 两种尺寸输入方式间切换（同一物理位置）
void CIconPickerWnd::SetSizeMode(bool bCustom)
{
	m_bCustomSize = bCustom;
	if( m_pComboSizeGrp != NULL ) m_pComboSizeGrp->SetVisible(!bCustom);
	if( m_pEditSizeGrp != NULL ) m_pEditSizeGrp->SetVisible(bCustom);
	if( m_pBtnSizeMode != NULL )
		m_pBtnSizeMode->SetText(bCustom ? _T("预设宽高") : _T("自定义宽高"));
	// 切到编辑模式时刷新两输入框
	if( bCustom ) SyncSizeEdits();
}

bool CIconPickerWnd::LibAllowed(LPCTSTR lib) const
{
	if( lib == NULL || *lib == _T('\0') ) return false;
	// 跟随 owner 白名单（运行中可即时生效）
	if( m_pOwner != NULL )
		return m_pOwner->IsLibListed(lib);
	if( m_sLibsFilter.IsEmpty() ) return true;
	CDuiString filter = m_sLibsFilter;
	filter.MakeLower();
	CDuiString one = lib;
	one.MakeLower();
	int start = 0;
	while( start <= filter.GetLength() ) {
		int comma = filter.Find(_T(','), start);
		CDuiString part = (comma < 0) ? filter.Mid(start) : filter.Mid(start, comma - start);
		part.TrimLeft();
		part.TrimRight();
		if( part == one ) return true;
		if( comma < 0 ) break;
		start = comma + 1;
	}
	return false;
}

void CIconPickerWnd::RebuildLibList()
{
	if( m_pLibList == NULL ) return;
	m_pLibList->RemoveAll();

	// 内置库：XML 属性 名 = CIconLibrary 库名
	static const TCHAR* const kLibs[] = {
		_T("bsicon"), _T("iconpark"), _T("lucide"), _T("remixicon"),
		_T("tabler-outline"), _T("tabler-filled"), _T("twicon")
	};

	bool bCurVisible = false;
	for( int i = 0; i < (int)(sizeof(kLibs) / sizeof(kLibs[0])); ++i ) {
		const TCHAR* lib = kLibs[i];
		if( !LibAllowed(lib) ) continue;
		if( CIconLibrary::GetIconCount(lib) <= 0 ) continue;
		CButtonUI* pBtn = new CButtonUI;
		CDuiString nm;
		nm.Format(_T("lib_item_%d"), i);
		pBtn->SetName(nm.GetData());
		pBtn->AddCustomAttribute(_T("lib-name"), lib);
		pBtn->SetText(lib);
		pBtn->SetFixedHeight(30);
		pBtn->SetPadding(CDuiBox(0, 6, 0, 6));
		pBtn->SetTextAlign(DT_LEFT | DT_VCENTER);
		bool bSel = (m_sCurLib.CompareNoCase(lib) == 0);
		if( bSel ) bCurVisible = true;
		pBtn->SetKind(bSel ? CONTROLKIND_PRIMARY : CONTROLKIND_DEFAULT);
		pBtn->SetAttribute(_T("align"), _T("left"));
		m_pLibList->Add(pBtn);
	}

	// 当前库不在白名单时，回落到第一个可见库
	if( m_pLibList->GetCount() > 0 && !bCurVisible ) {
		for( int i = 0; i < m_pLibList->GetCount(); ++i ) {
			CControlUI* pCtl = m_pLibList->GetItemAt(i);
			LPCTSTR lib = pCtl ? pCtl->GetCustomAttribute(_T("lib-name")) : NULL;
			if( lib != NULL && *lib != _T('\0') ) { SelectLib(lib); break; }
		}
	}
}

void CIconPickerWnd::SelectLib(LPCTSTR lib)
{
	if( lib == NULL || *lib == _T('\0') ) return;
	m_sCurLib = lib;
	m_sSelectedName.Empty();
	if( m_pSearch != NULL ) m_pSearch->SetText(_T(""));
	// 刷新左列高亮
	if( m_pLibList != NULL ) {
		for( int i = 0; i < m_pLibList->GetCount(); ++i ) {
			CControlUI* pCtl = m_pLibList->GetItemAt(i);
			CButtonUI* pBtn = static_cast<CButtonUI*>(pCtl ? pCtl->GetInterface(DUI_CTR_BUTTON) : NULL);
			LPCTSTR bLib = pCtl ? pCtl->GetCustomAttribute(_T("lib-name")) : NULL;
			if( pBtn != NULL )
				pBtn->SetKind((bLib != NULL && m_sCurLib.CompareNoCase(bLib) == 0) ? CONTROLKIND_PRIMARY : CONTROLKIND_DEFAULT);
		}
	}
	RebuildIconGrid();
}

void CIconPickerWnd::SetIconSize(int nW, int nH)
{
	m_nIconW = ClampSize(nW);
	m_nIconH = ClampSize(nH);
	SyncSizeCombos();
	SyncSizeEdits();
	RebuildIconGrid();
}

int CIconPickerWnd::ClampSize(int v) const
{
	if( v < m_nSizeMin ) return m_nSizeMin;
	if( v > m_nSizeMax ) return m_nSizeMax;
	return v;
}

static bool IsPresetSize(int v, int nMin, int nMax)
{
	static const int kPresets[] = { 16, 24, 32, 48, 64, 96, 128 };
	for( int i = 0; i < (int)(sizeof(kPresets) / sizeof(kPresets[0])); ++i )
		if( kPresets[i] == v && v >= nMin && v <= nMax ) return true;
	return false;
}

// 把纯色板常量填到各色块的填充（单一数据源：改常量即改块面与选中值）
void CIconPickerWnd::ApplySwatchColors()
{
	struct Sw { LPCTSTR name; DWORD color; };
	static const Sw kSw[] = {
		{ _T("color_green"),  DuiColor_Green },
		{ _T("color_blue"),   DuiColor_Blue },
		{ _T("color_cyan"),   DuiColor_Cyan },
		{ _T("color_purple"), DuiColor_Magenta },
		{ _T("color_orange"), DuiColor_Orange },
		{ _T("color_red"),    DuiColor_Red },
	};
	for( int i = 0; i < (int)(sizeof(kSw) / sizeof(kSw[0])); ++i ) {
		CButtonUI* pBtn = static_cast<CButtonUI*>(m_pm.FindControl(kSw[i].name));
		if( pBtn == NULL ) continue;
		pBtn->SetBackgroundColor(kSw[i].color);
	}
}

// 给某个网格按钮应用当前图标颜色（0=无→跟随主题默认）
void CIconPickerWnd::ApplyCellColor(CButtonUI* pBtn) const
{
	if( pBtn == NULL ) return;
	if( m_dwIconColor != 0 ) {
		pBtn->SetIconTintAuto(false);
		pBtn->SetIconTint(m_dwIconColor);
	}
	else {
		pBtn->SetIconTintAuto(true);
	}
}

// 只重上色网格现有按钮（不重建结构），用于调色板拖动中实时预览
void CIconPickerWnd::ReapplyGridCellColors()
{
	if( m_pIconGrid == NULL ) return;
	for( int i = 0; i < m_pIconGrid->GetCount(); ++i ) {
		CControlUI* pCtl = m_pIconGrid->GetItemAt(i);
		CButtonUI* pBtn = static_cast<CButtonUI*>(pCtl ? pCtl->GetInterface(DUI_CTR_BUTTON) : NULL);
		if( pBtn ) ApplyCellColor(pBtn);
	}
}

// 设置图标颜色并实时刷新网格预览
void CIconPickerWnd::SelectColor(DWORD dwColor)
{
	m_dwIconColor = dwColor;
	SyncCustomColorControls();
	SyncColorSwatches();
	RebuildIconGrid();
}

// 用当前 m_dwIconColor 同步自定义面板（色板/预览色块/hex 输入）
void CIconPickerWnd::SyncCustomColorControls()
{
	if( m_pColorPicker ) m_pColorPicker->SetSelectColor(m_dwIconColor ? m_dwIconColor : DuiColor_Black);
	if( m_pColorEdit ) {
		CDuiString s;
		s.Format(_T("#%02X%02X%02X"),
			DuiColorR(m_dwIconColor ? m_dwIconColor : DuiColor_Black),
			DuiColorG(m_dwIconColor ? m_dwIconColor : DuiColor_Black),
			DuiColorB(m_dwIconColor ? m_dwIconColor : DuiColor_Black));
		m_pColorEdit->SetText(s.GetData());
	}
	if( m_pColorPreview && m_dwIconColor != 0 )
		m_pColorPreview->SetBackgroundColor(m_dwIconColor);
}

// 显示/隐藏“自定义颜色”面板；显示时同步控件
void CIconPickerWnd::SetCustomColorPanel(bool bShow)
{
	if( m_pCustomColorPanel ) {
		if( m_pCustomColorPanel->IsVisible() == bShow ) return;
		m_pCustomColorPanel->SetVisible(bShow);
	}
	if( bShow ) {
		m_bCustomActive = true;
		SyncCustomColorControls();
	}
}

// 从 hex 输入框读取并应用自定义色；非法则忽略
void CIconPickerWnd::CommitHexColor()
{
	if( m_pColorEdit == NULL ) return;
	CDuiString s = m_pColorEdit->GetText();
	s.Trim();
	if( s.GetLength() == 7 && s[0] == _T('#') ) {
		DWORD clr = 0;
		if( ParseColorString(CDuiString(s).GetData(), clr) ) {
			m_bCustomActive = true;
			SelectColor(DuiColorSetA(clr, 0xFF));
		}
	}
}

// 高亮当前选中的颜色块/“自定义”/“无”
void CIconPickerWnd::SyncColorSwatches()
{
	// 预设色到按钮名的映射（颜色取自纯色板常量）
	struct Swatch { LPCTSTR name; DWORD color; };
	static const Swatch kSw[] = {
		{ _T("color_green"), DuiColor_Green },
		{ _T("color_blue"), DuiColor_Blue },
		{ _T("color_cyan"), DuiColor_Cyan },
		{ _T("color_purple"), DuiColor_Magenta },
		{ _T("color_orange"), DuiColor_Orange },
		{ _T("color_red"), DuiColor_Red },
	};
	bool bCustom = m_bCustomActive;
	for( int i = 0; i < (int)(sizeof(kSw) / sizeof(kSw[0])); ++i ) {
		CButtonUI* pBtn = static_cast<CButtonUI*>(m_pm.FindControl(kSw[i].name));
		if( pBtn == NULL ) continue;
		bool bSel = (bCustom ? false : (m_dwIconColor == kSw[i].color));
		// 色块：选中用粗边框高亮（kind=none 不会被主题填充覆盖）
		pBtn->SetBorderWidth(bSel ? 2 : 1);
		pBtn->SetBorderColor(bSel ? 0xFF1677FF : 0x44000000);
	}
	// “无”用 kind 高亮
	CButtonUI* pNone = static_cast<CButtonUI*>(m_pm.FindControl(_T("color_none")));
	if( pNone != NULL )
		pNone->SetKind((m_dwIconColor == 0) ? CONTROLKIND_PRIMARY : CONTROLKIND_DEFAULT);
	// “自定义”用 kind 高亮
	CButtonUI* pCustom = static_cast<CButtonUI*>(m_pm.FindControl(_T("color_custom")));
	if( pCustom != NULL )
		pCustom->SetKind(bCustom ? CONTROLKIND_PRIMARY : CONTROLKIND_DEFAULT);
}

// 把当前 W/H 写回两个输入框
void CIconPickerWnd::SyncSizeEdits()
{
	if( m_pSizeW != NULL ) {
		CDuiString s;
		s.Format(_T("%d"), m_nIconW);
		m_pSizeW->SetText(s.GetData());
	}
	if( m_pSizeH != NULL ) {
		CDuiString s;
		s.Format(_T("%d"), m_nIconH);
		m_pSizeH->SetText(s.GetData());
	}
}

// 从两个输入框读取；非法（<=0 / 非数字 / 过大）时回退到当前值并刷新输入框，不采纳
void CIconPickerWnd::ApplySizeInputs()
{
	int nW = m_nIconW;
	int nH = m_nIconH;
	if( m_pSizeW != NULL ) {
		CDuiString s = m_pSizeW->GetText();
		s.Trim();
		int v = _ttoi(s.GetData());
		if( v > 0 ) nW = v;
	}
	if( m_pSizeH != NULL ) {
		CDuiString s = m_pSizeH->GetText();
		s.Trim();
		int v = _ttoi(s.GetData());
		if( v > 0 ) nH = v;
	}
	m_nIconW = ClampSize(nW);
	m_nIconH = ClampSize(nH);
	SyncSizeCombos();
	SyncSizeEdits();
	RebuildIconGrid();
}

void CIconPickerWnd::ApplySearchFilter()
{
	// 直接重建当前库网格；搜索词在 RebuildIconGrid 里应用
	RebuildIconGrid();
}

void CIconPickerWnd::RebuildIconGrid()
{
	if( m_pIconGrid == NULL ) return;
	m_pIconGrid->RemoveAll();

	if( m_sCurLib.IsEmpty() ) return;

	// 搜索词
	CDuiString sFilter;
	if( m_pSearch != NULL ) {
		sFilter = m_pSearch->GetText();
		sFilter.Trim();
	}
	sFilter.MakeLower();

	const int nCount = CIconLibrary::GetIconCount(m_sCurLib.GetData());
	const int cellW = m_nIconW + 14;   // 格宽：图标 + 内边距
	const int cellH = m_nIconH + 14;   // 格高：图标 + 内边距
	int nShown = 0;
	int nMatched = 0;
	for( int i = 0; i < nCount; ++i ) {
		const wchar_t* name = CIconLibrary::GetNameByIndex(m_sCurLib.GetData(), i);
		if( name == NULL ) continue;
		++nShown;
		if( !sFilter.IsEmpty() ) {
			CDuiString low = name;
			low.MakeLower();
			if( low.Find(sFilter.GetData()) < 0 ) continue;
		}
		++nMatched;

		CButtonUI* pBtn = new CButtonUI;
		CDuiString nm;
		nm.Format(_T("icon_cell_%d"), i);
		pBtn->SetName(nm.GetData());
		pBtn->AddCustomAttribute(_T("icon-name"), name);
		pBtn->SetFixedWidth(cellW);
		pBtn->SetFixedHeight(cellH);
		pBtn->SetIconLib(m_sCurLib.GetData(), name);
		pBtn->SetIconSize(m_nIconW > m_nIconH ? m_nIconW : m_nIconH);
		ApplyCellColor(pBtn);
		bool bSel = (m_sSelectedName.CompareNoCase(name) == 0);
		pBtn->SetKind(bSel ? CONTROLKIND_PRIMARY : CONTROLKIND_NONE);
		pBtn->SetToolTip(name);
		m_pIconGrid->Add(pBtn);
	}

	if( m_pStatus != NULL ) {
		CDuiString s;
		if( !sFilter.IsEmpty() )
			s.Format(_T("%d/%d 个匹配"), nMatched, nShown);
		else
			s.Format(_T("%d 个图标"), nShown);
		m_pStatus->SetText(s.GetData());
	}
}

bool CIconPickerWnd::SelectIconByName(LPCTSTR name, bool bPreview)
{
	if( name == NULL || *name == _T('\0') || m_sCurLib.IsEmpty() ) return false;
	// 校验存在
	if( CIconLibrary::GetDataByName(m_sCurLib.GetData(), name) == NULL ) return false;
	m_sSelectedName = name;

	// 刷新网格选中高亮
	if( m_pIconGrid != NULL ) {
		for( int i = 0; i < m_pIconGrid->GetCount(); ++i ) {
			CControlUI* pCtl = m_pIconGrid->GetItemAt(i);
			CButtonUI* pBtn = static_cast<CButtonUI*>(pCtl ? pCtl->GetInterface(DUI_CTR_BUTTON) : NULL);
			const TCHAR* iconName = pCtl ? pCtl->GetCustomAttribute(_T("icon-name")) : NULL;
			if( pBtn != NULL )
				pBtn->SetKind((iconName != NULL && m_sSelectedName.CompareNoCase(iconName) == 0) ? CONTROLKIND_PRIMARY : CONTROLKIND_NONE);
		}
	}
	if( m_pStatus != NULL ) {
		CDuiString s;
		s.Format(_T("已选 %s"), name);
		m_pStatus->SetText(s.GetData());
	}
	return true;
}

void CIconPickerWnd::CommitAndClose(bool bOk)
{
	if( bOk ) {
		// 确定前把两个输入框里可能未按 Enter 的值校验并采纳
		ApplySizeInputs();
		if( m_pOwner != NULL ) {
			// 写回 owner：库 / 图标名 / 颜色 / 宽高（统一从 pSender 在 selectchanged 中读取）
			m_pOwner->SetLibrary(m_sCurLib.GetData());
			m_pOwner->SetSelectedIcon(m_sSelectedName.GetData());
			m_pOwner->SetIconColor(m_dwIconColor);
			m_pOwner->SetIconSize(m_nIconW, m_nIconH);
			// 通知应用
			if( m_pOwner->GetManager() != NULL )
				m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_SELECTCHANGED,
					(WPARAM)m_sCurLib.GetData(), (LPARAM)m_sSelectedName.GetData());
		}
	}
	Close(bOk ? IDOK : IDCANCEL);
}

void CIconPickerWnd::Notify(TNotifyUI& msg)
{
	if( msg.sType == DUI_MSGTYPE_TEXTCHANGED ) {
		if( msg.pSender == m_pSearch ) {
			ApplySearchFilter();
			return;
		}
	}
	if( msg.sType == DUI_MSGTYPE_RETURN ) {
		// 尺寸输入框回车：校验并应用
		if( msg.pSender == m_pSizeW || msg.pSender == m_pSizeH ) {
			ApplySizeInputs();
			return;
		}
		// 自定义颜色输入框回车：解析并应用
		if( msg.pSender == m_pColorEdit && m_pColorEdit != NULL ) {
			CommitHexColor();
			return;
		}
	}
	if( msg.sType == DUI_MSGTYPE_COLORCHANGED || msg.sType == DUI_MSGTYPE_COLORCHANGING ) {
		// 自定义色 HSL 调色板
		if( msg.pSender == m_pColorPicker && msg.wParam != 0 ) {
			m_bCustomActive = true;
			m_dwIconColor = (DWORD)msg.wParam;
			SyncCustomColorControls();
			SyncColorSwatches();
			if( msg.sType == DUI_MSGTYPE_COLORCHANGED )
				RebuildIconGrid();      // 松开鼠标：重建网格刷新图标颜色
			else
				ReapplyGridCellColors();  // 拖动中：只重上色已有格子，避免频繁重建
			return;
		}
	}
	if( msg.sType == DUI_MSGTYPE_ITEMSELECT ) {
		// 宽/高尺寸下拉：选中预设即设该维
		CComboUI* pCombo = (msg.pSender == m_pSizeComboW) ? m_pSizeComboW
			: (msg.pSender == m_pSizeComboH) ? m_pSizeComboH : NULL;
		if( pCombo != NULL ) {
			int sel = pCombo->GetCurSel();
			CControlUI* pItem = (sel >= 0) ? pCombo->GetItemAt(sel) : NULL;
			int v = (pItem != NULL) ? _ttoi(pItem->GetText().GetData()) : 0;
			if( v > 0 ) {
				if( pCombo == m_pSizeComboW ) SetIconSize(v, m_nIconH);
				else SetIconSize(m_nIconW, v);
			}
			return;
		}
	}
	WindowImplBase::Notify(msg);
}

void CIconPickerWnd::OnClick(TNotifyUI& msg)
{
	CControlUI* pSender = msg.pSender;
	if( pSender == NULL ) return;
	CDuiString sName = pSender->GetName();

	// 左列库按钮
	if( sName.Find(_T("lib_item_")) == 0 ) {
		LPCTSTR lib = pSender->GetCustomAttribute(_T("lib-name"));
		if( lib != NULL && *lib != _T('\0') )
			SelectLib(lib);
		return;
	}
	// 网格图标按钮
	if( sName.Find(_T("icon_cell_")) == 0 ) {
		LPCTSTR iconName = pSender->GetCustomAttribute(_T("icon-name"));
		if( iconName != NULL && *iconName != _T('\0') )
			SelectIconByName(iconName, true);
		return;
	}
	if( sName == _T("btnSizeMode") ) {
		// 在 预设(combo) 与 自定义(edit) 之间切换
		if( m_bCustomSize ) {
			// 切回预设：先采纳输入框值
			ApplySizeInputs();
			SetSizeMode(false);
		} else {
			SetSizeMode(true);
		}
		return;
	}
	// 颜色块：无 / 绿 / 蓝 / 青 / 紫 / 橙 / 红（值取自通用纯色常量）
	// 选预设/“无”即退出自定义模式并收起自定义面板。
	if( sName == _T("color_none")
		|| sName == _T("color_green") || sName == _T("color_blue") || sName == _T("color_cyan")
		|| sName == _T("color_purple") || sName == _T("color_orange") || sName == _T("color_red") ) {
		m_bCustomActive = false;
		SetCustomColorPanel(false);
		DWORD c = 0;
		if( sName == _T("color_green") ) c = DuiColor_Green;
		else if( sName == _T("color_blue") ) c = DuiColor_Blue;
		else if( sName == _T("color_cyan") ) c = DuiColor_Cyan;
		else if( sName == _T("color_purple") ) c = DuiColor_Magenta;
		else if( sName == _T("color_orange") ) c = DuiColor_Orange;
		else if( sName == _T("color_red") ) c = DuiColor_Red;
		SelectColor(c);
		return;
	}
	if( sName == _T("color_custom") ) {
		// 切换自定义颜色面板
		bool bOpen = (m_pCustomColorPanel && !m_pCustomColorPanel->IsVisible());
		if( bOpen && !m_bCustomActive ) {
			// 首次进入自定义模式：若无已定义颜色，先给一个默认(黑)作为起始预览但不应用
			m_bCustomActive = true;
			SyncCustomColorControls();
		}
		SetCustomColorPanel(bOpen);
		return;
	}
	if( sName == _T("colorApply") ) {
		// 应用 hex 输入为自定义色
		CommitHexColor();
		return;
	}
	if( sName == _T("colorClose") ) {
		SetCustomColorPanel(false);
		return;
	}
	if( sName == _T("btn_ok") ) {
		CommitAndClose(true);
		return;
	}
	if( sName == _T("btn_cancel") || sName == _T("closebtn") ) {
		CommitAndClose(false);
		return;
	}
	WindowImplBase::OnClick(msg);
}

} // namespace DuiLib
