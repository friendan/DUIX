#ifndef __UIDATETIME_H__
#define __UIDATETIME_H__

#pragma once

namespace DuiLib
{
	class CDateTimeWnd;
	class CCalendarPanelUI;

	/// 自绘日期/时间：弹层月历 + 月/年面板 + 时分秒。
	class UILIB_API CDateTimeUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CDateTimeUI)
		friend class CDateTimeWnd;
		friend class CCalendarPanelUI;

	public:
		CDateTimeUI();
		~CDateTimeUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		SYSTEMTIME& GetTime();
		void SetTime(const SYSTEMTIME& st);
		void SetTime(SYSTEMTIME* pst);

		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;

		void SetFormat(LPCTSTR pstrFormat);
		CDuiString GetFormat() const;

		void SetShowToday(bool bShow);
		bool IsShowToday() const;
		void SetShowTime(bool bShow);
		bool IsShowTime() const;
		void SetShowSeconds(bool bShow);
		bool IsShowSeconds() const;
		bool IsShowDate() const;

		void SetFirstDayOfWeek(int nFirst); // 0=周日 … 6=周六
		int GetFirstDayOfWeek() const;

		bool IsDropDownOpened() const;
		void ActivateDropDown();
		void CloseDropDown();
		/// 若日历已打开，按当前主题重刷弹层壳
		void SyncOpenCalendarShell();

		void UpdateText();
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintStatusImage(IRenderContext& ctx);

		SIZE GetDropBoxSize() const;
		DWORD GetDayTextColor() const;

	protected:
		void SyncFormatFlags();
		void OnPicked(const SYSTEMTIME& st, bool bClose = true);
		void OnLiveTimeChanged(const SYSTEMTIME& st);
		int ScaleValue(int v) const;

	protected:
		SYSTEMTIME m_sysTime;
		bool m_bReadOnly;
		bool m_bShowToday;
		bool m_bShowTime;      // 显式或由 format 推断
		bool m_bShowSeconds;
		bool m_bShowDate;
		bool m_bFormatDrivesFlags; // format 自动推导 show-time/date；属性覆盖后为 false
		int m_nFirstDayOfWeek;
		CDuiString m_sFormat;
		CDateTimeWnd* m_pWindow;

		DWORD m_dwSelectedBk;
		DWORD m_dwHoverBk;
		DWORD m_dwTodayColor;
		DWORD m_dwOtherMonthColor;
		DWORD m_dwHeaderColor;
		DWORD m_dwDayColor;
		DWORD m_dwSelectedText;
		DWORD m_dwMutedBtnColor;
	};
}
#endif // __UIDATETIME_H__
