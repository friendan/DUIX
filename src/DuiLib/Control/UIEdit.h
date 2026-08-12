#ifndef __UIEDIT_H__
#define __UIEDIT_H__

#pragma once

namespace DuiLib
{
	class CEditWnd;

	class UILIB_API CEditUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CEditUI)
		friend class CEditWnd;
	public:
		CEditUI();
		~CEditUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(bool bEnable = true);
		void SetText(LPCTSTR pstrText);
		void SetMaxChar(UINT uMax);
		UINT GetMaxChar();
		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;
		void SetPasswordMode(bool bPasswordMode);
		bool IsPasswordMode() const;
		void SetPasswordChar(TCHAR cPasswordChar);
		TCHAR GetPasswordChar() const;
		void SetNumberOnly(bool bNumberOnly);
		bool IsNumberOnly() const;
		int GetWindowStyls() const;

		/// 原生编辑区右侧预留（Spin 按钮等），逻辑像素，CalPos 内会 Scale。
		virtual int GetNativeEditRightReserve() const;
		/// 原生 Edit 文本已写回 m_sText 后回调（可重载做数值解析）。
		virtual void OnNativeEditChanged();

		LPCTSTR GetImage();
		void SetImage(LPCTSTR pStrImage);
		LPCTSTR GetHoverImage();
		void SetHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusImage();
		void SetFocusImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage();
		void SetDisabledImage(LPCTSTR pStrImage);
		void SetNativeEditBackgroundColor(DWORD dwBackgroundColor);
		DWORD GetNativeEditBackgroundColor() const;
		void SetNativeEditColor( LPCTSTR pStrColor );
		DWORD GetNativeEditColor() const;
		/// 主题热切时：已打开的原生 WC_EDIT 重刷底/字色
		void SyncNativeEditColors();

		bool IsAutoSelAll();
		void SetAutoSelAll(bool bAutoSelAll);
		void SetSel(long nStartChar, long nEndChar);
		void SetSelAll();
		void SetReplaceSel(LPCTSTR lpszReplace);

		void SetPlaceholder(LPCTSTR pStrPlaceholder);
		LPCTSTR GetPlaceholder();
		void SetPlaceholderColor(LPCTSTR pStrColor);
		DWORD GetPlaceholderColor();

		HWND GetHWND();

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		void SetVisible(bool bVisible = true);
		void SetInternVisible(bool bVisible = true);
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetOpacity(BYTE nOpacity) override;

		void PaintStatusImage(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);

	protected:
		/// 有效透明度 < 255 时不用原生 WC_EDIT，走自绘以支持 opacity
		bool CanHostNativeEdit() const;
		void DismissNativeEdit();

		CEditWnd* m_pWindow;

		UINT m_uMaxChar;
		bool m_bReadOnly;
		bool m_bPasswordMode;
		bool m_bAutoSelAll;
		TCHAR m_cPasswordChar;
		UINT m_uButtonState;
		CDuiString m_sImage;
		CDuiString m_sHoverImage;
		CDuiString m_sFocusImage;
		CDuiString m_sDisabledImage;
		CDuiString m_sPlaceholder;
		DWORD m_dwPlaceholderColor;
		DWORD m_dwEditbkColor;
		DWORD m_dwEditTextColor;
		bool m_bNativeBkColorCustom;
		bool m_bNativeTextColorCustom;
		int m_iWindowStyls;
	};
}
#endif // __UIEDIT_H__