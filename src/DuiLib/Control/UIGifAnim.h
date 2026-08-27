#ifndef GifAnimUI_h__
#define GifAnimUI_h__

#pragma once

namespace DuiLib
{
	class UILIB_API CGifAnimUI : public CControlUI
	{
		DECLARE_DUICONTROL(CGifAnimUI)
	public:
		CGifAnimUI(void);
		~CGifAnimUI(void);

		LPCTSTR	GetClass() const;
		LPVOID	GetInterface(LPCTSTR pstrName);
		void	DoInit();
		bool	DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void	DoEvent(TEventUI& event);
		void	SetVisible(bool bVisible = true );
		void	SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void	SetBackgroundImage(LPCTSTR pStrImage);
		LPCTSTR GetBackgroundImage();

		void	SetAutoPlay(bool bIsAuto = true );
		bool	IsAutoPlay() const;
		void	SetAutoSize(bool bIsAuto = true );
		bool	IsAutoSize() const;
		void	PlayGif();
		void	PauseGif();
		void	StopGif();
		/// TimerQueue → UIMSG_GIFANIM_TICK
		void	OnAnimTick();

	private:
		void	InitGifImage();
		void	DeleteGif();
		void	StartQueueTimer(UINT uElapse);
		void	StopQueueTimer();
		UINT	GetFrameDelayMs() const;
		void	DrawFrame( IRenderContext& ctx );		// 绘制GIF每帧

	private:
		Gdiplus::Image	*m_pGifImage;
		UINT			m_nFrameCount;				// gif图片总帧数
		UINT			m_nFramePosition;			// 当前放到第几帧
		Gdiplus::PropertyItem*	m_pPropertyItem;	// 帧与帧之间间隔时间

		CDuiString		m_sBackgroundImage;
		bool			m_bIsAutoPlay;				// 是否自动播放gif
		bool			m_bIsAutoSize;				// 是否自动根据图片设置大小
		bool			m_bIsPlaying;
		HANDLE			m_hQueueTimer;
	};

	void DuiLib_GifAnimOnQueueTick(CGifAnimUI* pGif);
}

#endif // GifAnimUI_h__
