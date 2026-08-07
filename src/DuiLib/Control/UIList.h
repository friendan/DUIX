#ifndef __UILIST_H__
#define __UILIST_H__

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	typedef int (CALLBACK *PULVCompareFunc)(UINT_PTR, UINT_PTR, UINT_PTR);

	class CListHeaderUI;

#define UILIST_MAX_COLUMNS 32

	typedef struct tagTListInfoUI
	{
		int nColumns;
		RECT rcColumn[UILIST_MAX_COLUMNS];
		int nFont;
		UINT uTextStyle;
		RECT rcTextPadding;
		DWORD dwColor;
		DWORD dwBackgroundColor;
		CDuiString sBkImage;
		bool bAlternateBk;
		DWORD dwAlternateBackgroundColor; // 奇数行；0=透明（露出列表底）
		DWORD dwSelectedColor;
		DWORD dwSelectedBackgroundColor;
		CDuiString sSelectedImage;
		DWORD dwHoverColor;
		DWORD dwHoverBackgroundColor;
		CDuiString sHoverImage;
		DWORD dwDisabledColor;
		DWORD dwDisabledBackgroundColor;
		CDuiString sDisabledImage;
		CDuiString sForegroundImage;
		CDuiString sHoverForegroundImage;
		CDuiString sSelectedForegroundImage;

		DWORD dwLineColor;
		bool bShowRowLine;
		bool bShowColumnLine;
		bool bShowHtml;
		bool bMultiExpandable;
		bool bRSelected;
	} TListInfoUI;


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class IListCallbackUI
	{
	public:
		virtual LPCTSTR GetItemText(CControlUI* pList, int iItem, int iSubItem) = 0;
		virtual DWORD GetItemColor(CControlUI* pList, int iItem, int iSubItem, int iState) = 0;// iState：0-正常、1-激活、2-选择、3-禁用
	};

	class IListOwnerUI
	{
	public:
		virtual UINT GetListType() = 0;
		virtual TListInfoUI* GetListInfo() = 0;
		virtual int GetCurSel() const = 0;
		virtual bool SelectItem(int iIndex, bool bTakeFocus = false) = 0;
		virtual bool SelectMultiItem(int iIndex, bool bTakeFocus = false) = 0;
		virtual bool UnSelectItem(int iIndex, bool bOthers = false) = 0;
		virtual void DoEvent(TEventUI& event) = 0;
	};

	class IListUI : public IListOwnerUI
	{
	public:
		virtual CListHeaderUI* GetHeader() const = 0;
		virtual CContainerUI* GetList() const = 0;
		virtual IListCallbackUI* GetTextCallback() const = 0;
		virtual void SetTextCallback(IListCallbackUI* pCallback) = 0;
		virtual bool ExpandItem(int iIndex, bool bExpand = true) = 0;
		virtual int GetExpandedItem() const = 0;

		virtual void SetMultiSelect(bool bMultiSel) = 0;
		virtual bool IsMultiSelect() const = 0;
		virtual void SelectAllItems() = 0;
		virtual void UnSelectAllItems() = 0;
		virtual int GetSelectItemCount() const = 0;
		virtual int GetNextSelItem(int nItem) const = 0;
	};

	class IListItemUI
	{
	public:
		virtual int GetIndex() const = 0;
		virtual void SetIndex(int iIndex) = 0;
		virtual IListOwnerUI* GetOwner() = 0;
		virtual void SetOwner(CControlUI* pOwner) = 0;
		virtual bool IsSelected() const = 0;
		virtual bool Select(bool bSelect = true) = 0;
		virtual bool SelectMulti(bool bSelect = true) = 0;
		virtual bool IsExpanded() const = 0;
		virtual bool Expand(bool bExpand = true) = 0;
		virtual void DrawItemText(IRenderContext& ctx, const RECT& rcItem) = 0;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CListBodyUI;
	class CListHeaderUI;
	class CEditUI;
	class CComboBoxUI;
	class CEmptyUI;
	class UILIB_API CListUI : public CVerticalLayoutUI, public IListUI
	{
		DECLARE_DUICONTROL(CListUI)

	public:
		CListUI();

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		bool GetScrollSelect();
		void SetScrollSelect(bool bScrollSelect);
		int GetCurSel() const;
		int GetCurSelActivate() const;
		bool SelectItem(int iIndex, bool bTakeFocus = false);
		bool SelectItemActivate(int iIndex);    // 双击选中

		bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
		void SetMultiSelect(bool bMultiSel);
		bool IsMultiSelect() const;
		bool UnSelectItem(int iIndex, bool bOthers = false);
		void SelectAllItems();
		void UnSelectAllItems();
		int GetSelectItemCount() const;
		int GetNextSelItem(int nItem) const;

		CListHeaderUI* GetHeader() const;  
		CContainerUI* GetList() const;
		UINT GetListType();
		TListInfoUI* GetListInfo();

		CControlUI* GetItemAt(int iIndex) const;
		int GetItemIndex(CControlUI* pControl) const;
		bool SetItemIndex(CControlUI* pControl, int iIndex);
		int GetCount() const;
		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);
		bool RemoveAt(int iIndex);
		void RemoveAll();

		void EnsureVisible(int iIndex);
		void Scroll(int dx, int dy);

		bool IsDelayedDestroy() const;
		void SetDelayedDestroy(bool bDelayed);
		int GetGap() const;
		void SetGap(int iPadding);

		void SetItemFont(int index);
		void SetItemTextStyle(UINT uStyle);
		void SetItemTextPadding(RECT rc);
		void SetItemColor(DWORD dwColor);
		void SetItemBackgroundColor(DWORD dwBackgroundColor);
		void SetItemBkImage(LPCTSTR pStrImage);
		void SetAlternateBk(bool bAlternateBk);
		void SetAlternateBkColor(DWORD dwColor);
		void SetSelectedItemColor(DWORD dwColor);
		void SetSelectedItemBackgroundColor(DWORD dwBackgroundColor);
		void SetSelectedItemImage(LPCTSTR pStrImage); 
		void SetHoverItemColor(DWORD dwColor);
		void SetHoverItemBackgroundColor(DWORD dwBackgroundColor);
		void SetHoverItemImage(LPCTSTR pStrImage);
		void SetDisabledItemColor(DWORD dwColor);
		void SetDisabledItemBackgroundColor(DWORD dwBackgroundColor);
		void SetDisabledItemImage(LPCTSTR pStrImage);
		void SetItemLineColor(DWORD dwLineColor);
		void SetItemShowRowLine(bool bShowLine = false);
		void SetItemShowColumnLine(bool bShowLine = false);
		bool IsItemShowHtml();
		void SetItemShowHtml(bool bShowHtml = true);
		bool IsItemRSelected();
		void SetItemRSelected(bool bSelected = true);
		RECT GetItemTextPadding() const;
		DWORD GetItemColor() const;
		DWORD GetItemBackgroundColor() const;
		LPCTSTR GetItemBkImage() const;
		bool IsAlternateBk() const;
		DWORD GetAlternateBkColor() const;
		DWORD GetSelectedItemColor() const;
		DWORD GetSelectedItemBackgroundColor() const;
		LPCTSTR GetSelectedItemImage() const;
		DWORD GetHoverItemColor() const;
		DWORD GetHoverItemBackgroundColor() const;
		LPCTSTR GetHoverItemImage() const;
		DWORD GetDisabledItemColor() const;
		DWORD GetDisabledItemBackgroundColor() const;
		LPCTSTR GetDisabledItemImage() const;
		DWORD GetItemLineColor() const;

		void SetMultiExpanding(bool bMultiExpandable); 
		int GetExpandedItem() const;
		bool ExpandItem(int iIndex, bool bExpand = true);

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		IListCallbackUI* GetTextCallback() const;
		void SetTextCallback(IListCallbackUI* pCallback);

		SIZE GetScrollPos() const;
		SIZE GetScrollRange() const;
		void SetScrollPos(SIZE szPos, bool bMsg = true);
		void LineUp();
		void LineDown();
		void PageUp();
		void PageDown();
		void HomeUp();
		void EndDown();
		void LineLeft();
		void LineRight();
		void PageLeft();
		void PageRight();
		void HomeLeft();
		void EndRight();
		void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
		virtual CScrollBarUI* GetVerticalScrollBar() const;
		virtual CScrollBarUI* GetHorizontalScrollBar() const;
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

		virtual BOOL CheckColumEditable(int nColum) { return FALSE; };
		virtual CRichEditUI* GetEditUI() { return NULL; };
		virtual BOOL CheckColumComboBoxable(int nColum) { return FALSE; };
		virtual CComboBoxUI* GetComboBoxUI() { return NULL; };

		/// 空态：嵌套 `<Empty>` 或 `empty-text`；0 项时盖在列表体上
		void SetEmptyControl(CEmptyUI* pEmpty);
		CEmptyUI* GetEmptyControl() const { return m_pEmpty; }
		void UpdateEmptyVisibility();

	protected:
		bool AttachEmptyControl(CControlUI* pControl);
		int GetMinSelItemIndex();
		int GetMaxSelItemIndex();

	protected:
		bool m_bScrollSelect;
		bool m_bMultiSel;
		int m_iCurSel;
		int m_iFirstSel;
		CStdPtrArray m_aSelItems;
		int m_iCurSelActivate;  // 双击的列
		int m_iExpandedItem;
		IListCallbackUI* m_pCallback;
		CListBodyUI* m_pList;
		CListHeaderUI* m_pHeader;
		CEmptyUI* m_pEmpty;
		TListInfoUI m_ListInfo;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CListBodyUI : public CVerticalLayoutUI
	{
	public:
		CListBodyUI(CListUI* pOwner);


		int GetScrollStepSize() const;
		void SetScrollPos(SIZE szPos, bool bMsg = true);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);
	protected:
		static int __cdecl ItemComareFunc(void *pvlocale, const void *item1, const void *item2);
		int __cdecl ItemComareFunc(const void *item1, const void *item2);
	protected:
		CListUI* m_pOwner;
		PULVCompareFunc m_pCompareFunc;
		UINT_PTR m_compareData;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CListHeaderUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CListHeaderUI)
	public:
		CListHeaderUI();
		virtual ~CListHeaderUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		SIZE EstimateSize(SIZE szAvailable);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetScaleHeader(bool bIsScale);
		bool IsScaleHeader() const;

		void DoInit();
		void DoPostPaint(IRenderContext& ctx, const RECT& rcPaint);
	private:
		bool m_bIsScaleHeader;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CListHeaderItemUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CListHeaderItemUI)

	public:
		CListHeaderItemUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(bool bEnable = true);

		bool IsDragable() const;
		void SetDragable(bool bDragable);
		DWORD GetSepWidth() const;
		void SetSepWidth(int iWidth);
		DWORD GetTextStyle() const;
		void SetTextStyle(UINT uStyle);
		DWORD GetColor() const;
		void SetColor(DWORD dwColor);
		void SetTextPadding(RECT rc);
		RECT GetTextPadding() const;
		void SetFont(int index);
		bool IsShowHtml();
		void SetShowHtml(bool bShowHtml = true);
		LPCTSTR GetImage() const;
		void SetImage(LPCTSTR pStrImage);
		LPCTSTR GetHoverImage() const;
		void SetHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetActiveImage() const;
		void SetActiveImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusImage() const;
		void SetFocusImage(LPCTSTR pStrImage);
		LPCTSTR GetSepImage() const;
		void SetSepImage(LPCTSTR pStrImage);
		void SetScale(int nScale);
		int GetScale() const;

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		RECT GetThumbRect() const;

		void PaintText(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);

	protected:
		POINT ptLastMouse;
		bool m_bDragable;
		UINT m_uButtonState;
		int m_iSepWidth;
		DWORD m_dwColor;
		int m_iFont;
		UINT m_uTextStyle;
		bool m_bShowHtml;
		RECT m_rcTextPadding;
		CDuiString m_sImage;
		CDuiString m_sHoverImage;
		CDuiString m_sActiveImage;
		CDuiString m_sFocusImage;
		CDuiString m_sSepImage;
		CDuiString m_sSepImageModify;
		int m_nScale;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CListElementUI : public CControlUI, public IListItemUI
	{
	public:
		CListElementUI();

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		bool PreferClientHit() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetEnabled(bool bEnable = true);

		int GetIndex() const;
		void SetIndex(int iIndex);

		IListOwnerUI* GetOwner();
		void SetOwner(CControlUI* pOwner);
		void SetVisible(bool bVisible = true);

		bool IsSelected() const;
		bool Select(bool bSelect = true);
		bool SelectMulti(bool bSelect = true);
		bool IsExpanded() const;
		bool Expand(bool bExpand = true);

		void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
		bool Activate();

		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void DrawItemBk(IRenderContext& ctx, const RECT& rcItem);

	protected:
		int m_iIndex;
		bool m_bSelected;
		UINT m_uButtonState;
		IListOwnerUI* m_pOwner;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CSvgBoxUI;

	class UILIB_API CListLabelElementUI : public CListElementUI
	{
		DECLARE_DUICONTROL(CListLabelElementUI)
	public:
		CListLabelElementUI();
		~CListLabelElementUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void DrawItemText(IRenderContext& ctx, const RECT& rcItem);
		/// 在指定内容区绘制图标+文字（Combo 闭合态等复用；rcPaint 用于 SVG 裁剪）
		void PaintIconAndText(IRenderContext& ctx, const RECT& rcContent, const RECT& rcPaint,
			DWORD dwTextColor, int iFont, UINT uTextStyle, bool bShowHtml);

		void SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName);
		void SetIconSrc(LPCTSTR pstrPath);
		void ClearIcon();
		bool HasIcon() const;
		void SetIconSize(int nSize);
		int GetIconSize() const { return m_nIconSize; }
		void SetIconGap(int nGap);
		int GetIconGap() const { return m_nIconGap; }
		void SetIconPosition(LPCTSTR pstrPos);
		LPCTSTR GetIconPosition() const { return m_sIconPos; }
		void SetIconTint(DWORD dwColor);
		void SetIconTintHover(DWORD dwColor);
		void SetIconTintSelected(DWORD dwColor);
		void SetIconTintDisabled(DWORD dwColor);
		void SetIconTintAuto(bool bAuto);
		bool IsIconTintAuto() const { return m_bIconTintAuto; }

	protected:
		enum IconKind { IconNone = 0, IconSvg = 1, IconRaster = 2 };

		void EnsureIcon();
		void EnsureRasterIcon();
		bool IsIconAttr(LPCTSTR pstrName) const;
		static bool IsRasterImagePath(LPCTSTR pstrPath);
		void ShowSvgIcon();
		void ShowRasterIcon(LPCTSTR pstrPath);
		void RefreshRasterIconImage();
		void ClearRasterTintCache();
		bool EnsureRasterTintCache(DWORD dwColor);
		void PaintRasterIcon(IRenderContext& ctx, const RECT& rcIcon);
		bool ShouldTintRasterIcon() const;
		void SyncIconAppearance();
		DWORD ResolveIconColor() const;
		DWORD ResolvePaintIconColor() const;
		bool LayoutIconAndText(const RECT& rcContent, RECT& rcIcon, RECT& rcText) const;

		CSvgBoxUI* m_pIcon;
		CControlUI* m_pRasterIcon;
		IconKind m_eIconKind;
		CDuiString m_sRasterPath;
		HBITMAP m_hRasterTint;
		DWORD m_dwRasterTintColor;
		int m_nRasterTintW;
		int m_nRasterTintH;
		int m_nIconSize;
		int m_nIconGap;
		CDuiString m_sIconPos; // left / right / top / bottom
		DWORD m_dwIconTint;
		DWORD m_dwIconTintHover;
		DWORD m_dwIconTintSelected;
		DWORD m_dwIconTintDisabled;
		bool m_bIconTint;
		bool m_bIconTintAuto;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CListTextElementUI : public CListLabelElementUI
	{
		DECLARE_DUICONTROL(CListTextElementUI)
	public:
		CListTextElementUI();
		~CListTextElementUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		LPCTSTR GetText(int iIndex) const;
		void SetText(int iIndex, LPCTSTR pstrText);

		DWORD GetColor(int iIndex) const;
		void SetColor(int iIndex, DWORD dwColor);

		void SetOwner(CControlUI* pOwner);
		CDuiString* GetLinkContent(int iIndex);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);

		void DrawItemText(IRenderContext& ctx, const RECT& rcItem);

	protected:
		enum { MAX_LINK = 8 };
		int m_nLinks;
		RECT m_rcLinks[MAX_LINK];
		CDuiString m_sLinks[MAX_LINK];
		int m_nHoverLink;
		IListUI* m_pOwner;
		CStdPtrArray m_aTexts;
		CStdPtrArray m_aTextColors;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CListContainerElementUI : public CHorizontalLayoutUI, public IListItemUI
	{
		DECLARE_DUICONTROL(CListContainerElementUI)
	public:
		CListContainerElementUI();

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		bool PreferClientHit() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		int GetIndex() const;
		void SetIndex(int iIndex);

		IListOwnerUI* GetOwner();
		void SetOwner(CControlUI* pOwner);
		void SetVisible(bool bVisible = true);
		void SetEnabled(bool bEnable = true);

		bool IsSelected() const;
		bool Select(bool bSelect = true);
		bool SelectMulti(bool bSelect = true);
		bool IsExpanded() const;
		bool Expand(bool bExpand = true);

		void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
		bool Activate();

		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);

		virtual void DrawItemText(IRenderContext& ctx, const RECT& rcItem);    
		virtual void DrawItemBk(IRenderContext& ctx, const RECT& rcItem);

		void SetPos(RECT rc, bool bNeedInvalidate = true);

	protected:
		int m_iIndex;
		bool m_bSelected;
		UINT m_uButtonState;
		IListOwnerUI* m_pOwner;
	};

} // namespace DuiLib

#endif // __UILIST_H__
