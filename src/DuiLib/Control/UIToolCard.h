#ifndef __UITOOLCARD_H__
#define __UITOOLCARD_H__

#pragma once

namespace DuiLib
{
	/// 标题栏右侧自定义槽（XML: ToolCardHeader），按内容收缩宽度
	class UILIB_API CToolCardHeaderUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CToolCardHeaderUI)
	public:
		CToolCardHeaderUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		SIZE EstimateSize(SIZE szAvailable) override;
	};

	/// 内容区容器（XML: ToolCardBody）：竖滚动 + 默认右键「全选 / 复制」
	class UILIB_API CToolCardBodyUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CToolCardBodyUI)
	public:
		CToolCardBodyUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		void DoEvent(TEventUI& event) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		void SelectAllText();
		void ClearTextSelection();
		bool IsTextSelected() const { return m_bTextSelected; }
		/// 将内容区可见文本复制到剪贴板（换行拼接）
		bool CopyBodyText();
		CDuiString CollectBodyText() const;

	protected:
		void ShowBuiltinContextMenu(POINT ptClient);
		void ApplySelectionChrome();
		static void CollectTextRecursive(CControlUI* pControl, CDuiString& sOut);
		static bool CopyTextToClipboard(HWND hWnd, LPCTSTR pstrText);

		bool m_bTextSelected;
		DWORD m_dwNormalBk;
		DWORD m_dwSelectedBk;
	};

	/// 聊天工具卡：标题栏 + 内容区（对齐 Cursor Agent 工具结果块）
	/// 点标题栏（箭头等）→ 展开/折叠；kind=file 时点蓝色文件名 → toolcardopen
	class UILIB_API CToolCardUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CToolCardUI)
	public:
		enum Kind {
			TOOLCARD_FILE = 0,
			TOOLCARD_CMD = 1,
			TOOLCARD_GENERIC = 2
		};

		CToolCardUI();
		~CToolCardUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		void DoInit() override;

		void SetCardKind(Kind kind);
		Kind GetCardKind() const { return m_kind; }
		void SetCardKindString(LPCTSTR pstr);
		CDuiString GetCardKindString() const;

		void SetTitle(LPCTSTR pstrText);
		CDuiString GetTitle() const;
		void SetPath(LPCTSTR pstrPath);
		LPCTSTR GetPath() const { return m_sPath.GetData(); }
		void SetCommand(LPCTSTR pstrCmd);
		LPCTSTR GetCommand() const { return m_sCommand.GetData(); }

		void SetExpanded(bool bExpanded, bool bNotify = true);
		bool IsExpanded() const { return m_bExpanded; }
		void ToggleExpanded();
		/// 程序化触发与点文件名相同的 toolcardopen（仅 kind=file 有效）
		void OpenFile();

		void SetHeaderHeight(int n);
		int GetHeaderHeight() const { return m_nHeaderHeight; }
		/// 展开时内容区固定视口高度；0=按内容估高（再受 min/max 限制）
		void SetBodyHeight(int n);
		int GetBodyHeight() const { return m_nBodyHeight; }
		/// 内容区最小高度（逻辑像素）。0=不强制
		void SetBodyMinHeight(int n);
		int GetBodyMinHeight() const { return m_nBodyMinHeight; }
		/// 内容区最大高度（逻辑像素）；超出出现竖滚动条。0=不限制
		void SetBodyMaxHeight(int n);
		int GetBodyMaxHeight() const { return m_nBodyMaxHeight; }

		/// 无子项时可用 body 文本做只读预览（写入内建 Label）
		void SetBodyText(LPCTSTR pstr);
		LPCTSTR GetBodyText() const;
		/// 向内容区追加一行 Label（运行时日志）；返回该 Label
		CLabelUI* AppendBodyLine(LPCTSTR pstrText, DWORD dwColor = 0x495057FF, int nHeight = 18);
		/// 清空内容区子控件（含 body-text Label）
		void ClearBody();
		/// 收集内容区全部可见文本（含子控件），非仅 body-text
		CDuiString CollectBodyText() const;
		void SelectAllBody();
		bool CopyBodyText();

		/// 标题栏右侧自定义槽 / 内容区槽
		CToolCardHeaderUI* GetHeaderSlot() const { return m_pHeaderSlot; }
		CToolCardBodyUI* GetBody() const { return m_pBody; }
		/// 内建标题栏 chrome（非 ToolCardHeader 槽）
		CHorizontalLayoutUI* GetHeaderChrome() const { return m_pHeader; }
		CLabelUI* GetChevron() const { return m_pChevron; }
		CLabelUI* GetKindBadge() const { return m_pKindBadge; }
		CLabelUI* GetTitleLabel() const { return m_pTitle; }

		void SetShowChevron(bool b);
		bool IsShowChevron() const { return m_bShowChevron; }
		void SetShowKindBadge(bool b);
		bool IsShowKindBadge() const { return m_bShowKindBadge; }
		void SetShowTitle(bool b);
		bool IsShowTitle() const { return m_bShowTitle; }

		void SetHeaderBkColor(DWORD dwColor);
		DWORD GetHeaderBkColor() const { return m_dwHeaderBk; }
		void SetHeaderHoverBkColor(DWORD dwColor);
		DWORD GetHeaderHoverBkColor() const { return m_dwHeaderHoverBk; }
		void SetChevronColor(DWORD dwColor);
		DWORD GetChevronColor() const { return m_dwChevronColor; }
		void SetTitleColor(DWORD dwColor);
		DWORD GetTitleColor() const { return m_dwTitleColor; }
		/// kind=file 时文件名链接色
		void SetTitleLinkColor(DWORD dwColor);
		DWORD GetTitleLinkColor() const { return m_dwTitleLinkColor; }

		/// 空串=按 kind 自动 File/Cmd/Tool
		void SetKindBadgeText(LPCTSTR pstr);
		CDuiString GetKindBadgeText() const;
		void SetKindBadgeTextColor(DWORD dwColor);
		DWORD GetKindBadgeTextColor() const { return m_dwBadgeTextColor; }
		/// 设后不再随 kind 自动改底色；传 0 恢复自动
		void SetKindBadgeBkColor(DWORD dwColor);
		DWORD GetKindBadgeBkColor() const;
		void SetKindBadgeWidth(int n);
		int GetKindBadgeWidth() const { return m_nBadgeWidth; }

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		SIZE EstimateSize(SIZE szAvailable) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		void OnHeaderClick();
		void OnTitleClick();
		void OnHeaderHoverChanged(bool bHot);

	protected:
		void EnsureBuilt();
		void SyncHeaderChrome();
		void SyncBodyVisibility();
		void SyncHeaderSlotMetrics();
		void SyncBodySlotDefaults(CToolCardBodyUI* pBody);
		void UpdateFixedHeight();
		int ScaleValue(int v) const;
		/// 内容区视口高度（已含 min/max 钳制）；折叠返回 0
		int CalcBodyViewportHeight(int cxAvail) const;
		void RequestAncestorLayout();
		void SyncKindBadge();
		void SyncTitleFromMeta();
		bool IsHeaderSlotControl(CControlUI* pControl) const;
		bool IsBodySlotControl(CControlUI* pControl) const;
		bool AdoptHeaderSlot(CToolCardHeaderUI* pNew);
		bool AdoptBodySlot(CToolCardBodyUI* pNew);
		static void DetachFromParent(CContainerUI* pParent, CControlUI* pChild);
		static bool ParseBoolValue(LPCTSTR pstrValue);
		static DWORD ParseColorValue(LPCTSTR pstrValue);

	protected:
		Kind m_kind;
		bool m_bExpanded;
		bool m_bBuilt;
		bool m_bHeaderHover;
		bool m_bShowChevron;
		bool m_bShowKindBadge;
		bool m_bShowTitle;
		bool m_bBadgeBkCustom;
		int m_nHeaderHeight;
		int m_nBodyHeight;
		int m_nBodyMinHeight;
		int m_nBodyMaxHeight;
		int m_nBadgeWidth;
		CDuiString m_sPath;
		CDuiString m_sCommand;
		CDuiString m_sTitleOverride;
		CDuiString m_sBadgeTextOverride;
		CHorizontalLayoutUI* m_pHeader;
		CToolCardHeaderUI* m_pHeaderSlot;
		CLabelUI* m_pChevron;
		CLabelUI* m_pKindBadge;
		CLabelUI* m_pTitle;
		CToolCardBodyUI* m_pBody;
		CLabelUI* m_pBodyLabel;
		DWORD m_dwHeaderBk;
		DWORD m_dwHeaderHoverBk;
		DWORD m_dwChevronColor;
		DWORD m_dwTitleColor;
		DWORD m_dwTitleLinkColor;
		DWORD m_dwBadgeTextColor;
		DWORD m_dwBadgeBkCustom;
	};
}

#endif // __UITOOLCARD_H__

