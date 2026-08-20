#ifndef __UILOOKUPEDIT_H__
#define __UILOOKUPEDIT_H__

#pragma once

namespace DuiLib {

	class CLookupWnd;

	/// 行数据回调：下标均为全集行（过滤前）。单元格指针只需在本次调用内有效。
	class ILookupEditCallback
	{
	public:
		virtual ~ILookupEditCallback() {}
		virtual int GetRowCount() = 0;
		virtual LPCTSTR GetCellText(int nRow, int nCol) = 0;
	};

	/// 列元数据：name / text / width 走 CControlUI；不参与 LookupEdit 闭合态排版。
	class UILIB_API CLookupColumnUI : public CControlUI
	{
		DECLARE_DUICONTROL(CLookupColumnUI)
	public:
		CLookupColumnUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;

		/// XML width 的逻辑像素（未 Scale）。
		int GetLogicWidth() const;
		void SetLogicWidth(int nWidth);
	};

	/// 只读查找框：点击弹出模态多列表，按列过滤后双击/回车确认。
	/// 确认只投 itemselect（wParam=全集行号），不自动改 text；由调用方 SetText。
	class UILIB_API CLookupEditUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CLookupEditUI)
		friend class CLookupWnd;
	public:
		CLookupEditUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		UINT GetControlFlags() const override;

		void SetCallback(ILookupEditCallback* pCallback);
		ILookupEditCallback* GetCallback() const;

		int GetCurSel() const;
		void SetCurSel(int iIndex);

		/// 弹窗宽高（逻辑像素，走 DPI）。cx==0 按编辑框与列宽自适应；cy==0 用 280。
		SIZE GetDropBoxSize() const;
		void SetDropBoxSize(SIZE szDropBox);

		/// 弹窗相对编辑框：下 / 上 / 左 / 右 / 中。
		enum DropPosition
		{
			DropBottom = 0,
			DropTop,
			DropLeft,
			DropRight,
			DropCenter
		};
		DropPosition GetDropPosition() const;
		void SetDropPosition(DropPosition ePos);

		int GetItemHeight() const;
		void SetItemHeight(int nHeight);

		void SetColor(DWORD dwColor);
		DWORD GetColor() const;
		void SetDisabledColor(DWORD dwColor);
		DWORD GetDisabledColor() const;
		void SetFont(int index);
		int GetFont() const;
		void SetTextStyle(UINT uStyle);
		UINT GetTextStyle() const;
		RECT GetTextPadding() const;
		void SetTextPadding(RECT rc);

		void SetPlaceholder(LPCTSTR pstrText);
		LPCTSTR GetPlaceholder() const;
		void SetPlaceholderColor(DWORD dwColor);
		DWORD GetPlaceholderColor() const;

		CLookupColumnUI* GetColumn(int iIndex) const;
		int GetColumnCount() const;
		void AddColumn(LPCTSTR pstrName, LPCTSTR pstrText, int nWidth);

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;

		bool Activate() override;
		SIZE EstimateSize(SIZE szAvailable) override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void DoEvent(TEventUI& event) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		void PaintStatusImage(IRenderContext& ctx) override;
		void PaintText(IRenderContext& ctx) override;

	protected:
		CLookupWnd* m_pWindow;
		ILookupEditCallback* m_pCallback;
		int m_iCurSel;
		int m_iPendingPick;
		DWORD m_dwColor;
		DWORD m_dwDisabledColor;
		int m_iFont;
		UINT m_uTextStyle;
		RECT m_rcTextPadding;
		CDuiString m_sPlaceholder;
		DWORD m_dwPlaceholderColor;
		SIZE m_szDropBox;
		DropPosition m_eDropPos;
		UINT m_uButtonState;
		int m_nItemHeight;
	};

} // namespace DuiLib

#endif // __UILOOKUPEDIT_H__
