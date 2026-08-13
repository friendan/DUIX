#ifndef WIN_IMPL_BASE_HPP
#define WIN_IMPL_BASE_HPP

#include "TrayIcon.h"

namespace DuiLib
{
	class CMenuWnd;

	class UILIB_API WindowImplBase
		: public CWindowWnd
		, public CNotifyPump
		, public INotifyUI
		, public IMessageFilterUI
		, public IDialogBuilderCallback
		, public IQueryControlText
	{
	public:
		WindowImplBase();
		virtual ~WindowImplBase(){};
		// 只需主窗口重写（初始化资源与多语言接口）
		virtual void InitResource(){};
		// 每个窗口都可以重写
		virtual void InitWindow(){};
		virtual void OnFinalMessage( HWND hWnd );
		virtual void Notify(TNotifyUI& msg);

		DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);
		virtual BOOL IsInStaticControl(CControlUI *pControl);

	public:
		/// 拖/移本窗时同步移动 Create 时的 Owner（默认 false）。
		void SetSyncOwnerMove(bool sync);
		bool IsSyncOwnerMove() const { return m_bSyncOwnerMove; }

		/// 缩放本窗时同步缩放 Owner（默认 false；保留打开时的宽高差，铺满场景差为 0）。
		/// 设置窗铺满主窗时与 SetSyncOwnerMove(true) 一起开。
		void SetSyncOwnerSize(bool sync);
		bool IsSyncOwnerSize() const { return m_bSyncOwnerSize; }

		/// 进入模态前抓取与 Owner 的屏幕偏移/尺寸差；关闭后自动清除。
		UINT ShowModal();
		void ShowModalFake();

		/// 按 shape-image 像素 ResizeClient（默认同屏工作区 95% 钳制）并可选居中
		bool FitToShapeImage(bool clampWorkArea = true, bool bCenter = true, int workAreaPercent = 95);

		/// 真正关闭窗口（绕过 TitleBar close-to-tray）。托盘菜单「退出」等应调用此方法。
		void ForceClose(UINT nRet = 0);

		/// 内置托盘（min-to-tray / close-to-tray 时 EnsureAutoTray 可能自动 Create）
		CTrayIcon& GetTrayIcon() { return m_trayIcon; }
		const CTrayIcon& GetTrayIcon() const { return m_trayIcon; }
		/// 是否由库自动创建的托盘（应用在 InitWindow 里自行 Create 则为 false）
		bool IsTrayAutoCreated() const { return m_bTrayAutoCreated; }

	protected:
		virtual CDuiString GetSkinType() { return _T(""); }
		virtual CDuiString GetSkinFile() = 0;
		virtual LPCTSTR GetWindowClassName(void) const = 0 ;
		virtual LPCTSTR GetManagerName() { return NULL; }
		virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
		CPaintManagerUI m_pm;

		/// TitleBar 带 min-to-tray / close-to-tray 且尚未 Create 托盘时自动创建
		void EnsureAutoTray();
		/// 自动托盘默认提示（默认同窗口标题）
		virtual CDuiString GetAutoTrayTooltip() const;
		/// 处理自动托盘消息；返回 true 表示已处理。自建托盘+自定义菜单的窗口不会走到这里（m_bTrayAutoCreated=false）
		bool ProcessAutoTrayMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult);
		/// 默认右键菜单：「显示主窗口」「退出」
		void ShowDefaultTrayMenu(POINT pt);
		/// 处理默认托盘菜单命令（tray_show / tray_exit）
		bool ProcessDefaultTrayMenuCommand(LPCTSTR pstrName);

		HWND ResolveSyncOwner() const;
		void CaptureOwnerSyncOffset();
		void SyncOwnerPosition();
		void SyncOwnerSize();
		void SyncOwnerGeometry(bool bPos, bool bSize);

		CTrayIcon m_trayIcon;
		CMenuWnd* m_pDefaultTrayMenu;
		bool m_bTrayAutoCreated;

	public:
		virtual UINT GetClassStyle() const;
		virtual CControlUI* CreateControl(LPCTSTR pstrClass);
		virtual LPCTSTR QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType);

		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
		virtual LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

#if defined(WIN32) && !defined(UNDER_CE)
		virtual LRESULT OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		virtual LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif
		virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnDisplayChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LONG GetStyle();

	private:
		bool m_bSyncOwnerMove;
		bool m_bSyncOwnerSize;
		bool m_bHaveOwnerOffset;
		bool m_bSyncingOwner;
		bool m_bForceClose;
		POINT m_ptOwnerOffset;
		SIZE m_szOwnerDelta;
	};
}

#endif // WIN_IMPL_BASE_HPP
