#pragma once

namespace DuiLib
{
#define MAX_FONT_ID		30000
#define CARET_TIMERID	0x1999

	// 列表类型
	enum ListType
	{
		LT_LIST = 0,
		LT_COMBO,
		LT_TREE,
		LT_MENU,
	};

	// 鼠标光标定义
#define DUI_ARROW           32512
#define DUI_IBEAM           32513
#define DUI_WAIT            32514
#define DUI_CROSS           32515
#define DUI_UPARROW         32516
#define DUI_SIZE            32640
#define DUI_ICON            32641
#define DUI_SIZENWSE        32642
#define DUI_SIZENESW        32643
#define DUI_SIZEWE          32644
#define DUI_SIZENS          32645
#define DUI_SIZEALL         32646
#define DUI_NO              32648
#define DUI_HAND            32649

	// 消息类型
	enum DuiSig
	{
		DuiSig_end = 0, // [marks end of message map]
		DuiSig_lwl,     // LRESULT (WPARAM, LPARAM)
		DuiSig_vn,      // void (TNotifyUI)
	};

	// 核心控件
	class CControlUI;

	// Structure for notifications to the outside world
	typedef struct tagTNotifyUI
	{
		CDuiString sType;
		CDuiString sVirtualWnd;
		CControlUI* pSender;
		DWORD dwTimestamp;
		POINT ptMouse;      // 客户端坐标（SendNotify 自动填 m_ptLastMousePos）
		POINT ptScreen;     // 屏幕坐标（SendNotify 自动由 ptMouse 经 ClientToScreen 求得，统一可信）
		WPARAM wParam;
		LPARAM lParam;
	} TNotifyUI;

	class CNotifyPump;
	typedef void (CNotifyPump::*DUI_PMSG)(TNotifyUI& msg);  //指针类型

	union DuiMessageMapFunctions
	{
		DUI_PMSG pfn;   // generic member function pointer
		LRESULT(CNotifyPump::*pfn_Notify_lwl)(WPARAM, LPARAM);
		void (CNotifyPump::*pfn_Notify_vn)(TNotifyUI&);
	};

	//定义所有消息类型
	//////////////////////////////////////////////////////////////////////////

#define DUI_MSGTYPE_MENU                   (_T("menu"))
#define DUI_MSGTYPE_LINK                   (_T("link"))

#define DUI_MSGTYPE_TIMER                  (_T("timer"))
#define DUI_MSGTYPE_CLICK                  (_T("click"))
#define DUI_MSGTYPE_DBCLICK                (_T("dbclick"))

#define DUI_MSGTYPE_RETURN                 (_T("return"))
#define DUI_MSGTYPE_SCROLL                 (_T("scroll"))

#define DUI_MSGTYPE_PREDROPDOWN            (_T("predropdown"))
#define DUI_MSGTYPE_DROPDOWN               (_T("dropdown"))
#define DUI_MSGTYPE_SETFOCUS               (_T("setfocus"))

#define DUI_MSGTYPE_KILLFOCUS              (_T("killfocus"))
#define DUI_MSGTYPE_ITEMCLICK 		   	   (_T("itemclick"))
#define DUI_MSGTYPE_ITEMRCLICK 			   (_T("itemrclick"))
#define DUI_MSGTYPE_TABSELECT              (_T("tabselect"))
#define DUI_MSGTYPE_TABSELECTING           (_T("tabselecting"))
#define DUI_MSGTYPE_TABCLOSE               (_T("tabclose"))
#define DUI_MSGTYPE_TABCLOSING             (_T("tabclosing"))
#define DUI_MSGTYPE_TABMOVE                (_T("tabmove"))
#define DUI_MSGTYPE_TABADD                 (_T("tabadd"))

#define DUI_MSGTYPE_TITLEBARMIN            (_T("titlebarmin"))
#define DUI_MSGTYPE_TITLEBARMINING         (_T("titlebarmining"))
#define DUI_MSGTYPE_TITLEBARMAX            (_T("titlebarmax"))
#define DUI_MSGTYPE_TITLEBARMAXING         (_T("titlebarmaxing"))
#define DUI_MSGTYPE_TITLEBARCLOSE          (_T("titlebarclose"))
#define DUI_MSGTYPE_TITLEBARCLOSING        (_T("titlebarclosing"))

#define DUI_MSGTYPE_ITEMSELECT 		   	   (_T("itemselect"))
#define DUI_MSGTYPE_ITEMEXPAND             (_T("itemexpand"))
#define DUI_MSGTYPE_WINDOWINIT             (_T("windowinit"))
#define DUI_MSGTYPE_WINDOWSIZE             (_T("windowsize"))
#define DUI_MSGTYPE_BUTTONDOWN 		   	   (_T("buttondown"))
#define DUI_MSGTYPE_MOUSEENTER			   (_T("mouseenter"))
#define DUI_MSGTYPE_MOUSELEAVE			   (_T("mouseleave"))

#define DUI_MSGTYPE_TEXTCHANGED            (_T("textchanged"))
#define DUI_MSGTYPE_HISTORYCHANGED         (_T("historychanged")) ///< EditBox 历史增删改；wParam=DUI_HISTORYCHANGE_*
#define DUI_HISTORYCHANGE_ADD              1
#define DUI_HISTORYCHANGE_REMOVE           2
#define DUI_HISTORYCHANGE_CLEAR            3
#define DUI_HISTORYCHANGE_SET              4
#define DUI_MSGTYPE_HEADERCLICK            (_T("headerclick"))
#define DUI_MSGTYPE_ITEMDBCLICK            (_T("itemdbclick"))
#define DUI_MSGTYPE_SHOWACTIVEX            (_T("showactivex"))

#define DUI_MSGTYPE_ITEMCOLLAPSE           (_T("itemcollapse"))
#define DUI_MSGTYPE_ITEMACTIVATE           (_T("itemactivate"))
#define DUI_MSGTYPE_VALUECHANGED           (_T("valuechanged"))
#define DUI_MSGTYPE_VALUECHANGED_MOVE      (_T("movevaluechanged"))

#define DUI_MSGTYPE_SELECTCHANGED 		   (_T("selectchanged"))
#define DUI_MSGTYPE_UNSELECTED	 		   (_T("unselected"))

#define DUI_MSGTYPE_TREEITEMDBCLICK 		(_T("treeitemdbclick"))
#define DUI_MSGTYPE_CHECKCLICK				(_T("checkclick"))
#define DUI_MSGTYPE_TEXTROLLEND 			(_T("textrollend"))
#define DUI_MSGTYPE_COLORCHANGING		    (_T("colorchanging"))
#define DUI_MSGTYPE_COLORCHANGED		    (_T("colorchanged"))
#define DUI_MSGTYPE_THEMEFILESAVED		    (_T("themefilesaved"))

#define DUI_MSGTYPE_LISTITEMSELECT 		   	(_T("listitemselect"))
#define DUI_MSGTYPE_LISTITEMCHECKED 		(_T("listitemchecked"))
#define DUI_MSGTYPE_COMBOITEMSELECT 		(_T("comboitemselect"))
#define DUI_MSGTYPE_LISTHEADERCLICK			(_T("listheaderclick"))
#define DUI_MSGTYPE_LISTHEADITEMCHECKED		(_T("listheaditemchecked"))
#define DUI_MSGTYPE_LISTPAGECHANGED			(_T("listpagechanged"))

#define DUI_MSGTYPE_PAGECHANED				(_T("page_selected_changed"))
#define DUI_MSGTYPE_PAGECHANGED				(_T("pagechanged"))
#define DUI_MSGTYPE_ITEMMOVED				(_T("itemmoved"))
#define DUI_MSGTYPE_FILTERCHANGED			(_T("filterchanged"))
#define DUI_MSGTYPE_DRAGBEGIN				(_T("dragbegin"))
#define DUI_MSGTYPE_DRAGEND					(_T("dragend"))
#define DUI_MSGTYPE_TOOLCARDOPEN			(_T("toolcardopen"))
#define DUI_MSGTYPE_SLIDECHANGED			(_T("slidechanged"))
#define DUI_MSGTYPE_SIDEPANELOPEN			(_T("sidepanelopen"))
#define DUI_MSGTYPE_SIDEPANELCLOSE			(_T("sidepanelclose"))


	//////////////////////////////////////////////////////////////////////////

	struct DUI_MSGMAP_ENTRY;
	struct DUI_MSGMAP
	{
#ifndef UILIB_STATIC
		const DUI_MSGMAP* (PASCAL* pfnGetBaseMap)();
#else
		const DUI_MSGMAP* pBaseMap;
#endif
		const DUI_MSGMAP_ENTRY* lpEntries;
	};

	//结构定义
	struct DUI_MSGMAP_ENTRY //定义一个结构体，来存放消息信息
	{
		CDuiString sMsgType;          // DUI消息类型
		CDuiString sCtrlName;         // 控件名称
		UINT       nSig;              // 标记函数指针类型
		DUI_PMSG   pfn;               // 指向函数的指针
	};

	//定义
#ifndef UILIB_STATIC
#define DUI_DECLARE_MESSAGE_MAP()                                         \
private:                                                                  \
	static const DUI_MSGMAP_ENTRY _messageEntries[];                      \
protected:                                                                \
	static const DUI_MSGMAP messageMap;                                   \
	static const DUI_MSGMAP* PASCAL _GetBaseMessageMap();                 \
	virtual const DUI_MSGMAP* GetMessageMap() const;                      \

#else
#define DUI_DECLARE_MESSAGE_MAP()                                         \
private:                                                                  \
	static const DUI_MSGMAP_ENTRY _messageEntries[];                      \
protected:                                                                \
	static  const DUI_MSGMAP messageMap;				                  \
	virtual const DUI_MSGMAP* GetMessageMap() const;                      \

#endif


	//基类声明开始
#ifndef UILIB_STATIC
#define DUI_BASE_BEGIN_MESSAGE_MAP(theClass)                              \
	const DUI_MSGMAP* PASCAL theClass::_GetBaseMessageMap()               \
	{ return NULL; }                                                  \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{  &theClass::_GetBaseMessageMap, &theClass::_messageEntries[0] };\
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#else
#define DUI_BASE_BEGIN_MESSAGE_MAP(theClass)                              \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{  NULL, &theClass::_messageEntries[0] };                         \
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#endif


	//子类声明开始
#ifndef UILIB_STATIC
#define DUI_BEGIN_MESSAGE_MAP(theClass, baseClass)                        \
	const DUI_MSGMAP* PASCAL theClass::_GetBaseMessageMap()               \
	{ return &baseClass::messageMap; }                                \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{ &theClass::_GetBaseMessageMap, &theClass::_messageEntries[0] }; \
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#else
#define DUI_BEGIN_MESSAGE_MAP(theClass, baseClass)                        \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{ &baseClass::messageMap, &theClass::_messageEntries[0] };        \
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#endif


	//声明结束
#define DUI_END_MESSAGE_MAP()                                             \
	{ _T(""), _T(""), DuiSig_end, (DUI_PMSG)0 }                           \
	};                                                                        \


	//定义消息类型--执行函数宏
#define DUI_ON_MSGTYPE(msgtype, memberFxn)                                \
	{ msgtype, _T(""), DuiSig_vn, (DUI_PMSG)&memberFxn},                  \


	//定义消息类型--控件名称--执行函数宏
#define DUI_ON_MSGTYPE_CTRNAME(msgtype,ctrname,memberFxn)                 \
	{ msgtype, ctrname, DuiSig_vn, (DUI_PMSG)&memberFxn },                \


	//定义click消息的控件名称--执行函数宏
#define DUI_ON_CLICK_CTRNAME(ctrname,memberFxn)                           \
	{ DUI_MSGTYPE_CLICK, ctrname, DuiSig_vn, (DUI_PMSG)&memberFxn },      \


	//定义selectchanged消息的控件名称--执行函数宏
#define DUI_ON_SELECTCHANGED_CTRNAME(ctrname,memberFxn)                   \
	{ DUI_MSGTYPE_SELECTCHANGED,ctrname,DuiSig_vn,(DUI_PMSG)&memberFxn }, \


	//定义killfocus消息的控件名称--执行函数宏
#define DUI_ON_KILLFOCUS_CTRNAME(ctrname,memberFxn)                       \
	{ DUI_MSGTYPE_KILLFOCUS,ctrname,DuiSig_vn,(DUI_PMSG)&memberFxn },     \


	//定义menu消息的控件名称--执行函数宏
#define DUI_ON_MENU_CTRNAME(ctrname,memberFxn)                            \
	{ DUI_MSGTYPE_MENU,ctrname,DuiSig_vn,(DUI_PMSG)&memberFxn },          \


	//定义与控件名称无关的消息宏

	//定义timer消息--执行函数宏
#define DUI_ON_TIMER()                                                    \
	{ DUI_MSGTYPE_TIMER, _T(""), DuiSig_vn,(DUI_PMSG)&OnTimer },          \


	///
	//////////////END消息映射宏定义////////////////////////////////////////////////////


//////////////BEGIN控件名称宏定义//////////////////////////////////////////////////
///
#define  DUI_CTR_BOX							 (_T("Box")) //

#define  DUI_CTR_EDIT                            (_T("Edit"))
#define  DUI_CTR_EDITBOX                         (_T("EditBox"))
#define  DUI_CTR_EDITBOXSLOT                     (_T("EditBoxSlot"))
#define  DUI_CTR_EDITBOXLEFT                     (_T("EditBoxLeft"))
#define  DUI_CTR_EDITBOXRIGHT                    (_T("EditBoxRight"))
#define  DUI_CTR_EDITBOXPREFIX                   (_T("EditBoxPrefix"))
#define  DUI_CTR_EDITBOXSUFFIX                   (_T("EditBoxSuffix"))
#define  DUI_CTR_SPIN                            (_T("Spin"))
#define  DUI_CTR_NUMBER                          (_T("Number"))
#define  DUI_CTR_SEGMENTED                       (_T("Segmented"))
#define  DUI_CTR_SEGMENTITEM                     (_T("SegmentItem"))
#define  DUI_CTR_TAG                             (_T("Tag"))
#define  DUI_CTR_BADGE                           (_T("Badge"))
#define  DUI_CTR_TRANSFER                        (_T("Transfer"))
#define  DUI_CTR_TRANSFERITEM                    (_T("TransferItem"))
#define  DUI_CTR_RATE                            (_T("Rate"))
#define  DUI_CTR_EMPTY                           (_T("Empty"))
#define  DUI_CTR_SKELETON                        (_T("Skeleton"))
#define  DUI_CTR_LIST                            (_T("List"))
#define  DUI_CTR_VIRTUALLIST                     (_T("VirtualList"))
#define  DUI_CTR_TEXT                            (_T("Text"))
#define  DUI_CTR_HBOX                            (_T("HBox"))
#define  DUI_CTR_VBOX                            (_T("VBox"))
#define  DUI_CTR_SPLITLAYOUT                     (_T("SplitLayout"))
#define  DUI_CTR_HSPLIT                          (_T("HSplit"))
#define  DUI_CTR_VSPLIT                          (_T("VSplit"))
#define  DUI_CTR_BODY                            (_T("body"))
#define  DUI_CTR_DIV                             (_T("div"))
#define  DUI_CTR_RING							 (_T("Ring"))

#define  DUI_CTR_COMBO                           (_T("Combo"))
#define  DUI_CTR_LOOKUPEDIT                      (_T("LookupEdit"))
#define  DUI_CTR_LOOKUPCOLUMN                    (_T("LookupColumn"))
#define  DUI_CTR_LABEL                           (_T("Label"))
#define  DUI_CTR_ROLLTEXT                        (_T("RollText"))
#define  DUI_CTR_MARQUEE                         (_T("Marquee"))
#define  DUI_CTR_FLASH							 (_T("Flash"))

#define  DUI_CTR_BUTTON                          (_T("Button"))
#define  DUI_CTR_SHAPEBUTTON                     (_T("ShapeButton"))
#define  DUI_CTR_SHAPEBOX                        (_T("ShapeBox"))
#define  DUI_CTR_THEMESWITCHER                   (_T("ThemeSwitcher"))
#define  DUI_CTR_ICONPICKER                       (_T("IconPicker"))
#define  DUI_CTR_OPTION                          (_T("Option"))
#define  DUI_CTR_SLIDER                          (_T("Slider"))
#define  DUI_CTR_TAB_BOX					     (_T("TabBox")) //

#define  DUI_CTR_SPACER                          (_T("Spacer"))
#define  DUI_CTR_ACCORDION                       (_T("Accordion"))
#define  DUI_CTR_ACCORDIONITEM                   (_T("AccordionItem"))
#define  DUI_CTR_CAROUSEL                        (_T("Carousel"))
#define  DUI_CTR_CAROUSELITEM                    (_T("CarouselItem"))
#define  DUI_CTR_SIDEPANEL                       (_T("SidePanel"))
#define  DUI_CTR_SVGBOX                          (_T("SvgBox"))
#define  DUI_CTR_TABBAR                          (_T("TabBar"))
#define  DUI_CTR_TABBUTTON                       (_T("TabButton"))
#define  DUI_CTR_TITLEBAR                        (_T("TitleBar"))
#define  DUI_CTR_TITLEBARLEFT                    (_T("TitleBarLeft"))
#define  DUI_CTR_TITLEBARSYS                     (_T("TitleBarSys"))
#define  DUI_CTR_CONTROL                         (_T("Control"))
#define  DUI_CTR_ACTIVEX                         (_T("ActiveX"))
#define  DUI_CTR_GIFANIM                         (_T("GifAnim"))
#define	 DUI_CTR_TILE_BOX						 (_T("TileBox")) //
#define  DUI_CTR_LOADINGCIRCLE					 (_T("Loading")) //

#define  DUI_CTR_LISTITEM                        (_T("ListItem"))
#define  DUI_CTR_PROGRESS                        (_T("Progress"))
#define  DUI_CTR_RICHEDIT                        (_T("RichEdit"))
#define  DUI_CTR_CHECKBOX                        (_T("CheckBox"))
#define  DUI_CTR_SWITCH                          (_T("Switch"))
#define  DUI_CTR_IMAGE                           (_T("Img"))
#define  DUI_CTR_AVATAR                          (_T("Avatar"))
#define  DUI_CTR_FONTICON                        (_T("FontIcon"))
#define  DUI_CTR_APPICON                         (_T("AppIcon"))
#define  DUI_CTR_APPGRID                         (_T("AppGrid"))
#define  DUI_CTR_TOOLCARD                        (_T("ToolCard"))
#define  DUI_CTR_TOOLCARDHEADER                  (_T("ToolCardHeader"))
#define  DUI_CTR_TOOLCARDBODY                    (_T("ToolCardBody"))
#define  DUI_CTR_COMBOBOX                        (_T("ComboBox"))
#define  DUI_CTR_DATETIME                        (_T("DateTime"))
#define  DUI_CTR_HOTKEY                          (_T("HotKey"))
#define  DUI_CTR_STEPS                           (_T("Steps"))
#define  DUI_CTR_STEPITEM                        (_T("StepItem"))
#define  DUI_CTR_TIMELINE                        (_T("Timeline"))
#define  DUI_CTR_TIMELINEITEM                    (_T("TimelineItem"))
#define  DUI_CTR_TREEVIEW                        (_T("TreeView"))
#define  DUI_CTR_TREENODE                        (_T("TreeNode"))
#define  DUI_CTR_CHILD_BOX					     (_T("ChildBox")) //

#define  DUI_CTR_CONTAINER                       (_T("Container"))
#define  DUI_CTR_TABLAYOUT                       (_T("TabLayout"))
#define  DUI_CTR_SCROLLBAR                       (_T("ScrollBar"))
#define  DUI_CTR_IPADDRESS                       (_T("IPAddress"))

#define  DUI_CTR_LISTHEADER                      (_T("ListHeader"))
#define  DUI_CTR_LISTFOOTER                      (_T("ListFooter"))
#define  DUI_CTR_TILELAYOUT                      (_T("TileLayout"))
#define  DUI_CTR_WEBBROWSER                      (_T("WebBrowser"))
#define  DUI_CTR_WEBVIEW2                        (_T("WebView2"))

#define  DUI_CTR_CHILDLAYOUT                     (_T("ChildLayout"))
#define  DUI_CTR_LISTELEMENT                     (_T("ListElement"))

#define  DUI_CTR_VERTICALLAYOUT                  (_T("VerticalLayout"))
#define  DUI_CTR_LISTHEADERITEM                  (_T("ListHeaderItem"))

#define  DUI_CTR_LISTTEXTELEMENT                 (_T("ListTextElement"))

#define  DUI_CTR_HORIZONTALLAYOUT                (_T("HorizontalLayout"))
#define  DUI_CTR_LISTLABELELEMENT                (_T("ListLabelElement"))

#define  DUI_CTR_ANIMATIONTABLAYOUT				 (_T("AnimationTabLayout"))

#define  DUI_CTR_LISTCONTAINERELEMENT            (_T("ListContainerElement"))

#define  DUI_CTR_TEXTSCROLL						 (_T("TextScroll"))

#define DUI_CTR_COLORPALETTE					  (_T("ColorPalette"))
	///
	//////////////END控件名称宏定义//////////////////////////////////////////////////

	}// namespace DuiLib

