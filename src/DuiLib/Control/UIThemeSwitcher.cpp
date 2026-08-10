#include "StdAfx.h"
#include "UIThemeSwitcher.h"
#include "Core/UITheme.h"
#include <commdlg.h>

namespace DuiLib
{

namespace {

	DWORD ThemeTok(LPCTSTR name, DWORD fb)
	{
		CThemeManager* tm = CThemeManager::GetInstance();
		return tm ? tm->GetColor(name, fb) : fb;
	}

	void FormatHex(CDuiString& s, DWORD c)
	{
		s.Format(_T("#%08X"), (unsigned)c);
	}

	/// 内置 token 用途说明（未知则按后缀推断）
	LPCTSTR GetTokenUsageDesc(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return _T("");
		struct Item { LPCTSTR key; LPCTSTR desc; };
		static const Item kMap[] = {
			{ _T("color-bg"), _T("窗口/页面背景色") },
			{ _T("color-bg-elevated"), _T("抬升面板背景（panel）") },
			{ _T("color-bg-hover"), _T("列表/菜单等悬停底色") },
			{ _T("color-text"), _T("正文文字色") },
			{ _T("color-text-secondary"), _T("次要/说明文字色") },
			{ _T("color-text-disabled"), _T("禁用文字色") },
			{ _T("color-disabled-text"), _T("禁用文字色") },
			{ _T("color-disabled-bg"), _T("禁用背景色") },
			{ _T("color-border"), _T("常规边框色") },
			{ _T("color-border-strong"), _T("强调边框色") },
			{ _T("color-control-bg"), _T("输入控件背景色") },
			{ _T("color-control-border"), _T("输入控件边框色") },
			{ _T("color-control-border-focus"), _T("输入控件聚焦边框") },
			{ _T("color-selection"), _T("选中项高亮底色") },
			{ _T("color-scrollbar-rail"), _T("滚动条轨道色") },
			{ _T("color-scrollbar-thumb"), _T("滚动条滑块色") },
			{ _T("color-scrollbar-thumb-hover"), _T("滚动条滑块悬停色") },
			{ _T("color-titlebar-bg"), _T("标题栏背景色") },
			{ _T("color-titlebar-text"), _T("标题栏文字/图标色") },
			{ _T("color-titlebar-border"), _T("标题栏底边框色") },
			{ _T("color-titlebar-close-hover"), _T("标题栏关闭按钮悬停") },
			{ _T("color-modal-bg"), _T("模态框背景色") },
			{ _T("color-modal-text"), _T("模态框文字色") },
			{ _T("color-modal-border"), _T("模态框边框色") },
			{ _T("color-primary"), _T("主色（按钮/强调）") },
			{ _T("color-primary-hover"), _T("主色悬停") },
			{ _T("color-primary-active"), _T("主色按下") },
			{ _T("color-primary-border"), _T("主色边框") },
			{ _T("color-primary-border-hover"), _T("主色边框悬停") },
			{ _T("color-primary-border-active"), _T("主色边框按下") },
			{ _T("color-primary-text"), _T("主色上的文字色") },
			{ _T("color-secondary"), _T("次要色") },
			{ _T("color-secondary-hover"), _T("次要色悬停") },
			{ _T("color-secondary-active"), _T("次要色按下") },
			{ _T("color-secondary-border-hover"), _T("次要色边框悬停") },
			{ _T("color-secondary-border-active"), _T("次要色边框按下") },
			{ _T("color-secondary-text"), _T("次要色上的文字色") },
			{ _T("color-default"), _T("默认按钮背景") },
			{ _T("color-default-border"), _T("默认按钮边框") },
			{ _T("color-default-text"), _T("默认按钮文字") },
			{ _T("color-default-hover"), _T("默认按钮悬停") },
			{ _T("color-default-border-hover"), _T("默认按钮边框悬停") },
			{ _T("color-default-active"), _T("默认按钮按下") },
			{ _T("color-default-border-active"), _T("默认按钮边框按下") },
			{ _T("color-success"), _T("成功/正向语义色") },
			{ _T("color-success-hover"), _T("成功色悬停") },
			{ _T("color-success-active"), _T("成功色按下") },
			{ _T("color-success-border-hover"), _T("成功色边框悬停") },
			{ _T("color-success-border-active"), _T("成功色边框按下") },
			{ _T("color-success-text"), _T("成功色上的文字") },
			{ _T("color-danger"), _T("危险/错误语义色") },
			{ _T("color-danger-hover"), _T("危险色悬停") },
			{ _T("color-danger-active"), _T("危险色按下") },
			{ _T("color-danger-border-hover"), _T("危险色边框悬停") },
			{ _T("color-danger-border-active"), _T("危险色边框按下") },
			{ _T("color-danger-text"), _T("危险色上的文字") },
			{ _T("color-warning"), _T("警告语义色") },
			{ _T("color-warning-hover"), _T("警告色悬停") },
			{ _T("color-warning-active"), _T("警告色按下") },
			{ _T("color-warning-border-hover"), _T("警告色边框悬停") },
			{ _T("color-warning-border-active"), _T("警告色边框按下") },
			{ _T("color-warning-text"), _T("警告色上的文字") },
			{ _T("color-info"), _T("信息语义色") },
			{ _T("color-info-hover"), _T("信息色悬停") },
			{ _T("color-info-active"), _T("信息色按下") },
			{ _T("color-info-border-hover"), _T("信息色边框悬停") },
			{ _T("color-info-border-active"), _T("信息色边框按下") },
			{ _T("color-info-text"), _T("信息色上的文字") },
			{ _T("color-light"), _T("浅色表面") },
			{ _T("color-light-hover"), _T("浅色悬停") },
			{ _T("color-light-active"), _T("浅色按下") },
			{ _T("color-light-border-hover"), _T("浅色边框悬停") },
			{ _T("color-light-border-active"), _T("浅色边框按下") },
			{ _T("color-light-text"), _T("浅色上的文字") },
			{ _T("color-dark"), _T("深色表面") },
			{ _T("color-dark-hover"), _T("深色悬停") },
			{ _T("color-dark-active"), _T("深色按下") },
			{ _T("color-dark-border-hover"), _T("深色边框悬停") },
			{ _T("color-dark-border-active"), _T("深色边框按下") },
			{ _T("color-dark-text"), _T("深色上的文字") },
			{ _T("color-link"), _T("链接文字色") },
			{ _T("color-link-hover"), _T("链接悬停色") },
			{ _T("color-link-active"), _T("链接按下色") },
			{ _T("color-rate"), _T("评分星星色") },
			{ _T("color-skeleton"), _T("骨架屏色块") },
			{ _T("color-skeleton-highlight"), _T("骨架屏高光") },
		};
		for( size_t i = 0; i < sizeof(kMap) / sizeof(kMap[0]); ++i ) {
			if( _tcsicmp(name, kMap[i].key) == 0 )
				return kMap[i].desc;
		}
		// 自定义 / 未收录：按常见后缀给短提示（长后缀优先）
		size_t len = _tcslen(name);
		if( len > 14 && _tcsicmp(name + len - 14, _T("-border-active")) == 0 ) return _T("边框按下");
		if( len > 13 && _tcsicmp(name + len - 13, _T("-border-hover")) == 0 ) return _T("边框悬停");
		if( len > 7 && _tcsicmp(name + len - 7, _T("-border")) == 0 ) return _T("边框色");
		if( len > 7 && _tcsicmp(name + len - 7, _T("-active")) == 0 ) return _T("按下态");
		if( len > 6 && _tcsicmp(name + len - 6, _T("-hover")) == 0 ) return _T("悬停态");
		if( len > 5 && _tcsicmp(name + len - 5, _T("-text")) == 0 ) return _T("文字色");
		return _T("自定义色值");
	}

	enum TokenGroupId {
		TOKEN_GROUP_TITLEBAR = 0,
		TOKEN_GROUP_WINDOW,
		TOKEN_GROUP_BUTTON,
		TOKEN_GROUP_TEXT,
		TOKEN_GROUP_CONTROL,
		TOKEN_GROUP_OTHER,
		TOKEN_GROUP_COUNT
	};

	LPCTSTR GetTokenGroupTitle(int group)
	{
		switch( group ) {
		case TOKEN_GROUP_TITLEBAR: return _T("标题栏");
		case TOKEN_GROUP_WINDOW:   return _T("窗口");
		case TOKEN_GROUP_BUTTON:   return _T("按钮 / 语义色");
		case TOKEN_GROUP_TEXT:     return _T("文本");
		case TOKEN_GROUP_CONTROL:  return _T("输入控件");
		default:                   return _T("其它");
		}
	}

	int GetTokenGroupId(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return TOKEN_GROUP_OTHER;
		if( _tcsnicmp(name, _T("color-titlebar"), 14) == 0 ) return TOKEN_GROUP_TITLEBAR;
		if( _tcsnicmp(name, _T("color-bg"), 8) == 0
			|| _tcsnicmp(name, _T("color-border"), 12) == 0
			|| _tcsnicmp(name, _T("color-selection"), 15) == 0
			|| _tcsnicmp(name, _T("color-scrollbar"), 15) == 0
			|| _tcsnicmp(name, _T("color-modal"), 11) == 0 )
			return TOKEN_GROUP_WINDOW;
		if( _tcsnicmp(name, _T("color-text"), 10) == 0
			|| _tcsicmp(name, _T("color-disabled-text")) == 0 )
			return TOKEN_GROUP_TEXT;
		if( _tcsnicmp(name, _T("color-control"), 13) == 0
			|| _tcsicmp(name, _T("color-disabled-bg")) == 0 )
			return TOKEN_GROUP_CONTROL;
		static const LPCTSTR kBtn[] = {
			_T("color-default"), _T("color-primary"), _T("color-secondary"),
			_T("color-success"), _T("color-danger"), _T("color-warning"),
			_T("color-info"), _T("color-light"), _T("color-dark"), _T("color-link")
		};
		for( size_t i = 0; i < sizeof(kBtn) / sizeof(kBtn[0]); ++i ) {
			size_t n = _tcslen(kBtn[i]);
			if( _tcsnicmp(name, kBtn[i], (int)n) == 0
				&& (name[n] == _T('\0') || name[n] == _T('-')) )
				return TOKEN_GROUP_BUTTON;
		}
		return TOKEN_GROUP_OTHER;
	}

	/// 展示顺序：常用分组在前，组内同类挨在一起（仅影响选择器列表，不改主题存储序）
	static const LPCTSTR kTokenDisplayOrder[] = {
		// 标题栏
		_T("color-titlebar-bg"), _T("color-titlebar-text"), _T("color-titlebar-border"),
		_T("color-titlebar-close-hover"),
		// 窗口
		_T("color-bg"), _T("color-bg-elevated"), _T("color-bg-hover"),
		_T("color-border"), _T("color-border-strong"),
		_T("color-selection"),
		_T("color-scrollbar-rail"), _T("color-scrollbar-thumb"), _T("color-scrollbar-thumb-hover"),
		_T("color-modal-bg"), _T("color-modal-text"), _T("color-modal-border"),
		// 按钮 / 语义色
		_T("color-default"), _T("color-default-hover"), _T("color-default-active"),
		_T("color-default-border"), _T("color-default-border-hover"), _T("color-default-border-active"),
		_T("color-default-text"),
		_T("color-primary"), _T("color-primary-hover"), _T("color-primary-active"),
		_T("color-primary-border"), _T("color-primary-border-hover"), _T("color-primary-border-active"),
		_T("color-primary-text"),
		_T("color-secondary"), _T("color-secondary-hover"), _T("color-secondary-active"),
		_T("color-secondary-border-hover"), _T("color-secondary-border-active"),
		_T("color-secondary-text"),
		_T("color-success"), _T("color-success-hover"), _T("color-success-active"),
		_T("color-success-border-hover"), _T("color-success-border-active"),
		_T("color-success-text"),
		_T("color-danger"), _T("color-danger-hover"), _T("color-danger-active"),
		_T("color-danger-border-hover"), _T("color-danger-border-active"),
		_T("color-danger-text"),
		_T("color-warning"), _T("color-warning-hover"), _T("color-warning-active"),
		_T("color-warning-border-hover"), _T("color-warning-border-active"),
		_T("color-warning-text"),
		_T("color-info"), _T("color-info-hover"), _T("color-info-active"),
		_T("color-info-border-hover"), _T("color-info-border-active"),
		_T("color-info-text"),
		_T("color-light"), _T("color-light-hover"), _T("color-light-active"),
		_T("color-light-border-hover"), _T("color-light-border-active"),
		_T("color-light-text"),
		_T("color-dark"), _T("color-dark-hover"), _T("color-dark-active"),
		_T("color-dark-border-hover"), _T("color-dark-border-active"),
		_T("color-dark-text"),
		_T("color-link"), _T("color-link-hover"), _T("color-link-active"),
		// 文本
		_T("color-text"), _T("color-text-secondary"), _T("color-text-disabled"),
		_T("color-disabled-text"),
		// 输入控件
		_T("color-control-bg"), _T("color-control-border"), _T("color-control-border-focus"),
		_T("color-disabled-bg"),
		// 其它
		_T("color-rate"), _T("color-skeleton"), _T("color-skeleton-highlight"),
	};

	/// 捕获阶段拦截子控件鼠标进出，整行统一 HOT（不依赖冒泡、不关 mouse-child）
	class CThemeTokenRowUI : public CHorizontalLayoutUI
	{
	public:
		LPCTSTR GetClass() const { return _T("ThemeTokenRow"); }
		void DoCaptureEvent(TEventUI& event)
		{
			if( event.Type == UIEVENT_MOUSEENTER ) {
				if( IsEnabled() ) {
					m_uControlState |= UISTATE_HOT;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSELEAVE ) {
				POINT pt = event.ptMouse;
				if( m_pManager != NULL ) pt = m_pManager->GetMousePos();
				if( !::PtInRect(&m_rcItem, pt) ) {
					m_uControlState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
			CHorizontalLayoutUI::DoCaptureEvent(event);
		}
	};

	/// C++11 raw string：内嵌皮肤，使用者无需再附带 themeswitcher_popup.html
	LPCTSTR GetBuiltinThemePickerSkin()
	{
#ifdef _UNICODE
		// Must start with '<' — DialogBuilder treats non-'<' as a file path (else ExitProcess).
		return LR"dui(<html theme="chrome">
  <VBox name="root" padding="10,10,10,10" gap="8">
    <TitleBar name="titlebar" title="选择主题" height="36"
        show-min="false" show-max="true" show-close="true" action="title" />
    <HBox height="36" gap="8" align-items="vcenter">
      <Button name="btn_new" text="新建主题" kind="default" width="88" height="28" />
      <Button name="btn_edit" text="编辑颜色" kind="default" width="88" height="28" />
      <Button name="btn_import" text="导入…" kind="default" width="72" height="28" />
      <Button name="btn_save" text="另存为…" kind="default" width="80" height="28" />
      <Control flexible="true" />
      <Button name="btn_ok" text="确定" kind="primary" width="72" height="28" />
      <Button name="btn_cancel" text="取消" kind="default" width="72" height="28" />
    </HBox>
    <Label name="editHint" text="编辑模式：点选色值行用调色板修改；双击右侧 #色值 可直接键入" height="22"
        theme="secondary" visible="false" />
    <HBox gap="10" flexible="true">
      <VBox name="themeList" width="168" overflow="auto" theme="panel" padding="6,6,6,6" gap="4" />
      <VBox flexible="true" gap="8">
        <HBox name="miniPreview" height="40" gap="6" padding="4,4,4,4" theme="panel" align-items="vcenter" />
        <VBox name="tokenList" overflow="auto" flexible="true" theme="panel" padding="4,4,4,4" gap="2" />
        <ColorPalette name="palette" height="180" palette-height="140" bar-height="16"
            select-color="#1677FFFF" visible="false" />
      </VBox>
    </HBox>
  </VBox>
</html>
<style>
  html {
    size: 860,560;
    caption: 0,0,0,36;
    min-size: 720,440;
  }
</style>
)dui";
#else
		return R"dui(<html theme="chrome">
  <VBox name="root" padding="10,10,10,10" gap="8">
    <TitleBar name="titlebar" title="选择主题" height="36"
        show-min="false" show-max="true" show-close="true" action="title" />
    <HBox height="36" gap="8" align-items="vcenter">
      <Button name="btn_new" text="新建主题" kind="default" width="88" height="28" />
      <Button name="btn_edit" text="编辑颜色" kind="default" width="88" height="28" />
      <Button name="btn_import" text="导入…" kind="default" width="72" height="28" />
      <Button name="btn_save" text="另存为…" kind="default" width="80" height="28" />
      <Control flexible="true" />
      <Button name="btn_ok" text="确定" kind="primary" width="72" height="28" />
      <Button name="btn_cancel" text="取消" kind="default" width="72" height="28" />
    </HBox>
    <Label name="editHint" text="编辑模式：点选色值行用调色板修改；双击右侧 #色值 可直接键入" height="22"
        theme="secondary" visible="false" />
    <HBox gap="10" flexible="true">
      <VBox name="themeList" width="168" overflow="auto" theme="panel" padding="6,6,6,6" gap="4" />
      <VBox flexible="true" gap="8">
        <HBox name="miniPreview" height="40" gap="6" padding="4,4,4,4" theme="panel" align-items="vcenter" />
        <VBox name="tokenList" overflow="auto" flexible="true" theme="panel" padding="4,4,4,4" gap="2" />
        <ColorPalette name="palette" height="180" palette-height="140" bar-height="16"
            select-color="#1677FFFF" visible="false" />
      </VBox>
    </HBox>
  </VBox>
</html>
<style>
  html {
    size: 860,560;
    caption: 0,0,0,36;
    min-size: 720,440;
  }
</style>
)dui";
#endif
	}

	bool PickOpenThemePath(HWND hOwner, CDuiString& sOut)
	{
		TCHAR szFile[MAX_PATH] = { 0 };
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hOwner;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = _T("Theme (*.css;*.theme)\0*.css;*.theme\0All\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if( !::GetOpenFileName(&ofn) ) return false;
		sOut = szFile;
		return true;
	}

	bool PickSaveThemePath(HWND hOwner, CDuiString& sOut)
	{
		TCHAR szFile[MAX_PATH] = { 0 };
		_tcscpy_s(szFile, _T("mytheme.css"));
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hOwner;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = _T("Theme (*.css)\0*.css\0All\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = _T("css");
		if( !::GetSaveFileName(&ofn) ) return false;
		sOut = szFile;
		return true;
	}

	CDuiString MakeUniqueThemeId(LPCTSTR base)
	{
		CThemeManager* tm = CThemeManager::GetInstance();
		CDuiString id = base && *base ? base : _T("custom");
		if( tm->FindTheme(id.GetData()) == NULL ) return id;
		for( int i = 2; i < 10000; ++i ) {
			CDuiString s;
			s.Format(_T("%s_%d"), id.GetData(), i);
			if( tm->FindTheme(s.GetData()) == NULL ) return s;
		}
		CDuiString last;
		last.Format(_T("%s_%u"), id.GetData(), (unsigned)::GetTickCount());
		return last;
	}

} // namespace

/////////////////////////////////////////////////////////////////////////////////////
// CThemeSwitcherUI

IMPLEMENT_DUICONTROL(CThemeSwitcherUI)

CThemeSwitcherUI::CThemeSwitcherUI()
	: m_bTintAuto(true)
{
	// 默认：调色板图标（换主题语义清晰）；无字，靠 tooltip 提示
	SetText(_T(""));
	SetKind(CONTROLKIND_NONE);
	SetBackgroundColor(0);
	SetToolTip(_T("选择主题"));
	SetIconLib(_T("lucide"), _T("palette"));
	SetIconSize(18);
}

LPCTSTR CThemeSwitcherUI::GetClass() const
{
	return _T("ThemeSwitcherUI");
}

LPVOID CThemeSwitcherUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcsicmp(pstrName, DUI_CTR_THEMESWITCHER) == 0 ) return static_cast<CThemeSwitcherUI*>(this);
	if( _tcsicmp(pstrName, _T("ThemeSwitcher")) == 0 ) return static_cast<CThemeSwitcherUI*>(this);
	return CButtonUI::GetInterface(pstrName);
}

void CThemeSwitcherUI::SetThemesFilter(LPCTSTR pstrThemes)
{
	m_sThemesFilter = pstrThemes ? pstrThemes : _T("");
}

LPCTSTR CThemeSwitcherUI::GetThemesFilter() const
{
	return m_sThemesFilter.GetData();
}

LPCTSTR CThemeSwitcherUI::GetThemeId() const
{
	return m_sThemeId.GetData();
}

void CThemeSwitcherUI::SetTintAuto(bool bAuto)
{
	if( m_bTintAuto == bAuto ) return;
	m_bTintAuto = bAuto;
	if( m_bTintAuto ) SyncFromManager();
}

bool CThemeSwitcherUI::IsTintAuto() const
{
	return m_bTintAuto;
}

void CThemeSwitcherUI::DoInit()
{
	CButtonUI::DoInit();
	SyncFromManager();
}

void CThemeSwitcherUI::SyncFromManager()
{
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return;

	CTheme* p = tm->GetCurrentTheme();
	LPCTSTR name = _T("主题");
	if( p != NULL ) {
		m_sThemeId = p->GetId();
		name = p->GetDisplayName();
		if( name == NULL || *name == _T('\0') ) name = p->GetId();
	}
	else {
		m_sThemeId = tm->GetDefaultThemeId();
	}

	CDuiString tip;
	tip.Format(_T("选择主题（当前: %s）"), name);
	SetToolTip(tip.GetData());

	if( !m_bTintAuto ) {
		Invalidate();
		return;
	}

	// 标题栏内跟 titlebar 字色，否则跟正文色，保证随主题可见
	bool bInTitleBar = false;
	for( CControlUI* pPar = GetParent(); pPar != NULL; pPar = pPar->GetParent() ) {
		if( pPar->GetInterface(DUI_CTR_TITLEBAR) != NULL ) {
			bInTitleBar = true;
			break;
		}
	}
	if( bInTitleBar ) {
		// 与 TitleBar 系统按钮一致：字色悬停 + 半透明底。
		// 不能用 color-primary 作图标悬停——晴空蓝/石墨的 primary 等于 titlebar-bg，会“隐身”。
		DWORD titleBg = tm->GetColor(_T("color-titlebar-bg"), 0x333333FF);
		DWORD titleTx = tm->GetColor(_T("color-titlebar-text"), 0xFFFFFFFF);
		const int lum = ((int)DuiColorR(titleBg) * 299
			+ (int)DuiColorG(titleBg) * 587
			+ (int)DuiColorB(titleBg) * 114) / 1000;
		const bool bLightBar = (lum >= 160);
		SetIconTint(DuiColorSetA(titleTx, 0xB4));
		SetIconTintHover(DuiColorSetA(titleTx, 0xFF));
		SetBackgroundColor(0);
		SetHoverBackgroundColor(bLightBar ? 0x0000001Au : 0xFFFFFF33u);
		SetActiveBackgroundColor(bLightBar ? 0x0000001Au : 0xFFFFFF33u);
	}
	else {
		DWORD tint = tm->GetColor(_T("color-text"), 0x333333FF);
		SetIconTint(tint);
		SetIconTintHover(tm->GetColor(_T("color-primary"), tint));
		SetHoverBackgroundColor(0);
		SetActiveBackgroundColor(0);
	}
	Invalidate();
}

bool CThemeSwitcherUI::Activate()
{
	if( !CButtonUI::Activate() ) return false;
	OpenPicker();
	return true;
}

void CThemeSwitcherUI::OpenPicker()
{
	HWND hOwner = NULL;
	if( m_pManager != NULL ) hOwner = m_pManager->GetPaintWindow();
	CThemePickerWnd::Open(hOwner, this, m_sThemesFilter.GetData());
}

void CThemeSwitcherUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcsicmp(pstrName, _T("themes")) == 0 ) {
		SetThemesFilter(pstrValue);
		return;
	}
	if( _tcsicmp(pstrName, _T("tint-auto")) == 0
		|| _tcsicmp(pstrName, _T("tintauto")) == 0 ) {
		SetTintAuto(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		return;
	}
	if( _tcsicmp(pstrName, _T("sync-from-manager")) == 0 ) {
		SyncFromManager();
		return;
	}
	CButtonUI::SetAttribute(pstrName, pstrValue);
}

/////////////////////////////////////////////////////////////////////////////////////
// CThemePickerWnd

CThemePickerWnd* CThemePickerWnd::s_pActive = NULL;

DUI_BEGIN_MESSAGE_MAP(CThemePickerWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CThemePickerWnd::OnClick)
DUI_END_MESSAGE_MAP()

CThemePickerWnd::CThemePickerWnd(CThemeSwitcherUI* pOwner, LPCTSTR pstrThemesFilter)
	: m_pOwner(pOwner)
	, m_sThemesFilter(pstrThemesFilter ? pstrThemesFilter : _T(""))
	, m_dwLastHexClick(0)
	, m_bEditMode(false)
	, m_bHexEditing(false)
	, m_bCommitted(false)
	, m_pThemeList(NULL)
	, m_pTokenList(NULL)
	, m_pMiniPreview(NULL)
	, m_pPalette(NULL)
{
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm != NULL ) m_sEnterThemeId = tm->GetCurrentThemeId();
}

CThemePickerWnd::~CThemePickerWnd()
{
	if( s_pActive == this ) s_pActive = NULL;
}

void CThemePickerWnd::Open(HWND hOwner, CThemeSwitcherUI* pSwitcher, LPCTSTR pstrThemesFilter)
{
	if( s_pActive != NULL && ::IsWindow(s_pActive->GetHWND()) ) {
		::SetForegroundWindow(s_pActive->GetHWND());
		return;
	}
	CThemePickerWnd* pWnd = new CThemePickerWnd(pSwitcher, pstrThemesFilter);
	s_pActive = pWnd;
	pWnd->Create(hOwner, _T("选择主题"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 860, 560);
	pWnd->CenterWindow();
	pWnd->ShowModal();
}

void CThemePickerWnd::OnFinalMessage(HWND hWnd)
{
	if( s_pActive == this ) s_pActive = NULL;
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CThemePickerWnd::GetSkinFile()
{
	// 以 '<' 开头：DialogBuilder 按内联 XML 加载（C++11 LR/R raw string）
	return GetBuiltinThemePickerSkin();
}

LPCTSTR CThemePickerWnd::GetWindowClassName() const
{
	return _T("ThemePickerWnd");
}

void CThemePickerWnd::InitWindow()
{
	m_pThemeList = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("themeList")));
	m_pTokenList = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("tokenList")));
	m_pMiniPreview = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("miniPreview")));
	m_pPalette = static_cast<CColorPaletteUI*>(m_pm.FindControl(_T("palette")));
	if( m_pPalette != NULL ) m_pPalette->SetVisible(false);

	CThemeManager* tm = CThemeManager::GetInstance();
	LPCTSTR cur = tm ? tm->GetCurrentThemeId() : _T("default");
	m_sSelectedId = cur;
	RebuildThemeList();
	SelectTheme(m_sSelectedId.GetData(), true);
	SetEditMode(false);
}

bool CThemePickerWnd::ThemeAllowed(LPCTSTR id) const
{
	if( id == NULL || *id == _T('\0') ) return false;
	if( m_sThemesFilter.IsEmpty() ) return true;
	CDuiString filter = m_sThemesFilter;
	filter.MakeLower();
	CDuiString one = id;
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

void CThemePickerWnd::RebuildThemeList()
{
	if( m_pThemeList == NULL ) return;
	m_pThemeList->RemoveAll();
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return;
	for( int i = 0; i < tm->GetThemeCount(); ++i ) {
		CTheme* p = tm->GetThemeAt(i);
		if( p == NULL || !ThemeAllowed(p->GetId()) ) continue;
		CButtonUI* pBtn = new CButtonUI;
		CDuiString id = p->GetId();
		pBtn->SetName(_T("theme_item"));
		pBtn->AddCustomAttribute(_T("theme-id"), id.GetData());
		LPCTSTR dn = p->GetDisplayName();
		if( dn == NULL || *dn == _T('\0') ) dn = id.GetData();
		pBtn->SetText(dn);
		pBtn->SetFixedHeight(32);
		pBtn->SetAttribute(_T("text-align"), _T("left"));
		pBtn->SetPadding(CDuiBox(0, 10, 0, 10));
		bool bSel = (id == m_sSelectedId);
		pBtn->SetKind(bSel ? CONTROLKIND_PRIMARY : CONTROLKIND_DEFAULT);
		m_pThemeList->Add(pBtn);
	}
}

void CThemePickerWnd::SelectTheme(LPCTSTR id, bool bPreviewApply)
{
	if( id == NULL || *id == _T('\0') ) return;
	if( m_bHexEditing )
		CommitHexEdit(true);
	m_sSelectedId = id;
	m_sEditToken.Empty();
	CThemeManager* tm = CThemeManager::GetInstance();
	if( bPreviewApply && tm != NULL )
		tm->ApplyTheme(id, true);

	if( m_pThemeList != NULL ) {
		for( int i = 0; i < m_pThemeList->GetCount(); ++i ) {
			CControlUI* p = m_pThemeList->GetItemAt(i);
			if( p == NULL ) continue;
			LPCTSTR tid = p->GetCustomAttribute(_T("theme-id"));
			bool bSel = (tid != NULL && _tcsicmp(tid, id) == 0);
			CButtonUI* pBtn = static_cast<CButtonUI*>(p->GetInterface(DUI_CTR_BUTTON));
			if( pBtn != NULL )
				pBtn->SetKind(bSel ? CONTROLKIND_PRIMARY : CONTROLKIND_DEFAULT);
		}
	}
	RebuildMiniPreview();
	RebuildTokenList();
	if( m_pPalette != NULL && m_bEditMode && !m_sEditToken.IsEmpty() ) {
		CTheme* pTh = GetSelectedTheme();
		if( pTh != NULL )
			m_pPalette->SetSelectColor(pTh->GetToken(m_sEditToken.GetData(), 0x808080FF));
	}
}

CTheme* CThemePickerWnd::GetSelectedTheme() const
{
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return NULL;
	return tm->FindTheme(m_sSelectedId.GetData());
}

void CThemePickerWnd::RebuildMiniPreview()
{
	if( m_pMiniPreview == NULL ) return;
	m_pMiniPreview->RemoveAll();
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return;

	static const LPCTSTR kKeys[] = {
		_T("color-titlebar-bg"), _T("color-primary"), _T("color-success"),
		_T("color-warning"), _T("color-danger"), _T("color-bg"), _T("color-text")
	};
	for( size_t i = 0; i < sizeof(kKeys) / sizeof(kKeys[0]); ++i ) {
		CControlUI* sw = new CControlUI;
		sw->SetFixedWidth(28);
		sw->SetFixedHeight(28);
		sw->SetBackgroundColor(p->GetToken(kKeys[i], 0x808080FF));
		sw->SetBorderWidth(1);
		sw->SetBorderColor(0x00000033);
		sw->SetToolTip(kKeys[i]);
		m_pMiniPreview->Add(sw);
	}
	CLabelUI* pLab = new CLabelUI;
	CDuiString s;
	s.Format(_T("  %s (%s)"), p->GetDisplayName(), p->GetId());
	pLab->SetText(s.GetData());
	pLab->SetColor(p->GetToken(_T("color-text"), 0x333333FF));
	m_pMiniPreview->Add(pLab);
}

void CThemePickerWnd::RebuildTokenList()
{
	if( m_pTokenList == NULL ) return;
	m_pTokenList->RemoveAll();
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return;

	const int nTheme = p->GetTokenCount();
	if( nTheme <= 0 ) return;

	bool* pUsed = new bool[nTheme];
	for( int i = 0; i < nTheme; ++i ) pUsed[i] = false;

	CStdPtrArray buckets[TOKEN_GROUP_COUNT];
	const int nPref = (int)(sizeof(kTokenDisplayOrder) / sizeof(kTokenDisplayOrder[0]));
	for( int i = 0; i < nPref; ++i ) {
		LPCTSTR want = kTokenDisplayOrder[i];
		for( int j = 0; j < nTheme; ++j ) {
			if( pUsed[j] ) continue;
			LPCTSTR name = p->GetTokenNameAt(j);
			if( name == NULL || _tcsicmp(name, want) != 0 ) continue;
			int g = GetTokenGroupId(name);
			if( g < 0 || g >= TOKEN_GROUP_COUNT ) g = TOKEN_GROUP_OTHER;
			buckets[g].Add(new CDuiString(name));
			pUsed[j] = true;
			break;
		}
	}
	for( int j = 0; j < nTheme; ++j ) {
		if( pUsed[j] ) continue;
		LPCTSTR name = p->GetTokenNameAt(j);
		if( name == NULL || *name == _T('\0') ) continue;
		int g = GetTokenGroupId(name);
		if( g < 0 || g >= TOKEN_GROUP_COUNT ) g = TOKEN_GROUP_OTHER;
		buckets[g].Add(new CDuiString(name));
		pUsed[j] = true;
	}
	delete[] pUsed;

	int displayIndex = 0;
	for( int g = 0; g < TOKEN_GROUP_COUNT; ++g ) {
		const int nInGroup = buckets[g].GetSize();
		if( nInGroup <= 0 ) continue;

		CLabelUI* pHead = new CLabelUI;
		CDuiString sHead;
		sHead.Format(_T("— %s —"), GetTokenGroupTitle(g));
		pHead->SetText(sHead.GetData());
		pHead->SetFixedHeight(28);
		pHead->SetAttribute(_T("text-align"), _T("left"));
		pHead->SetPadding(CDuiBox(0, 8, 0, 8));
		pHead->SetColor(ThemeTok(_T("color-primary"), 0x1677FFFF));
		pHead->SetBackgroundColor(ThemeTok(_T("color-bg-elevated"), 0xF8F9FAFF));
		pHead->SetMouseEnabled(false);
		m_pTokenList->Add(pHead);

		for( int i = 0; i < nInGroup; ++i ) {
			CDuiString* pNameStr = static_cast<CDuiString*>(buckets[g].GetAt(i));
			LPCTSTR name = pNameStr->GetData();
			DWORD c = p->GetToken(name, 0);

			const bool bSel = (!m_sEditToken.IsEmpty() && m_sEditToken == name);
			DWORD rowBk = ThemeTok(_T("color-bg"), 0xFFFFFFFF);
			if( (displayIndex % 2) == 1 )
				rowBk = ThemeTok(_T("color-bg-elevated"), 0xF8F9FAFF);
			const DWORD hoverBk = ThemeTok(_T("color-bg-hover"), 0xE9F5FFFF);
			const DWORD selBk = ThemeTok(_T("color-selection"), 0xE7F1FFFF);
			if( bSel )
				rowBk = selBk;
			const DWORD rowHover = bSel ? selBk : hoverBk;

			CThemeTokenRowUI* pRow = new CThemeTokenRowUI;
			pRow->SetFixedHeight(30);
			pRow->SetAttribute(_T("align-items"), _T("vcenter"));
			pRow->SetAttribute(_T("gap"), _T("6"));
			pRow->SetPadding(CDuiBox(0, 4, 0, 4));
			pRow->AddCustomAttribute(_T("token-name"), name);
			pRow->SetMouseEnabled(true);
			pRow->SetBackgroundColor(rowBk);
			pRow->SetHoverBackgroundColor(rowHover);

			CLabelUI* pIdx = new CLabelUI;
			CDuiString sIdx;
			sIdx.Format(_T("%d"), displayIndex + 1);
			pIdx->SetText(sIdx.GetData());
			pIdx->SetFixedWidth(28);
			pIdx->SetAttribute(_T("text-align"), _T("right"));
			pIdx->SetColor(ThemeTok(_T("color-secondary"), 0x8C8C8CFF));
			pIdx->SetMouseEnabled(false);
			pRow->Add(pIdx);

			CControlUI* pSwatch = new CControlUI;
			pSwatch->SetFixedWidth(20);
			pSwatch->SetFixedHeight(20);
			pSwatch->SetBackgroundColor(c);
			pSwatch->SetBorderWidth(1);
			pSwatch->SetBorderColor(0x00000040);
			pSwatch->SetMouseEnabled(false);
			pSwatch->SetToolTip(name);
			pRow->Add(pSwatch);

			CDuiString hex;
			FormatHex(hex, c);
			if( m_bHexEditing && m_sEditToken == name ) {
				CEditUI* pEdit = new CEditUI;
				pEdit->SetName(_T("token_hex_edit"));
				pEdit->AddCustomAttribute(_T("token-name"), name);
				pEdit->SetText(hex.GetData());
				pEdit->SetFixedWidth(118);
				pEdit->SetFixedHeight(24);
				pEdit->SetMaxChar(16);
				pEdit->SetAttribute(_T("text-align"), _T("left"));
				pRow->Add(pEdit);
			}
			else {
				CButtonUI* pHex = new CButtonUI;
				pHex->SetName(_T("token_hex"));
				pHex->AddCustomAttribute(_T("token-name"), name);
				pHex->SetText(hex.GetData());
				pHex->SetFixedWidth(118);
				pHex->SetFixedHeight(28);
				pHex->SetAttribute(_T("text-align"), _T("left"));
				pHex->SetPadding(CDuiBox(0, 4, 0, 4));
				pHex->SetKind(CONTROLKIND_NONE);
				pHex->SetBackgroundColor(0);
				pHex->SetColor(ThemeTok(_T("color-text"), 0x333333FF));
				pHex->SetToolTip(_T("双击编辑色值"));
				pRow->Add(pHex);
			}

			CButtonUI* pNameHit = new CButtonUI;
			pNameHit->SetName(_T("token_row"));
			pNameHit->AddCustomAttribute(_T("token-name"), name);
			pNameHit->SetText(name);
			pNameHit->SetFixedWidth(168);
			pNameHit->SetFixedHeight(28);
			pNameHit->SetAttribute(_T("text-align"), _T("left"));
			pNameHit->SetPadding(CDuiBox(0, 4, 0, 4));
			pNameHit->SetKind(CONTROLKIND_NONE);
			pNameHit->SetBackgroundColor(0);
			pNameHit->SetColor(ThemeTok(_T("color-text"), 0x333333FF));
			pNameHit->SetToolTip(name);
			pRow->Add(pNameHit);

			CButtonUI* pDesc = new CButtonUI;
			pDesc->SetName(_T("token_row"));
			pDesc->AddCustomAttribute(_T("token-name"), name);
			pDesc->SetText(GetTokenUsageDesc(name));
			pDesc->SetFixedHeight(28);
			pDesc->SetAttribute(_T("flexible"), _T("true"));
			pDesc->SetAttribute(_T("text-align"), _T("left"));
			pDesc->SetPadding(CDuiBox(0, 4, 0, 4));
			pDesc->SetKind(CONTROLKIND_NONE);
			pDesc->SetBackgroundColor(0);
			pDesc->SetColor(ThemeTok(_T("color-text-secondary"), 0x8C8C8CFF));
			pDesc->SetToolTip(GetTokenUsageDesc(name));
			pRow->Add(pDesc);

			m_pTokenList->Add(pRow);
			++displayIndex;
		}
	}

	for( int g = 0; g < TOKEN_GROUP_COUNT; ++g ) {
		for( int i = 0; i < buckets[g].GetSize(); ++i )
			delete static_cast<CDuiString*>(buckets[g].GetAt(i));
		buckets[g].Empty();
	}
}

void CThemePickerWnd::SetEditMode(bool bEdit)
{
	if( !bEdit && m_bHexEditing )
		CommitHexEdit(true);
	m_bEditMode = bEdit;
	if( m_pPalette != NULL )
		m_pPalette->SetVisible(bEdit);
	CControlUI* pHint = m_pm.FindControl(_T("editHint"));
	if( pHint != NULL ) pHint->SetVisible(bEdit);
	RebuildTokenList();
}

void CThemePickerWnd::OnTokenRowClick(CControlUI* pRow)
{
	if( pRow == NULL ) return;
	if( m_bHexEditing )
		CommitHexEdit(true);
	LPCTSTR name = pRow->GetCustomAttribute(_T("token-name"));
	if( name == NULL ) return;
	m_sEditToken = name;
	if( m_bEditMode ) {
		CTheme* p = GetSelectedTheme();
		if( p != NULL && m_pPalette != NULL )
			m_pPalette->SetSelectColor(p->GetToken(name, 0x808080FF));
	}
	RebuildTokenList();
}

void CThemePickerWnd::OnTokenHexClick(CControlUI* pHex)
{
	if( pHex == NULL ) return;
	LPCTSTR name = pHex->GetCustomAttribute(_T("token-name"));
	if( name == NULL || *name == _T('\0') ) return;

	const DWORD now = ::GetTickCount();
	const DWORD limit = ::GetDoubleClickTime();
	if( !m_sLastHexClick.IsEmpty()
		&& m_sLastHexClick == name
		&& (now - m_dwLastHexClick) <= limit ) {
		m_sLastHexClick.Empty();
		m_dwLastHexClick = 0;
		BeginHexEdit(name);
		return;
	}
	m_sLastHexClick = name;
	m_dwLastHexClick = now;
	OnTokenRowClick(pHex);
}

void CThemePickerWnd::BeginHexEdit(LPCTSTR token)
{
	if( token == NULL || *token == _T('\0') ) return;
	if( m_bHexEditing )
		CommitHexEdit(true);
	if( !m_bEditMode ) {
		m_bEditMode = true;
		if( m_pPalette != NULL )
			m_pPalette->SetVisible(true);
		CControlUI* pHint = m_pm.FindControl(_T("editHint"));
		if( pHint != NULL ) pHint->SetVisible(true);
	}
	m_sEditToken = token;
	m_bHexEditing = true;
	CTheme* p = GetSelectedTheme();
	if( p != NULL && m_pPalette != NULL )
		m_pPalette->SetSelectColor(p->GetToken(token, 0x808080FF));
	RebuildTokenList();
	FocusHexEdit();
}

void CThemePickerWnd::CommitHexEdit(bool bApply)
{
	if( !m_bHexEditing ) return;
	CDuiString text;
	CControlUI* pCtrl = m_pm.FindControl(_T("token_hex_edit"));
	if( pCtrl != NULL ) text = pCtrl->GetText();
	CDuiString token = m_sEditToken;
	m_bHexEditing = false;

	if( bApply && !token.IsEmpty() && !text.IsEmpty() ) {
		DWORD c = 0;
		CDuiString s = text;
		s.TrimLeft();
		s.TrimRight();
		if( ParseColorString(s.GetData(), c) ) {
			CTheme* p = GetSelectedTheme();
			if( p != NULL ) {
				p->SetToken(token.GetData(), c);
				CThemeManager* tm = CThemeManager::GetInstance();
				if( tm != NULL ) tm->RefreshCurrentTheme(true);
				if( m_pPalette != NULL )
					m_pPalette->SetSelectColor(c);
			}
		}
		else {
			CToast::ShowWarning(_T("颜色格式无效，请使用 #RRGGBB 或 #RRGGBBAA"), 2500);
		}
	}
	RebuildMiniPreview();
	RebuildTokenList();
}

void CThemePickerWnd::FocusHexEdit()
{
	CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("token_hex_edit")));
	if( pEdit == NULL ) return;
	pEdit->SetFocus();
	pEdit->SetSelAll();
}

void CThemePickerWnd::OnPaletteColor(DWORD dwColor, bool /*bChanging*/)
{
	if( !m_bEditMode || m_sEditToken.IsEmpty() ) return;
	if( m_bHexEditing )
		m_bHexEditing = false;
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return;
	p->SetToken(m_sEditToken.GetData(), dwColor);
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm != NULL ) tm->RefreshCurrentTheme(true);
	RebuildMiniPreview();
	RebuildTokenList();
}

bool CThemePickerWnd::DoNewTheme()
{
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return false;
	CTheme* pBase = GetSelectedTheme();
	if( pBase == NULL ) pBase = tm->FindTheme(tm->GetDefaultThemeId());
	if( pBase == NULL ) return false;

	CDuiString id = MakeUniqueThemeId(_T("custom"));
	CDuiString title;
	title.Format(_T("自定义 (%s)"), id.GetData());
	CTheme* pNew = new CTheme(id.GetData(), title.GetData());
	pNew->CopyTokensFrom(*pBase);
	if( !tm->RegisterTheme(pNew, true) ) {
		delete pNew;
		return false;
	}
	m_sSelectedId = id;
	RebuildThemeList();
	SelectTheme(id.GetData(), true);
	SetEditMode(true);
	CToast::ShowSuccess(_T("已新建主题，可在右侧修改任意颜色"), 2500);
	return true;
}

bool CThemePickerWnd::DoImport()
{
	CDuiString path;
	if( !PickOpenThemePath(m_hWnd, path) ) return false;
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return false;
	if( !tm->ApplyThemeFile(path.GetData(), NULL) ) {
		CToast::ShowDanger(_T("导入主题失败"), 2500);
		return false;
	}
	m_sSelectedId = tm->GetCurrentThemeId();
	RebuildThemeList();
	SelectTheme(m_sSelectedId.GetData(), true);
	CToast::ShowSuccess(_T("已导入主题"), 2000);
	return true;
}

bool CThemePickerWnd::DoSaveAs()
{
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return false;
	CDuiString path;
	if( !PickSaveThemePath(m_hWnd, path) ) return false;
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL || !tm->SaveThemeFile(p, path.GetData()) ) {
		CToast::ShowDanger(_T("保存主题文件失败"), 2500);
		return false;
	}
	CToast::ShowSuccess(_T("主题已保存"), 2000);
	return true;
}

void CThemePickerWnd::CommitAndClose(bool bOk)
{
	if( m_bHexEditing )
		CommitHexEdit(true);
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) {
		Close(bOk ? IDOK : IDCANCEL);
		return;
	}
	if( bOk ) {
		m_bCommitted = true;
		tm->ApplyTheme(m_sSelectedId.GetData(), false);
		if( m_pOwner != NULL ) {
			m_pOwner->SyncFromManager();
			if( m_pOwner->GetManager() != NULL )
				m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_SELECTCHANGED);
		}
	}
	else {
		if( !m_sEnterThemeId.IsEmpty() )
			tm->ApplyTheme(m_sEnterThemeId.GetData(), false);
	}
	Close(bOk ? IDOK : IDCANCEL);
}

void CThemePickerWnd::Notify(TNotifyUI& msg)
{
	if( msg.sType == DUI_MSGTYPE_COLORCHANGED || msg.sType == DUI_MSGTYPE_COLORCHANGING ) {
		if( msg.pSender == m_pPalette )
			OnPaletteColor((DWORD)msg.wParam, msg.sType == DUI_MSGTYPE_COLORCHANGING);
		return;
	}
	if( msg.sType == DUI_MSGTYPE_RETURN ) {
		if( msg.pSender != NULL && msg.pSender->GetName() == _T("token_hex_edit") ) {
			CommitHexEdit(true);
			return;
		}
	}
	WindowImplBase::Notify(msg);
}

void CThemePickerWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sName = msg.pSender ? msg.pSender->GetName() : CDuiString();
	if( m_bHexEditing && sName.CompareNoCase(_T("token_hex_edit")) != 0
		&& sName.CompareNoCase(_T("token_hex")) != 0 ) {
		// 点到其它控件时先提交当前键入（token_hex 自身走双击逻辑）
		if( sName.CompareNoCase(_T("token_row")) != 0 )
			CommitHexEdit(true);
	}
	if( sName.CompareNoCase(_T("btn_ok")) == 0 ) {
		CommitAndClose(true);
		return;
	}
	if( sName.CompareNoCase(_T("btn_cancel")) == 0 || sName.CompareNoCase(_T("closebtn")) == 0 ) {
		CommitAndClose(false);
		return;
	}
	if( sName.CompareNoCase(_T("btn_new")) == 0 ) {
		DoNewTheme();
		return;
	}
	if( sName.CompareNoCase(_T("btn_edit")) == 0 ) {
		SetEditMode(!m_bEditMode);
		CToast::ShowInfo(m_bEditMode
			? _T("编辑模式：点选色值，或双击 #色值 直接键入")
			: _T("已退出编辑模式"), 2200);
		return;
	}
	if( sName.CompareNoCase(_T("btn_import")) == 0 ) {
		DoImport();
		return;
	}
	if( sName.CompareNoCase(_T("btn_save")) == 0 ) {
		DoSaveAs();
		return;
	}
	if( sName.CompareNoCase(_T("token_hex")) == 0 ) {
		OnTokenHexClick(msg.pSender);
		return;
	}
	if( sName.CompareNoCase(_T("token_row")) == 0 ) {
		OnTokenRowClick(msg.pSender);
		return;
	}
	if( sName.CompareNoCase(_T("theme_item")) == 0 ) {
		LPCTSTR id = msg.pSender->GetCustomAttribute(_T("theme-id"));
		if( id != NULL ) SelectTheme(id, true);
		return;
	}
	WindowImplBase::OnClick(msg);
}

LRESULT CThemePickerWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
	if( uMsg == WM_KEYDOWN && wParam == VK_ESCAPE ) {
		CommitAndClose(false);
		bHandled = true;
		return TRUE;
	}
	bHandled = false;
	return 0;
}

} // namespace DuiLib
