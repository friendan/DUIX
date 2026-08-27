#ifndef __UICOMBO_H__
#define __UICOMBO_H__

#pragma once

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CComboWnd;
	class CSvgBoxUI;

	class UILIB_API CComboUI : public CContainerUI, public IListOwnerUI
	{
		DECLARE_DUICONTROL(CComboUI)
		friend class CComboWnd;
	public:
		CComboUI();
		~CComboUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;

		void DoInit() override;
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
		UINT GetControlFlags() const override;

		CDuiString GetText() const override;
		void SetEnabled(bool bEnable = true) override;

		void SetTextStyle(UINT uStyle);
		UINT GetTextStyle() const;
		void SetColor(DWORD dwColor);
		DWORD GetColor() const;
		void SetDisabledColor(DWORD dwColor);
		DWORD GetDisabledColor() const;
		void SetFont(int index);
		int GetFont() const;
		RECT GetTextPadding() const;
		void SetTextPadding(RECT rc);
		bool IsShowHtml();
		void SetShowHtml(bool bShowHtml = true);
		bool IsShowShadow();
		void SetShowShadow(bool bShow = true);

		CDuiString GetDropBoxAttributeList();
		void SetDropBoxAttributeList(LPCTSTR pstrList);
		SIZE GetDropBoxSize() const;
		void SetDropBoxSize(SIZE szDropBox);
		RECT GetDropBoxPadding() const;
		void SetDropBoxPadding(RECT rcDropBoxPadding);

		UINT GetListType() override;
		TListInfoUI* GetListInfo() override;
		int GetCurSel() const override;
		bool SelectItem(int iIndex, bool bTakeFocus = false) override;
		bool SelectMultiItem(int iIndex, bool bTakeFocus = false) override;
		bool UnSelectItem(int iIndex, bool bOthers = false) override;
		bool SetItemIndex(CControlUI* pControl, int iIndex) override;

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		bool Remove(CControlUI* pControl) override;
		bool RemoveAt(int iIndex) override;
		void RemoveAll() override;

		bool Activate() override;
		void SyncOpenDropShell();

		LPCTSTR GetImage() const;
		void SetImage(LPCTSTR pStrImage);
		LPCTSTR GetHoverImage() const;
		void SetHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetActiveImage() const;
		void SetActiveImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusImage() const;
		void SetFocusImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage() const;
		void SetDisabledImage(LPCTSTR pStrImage);

		bool GetScrollSelect();
		void SetScrollSelect(bool bScrollSelect);

		/// 闭合态右侧倒三角（无自定义 image 时）；默认 true
		bool IsShowArrow() const { return m_bShowArrow; }
		void SetShowArrow(bool bShow);
		/// 按最长项文字（含图标）自适应宽度；默认 false
		bool IsAutoWidth() const { return m_bAutoWidth; }
		void SetAutoWidth(bool bAuto);

		void SetItemFont(int index);
		void SetItemTextStyle(UINT uStyle);
		RECT GetItemTextPadding() const;
		void SetItemTextPadding(RECT rc);
		DWORD GetItemColor() const;
		void SetItemColor(DWORD dwColor);
		DWORD GetItemBackgroundColor() const;
		void SetItemBackgroundColor(DWORD dwBackgroundColor);
		LPCTSTR GetItemBkImage() const;
		void SetItemBkImage(LPCTSTR pStrImage);
		bool IsAlternateBk() const;
		void SetAlternateBk(bool bAlternateBk);
		DWORD GetAlternateBkColor() const;
		void SetAlternateBkColor(DWORD dwColor);
		DWORD GetSelectedItemColor() const;
		void SetSelectedItemColor(DWORD dwColor);
		DWORD GetSelectedItemBackgroundColor() const;
		void SetSelectedItemBackgroundColor(DWORD dwBackgroundColor);
		LPCTSTR GetSelectedItemImage() const;
		void SetSelectedItemImage(LPCTSTR pStrImage);
		DWORD GetHoverItemColor() const;
		void SetHoverItemColor(DWORD dwColor);
		DWORD GetHoverItemBackgroundColor() const;
		void SetHoverItemBackgroundColor(DWORD dwBackgroundColor);
		LPCTSTR GetHoverItemImage() const;
		void SetHoverItemImage(LPCTSTR pStrImage);
		DWORD GetDisabledItemColor() const;
		void SetDisabledItemColor(DWORD dwColor);
		DWORD GetDisabledItemBackgroundColor() const;
		void SetDisabledItemBackgroundColor(DWORD dwBackgroundColor);
		LPCTSTR GetDisabledItemImage() const;
		void SetDisabledItemImage(LPCTSTR pStrImage);
		DWORD GetItemLineColor() const;
		void SetItemLineColor(DWORD dwLineColor);
		bool IsItemShowHtml();
		void SetItemShowHtml(bool bShowHtml = true);

		SIZE EstimateSize(SIZE szAvailable) override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void Move(SIZE szOffset, bool bNeedInvalidate = true) override;
		void DoEvent(TEventUI& event) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		void PaintText(IRenderContext& ctx) override;
		void PaintStatusImage(IRenderContext& ctx) override;

	public:
		void SortItems();
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

	protected:
		int GetArrowReserve() const;
		int CalcAutoWidth() const;
		void EnsureArrowIcon();
		static int __cdecl ItemComareFunc(void* pvlocale, const void* item1, const void* item2);
		int __cdecl ItemComareFunc(const void* item1, const void* item2);

	protected:
		PULVCompareFunc m_pCompareFunc;
		UINT_PTR m_compareData;

	protected:
		CComboWnd* m_pWindow;

		int m_iCurSel;
		DWORD	m_dwColor;
		DWORD	m_dwDisabledColor;
		int		m_iFont;
		UINT	m_uTextStyle;
		RECT	m_rcTextPadding;
		bool	m_bShowHtml;
		bool	m_bShowShadow;
		CDuiString m_sDropBoxAttributes;
		SIZE m_szDropBox;
		RECT m_rcDropBoxPadding;
		UINT m_uButtonState;

		CDuiString m_sImage;
		CDuiString m_sHoverImage;
		CDuiString m_sActiveImage;
		CDuiString m_sFocusImage;
		CDuiString m_sDisabledImage;

		bool m_bScrollSelect;
		bool m_bShowArrow;
		bool m_bAutoWidth;
		CSvgBoxUI* m_pArrowIcon;
		TListInfoUI m_ListInfo;
	};

} // namespace DuiLib

#endif // __UICOMBO_H__
