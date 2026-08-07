#include "StdAfx.h"
#include "UITheme.h"
#include <stdio.h>

namespace DuiLib {

	/////////////////////////////////////////////////////////////////////////////////////
	// CTheme

	CTheme::CTheme()
	{
	}

	CTheme::CTheme(LPCTSTR id, LPCTSTR displayName)
		: m_sId(id)
		, m_sDisplayName(displayName)
	{
	}

	CTheme::~CTheme()
	{
	}

	LPCTSTR CTheme::GetId() const
	{
		return m_sId.GetData();
	}

	void CTheme::SetId(LPCTSTR id)
	{
		m_sId = id;
	}

	LPCTSTR CTheme::GetDisplayName() const
	{
		return m_sDisplayName.GetData();
	}

	void CTheme::SetDisplayName(LPCTSTR name)
	{
		m_sDisplayName = name;
	}

	void CTheme::SetToken(LPCTSTR name, DWORD color)
	{
		if (name == NULL || *name == _T('\0')) return;
		CDuiString key(name);
		key.MakeLower();
		if (m_tokens.find(key) == m_tokens.end())
			m_tokenOrder.push_back(key);
		m_tokens[key] = color;
	}

	void CTheme::SetToken(LPCTSTR name, LPCTSTR colorStr)
	{
		DWORD c = 0;
		if (colorStr != NULL && ParseColorString(colorStr, c))
			SetToken(name, c);
	}

	bool CTheme::TryGetToken(LPCTSTR name, DWORD& color) const
	{
		if (name == NULL) return false;
		CDuiString key(name);
		key.MakeLower();
		std::map<CDuiString, DWORD>::const_iterator it = m_tokens.find(key);
		if (it == m_tokens.end()) return false;
		color = it->second;
		return true;
	}

	DWORD CTheme::GetToken(LPCTSTR name, DWORD fallback) const
	{
		DWORD c = 0;
		if (TryGetToken(name, c)) return c;
		return fallback;
	}

	int CTheme::GetTokenCount() const
	{
		return (int)m_tokenOrder.size();
	}

	LPCTSTR CTheme::GetTokenNameAt(int index) const
	{
		if (index < 0 || index >= (int)m_tokenOrder.size()) return _T("");
		return m_tokenOrder[index].GetData();
	}

	DWORD CTheme::GetTokenValueAt(int index) const
	{
		if (index < 0 || index >= (int)m_tokenOrder.size()) return 0;
		return GetToken(m_tokenOrder[index].GetData(), 0);
	}

	void CTheme::ClearTokens()
	{
		m_tokens.clear();
		m_tokenOrder.clear();
	}

	void CTheme::CopyTokensFrom(const CTheme& other)
	{
		ClearTokens();
		for (size_t i = 0; i < other.m_tokenOrder.size(); ++i) {
			const CDuiString& key = other.m_tokenOrder[i];
			std::map<CDuiString, DWORD>::const_iterator it = other.m_tokens.find(key);
			if (it != other.m_tokens.end())
				SetToken(key.GetData(), it->second);
		}
	}

	DWORD CTheme::TokenOr(LPCTSTR name, DWORD fallback) const
	{
		return GetToken(name, fallback);
	}

	static void FillKind(KindColors& out, DWORD bg, DWORD border, DWORD text,
		DWORD bgH, DWORD borderH, DWORD textH,
		DWORD bgA, DWORD borderA, DWORD textA)
	{
		out.Normal.dwBackgroundColor = bg;
		out.Normal.dwBorderColor = border;
		out.Normal.dwColor = text;
		out.Hover.dwBackgroundColor = bgH;
		out.Hover.dwBorderColor = borderH;
		out.Hover.dwColor = textH;
		out.Active.dwBackgroundColor = bgA;
		out.Active.dwBorderColor = borderA;
		out.Active.dwColor = textA;
	}

	void CTheme::ApplyKindColors() const
	{
		// None
		FillKind(g_kindColors[0], 0, 0, 0, 0, 0, 0, 0, 0, 0);

		DWORD defBg = TokenOr(_T("color-default"), 0xEEEEEEFF);
		DWORD defBd = TokenOr(_T("color-default-border"), 0xDEE2E6FF);
		DWORD defTx = TokenOr(_T("color-default-text"), 0x212529FF);
		DWORD defBgH = TokenOr(_T("color-default-hover"), 0xD3D4D5FF);
		DWORD defBdH = TokenOr(_T("color-default-border-hover"), 0xC6C7C8FF);
		DWORD defBgA = TokenOr(_T("color-default-active"), 0xC6C7C8FF);
		DWORD defBdA = TokenOr(_T("color-default-border-active"), 0xBABBBCFF);
		FillKind(g_kindColors[1], defBg, defBd, defTx, defBgH, defBdH, defTx, defBgA, defBdA, defTx);

		DWORD p = TokenOr(_T("color-primary"), 0x0D6EFDFF);
		DWORD pH = TokenOr(_T("color-primary-hover"), 0x0B5ED7FF);
		DWORD pA = TokenOr(_T("color-primary-active"), 0x0A58CAFF);
		DWORD pOn = TokenOr(_T("color-primary-text"), 0xFFFFFFFF);
		DWORD pBd = TokenOr(_T("color-primary-border"), p);
		DWORD pBdH = TokenOr(_T("color-primary-border-hover"), TokenOr(_T("color-primary-hover-border"), 0x0A58CAFF));
		DWORD pBdA = TokenOr(_T("color-primary-border-active"), 0x0A53BEFF);
		FillKind(g_kindColors[2], p, pBd, pOn, pH, pBdH, pOn, pA, pBdA, pOn);

		DWORD s = TokenOr(_T("color-secondary"), 0x6C757DFF);
		DWORD sH = TokenOr(_T("color-secondary-hover"), 0x5C636AFF);
		DWORD sA = TokenOr(_T("color-secondary-active"), 0x565E64FF);
		DWORD sOn = TokenOr(_T("color-secondary-text"), 0xFFFFFFFF);
		FillKind(g_kindColors[3], s, s, sOn, sH, TokenOr(_T("color-secondary-border-hover"), 0x565E64FF), sOn,
			sA, TokenOr(_T("color-secondary-border-active"), 0x51585EFF), sOn);

		DWORD ok = TokenOr(_T("color-success"), 0x198754FF);
		DWORD okH = TokenOr(_T("color-success-hover"), 0x157347FF);
		DWORD okA = TokenOr(_T("color-success-active"), 0x146C43FF);
		DWORD okOn = TokenOr(_T("color-success-text"), 0xFFFFFFFF);
		FillKind(g_kindColors[4], ok, ok, okOn, okH, TokenOr(_T("color-success-border-hover"), 0x146C43FF), okOn,
			okA, TokenOr(_T("color-success-border-active"), 0x13653FFF), okOn);

		DWORD dg = TokenOr(_T("color-danger"), 0xDC3545FF);
		DWORD dgH = TokenOr(_T("color-danger-hover"), 0xBB2D3BFF);
		DWORD dgA = TokenOr(_T("color-danger-active"), 0xB02A37FF);
		DWORD dgOn = TokenOr(_T("color-danger-text"), 0xFFFFFFFF);
		FillKind(g_kindColors[5], dg, dg, dgOn, dgH, TokenOr(_T("color-danger-border-hover"), 0xB02A37FF), dgOn,
			dgA, TokenOr(_T("color-danger-border-active"), 0xA52834FF), dgOn);

		DWORD wn = TokenOr(_T("color-warning"), 0xFFC107FF);
		DWORD wnH = TokenOr(_T("color-warning-hover"), 0xE0A800FF);
		DWORD wnA = TokenOr(_T("color-warning-active"), 0xD39E00FF);
		DWORD wnOn = TokenOr(_T("color-warning-text"), 0x000000FF);
		FillKind(g_kindColors[6], wn, wn, wnOn, wnH, TokenOr(_T("color-warning-border-hover"), 0xD39E00FF), wnOn,
			wnA, TokenOr(_T("color-warning-border-active"), 0xC69500FF), wnOn);

		DWORD info = TokenOr(_T("color-info"), 0x0DCAF0FF);
		DWORD infoH = TokenOr(_T("color-info-hover"), 0x0BA5C7FF);
		DWORD infoA = TokenOr(_T("color-info-active"), 0x0A98B8FF);
		DWORD infoOn = TokenOr(_T("color-info-text"), 0x000000FF);
		FillKind(g_kindColors[7], info, info, infoOn, infoH, TokenOr(_T("color-info-border-hover"), 0x0A98B8FF), infoOn,
			infoA, TokenOr(_T("color-info-border-active"), 0x098BA8FF), infoOn);

		DWORD lt = TokenOr(_T("color-light"), 0xF8F9FAFF);
		DWORD ltH = TokenOr(_T("color-light-hover"), 0xD3D4D5FF);
		DWORD ltA = TokenOr(_T("color-light-active"), 0xC6C7C8FF);
		DWORD ltOn = TokenOr(_T("color-light-text"), 0x000000FF);
		FillKind(g_kindColors[8], lt, lt, ltOn, ltH, TokenOr(_T("color-light-border-hover"), 0xC6C7C8FF), ltOn,
			ltA, TokenOr(_T("color-light-border-active"), 0xBABBBCFF), ltOn);

		DWORD dk = TokenOr(_T("color-dark"), 0x212529FF);
		DWORD dkH = TokenOr(_T("color-dark-hover"), 0x424649FF);
		DWORD dkA = TokenOr(_T("color-dark-active"), 0x4D5154FF);
		DWORD dkOn = TokenOr(_T("color-dark-text"), 0xFFFFFFFF);
		FillKind(g_kindColors[9], dk, dk, dkOn, dkH, TokenOr(_T("color-dark-border-hover"), 0x373B3EFF), dkOn,
			dkA, TokenOr(_T("color-dark-border-active"), 0x373B3EFF), dkOn);

		DWORD link = TokenOr(_T("color-link"), p);
		DWORD linkH = TokenOr(_T("color-link-hover"), pH);
		DWORD linkA = TokenOr(_T("color-link-active"), pA);
		FillKind(g_kindColors[10], 0, 0, link, 0, 0, linkH, 0, 0, linkA);

		MarkKindColorsInitialized();
	}

	void CTheme::ApplyToManager(CPaintManagerUI* pManager) const
	{
		if (pManager == NULL) return;

		DWORD text = TokenOr(_T("color-text"), 0x000000E0);
		DWORD textSec = TokenOr(_T("color-text-secondary"), 0x000000A6);
		DWORD textDis = TokenOr(_T("color-text-disabled"),
			TokenOr(_T("color-disabled-text"), 0xADB5BDFF));
		DWORD border = TokenOr(_T("color-border"), 0xDEE2E6FF);
		DWORD borderStrong = TokenOr(_T("color-border-strong"), 0xADB5BDFF);
		DWORD bg = TokenOr(_T("color-bg"), 0xFFFFFFFF);
		DWORD bgElev = TokenOr(_T("color-bg-elevated"), 0xF8F9FAFF);
		DWORD disBg = TokenOr(_T("color-disabled-bg"), 0xE9ECEFFF);
		DWORD ctrlBg = TokenOr(_T("color-control-bg"), bg);
		DWORD ctrlBd = TokenOr(_T("color-control-border"), border);
		DWORD ctrlFocus = TokenOr(_T("color-control-border-focus"),
			TokenOr(_T("color-primary"), borderStrong));
		DWORD titleBg = TokenOr(_T("color-titlebar-bg"), 0x333333FF);
		DWORD titleTx = TokenOr(_T("color-titlebar-text"), 0xFFFFFFFF);
		DWORD titleBd = TokenOr(_T("color-titlebar-border"), 0x222222FF);
		DWORD link = TokenOr(_T("color-link"), TokenOr(_T("color-primary"), 0x0D6EFDFF));
		DWORD linkH = TokenOr(_T("color-link-hover"), link);
		DWORD selection = TokenOr(_T("color-selection"), bgElev);

		// 分层窗若已显式设过窗口底（Modal 圆角要透明），勿用主题色盖掉，
		// 否则 AttachDialog→ApplyToManager 会把角外重新铺成不透明白。
		if( !(pManager->IsLayered() && pManager->IsWindowBackgroundColorCustom()) )
			pManager->SetWindowBackgroundColor(bg);
		pManager->SetDefaultFontColor(text, true);
		pManager->SetDefaultDisabledColor(textDis, true);
		pManager->SetDefaultLinkFontColor(link, true);
		pManager->SetDefaultLinkHoverFontColor(linkH, true);
		pManager->SetDefaultSelectedBackgroundColor(selection, true);

		CDuiString sLabel;
		sLabel.Format(_T("color=\"#%08X\""), text);
		pManager->AddDefaultAttributeList(_T("Label"), sLabel.GetData(), true);
		pManager->AddDefaultAttributeList(_T("Text"), sLabel.GetData(), true);

		CDuiString sLabelSec;
		sLabelSec.Format(_T("color=\"#%08X\""), textSec);
		pManager->AddDefaultAttributeList(_T("Label.secondary"), sLabelSec.GetData(), true);

		CDuiString sRoot;
		sRoot.Format(_T("background-color=\"#%08X\""), bg);
		pManager->AddDefaultAttributeList(_T("Window"), sRoot.GetData(), true);
		pManager->AddDefaultAttributeList(_T("html"), sRoot.GetData(), true);

		CDuiString sTitle;
		sTitle.Format(_T("background-color=\"#%08X\" color=\"#%08X\" border-color=\"#%08X\""),
			titleBg, titleTx, titleBd);
		pManager->AddDefaultAttributeList(_T("TitleBar"), sTitle.GetData(), true);

		CDuiString sForm;
		sForm.Format(
			_T("background-color=\"#%08X\" border-color=\"#%08X\" border-color-focus=\"#%08X\" ")
			_T("color=\"#%08X\" background-color-disabled=\"#%08X\""),
			ctrlBg, ctrlBd, ctrlFocus, text, disBg);
		pManager->AddDefaultAttributeList(_T("Edit"), sForm.GetData(), true);
		pManager->AddDefaultAttributeList(_T("RichEdit"), sForm.GetData(), true);
		pManager->AddDefaultAttributeList(_T("Spin"), sForm.GetData(), true);
		pManager->AddDefaultAttributeList(_T("Number"), sForm.GetData(), true);
		pManager->AddDefaultAttributeList(_T("Combo"), sForm.GetData(), true);
		pManager->AddDefaultAttributeList(_T("ComboBox"), sForm.GetData(), true);
		pManager->AddDefaultAttributeList(_T("DateTime"), sForm.GetData(), true);

		CDuiString sAccItem;
		sAccItem.Format(
			_T("header-background-color=\"#%08X\" header-color=\"#%08X\" ")
			_T("header-background-color-hover=\"#%08X\""),
			bgElev, text, borderStrong);
		pManager->AddDefaultAttributeList(_T("AccordionItem"), sAccItem.GetData(), true);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CThemeManager

	CThemeManager::CThemeManager()
		: m_sDefaultId(_T("default"))
		, m_bInited(false)
		, m_bEnabled(true)
	{
	}

	CThemeManager::~CThemeManager()
	{
		for (size_t i = 0; i < m_themes.size(); ++i) {
			if (m_themes[i].bOwn && m_themes[i].pTheme != NULL)
				delete m_themes[i].pTheme;
		}
		m_themes.clear();
	}

	CThemeManager* CThemeManager::GetInstance()
	{
		static CThemeManager inst;
		inst.EnsureInitialized();
		return &inst;
	}

	void CThemeManager::EnsureInitialized()
	{
		if (m_bInited) return;
		m_bInited = true;
		RegisterBuiltinThemes();
		if (!m_bEnabled) {
			FillBuiltinKindColors();
			m_sCurrentId.Empty();
			return;
		}
		// 仅写入 g_kindColors，禁止在控件构造/建树阶段 RefreshAllManagers（会重入崩溃）
		CTheme* p = FindTheme(m_sDefaultId.GetData());
		if (p != NULL) {
			m_sCurrentId = p->GetId();
			p->ApplyKindColors();
		}
		else {
			FillBuiltinKindColors();
		}
	}

	void CThemeManager::SetEnabled(bool bEnabled)
	{
		EnsureInitialized();
		if (m_bEnabled == bEnabled) return;
		m_bEnabled = bEnabled;
		if (!m_bEnabled) {
			m_sCurrentId.Empty();
			FillBuiltinKindColors();
			RefreshAllManagers();
			return;
		}
		LPCTSTR id = m_sDefaultId.GetData();
		if (FindTheme(id) != NULL)
			ApplyTheme(id);
	}

	bool CThemeManager::IsEnabled() const
	{
		return m_bEnabled;
	}

	bool CThemeManager::RegisterTheme(CTheme* pTheme, bool bOwn)
	{
		if (pTheme == NULL || pTheme->GetId() == NULL || *pTheme->GetId() == _T('\0'))
			return false;
		CTheme* existing = FindTheme(pTheme->GetId());
		if (existing != NULL) {
			if (existing == pTheme) return true;
			UnregisterTheme(pTheme->GetId());
		}
		ThemeEntry e;
		e.pTheme = pTheme;
		e.bOwn = bOwn;
		m_themes.push_back(e);
		return true;
	}

	bool CThemeManager::UnregisterTheme(LPCTSTR id)
	{
		if (id == NULL) return false;
		for (size_t i = 0; i < m_themes.size(); ++i) {
			if (m_themes[i].pTheme != NULL &&
				_tcsicmp(m_themes[i].pTheme->GetId(), id) == 0) {
				if (m_themes[i].bOwn) delete m_themes[i].pTheme;
				m_themes.erase(m_themes.begin() + (int)i);
				return true;
			}
		}
		return false;
	}

	CTheme* CThemeManager::FindTheme(LPCTSTR id) const
	{
		if (id == NULL) return NULL;
		for (size_t i = 0; i < m_themes.size(); ++i) {
			if (m_themes[i].pTheme != NULL &&
				_tcsicmp(m_themes[i].pTheme->GetId(), id) == 0)
				return m_themes[i].pTheme;
		}
		return NULL;
	}

	int CThemeManager::GetThemeCount() const
	{
		return (int)m_themes.size();
	}

	CTheme* CThemeManager::GetThemeAt(int index) const
	{
		if (index < 0 || index >= (int)m_themes.size()) return NULL;
		return m_themes[index].pTheme;
	}

	void CThemeManager::SetDefaultThemeId(LPCTSTR id)
	{
		if (id != NULL) m_sDefaultId = id;
	}

	LPCTSTR CThemeManager::GetDefaultThemeId() const
	{
		return m_sDefaultId.GetData();
	}

	CTheme* CThemeManager::GetCurrentTheme() const
	{
		return FindTheme(m_sCurrentId.GetData());
	}

	LPCTSTR CThemeManager::GetCurrentThemeId() const
	{
		return m_sCurrentId.GetData();
	}

	bool CThemeManager::ApplyTheme(LPCTSTR id)
	{
		if (!m_bEnabled) return false;
		CTheme* p = FindTheme(id);
		if (p == NULL) return false;
		return ApplyTheme(p);
	}

	bool CThemeManager::ApplyTheme(CTheme* pTheme)
	{
		if (!m_bEnabled || pTheme == NULL) return false;
		// 重入保护：SetKind → InitKindColors → ApplyTheme → Refresh → SetKind
		static bool s_bApplying = false;
		if (s_bApplying) {
			m_sCurrentId = pTheme->GetId();
			pTheme->ApplyKindColors();
			return true;
		}
		s_bApplying = true;
		m_sCurrentId = pTheme->GetId();
		ApplyCurrentToGlobals();
		RefreshAllManagers();
		s_bApplying = false;
		return true;
	}

	void CThemeManager::ApplyCurrentToGlobals()
	{
		CTheme* p = GetCurrentTheme();
		if (p == NULL) return;
		p->ApplyKindColors();
	}

	DWORD CThemeManager::GetColor(LPCTSTR token, DWORD fallback) const
	{
		if (!m_bEnabled) return fallback;
		CTheme* p = GetCurrentTheme();
		if (p == NULL) return fallback;
		return p->GetToken(token, fallback);
	}

	void CThemeManager::ReapplyKindRecursive(CControlUI* pControl)
	{
		if (pControl == NULL) return;
		ControlKind kind = pControl->GetKind();
		if (kind != CONTROLKIND_NONE) {
			bool bOutline = pControl->IsOutline();
			pControl->SetKind(kind);
			if (bOutline) pControl->SetOutline(true);
		}
		IContainerUI* pCont = static_cast<IContainerUI*>(pControl->GetInterface(_T("IContainer")));
		if (pCont != NULL) {
			for (int i = 0; i < pCont->GetCount(); ++i)
				ReapplyKindRecursive(pCont->GetItemAt(i));
		}
	}

	CTheme* CThemeManager::GetChromeTheme() const
	{
		if (m_bEnabled) {
			CTheme* p = GetCurrentTheme();
			if (p != NULL) return p;
		}
		return FindTheme(m_sDefaultId.GetData());
	}

	bool CThemeManager::IsSelfPanel(CControlUI* pControl)
	{
		if (pControl == NULL) return false;
		LPCTSTR t = pControl->GetCustomAttribute(_T("theme"));
		return (t != NULL && _tcsicmp(t, _T("panel")) == 0);
	}

	void CThemeManager::ResolveEffectiveTheme(CControlUI* pControl, CDuiString& modeOut, CTheme** ppTheme) const
	{
		CTheme* pFallback = const_cast<CThemeManager*>(this)->GetChromeTheme();
		CTheme* pUse = pFallback;
		bool bSelfPanel = false;
		ResolveThemeChrome(pControl, modeOut, bSelfPanel, pUse, pFallback);
		if (ppTheme != NULL) *ppTheme = pUse;
		(void)bSelfPanel;
	}

	void CThemeManager::ResolveThemeChrome(CControlUI* pControl, CDuiString& modeOut, bool& bSelfPanel,
		CTheme*& pThemeOut, CTheme* pFallback)
	{
		modeOut.Empty();
		bSelfPanel = false;
		pThemeOut = pFallback;
		if (pControl == NULL) return;

		CThemeManager* tm = CThemeManager::GetInstance();
		bSelfPanel = IsSelfPanel(pControl);

		for (CControlUI* p = pControl; p != NULL; p = p->GetParent()) {
			LPCTSTR t = p->GetCustomAttribute(_T("theme"));
			if (t == NULL || *t == _T('\0')) continue;
			if (_tcsicmp(t, _T("none")) == 0) {
				modeOut = _T("none");
				break;
			}
			if (_tcsicmp(t, _T("panel")) == 0)
				continue;
			if (_tcsicmp(t, _T("chrome")) == 0 || _tcsicmp(t, _T("secondary")) == 0) {
				modeOut = t;
				break;
			}
			if (tm != NULL) {
				CTheme* local = tm->FindTheme(t);
				if (local != NULL) {
					modeOut = _T("chrome");
					pThemeOut = local;
					break;
				}
			}
			// 未知 theme 值：忽略，继续向上找（避免拼写错误静默关掉 TitleBar/ScrollBar）
			continue;
		}

		for (CControlUI* p = pControl; p != NULL; p = p->GetParent()) {
			LPCTSTR tid = p->GetCustomAttribute(_T("theme-id"));
			if (tid != NULL && *tid != _T('\0') && tm != NULL) {
				CTheme* local = tm->FindTheme(tid);
				if (local != NULL) {
					pThemeOut = local;
					break;
				}
			}
			LPCTSTR t = p->GetCustomAttribute(_T("theme"));
			if (t == NULL || *t == _T('\0')) continue;
			if (_tcsicmp(t, _T("none")) == 0 || _tcsicmp(t, _T("chrome")) == 0
				|| _tcsicmp(t, _T("secondary")) == 0 || _tcsicmp(t, _T("panel")) == 0)
				continue;
			if (tm != NULL) {
				CTheme* local = tm->FindTheme(t);
				if (local != NULL) {
					pThemeOut = local;
					break;
				}
			}
		}
	}

	static void ThemeSetColorAttr(CControlUI* p, LPCTSTR name, DWORD color)
	{
		if (p == NULL || name == NULL) return;
		CDuiString s;
		s.Format(_T("#%08X"), color);
		p->SetAttribute(name, s.GetData());
	}

	static DWORD ThemeSoftMix(DWORD accent, DWORD baseBg, int accentWeight /*0-255*/)
	{
		if (accentWeight < 0) accentWeight = 0;
		if (accentWeight > 255) accentWeight = 255;
		const int w = accentWeight;
		const int iw = 255 - w;
		const int r = (DuiColorR(accent) * w + DuiColorR(baseBg) * iw) / 255;
		const int g = (DuiColorG(accent) * w + DuiColorG(baseBg) * iw) / 255;
		const int b = (DuiColorB(accent) * w + DuiColorB(baseBg) * iw) / 255;
		return DuiColorFromRGB((BYTE)r, (BYTE)g, (BYTE)b, 0xFF);
	}

	namespace {
		CTheme* g_pColorParseTheme = NULL;
		int g_nColorParseThemeDepth = 0;
	}

	void CThemeManager::PushColorParseTheme(CTheme* pTheme)
	{
		if (g_nColorParseThemeDepth == 0)
			g_pColorParseTheme = pTheme;
		++g_nColorParseThemeDepth;
	}

	void CThemeManager::PopColorParseTheme()
	{
		if (g_nColorParseThemeDepth <= 0) return;
		--g_nColorParseThemeDepth;
		if (g_nColorParseThemeDepth == 0)
			g_pColorParseTheme = NULL;
	}

	CTheme* CThemeManager::GetColorParseTheme()
	{
		return g_pColorParseTheme;
	}

	void CThemeManager::RefreshVarAttributesRecursive(CControlUI* pControl)
	{
		if (pControl == NULL) return;

		CDuiString mode;
		CTheme* pUse = NULL;
		CThemeManager* tm = CThemeManager::GetInstance();
		if (tm != NULL)
			tm->ResolveEffectiveTheme(pControl, mode, &pUse);
		const bool bModeNone = (!mode.IsEmpty() && _tcsicmp(mode.GetData(), _T("none")) == 0);
		if (!bModeNone)
			pControl->RefreshThemeVarAttributes(pUse);

		IContainerUI* pCont = static_cast<IContainerUI*>(pControl->GetInterface(_T("IContainer")));
		if (pCont != NULL) {
			for (int i = 0; i < pCont->GetCount(); ++i)
				RefreshVarAttributesRecursive(pCont->GetItemAt(i));
		}
	}

	void CThemeManager::ApplyChromeRecursive(CControlUI* pControl, const CTheme* pTheme)
	{
		if (pControl == NULL || pTheme == NULL) return;

		CDuiString mode;
		bool bSelfPanel = false;
		CTheme* pUse = const_cast<CTheme*>(pTheme);
		ResolveThemeChrome(pControl, mode, bSelfPanel, pUse, const_cast<CTheme*>(pTheme));
		if (pUse == NULL) pUse = const_cast<CTheme*>(pTheme);

		const bool bTitleBar = (pControl->GetInterface(DUI_CTR_TITLEBAR) != NULL);
		const bool bScrollBar = (pControl->GetInterface(DUI_CTR_SCROLLBAR) != NULL);
		const bool bModeNone = (!mode.IsEmpty() && _tcsicmp(mode.GetData(), _T("none")) == 0);
		const bool bModeChrome = (!mode.IsEmpty() && _tcsicmp(mode.GetData(), _T("chrome")) == 0);
		const bool bModeSecondary = (!mode.IsEmpty() && _tcsicmp(mode.GetData(), _T("secondary")) == 0);
		const bool bTypedChrome = (bModeChrome || bModeSecondary);
		const bool bApplyTyped = !bModeNone && (bTypedChrome || ((bTitleBar || bScrollBar) && mode.IsEmpty()));

		if (!bModeNone && (bApplyTyped || bSelfPanel)) {
			DWORD text = pUse->GetToken(_T("color-text"), 0x000000E0);
			DWORD textSec = pUse->GetToken(_T("color-text-secondary"), 0x000000A6);
			DWORD ctrlBg = pUse->GetToken(_T("color-control-bg"), pUse->GetToken(_T("color-bg"), 0xFFFFFFFF));
			DWORD ctrlBd = pUse->GetToken(_T("color-control-border"), pUse->GetToken(_T("color-border"), 0xDEE2E6FF));
			DWORD borderStrong = pUse->GetToken(_T("color-border-strong"), 0xADB5BDFF);
			DWORD ctrlFocus = pUse->GetToken(_T("color-control-border-focus"), pUse->GetToken(_T("color-primary"), borderStrong));
			DWORD disBg = pUse->GetToken(_T("color-disabled-bg"), 0xE9ECEFFF);
			DWORD titleBg = pUse->GetToken(_T("color-titlebar-bg"), 0x333333FF);
			DWORD titleTx = pUse->GetToken(_T("color-titlebar-text"), 0xFFFFFFFF);
			DWORD titleBd = pUse->GetToken(_T("color-titlebar-border"), 0x222222FF);
			DWORD bgElev = pUse->GetToken(_T("color-bg-elevated"), 0xF8F9FAFF);
			DWORD bgHover = pUse->GetToken(_T("color-bg-hover"), bgElev);
			DWORD selection = pUse->GetToken(_T("color-selection"), bgElev);
			DWORD primary = pUse->GetToken(_T("color-primary"), 0x0D6EFDFF);
			DWORD primaryH = pUse->GetToken(_T("color-primary-hover"), primary);
			DWORD primaryA = pUse->GetToken(_T("color-primary-active"), primaryH);
			DWORD primaryOn = pUse->GetToken(_T("color-primary-text"), 0xFFFFFFFF);
			DWORD border = pUse->GetToken(_T("color-border"), 0xDEE2E6FF);
			DWORD sbRail = pUse->GetToken(_T("color-scrollbar-rail"), bgElev);
			DWORD sbThumb = pUse->GetToken(_T("color-scrollbar-thumb"), borderStrong);
			DWORD sbThumbH = pUse->GetToken(_T("color-scrollbar-thumb-hover"), primary);
			DWORD success = pUse->GetToken(_T("color-success"), 0x198754FF);
			DWORD danger = pUse->GetToken(_T("color-danger"), 0xDC3545FF);
			DWORD warning = pUse->GetToken(_T("color-warning"), 0xFFC107FF);
			DWORD rateStar = pUse->GetToken(_T("color-rate"), warning != 0 ? warning : 0xFADB14FF);
			DWORD skeletonBlk = pUse->GetToken(_T("color-skeleton"), ThemeSoftMix(text, bgElev, 24));
			DWORD skeletonHi = pUse->GetToken(_T("color-skeleton-highlight"), DuiColorSetA(0xFFFFFFFF, 0xA0));
			(void)success; (void)warning;

			const bool bForm = (pControl->GetInterface(DUI_CTR_EDIT) != NULL
				|| pControl->GetInterface(DUI_CTR_RICHEDIT) != NULL
				|| pControl->GetInterface(DUI_CTR_SPIN) != NULL
				|| pControl->GetInterface(DUI_CTR_NUMBER) != NULL
				|| pControl->GetInterface(DUI_CTR_COMBO) != NULL
				|| pControl->GetInterface(DUI_CTR_COMBOBOX) != NULL
				|| pControl->GetInterface(DUI_CTR_DATETIME) != NULL
				|| pControl->GetInterface(DUI_CTR_HOTKEY) != NULL
				|| pControl->GetInterface(DUI_CTR_IPADDRESS) != NULL);
			const bool bCombo = (pControl->GetInterface(DUI_CTR_COMBO) != NULL
				|| pControl->GetInterface(DUI_CTR_COMBOBOX) != NULL);
			const bool bDateTime = (pControl->GetInterface(DUI_CTR_DATETIME) != NULL);
			const bool bEdit = (pControl->GetInterface(DUI_CTR_EDIT) != NULL);
			const bool bRichEdit = (pControl->GetInterface(DUI_CTR_RICHEDIT) != NULL);
			const bool bHotKey = (pControl->GetInterface(DUI_CTR_HOTKEY) != NULL);
			const bool bIPAddress = (pControl->GetInterface(DUI_CTR_IPADDRESS) != NULL);
			const bool bAccItem = (pControl->GetInterface(DUI_CTR_ACCORDIONITEM) != NULL);
			const bool bAccordion = (pControl->GetInterface(DUI_CTR_ACCORDION) != NULL && !bAccItem);
			const bool bSwitch = (pControl->GetInterface(DUI_CTR_SWITCH) != NULL);
			const bool bCheckBox = (pControl->GetInterface(DUI_CTR_CHECKBOX) != NULL);
			const bool bOption = (pControl->GetInterface(DUI_CTR_OPTION) != NULL && !bCheckBox && !bSwitch);
			const bool bSlider = (pControl->GetInterface(DUI_CTR_SLIDER) != NULL);
			const bool bProgress = (pControl->GetInterface(DUI_CTR_PROGRESS) != NULL && !bSlider);
			const bool bSegmented = (pControl->GetInterface(DUI_CTR_SEGMENTED) != NULL);
			const bool bTabBar = (pControl->GetInterface(DUI_CTR_TABBAR) != NULL);
			const bool bList = (pControl->GetInterface(DUI_CTR_LIST) != NULL || pControl->GetInterface(DUI_CTR_VIRTUALLIST) != NULL);
			const bool bTransfer = (pControl->GetInterface(DUI_CTR_TRANSFER) != NULL);
			const bool bTag = (pControl->GetInterface(DUI_CTR_TAG) != NULL);
			const bool bBadge = (pControl->GetInterface(DUI_CTR_BADGE) != NULL);
			const bool bRate = (pControl->GetInterface(DUI_CTR_RATE) != NULL);
			const bool bSteps = (pControl->GetInterface(DUI_CTR_STEPS) != NULL);
			const bool bTimeline = (pControl->GetInterface(DUI_CTR_TIMELINE) != NULL);
			const bool bEmpty = (pControl->GetInterface(DUI_CTR_EMPTY) != NULL);
			const bool bSkeleton = (pControl->GetInterface(DUI_CTR_SKELETON) != NULL);
			const bool bLoading = (pControl->GetInterface(DUI_CTR_LOADINGCIRCLE) != NULL);
			const bool bColorPalette = (pControl->GetInterface(DUI_CTR_COLORPALETTE) != NULL);
			const bool bGroupBox = (pControl->GetInterface(_T("GroupBox")) != NULL);
			const bool bPageControl = (_tcscmp(pControl->GetClass(), _T("PageControlUI")) == 0);
			const bool bCarousel = (pControl->GetInterface(DUI_CTR_CAROUSEL) != NULL);
			const bool bCarouselItem = (pControl->GetInterface(DUI_CTR_CAROUSELITEM) != NULL);
			const bool bSidePanel = (pControl->GetInterface(DUI_CTR_SIDEPANEL) != NULL);
			const bool bAvatar = (pControl->GetInterface(DUI_CTR_AVATAR) != NULL);
			const bool bButton = (pControl->GetInterface(DUI_CTR_BUTTON) != NULL);
			const bool bPlainTextLabel = (pControl->GetInterface(DUI_CTR_LABEL) != NULL
				&& !bForm && !bOption && !bSwitch && !bCheckBox && !bProgress && !bSlider
				&& !bTag && !bButton && !bBadge && !bRate && !bAvatar
				&& pControl->GetKind() == CONTROLKIND_NONE
				&& (pControl->GetInterface(DUI_CTR_TEXT) != NULL
					|| _tcscmp(pControl->GetClass(), _T("LabelUI")) == 0));

			if (bSelfPanel) {
				pControl->SetBackgroundColor(bgElev);
				pControl->SetBorderColor(border);
			}

			if (bApplyTyped) {
				if (bTitleBar) {
					ThemeSetColorAttr(pControl, _T("background-color"), titleBg);
					ThemeSetColorAttr(pControl, _T("color"), titleTx);
					ThemeSetColorAttr(pControl, _T("border-color"), titleBd);
					CTitleBarUI* pTb = static_cast<CTitleBarUI*>(pControl->GetInterface(DUI_CTR_TITLEBAR));
					if (pTb != NULL) pTb->SyncSysButtonChrome();
				}
				else if (bScrollBar) {
					CScrollBarUI* pSb = static_cast<CScrollBarUI*>(pControl->GetInterface(DUI_CTR_SCROLLBAR));
					if (pSb != NULL) {
						pSb->SetBackgroundColor(sbRail);
						pSb->SetThumbColor(sbThumb);
						pSb->SetThumbHoverColor(sbThumbH);
						pSb->SetThumbActiveColor(primary);
						pSb->SetThumbDisabledColor(border);
					}
				}
				else if (bSwitch) {
					ThemeSetColorAttr(pControl, _T("color"), text);
					ThemeSetColorAttr(pControl, _T("track-color-checked"), primary);
					ThemeSetColorAttr(pControl, _T("track-color-checked-hover"), primaryH);
					ThemeSetColorAttr(pControl, _T("track-color"), borderStrong);
					ThemeSetColorAttr(pControl, _T("track-color-hover"), border);
					ThemeSetColorAttr(pControl, _T("track-color-disabled"), DuiColorSetA(borderStrong, 0x40));
					ThemeSetColorAttr(pControl, _T("track-color-checked-disabled"), DuiColorSetA(primary, 0x80));
					ThemeSetColorAttr(pControl, _T("thumb-color"), primaryOn);
					ThemeSetColorAttr(pControl, _T("thumb-color-disabled"), disBg);
				}
				else if (bCheckBox) {
					ThemeSetColorAttr(pControl, _T("color"), text);
					ThemeSetColorAttr(pControl, _T("box-background-color"), ctrlBg);
					ThemeSetColorAttr(pControl, _T("box-border-color"), border);
					ThemeSetColorAttr(pControl, _T("box-background-color-hover"), ctrlBg);
					ThemeSetColorAttr(pControl, _T("box-border-color-hover"), primary);
					ThemeSetColorAttr(pControl, _T("box-background-color-selected"), primary);
					ThemeSetColorAttr(pControl, _T("box-border-color-selected"), primary);
					ThemeSetColorAttr(pControl, _T("box-background-color-selected-hover"), primaryA);
					ThemeSetColorAttr(pControl, _T("box-border-color-selected-hover"), primaryA);
					ThemeSetColorAttr(pControl, _T("box-background-color-disabled"), disBg);
					ThemeSetColorAttr(pControl, _T("box-border-color-disabled"), border);
					ThemeSetColorAttr(pControl, _T("checkmark-color"), primaryOn);
				}
				else if (bOption) {
					pControl->SetBackgroundColor(0);
					ThemeSetColorAttr(pControl, _T("color"), text);
					ThemeSetColorAttr(pControl, _T("color-selected"), primary);
					ThemeSetColorAttr(pControl, _T("background-color-selected"), selection);
					ThemeSetColorAttr(pControl, _T("background-color-hover"), bgHover);
				}
				else if (bForm) {
					pControl->SetBackgroundColor(ctrlBg);
					pControl->SetBorderColor(ctrlBd);
					pControl->SetFocusBorderColor(ctrlFocus);
					ThemeSetColorAttr(pControl, _T("background-color-disabled"), disBg);
					CLabelUI* pLabel = static_cast<CLabelUI*>(pControl->GetInterface(DUI_CTR_LABEL));
					if (pLabel != NULL) pLabel->SetColor(text);
					else ThemeSetColorAttr(pControl, _T("color"), text);
					if (bEdit) {
						ThemeSetColorAttr(pControl, _T("native-background-color"), ctrlBg);
						ThemeSetColorAttr(pControl, _T("native-color"), text);
						ThemeSetColorAttr(pControl, _T("placeholder-color"), textSec);
					}
					if (bRichEdit) {
						ThemeSetColorAttr(pControl, _T("placeholder-color"), textSec);
					}
					if (bHotKey) {
						ThemeSetColorAttr(pControl, _T("native-background-color"), ctrlBg);
					}
					if (bIPAddress && !bEdit) {
						ThemeSetColorAttr(pControl, _T("native-background-color"), ctrlBg);
						ThemeSetColorAttr(pControl, _T("native-color"), text);
						CIPAddressUI* pIp = static_cast<CIPAddressUI*>(pControl->GetInterface(DUI_CTR_IPADDRESS));
						if (pIp != NULL) pIp->SyncNativeShellColors();
					}
					if (bCombo) {
						ThemeSetColorAttr(pControl, _T("item-color"), text);
						ThemeSetColorAttr(pControl, _T("item-background-color"), ctrlBg);
						ThemeSetColorAttr(pControl, _T("item-color-hover"), text);
						ThemeSetColorAttr(pControl, _T("item-background-color-hover"), bgHover);
						ThemeSetColorAttr(pControl, _T("item-color-selected"), primary);
						ThemeSetColorAttr(pControl, _T("item-background-color-selected"), selection);
						ThemeSetColorAttr(pControl, _T("item-color-disabled"), textSec);
						ThemeSetColorAttr(pControl, _T("item-background-color-disabled"), disBg);
						ThemeSetColorAttr(pControl, _T("item-line-color"), border);
					}
					if (bDateTime) {
						ThemeSetColorAttr(pControl, _T("selected-background-color"), primary);
						ThemeSetColorAttr(pControl, _T("today-color"), primary);
						ThemeSetColorAttr(pControl, _T("day-hover-background-color"), selection);
						ThemeSetColorAttr(pControl, _T("other-month-color"), textSec);
						ThemeSetColorAttr(pControl, _T("header-color"), text);
						ThemeSetColorAttr(pControl, _T("day-color"), text);
						ThemeSetColorAttr(pControl, _T("selected-color"), primaryOn);
						ThemeSetColorAttr(pControl, _T("muted-color"), textSec);
						CDateTimeUI* pDt = static_cast<CDateTimeUI*>(pControl->GetInterface(DUI_CTR_DATETIME));
						if (pDt != NULL) pDt->SyncOpenCalendarShell();
					}
					if (bCombo) {
						CComboUI* pCombo = static_cast<CComboUI*>(pControl->GetInterface(DUI_CTR_COMBO));
						if (pCombo == NULL)
							pCombo = static_cast<CComboUI*>(pControl->GetInterface(DUI_CTR_COMBOBOX));
						if (pCombo != NULL) pCombo->SyncOpenDropShell();
					}
				}
				else if (bAccordion) {
					ThemeSetColorAttr(pControl, _T("background-color"), ctrlBg);
					ThemeSetColorAttr(pControl, _T("border-color"), border);
				}
				else if (bAccItem) {
					ThemeSetColorAttr(pControl, _T("background-color"), ctrlBg);
					ThemeSetColorAttr(pControl, _T("header-background-color"), bgElev);
					ThemeSetColorAttr(pControl, _T("header-color"), text);
					ThemeSetColorAttr(pControl, _T("header-background-color-hover"), bgHover);
					ThemeSetColorAttr(pControl, _T("header-background-color-active"), selection);
					ThemeSetColorAttr(pControl, _T("header-background-color-active-hover"),
						ThemeSoftMix(primary, selection, 48));
				}
				else if (bSlider || bProgress) {
					ThemeSetColorAttr(pControl, _T("background-color"), border);
					ThemeSetColorAttr(pControl, _T("fore-color"), primary);
					ThemeSetColorAttr(pControl, _T("color"), text);
				}
				else if (bSegmented) {
					ThemeSetColorAttr(pControl, _T("track-color"), bgElev);
					ThemeSetColorAttr(pControl, _T("selected-background-color"), primary);
					ThemeSetColorAttr(pControl, _T("selected-color"), primaryOn);
					ThemeSetColorAttr(pControl, _T("color"), text);
					ThemeSetColorAttr(pControl, _T("hover-color"), primary);
				}
				else if (bTabBar) {
					// 嵌在 TitleBar 内：底/字/悬停跟标题栏亮度，避免正文次要色在深色栏上发黑
					bool bInTitleBar = false;
					for (CControlUI* pAnc = pControl->GetParent(); pAnc != NULL; pAnc = pAnc->GetParent()) {
						if (pAnc->GetInterface(DUI_CTR_TITLEBAR) != NULL) {
							bInTitleBar = true;
							break;
						}
					}
					DWORD tabBarBg = bInTitleBar ? titleBg : bgElev;
					DWORD tabTx = textSec;
					DWORD tabTxHot = primary;
					DWORD tabTxSel = primary;
					DWORD tabHoverBg = bgHover;
					DWORD tabSelBg = selection;
					DWORD tabSep = border;
					DWORD tabClose = textSec;
					DWORD tabIcon = 0;
					DWORD tabIconHot = 0;
					if (bInTitleBar) {
						const int r = (int)((titleBg >> 24) & 0xFF);
						const int g = (int)((titleBg >> 16) & 0xFF);
						const int b = (int)((titleBg >> 8) & 0xFF);
						const int lum = (r * 299 + g * 587 + b * 114) / 1000;
						const bool bLightBar = (lum >= 160);
						const DWORD titleText = (titleTx != 0) ? titleTx
							: (bLightBar ? 0x000000E0u : 0xFFFFFFFFu);
						tabTx = bLightBar ? 0x000000A6u : ((titleText & 0xFFFFFF00u) | 0xB4u);
						tabTxHot = titleText;
						tabTxSel = titleText;
						tabHoverBg = bLightBar ? 0x0000001Au : 0xFFFFFF33u;
						tabSelBg = bLightBar ? 0x0000002Eu : 0xFFFFFF44u;
						tabSep = bLightBar ? 0x00000026u : 0xFFFFFF33u;
						tabClose = tabTx;
						tabIcon = tabTx;
						tabIconHot = titleText;
					}
					ThemeSetColorAttr(pControl, _T("background-color"), tabBarBg);
					ThemeSetColorAttr(pControl, _T("tab-background-color"), tabBarBg);
					ThemeSetColorAttr(pControl, _T("tab-background-color-hover"), tabHoverBg);
					ThemeSetColorAttr(pControl, _T("tab-background-color-selected"), tabSelBg);
					ThemeSetColorAttr(pControl, _T("tab-color"), tabTx);
					ThemeSetColorAttr(pControl, _T("tab-color-hover"), tabTxHot);
					ThemeSetColorAttr(pControl, _T("tab-color-selected"), tabTxSel);
					ThemeSetColorAttr(pControl, _T("tab-border-color"), tabSep);
					ThemeSetColorAttr(pControl, _T("tab-border-color-selected"), primary);
					ThemeSetColorAttr(pControl, _T("tab-separator-color"), tabSep);
					ThemeSetColorAttr(pControl, _T("close-color"), tabClose);
					ThemeSetColorAttr(pControl, _T("tab-loading-color"), primary);
					if (tabIcon != 0) {
						ThemeSetColorAttr(pControl, _T("tab-icon-color"), tabIcon);
						ThemeSetColorAttr(pControl, _T("tab-icon-color-hover"), tabIconHot);
						ThemeSetColorAttr(pControl, _T("tab-icon-color-selected"), tabIconHot);
					}
					CTabBarUI* pTabBar = static_cast<CTabBarUI*>(pControl->GetInterface(DUI_CTR_TABBAR));
					if (pTabBar != NULL) pTabBar->SyncThemeChromeButtons();
				}
				else if (bList) {
					const bool bMenu = (pControl->GetInterface(_T("Menu")) != NULL);
					ThemeSetColorAttr(pControl, _T("item-color"), text);
					ThemeSetColorAttr(pControl, _T("item-background-color"), ctrlBg);
					ThemeSetColorAttr(pControl, _T("item-color-hover"), text);
					ThemeSetColorAttr(pControl, _T("item-background-color-hover"), bgHover);
					ThemeSetColorAttr(pControl, _T("item-color-selected"), primary);
					ThemeSetColorAttr(pControl, _T("item-background-color-selected"), selection);
					ThemeSetColorAttr(pControl, _T("item-color-disabled"), textSec);
					// Menu：禁用项只灰字、不铺灰底（否则「关闭左/右/其他」不可用时像整块脏色且无悬停）
					if( bMenu )
						ThemeSetColorAttr(pControl, _T("item-background-color-disabled"), 0);
					else
						ThemeSetColorAttr(pControl, _T("item-background-color-disabled"), disBg);
					ThemeSetColorAttr(pControl, _T("item-line-color"), border);
					if( !bMenu ) {
						ThemeSetColorAttr(pControl, _T("item-alternate-background-color"), bgElev);
						pControl->SetAttribute(_T("item-alternate-background"), _T("true"));
					}
					else {
						pControl->SetAttribute(_T("item-alternate-background"), _T("false"));
					}
					ThemeSetColorAttr(pControl, _T("background-color"), ctrlBg);
					ThemeSetColorAttr(pControl, _T("border-color"), ctrlBd);
					// Menu 分隔线用 MenuElement::line-color，不走 List item-line
					if (bMenu) {
						IContainerUI* pMenuItems = static_cast<IContainerUI*>(
							pControl->GetInterface(_T("IContainer")));
						if (pMenuItems != NULL) {
							for (int mi = 0; mi < pMenuItems->GetCount(); ++mi) {
								CControlUI* pItem = pMenuItems->GetItemAt(mi);
								if (pItem == NULL) continue;
								CMenuElementUI* pEl = static_cast<CMenuElementUI*>(
									pItem->GetInterface(_T("MenuElement")));
								if (pEl != NULL) pEl->SetLineColor(border);
							}
						}
					}
				}
				else if (bTransfer) {
					ThemeSetColorAttr(pControl, _T("background-color"), ctrlBg);
					ThemeSetColorAttr(pControl, _T("border-color"), border);
					CTransferUI* pTr = static_cast<CTransferUI*>(pControl->GetInterface(DUI_CTR_TRANSFER));
					if (pTr != NULL) {
						pTr->ApplyThemeChrome(ctrlBg, bgElev, border, text, textSec,
							ctrlBg, text, bgHover, selection, primaryOn, border);
					}
				}
				else if (bGroupBox) {
					ThemeSetColorAttr(pControl, _T("color"), text);
					ThemeSetColorAttr(pControl, _T("color-disabled"), textSec);
					ThemeSetColorAttr(pControl, _T("border-color"), border);
				}
				else if (bPageControl) {
					ThemeSetColorAttr(pControl, _T("page-color"), text);
					ThemeSetColorAttr(pControl, _T("page-color-selected"), primary);
					ThemeSetColorAttr(pControl, _T("page-background-color-selected"), selection);
					ThemeSetColorAttr(pControl, _T("page-color-hover"), primary);
					ThemeSetColorAttr(pControl, _T("page-background-color-hover"), bgHover);
					ThemeSetColorAttr(pControl, _T("goto-edit-border-color"), border);
				}
				else if (bCarousel) {
					ThemeSetColorAttr(pControl, _T("background-color"), bgElev);
					CCarouselUI* pCar = static_cast<CCarouselUI*>(pControl->GetInterface(DUI_CTR_CAROUSEL));
					if (pCar != NULL) {
						DWORD barBg = pUse->GetToken(_T("color-titlebar-bg"), 0x32323CFF);
						pCar->ApplyThemeChrome(barBg, textSec, borderStrong, primaryOn);
					}
				}
				else if (bCarouselItem) {
					CCarouselItemUI* pItem = static_cast<CCarouselItemUI*>(
						pControl->GetInterface(DUI_CTR_CAROUSELITEM));
					if (pItem != NULL) {
						DWORD barBg = pUse->GetToken(_T("color-titlebar-bg"), 0x32323CFF);
						pItem->ApplyThemeCaption(barBg, primaryOn, textSec);
					}
				}
				else if (bSidePanel) {
					CSidePanelUI* pSp = static_cast<CSidePanelUI*>(
						pControl->GetInterface(DUI_CTR_SIDEPANEL));
					if (pSp != NULL)
						pSp->ApplyThemeChrome(bgElev, border, text);
				}
				else if (bTag) {
					CTagUI* pTag = static_cast<CTagUI*>(pControl->GetInterface(DUI_CTR_TAG));
					if (pTag != NULL) pTag->ApplyStatusColors();
				}
				else if (bBadge) {
					ThemeSetColorAttr(pControl, _T("badge-color"), danger);
					ThemeSetColorAttr(pControl, _T("badge-text-color"), primaryOn);
				}
				else if (bRate) {
					ThemeSetColorAttr(pControl, _T("star-color"), rateStar);
					ThemeSetColorAttr(pControl, _T("void-color"), DuiColorSetA(text, 0x26));
				}
				else if (bSteps || bTimeline) {
					ThemeSetColorAttr(pControl, _T("finish-color"), primary);
					ThemeSetColorAttr(pControl, _T("process-color"), primary);
					ThemeSetColorAttr(pControl, _T("wait-color"), DuiColorSetA(text, 0x40));
					ThemeSetColorAttr(pControl, _T("title-color"), text);
					ThemeSetColorAttr(pControl, _T("description-color"), textSec);
					if (bSteps) {
						ThemeSetColorAttr(pControl, _T("error-color"), danger);
					}
					if (bTimeline) {
						ThemeSetColorAttr(pControl, _T("line-color"), DuiColorSetA(text, 0x26));
						ThemeSetColorAttr(pControl, _T("time-color"), textSec);
					}
				}
				else if (bEmpty) {
					ThemeSetColorAttr(pControl, _T("description-color"), textSec);
				}
				else if (bSkeleton) {
					ThemeSetColorAttr(pControl, _T("block-color"), skeletonBlk);
					ThemeSetColorAttr(pControl, _T("highlight-color"), skeletonHi);
				}
				else if (bLoading) {
					ThemeSetColorAttr(pControl, _T("color"), primary);
					ThemeSetColorAttr(pControl, _T("track-color"), border);
				}
				else if (bColorPalette) {
					ThemeSetColorAttr(pControl, _T("select-color"), primary);
				}
				else if (bModeSecondary && bPlainTextLabel) {
					CLabelUI* pLabel = static_cast<CLabelUI*>(pControl->GetInterface(DUI_CTR_LABEL));
					if (pLabel != NULL) pLabel->SetColor(textSec);
				}
			}
		}

		CContainerUI* pBox = static_cast<CContainerUI*>(pControl->GetInterface(DUI_CTR_CONTAINER));
		if (pBox != NULL) {
			CScrollBarUI* bars[2] = { pBox->GetVerticalScrollBar(), pBox->GetHorizontalScrollBar() };
			for (int bi = 0; bi < 2; ++bi) {
				if (bars[bi] != NULL)
					ApplyChromeRecursive(bars[bi], pTheme);
			}
		}

		IContainerUI* pCont = static_cast<IContainerUI*>(pControl->GetInterface(_T("IContainer")));
		if (pCont != NULL) {
			for (int i = 0; i < pCont->GetCount(); ++i)
				ApplyChromeRecursive(pCont->GetItemAt(i), pTheme);
		}
	}

void CThemeManager::ApplyManagerDefaults(CPaintManagerUI* pManager)
	{
		if (pManager == NULL) return;
		EnsureInitialized();
		CTheme* pTheme = GetChromeTheme();
		if (pTheme != NULL)
			pTheme->ApplyToManager(pManager);
	}

	void CThemeManager::ApplyMenuChrome(CMenuUI* pMenu)
	{
		if (pMenu == NULL || !m_bEnabled) return;
		EnsureInitialized();

		LPCTSTR mode = pMenu->GetCustomAttribute(_T("theme"));
		if (mode != NULL && _tcsicmp(mode, _T("none")) == 0) return;

		LPCTSTR img = pMenu->GetBackgroundImage();
		if (img != NULL && *img != _T('\0')) return;

		if (mode == NULL || *mode == _T('\0'))
			pMenu->SetAttribute(_T("theme"), _T("chrome"));

		CTheme* pTheme = GetChromeTheme();
		if (pTheme == NULL) return;
		ApplyChromeRecursive(pMenu, pTheme);
		pMenu->Invalidate();
	}

	void CThemeManager::ApplyChromeToManager(CPaintManagerUI* pManager)
	{
		if (pManager == NULL) return;
		EnsureInitialized();
		CTheme* pTheme = GetChromeTheme();
		CControlUI* pRoot = pManager->GetRoot();
		if (pTheme != NULL && pRoot != NULL)
			ApplyChromeRecursive(pRoot, pTheme);
	}

	void CThemeManager::ApplyToExistingManager(CPaintManagerUI* pManager)
	{
		ApplyManagerDefaults(pManager);
		ApplyChromeToManager(pManager);
	}

	void CThemeManager::RefreshAllManagers()
	{
		static bool s_bRefreshing = false;
		if (s_bRefreshing) return;
		s_bRefreshing = true;

		CTheme* pChrome = GetChromeTheme();
		CStdPtrArray* pManagers = CPaintManagerUI::GetPaintManagers();
		if (pManagers != NULL) {
			for (int i = 0; i < pManagers->GetSize(); ++i) {
				CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>((*pManagers)[i]);
				if (pManager == NULL) continue;
				CControlUI* pRoot = pManager->GetRoot();
				// Toast / Modal 内容在 BuildUI 时按主题快照；半套 Refresh 会花屏，跳过
				if (pRoot != NULL) {
					LPCTSTR rootName = pRoot->GetName();
					if (rootName != NULL && (_tcscmp(rootName, _T("toastRoot")) == 0
						|| _tcscmp(rootName, _T("modalRoot")) == 0))
						continue;
				}
				if (pChrome != NULL)
					pChrome->ApplyToManager(pManager);
				if (pRoot != NULL) {
					ReapplyKindRecursive(pRoot);
					CMenuUI* pMenuRoot = static_cast<CMenuUI*>(pRoot->GetInterface(_T("Menu")));
					if (pMenuRoot != NULL)
						ApplyMenuChrome(pMenuRoot);
					else if (pChrome != NULL)
						ApplyChromeRecursive(pRoot, pChrome);
					RefreshVarAttributesRecursive(pRoot);
				}
				pManager->NeedUpdate();
			}
		}
		s_bRefreshing = false;
	}

	static bool ParseThemeFileTokens(LPCTSTR path, CTheme* pTheme)
	{
		if (path == NULL || pTheme == NULL) return false;
		FILE* fp = NULL;
#if defined(_MSC_VER)
		if (_tfopen_s(&fp, path, _T("r")) != 0 || fp == NULL) return false;
#else
		fp = _tfopen(path, _T("r"));
		if (fp == NULL) return false;
#endif
		TCHAR line[1024];
		bool bInRoot = false;
		while (_fgetts(line, 1023, fp) != NULL) {
			CDuiString s(line);
			s.TrimLeft();
			s.TrimRight();
			if (s.IsEmpty()) continue;
			if (s[0] == _T('/') && s.GetLength() > 1 && s[1] == _T('/')) continue;
			if (s.Find(_T(":root")) >= 0) {
				bInRoot = true;
				continue;
			}
			if (s.Find(_T('}')) >= 0) {
				bInRoot = false;
				continue;
			}
			if (!bInRoot) continue;
			// --color-primary: #1677FFFF;
			int dash = s.Find(_T("--"));
			if (dash < 0) continue;
			int colon = s.Find(_T(':'), dash);
			if (colon < 0) continue;
			CDuiString name = s.Mid(dash + 2, colon - dash - 2);
			name.TrimLeft();
			name.TrimRight();
			CDuiString val = s.Mid(colon + 1);
			int semi = val.Find(_T(';'));
			if (semi >= 0) val = val.Left(semi);
			val.TrimLeft();
			val.TrimRight();
			if (!name.IsEmpty() && !val.IsEmpty())
				pTheme->SetToken(name.GetData(), val.GetData());
		}
		fclose(fp);
		return pTheme->GetTokenCount() > 0;
	}

	bool CThemeManager::ApplyThemeFile(LPCTSTR path, LPCTSTR idOverride)
	{
		if (!m_bEnabled || path == NULL) return false;
		CDuiString id = idOverride;
		if (id.IsEmpty()) {
			id = path;
			int slash = id.ReverseFind(_T('\\'));
			int slash2 = id.ReverseFind(_T('/'));
			if (slash2 > slash) slash = slash2;
			if (slash >= 0) id = id.Mid(slash + 1);
			int dot = id.ReverseFind(_T('.'));
			if (dot > 0) id = id.Left(dot);
		}
		CTheme* p = new CTheme(id.GetData(), id.GetData());
		CTheme* pBase = FindTheme(_T("default"));
		if (pBase != NULL) p->CopyTokensFrom(*pBase);
		if (!ParseThemeFileTokens(path, p)) {
			delete p;
			return false;
		}
		RegisterTheme(p, true);
		return ApplyTheme(p);
	}

} // namespace DuiLib
