#ifndef __UITRANSFER_H__
#define __UITRANSFER_H__

#pragma once

#include <vector>

namespace DuiLib
{
	/// 穿梭项（数据节点，由父 Transfer 消费后移除）。
	class UILIB_API CTransferItemUI : public CControlUI
	{
		DECLARE_DUICONTROL(CTransferItemUI)
	public:
		CTransferItemUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetValue(LPCTSTR pstr);
		LPCTSTR GetValue() const;
		void SetTarget(bool b);
		bool IsTarget() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		SIZE EstimateSize(SIZE szAvailable);

	protected:
		CDuiString m_sValue;
		bool m_bTarget;
	};

	/// 左右穿梭框：源列表 / 目标列表 + 中间移动按钮。
	class UILIB_API CTransferUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CTransferUI)
	public:
		struct Item
		{
			CDuiString sText;
			CDuiString sValue;
			bool bTarget;
			bool bChecked;
			Item() : bTarget(false), bChecked(false) {}
		};

		CTransferUI();
		~CTransferUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetTitles(LPCTSTR pstrTitles); // 源|目标
		LPCTSTR GetSourceTitle() const;
		LPCTSTR GetTargetTitle() const;

		void SetItems(LPCTSTR pstrItems); // text|text 或 text:value|…
		void SetTargetKeys(LPCTSTR pstrKeys); // value|value… 初始在右侧
		CDuiString GetTargetKeys() const;
		CDuiString GetSourceKeys() const;

		int GetItemCount() const;
		const Item* GetItem(int i) const;
		void ClearItems();
		void AddItem(LPCTSTR pstrText, LPCTSTR pstrValue = NULL, bool bTarget = false);
		void MoveCheckedToTarget(bool bNotify = true);
		void MoveCheckedToSource(bool bNotify = true);

		void SetItemHeight(int n);
		int GetItemHeight() const;
		void SetShowSelectAll(bool b);
		bool IsShowSelectAll() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoInit();
		/// 主题热切：左右面板 / 表头 / 标题·计数 / 列表行色
		void ApplyThemeChrome(DWORD dwPanelBg, DWORD dwHeaderBg, DWORD dwBorder,
			DWORD dwTitleColor, DWORD dwCountColor,
			DWORD dwListBg, DWORD dwItemColor, DWORD dwItemHoverBg,
			DWORD dwItemSelBg, DWORD dwItemSelColor, DWORD dwItemLine);

	protected:
		int ScaleValue(int v) const;
		void CollectDataNodes();
		void ParseItemsAttr(LPCTSTR pstr);
		void ApplyTargetKeysAttr(LPCTSTR pstr);
		void EnsureBuilt();
		void BuildShell();
		void RefreshLists();
		void FillList(CListUI* pList, bool bTarget);
		void UpdateHeaders();
		void UpdateButtons();
		int CountSide(bool bTarget) const;
		int CountChecked(bool bTarget) const;
		void SetAllChecked(bool bTarget, bool bChecked);

		bool OnBtnNotify(void* param);
		bool OnCheckNotify(void* param);
		bool OnHeaderCheckNotify(void* param);

	protected:
		bool m_bBuilt;
		bool m_bShowSelectAll;
		int m_nItemHeight;
		CDuiString m_sSourceTitle;
		CDuiString m_sTargetTitle;
		CDuiString m_sItemsAttr;
		CDuiString m_sTargetAttr;
		std::vector<Item> m_items;

		CVerticalLayoutUI* m_pLeftPanel;
		CVerticalLayoutUI* m_pRightPanel;
		CHorizontalLayoutUI* m_pLeftHeader;
		CHorizontalLayoutUI* m_pRightHeader;
		CCheckBoxUI* m_pLeftAll;
		CCheckBoxUI* m_pRightAll;
		CLabelUI* m_pLeftTitle;
		CLabelUI* m_pRightTitle;
		CLabelUI* m_pLeftCount;
		CLabelUI* m_pRightCount;
		CListUI* m_pLeftList;
		CListUI* m_pRightList;
		CButtonUI* m_pBtnToTarget;
		CButtonUI* m_pBtnToSource;
		bool m_bRefreshing;
	};
}

#endif // __UITRANSFER_H__
