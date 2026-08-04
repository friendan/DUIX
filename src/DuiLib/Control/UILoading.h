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
	LoadingChase       // 圆周追逐点
};

class UILIB_API CLoadingUI : public CControlUI
{
	DECLARE_DUICONTROL(CLoadingUI)

	enum TIMEID
	{
		kTimerLoadingId = 100,
	};
public:
	CLoadingUI();
	virtual ~CLoadingUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	void SetLoadingType(LoadingType t);
	LoadingType GetLoadingType() const;
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void Start();
	void Stop();
	bool IsStopped() const;

protected:
	virtual void PaintBackgroundImage(IRenderContext& ctx);
	virtual void DoEvent(TEventUI& event);
	virtual void Init();

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
};

CControlUI* CreateLoadingControl(LPCTSTR pstrType);

}

#endif //__UILoadingCircle_H
