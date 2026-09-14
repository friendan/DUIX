#ifndef __UILoadingCircle_H
#define __UILoadingCircle_H

namespace DuiLib
{

/// Loading 图形类型（XML：type / variant）
enum LoadingType
{
	LoadingSpoke = 0,  // 辐条
	LoadingCss,        // CSS：灰圈 + 彩头
	LoadingGap,        // 硬缺口 C 形
	LoadingFade,       // 渐隐拖尾
	LoadingArc,        // 短弧
	LoadingDots,       // 三点依次亮
	LoadingWave,       // 正弦波浪横移
	LoadingBars,       // 柱状信号
	LoadingDrop,       // 水滴溅开（偏写实）
	LoadingDrip,       // 水滴溅开（几何卡通）
	LoadingStars,      // 满天星闪烁
	LoadingStar,       // 单星呼吸
	LoadingDog,        // Lucide/Tabler dog 蹦跳
	LoadingFish,       // IconPark fish-one 游动
	LoadingPulse,      // 脉冲圆
	LoadingChase,      // 圆周追逐点
	LoadingWait        // 字符转圈 |/-\\ + 可选「等待中 / 已等待 Ns」
};

class UILIB_API CLoadingUI : public CControlUI
{
	DECLARE_DUICONTROL(CLoadingUI)

	enum TIMEID
	{
		kTimerLoadingId = 0x4C444731, // 'LDG1'（本地 ID；Win32 定时走 TimerQueue）
	};
public:
	CLoadingUI();
	virtual ~CLoadingUI();

	LPCTSTR GetClass() const override;
	LPVOID GetInterface(LPCTSTR pstrName) override;

	void SetLoadingType(LoadingType t);
	LoadingType GetLoadingType() const;
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
	void Start();
	void Stop();
	bool IsStopped() const;
	/// TimerQueue → UIMSG_LOADING_TICK 回调（UIManager 派发）
	void OnAnimTick();

	void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
	void SetVisible(bool bVisible = true) override;
	void SetInternVisible(bool bVisible = true) override;

	/// wait：是否显示文案（默认 true）；false 时仅字符转圈
	void SetWaitTextVisible(bool bVisible);
	bool IsWaitTextVisible() const { return m_bWaitTextVisible; }
	/// wait：满多少秒后从「等待中」切到「已等待 Ns」（默认 2）
	void SetWaitTextThresholdSec(int nSec);
	int GetWaitTextThresholdSec() const { return m_nWaitTextThresholdSec; }
	void SetWaitPendingText(LPCTSTR pstrText);
	LPCTSTR GetWaitPendingText() const { return m_sWaitPendingText.GetData(); }
	/// 已等待文案格式，需含一个 %d（默认「已等待 %ds」）
	void SetWaitElapsedFormat(LPCTSTR pstrFormat);
	LPCTSTR GetWaitElapsedFormat() const { return m_sWaitElapsedFormat.GetData(); }
	/// 转圈字符序列，每字符一帧；默认 `|/-\\`，可设 `ABCD` / `3210` 等
	void SetWaitFrames(LPCTSTR pstrFrames);
	LPCTSTR GetWaitFrames() const { return m_sWaitFrames.GetData(); }
	/// 自 Start/ResetWaitClock 起已经过的秒数
	int GetWaitElapsedSec() const;
	/// 当前完整展示串（含转圈字符与可选文案）
	CDuiString GetWaitDisplayText() const;
	/// 重置等待计时（不停止动画）
	void ResetWaitClock();

protected:
	void PaintBackgroundImage(IRenderContext& ctx) override;
	void DoEvent(TEventUI& event) override;
	void Init() override;

	void EnsureSpokeData();
	void ClearSpokeData();
	Gdiplus::Color* GenerateColorsPallet(Gdiplus::Color base, bool shade, int nb);

	void DestroySpinBmps();
	void EnsureSpinBmps(int w, int h);
	void BuildGapBmp(Gdiplus::Bitmap* bmp, int w, int h, float sweep);
	void BuildFadeBmp(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildSpokeBmp(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildCssTrackBmp(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildCssHeadBmp(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildDotsFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildWaveFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildBarsFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildDropFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildDripFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildStarsFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildStarFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildDogFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildFishFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildPulseFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void BuildChaseFrame(Gdiplus::Bitmap* bmp, int w, int h);
	void GetRingBounds(int w, int h, Gdiplus::REAL& x, Gdiplus::REAL& y, Gdiplus::REAL& size, int& thick) const;
	int ResolveThick(int side) const;
	void TickFrame();
	void RestartTimer();
	void StartQueueTimer();
	void StopQueueTimer();
	void PaintWait(IRenderContext& ctx);
	CDuiString BuildWaitBodyText() const;
	DWORD WaitTextColorDui() const;
	int WaitFrameCount() const;
	TCHAR WaitFrameChar() const;

	BYTE ColorA() const;
	Gdiplus::Color MakeColor(BYTE a) const;

protected:
	LoadingType m_eType;
	int m_nTime;      // 帧间隔 ms
	int m_nDuration;  // 转一圈时长 ms
	bool m_bStop;
	float m_fAngle;

	int m_NumberOfSpoke;
	int m_SpokeThickness;
	int m_OuterCircleRadius;
	int m_InnerCircleRadius;
	Gdiplus::PointF m_CenterPoint;
	Gdiplus::Color m_Color;
	Gdiplus::Color m_TrackColor;

	Gdiplus::Color* m_Colors;
	double* m_Angles;

	// 静态图 + 旋转（与 Ring 相同，避免 D2D 按指针缓存脏帧）
	Gdiplus::Bitmap* m_pSpinBmp;
	Gdiplus::Bitmap* m_pTrackBmp; // CSS 灰轨（不转）
	int m_nBmpW;
	int m_nBmpH;
	LoadingType m_eBmpType;
	bool m_bMorphType; // dots/wave/bars/drop/drip/…：每帧重画
	HANDLE m_hQueueTimer; // CreateTimerQueueTimer（绕开 Shadow 子类化下 WM_TIMER 丢失）

	// wait
	bool m_bWaitTextVisible;
	int m_nWaitTextThresholdSec;
	int m_nWaitFrame;
	ULONGLONG m_ullWaitStart;
	CDuiString m_sWaitPendingText;
	CDuiString m_sWaitElapsedFormat;
	CDuiString m_sWaitFrames;
};

/// UIManager 处理 UIMSG_LOADING_TICK 时调用（避免 Core 依赖 Control 头文件）
void DuiLib_LoadingOnQueueTick(CLoadingUI* pLoad);

CControlUI* CreateLoadingControl(LPCTSTR pstrType);

}

#endif //__UILoadingCircle_H
