#ifndef WIN_IMPL_BASE_HPP
#define WIN_IMPL_BASE_HPP

namespace DuiLib
{
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

	protected:
		virtual CDuiString GetSkinType() { return _T(""); }
		virtual CDuiString GetSkinFile() = 0;
		virtual LPCTSTR GetWindowClassName(void) const = 0 ;
		virtual LPCTSTR GetManagerName() { return NULL; }
		virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
		CPaintManagerUI m_pm;

		HWND ResolveSyncOwner() const;
		void CaptureOwnerSyncOffset();
		void SyncOwnerPosition();
		void SyncOwnerSize();
		void SyncOwnerGeometry(bool bPos, bool bSize);

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
		POINT m_ptOwnerOffset;
		SIZE m_szOwnerDelta;
	};
}

#endif // WIN_IMPL_BASE_HPP
