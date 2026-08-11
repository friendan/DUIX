#ifndef __UITHEME_H__
#define __UITHEME_H__
#pragma once

#include <map>
#include <vector>

namespace DuiLib {

	class CPaintManagerUI;
	class CControlUI;
	class CMenuUI;

	/// 颜色主题对象：持有语义 token（RRGGBBAA），可注册到 CThemeManager
	class UILIB_API CTheme
	{
	public:
		CTheme();
		CTheme(LPCTSTR id, LPCTSTR displayName);
		virtual ~CTheme();

		LPCTSTR GetId() const;
		void SetId(LPCTSTR id);
		LPCTSTR GetDisplayName() const;
		void SetDisplayName(LPCTSTR name);

		void SetToken(LPCTSTR name, DWORD color);
		void SetToken(LPCTSTR name, LPCTSTR colorStr);
		bool TryGetToken(LPCTSTR name, DWORD& color) const;
		DWORD GetToken(LPCTSTR name, DWORD fallback = 0) const;
		int GetTokenCount() const;
		LPCTSTR GetTokenNameAt(int index) const;
		DWORD GetTokenValueAt(int index) const;

		void ClearTokens();
		void CopyTokensFrom(const CTheme& other);

		/// 写入全局 g_kindColors
		void ApplyKindColors() const;
		/// 注入本 PaintManager 的 Default（共享表）
		void ApplyToManager(CPaintManagerUI* pManager) const;

	protected:
		DWORD TokenOr(LPCTSTR name, DWORD fallback) const;

	private:
		CDuiString m_sId;
		CDuiString m_sDisplayName;
		std::map<CDuiString, DWORD> m_tokens;
		mutable std::vector<CDuiString> m_tokenOrder; // 枚举顺序
	};

	/// 主题变更监听：便于外部持久化当前主题 id（预览阶段可忽略）
	class UILIB_API IThemeNotifyUI
	{
	public:
		virtual ~IThemeNotifyUI() {}
		/// bPreview=true：预览窗临时切换；false：正式提交
		virtual void OnThemeChanged(LPCTSTR oldId, LPCTSTR newId, bool bPreview) = 0;
	};

	/// 进程级主题管理：注册、枚举、切换、热刷新
	class UILIB_API CThemeManager
	{
	public:
		static CThemeManager* GetInstance();

		void EnsureInitialized();
		void RegisterBuiltinThemes();

		bool RegisterTheme(CTheme* pTheme, bool bOwn = true);
		bool UnregisterTheme(LPCTSTR id);
		CTheme* FindTheme(LPCTSTR id) const;
		int GetThemeCount() const;
		CTheme* GetThemeAt(int index) const;

		void SetDefaultThemeId(LPCTSTR id);
		LPCTSTR GetDefaultThemeId() const;
		CTheme* GetCurrentTheme() const;
		LPCTSTR GetCurrentThemeId() const;

		/// 总开关：禁用后不套主题，kind 回退 Bootstrap 内置表；仍可枚举已注册主题
		void SetEnabled(bool bEnabled);
		bool IsEnabled() const;

		bool ApplyTheme(LPCTSTR id, bool bPreview = false);
		bool ApplyTheme(CTheme* pTheme, bool bPreview = false);
		/// 当前主题 token 已改：重刷 UI（不改 id）
		void RefreshCurrentTheme(bool bPreview = false);
		/// 加载 :root 主题文件并注册，不切换当前主题；失败返回 NULL
		CTheme* LoadThemeFile(LPCTSTR path, LPCTSTR idOverride = NULL, LPCTSTR displayName = NULL);
		/// 简易 :root { --token: #RRGGBBAA; } 文件；成功则注册为 id（文件名）并 Apply
		bool ApplyThemeFile(LPCTSTR path, LPCTSTR idOverride = NULL);
		/// 写出与 ApplyThemeFile 对称的 :root 文件
		bool SaveThemeFile(const CTheme* pTheme, LPCTSTR path) const;

		void AddThemeNotify(IThemeNotifyUI* pNotify);
		void RemoveThemeNotify(IThemeNotifyUI* pNotify);

		DWORD GetColor(LPCTSTR token, DWORD fallback = 0) const;

		/// 解析控件有效 theme 角色与色板（向上继承；panel 不阻断 chrome）
		/// modeOut: none/chrome/secondary/空；ppTheme 可选返回色板（局部 theme-id / 主题 id）
		void ResolveEffectiveTheme(CControlUI* pControl, CDuiString& modeOut, CTheme** ppTheme = NULL) const;
		/// 控件自身是否 theme=panel（仅本节点铺 elevated）
		static bool IsSelfPanel(CControlUI* pControl);

		/// ParseColorString(var(--token)) 解析时临时色板（嵌套计数）；NULL=用当前主题
		static void PushColorParseTheme(CTheme* pTheme);
		static void PopColorParseTheme();
		static CTheme* GetColorParseTheme();

		/// 对所有已创建的 PaintManager 重套 kind / chrome 并刷新
		void RefreshAllManagers();
		/// 仅写 Default / 窗口色（可在 InitControls 之前调用）
		void ApplyManagerDefaults(CPaintManagerUI* pManager);
		/// 窗口 root 就绪后套 Default + chrome（勿在控件 ctor 路径调用）
		void ApplyToExistingManager(CPaintManagerUI* pManager);

		/// 纯色弹出 Menu：套 list chrome + 分隔线；图片壳 / theme=none 跳过
		void ApplyMenuChrome(CMenuUI* pMenu);

		/// 热切后同步所有 ThemeSwitcher 显示
		void SyncThemeSwitchers();

	private:
		CThemeManager();
		~CThemeManager();
		CThemeManager(const CThemeManager&);
		CThemeManager& operator=(const CThemeManager&);

		void ApplyCurrentToGlobals();
		void NotifyThemeChanged(LPCTSTR oldId, LPCTSTR newId, bool bPreview);
		CTheme* GetChromeTheme() const;
		void ApplyChromeToManager(CPaintManagerUI* pManager);
		static void ReapplyKindRecursive(CControlUI* pControl);
		static void ApplyChromeRecursive(CControlUI* pControl, const CTheme* pTheme);
		static void RefreshVarAttributesRecursive(CControlUI* pControl);
		static void ResolveThemeChrome(CControlUI* pControl, CDuiString& modeOut, bool& bSelfPanel,
			CTheme*& pThemeOut, CTheme* pFallback);
		static void SyncThemeSwitchersRecursive(CControlUI* pControl);

		struct ThemeEntry {
			CTheme* pTheme;
			bool bOwn;
		};

		std::vector<ThemeEntry> m_themes;
		std::vector<IThemeNotifyUI*> m_notifies;
		CDuiString m_sDefaultId;
		CDuiString m_sCurrentId;
		bool m_bInited;
		bool m_bEnabled;
	};

	/// 供 InitKindColors / 外部标记「kind 表已由主题填充」
	UILIB_API void MarkKindColorsInitialized();

} // namespace DuiLib

#endif // __UITHEME_H__
