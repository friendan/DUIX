#ifndef __UISKELETON_H__
#define __UISKELETON_H__

#pragma once

namespace DuiLib
{
	/// 加载骨架：占位块 + 可选扫光动画。
	class UILIB_API CSkeletonUI : public CControlUI
	{
		DECLARE_DUICONTROL(CSkeletonUI)
	public:
		enum Type {
			TypeDefault = 0, // avatar + title + paragraph
			TypeAvatar,
			TypeButton,
			TypeInput,
			TypeParagraph
		};

		CSkeletonUI();
		~CSkeletonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetActive(bool b);
		bool IsActive() const;
		void SetAvatar(bool b);
		bool IsAvatar() const;
		void SetTitle(bool b);
		bool IsTitle() const;
		void SetParagraphRows(int n);
		int GetParagraphRows() const;
		void SetType(Type t);
		Type GetType() const;
		void SetRound(int n);
		int GetRound() const;
		void SetBlockColor(DWORD dw);
		void SetHighlightColor(DWORD dw);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
		void SetVisible(bool bVisible = true);
		void SetInternVisible(bool bVisible = true);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void DoEvent(TEventUI& event);
		/// TimerQueue → UIMSG_SKELETON_TICK
		void OnAnimTick();

	protected:
		int ScaleValue(int v) const;
		void StartAnim();
		void StopAnim();
		void StartQueueTimer();
		void StopQueueTimer();
		void PaintBlock(IRenderContext& ctx, const RECT& rc);
		void PaintDefault(IRenderContext& ctx);
		void PaintAvatarOnly(IRenderContext& ctx);
		void PaintButton(IRenderContext& ctx);
		void PaintInput(IRenderContext& ctx);
		void PaintParagraphOnly(IRenderContext& ctx);

	protected:
		Type m_eType;
		bool m_bActive;
		bool m_bAvatar;
		bool m_bTitle;
		int m_nParagraph;
		int m_nRound;
		int m_nPhase; // 0..100 扫光进度
		DWORD m_dwBlockColor;
		DWORD m_dwHighlightColor;
		HANDLE m_hQueueTimer;
		static const UINT kSkeletonTickMs = 40;
	};

	void DuiLib_SkeletonOnQueueTick(CSkeletonUI* pSkeleton);
}

#endif // __UISKELETON_H__
