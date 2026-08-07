#ifndef __UIMANAGER_H__
#define __UIMANAGER_H__

#pragma once
namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CControlUI;
	class CRichEditUI;
	class CIDropTarget;
	class IRenderContext;
	class IRenderSurface;

	/////////////////////////////////////////////////////////////////////////////////////
	//
	enum UILIB_RESTYPE
	{
		UILIB_FILE=1,		// 来自磁盘文件
		UILIB_ZIP,			// 来自磁盘zip压缩包
		UILIB_RESOURCE,		// 来自资源
		UILIB_ZIPRESOURCE,	// 来自资源的zip压缩包
	};

	// 控件/窗口行为（对齐 EZUI 的 action 属性；html { action: title } 亦用此枚举）
	enum UIAction
	{
		UIACTION_NONE = 0,
		UIACTION_CLOSE,      // 关闭窗口
		UIACTION_MIN,        // 最小化
		UIACTION_MAX,        // 最大化/还原切换
		UIACTION_TITLE,      // 标题栏：拖动 + 双击最大化
		UIACTION_MOVEWINDOW, // 拖动窗口
		UIACTION_COPY,       // 复制文本到剪贴板
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	typedef enum EVENTTYPE_UI
	{
		UIEVENT__FIRST = 1,
		UIEVENT__KEYBEGIN,
		UIEVENT_KEYDOWN,
		UIEVENT_KEYUP,
		UIEVENT_CHAR,
		UIEVENT_SYSKEY,
		UIEVENT__KEYEND,
		UIEVENT__MOUSEBEGIN,
		UIEVENT_MOUSEMOVE,
		UIEVENT_MOUSELEAVE,
		UIEVENT_MOUSEENTER,
		UIEVENT_MOUSEHOVER,
		UIEVENT_BUTTONDOWN,
		UIEVENT_BUTTONUP,
		UIEVENT_RBUTTONDOWN,
		UIEVENT_RBUTTONUP,
		UIEVENT_MBUTTONDOWN,
		UIEVENT_MBUTTONUP,
		UIEVENT_DBLCLICK,
		UIEVENT_CONTEXTMENU,
		UIEVENT_SCROLLWHEEL,
		UIEVENT__MOUSEEND,
		UIEVENT_KILLFOCUS,
		UIEVENT_SETFOCUS,
		UIEVENT_WINDOWSIZE,
		UIEVENT_SETCURSOR,
		UIEVENT_TIMER,
		UIEVENT__LAST,
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//
	// 内部保留的消息
	typedef enum MSGTYPE_UI
	{
		UIMSG_TRAYICON = WM_USER + 1,// 托盘消息
		UIMSG_SET_DPI,				 // DPI
		WM_MENUCLICK,				 // 菜单消息
		UIMSG_USER = WM_USER + 100,	 // 程序自定义消息
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	// Flags for CControlUI::GetControlFlags()
#define UIFLAG_TABSTOP       0x00000001
#define UIFLAG_SETCURSOR     0x00000002
#define UIFLAG_WANTRETURN    0x00000004

	// Flags for FindControl()
#define UIFIND_ALL           0x00000000
#define UIFIND_VISIBLE       0x00000001
#define UIFIND_ENABLED       0x00000002
#define UIFIND_HITTEST       0x00000004
#define UIFIND_UPDATETEST    0x00000008
#define UIFIND_TOP_FIRST     0x00000010
#define UIFIND_ME_FIRST      0x80000000

	// Flags used for controlling the paint
#define UISTATE_FOCUSED      0x00000001
#define UISTATE_SELECTED     0x00000002
#define UISTATE_DISABLED     0x00000004
#define UISTATE_HOT          0x00000008
#define UISTATE_PUSHED       0x00000010
#define UISTATE_READONLY     0x00000020
#define UISTATE_CAPTURED     0x00000040

	/////////////////////////////////////////////////////////////////////////////////////
	//

	// 渲染资源后端标记（TImageInfo / TFontInfo 的 pBackend）
	enum RENDER_BACKEND
	{
		RENDER_BACKEND_GDI = 0,
		RENDER_BACKEND_D2D = 1,
		RENDER_BACKEND_SKIA = 2,
	};

	typedef struct UILIB_API tagTFontInfo
	{
		HFONT hFont;
		CDuiString sFontName;
		int iSize;
		bool bBold;
		bool bUnderline;
		bool bItalic;
		bool bStrikeout;
		TEXTMETRIC tm;
		// 非 GDI 后端字体句柄（如 IDWriteTextFormat* / SkFont*）；GDI 时为 NULL
		void* pBackend;
		int nBackend;
	} TFontInfo;

	typedef struct UILIB_API tagTImageInfo
	{
		tagTImageInfo();
		Gdiplus::Image* pImage;
		HBITMAP hBitmap;
		LPBYTE pBits;
		LPBYTE pSrcBits;
		int nX;
		int nY;
		bool bAlpha;
		bool bUseHSL;
		CDuiString sResType;
		DWORD dwMask;
		// 非 GDI 后端纹理（如 ID2D1Bitmap* / SkImage*）；GDI 时为 NULL，走 hBitmap
		void* pBackend;
		int nBackend;
	} TImageInfo;

	typedef struct UILIB_API tagTDrawInfo
	{
		tagTDrawInfo();
		void Parse(LPCTSTR pStrImage, LPCTSTR pStrModify, CPaintManagerUI *pManager);
		void Clear();

		CDuiString sDrawString;
		CDuiString sDrawModify;
		CDuiString sImageName;
		CDuiString sResType;
		RECT rcDest;
		RECT rcSource;
		RECT rcCorner;
		DWORD dwMask;
		UINT uFade;
		UINT uRotate;
		bool bHole;
		bool bTiledX;
		bool bTiledY;
		bool bHSL;
		bool bGdiplus;

		CDuiSize szImage;
		RECT rcPadding;
		CDuiString sAlign;
	} TDrawInfo;

	typedef struct UILIB_API tagTPercentInfo
	{
		double left;
		double top;
		double right;
		double bottom;
	} TPercentInfo;

	typedef struct UILIB_API tagTResInfo
	{
		DWORD m_dwDefaultDisabledColor;
		DWORD m_dwDefaultFontColor;
		DWORD m_dwDefaultLinkFontColor;
		DWORD m_dwDefaultLinkHoverFontColor;
		DWORD m_dwDefaultSelectedBackgroundColor;
		TFontInfo m_DefaultFontInfo;
		CStdStringPtrMap m_CustomFonts;
		CStdStringPtrMap m_ImageHash;
		CStdStringPtrMap m_AttrHash;
		CStdStringPtrMap m_StyleHash;
		CStdStringPtrMap m_DrawInfoHash;
		CStdStringPtrMap m_CssTypeRules;
		CStdStringPtrMap m_CssIdRules;
	} TResInfo;

	// Structure for notifications from the system
	// to the control implementation.
	enum EventPhase
	{
		PHASE_CAPTURE = 0,
		PHASE_TARGET,
		PHASE_BUBBLE,
	};

	typedef struct UILIB_API tagTEventUI
	{
		int Type;
		CControlUI* pSender;
		CControlUI* pCurrentTarget;
		DWORD dwTimestamp;
		POINT ptMouse;
		TCHAR chKey;
		WORD wKeyState;
		WPARAM wParam;
		LPARAM lParam;
		bool bPropagationStopped;
		bool bDefaultPrevented;
		EventPhase ePhase;

		void StopPropagation() { bPropagationStopped = true; }
		bool IsPropagationStopped() const { return bPropagationStopped; }
		void PreventDefault() { bDefaultPrevented = true; }
		bool IsDefaultPrevented() const { return bDefaultPrevented; }
	} TEventUI;

	// Drag&Drop control
	const TCHAR* const CF_MOVECONTROL = _T("CF_MOVECONTROL");

	typedef struct UILIB_API tagTCFMoveUI
	{
		CControlUI* pControl;
	} TCFMoveUI;

	// Listener interface
	class INotifyUI
	{
	public:
		virtual void Notify(TNotifyUI& msg) = 0;
	};

	// MessageFilter interface
	class IMessageFilterUI
	{
	public:
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;
	};

	class ITranslateAccelerator
	{
	public:
		virtual LRESULT TranslateAccelerator(MSG *pMsg) = 0;
	};

	class IDragDropUI
	{
	public:
		virtual bool OnDragDrop(CControlUI* pControl) { return false; }
	};
	/////////////////////////////////////////////////////////////////////////////////////
	//
	typedef CControlUI* (*LPCREATECONTROL)(LPCTSTR pstrType);

	class UILIB_API CPaintManagerUI : public CIDropTarget
	{
	public:
		CPaintManagerUI();
		~CPaintManagerUI();

	public:
		void Init(HWND hWnd, LPCTSTR pstrName = NULL);
		bool IsUpdateNeeded() const;
		void NeedUpdate();
		void Invalidate();
		void Invalidate(RECT& rcItem);

		LPCTSTR GetName() const;
		HDC GetPaintDC() const;
		IRenderContext* GetRenderContext() const;
		IRenderSurface* GetOffscreenSurface() const { return m_pOffscreenSurface; }
		IRenderSurface* GetBackgroundSurface() const { return m_pBackgroundSurface; }
		void SetRenderContext(IRenderContext* pRenderContext);
		HWND GetPaintWindow() const;
		HWND GetTooltipWindow() const;
		int GetHoverTime() const;
		void SetHoverTime(int iTime);

		POINT GetMousePos() const;
		SIZE GetClientSize() const;
		SIZE GetInitSize();
		void SetInitSize(int cx, int cy);
		RECT GetSizeBox();
		void SetSizeBox(RECT& rcSizeBox);
		RECT GetCaptionRect();
		void SetCaptionRect(RECT& rcCaption);
		SIZE GetBorderRadius();
		void SetBorderRadius(int cx, int cy);
		SIZE GetMinSize();
		void SetMinSize(int cx, int cy);
		SIZE GetMaxSize();
		void SetMaxSize(int cx, int cy);
		bool IsShowUpdateRect();
		void SetShowUpdateRect(bool show);
		bool IsNoActivate();
		void SetNoActivate(bool bNoActivate);

		BYTE GetOpacity() const;
		void SetOpacity(BYTE nOpacity);

		// html/Window 默认背景落到 root background-color（默认 #FFF0F0F0）；分层未显式设置则保持透明
		DWORD GetWindowBackgroundColor() const;
		void SetWindowBackgroundColor(DWORD dwColor);
		bool IsWindowBackgroundColorCustom() const;

		// html/Window 的 action（如 title）；Attach 后落到 root（root 已有 action 则不覆盖）
		UIAction GetWindowAction() const;
		void SetWindowAction(UIAction action);
		/// html/Window 上的 theme / theme-id（Attach 后落到 root，见 ApplyDefaultWindowTheme）
		LPCTSTR GetWindowTheme() const;
		void SetWindowTheme(LPCTSTR pstrTheme);
		LPCTSTR GetWindowThemeId() const;
		void SetWindowThemeId(LPCTSTR pstrThemeId);

		bool IsLayered();
		void SetLayered(bool bLayered);
		/// 分层 Present：默认 true 走 DComp；弹窗可关，强制 BitmapRT+ULW（避免首帧圆角锯齿要拖一下才好）
		void SetLayeredCompositionEnabled(bool bEnable);
		bool IsLayeredCompositionEnabled() const;
		RECT& GetLayeredPadding();
		void SetLayeredPadding(RECT& rcLayeredPadding);
		BYTE GetLayeredOpacity();
		void SetLayeredOpacity(BYTE nOpacity);
		LPCTSTR GetLayeredImage();
		void SetLayeredImage(LPCTSTR pstrImage);

		CShadowUI* GetShadow();

		void SetUseGdiplusText(bool bUse);
		bool IsUseGdiplusText() const;
		void SetGdiplusTextRenderingHint(int trh);
		int GetGdiplusTextRenderingHint() const;

		static HINSTANCE GetInstance();
		static CDuiString GetInstancePath();
		static CDuiString GetCurrentPath();
		static HINSTANCE GetResourceDll();
		static const CDuiString& GetResourcePath();
		static const CDuiString& GetResourceZip();
		static const CDuiString& GetResourceZipPwd();
		static bool IsCachedResourceZip();
		static HANDLE GetResourceZipHandle();
		static void SetInstance(HINSTANCE hInst);
		static void SetCurrentPath(LPCTSTR pStrPath);
		static void SetResourceDll(HINSTANCE hInst);
		static void SetResourcePath(LPCTSTR pStrPath);
		static void SetResourceZip(LPVOID pVoid, unsigned int len, LPCTSTR password = NULL);
		static void SetResourceZip(LPCTSTR pstrZip, bool bCachedResourceZip = false, LPCTSTR password = NULL);
		/// 读相对资源：优先 ResourcePath（skin），找不到再读 ZIP/ZIPRESOURCE，最后试绝对路径。
		/// 成功时 *ppData 为 new BYTE[]，调用方负责 delete[]；失败返回 false 且 *ppData=NULL。
		static bool LoadResourceData(LPCTSTR pstrRelativePath, BYTE** ppData, DWORD* pdwSize);
		static void SetResourceType(int nType);
		static int GetResourceType();
		static bool GetHSL(short* H, short* S, short* L);
		static void SetHSL(bool bUseHSL, short H, short S, short L); // H:0~360, S:0~200, L:0~200 
		static void ReloadSkin();
		static CPaintManagerUI* GetPaintManager(LPCTSTR pstrName);
		static CStdPtrArray* GetPaintManagers();
		static bool LoadPlugin(LPCTSTR pstrModuleName);
		static CStdPtrArray* GetPlugins();

		bool IsForceUseSharedRes() const;
		void SetForceUseSharedRes(bool bForce);
		// 注意：只支持简单类型指针，因为只释放内存，不会调用类对象的析构函数
		void DeletePtr(void* ptr);

		DWORD GetDefaultDisabledColor() const;
		void SetDefaultDisabledColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultFontColor() const;
		void SetDefaultFontColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultLinkFontColor() const;
		void SetDefaultLinkFontColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultLinkHoverFontColor() const;
		void SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultSelectedBackgroundColor() const;
		void SetDefaultSelectedBackgroundColor(DWORD dwColor, bool bShared = false);
		TFontInfo* GetDefaultFontInfo();
		void SetDefaultFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		DWORD GetCustomFontCount(bool bShared = false) const;
		void AddFontArray(LPCTSTR pstrPath);
		HFONT AddFont(int id, LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		/// 按家族/字号查找或创建字体，返回字体表内部 id
		int EnsureFont(LPCTSTR pStrFontName, int nSize, bool bBold = false, bool bUnderline = false, bool bItalic = false, bool bStrikeout = false, bool bShared = false);
		HFONT GetFont(int id);
		HFONT GetFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout);
		int GetFontIndex(HFONT hFont, bool bShared = false);
		int GetFontIndex(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		void RemoveFont(HFONT hFont, bool bShared = false);
		void RemoveFont(int id, bool bShared = false);
		void RemoveAllFonts(bool bShared = false);
		TFontInfo* GetFontInfo(int id);
		TFontInfo* GetFontInfo(HFONT hFont);

		const TImageInfo* GetImage(LPCTSTR bitmap);
		const TImageInfo* GetImageEx(LPCTSTR bitmap, LPCTSTR type = NULL, DWORD mask = 0, bool bUseHSL = false, bool bGdiplus = false, HINSTANCE instance = NULL);
		const TImageInfo* AddImage(LPCTSTR bitmap, LPCTSTR type = NULL, DWORD mask = 0, bool bUseHSL = false, bool bGdiplus = false, bool bShared = false, HINSTANCE instance = NULL);
		const TImageInfo* AddImage(LPCTSTR bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared = false);
		void RemoveImage(LPCTSTR bitmap, bool bShared = false);
		void RemoveAllImages(bool bShared = false);
		static void ReloadSharedImages();
		void ReloadImages();

		const TDrawInfo* GetDrawInfo(LPCTSTR pStrImage, LPCTSTR pStrModify);
		void RemoveDrawInfo(LPCTSTR pStrImage, LPCTSTR pStrModify);
		void RemoveAllDrawInfos();

		void AddDefaultAttributeList(LPCTSTR pStrControlName, LPCTSTR pStrControlAttrList, bool bShared = false);
		LPCTSTR GetDefaultAttributeList(LPCTSTR pStrControlName) const;
		bool RemoveDefaultAttributeList(LPCTSTR pStrControlName, bool bShared = false);
		void RemoveAllDefaultAttributeList(bool bShared = false);

		void AddWindowCustomAttribute(LPCTSTR pstrName, LPCTSTR pstrAttr);
		LPCTSTR GetWindowCustomAttribute(LPCTSTR pstrName) const;
		bool RemoveWindowCustomAttribute(LPCTSTR pstrName);
		void RemoveAllWindowCustomAttribute();

		// 样式管理
		void AddStyle(LPCTSTR pName, LPCTSTR pStyle, bool bShared = false);
		LPCTSTR GetStyle(LPCTSTR pName) const;
		BOOL RemoveStyle(LPCTSTR pName, bool bShared = false);
		const CStdStringPtrMap& GetStyles(bool bShared = false) const;
		void RemoveAllStyle(bool bShared = false);

		// CSS style块管理
		void AddCssRule(LPCTSTR pstrSelector, LPCTSTR pstrAttrList);
		LPCTSTR GetCssTypeRule(LPCTSTR pstrType) const;
		LPCTSTR GetCssIdRule(LPCTSTR pstrId) const;
		void RemoveAllCssRules();

		const TImageInfo* GetImageString(LPCTSTR pStrImage, LPCTSTR pStrModify = NULL);

		// 初始化拖拽
		bool EnableDragDrop(bool bEnable);
		void SetDragDrop(IDragDropUI* pDragDrop);
		virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect);

		bool IsValid();
		bool AttachDialog(CControlUI* pControl);
		bool InitControls(CControlUI* pControl, CControlUI* pParent = NULL);
		void ReapObjects(CControlUI* pControl);

		bool AddOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl);
		CStdPtrArray* GetOptionGroup(LPCTSTR pStrGroupName);
		void RemoveOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl);
		void RemoveAllOptionGroups();

		CControlUI* GetFocus() const;
		void SetFocus(CControlUI* pControl);
		void SetFocusNeeded(CControlUI* pControl);

		bool SetNextTabControl(bool bForward = true);

		bool SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse);
		bool KillTimer(CControlUI* pControl, UINT nTimerID);
		void KillTimer(CControlUI* pControl);
		void RemoveAllTimers();

		void SetCapture();
		void ReleaseCapture();
		bool IsCaptured();

		bool IsPainting();
		void SetPainting(bool bIsPainting);

		bool AddNotifier(INotifyUI* pControl);
		bool RemoveNotifier(INotifyUI* pControl);   
		void SendNotify(TNotifyUI& Msg, bool bAsync = false);
		void SendNotify(CControlUI* pControl, LPCTSTR pstrMessage, WPARAM wParam = 0, LPARAM lParam = 0, bool bAsync = false);

		bool AddPreMessageFilter(IMessageFilterUI* pFilter);
		bool RemovePreMessageFilter(IMessageFilterUI* pFilter);

		bool AddMessageFilter(IMessageFilterUI* pFilter);
		bool RemoveMessageFilter(IMessageFilterUI* pFilter);

		int GetPostPaintCount() const;
		bool IsPostPaint(CControlUI* pControl);
		bool AddPostPaint(CControlUI* pControl);
		bool RemovePostPaint(CControlUI* pControl);
		bool SetPostPaintIndex(CControlUI* pControl, int iIndex);

		int GetNativeWindowCount() const;
		RECT GetNativeWindowRect(HWND hChildWnd);
		bool AddNativeWindow(CControlUI* pControl, HWND hChildWnd);
		bool RemoveNativeWindow(HWND hChildWnd);

		void AddDelayedCleanup(CControlUI* pControl);
		void AddMouseLeaveNeeded(CControlUI* pControl);
		bool RemoveMouseLeaveNeeded(CControlUI* pControl);

		bool AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
		bool RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
		bool TranslateAccelerator(LPMSG pMsg);

		CControlUI* GetRoot() const;
		CControlUI* FindControl(POINT pt) const;
		CControlUI* FindControl(LPCTSTR pstrName) const;
		CControlUI* FindSubControlByPoint(CControlUI* pParent, POINT pt) const;
		CControlUI* FindSubControlByName(CControlUI* pParent, LPCTSTR pstrName) const;
		CControlUI* FindSubControlByClass(CControlUI* pParent, LPCTSTR pstrClass, int iIndex = 0);
		CStdPtrArray* FindSubControlsByClass(CControlUI* pParent, LPCTSTR pstrClass);

		static void MessageLoop();
		static bool TranslateMessage(const LPMSG pMsg);
		static void Term();

		CDPI* GetDPIObj();
		void ResetDPIAssets();
		void RebuildFont(TFontInfo* pFontInfo);
		// bResizeWindow：按新旧 scale 比例调整窗口；系统 WM_DPICHANGED 应传 false，改用系统建议矩形
		void SetDPI(int iDPI, bool bResizeWindow = true);
		static void SetAllDPI(int iDPI);

		bool MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
		bool PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
		void UsedVirtualWnd(bool bUsed);

	private:
		CStdPtrArray* GetFoundControls();
		static CControlUI* CALLBACK __FindControlFromNameHash(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromCount(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromPoint(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromTab(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromShortcut(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromName(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromClass(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlsFromClass(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlsFromUpdate(CControlUI* pThis, LPVOID pData);

		static void AdjustSharedImagesHSL();
		void AdjustImagesHSL();
		void PostAsyncNotify();
		void ApplyDefaultWindowBackgroundColor();
		void ApplyDefaultWindowAction();
		void ApplyDefaultWindowTheme();

	private:
		CDuiString m_sName;
		HWND m_hWndPaint;	//所附加的窗体的句柄
		HDC m_hDcPaint;
		IRenderContext* m_pRenderContext;
		IRenderSurface* m_pOffscreenSurface;
		IRenderSurface* m_pBackgroundSurface;

		// 提示信息
		HWND m_hwndTooltip;
		TOOLINFO m_ToolTip;
		int m_iHoverTime;
		bool m_bNoActivate;
		bool m_bShowUpdateRect;

		//
		CControlUI* m_pRoot;
		CControlUI* m_pFocus;
		CControlUI* m_pEventHover;
        CControlUI* m_pEventClick;
        CControlUI* m_pEventRClick;
		CControlUI* m_pEventKey;
		CControlUI* m_pLastToolTip;
		//
		POINT m_ptLastMousePos;
		SIZE m_szMinWindow;
		SIZE m_szMaxWindow;
		SIZE m_szInitWindowSize;
		RECT m_rcSizeBox;
		SIZE m_szBorderRadius;
		RECT m_rcCaption;
		UINT m_uTimerID;
		bool m_bFirstLayout;
		bool m_bUpdateNeeded;
		bool m_bFocusNeeded;
		bool m_bOffscreenPaint;
		
		BYTE m_nOpacity;
		DWORD m_dwWindowBackgroundColor;
		bool m_bWindowBackgroundColorCustom;
		UIAction m_windowAction;
		CDuiString m_sWindowTheme;
		CDuiString m_sWindowThemeId;
		bool m_bLayered;
		bool m_bLayeredCompositionEnabled;
		RECT m_rcLayeredPadding;
		bool m_bLayeredChanged;
		RECT m_rcLayeredUpdate;
		TDrawInfo m_diLayered;

		bool m_bMouseTracking;
		bool m_bMouseCapture;
		bool m_bIsPainting;
		bool m_bUsedVirtualWnd;
		bool m_bAsyncNotifyPosted;

		//
		CStdPtrArray m_aNotifiers;
		CStdPtrArray m_aTimers;
		CStdPtrArray m_aTranslateAccelerator;
		CStdPtrArray m_aPreMessageFilters;
		CStdPtrArray m_aMessageFilters;
		CStdPtrArray m_aPostPaintControls;
		CStdPtrArray m_aNativeWindow;
		CStdPtrArray m_aNativeWindowControl;
		CStdPtrArray m_aDelayedCleanup;
		CStdPtrArray m_aAsyncNotify;
		CStdPtrArray m_aFoundControls;
		CStdPtrArray m_aFonts;
		CStdPtrArray m_aNeedMouseLeaveNeeded;
		CStdStringPtrMap m_mNameHash;
		CStdStringPtrMap m_mWindowCustomAttrHash;
		CStdStringPtrMap m_mOptionGroup;
		
		bool m_bForceUseSharedRes;
		TResInfo m_ResInfo;
		
		// 窗口阴影
		CShadowUI m_shadow;
		
		// DPI管理器
		CDPI* m_pDPI;
		// 是否开启Gdiplus
		bool m_bUseGdiplusText;
		int m_trh;
		ULONG_PTR m_gdiplusToken;
		Gdiplus::GdiplusStartupInput *m_pGdiplusStartupInput;

		// 拖拽
		bool m_bDragDrop;
		bool m_bDragMode;
		HBITMAP m_hDragBitmap;
		IDragDropUI *m_pDragDrop;
		//
		static HINSTANCE m_hInstance;
		static HINSTANCE m_hResourceInstance;
		static CDuiString m_pStrResourcePath;
		static CDuiString m_pStrResourceZip;
		static CDuiString m_pStrResourceZipPwd;
		static HANDLE m_hResourceZip;
        static BYTE* m_cbZipBuf;

		static bool m_bCachedResourceZip;
		static int m_nResType;
		static TResInfo m_SharedResInfo;
		static bool m_bUseHSL;
		static short m_H;
		static short m_S;
		static short m_L;
		static CStdPtrArray m_aPreMessages;
		static CStdPtrArray m_aPlugins;
	};

} // namespace DuiLib

#endif // __UIMANAGER_H__
