#ifndef __UIHOTKEYBINDER_H__
#define __UIHOTKEYBINDER_H__

#pragma once

namespace DuiLib {

	class CButtonUI;

	/// 程序内 / 全局快捷键绑定（scope 由调用方决定）。
	/// APP：PreMessageFilter 捕获本窗按键；GLOBAL：RegisterHotKey + 子类化收 WM_HOTKEY。
	class UILIB_API CHotKeyBinder : public IMessageFilterUI
	{
	public:
		typedef void (CALLBACK *LPHotKeyHandler)(int id, WORD vk, WORD mod, int scope, LPVOID pUser);

		CHotKeyBinder();
		~CHotKeyBinder();

		void SetHandler(LPHotKeyHandler fn, LPVOID pUser = NULL);
		/// 字母/数字裸键是否要求修饰键；默认 true（F1~F12、Home 等不受限）
		void SetRequireModifier(bool require);
		bool IsRequireModifier() const { return m_bRequireModifier; }

		/// 挂到主窗 PaintManager（程序快捷键依赖）；可重复调用，先 Detach
		bool Attach(CPaintManagerUI* pm);
		void Detach();
		bool IsAttached() const { return m_pManager != NULL; }

		/// 绑定。id 由调用方分配且唯一；scope = HOTKEYBOX_SCOPE_APP(0) / GLOBAL(1)
		bool Bind(HWND hWnd, int id, WORD vk, WORD mod, int scope = 0);
		/// 从 Button 已存快捷键绑定（含 scope）
		bool BindButton(CButtonUI* pButton, int id);
		bool Unbind(int id);
		void UnbindAll();

		bool IsBound(int id) const;
		/// 查找已绑定项；找到返回 true
		bool Find(WORD vk, WORD mod, int* pId = NULL, int* pScope = NULL) const;

		/// 可选：自行处理 WM_HOTKEY 时调用（Attach 已子类化时可不必）
		bool HandleHotKeyMessage(WPARAM wParam, LPARAM lParam);

		// IMessageFilterUI — 程序快捷键
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

	private:
		struct Item {
			int id;
			HWND hWnd;
			WORD vk;
			WORD mod;
			int scope;
			bool bGlobalOk;
		};

		static bool IsModifierVk(UINT vk);
		static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
			UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

		void EnsureSubclass(HWND hWnd);
		void ReleaseSubclass(HWND hWnd);
		void Fire(const Item& item);
		int FindIndexById(int id) const;
		bool UnregisterGlobal(Item& item);

		CPaintManagerUI* m_pManager;
		LPHotKeyHandler m_pfnHandler;
		LPVOID m_pUser;
		bool m_bRequireModifier;
		CStdPtrArray m_aItems;     // Item*
		CStdPtrArray m_aSubclass;  // HWND 已子类化列表（存 HWND 值）
	};

} // namespace DuiLib

#endif
