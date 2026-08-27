#ifndef __UIINPUTBOX_H__
#define __UIINPUTBOX_H__

#pragma once

namespace DuiLib {

	enum {
		INPUTBOX_OK = 1,
		INPUTBOX_CANCEL = 0,
	};

	/// 同步输入对话框配置（链式）。
	class UILIB_API CInputBoxOptions
	{
		friend class CInputBox;
		friend class CInputBoxWnd;
	public:
		CInputBoxOptions();

		CInputBoxOptions& Title(LPCTSTR text);
		/// 输入框上方说明文字（可空）
		CInputBoxOptions& Prompt(LPCTSTR text);
		/// 初始文本
		CInputBoxOptions& Value(LPCTSTR text);
		CInputBoxOptions& Placeholder(LPCTSTR text);
		CInputBoxOptions& Password(bool password);
		/// 仅允许数字（Edit `ES_NUMBER`，不含小数点/负号）
		CInputBoxOptions& Number(bool numberOnly);
		CInputBoxOptions& MaxLength(UINT nMax);
		CInputBoxOptions& OkText(LPCTSTR text);
		CInputBoxOptions& CancelText(LPCTSTR text);
		CInputBoxOptions& Width(int w);
		CInputBoxOptions& Height(int h);
		/// 打开后是否全选初始文本；默认 true
		CInputBoxOptions& SelectAll(bool selectAll);
		CInputBoxOptions& Owner(HWND hOwner);

	private:
		CDuiString m_sTitle;
		CDuiString m_sPrompt;
		CDuiString m_sValue;
		CDuiString m_sPlaceholder;
		bool m_bPassword;
		bool m_bNumberOnly;
		UINT m_nMaxLength;
		CDuiString m_sOkText;
		CDuiString m_sCancelText;
		int m_nWidth;
		int m_nHeight;
		bool m_bSelectAll;
		HWND m_hOwner;
	};

	/// 同步输入对话框：阻塞直到关闭；确定时把文本写入 outText。
	/// 默认内置皮肤（无需工程 html）；确定 / 回车 = OK，取消 / Esc / 关窗 = CANCEL。
	class UILIB_API CInputBox
	{
	public:
		/// 标题 + 提示；初始值为空
		static int Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt, CDuiString& outText);

		/// 标题 + 提示 + 初始值
		static int Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt, LPCTSTR defaultValue, CDuiString& outText);

		/// 完整配置
		static int Show(HWND hOwner, const CInputBoxOptions& opts, CDuiString& outText);
	};

} // namespace DuiLib

#endif // __UIINPUTBOX_H__
