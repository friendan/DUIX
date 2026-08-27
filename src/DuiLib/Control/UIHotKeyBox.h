#ifndef __UIHOTKEYBOX_H__
#define __UIHOTKEYBOX_H__

#pragma once

namespace DuiLib {

	enum {
		HOTKEYBOX_OK = 1,
		HOTKEYBOX_CANCEL = 0,
	};

	/// 快捷键作用域（HotKeyBox Segmented；仅选择结果，不负责注册）
	enum HotKeyBoxScope {
		HOTKEYBOX_SCOPE_APP = 0,     ///< 程序快捷键（默认）
		HOTKEYBOX_SCOPE_GLOBAL = 1,  ///< 全局快捷键
	};

	/// 自定义冲突检测：返回 true 表示冲突（禁止设置）；pReason 可写说明
	typedef bool (CALLBACK *LPHotKeyConflictCheck)(WORD vk, WORD mod, LPVOID pUser, CDuiString* pReason);

	/// 同步快捷键对话框配置（链式）。
	class UILIB_API CHotKeyBoxOptions
	{
		friend class CHotKeyBox;
		friend class CHotKeyBoxWnd;
	public:
		CHotKeyBoxOptions();

		CHotKeyBoxOptions& Title(LPCTSTR text);
		/// 说明文字（可空）
		CHotKeyBoxOptions& Prompt(LPCTSTR text);
		/// 初始快捷键（0,0 = 空）
		CHotKeyBoxOptions& HotKey(WORD wVirtualKeyCode, WORD wModifiers);
		/// 作用域：程序 / 全局；默认 HOTKEYBOX_SCOPE_APP
		CHotKeyBoxOptions& Scope(int scope);
		/// 是否显示「程序/全局」Segmented；默认 true
		CHotKeyBoxOptions& ShowScope(bool show);
		/// 允许确定时为空（表示清除）；默认 true
		CHotKeyBoxOptions& AllowEmpty(bool allow);
		/// 字母/数字裸键需含 Ctrl/Alt/Shift/Win；F1~F12、Home 等不受限；默认 true
		CHotKeyBoxOptions& RequireModifier(bool require);
		/// 确定时检测冲突；冲突则提示后关闭（CANCEL，不写出）；默认 true
		CHotKeyBoxOptions& CheckConflict(bool enable);
		/// 冲突扫描用的 PaintManager（通常为主窗）；空则按 Owner HWND 查找
		CHotKeyBoxOptions& ConflictManager(CPaintManagerUI* pm);
		/// 排除控件（正在编辑自身，不与自己撞车）
		CHotKeyBoxOptions& ExcludeControl(CControlUI* pControl);
		/// 额外保留项（最多 32）；与树扫描一并检测
		CHotKeyBoxOptions& AddReserved(WORD vk, WORD mod, LPCTSTR name = NULL);
		/// 自定义冲突回调（在树扫描之前）；返回 true=冲突
		CHotKeyBoxOptions& ConflictCheck(LPHotKeyConflictCheck fn, LPVOID pUser = NULL);
		CHotKeyBoxOptions& OkText(LPCTSTR text);
		CHotKeyBoxOptions& CancelText(LPCTSTR text);
		CHotKeyBoxOptions& Width(int w);
		CHotKeyBoxOptions& Height(int h);
		CHotKeyBoxOptions& Owner(HWND hOwner);

	private:
		enum { kMaxReserved = 32 };

		CDuiString m_sTitle;
		CDuiString m_sPrompt;
		WORD m_wVk;
		WORD m_wModifiers;
		int m_nScope;
		bool m_bShowScope;
		bool m_bAllowEmpty;
		bool m_bRequireModifier;
		bool m_bCheckConflict;
		CPaintManagerUI* m_pConflictManager;
		CControlUI* m_pExcludeControl;
		LPHotKeyConflictCheck m_pfnConflictCheck;
		LPVOID m_pConflictUser;
		int m_nReserved;
		WORD m_aReservedVk[kMaxReserved];
		WORD m_aReservedMod[kMaxReserved];
		CDuiString m_aReservedName[kMaxReserved];
		CDuiString m_sOkText;
		CDuiString m_sCancelText;
		int m_nWidth;
		int m_nHeight;
		HWND m_hOwner;
	};

	/// 同步快捷键对话框：内嵌 HotKey + 程序/全局 Segmented；确定时写出 vk/修饰键/作用域与显示名。
	class UILIB_API CHotKeyBox
	{
	public:
		/// 标题 + 提示；初始为空。outDisplay / outScope 可空。
		static int Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt,
			WORD& outVk, WORD& outModifiers, CDuiString* pOutDisplay = NULL, int* pOutScope = NULL);

		/// 标题 + 提示 + 初始快捷键
		static int Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt,
			WORD wVk, WORD wModifiers,
			WORD& outVk, WORD& outModifiers, CDuiString* pOutDisplay = NULL, int* pOutScope = NULL);

		static int Show(HWND hOwner, const CHotKeyBoxOptions& opts,
			WORD& outVk, WORD& outModifiers, CDuiString* pOutDisplay = NULL, int* pOutScope = NULL);
	};

} // namespace DuiLib

#endif
