#ifndef UITreeView_h__
#define UITreeView_h__

#include <vector>
using namespace std;

#pragma once

namespace DuiLib
{
	class CTreeViewUI;
	class CCheckBoxUI;
	class CLabelUI;
	class COptionUI;

	class UILIB_API CTreeNodeUI : public CListContainerElementUI
	{
		DECLARE_DUICONTROL(CTreeNodeUI)
	public:
		CTreeNodeUI(CTreeNodeUI* _ParentNode = NULL);
		~CTreeNodeUI(void);

	public:
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void DoEvent(TEventUI& event);
		void Invalidate();
		bool Select(bool bSelect = true);
		bool SelectMulti(bool bSelect = true);

		bool Add(CControlUI* _pTreeNodeUI);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);
		using CListContainerElementUI::RemoveAt;
		bool RemoveAt(CTreeNodeUI* _pTreeNodeUI);

		void SetVisibleTag(bool _IsVisible);
		bool GetVisibleTag();
		void SetItemText(LPCTSTR pstrValue);
		CDuiString GetItemText();
		void CheckBoxSelected(bool _Selected);
		bool IsCheckBoxSelected() const;
		bool IsHasChild() const;
		long GetTreeLevel();
		bool AddChildNode(CTreeNodeUI* _pTreeNodeUI);
		void SetParentNode(CTreeNodeUI* _pParentTreeNode);
		CTreeNodeUI* GetParentNode();
		long GetCountChild();
		void SetTreeView(CTreeViewUI* _CTreeViewUI);
		CTreeViewUI* GetTreeView();
		CTreeNodeUI* GetChildNode(int _nIndex);
		void SetVisibleFolderBtn(bool _IsVisibled);
		bool GetVisibleFolderBtn();
		void SetVisibleCheckBtn(bool _IsVisibled);
		bool GetVisibleCheckBtn();
		void SetItemColor(DWORD _dwItemTextColor);
		DWORD GetItemColor() const;
		void SetItemHoverColor(DWORD _dwItemHotTextColor);
		DWORD GetItemHoverColor() const;
		void SetSelItemColor(DWORD _dwSelItemTextColor);
		DWORD GetSelItemColor() const;
		void SetSelItemHoverColor(DWORD _dwSelHotItemTextColor);
		DWORD GetSelItemHoverColor() const;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		/// 节点图标（转发到内部 Option/Button，语义同 Button / ListLabel）
		void SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName);
		void SetIconSrc(LPCTSTR pstrPath);
		void ClearIcon();
		bool HasIcon() const;
		void SetIconSize(int nSize);
		int GetIconSize() const;
		void SetIconTint(DWORD dwColor);
		void SetIconTintAuto(bool bAuto);

		CStdPtrArray GetTreeNodes();
		int			 GetTreeIndex();
		int			 GetNodeIndex();

	public:
		CHorizontalLayoutUI*	GetTreeNodeHoriznotal() const {return pHoriz;};
		CCheckBoxUI*			GetFolderButton() const {return pFolderButton;};
		CLabelUI*				GetDottedLine() const {return pDottedLine;};
		CCheckBoxUI*			GetCheckBox() const {return pCheckBox;};
		COptionUI*				GetItemButton() const {return pItemButton;};

	private:
		static bool IsIconAttrName(LPCTSTR pstrName);
		CTreeNodeUI* GetLastNode();
		CTreeNodeUI* CalLocation(CTreeNodeUI* _pTreeNodeUI);

	private:
		long	m_iTreeLavel;
		bool	m_bIsVisable;
		bool	m_bIsCheckBox;
		DWORD	m_dwItemColor;
		DWORD	m_dwItemHoverColor;
		DWORD	m_dwSelItemColor;
		DWORD	m_dwSelItemHoverColor;

		CTreeViewUI*			pTreeView;
		CHorizontalLayoutUI*	pHoriz;
		CCheckBoxUI*			pFolderButton;
		CLabelUI*				pDottedLine;
		CCheckBoxUI*			pCheckBox;
		COptionUI*				pItemButton;
		CTreeNodeUI*			pParentTreeNode;
		CStdPtrArray			mTreeNodes;
	};

	class UILIB_API CTreeViewUI : public CListUI,public INotifyUI
	{
		DECLARE_DUICONTROL(CTreeViewUI)
	public:
		CTreeViewUI(void);
		~CTreeViewUI(void);

	public:
		virtual LPCTSTR GetClass() const;
		virtual LPVOID	GetInterface(LPCTSTR pstrName);

		using CListUI::Add;
		using CListUI::AddAt;
		using CListUI::Remove;
		virtual UINT GetListType();
		virtual bool Add(CTreeNodeUI* pControl );
		virtual long AddAt(CTreeNodeUI* pControl, int iIndex );
		virtual bool AddAt(CTreeNodeUI* pControl,CTreeNodeUI* _IndexNode);
		virtual bool Remove(CTreeNodeUI* pControl);
		virtual bool RemoveAt(int iIndex);
		virtual void RemoveAll();
		virtual bool OnCheckBoxChanged(void* param);
		virtual bool OnFolderChanged(void* param);
		virtual bool OnDBClickItem(void* param);
		virtual bool SetItemCheckBox(bool _Selected,CTreeNodeUI* _TreeNode = NULL);
		virtual void SetItemExpand(bool _Expanded,CTreeNodeUI* _TreeNode = NULL);
		virtual void Notify(TNotifyUI& msg);
		virtual void SetVisibleFolderBtn(bool _IsVisibled);
		virtual bool GetVisibleFolderBtn();
		virtual void SetVisibleCheckBtn(bool _IsVisibled);
		virtual bool GetVisibleCheckBtn();
		virtual void SetItemMinWidth(UINT _ItemMinWidth);
		virtual UINT GetItemMinWidth();
		virtual void SetItemColor(DWORD _dwItemTextColor);
		virtual void SetItemHoverColor(DWORD _dwItemHotTextColor);
		virtual void SetSelItemColor(DWORD _dwSelItemTextColor);
		virtual void SetSelItemHoverColor(DWORD _dwSelHotItemTextColor);
		
		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	private:
		UINT m_uItemMinWidth;
		bool m_bVisibleFolderBtn;
		bool m_bVisibleCheckBtn;
	};
}


#endif // UITreeView_h__
