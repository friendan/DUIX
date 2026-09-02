#include "StdAfx.h"
#include "UIMessageBox.h"
#include "UIModal.h"

namespace DuiLib {

	namespace {

		struct MsgBoxSyncState
		{
			bool done;
			bool ok;
			HWND hModal;
			ModalResultCallback userFn;
			void* userPtr;
		};

		void CALLBACK OnMsgBoxSync(bool ok, LPCTSTR data, void* pUser)
		{
			MsgBoxSyncState* s = (MsgBoxSyncState*)pUser;
			if( s == NULL ) return;
			s->ok = ok;
			s->done = true;
			if( s->userFn )
				s->userFn(ok, data, s->userPtr);
		}

		int RunModalMessageLoop(MsgBoxSyncState& sync)
		{
			if( sync.hModal == NULL )
				return MESSAGEBOX_CANCEL;

			MSG msg = { 0 };
			while( !sync.done && ::IsWindow(sync.hModal) && ::GetMessage(&msg, NULL, 0, 0) ) {
				if( !CPaintManagerUI::TranslateMessage(&msg) ) {
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
				if( msg.message == WM_QUIT ) {
					::PostQuitMessage((int)msg.wParam);
					break;
				}
			}
			return sync.ok ? MESSAGEBOX_OK : MESSAGEBOX_CANCEL;
		}

		// ShowSkin 未指定 skin 时的内置 XML（控件名与旧 msg.html 对齐）
		LPCTSTR GetBuiltinMsgSkin()
		{
			return
				_T("<Window caption=\"0,0,0,40\" size=\"400,195\">")
				_T("<Font id=\"0\" font-family=\"Microsoft YaHei UI\" font-size=\"12\" default=\"true\"/>")
				_T("<Font id=\"1\" font-family=\"Microsoft YaHei UI\" font-size=\"14\"/>")
				_T("<vbox background-color=\"#FFFFFFFF\">")
				_T("<hbox name=\"appbar\" height=\"36\" align-items=\"vcenter\" background-color=\"#202223FF\">")
				_T("<Label name=\"MessageTitle\" vertical-align=\"vcenter\" padding-left=\"12\" text=\"提示\" color=\"#FFFFFFFF\" font=\"1\"/>")
				_T("<Button name=\"closebtn\" text=\"✕\" width=\"39\" height=\"28\" kind=\"none\" ")
				_T("background-color=\"#202223FF\" color=\"#B4B4BEFF\" color-hover=\"#FFFFFFFF\" background-color-hover=\"#E81123FF\"/>")
				_T("</hbox>")
				_T("<vbox padding=\"10,20,10,20\">")
				_T("<Label name=\"MessageText\" color=\"#3C3C3CFF\" font=\"1\" text-align=\"left\" vertical-align=\"vcenter\"/>")
				_T("</vbox>")
				_T("<hbox height=\"60\" padding=\"10,10,10,10\" align-items=\"vcenter\">")
				_T("<Control/>")
				_T("<Button name=\"confirm_btn\" text=\"确定\" kind=\"primary\" width=\"72\" height=\"28\"/>")
				_T("<Control width=\"12\"/>")
				_T("<Button name=\"cancel_btn\" text=\"取消\" kind=\"default\" width=\"72\" height=\"28\"/>")
				_T("<Control width=\"20\"/>")
				_T("</hbox>")
				_T("</vbox>")
				_T("</Window>");
		}

	} // namespace

	// 消息映射需要外部链接，不能放匿名命名空间
	class CMessageBoxSkinWnd : public WindowImplBase
	{
	public:
		CMessageBoxSkinWnd(LPCTSTR skin, LPCTSTR skinType)
			: m_pCloseBtn(NULL)
		{
			if( skin && *skin )
				m_sSkin = skin;
			else
				m_sSkin = GetBuiltinMsgSkin();
			if( skinType && *skinType )
				m_sSkinType = skinType;
		}

		void SetTitle(LPCTSTR title)
		{
			if( title == NULL || *title == 0 ) return;
			CControlUI* p = m_pm.FindControl(_T("MessageTitle"));
			if( p ) p->SetText(title);
		}

		void SetMsg(LPCTSTR text)
		{
			if( text == NULL || *text == 0 ) return;
			CControlUI* p = m_pm.FindControl(_T("MessageText"));
			if( p ) p->SetText(text);
		}

		void OnFinalMessage(HWND hWnd) override
		{
			WindowImplBase::OnFinalMessage(hWnd);
			delete this;
		}

		CDuiString GetSkinFile() override { return m_sSkin; }
		CDuiString GetSkinType() override { return m_sSkinType; }
		LPCTSTR GetWindowClassName() const override { return _T("DuiMessageBoxWnd"); }

		void InitWindow() override
		{
			m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
		}

		DUI_DECLARE_MESSAGE_MAP()
		void OnClick(TNotifyUI& msg) override
		{
			if( msg.pSender == m_pCloseBtn ) {
				Close(MESSAGEBOX_CANCEL);
				return;
			}
			CDuiString sName = msg.pSender->GetName();
			if( sName.CompareNoCase(_T("confirm_btn")) == 0 ) {
				Close(MESSAGEBOX_OK);
				return;
			}
			if( sName.CompareNoCase(_T("cancel_btn")) == 0 ) {
				Close(MESSAGEBOX_CANCEL);
				return;
			}
			WindowImplBase::OnClick(msg);
		}

	private:
		CDuiString m_sSkin;
		CDuiString m_sSkinType;
		CButtonUI* m_pCloseBtn;
	};

	DUI_BEGIN_MESSAGE_MAP(CMessageBoxSkinWnd, WindowImplBase)
		DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CMessageBoxSkinWnd::OnClick)
	DUI_END_MESSAGE_MAP()

	/////////////////////////////////////////////////////////////////////////////////////
	// CMessageBox

	int CMessageBox::Show(HWND hOwner, LPCTSTR title, LPCTSTR text)
	{
		return Show(hOwner, title, text,
			CModalOptions()
				.Kind(CONTROLKIND_PRIMARY)
				.ShowCancel(true));
	}

	int CMessageBox::Show(HWND hOwner, LPCTSTR title, LPCTSTR text, const CModalOptions& opts)
	{
		MsgBoxSyncState sync = { false, false, NULL, NULL, NULL };
		CModalOptions o = opts;
		sync.userFn = o.m_fnOnResult;
		sync.userPtr = o.m_pResultUser;
		o.Owner(hOwner);
		o.OnResult(OnMsgBoxSync, &sync);

		sync.hModal = CModal::Show(title, text, o);
		return RunModalMessageLoop(sync);
	}

	int CMessageBox::ShowInfo(HWND hOwner, LPCTSTR title, LPCTSTR text)
	{
		return Show(hOwner, title, text,
			CModalOptions()
				.Kind(CONTROLKIND_INFO)
				.ShowCancel(false));
	}

	int CMessageBox::ShowSkin(HWND hOwner, LPCTSTR title, LPCTSTR text,
		LPCTSTR skin, LPCTSTR skinType)
	{
		CMessageBoxSkinWnd* pWnd = new CMessageBoxSkinWnd(skin, skinType);
		pWnd->Create(hOwner, _T("MessageBox"), WS_POPUP | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW);
		if( pWnd->GetHWND() == NULL ) {
			delete pWnd;
			return MESSAGEBOX_CANCEL;
		}
		pWnd->CenterWindow();
		pWnd->SetTitle(title);
		pWnd->SetMsg(text);
		return (int)pWnd->ShowModal();
	}

} // namespace DuiLib
