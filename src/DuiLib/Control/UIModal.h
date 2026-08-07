#ifndef __UIMODAL_H__
#define __UIMODAL_H__

#pragma once

namespace DuiLib {

	class CMessageBox;

	/// 结果回调（ok=true 点确定；false 点取消 / Esc / 关窗 / 点遮罩）
	typedef void (CALLBACK *ModalResultCallback)(bool ok, LPCTSTR pUserData, void* pUser);

	/// Modal 配置（链式设置）
	class UILIB_API CModalOptions
	{
		friend class CModal;
		friend class CModalWnd;
		friend class CMessageBox;
	public:
		CModalOptions();

		CModalOptions& Title(LPCTSTR text);
		CModalOptions& Text(LPCTSTR text);
		CModalOptions& Kind(ControlKind kind);
		CModalOptions& ShowCancel(bool show);
		CModalOptions& OkText(LPCTSTR text);
		CModalOptions& CancelText(LPCTSTR text);
		CModalOptions& Width(int w);
		CModalOptions& Height(int h);
		/// 点击半透明遮罩关闭（取消）。默认 true
		CModalOptions& ClickBackdropToClose(bool close);
		/// 对齐/禁用基准窗；Create 不挂 Owner（避免 D2D 脏区裂开）
		CModalOptions& Owner(HWND hOwner);
		CModalOptions& OnResult(ModalResultCallback fn, void* pUser = NULL);
		CModalOptions& UserData(LPCTSTR data);

	private:
		CDuiString m_sTitle;
		CDuiString m_sText;
		ControlKind m_kind;
		bool m_bShowCancel;
		CDuiString m_sOkText;
		CDuiString m_sCancelText;
		int m_nWidth;
		int m_nHeight;
		bool m_bClickBackdropToClose;
		HWND m_hOwner;
		ModalResultCallback m_fnOnResult;
		void* m_pResultUser;
		CDuiString m_sUserData;
	};

	/// Modal 便捷 API（独立弹出 HWND；异步回调，不阻塞消息泵）
	class UILIB_API CModal
	{
	public:
		static HWND Show(LPCTSTR text, const CModalOptions& opts = CModalOptions());
		static HWND Show(LPCTSTR title, LPCTSTR text, const CModalOptions& opts = CModalOptions());

		static HWND ShowSuccess(LPCTSTR text, ModalResultCallback fn = NULL, void* pUser = NULL);
		static HWND ShowDanger(LPCTSTR text, ModalResultCallback fn = NULL, void* pUser = NULL);
		static HWND ShowWarning(LPCTSTR text, ModalResultCallback fn = NULL, void* pUser = NULL);
		static HWND ShowInfo(LPCTSTR text, ModalResultCallback fn = NULL, void* pUser = NULL);

		/// 确认框：标题 + 正文 + 确定/取消
		static HWND Confirm(LPCTSTR title, LPCTSTR text, ModalResultCallback fn = NULL, void* pUser = NULL);

		/// 关闭（等同取消）
		static void Dismiss(HWND hModal);
	};

} // namespace DuiLib

#endif // __UIMODAL_H__
