#ifndef __UITEXTSCROLLH__
#define __UITEXTSCROLLH__

#pragma once

namespace DuiLib
{
	#define ROLLTEXT_LEFT		0
	#define ROLLTEXT_RIGHT		1
	#define ROLLTEXT_UP 		2
	#define ROLLTEXT_DOWN		3

	#define ROLLTEXT_TIMERID			20
	#define ROLLTEXT_TIMERID_SPAN		50U

	#define ROLLTEXT_ROLL_END			21
	#define ROLLTEXT_ROLL_END_SPAN		1000*6U

	class UILIB_API CRollTextUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CRollTextUI)
	public:
		CRollTextUI(void);
		~CRollTextUI(void);

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void DoInit();
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintText(IRenderContext& ctx);
		void DoEvent(TEventUI& event);
		void SetPos(RECT rc);
		void SetText(LPCTSTR pstrText);
		bool PreferClientHit() const;

		/// 开始滚动。nDirect: ROLLTEXT_LEFT/RIGHT/UP/DOWN；lTimeSpan: 帧间隔 ms；
		/// lMaxTimeLimited: 最长秒数，<=0 不限时。循环次数用 SetRollLoop。
		void BeginRoll(int nDirect = ROLLTEXT_LEFT, LONG lTimeSpan = ROLLTEXT_TIMERID_SPAN, LONG lMaxTimeLimited = 0);
		void EndRoll();
		bool IsRolling() const { return m_bUseRoll != FALSE; }

		/// 程序暂停/继续（与悬停暂停独立：两者任一生效即停表）
		void Pause();
		void Resume();
		bool IsPaused() const { return m_bPaused || m_bHoverPaused; }

		void SetRollDirection(int nDirect);
		int GetRollDirection() const { return m_nRollDirection; }
		void SetRollInterval(LONG lTimeSpan);
		LONG GetRollInterval() const { return m_lTimeSpan; }
		void SetRollDuration(LONG lSeconds);
		LONG GetRollDuration() const { return m_lMaxTimeLimited; }
		void SetRollStep(int nStep);
		int GetRollStep() const { return m_nStep; }
		/// 循环次数；0=一直滚，N=完整滚过 N 圈后停止并发 textrollend
		void SetRollLoop(int nLoop);
		int GetRollLoop() const { return m_nLoopLimit; }
		int GetRollLoopDone() const { return m_nLoopDone; }
		void SetPauseOnHover(bool bPause);
		bool IsPauseOnHover() const { return m_bPauseOnHover; }
		void SetAutoRoll(bool bAuto);
		bool IsAutoRoll() const { return m_bAutoRoll; }

	private:
		static int ParseDirection(LPCTSTR pstrValue);
		static bool ParseBoolAttr(LPCTSTR pstrValue);
		void RestartRollIfNeeded();
		void FinishRoll();
		void ApplyPauseTimers();

		int m_nStep;
		int m_nScrollPos;
		BOOL m_bUseRoll;
		bool m_bAutoRoll;
		bool m_bPendingRollEnd;
		bool m_bPauseOnHover;
		bool m_bHoverPaused;
		bool m_bPaused;
		int m_nRollDirection;
		LONG m_lTimeSpan;
		LONG m_lMaxTimeLimited;
		DWORD m_dwDurationDeadline;
		int m_nLoopLimit;
		int m_nLoopDone;
		int m_nText_W_H;
	};

}	// namespace DuiLib

#endif // __UITEXTSCROLLH__
