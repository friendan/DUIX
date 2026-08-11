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

	float ColorLum(DWORD c)
	{
		return (0.2126f * DuiColorR(c) + 0.7152f * DuiColorG(c) + 0.0722f * DuiColorB(c)) / 255.0f;
	}

	DWORD ColorMix(DWORD a, DWORD b, int aWeight)
	{
		if( aWeight < 0 ) aWeight = 0;
		if( aWeight > 255 ) aWeight = 255;
		const int w = aWeight;
		const int iw = 255 - w;
		return DuiColorFromRGB(
			(BYTE)((DuiColorR(a) * w + DuiColorR(b) * iw) / 255),
			(BYTE)((DuiColorG(a) * w + DuiColorG(b) * iw) / 255),
			(BYTE)((DuiColorB(a) * w + DuiColorB(b) * iw) / 255),
			0xFF);
	}

	DWORD ColorOn(DWORD bg)
	{
		return ColorLum(bg) > 0.55f
			? DuiColorFromRGB(0, 0, 0, 0xFF)
			: DuiColorFromRGB(255, 255, 255, 0xFF);
	}

	DWORD ColorHover(DWORD base)
	{
		if( ColorLum(base) < 0.45f )
			return ColorMix(DuiColorFromRGB(255, 255, 255, 0xFF), base, 48);
		return ColorMix(DuiColorFromRGB(0, 0, 0, 0xFF), base, 36);
	}

	DWORD ColorActive(DWORD base)
	{
		if( ColorLum(base) < 0.45f )
			return ColorMix(DuiColorFromRGB(0, 0, 0, 0xFF), base, 42);
		return ColorMix(DuiColorFromRGB(0, 0, 0, 0xFF), base, 58);
	}

	void ColorToHsl(DWORD c, float& H, float& S, float& L)
	{
		const float nR = DuiColorR(c) / 255.0f;
		const float nG = DuiColorG(c) / 255.0f;
		const float nB = DuiColorB(c) / 255.0f;
		const float m = min(min(nR, nG), nB);
		const float M = max(max(nR, nG), nB);
		L = (m + M) / 2.0f;
		if( M == m ) { H = S = 0; return; }
		const float f = (nR == m) ? (nG - nB) : ((nG == m) ? (nB - nR) : (nR - nG));
		const float i = (nR == m) ? 3.0f : ((nG == m) ? 5.0f : 1.0f);
		H = (i - f / (M - m));
		if( H >= 6.0f ) H -= 6.0f;
		H *= 60.0f;
		S = (2.0f * L <= 1.0f) ? ((M - m) / (M + m)) : ((M - m) / (2.0f - M - m));
	}

	DWORD ColorFromHsl(float H, float S, float L, BYTE a = 0xFF)
	{
		while( H < 0 ) H += 360.0f;
		while( H >= 360.0f ) H -= 360.0f;
		if( S < 0 ) S = 0;
		if( S > 1 ) S = 1;
		if( L < 0 ) L = 0;
		if( L > 1 ) L = 1;
		const float q = (2.0f * L < 1.0f) ? (L * (1.0f + S)) : (L + S - L * S);
		const float p = 2.0f * L - q;
		const float OneThird = 1.0f / 3.0f;
		const float h = H / 360.0f;
		auto channel = [&](float t) -> float {
			if( t < 0 ) t += 1.0f;
			if( t > 1 ) t -= 1.0f;
			if( 6.0f * t < 1.0f ) return p + (q - p) * 6.0f * t;
			if( 2.0f * t < 1.0f ) return q;
			if( 3.0f * t < 2.0f ) return p + (q - p) * 6.0f * (2.0f * OneThird - t);
			return p;
		};
		const float R = 255.0f * channel(h + OneThird);
		const float G = 255.0f * channel(h);
		const float B = 255.0f * channel(h - OneThird);
		auto clampb = [](float v) -> BYTE {
			if( v < 0 ) return 0;
			if( v > 255 ) return 255;
			return (BYTE)(v + 0.5f);
		};
		return DuiColorFromRGB(clampb(R), clampb(G), clampb(B), a);
	}

	DWORD ColorHueShift(DWORD c, float deg, float sMin = 0.55f, float lClampLo = 0.42f, float lClampHi = 0.58f)
	{
		float h = 0, s = 0, l = 0;
		ColorToHsl(c, h, s, l);
		h += deg;
		if( s < sMin ) s = sMin;
		if( l < lClampLo ) l = lClampLo;
		if( l > lClampHi ) l = lClampHi;
		return ColorFromHsl(h, s, l, 0xFF);
	}

	void SetKindFamily(CTheme* t, LPCTSTR stem, DWORD base, bool bFullBorder)
	{
		if( t == NULL || stem == NULL ) return;
		const DWORD hov = ColorHover(base);
		const DWORD act = ColorActive(base);
		const DWORD on = ColorOn(base);
		CDuiString n;
		t->SetToken(stem, base);
		n.Format(_T("%s-hover"), stem); t->SetToken(n.GetData(), hov);
		n.Format(_T("%s-active"), stem); t->SetToken(n.GetData(), act);
		n.Format(_T("%s-text"), stem); t->SetToken(n.GetData(), on);
		if( bFullBorder ) {
			n.Format(_T("%s-border"), stem); t->SetToken(n.GetData(), base);
			n.Format(_T("%s-border-hover"), stem); t->SetToken(n.GetData(), hov);
			n.Format(_T("%s-border-active"), stem); t->SetToken(n.GetData(), act);
		}
		else {
			n.Format(_T("%s-border-hover"), stem); t->SetToken(n.GetData(), hov);
			n.Format(_T("%s-border-active"), stem); t->SetToken(n.GetData(), act);
		}
	}

	struct SeedDef { LPCTSTR token; LPCTSTR title; LPCTSTR tip; };
	const SeedDef* GetSeedDefs(int& nOut)
	{
		static const SeedDef kSeeds[] = {
			{ _T("color-primary"), _T("主色"), _T("按钮 / 强调 / 链接基调") },
			{ _T("color-bg"), _T("背景"), _T("窗口与页面底色") },
			{ _T("color-titlebar-bg"), _T("标题栏"), _T("标题栏背景") },
			{ _T("color-text"), _T("正文"), _T("主要文字色") },
			{ _T("color-success"), _T("成功"), _T("正向语义色") },
			{ _T("color-warning"), _T("警告"), _T("警告语义色") },
			{ _T("color-danger"), _T("危险"), _T("错误 / 危险语义色") },
		};
		nOut = (int)(sizeof(kSeeds) / sizeof(kSeeds[0]));
		return kSeeds;
	}

	bool IsSeedTokenName(LPCTSTR token)
	{
		int n = 0;
		const SeedDef* p = GetSeedDefs(n);
		for( int i = 0; i < n; ++i ) {
			if( _tcsicmp(token, p[i].token) == 0 ) return true;
		}
		return false;
	}

	/// 用 7 个种子色铺开整盘 token（覆盖派生色；种子本身保留）
	void GenerateThemeTokensFromSeeds(CTheme* t)
	{
		if( t == NULL ) return;
		const DWORD primary = t->GetToken(_T("color-primary"), 0x1677FFFF);
		const DWORD bg = t->GetToken(_T("color-bg"), 0xFFFFFFFF);
		const DWORD title = t->GetToken(_T("color-titlebar-bg"), primary);
		const DWORD text = t->GetToken(_T("color-text"), 0x000000E0);
		const DWORD success = t->GetToken(_T("color-success"), 0x198754FF);
		const DWORD warning = t->GetToken(_T("color-warning"), 0xFFC107FF);
		const DWORD danger = t->GetToken(_T("color-danger"), 0xDC3545FF);

		const bool bDarkBg = ColorLum(bg) < 0.45f;
		const DWORD ink = bDarkBg
			? DuiColorFromRGB(255, 255, 255, 0xFF)
			: DuiColorFromRGB(0, 0, 0, 0xFF);
		const DWORD paper = bDarkBg
			? DuiColorFromRGB(0, 0, 0, 0xFF)
			: DuiColorFromRGB(255, 255, 255, 0xFF);

		DWORD textSolid = DuiColorFromRGB(DuiColorR(text), DuiColorG(text), DuiColorB(text), 0xFF);
		if( ColorLum(ColorMix(textSolid, bg, 200)) < 0.15f && !bDarkBg )
			textSolid = DuiColorFromRGB(33, 37, 41, 0xFF);
		else if( ColorLum(ColorMix(textSolid, bg, 200)) > 0.85f && bDarkBg )
			textSolid = DuiColorFromRGB(222, 226, 230, 0xFF);

		const DWORD textSec = ColorMix(textSolid, bg, 150);
		const DWORD textDis = ColorMix(textSolid, bg, 90);
		const DWORD elev = bDarkBg ? ColorMix(paper, bg, 28) : ColorMix(ink, bg, 10);
		const DWORD bgHover = ColorMix(primary, bg, 28);
		const DWORD border = ColorMix(textSolid, bg, 48);
		const DWORD borderStrong = ColorMix(textSolid, bg, 90);
		const DWORD ctrlBg = bDarkBg ? ColorMix(paper, bg, 18) : bg;
		const DWORD disabledBg = ColorMix(textSolid, bg, 28);
		const DWORD selection = ColorMix(primary, bg, 36);
		const DWORD scrollRail = ColorMix(textSolid, bg, 20);
		const DWORD scrollThumb = ColorMix(textSolid, bg, 70);
		const DWORD scrollThumbH = ColorMix(textSolid, bg, 100);
		const DWORD secondary = ColorMix(textSolid, bg, 110);
		const DWORD light = bDarkBg ? ColorMix(paper, bg, 40) : ColorMix(ink, bg, 6);
		const DWORD dark = bDarkBg ? ColorMix(paper, bg, 70) : ColorMix(ink, bg, 200);
		const DWORD info = ColorHueShift(primary, 155.0f);
		const DWORD rate = ColorMix(warning, primary, 80);
		const DWORD skeleton = ColorMix(textSolid, bg, 36);
		const DWORD skeletonHi = ColorMix(textSolid, bg, 18);

		SetKindFamily(t, _T("color-primary"), primary, true);
		SetKindFamily(t, _T("color-secondary"), secondary, false);
		SetKindFamily(t, _T("color-success"), success, false);
		SetKindFamily(t, _T("color-danger"), danger, false);
		SetKindFamily(t, _T("color-warning"), warning, false);
		SetKindFamily(t, _T("color-info"), info, false);
		SetKindFamily(t, _T("color-light"), light, false);
		SetKindFamily(t, _T("color-dark"), dark, false);

		const DWORD defBase = elev;
		const DWORD defBorder = border;
		t->SetToken(_T("color-default"), defBase);
		t->SetToken(_T("color-default-border"), defBorder);
		t->SetToken(_T("color-default-text"), textSolid);
		t->SetToken(_T("color-default-hover"), ColorHover(defBase));
		t->SetToken(_T("color-default-border-hover"), ColorHover(defBorder));
		t->SetToken(_T("color-default-active"), ColorActive(defBase));
		t->SetToken(_T("color-default-border-active"), ColorActive(defBorder));

		t->SetToken(_T("color-link"), primary);
		t->SetToken(_T("color-link-hover"), ColorHover(primary));
		t->SetToken(_T("color-link-active"), ColorActive(primary));

		t->SetToken(_T("color-bg"), bg);
		t->SetToken(_T("color-bg-elevated"), elev);
		t->SetToken(_T("color-bg-hover"), bgHover);
		t->SetToken(_T("color-border"), border);
		t->SetToken(_T("color-border-strong"), borderStrong);
		t->SetToken(_T("color-text"), textSolid);
		t->SetToken(_T("color-text-secondary"), textSec);
		t->SetToken(_T("color-text-disabled"), textDis);
		t->SetToken(_T("color-disabled-text"), textDis);
		t->SetToken(_T("color-disabled-bg"), disabledBg);
		t->SetToken(_T("color-control-bg"), ctrlBg);
		t->SetToken(_T("color-control-border"), border);
		t->SetToken(_T("color-control-border-focus"), primary);
		t->SetToken(_T("color-selection"), selection);
		t->SetToken(_T("color-scrollbar-rail"), scrollRail);
		t->SetToken(_T("color-scrollbar-thumb"), scrollThumb);
		t->SetToken(_T("color-scrollbar-thumb-hover"), scrollThumbH);

		t->SetToken(_T("color-titlebar-bg"), title);
		t->SetToken(_T("color-titlebar-text"), ColorOn(title));
		t->SetToken(_T("color-titlebar-border"), ColorActive(title));
		t->SetToken(_T("color-titlebar-close-hover"), 0xE81123FF);

		t->SetToken(_T("color-modal-bg"), ctrlBg);
		t->SetToken(_T("color-modal-text"), textSolid);
		t->SetToken(_T("color-modal-border"), border);

		t->SetToken(_T("color-rate"), rate);
		t->SetToken(_T("color-skeleton"), skeleton);
		t->SetToken(_T("color-skeleton-highlight"), skeletonHi);
	}

	float ColorLerp(float a, float b, float t)
	{
		if( t < 0 ) t = 0;
		if( t > 1 ) t = 1;
		return a + (b - a) * t;
	}

	struct FamilyDef {
		LPCTSTR id;
		LPCTSTR title;
		float hue;
		float sat;
		bool forceDark;
		DWORD swatch;
	};

	const FamilyDef* GetFamilyDefs(int& nOut)
	{
		// 日常习惯：冷色优先（绿→蓝→紫），再暖色，最后中性/深色
		static const FamilyDef kFam[] = {
			{ _T("green"),  _T("绿色系"), 152.0f, 0.62f, false, 0x10B981FF },
			{ _T("cyan"),   _T("青色系"), 190.0f, 0.70f, false, 0x06B6D4FF },
			{ _T("blue"),   _T("蓝色系"), 210.0f, 0.78f, false, 0x1677FFFF },
			{ _T("purple"), _T("紫色系"), 270.0f, 0.58f, false, 0x8B5CF6FF },
			{ _T("orange"), _T("橙色系"),  24.0f, 0.88f, false, 0xF97316FF },
			{ _T("red"),    _T("红色系"),   4.0f, 0.72f, false, 0xEF4444FF },
			{ _T("gray"),   _T("灰色系"), 220.0f, 0.10f, false, 0x64748BFF },
			{ _T("dark"),   _T("深色系"), 215.0f, 0.28f, true,  0x334155FF },
		};
		nOut = (int)(sizeof(kFam) / sizeof(kFam[0]));
		return kFam;
	}

	const FamilyDef* FindFamilyDef(LPCTSTR id)
	{
		int n = 0;
		const FamilyDef* p = GetFamilyDefs(n);
		if( id == NULL || *id == _T('\0') ) return &p[0];
		for( int i = 0; i < n; ++i ) {
			if( _tcsicmp(id, p[i].id) == 0 ) return &p[i];
		}
		return &p[0];
	}

	/// 色系 + 色相微调(度) + 明暗(0暗~100亮) → 写入种子并 Generate
	void ApplyColorFamilyToTheme(CTheme* t, LPCTSTR familyId, int brightness, int hueShift)
	{
		if( t == NULL ) return;
		if( brightness < 0 ) brightness = 0;
		if( brightness > 100 ) brightness = 100;
		if( hueShift < -40 ) hueShift = -40;
		if( hueShift > 40 ) hueShift = 40;
		const FamilyDef* fam = FindFamilyDef(familyId);
		const float tb = brightness / 100.0f;
		float hue = fam->hue + (float)hueShift;
		while( hue < 0.0f ) hue += 360.0f;
		while( hue >= 360.0f ) hue -= 360.0f;
		float sat = fam->sat;

		const DWORD success = ColorFromHsl(145.0f, 0.55f, ColorLerp(0.38f, 0.48f, tb));
		const DWORD warning = ColorFromHsl(42.0f, 0.90f, ColorLerp(0.48f, 0.56f, tb));
		const DWORD danger = ColorFromHsl(2.0f, 0.72f, ColorLerp(0.45f, 0.52f, tb));

		DWORD primary = 0, bg = 0, text = 0, title = 0;
		const bool useDark = fam->forceDark || tb < 0.40f;
		if( useDark ) {
			const float darkT = fam->forceDark ? tb : (tb / 0.40f);
			bg = ColorFromHsl(hue, sat * 0.14f, ColorLerp(0.07f, 0.16f, darkT));
			text = ColorFromHsl(hue, 0.05f, ColorLerp(0.86f, 0.94f, darkT));
			primary = ColorFromHsl(hue, (sat < 0.35f ? 0.40f : sat), ColorLerp(0.48f, 0.58f, darkT));
			title = ColorFromHsl(hue, (sat < 0.20f ? 0.18f : sat * 0.55f), ColorLerp(0.12f, 0.22f, darkT));
		}
		else {
			const float lightT = (tb - 0.40f) / 0.60f;
			bg = ColorFromHsl(hue, sat * 0.05f, ColorLerp(0.90f, 0.985f, lightT));
			text = ColorFromHsl(hue, (sat < 0.15f ? 0.04f : 0.12f), ColorLerp(0.22f, 0.14f, lightT));
			primary = ColorFromHsl(hue, sat, ColorLerp(0.42f, 0.52f, lightT));
			title = primary;
		}

		t->SetToken(_T("color-primary"), primary);
		t->SetToken(_T("color-bg"), bg);
		t->SetToken(_T("color-titlebar-bg"), title);
		t->SetToken(_T("color-text"), text);
		t->SetToken(_T("color-success"), success);
		t->SetToken(_T("color-warning"), warning);
		t->SetToken(_T("color-danger"), danger);
		GenerateThemeTokensFromSeeds(t);
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
		// Must start with '<' — DialogBuilder treats non-'<' as a file path (else ExitProcess).
		// 工程固定 UNICODE，只用宽字面量（见 StdAfx.h / AGENTS.md）。
		return LR"dui(<html theme="chrome">
  <VBox name="root" gap="0">
    <TitleBar name="titlebar" title="选择主题" height="36"
        show-min="true" show-max="true" show-close="true" action="title" />
    <VBox padding="10,10,10,10" gap="8" flexible="true">
      <HBox height="36" gap="8" align-items="vcenter">
        <Button name="btn_new" text="新建主题" kind="default" width="88" height="28" />
        <Button name="btn_import" text="导入…" kind="default" width="72" height="28" />
        <Button name="btn_save" text="另存为…" kind="default" width="80" height="28" />
        <Control flexible="true" />
        <Button name="btn_ok" text="确定" kind="primary" width="72" height="28" />
        <Button name="btn_cancel" text="取消" kind="default" width="72" height="28" />
      </HBox>
      <HBox height="32" gap="8" align-items="vcenter">
        <Segmented name="modeSeg" options="色系:family|简易:simple|高级:advanced" selected="0"
            width="220" height="28" block="true" />
        <Label name="editHint" flexible="true" height="22" theme="secondary"
            text="选色系，再拖色相/明暗即可生成主题" />
      </HBox>
      <HBox gap="10" flexible="true">
        <VBox name="themeList" width="168" overflow="auto" theme="panel" padding="6,6,6,6" gap="4" />
        <VBox flexible="true" gap="8">
          <HBox name="miniPreview" height="40" gap="6" padding="4,4,4,4" theme="panel" align-items="vcenter" />
          <VBox name="familyPanel" overflow="auto" flexible="true" theme="panel" padding="10,10,10,10" gap="10">
            <Label text="选色系；色相微调可滑到邻近色（如绿↔青），再调明暗" height="24" theme="secondary" />
            <HBox name="familyChips" height="44" gap="8" align-items="vcenter" />
            <HBox height="36" gap="10" align-items="vcenter">
              <Label text="色相" width="36" theme="secondary" />
              <Slider name="hueSlider" flexible="true" height="22" min="-30" max="30" value="0"
                  step="1" send-move="true" horizontal="true" />
              <Label name="hueLabel" text="0°" width="48" />
            </HBox>
            <HBox height="36" gap="10" align-items="vcenter">
              <Label text="偏暗" width="36" theme="secondary" />
              <Slider name="brightSlider" flexible="true" height="22" min="0" max="100" value="70"
                  step="1" send-move="true" horizontal="true" />
              <Label text="偏亮" width="36" theme="secondary" />
              <Label name="brightLabel" text="70" width="48" />
            </HBox>
            <Label text="换色系会重置色相；「深色系」始终深色表面" height="22" theme="secondary" />
          </VBox>
          <VBox name="seedPanel" overflow="auto" flexible="true" theme="panel" padding="8,8,8,8" gap="4" visible="false" />
          <VBox name="tokenList" overflow="auto" flexible="true" theme="panel" padding="4,4,4,4" gap="2" visible="false" />
          <ColorPalette name="palette" height="180" palette-height="140" bar-height="16"
              select-color="#1677FFFF" visible="false" />
        </VBox>
      </HBox>
    </VBox>
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
	, m_bModal(false)
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

bool CThemeSwitcherUI::IsThemeListed(LPCTSTR id) const
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

bool CThemeSwitcherUI::IncludeTheme(LPCTSTR id, LPCTSTR displayName)
{
	if( id == NULL || *id == _T('\0') ) return false;
	if( displayName != NULL && *displayName != _T('\0') )
		SetThemeDisplayName(id, displayName);
	// 白名单为空=显示全部已注册，无需写入
	if( m_sThemesFilter.IsEmpty() ) return true;
	if( IsThemeListed(id) ) return true;
	if( !m_sThemesFilter.IsEmpty() )
		m_sThemesFilter += _T(",");
	m_sThemesFilter += id;
	return true;
}

bool CThemeSwitcherUI::ExcludeTheme(LPCTSTR id)
{
	if( id == NULL || *id == _T('\0') ) return false;
	if( m_sThemesFilter.IsEmpty() ) return false;
	CDuiString filter = m_sThemesFilter;
	CDuiString out;
	int start = 0;
	bool removed = false;
	while( start <= filter.GetLength() ) {
		int comma = filter.Find(_T(','), start);
		CDuiString part = (comma < 0) ? filter.Mid(start) : filter.Mid(start, comma - start);
		part.TrimLeft();
		part.TrimRight();
		if( !part.IsEmpty() ) {
			if( part.CompareNoCase(id) == 0 )
				removed = true;
			else {
				if( !out.IsEmpty() ) out += _T(",");
				out += part;
			}
		}
		if( comma < 0 ) break;
		start = comma + 1;
	}
	if( removed ) m_sThemesFilter = out;
	return removed;
}

bool CThemeSwitcherUI::SetThemeDisplayName(LPCTSTR id, LPCTSTR displayName)
{
	if( id == NULL || *id == _T('\0') ) return false;
	if( displayName == NULL || *displayName == _T('\0') ) return false;
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return false;
	CTheme* p = tm->FindTheme(id);
	if( p == NULL ) return false;
	p->SetDisplayName(displayName);
	if( m_sThemeId.CompareNoCase(id) == 0 )
		SyncFromManager();
	return true;
}

LPCTSTR CThemeSwitcherUI::GetThemeDisplayName(LPCTSTR id) const
{
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return _T("");
	CTheme* p = tm->FindTheme(id);
	if( p == NULL ) return _T("");
	LPCTSTR dn = p->GetDisplayName();
	if( dn == NULL || *dn == _T('\0') ) return p->GetId();
	return dn;
}

bool CThemeSwitcherUI::AddTheme(CTheme* pTheme, bool bOwn, LPCTSTR displayName)
{
	if( pTheme == NULL ) return false;
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return false;
	if( displayName != NULL && *displayName != _T('\0') )
		pTheme->SetDisplayName(displayName);
	if( !tm->RegisterTheme(pTheme, bOwn) ) return false;
	IncludeTheme(pTheme->GetId());
	return true;
}

CTheme* CThemeSwitcherUI::AddThemeFile(LPCTSTR path, LPCTSTR idOverride, LPCTSTR displayName)
{
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return NULL;
	CTheme* p = tm->LoadThemeFile(path, idOverride, displayName);
	if( p == NULL ) return NULL;
	IncludeTheme(p->GetId());
	return p;
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

void CThemeSwitcherUI::SetModal(bool bModal)
{
	m_bModal = bModal;
}

bool CThemeSwitcherUI::IsModal() const
{
	return m_bModal;
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
	CThemePickerWnd::Open(hOwner, this, m_sThemesFilter.GetData(), m_bModal);
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
	if( _tcsicmp(pstrName, _T("modal")) == 0 ) {
		SetModal(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
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
	, m_sFamilyId(_T("blue"))
	, m_dwLastHexClick(0)
	, m_nEditMode(EDITMODE_FAMILY)
	, m_nBrightness(70)
	, m_nHueShift(0)
	, m_bEditMode(false)
	, m_bHexEditing(false)
	, m_bCommitted(false)
	, m_bFamilyUIGuard(false)
	, m_bModal(false)
	, m_pThemeList(NULL)
	, m_pTokenList(NULL)
	, m_pSeedPanel(NULL)
	, m_pFamilyPanel(NULL)
	, m_pFamilyChips(NULL)
	, m_pMiniPreview(NULL)
	, m_pPalette(NULL)
	, m_pModeSeg(NULL)
	, m_pBrightSlider(NULL)
	, m_pHueSlider(NULL)
	, m_pEditHint(NULL)
	, m_pBrightLabel(NULL)
	, m_pHueLabel(NULL)
{
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm != NULL ) m_sEnterThemeId = tm->GetCurrentThemeId();
}

CThemePickerWnd::~CThemePickerWnd()
{
	if( s_pActive == this ) s_pActive = NULL;
}

void CThemePickerWnd::Open(HWND hOwner, CThemeSwitcherUI* pSwitcher, LPCTSTR pstrThemesFilter, bool bModal)
{
	if( s_pActive != NULL && ::IsWindow(s_pActive->GetHWND()) ) {
		HWND hExist = s_pActive->GetHWND();
		if( ::IsIconic(hExist) )
			::ShowWindow(hExist, SW_RESTORE);
		::ShowWindow(hExist, SW_SHOW);
		::SetForegroundWindow(hExist);
		return;
	}
	CThemePickerWnd* pWnd = new CThemePickerWnd(pSwitcher, pstrThemesFilter);
	pWnd->m_bModal = bModal;
	s_pActive = pWnd;
	// 非模态勿设 owner：Windows 规定 owned 窗永远压在 owner 之上，主窗无法前置对照预览
	HWND hCreateOwner = bModal ? hOwner : NULL;
	pWnd->Create(hCreateOwner, _T("选择主题"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 860, 560);
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
	m_pSeedPanel = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("seedPanel")));
	m_pFamilyPanel = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("familyPanel")));
	m_pFamilyChips = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("familyChips")));
	m_pMiniPreview = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("miniPreview")));
	m_pPalette = static_cast<CColorPaletteUI*>(m_pm.FindControl(_T("palette")));
	m_pModeSeg = static_cast<CSegmentedUI*>(m_pm.FindControl(_T("modeSeg")));
	m_pBrightSlider = static_cast<CSliderUI*>(m_pm.FindControl(_T("brightSlider")));
	m_pHueSlider = static_cast<CSliderUI*>(m_pm.FindControl(_T("hueSlider")));
	m_pEditHint = static_cast<CLabelUI*>(m_pm.FindControl(_T("editHint")));
	m_pBrightLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("brightLabel")));
	m_pHueLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("hueLabel")));
	if( m_pPalette != NULL ) m_pPalette->SetVisible(false);
	// 非模态可最小化；模态禁主窗时隐藏最小化
	CControlUI* pTb = m_pm.FindControl(_T("titlebar"));
	if( pTb != NULL ) {
		CTitleBarUI* pTitle = static_cast<CTitleBarUI*>(pTb->GetInterface(DUI_CTR_TITLEBAR));
		if( pTitle != NULL )
			pTitle->SetShowMin(!m_bModal);
	}
	m_bFamilyUIGuard = true;
	if( m_pBrightSlider != NULL ) {
		m_pBrightSlider->SetCanSendMove(true);
		m_pBrightSlider->SetValue(m_nBrightness);
	}
	if( m_pHueSlider != NULL ) {
		m_pHueSlider->SetCanSendMove(true);
		m_pHueSlider->SetMinValue(-30);
		m_pHueSlider->SetMaxValue(30);
		m_pHueSlider->SetValue(m_nHueShift);
	}
	m_bFamilyUIGuard = false;

	CThemeManager* tm = CThemeManager::GetInstance();
	LPCTSTR cur = tm ? tm->GetCurrentThemeId() : _T("default");
	m_sSelectedId = cur;
	RebuildThemeList();
	SelectTheme(m_sSelectedId.GetData(), true);
	SetEditMode(false);
	UpdateModeUI();
	RebuildFamilyChips();
	UpdateBrightLabel();
	UpdateHueLabel();
}

bool CThemePickerWnd::ThemeAllowed(LPCTSTR id) const
{
	if( id == NULL || *id == _T('\0') ) return false;
	// 跟随 owner 白名单（运行中 IncludeTheme 可即时生效）
	if( m_pOwner != NULL )
		return m_pOwner->IsThemeListed(id);
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
	if( m_nEditMode == EDITMODE_SIMPLE )
		RebuildSeedPanel();
	else if( m_nEditMode == EDITMODE_ADVANCED )
		RebuildTokenList();
	else
		RebuildFamilyChips();
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

bool CThemePickerWnd::IsSeedToken(LPCTSTR token)
{
	return IsSeedTokenName(token);
}

void CThemePickerWnd::UpdateModeUI()
{
	if( m_pFamilyPanel != NULL )
		m_pFamilyPanel->SetVisible(m_nEditMode == EDITMODE_FAMILY);
	if( m_pSeedPanel != NULL )
		m_pSeedPanel->SetVisible(m_nEditMode == EDITMODE_SIMPLE);
	if( m_pTokenList != NULL )
		m_pTokenList->SetVisible(m_nEditMode == EDITMODE_ADVANCED);
	if( m_pEditHint != NULL ) {
		if( m_nEditMode == EDITMODE_FAMILY )
			m_pEditHint->SetText(_T("色系：点色系，色相微调邻近色，再拖明暗"));
		else if( m_nEditMode == EDITMODE_SIMPLE )
			m_pEditHint->SetText(_T("简易：改种子色即可生成整盘；需要时再切高级精调"));
		else
			m_pEditHint->SetText(_T("高级：双击任意色值精调；改色系/种子会覆盖派生色"));
	}
	if( m_pModeSeg != NULL ) {
		int want = 0;
		if( m_nEditMode == EDITMODE_SIMPLE ) want = 1;
		else if( m_nEditMode == EDITMODE_ADVANCED ) want = 2;
		if( m_pModeSeg->GetSelected() != want )
			m_pModeSeg->SetSelected(want, false);
	}
}

void CThemePickerWnd::SetPickerMode(int mode)
{
	if( mode < EDITMODE_FAMILY ) mode = EDITMODE_FAMILY;
	if( mode > EDITMODE_ADVANCED ) mode = EDITMODE_ADVANCED;
	if( m_nEditMode == mode ) {
		UpdateModeUI();
		return;
	}
	if( m_bHexEditing )
		CommitHexEdit(true);
	m_nEditMode = mode;
	m_sEditToken.Empty();
	if( mode != EDITMODE_SIMPLE && mode != EDITMODE_ADVANCED )
		SetEditMode(false);
	UpdateModeUI();
	if( mode == EDITMODE_FAMILY ) {
		RebuildFamilyChips();
		UpdateBrightLabel();
		UpdateHueLabel();
	}
	else if( mode == EDITMODE_SIMPLE )
		RebuildSeedPanel();
	else
		RebuildTokenList();
}

void CThemePickerWnd::UpdateBrightLabel()
{
	if( m_pBrightLabel == NULL ) return;
	LPCTSTR tip = _T("适中");
	if( m_nBrightness <= 35 ) tip = _T("偏暗");
	else if( m_nBrightness >= 75 ) tip = _T("偏亮");
	CDuiString s;
	s.Format(_T("%d %s"), m_nBrightness, tip);
	m_pBrightLabel->SetText(s.GetData());
}

void CThemePickerWnd::UpdateHueLabel()
{
	if( m_pHueLabel == NULL ) return;
	CDuiString s;
	if( m_nHueShift > 0 ) s.Format(_T("+%d°"), m_nHueShift);
	else s.Format(_T("%d°"), m_nHueShift);
	m_pHueLabel->SetText(s.GetData());
}

void CThemePickerWnd::RebuildFamilyChips()
{
	if( m_pFamilyChips == NULL ) return;
	m_pFamilyChips->RemoveAll();
	int n = 0;
	const FamilyDef* fams = GetFamilyDefs(n);
	for( int i = 0; i < n; ++i ) {
		const bool bSel = (m_sFamilyId.CompareNoCase(fams[i].id) == 0);
		CButtonUI* pBtn = new CButtonUI;
		pBtn->SetName(_T("family_item"));
		pBtn->AddCustomAttribute(_T("family-id"), fams[i].id);
		pBtn->SetText(fams[i].title);
		pBtn->SetFixedWidth(72);
		pBtn->SetFixedHeight(36);
		pBtn->SetKind(bSel ? CONTROLKIND_PRIMARY : CONTROLKIND_DEFAULT);
		pBtn->SetCursor(DUI_HAND);
		CDuiString tip;
		tip.Format(_T("%s（可再拖色相靠近邻近色）"), fams[i].title);
		pBtn->SetToolTip(tip.GetData());
		pBtn->SetBorderWidth(bSel ? 2 : 1);
		pBtn->SetBorderColor(fams[i].swatch);
		m_pFamilyChips->Add(pBtn);
	}
}

void CThemePickerWnd::ApplyFamilyTheme(bool bChanging)
{
	if( m_bFamilyUIGuard ) return;
	if( !EnsureEditableTheme() ) return;
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return;
	if( m_pBrightSlider != NULL )
		m_nBrightness = m_pBrightSlider->GetValue();
	if( m_pHueSlider != NULL )
		m_nHueShift = m_pHueSlider->GetValue();
	ApplyColorFamilyToTheme(p, m_sFamilyId.GetData(), m_nBrightness, m_nHueShift);
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm != NULL ) tm->RefreshCurrentTheme(true);
	UpdateBrightLabel();
	UpdateHueLabel();
	RebuildMiniPreview();
	if( !bChanging )
		RebuildFamilyChips();
}

void CThemePickerWnd::ExpandFromSeeds(bool bRefreshUI)
{
	if( !EnsureEditableTheme() ) return;
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return;
	GenerateThemeTokensFromSeeds(p);
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm != NULL ) tm->RefreshCurrentTheme(true);
	if( bRefreshUI ) {
		RebuildSeedPanel();
		RebuildMiniPreview();
		if( m_nEditMode == EDITMODE_ADVANCED )
			RebuildTokenList();
	}
}

void CThemePickerWnd::UpdateSeedRowColor(LPCTSTR token, DWORD dwColor)
{
	if( m_pSeedPanel == NULL || token == NULL || *token == _T('\0') ) return;
	CDuiString hex;
	FormatHex(hex, dwColor);
	for( int i = 0; i < m_pSeedPanel->GetCount(); ++i ) {
		CControlUI* pItem = m_pSeedPanel->GetItemAt(i);
		if( pItem == NULL ) continue;
		LPCTSTR tn = pItem->GetCustomAttribute(_T("token-name"));
		if( tn == NULL || _tcsicmp(tn, token) != 0 ) continue;
		IContainerUI* pRow = static_cast<IContainerUI*>(pItem->GetInterface(_T("IContainer")));
		if( pRow == NULL ) continue;
		for( int j = 0; j < pRow->GetCount(); ++j ) {
			CControlUI* pChild = pRow->GetItemAt(j);
			if( pChild == NULL ) continue;
			CDuiString nm = pChild->GetName();
			if( nm.CompareNoCase(_T("token_swatch")) == 0 ) {
				pChild->SetBackgroundColor(dwColor);
				pChild->Invalidate();
			}
			else if( nm.CompareNoCase(_T("token_hex")) == 0 ) {
				pChild->SetText(hex.GetData());
				pChild->Invalidate();
			}
			else if( nm.CompareNoCase(_T("token_hex_edit")) == 0 ) {
				pChild->SetText(hex.GetData());
			}
		}
	}
}

void CThemePickerWnd::RebuildSeedPanel()
{
	if( m_pSeedPanel == NULL ) return;
	m_pSeedPanel->RemoveAll();
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return;

	CLabelUI* pIntro = new CLabelUI;
	pIntro->SetText(_T("只需调整这几项，其余悬停/边框/语义色自动生成"));
	pIntro->SetFixedHeight(24);
	pIntro->SetAttribute(_T("text-align"), _T("left"));
	pIntro->SetColor(ThemeTok(_T("color-text-secondary"), 0x8C8C8CFF));
	pIntro->SetMouseEnabled(false);
	m_pSeedPanel->Add(pIntro);

	CHorizontalLayoutUI* pActions = new CHorizontalLayoutUI;
	pActions->SetFixedHeight(32);
	pActions->SetAttribute(_T("gap"), _T("8"));
	pActions->SetAttribute(_T("align-items"), _T("vcenter"));
	CButtonUI* pGen = new CButtonUI;
	pGen->SetName(_T("btn_gen"));
	pGen->SetText(_T("从种子重新生成"));
	pGen->SetFixedWidth(128);
	pGen->SetFixedHeight(28);
	pGen->SetKind(CONTROLKIND_PRIMARY);
	pGen->SetToolTip(_T("用当前种子色覆盖全部派生色值"));
	pActions->Add(pGen);
	CLabelUI* pTip = new CLabelUI;
	pTip->SetText(_T("改内置主题会自动另存为自定义副本"));
	pTip->SetAttribute(_T("flexible"), _T("true"));
	pTip->SetAttribute(_T("text-align"), _T("left"));
	pTip->SetColor(ThemeTok(_T("color-text-secondary"), 0x8C8C8CFF));
	pTip->SetMouseEnabled(false);
	pActions->Add(pTip);
	m_pSeedPanel->Add(pActions);

	int nSeeds = 0;
	const SeedDef* seeds = GetSeedDefs(nSeeds);
	for( int i = 0; i < nSeeds; ++i ) {
		LPCTSTR name = seeds[i].token;
		DWORD c = p->GetToken(name, 0x808080FF);
		const bool bSel = (!m_sEditToken.IsEmpty() && m_sEditToken == name);
		const DWORD bg = ThemeTok(_T("color-bg"), 0xFFFFFFFF);
		const DWORD elev = ThemeTok(_T("color-bg-elevated"), 0xF8F9FAFF);
		const DWORD hoverBk = ThemeTok(_T("color-bg-hover"), 0xE9F5FFFF);
		const DWORD selBk = ThemeTok(_T("color-selection"), 0xE7F1FFFF);
		DWORD rowBk = ((i % 2) == 1) ? elev : bg;
		if( bSel ) rowBk = selBk;

		CThemeTokenRowUI* pRow = new CThemeTokenRowUI;
		pRow->SetFixedHeight(36);
		pRow->SetAttribute(_T("align-items"), _T("vcenter"));
		pRow->SetAttribute(_T("gap"), _T("8"));
		pRow->SetPadding(CDuiBox(0, 6, 0, 6));
		pRow->AddCustomAttribute(_T("token-name"), name);
		pRow->SetMouseEnabled(true);
		pRow->SetBackgroundColor(rowBk);
		pRow->SetHoverBackgroundColor(bSel ? selBk : hoverBk);

		CControlUI* pSwatch = new CControlUI;
		pSwatch->SetName(_T("token_swatch"));
		pSwatch->AddCustomAttribute(_T("token-name"), name);
		pSwatch->SetFixedWidth(28);
		pSwatch->SetFixedHeight(28);
		pSwatch->SetBackgroundColor(c);
		pSwatch->SetBorderWidth(1);
		pSwatch->SetBorderColor(0x00000040);
		pSwatch->SetMouseEnabled(false);
		pRow->Add(pSwatch);

		CDuiString hex;
		FormatHex(hex, c);
		if( m_bHexEditing && m_sEditToken == name ) {
			CEditUI* pEdit = new CEditUI;
			pEdit->SetName(_T("token_hex_edit"));
			pEdit->AddCustomAttribute(_T("token-name"), name);
			pEdit->SetText(hex.GetData());
			pEdit->SetFixedWidth(118);
			pEdit->SetFixedHeight(26);
			pEdit->SetMaxChar(16);
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
			pHex->SetCursor(DUI_HAND);
			pHex->SetToolTip(_T("双击编辑色值"));
			pRow->Add(pHex);
		}

		CButtonUI* pTitle = new CButtonUI;
		pTitle->SetName(_T("token_row"));
		pTitle->AddCustomAttribute(_T("token-name"), name);
		pTitle->SetText(seeds[i].title);
		pTitle->SetFixedWidth(72);
		pTitle->SetFixedHeight(28);
		pTitle->SetAttribute(_T("text-align"), _T("left"));
		pTitle->SetKind(CONTROLKIND_NONE);
		pTitle->SetBackgroundColor(0);
		pTitle->SetColor(ThemeTok(_T("color-text"), 0x333333FF));
		pTitle->SetToolTip(seeds[i].tip);
		pRow->Add(pTitle);

		CButtonUI* pDesc = new CButtonUI;
		pDesc->SetName(_T("token_row"));
		pDesc->AddCustomAttribute(_T("token-name"), name);
		pDesc->SetText(seeds[i].tip);
		pDesc->SetFixedHeight(28);
		pDesc->SetAttribute(_T("flexible"), _T("true"));
		pDesc->SetAttribute(_T("text-align"), _T("left"));
		pDesc->SetKind(CONTROLKIND_NONE);
		pDesc->SetBackgroundColor(0);
		pDesc->SetColor(ThemeTok(_T("color-text-secondary"), 0x8C8C8CFF));
		pDesc->SetToolTip(seeds[i].tip);
		pRow->Add(pDesc);

		m_pSeedPanel->Add(pRow);
	}
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
			pSwatch->SetName(_T("token_swatch"));
			pSwatch->AddCustomAttribute(_T("token-name"), name);
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
				pHex->SetCursor(DUI_HAND);
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
	// 预览寿命：退编辑不得还原 enterThemeId
	RebuildSeedPanel();
	if( m_nEditMode == EDITMODE_ADVANCED )
		RebuildTokenList();
}

void CThemePickerWnd::UpdateTokenRowColor(LPCTSTR token, DWORD dwColor)
{
	if( m_pTokenList == NULL || token == NULL || *token == _T('\0') ) return;
	CDuiString hex;
	FormatHex(hex, dwColor);
	for( int i = 0; i < m_pTokenList->GetCount(); ++i ) {
		CControlUI* pItem = m_pTokenList->GetItemAt(i);
		if( pItem == NULL ) continue;
		LPCTSTR tn = pItem->GetCustomAttribute(_T("token-name"));
		if( tn == NULL || _tcsicmp(tn, token) != 0 ) continue;

		IContainerUI* pRow = static_cast<IContainerUI*>(pItem->GetInterface(_T("IContainer")));
		if( pRow == NULL ) continue;
		for( int j = 0; j < pRow->GetCount(); ++j ) {
			CControlUI* pChild = pRow->GetItemAt(j);
			if( pChild == NULL ) continue;
			CDuiString nm = pChild->GetName();
			if( nm.CompareNoCase(_T("token_swatch")) == 0 ) {
				pChild->SetBackgroundColor(dwColor);
				pChild->Invalidate();
			}
			else if( nm.CompareNoCase(_T("token_hex")) == 0 ) {
				pChild->SetText(hex.GetData());
				pChild->Invalidate();
			}
			else if( nm.CompareNoCase(_T("token_hex_edit")) == 0 ) {
				pChild->SetText(hex.GetData());
			}
		}
	}
}

void CThemePickerWnd::UpdateTokenRowSelection()
{
	if( m_pTokenList == NULL ) return;
	const DWORD hoverBk = ThemeTok(_T("color-bg-hover"), 0xE9F5FFFF);
	const DWORD selBk = ThemeTok(_T("color-selection"), 0xE7F1FFFF);
	const DWORD bg = ThemeTok(_T("color-bg"), 0xFFFFFFFF);
	const DWORD elev = ThemeTok(_T("color-bg-elevated"), 0xF8F9FAFF);
	int displayIndex = 0;
	for( int i = 0; i < m_pTokenList->GetCount(); ++i ) {
		CControlUI* pItem = m_pTokenList->GetItemAt(i);
		if( pItem == NULL ) continue;
		LPCTSTR tn = pItem->GetCustomAttribute(_T("token-name"));
		if( tn == NULL || *tn == _T('\0') ) continue; // 分组标题
		const bool bSel = (!m_sEditToken.IsEmpty() && m_sEditToken == tn);
		DWORD rowBk = ((displayIndex % 2) == 1) ? elev : bg;
		if( bSel ) rowBk = selBk;
		pItem->SetBackgroundColor(rowBk);
		pItem->SetHoverBackgroundColor(bSel ? selBk : hoverBk);
		pItem->Invalidate();
		++displayIndex;
	}
}

bool CThemePickerWnd::IsBuiltinThemeId(LPCTSTR id)
{
	if( id == NULL || *id == _T('\0') ) return false;
	static const LPCTSTR kBuiltin[] = {
		_T("default"), _T("azure"), _T("emerald"), _T("graphite"), _T("dark")
	};
	for( int i = 0; i < (int)(sizeof(kBuiltin) / sizeof(kBuiltin[0])); ++i ) {
		if( _tcsicmp(id, kBuiltin[i]) == 0 ) return true;
	}
	return false;
}

bool CThemePickerWnd::EnsureEditableTheme()
{
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return false;
	if( !IsBuiltinThemeId(p->GetId()) ) return true;

	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return false;
	CDuiString baseName = p->GetDisplayName();
	if( baseName.IsEmpty() ) baseName = p->GetId();
	CDuiString id = MakeUniqueThemeId(_T("custom"));
	CDuiString title;
	title.Format(_T("%s 副本"), baseName.GetData());
	CTheme* pNew = new CTheme(id.GetData(), title.GetData());
	pNew->CopyTokensFrom(*p);
	if( !tm->RegisterTheme(pNew, true) ) {
		delete pNew;
		return false;
	}
	m_sSelectedId = id;
	if( m_pOwner != NULL )
		m_pOwner->IncludeTheme(id.GetData());
	RebuildThemeList();
	// 预览切到副本，不碰 enterThemeId
	tm->ApplyTheme(id.GetData(), true);
	RebuildMiniPreview();
	CDuiString tip;
	tip.Format(_T("已基于「%s」创建自定义主题，可放心修改"), baseName.GetData());
	CToast::ShowInfo(tip.GetData(), 2500);
	return true;
}

void CThemePickerWnd::ApplyTokenColor(LPCTSTR token, DWORD dwColor, bool bChanging)
{
	if( token == NULL || *token == _T('\0') ) return;
	if( !EnsureEditableTheme() ) return;
	CTheme* p = GetSelectedTheme();
	if( p == NULL ) return;
	p->SetToken(token, dwColor);

	const bool bSeedSimple = (m_nEditMode == EDITMODE_SIMPLE) && IsSeedToken(token);
	if( bSeedSimple )
		GenerateThemeTokensFromSeeds(p);

	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm != NULL ) tm->RefreshCurrentTheme(true);

	if( bSeedSimple ) {
		UpdateSeedRowColor(token, dwColor);
		RebuildMiniPreview();
		if( m_pPalette != NULL )
			m_pPalette->SetSelectColor(dwColor);
		return;
	}

	UpdateTokenRowColor(token, dwColor);
	UpdateSeedRowColor(token, dwColor);
	RebuildMiniPreview();
	if( m_pPalette != NULL )
		m_pPalette->SetSelectColor(dwColor);
}

void CThemePickerWnd::OnTokenRowClick(CControlUI* pRow)
{
	if( pRow == NULL ) return;
	if( m_bHexEditing )
		CommitHexEdit(true);
	LPCTSTR name = pRow->GetCustomAttribute(_T("token-name"));
	if( name == NULL ) return;
	m_sEditToken = name;
	m_bEditMode = true;
	if( m_pPalette != NULL ) {
		m_pPalette->SetVisible(true);
		CTheme* p = GetSelectedTheme();
		if( p != NULL )
			m_pPalette->SetSelectColor(p->GetToken(name, 0x808080FF));
	}
	if( m_nEditMode == EDITMODE_ADVANCED )
		UpdateTokenRowSelection();
	else if( m_nEditMode == EDITMODE_SIMPLE )
		RebuildSeedPanel();
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
	m_bEditMode = true;
	if( m_pPalette != NULL )
		m_pPalette->SetVisible(true);
	m_sEditToken = token;
	m_bHexEditing = true;
	CTheme* p = GetSelectedTheme();
	if( p != NULL && m_pPalette != NULL )
		m_pPalette->SetSelectColor(p->GetToken(token, 0x808080FF));
	RebuildSeedPanel();
	if( m_nEditMode == EDITMODE_ADVANCED )
		RebuildTokenList();
	FocusHexEdit();
}

void CThemePickerWnd::CommitHexEdit(bool bApply)
{
	if( !m_bHexEditing ) return;
	CDuiString text;
	CControlUI* pCtrl = m_pm.FindControl(_T("token_hex_edit"));
	CEditUI* pEdit = NULL;
	if( pCtrl != NULL )
		pEdit = static_cast<CEditUI*>(pCtrl->GetInterface(DUI_CTR_EDIT));
	if( pEdit != NULL ) text = pEdit->GetText();
	CDuiString token = m_sEditToken;
	m_bHexEditing = false;

	if( pEdit != NULL && m_pm.GetFocus() == pEdit )
		m_pm.SetFocus(NULL);

	if( bApply && !token.IsEmpty() && !text.IsEmpty() ) {
		DWORD c = 0;
		CDuiString s = text;
		s.TrimLeft();
		s.TrimRight();
		if( ParseColorString(s.GetData(), c) )
			ApplyTokenColor(token.GetData(), c, false);
		else
			CToast::ShowWarning(_T("颜色格式无效，请使用 #RRGGBB 或 #RRGGBBAA"), 2500);
	}
	// 退键入不还原主题预览
	RebuildSeedPanel();
	if( m_nEditMode == EDITMODE_ADVANCED )
		RebuildTokenList();
}

void CThemePickerWnd::FocusHexEdit()
{
	CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("token_hex_edit")));
	if( pEdit == NULL ) return;
	pEdit->SetFocus();
	pEdit->SetSelAll();
}

void CThemePickerWnd::OnPaletteColor(DWORD dwColor, bool bChanging)
{
	if( !m_bEditMode || m_sEditToken.IsEmpty() ) return;
	if( m_bHexEditing ) {
		// 拖色板时结束键入控件，但不还原预览
		m_bHexEditing = false;
		CControlUI* pCtrl = m_pm.FindControl(_T("token_hex_edit"));
		if( pCtrl != NULL && m_pm.GetFocus() == pCtrl )
			m_pm.SetFocus(NULL);
		RebuildSeedPanel();
		if( m_nEditMode == EDITMODE_ADVANCED )
			RebuildTokenList();
	}
	ApplyTokenColor(m_sEditToken.GetData(), dwColor, bChanging);
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
	if( m_pOwner != NULL )
		m_pOwner->IncludeTheme(id.GetData());
	RebuildThemeList();
	SelectTheme(id.GetData(), true);
	SetEditMode(false);
	SetPickerMode(EDITMODE_FAMILY);
	CToast::ShowSuccess(_T("已新建主题，在色系模式点色系/拖明暗即可"), 2500);
	return true;
}

bool CThemePickerWnd::DoImport()
{
	CDuiString path;
	if( !PickOpenThemePath(m_hWnd, path) ) return false;
	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return false;
	CTheme* pLoaded = tm->LoadThemeFile(path.GetData(), NULL);
	if( pLoaded == NULL ) {
		CToast::ShowDanger(_T("导入主题失败"), 2500);
		return false;
	}
	if( m_pOwner != NULL )
		m_pOwner->IncludeTheme(pLoaded->GetId());
	m_sSelectedId = pLoaded->GetId();
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
	if( m_pOwner != NULL ) {
		m_pOwner->IncludeTheme(p->GetId());
		if( m_pOwner->GetManager() != NULL ) {
			// lParam = 文件路径（回调内请立刻拷贝）
			m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_THEMEFILESAVED,
				(WPARAM)p->GetId(), (LPARAM)path.GetData());
		}
	}
	CToast::ShowSuccess(_T("主题已保存"), 2000);
	return true;
}

void CThemePickerWnd::CommitAndClose(bool bOk)
{
	if( m_bHexEditing )
		CommitHexEdit(bOk);
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
		// 唯一点：取消/关窗才结束预览，还原进入前主题
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
	if( msg.sType == DUI_MSGTYPE_SELECTCHANGED ) {
		if( msg.pSender == m_pModeSeg ) {
			LPCTSTR v = m_pModeSeg->GetSelectedValue();
			int mode = EDITMODE_FAMILY;
			if( v != NULL && _tcsicmp(v, _T("simple")) == 0 ) mode = EDITMODE_SIMPLE;
			else if( v != NULL && _tcsicmp(v, _T("advanced")) == 0 ) mode = EDITMODE_ADVANCED;
			SetPickerMode(mode);
			return;
		}
	}
	if( msg.sType == DUI_MSGTYPE_VALUECHANGED || msg.sType == DUI_MSGTYPE_VALUECHANGED_MOVE ) {
		if( m_nEditMode == EDITMODE_FAMILY
			&& (msg.pSender == m_pBrightSlider || msg.pSender == m_pHueSlider) ) {
			ApplyFamilyTheme(msg.sType == DUI_MSGTYPE_VALUECHANGED_MOVE);
			return;
		}
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
	const bool bClosing = (sName.CompareNoCase(_T("btn_ok")) == 0
		|| sName.CompareNoCase(_T("btn_cancel")) == 0
		|| sName.CompareNoCase(_T("closebtn")) == 0);
	if( m_bHexEditing && !bClosing
		&& sName.CompareNoCase(_T("token_hex_edit")) != 0
		&& sName.CompareNoCase(_T("token_hex")) != 0 ) {
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
	if( sName.CompareNoCase(_T("btn_import")) == 0 ) {
		DoImport();
		return;
	}
	if( sName.CompareNoCase(_T("btn_save")) == 0 ) {
		DoSaveAs();
		return;
	}
	if( sName.CompareNoCase(_T("btn_gen")) == 0 ) {
		ExpandFromSeeds(true);
		CToast::ShowSuccess(_T("已从种子色生成主题"), 1800);
		return;
	}
	if( sName.CompareNoCase(_T("family_item")) == 0 ) {
		LPCTSTR fid = msg.pSender->GetCustomAttribute(_T("family-id"));
		if( fid != NULL && *fid != _T('\0') ) {
			const bool bSame = (m_sFamilyId.CompareNoCase(fid) == 0);
			m_sFamilyId = fid;
			if( !bSame ) {
				m_nHueShift = 0;
				m_bFamilyUIGuard = true;
				if( m_pHueSlider != NULL )
					m_pHueSlider->SetValue(0);
				m_bFamilyUIGuard = false;
				UpdateHueLabel();
			}
			ApplyFamilyTheme(false);
		}
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

} // namespace DuiLib
