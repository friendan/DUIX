#include "StdAfx.h"
#include "UITheme.h"

namespace DuiLib {

	static void FillBootstrapDefault(CTheme* t)
	{
		if (t == NULL) return;
		// 与历史 InitKindColors / Bootstrap 5.3.8 对齐
		t->SetToken(_T("color-default"), 0xEEEEEEFF);
		t->SetToken(_T("color-default-border"), 0xDEE2E6FF);
		t->SetToken(_T("color-default-text"), 0x212529FF);
		t->SetToken(_T("color-default-hover"), 0xD3D4D5FF);
		t->SetToken(_T("color-default-border-hover"), 0xC6C7C8FF);
		t->SetToken(_T("color-default-active"), 0xC6C7C8FF);
		t->SetToken(_T("color-default-border-active"), 0xBABBBCFF);

		t->SetToken(_T("color-primary"), 0x0D6EFDFF);
		t->SetToken(_T("color-primary-hover"), 0x0B5ED7FF);
		t->SetToken(_T("color-primary-active"), 0x0A58CAFF);
		t->SetToken(_T("color-primary-border"), 0x0D6EFDFF);
		t->SetToken(_T("color-primary-border-hover"), 0x0A58CAFF);
		t->SetToken(_T("color-primary-border-active"), 0x0A53BEFF);
		t->SetToken(_T("color-primary-text"), 0xFFFFFFFF);

		t->SetToken(_T("color-secondary"), 0x6C757DFF);
		t->SetToken(_T("color-secondary-hover"), 0x5C636AFF);
		t->SetToken(_T("color-secondary-active"), 0x565E64FF);
		t->SetToken(_T("color-secondary-border-hover"), 0x565E64FF);
		t->SetToken(_T("color-secondary-border-active"), 0x51585EFF);
		t->SetToken(_T("color-secondary-text"), 0xFFFFFFFF);

		t->SetToken(_T("color-success"), 0x198754FF);
		t->SetToken(_T("color-success-hover"), 0x157347FF);
		t->SetToken(_T("color-success-active"), 0x146C43FF);
		t->SetToken(_T("color-success-border-hover"), 0x146C43FF);
		t->SetToken(_T("color-success-border-active"), 0x13653FFF);
		t->SetToken(_T("color-success-text"), 0xFFFFFFFF);

		t->SetToken(_T("color-danger"), 0xDC3545FF);
		t->SetToken(_T("color-danger-hover"), 0xBB2D3BFF);
		t->SetToken(_T("color-danger-active"), 0xB02A37FF);
		t->SetToken(_T("color-danger-border-hover"), 0xB02A37FF);
		t->SetToken(_T("color-danger-border-active"), 0xA52834FF);
		t->SetToken(_T("color-danger-text"), 0xFFFFFFFF);

		t->SetToken(_T("color-warning"), 0xFFC107FF);
		t->SetToken(_T("color-warning-hover"), 0xE0A800FF);
		t->SetToken(_T("color-warning-active"), 0xD39E00FF);
		t->SetToken(_T("color-warning-border-hover"), 0xD39E00FF);
		t->SetToken(_T("color-warning-border-active"), 0xC69500FF);
		t->SetToken(_T("color-warning-text"), 0x000000FF);

		t->SetToken(_T("color-info"), 0x0DCAF0FF);
		t->SetToken(_T("color-info-hover"), 0x0BA5C7FF);
		t->SetToken(_T("color-info-active"), 0x0A98B8FF);
		t->SetToken(_T("color-info-border-hover"), 0x0A98B8FF);
		t->SetToken(_T("color-info-border-active"), 0x098BA8FF);
		t->SetToken(_T("color-info-text"), 0x000000FF);

		t->SetToken(_T("color-light"), 0xF8F9FAFF);
		t->SetToken(_T("color-light-hover"), 0xD3D4D5FF);
		t->SetToken(_T("color-light-active"), 0xC6C7C8FF);
		t->SetToken(_T("color-light-border-hover"), 0xC6C7C8FF);
		t->SetToken(_T("color-light-border-active"), 0xBABBBCFF);
		t->SetToken(_T("color-light-text"), 0x000000FF);

		t->SetToken(_T("color-dark"), 0x212529FF);
		t->SetToken(_T("color-dark-hover"), 0x424649FF);
		t->SetToken(_T("color-dark-active"), 0x4D5154FF);
		t->SetToken(_T("color-dark-border-hover"), 0x373B3EFF);
		t->SetToken(_T("color-dark-border-active"), 0x373B3EFF);
		t->SetToken(_T("color-dark-text"), 0xFFFFFFFF);

		t->SetToken(_T("color-link"), 0x0D6EFDFF);
		t->SetToken(_T("color-link-hover"), 0x0A58CAFF);
		t->SetToken(_T("color-link-active"), 0x0A58CAFF);

		t->SetToken(_T("color-text"), 0x000000E0);
		t->SetToken(_T("color-text-secondary"), 0x000000A6);
		t->SetToken(_T("color-text-disabled"), 0xADB5BDFF);
		t->SetToken(_T("color-border"), 0xDEE2E6FF);
		t->SetToken(_T("color-border-strong"), 0xADB5BDFF);
		t->SetToken(_T("color-bg"), 0xFFFFFFFF);
		t->SetToken(_T("color-bg-elevated"), 0xF8F9FAFF);
		t->SetToken(_T("color-bg-hover"), 0xF0F0F0FF);
		t->SetToken(_T("color-disabled-bg"), 0xE9ECEFFF);
		t->SetToken(_T("color-disabled-text"), 0xADB5BDFF);
		t->SetToken(_T("color-control-bg"), 0xFFFFFFFF);
		t->SetToken(_T("color-control-border"), 0xDEE2E6FF);
		t->SetToken(_T("color-control-border-focus"), 0x0D6EFDFF);
		t->SetToken(_T("color-selection"), 0xE7F1FFFF);
		t->SetToken(_T("color-scrollbar-rail"), 0xEDEDF0FF);
		t->SetToken(_T("color-scrollbar-thumb"), 0xC0C0C6FF);
		t->SetToken(_T("color-scrollbar-thumb-hover"), 0xA6A6AEFF);
		t->SetToken(_T("color-titlebar-bg"), 0x333333FF);
		t->SetToken(_T("color-titlebar-text"), 0xFFFFFFFF);
		t->SetToken(_T("color-titlebar-border"), 0x222222FF);
		t->SetToken(_T("color-modal-bg"), 0xFFFFFFFF);
		t->SetToken(_T("color-modal-text"), 0x3C3C3CFF);
		t->SetToken(_T("color-modal-border"), 0xE6E6E6FF);
		t->SetToken(_T("color-rate"), 0xFADB14FF);
		t->SetToken(_T("color-titlebar-close-hover"), 0xE81123FF);
	}

	void CThemeManager::RegisterBuiltinThemes()
	{
		if (FindTheme(_T("default")) != NULL) return;

		CTheme* pDefault = new CTheme(_T("default"), _T("Default"));
		FillBootstrapDefault(pDefault);
		RegisterTheme(pDefault, true);

		CTheme* pAzure = new CTheme(_T("azure"), _T("晴空蓝"));
		pAzure->CopyTokensFrom(*pDefault);
		pAzure->SetToken(_T("color-primary"), 0x1677FFFF);
		pAzure->SetToken(_T("color-primary-hover"), 0x4096FFFF);
		pAzure->SetToken(_T("color-primary-active"), 0x0958D9FF);
		pAzure->SetToken(_T("color-primary-border"), 0x1677FFFF);
		pAzure->SetToken(_T("color-primary-border-hover"), 0x4096FFFF);
		pAzure->SetToken(_T("color-primary-border-active"), 0x0958D9FF);
		pAzure->SetToken(_T("color-link"), 0x1677FFFF);
		pAzure->SetToken(_T("color-link-hover"), 0x4096FFFF);
		pAzure->SetToken(_T("color-link-active"), 0x0958D9FF);
		pAzure->SetToken(_T("color-info"), 0x1677FFFF);
		pAzure->SetToken(_T("color-info-hover"), 0x4096FFFF);
		pAzure->SetToken(_T("color-info-active"), 0x0958D9FF);
		pAzure->SetToken(_T("color-info-text"), 0xFFFFFFFF);
		pAzure->SetToken(_T("color-info-border-hover"), 0x4096FFFF);
		pAzure->SetToken(_T("color-info-border-active"), 0x0958D9FF);
		pAzure->SetToken(_T("color-border"), 0xD6E4FFFF);
		pAzure->SetToken(_T("color-border-strong"), 0x91CAFFFF);
		pAzure->SetToken(_T("color-control-border"), 0xD9D9D9FF);
		pAzure->SetToken(_T("color-control-border-focus"), 0x1677FFFF);
		pAzure->SetToken(_T("color-selection"), 0xBAE0FFFF);
		pAzure->SetToken(_T("color-bg-hover"), 0xE6F4FFFF);
		pAzure->SetToken(_T("color-bg-elevated"), 0xF0F5FFFF);
		pAzure->SetToken(_T("color-scrollbar-rail"), 0xE6F4FFFF);
		pAzure->SetToken(_T("color-scrollbar-thumb"), 0x91CAFFFF);
		pAzure->SetToken(_T("color-scrollbar-thumb-hover"), 0x69B1FFFF);
		pAzure->SetToken(_T("color-titlebar-bg"), 0x1677FFFF);
		pAzure->SetToken(_T("color-titlebar-text"), 0xFFFFFFFF);
		pAzure->SetToken(_T("color-titlebar-border"), 0x0958D9FF);
		RegisterTheme(pAzure, true);

		CTheme* pEmerald = new CTheme(_T("emerald"), _T("翠绿"));
		pEmerald->CopyTokensFrom(*pDefault);
		pEmerald->SetToken(_T("color-primary"), 0x10B981FF);
		pEmerald->SetToken(_T("color-primary-hover"), 0x34D399FF);
		pEmerald->SetToken(_T("color-primary-active"), 0x059669FF);
		pEmerald->SetToken(_T("color-primary-border"), 0x10B981FF);
		pEmerald->SetToken(_T("color-primary-border-hover"), 0x34D399FF);
		pEmerald->SetToken(_T("color-primary-border-active"), 0x059669FF);
		pEmerald->SetToken(_T("color-link"), 0x059669FF);
		pEmerald->SetToken(_T("color-link-hover"), 0x10B981FF);
		pEmerald->SetToken(_T("color-link-active"), 0x047857FF);
		pEmerald->SetToken(_T("color-success"), 0x10B981FF);
		pEmerald->SetToken(_T("color-success-hover"), 0x34D399FF);
		pEmerald->SetToken(_T("color-success-active"), 0x059669FF);
		pEmerald->SetToken(_T("color-border"), 0xD1FAE5FF);
		pEmerald->SetToken(_T("color-border-strong"), 0x6EE7B7FF);
		pEmerald->SetToken(_T("color-control-border"), 0xA7F3D0FF);
		pEmerald->SetToken(_T("color-control-border-focus"), 0x10B981FF);
		pEmerald->SetToken(_T("color-selection"), 0xA7F3D0FF);
		pEmerald->SetToken(_T("color-bg-hover"), 0xD1FAE5FF);
		pEmerald->SetToken(_T("color-bg-elevated"), 0xECFDF5FF);
		pEmerald->SetToken(_T("color-scrollbar-rail"), 0xD1FAE5FF);
		pEmerald->SetToken(_T("color-scrollbar-thumb"), 0x6EE7B7FF);
		pEmerald->SetToken(_T("color-scrollbar-thumb-hover"), 0x34D399FF);
		pEmerald->SetToken(_T("color-titlebar-bg"), 0x059669FF);
		pEmerald->SetToken(_T("color-titlebar-text"), 0xFFFFFFFF);
		pEmerald->SetToken(_T("color-titlebar-border"), 0x047857FF);
		RegisterTheme(pEmerald, true);

		CTheme* pGraphite = new CTheme(_T("graphite"), _T("石墨"));
		pGraphite->CopyTokensFrom(*pDefault);
		// 冷灰 slate 一体：正文微灰、主色加深、标题栏跟 primary 同族（避免浅正文+近黑标题割裂）
		pGraphite->SetToken(_T("color-bg"), 0xF8FAFCFF);
		pGraphite->SetToken(_T("color-bg-elevated"), 0xF1F5F9FF);
		pGraphite->SetToken(_T("color-bg-hover"), 0xE2E8F0FF);
		pGraphite->SetToken(_T("color-control-bg"), 0xFFFFFFFF);
		pGraphite->SetToken(_T("color-modal-bg"), 0xF8FAFCFF);
		pGraphite->SetToken(_T("color-text"), 0x0F172AFF);
		pGraphite->SetToken(_T("color-text-secondary"), 0x64748BFF);
		pGraphite->SetToken(_T("color-text-disabled"), 0x94A3B8FF);
		pGraphite->SetToken(_T("color-disabled-text"), 0x94A3B8FF);
		pGraphite->SetToken(_T("color-disabled-bg"), 0xE2E8F0FF);
		pGraphite->SetToken(_T("color-primary"), 0x334155FF);
		pGraphite->SetToken(_T("color-primary-hover"), 0x475569FF);
		pGraphite->SetToken(_T("color-primary-active"), 0x1E293BFF);
		pGraphite->SetToken(_T("color-primary-border"), 0x334155FF);
		pGraphite->SetToken(_T("color-primary-border-hover"), 0x475569FF);
		pGraphite->SetToken(_T("color-primary-border-active"), 0x1E293BFF);
		pGraphite->SetToken(_T("color-primary-text"), 0xFFFFFFFF);
		pGraphite->SetToken(_T("color-secondary"), 0x64748BFF);
		pGraphite->SetToken(_T("color-secondary-hover"), 0x475569FF);
		pGraphite->SetToken(_T("color-secondary-active"), 0x334155FF);
		pGraphite->SetToken(_T("color-secondary-border-hover"), 0x475569FF);
		pGraphite->SetToken(_T("color-secondary-border-active"), 0x334155FF);
		pGraphite->SetToken(_T("color-link"), 0x334155FF);
		pGraphite->SetToken(_T("color-link-hover"), 0x1E293BFF);
		pGraphite->SetToken(_T("color-link-active"), 0x0F172AFF);
		pGraphite->SetToken(_T("color-dark"), 0x1E293BFF);
		pGraphite->SetToken(_T("color-dark-hover"), 0x334155FF);
		pGraphite->SetToken(_T("color-dark-active"), 0x0F172AFF);
		pGraphite->SetToken(_T("color-border"), 0xE2E8F0FF);
		pGraphite->SetToken(_T("color-border-strong"), 0x94A3B8FF);
		pGraphite->SetToken(_T("color-control-border"), 0xCBD5E1FF);
		pGraphite->SetToken(_T("color-control-border-focus"), 0x334155FF);
		pGraphite->SetToken(_T("color-selection"), 0xCBD5E1FF);
		pGraphite->SetToken(_T("color-scrollbar-rail"), 0xE2E8F0FF);
		pGraphite->SetToken(_T("color-scrollbar-thumb"), 0x94A3B8FF);
		pGraphite->SetToken(_T("color-scrollbar-thumb-hover"), 0x64748BFF);
		pGraphite->SetToken(_T("color-titlebar-bg"), 0x334155FF);
		pGraphite->SetToken(_T("color-titlebar-text"), 0xFFFFFFFF);
		pGraphite->SetToken(_T("color-titlebar-border"), 0x1E293BFF);
		RegisterTheme(pGraphite, true);

		CTheme* pDark = new CTheme(_T("dark"), _T("深色"));
		pDark->CopyTokensFrom(*pDefault);
		pDark->SetToken(_T("color-bg"), 0x141414FF);
		pDark->SetToken(_T("color-bg-elevated"), 0x1F1F1FFF);
		pDark->SetToken(_T("color-text"), 0xFFFFFFE0);
		pDark->SetToken(_T("color-text-secondary"), 0xFFFFFFA6);
		pDark->SetToken(_T("color-text-disabled"), 0xFFFFFF73);
		pDark->SetToken(_T("color-disabled-text"), 0xFFFFFF73);
		pDark->SetToken(_T("color-disabled-bg"), 0x2A2A2AFF);
		pDark->SetToken(_T("color-border"), 0x303030FF);
		pDark->SetToken(_T("color-border-strong"), 0x505050FF);
		pDark->SetToken(_T("color-control-bg"), 0x1F1F1FFF);
		pDark->SetToken(_T("color-control-border"), 0x424242FF);
		pDark->SetToken(_T("color-control-border-focus"), 0x4096FFFF);
		pDark->SetToken(_T("color-selection"), 0x111A2CFF);
		pDark->SetToken(_T("color-bg-hover"), 0x2A2A2AFF);
		pDark->SetToken(_T("color-scrollbar-rail"), 0x1F1F1FFF);
		pDark->SetToken(_T("color-scrollbar-thumb"), 0x505050FF);
		pDark->SetToken(_T("color-scrollbar-thumb-hover"), 0x69B1FFFF);
		pDark->SetToken(_T("color-titlebar-bg"), 0x0A0A0AFF);
		pDark->SetToken(_T("color-titlebar-text"), 0xFFFFFFFF);
		pDark->SetToken(_T("color-titlebar-border"), 0x000000FF);
		pDark->SetToken(_T("color-primary"), 0x4096FFFF);
		pDark->SetToken(_T("color-primary-hover"), 0x69B1FFFF);
		pDark->SetToken(_T("color-primary-active"), 0x1677FFFF);
		pDark->SetToken(_T("color-primary-border"), 0x4096FFFF);
		pDark->SetToken(_T("color-primary-border-hover"), 0x69B1FFFF);
		pDark->SetToken(_T("color-primary-border-active"), 0x1677FFFF);
		pDark->SetToken(_T("color-link"), 0x69B1FFFF);
		pDark->SetToken(_T("color-link-hover"), 0x91CAFFFF);
		pDark->SetToken(_T("color-link-active"), 0x4096FFFF);
		pDark->SetToken(_T("color-default"), 0x303030FF);
		pDark->SetToken(_T("color-default-border"), 0x424242FF);
		pDark->SetToken(_T("color-default-text"), 0xFFFFFFE0);
		pDark->SetToken(_T("color-default-hover"), 0x3A3A3AFF);
		pDark->SetToken(_T("color-default-border-hover"), 0x505050FF);
		pDark->SetToken(_T("color-default-active"), 0x424242FF);
		pDark->SetToken(_T("color-default-border-active"), 0x595959FF);
		pDark->SetToken(_T("color-secondary"), 0x6B7280FF);
		pDark->SetToken(_T("color-light"), 0x303030FF);
		pDark->SetToken(_T("color-light-text"), 0xFFFFFFE0);
		pDark->SetToken(_T("color-light-hover"), 0x3A3A3AFF);
		pDark->SetToken(_T("color-light-active"), 0x424242FF);
		pDark->SetToken(_T("color-dark"), 0x0A0A0AFF);
		pDark->SetToken(_T("color-dark-hover"), 0x1F1F1FFF);
		pDark->SetToken(_T("color-dark-active"), 0x303030FF);
		pDark->SetToken(_T("color-dark-text"), 0xFFFFFFFF);
		pDark->SetToken(_T("color-success"), 0x49AA19FF);
		pDark->SetToken(_T("color-success-hover"), 0x3C8618FF);
		pDark->SetToken(_T("color-success-active"), 0x306317FF);
		pDark->SetToken(_T("color-success-border-hover"), 0x3C8618FF);
		pDark->SetToken(_T("color-success-border-active"), 0x306317FF);
		pDark->SetToken(_T("color-success-text"), 0xFFFFFFFF);
		pDark->SetToken(_T("color-danger"), 0xD32029FF);
		pDark->SetToken(_T("color-danger-hover"), 0xA61D24FF);
		pDark->SetToken(_T("color-danger-active"), 0x791A1FFF);
		pDark->SetToken(_T("color-danger-border-hover"), 0xA61D24FF);
		pDark->SetToken(_T("color-danger-border-active"), 0x791A1FFF);
		pDark->SetToken(_T("color-danger-text"), 0xFFFFFFFF);
		pDark->SetToken(_T("color-warning"), 0xD89614FF);
		pDark->SetToken(_T("color-warning-hover"), 0xAA7714FF);
		pDark->SetToken(_T("color-warning-active"), 0x7C5914FF);
		pDark->SetToken(_T("color-warning-border-hover"), 0xAA7714FF);
		pDark->SetToken(_T("color-warning-border-active"), 0x7C5914FF);
		pDark->SetToken(_T("color-warning-text"), 0x141414FF);
		pDark->SetToken(_T("color-info"), 0x1668DCFF);
		pDark->SetToken(_T("color-info-hover"), 0x1554ADFF);
		pDark->SetToken(_T("color-info-active"), 0x153D7AFF);
		pDark->SetToken(_T("color-info-border-hover"), 0x1554ADFF);
		pDark->SetToken(_T("color-info-border-active"), 0x153D7AFF);
		pDark->SetToken(_T("color-info-text"), 0xFFFFFFFF);
		pDark->SetToken(_T("color-secondary-hover"), 0x4B5563FF);
		pDark->SetToken(_T("color-secondary-active"), 0x374151FF);
		pDark->SetToken(_T("color-secondary-text"), 0xFFFFFFFF);
		pDark->SetToken(_T("color-secondary-border-hover"), 0x4B5563FF);
		pDark->SetToken(_T("color-secondary-border-active"), 0x374151FF);
		pDark->SetToken(_T("color-rate"), 0xD89614FF);
		pDark->SetToken(_T("color-skeleton"), 0xFFFFFF14);
		pDark->SetToken(_T("color-skeleton-highlight"), 0xFFFFFF40);
		pDark->SetToken(_T("color-modal-bg"), 0x1F1F1FFF);
		pDark->SetToken(_T("color-modal-text"), 0xFFFFFFE0);
		pDark->SetToken(_T("color-modal-border"), 0x303030FF);
		RegisterTheme(pDark, true);
	}

} // namespace DuiLib
