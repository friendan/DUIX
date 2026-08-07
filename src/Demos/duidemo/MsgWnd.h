#pragma once

//////////////////////////////////////////////////////////////////////////
/// Demo 兼容封装：默认走库内 CMessageBox（无需 msg.html）
#define MSGID_OK		MESSAGEBOX_OK
#define MSGID_CANCEL	MESSAGEBOX_CANCEL

class CMsgWnd
{
public:
	/// 同步确认框（纯代码 UI，无需皮肤）
	static int MessageBox(HWND hParent, LPCTSTR lpstrTitle, LPCTSTR lpstrMsg)
	{
		return CMessageBox::Show(hParent, lpstrTitle, lpstrMsg);
	}

	/// 异步弹出（不阻塞）
	static void ShowMessageBox(HWND hParent, LPCTSTR lpstrTitle, LPCTSTR lpstrMsg)
	{
		CModal::Show(lpstrTitle, lpstrMsg,
			CModalOptions()
				.ShowCancel(true)
				.Owner(hParent));
	}

	/// 可选：自定义皮肤（XML_MSG / msg.html / 内联 XML）
	static int MessageBoxSkin(HWND hParent, LPCTSTR lpstrTitle, LPCTSTR lpstrMsg,
		LPCTSTR skin = _T("XML_MSG"), LPCTSTR skinType = NULL)
	{
		return CMessageBox::ShowSkin(hParent, lpstrTitle, lpstrMsg, skin, skinType);
	}
};
