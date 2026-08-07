#ifndef __UICOMBO_H__
#define __UICOMBO_H__

#pragma once

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CComboWnd;

	class UILIB_API CComboUI : public CContainerUI, public IListOwnerUI
	{
		DECLARE_DUICONTROL(CComboUI)
		friend class CComboWnd;
	public:
		CComboUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void DoInit();
		UINT GetControlFlags() const;

		CDuiString GetText() const;
		void SetEnabled(bool bEnable = true);

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

		UINT GetListType();
		TListInfoUI* GetListInfo();
		int GetCurSel() const;  
		bool SelectItem(int iIndex, bool bTakeFocus = false);
		bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
		bool UnSelectItem(int iIndex, bool bOthers = false);
		bool SetItemIndex(CControlUI* pControl, int iIndex);

		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);
		bool RemoveAt(int iIndex);
		void RemoveAll();

		bool Activate();
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

		SIZE EstimateSize(SIZE szAvailable);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void PaintText(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);

	public:
		void SortItems();
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

	protected:
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
		TListInfoUI m_ListInfo;
	};

} // namespace DuiLib

#endif // __UICOMBO_H__
