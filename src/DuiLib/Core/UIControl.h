#ifndef __UICONTROL_H__
#define __UICONTROL_H__

#pragma once

namespace DuiLib {

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class IRenderContext;
	class CTheme;
	typedef CControlUI* (CALLBACK* FINDCONTROLPROC)(CControlUI*, LPVOID);

	// Bootstrap 5.3.8 风格枚举
	enum ControlKind
	{
		CONTROLKIND_NONE = 0,
		CONTROLKIND_DEFAULT,
		CONTROLKIND_PRIMARY,
		CONTROLKIND_SECONDARY,
		CONTROLKIND_SUCCESS,
		CONTROLKIND_DANGER,
		CONTROLKIND_WARNING,
		CONTROLKIND_INFO,
		CONTROLKIND_LIGHT,
		CONTROLKIND_DARK,
		CONTROLKIND_LINK,
	};

	struct KindStateColors
	{
		DWORD dwBackgroundColor;
		DWORD dwBorderColor;
		DWORD dwColor;
	};

	struct KindColors
	{
		KindStateColors Normal;
		KindStateColors Hover;
		KindStateColors Active;
	};

	extern UILIB_API KindColors g_kindColors[11];
	UILIB_API void InitKindColors();
	UILIB_API void MarkKindColorsInitialized();
	/// 填入历史 Bootstrap kind 表（主题禁用时回退）
	UILIB_API void FillBuiltinKindColors();

	class UILIB_API CControlUI
	{
		DECLARE_DUICONTROL(CControlUI)
	public:
		CControlUI();
		virtual ~CControlUI();

	public:
		virtual CDuiString GetName() const;
		virtual void SetName(LPCTSTR pstrName);
		virtual LPCTSTR GetClass() const;
		virtual LPVOID GetInterface(LPCTSTR pstrName);
		virtual UINT GetControlFlags() const;
		/// action=title/move 时：该点是否应作为标题栏拖拽（HTCAPTION）。
		/// 默认整控件可拖；含交互子区域的控件（如 TabBar）可重写，仅空白区返回 true。
		virtual bool IsCaptionDragHit(POINT pt) const;
		/// 命中测试：是否应保留 HTCLIENT，不继承父级/`html { action: title }` 拖拽。
		/// 默认：UIFLAG_SETCURSOR、显式 cursor、或已配置热态/按下态背景与边框视觉。
		/// 子类自有热态字段（如 color-hover）应重写并先判断自身再调基类。
		virtual bool PreferClientHit() const;

		/// 控件边缘缩放宿主 HWND（与窗口 html{size-box} 互补）。位：L=1 T=2 R=4 B=8。
		enum {
			WINDOW_RESIZE_LEFT   = 0x01,
			WINDOW_RESIZE_TOP    = 0x02,
			WINDOW_RESIZE_RIGHT  = 0x04,
			WINDOW_RESIZE_BOTTOM = 0x08,
			WINDOW_RESIZE_ALL    = 0x0F
		};
		void SetWindowResizeEdges(UINT uEdges);
		UINT GetWindowResizeEdges() const;
		/// 逻辑像素边厚（LTRB，同窗口 size-box）；0 表示该边不参与；命中时经 DPI Scale。
		/// 已启用边若厚度为 0，回退窗口 GetSizeBox()。
		void SetWindowSizeBox(RECT rc);
		RECT GetWindowSizeBox() const;
		/// 已启用边的有效热区厚度（DPI 后；未启用边为 0；启用但未写厚度时回退窗口 size-box / 默认 4,4,6,6）
		RECT GetWindowResizeThickness() const;
		/// 原生子 HWND 布局：按自身及祖先 `window-resize` 内缩，避免盖住父窗缩放热区（与窗口 size-box 互补）
		void ApplyAncestorWindowResizeHostInset(RECT& rcHost) const;
		/// 点在本控件矩形启用边上则返回 HTLEFT/…，否则 HTCLIENT。最大化跳过。
		LRESULT HitWindowResize(POINT ptClient) const;

		virtual bool Activate();
		virtual CPaintManagerUI* GetManager() const;
		virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
		virtual CControlUI* GetParent() const;
	    void setInstance(HINSTANCE instance = NULL) {m_instance = instance;};
		
		// 定时器
		bool SetTimer(UINT nTimerID, UINT nElapse);
		void KillTimer(UINT nTimerID);

		// 文本相关
		virtual CDuiString GetText() const;
		virtual void SetText(LPCTSTR pstrText);

		virtual bool IsResourceText() const;
		virtual void SetResourceText(bool bResource);

		virtual bool IsDragEnabled() const;
		virtual void SetDragEnable(bool bDrag);

		virtual bool IsDropEnabled() const;
		virtual void SetDropEnable(bool bDrop);

		virtual bool IsRichEvent() const;
		virtual void SetRichEvent(bool bEnable);
		// 图形相关
		/// 背景：纯色或 CSS linear-gradient（如 linear-gradient(to right, #fff, #000)）
		void SetBackground(LPCTSTR pstrValue);
		LPCTSTR GetBackground() const;
		DWORD GetBackgroundColor() const;
		void SetBackgroundColor(DWORD dwBackColor);
		DWORD GetForeColor() const;
		void SetForeColor(DWORD dwForeColor);
		LPCTSTR GetBackgroundImage();
		void SetBackgroundImage(LPCTSTR pStrImage);
		/// 从内存设背景图：PNG/JPEG/BMP/GIF；需已挂 Manager
		bool SetBackgroundImageFromMemory(const BYTE* pData, DWORD dwSize, DWORD mask = 0);
		bool SetBackgroundImageFromSvg(const char* utf8Svg, size_t nBytes,
			int width = 0, int height = 0, DWORD dwTintColor = 0);
		bool SetBackgroundImageFromSvg(LPCTSTR pstrSvg,
			int width = 0, int height = 0, DWORD dwTintColor = 0);
		LPCTSTR GetHoverBackgroundImage() const;
		void SetHoverBackgroundImage(LPCTSTR pStrImage);
		LPCTSTR GetActiveBackgroundImage() const;
		void SetActiveBackgroundImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledBackgroundImage() const;
		void SetDisabledBackgroundImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusBackgroundImage() const;
		void SetFocusBackgroundImage(LPCTSTR pStrImage);
		LPCTSTR GetSelectedBackgroundImage() const;
		void SetSelectedBackgroundImage(LPCTSTR pStrImage);
		LPCTSTR GetForegroundImage() const;
		void SetForegroundImage(LPCTSTR pStrImage);

		// 通用状态色（容器 / 基类控件；Button 等子类可自有同名属性覆盖绘制）
		DWORD GetHoverBackgroundColor() const;
		void SetHoverBackgroundColor(DWORD dwColor);
		DWORD GetActiveBackgroundColor() const;
		void SetActiveBackgroundColor(DWORD dwColor);
		DWORD GetDisabledBackgroundColor() const;
		void SetDisabledBackgroundColor(DWORD dwColor);
		DWORD GetFocusBackgroundColor() const;
		void SetFocusBackgroundColor(DWORD dwColor);
		DWORD GetHoverBorderColor() const;
		void SetHoverBorderColor(DWORD dwColor);
		DWORD GetActiveBorderColor() const;
		void SetActiveBorderColor(DWORD dwColor);
		DWORD GetDisabledBorderColor() const;
		void SetDisabledBorderColor(DWORD dwColor);

		DWORD GetFocusBorderColor() const;
		void SetFocusBorderColor(DWORD dwBorderColor);
		bool IsColorHSL() const;
		void SetColorHSL(bool bColorHSL);
		BYTE GetOpacity() const;
		virtual void SetOpacity(BYTE nOpacity);
		/// 0~1 浮点；内部仍存 0~255
		void SetOpacityF(float fOpacity01);
		float GetOpacityF() const;
		/// 设透明度；bIsolateFromParent=true 时同时断开祖先乘算（≡ opacity-isolate）
		void SetOpacity(BYTE nOpacity, bool bIsolateFromParent);
		void SetOpacityF(float fOpacity01, bool bIsolateFromParent);
		/// 绘制时是否 × 祖先 opacity（默认 true）。false / opacity-isolate 则只看自身
		bool IsOpacityInherit() const;
		void SetOpacityInherit(bool bInherit);
		/// 本控件 opacity 是否传给子孙（默认 true）。容器设 false 时子控件乘算会跳过本节点
		bool IsOpacityPropagate() const;
		void SetOpacityPropagate(bool bPropagate);
		/// 绘制用透明度（已含继承规则）
		BYTE GetEffectiveOpacity() const;
		float GetEffectiveOpacityF() const;
		/// 图片 fade 再乘有效透明度
		UINT ScaleImageFade(UINT uFade = 255) const;

		enum { WALLPAPER_BLEED_INHERIT = -1, WALLPAPER_BLEED_SOLID = -2 };
		/// 壁纸透出：面板底色 alpha 系数（继承窗口值）。INHERIT / SOLID / 0~255
		void SetWallpaperBleed(int nBleedOrSentinel);
		int GetWallpaperBleed() const;
		BYTE ResolveWallpaperBleedFactor() const;
		SIZE GetBorderRadius() const; // CSS 圆角半径（非 GDI 椭圆直径）
		void SetBorderRadius(SIZE cxyRound);
		bool DrawImage(IRenderContext& ctx, LPCTSTR pStrImage, LPCTSTR pStrModify = NULL);

		//边框相关
		int GetBorderWidth() const;
		void SetBorderWidth(int nSize);
		DWORD GetBorderColor() const;
		void SetBorderColor(DWORD dwBorderColor);
		RECT GetBorderRectWidth() const;
		void SetBorderWidth(RECT rc);
		int GetLeftBorderWidth() const;
		void SetLeftBorderWidth(int nSize);
		int GetTopBorderWidth() const;
		void SetTopBorderWidth(int nSize);
		int GetRightBorderWidth() const;
		void SetRightBorderWidth(int nSize);
		int GetBottomBorderWidth() const;
		void SetBottomBorderWidth(int nSize);
		int GetBorderStyle() const;
		void SetBorderStyle(int nStyle);

		// 位置相关
		virtual RECT GetRelativePos() const; // 相对(父控件)位置
		virtual RECT GetClientPos() const; // 客户区域（除去scrollbar和padding）
		virtual const RECT& GetPos() const;
		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		virtual void Move(SIZE szOffset, bool bNeedInvalidate = true);
		virtual int GetWidth() const;
		virtual int GetHeight() const;
		virtual int GetX() const;
		virtual int GetY() const;
		virtual CDuiBox GetMargin() const;
		virtual void SetMargin(CDuiBox rcMargin); // 外边距（属性 margin；根节点相对窗口）
		virtual CDuiBox GetPadding() const;
		virtual void SetPadding(CDuiBox rcPadding);     // 内边距（属性 padding；内容区相对边框）
		virtual SIZE GetFixedXY() const;         // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
		virtual void SetFixedXY(SIZE szXY);      // 仅float为true时有效
		virtual SIZE GetFixedSize() const;
		virtual int GetFixedWidth() const;       // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
		virtual void SetFixedWidth(int cx);      // 预设的参考值（像素）；会清除 width 百分比
		virtual int GetFixedHeight() const;      // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
		virtual void SetFixedHeight(int cy);     // 预设的参考值（像素）；会清除 height 百分比
		/// 相对父级可用尺寸的百分比（0=未使用；1.0=100%）。与「未设固定尺寸→撑满剩余」不同：百分比按父级全量比例计算。
		virtual float GetWidthPercent() const;
		virtual void SetWidthPercent(float fPercent);
		virtual float GetHeightPercent() const;
		virtual void SetHeightPercent(float fPercent);
		virtual bool IsWidthPercent() const;
		virtual bool IsHeightPercent() const;
		/// width/height="auto"|"fit-content"：按 EstimateSize 固有尺寸（Label 等可测内容）；EstimateSize 仍为 0 时布局仍作撑满
		virtual bool GetAutoCalcWidth() const;
		virtual void SetAutoCalcWidth(bool bAutoCalcWidth);
		virtual bool GetAutoCalcHeight() const;
		virtual void SetAutoCalcHeight(bool bAutoCalcHeight);
		virtual int GetMinWidth() const;
		virtual void SetMinWidth(int cx);
		virtual int GetMaxWidth() const;
		virtual void SetMaxWidth(int cx);
		virtual int GetMinHeight() const;
		virtual void SetMinHeight(int cy);
		virtual int GetMaxHeight() const;
		virtual void SetMaxHeight(int cy);
		virtual TPercentInfo GetAbsolutePercent() const;
		virtual void SetAbsolutePercent(TPercentInfo piAbsolutePercent);
		virtual void SetAbsoluteAlign(UINT uAlign);
		virtual UINT GetAbsoluteAlign() const;
		virtual void SetTextAlign(int iAlign);
		virtual int GetTextAlign() const;
		virtual void SetVerticalAlign(int iAlign);
		virtual int GetVerticalAlign() const;
		// 鼠标提示
		virtual CDuiString GetToolTip() const;
		virtual void SetToolTip(LPCTSTR pstrText);
		virtual void SetToolTipWidth(int nWidth);
		virtual int	  GetToolTipWidth(void);	// 多行ToolTip单行最长宽度
		
		// 光标
		virtual WORD GetCursor();
		virtual void SetCursor(WORD wCursor);

		// 快捷键
		virtual TCHAR GetShortcut() const;
		virtual void SetShortcut(TCHAR ch);

		// 菜单
		virtual bool IsContextMenuUsed() const;
		virtual void SetContextMenuUsed(bool bMenuUsed);

		// 用户属性
		virtual const CDuiString& GetUserData(); // 辅助函数，供用户使用
		virtual void SetUserData(LPCTSTR pstrText); // 辅助函数，供用户使用
		virtual UINT_PTR GetTag() const; // 辅助函数，供用户使用
		virtual void SetTag(UINT_PTR pTag); // 辅助函数，供用户使用

		// 控件行为（action 属性），用于承担窗口关闭/最小化/最大化/标题拖动等角色
		virtual UIAction GetAction() const;
		virtual void SetAction(UIAction action);

		// Bootstrap kind 风格
		virtual void SetKind(ControlKind kind);
		ControlKind GetKind() const;
		virtual void SetOutline(bool bOutline);
		bool IsOutline() const;

		// 一些重要的属性
		virtual bool IsVisible() const;
		/// 仅自身 visible 标志（不含父级 InternVisible）
		bool IsSelfVisible() const { return m_bVisible; }
		virtual void SetVisible(bool bVisible = true);
		virtual void SetInternVisible(bool bVisible = true); // 仅供内部调用，有些UI拥有窗口句柄，需要重写此函数
		virtual bool IsEnabled() const;
		virtual void SetEnabled(bool bEnable = true);
		virtual bool IsMouseEnabled() const;
		virtual void SetMouseEnabled(bool bEnable = true);
		virtual bool IsKeyboardEnabled() const;
		virtual void SetKeyboardEnabled(bool bEnable = true);
		virtual bool IsFocused() const;
		virtual void SetFocus();
		virtual bool IsAbsolute() const;
		virtual void SetAbsolute(bool bAbsolute = true);

		virtual CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

		virtual void Invalidate();
		bool IsUpdateNeeded() const;
		void NeedUpdate();
		void NeedParentUpdate();
		DWORD GetAdjustColor(DWORD dwColor);

		virtual void Init();
		virtual void DoInit();
		/// DPI / RemoveAllImages 后重建依赖内存图的资源（默认空；AppIcon 等重写）
		virtual void OnResetDpiAssets();

		virtual void Event(TEventUI& event);
		virtual void DoCaptureEvent(TEventUI& event);
		virtual void DoEvent(TEventUI& event);
		bool BubbleEvent(TEventUI& event);

		// 自定义(未处理的)属性
		void AddCustomAttribute(LPCTSTR pstrName, LPCTSTR pstrAttr);
		LPCTSTR GetCustomAttribute(LPCTSTR pstrName) const;
		bool RemoveCustomAttribute(LPCTSTR pstrName);
		void RemoveAllCustomAttribute();
		/// 重解 markup 中记录的 var(--token) 颜色属性（热切换主题）
		void RefreshThemeVarAttributes(CTheme* pParseTheme);

		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		CControlUI* ApplyAttributeList(LPCTSTR pstrList);

		virtual SIZE EstimateSize(SIZE szAvailable);
		virtual bool Paint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl = NULL); // 返回要不要继续绘制
		virtual bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		virtual void PaintBackgroundColor(IRenderContext& ctx);
		virtual void PaintBackgroundImage(IRenderContext& ctx);
		virtual void PaintStatusImage(IRenderContext& ctx);
		virtual void PaintForeColor(IRenderContext& ctx);
		virtual void PaintForegroundImage(IRenderContext& ctx);
		virtual void PaintText(IRenderContext& ctx);
		virtual void PaintBorder(IRenderContext& ctx);

		virtual void DoPostPaint(IRenderContext& ctx, const RECT& rcPaint);

		//虚拟窗口参数
		void SetVirtualWnd(LPCTSTR pstrValue);
		CDuiString GetVirtualWnd() const;

	public:
		CEventSource OnInit;
		CEventSource OnDestroy;
		CEventSource OnSize;
		CEventSource OnEvent;
		CEventSource OnNotify;

	protected:
		bool HasStateVisual() const;
		DWORD GetPaintBackgroundColor() const;
		DWORD GetPaintBorderColor() const;

		CPaintManagerUI* m_pManager;
		CControlUI* m_pParent;
		CDuiString m_sVirtualWnd;
		CDuiString m_sName;
		bool m_bUpdateNeeded;
		bool m_bMenuUsed;
		RECT m_rcItem;
		CDuiBox m_rcMargin;
		CDuiBox m_rcPadding;
		SIZE m_cXY;
		SIZE m_cxyFixed;
		float m_fWidthPercent;   // >0 时 EstimateSize 按父级可用宽 * 百分比
		float m_fHeightPercent;  // >0 时 EstimateSize 按父级可用高 * 百分比
		bool m_bAutoCalcWidth;   // width=auto|fit-content
		bool m_bAutoCalcHeight;  // height=auto|fit-content
		SIZE m_cxyMin;
		SIZE m_cxyMax;
		bool m_bVisible;
		bool m_bInternVisible;
		bool m_bEnabled;
		bool m_bMouseEnabled;
		bool m_bKeyboardEnabled;
		bool m_bFocused;
		bool m_bAbsolute;
		TPercentInfo m_piAbsolutePercent;
		UINT m_uAbsoluteAlign;
		int m_iTextAlign;
		int m_iVerticalAlign;
		bool m_bSetPos; // 防止SetPos循环调用

		bool m_bRichEvent;
		bool m_bDragEnabled;
		bool m_bDropEnabled;

		bool m_bResourceText;
		CDuiString m_sText;
		CDuiString m_sToolTip;
		TCHAR m_chShortcut;
		CDuiString m_sUserData;
		UINT_PTR m_pTag;
		UIAction m_uAction;
		ControlKind m_controlKind;
		bool m_bOutline;

		CDuiString m_sBackground; // 原始 background 值（linear-gradient 时保留）
		bool m_bGradientVertical; // true=纵向(to bottom)，false=横向(to right)
		DWORD m_dwBackColor;
		DWORD m_dwBackColor2;
		DWORD m_dwBackColor3;
		DWORD m_dwForeColor;
		CDuiString m_sBackgroundImage;
		CDuiString m_sBackgroundImageHover;
		CDuiString m_sBackgroundImageActive;
		CDuiString m_sBackgroundImageDisabled;
		CDuiString m_sBackgroundImageFocus;
		CDuiString m_sBackgroundImageSelected;
		CDuiString m_sForegroundImage;
		DWORD m_dwBorderColor;
		DWORD m_dwFocusBorderColor;
		DWORD m_dwHoverBackgroundColor;
		DWORD m_dwActiveBackgroundColor;
		DWORD m_dwDisabledBackgroundColor;
		DWORD m_dwFocusBackgroundColor;
		DWORD m_dwHoverBorderColor;
		DWORD m_dwActiveBorderColor;
		DWORD m_dwDisabledBorderColor;
		UINT m_uControlState;
		bool m_bColorHSL;
		BYTE m_nOpacity; // 本控件 opacity，255=不透明
		bool m_bOpacityInherit; // 默认 true：GetEffectiveOpacity × 祖先
		bool m_bOpacityPropagate; // 默认 true：子孙乘算时计入本节点；false 则跳过
		int m_iWallpaperBleed; // -1 继承窗口；-2 solid；0~255 本控件系数
		int m_nPaintBackgroundDepth; // PaintBackgroundColor 期间对 GetAdjustColor 套 bleed
		int m_nBorderWidth;
		int m_nBorderStyle;
		int m_nTooltipWidth;
		WORD m_wCursor;
		SIZE m_cxyBorderRadius;
		RECT m_rcPaint;
		RECT m_rcBorderWidth;
		UINT m_uWindowResizeEdges; // 0=关；WINDOW_RESIZE_*
		RECT m_rcWindowSizeBox;    // 逻辑边厚 LTRB；0=该边用窗口 size-box 或禁用
	    HINSTANCE m_instance;

		CStdStringPtrMap m_mCustomAttrHash;
	};

} // namespace DuiLib

#endif // __UICONTROL_H__
