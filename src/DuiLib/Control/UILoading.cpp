#include "stdafx.h"
#include "UILoading.h"
#include "UILucideIcons.h"
#include "UITablerIcons.h"
#include "UIIconParkIcons.h"
#include <Gdiplus.h>
#include <lunasvg.h>
#include <math.h>
#include <string>

using namespace DuiLib;
using namespace Gdiplus;

namespace
{
	const double kPi = 3.14159265358979323846;

	void DrawSpokeLine(Graphics* g, PointF a, PointF b, Color c, int thickness)
	{
		SolidBrush brush(c);
		Pen pen(&brush, (REAL)thickness);
		pen.SetStartCap(LineCapRound);
		pen.SetEndCap(LineCapRound);
		g->DrawLine(&pen, a, b);
	}

	PointF Polar(PointF center, double radius, double angleDeg)
	{
		double rad = kPi * angleDeg / 180.0;
		return PointF(
			center.X + (REAL)(radius * cos(rad)),
			center.Y + (REAL)(radius * sin(rad)));
	}

	double* MakeSpokeAngles(int n)
	{
		if( n <= 0 ) return NULL;
		double* a = new double[n];
		double step = 360.0 / n;
		for( int i = 0; i < n; ++i )
			a[i] = -90.0 + step * i;
		return a;
	}

	float WrapDeg(float a)
	{
		while( a >= 360.0f ) a -= 360.0f;
		while( a < 0.0f ) a += 360.0f;
		return a;
	}

	bool IsMorphType(LoadingType t)
	{
		return t == LoadingDots || t == LoadingWave || t == LoadingBars
			|| t == LoadingDrop || t == LoadingDrip
			|| t == LoadingStars || t == LoadingStar || t == LoadingDog || t == LoadingFish
			|| t == LoadingPulse || t == LoadingChase;
	}

	float EaseOutCubic(float t)
	{
		if( t < 0.0f ) t = 0.0f;
		if( t > 1.0f ) t = 1.0f;
		float u = 1.0f - t;
		return 1.0f - u * u * u;
	}

	float EaseInQuad(float t)
	{
		if( t < 0.0f ) t = 0.0f;
		if( t > 1.0f ) t = 1.0f;
		return t * t;
	}

	// 经典五角星（尖角朝上）
	void FillStar5(Graphics& g, REAL cx, REAL cy, REAL outer, Color c)
	{
		if( outer < 1.5f ) return;
		REAL inner = outer * 0.38f;
		PointF pts[10];
		for( int k = 0; k < 10; ++k ) {
			double ang = -90.0 + k * 36.0;
			double rr = (k % 2 == 0) ? outer : inner;
			pts[k] = Polar(PointF(cx, cy), rr, ang);
		}
		SolidBrush br(c);
		g.FillPolygon(&br, pts, 10);
	}
}

IMPLEMENT_DUICONTROL(CLoadingUI)

CLoadingUI::CLoadingUI()
	: m_eType(LoadingSpoke)
	, m_nTime(16)
	, m_nDuration(1200)
	, m_bStop(true)
	, m_fAngle(0.0f)
	, m_NumberOfSpoke(12)
	, m_SpokeThickness(0)
	, m_OuterCircleRadius(0)
	, m_InnerCircleRadius(0)
	, m_Colors(NULL)
	, m_Angles(NULL)
	, m_pSpinBmp(NULL)
	, m_pTrackBmp(NULL)
	, m_nBmpW(0)
	, m_nBmpH(0)
	, m_eBmpType(LoadingSpoke)
	, m_bMorphType(false)
{
	m_CenterPoint = PointF(0, 0);
	// 默认跟当前主题 primary；无主题时中性灰，避免首帧钉死 #1677FF
	DWORD dwSpin = 0x808080FF;
	DWORD dwTrack = 0x0000001A;
	CThemeManager* pTm = CThemeManager::GetInstance();
	if( pTm != NULL ) {
		CTheme* pTh = pTm->GetCurrentTheme();
		if( pTh == NULL ) pTh = pTm->FindTheme(pTm->GetDefaultThemeId());
		if( pTh != NULL ) {
			dwSpin = pTh->GetToken(_T("color-primary"), dwSpin);
			dwTrack = pTh->GetToken(_T("color-border"), dwTrack);
		}
	}
	m_Color = Color(DuiColorA(dwSpin), DuiColorR(dwSpin), DuiColorG(dwSpin), DuiColorB(dwSpin));
	m_TrackColor = Color(DuiColorA(dwTrack), DuiColorR(dwTrack), DuiColorG(dwTrack), DuiColorB(dwTrack));
	SetKind(CONTROLKIND_NONE);
	SetMouseEnabled(false);
}

CLoadingUI::~CLoadingUI()
{
	Stop();
	ClearSpokeData();
	DestroySpinBmps();
}

LPCTSTR CLoadingUI::GetClass() const
{
	return DUI_CTR_LOADINGCIRCLE;
}

LPVOID CLoadingUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcsicmp(pstrName, DUI_CTR_LOADINGCIRCLE) == 0 ) return static_cast<CLoadingUI*>(this);
	if( _tcsicmp(pstrName, _T("Loading")) == 0 ) return static_cast<CLoadingUI*>(this);
	return CControlUI::GetInterface(pstrName);
}

void CLoadingUI::SetLoadingType(LoadingType t)
{
	if( m_eType == t ) return;
	m_eType = t;
	m_fAngle = 0.0f;
	m_bMorphType = IsMorphType(t);
	DestroySpinBmps();
	if( m_eType == LoadingSpoke ) EnsureSpokeData();
	Invalidate();
}

LoadingType CLoadingUI::GetLoadingType() const
{
	return m_eType;
}

bool CLoadingUI::IsStopped() const
{
	return m_bStop;
}

BYTE CLoadingUI::ColorA() const
{
	BYTE a = m_Color.GetA();
	return a ? a : 255;
}

Gdiplus::Color CLoadingUI::MakeColor(BYTE a) const
{
	return Color(a, m_Color.GetR(), m_Color.GetG(), m_Color.GetB());
}

int CLoadingUI::ResolveThick(int side) const
{
	if( m_SpokeThickness > 0 ) return m_SpokeThickness;
	int t = side / 10;
	if( t < 3 ) t = 3;
	if( t > 8 ) t = 8;
	return t;
}

void CLoadingUI::ClearSpokeData()
{
	if( m_Angles ) { delete[] m_Angles; m_Angles = NULL; }
	if( m_Colors ) { delete[] m_Colors; m_Colors = NULL; }
}

void CLoadingUI::EnsureSpokeData()
{
	ClearSpokeData();
	if( m_NumberOfSpoke <= 0 ) m_NumberOfSpoke = 12;
	m_Angles = MakeSpokeAngles(m_NumberOfSpoke);
	m_Colors = GenerateColorsPallet(m_Color, true, m_NumberOfSpoke);
}

Gdiplus::Color* CLoadingUI::GenerateColorsPallet(Color base, bool shade, int /*nb*/)
{
	Color* colors = new Color[m_NumberOfSpoke];
	for( int i = 0; i < m_NumberOfSpoke; ++i ) {
		if( !shade ) {
			colors[i] = base;
			continue;
		}
		float t = 1.0f - (float)i / (float)m_NumberOfSpoke;
		BYTE a = (BYTE)(80 + t * t * (base.GetA() ? base.GetA() : 255 - 80));
		colors[i] = Color(a, base.GetR(), base.GetG(), base.GetB());
	}
	return colors;
}

void CLoadingUI::DestroySpinBmps()
{
	if( m_pSpinBmp || m_pTrackBmp ) {
		IRenderDevice* pDev = GetRenderDevice();
		if( pDev && pDev->GetBackendKind() == DUILIB_RENDER_D2D ) {
			CD2dRenderDevice* pD2d = static_cast<CD2dRenderDevice*>(pDev);
			if( m_pSpinBmp ) pD2d->InvalidateBitmapCacheForImage(NULL, m_pSpinBmp);
			if( m_pTrackBmp ) pD2d->InvalidateBitmapCacheForImage(NULL, m_pTrackBmp);
		}
	}
	if( m_pSpinBmp ) { delete m_pSpinBmp; m_pSpinBmp = NULL; }
	if( m_pTrackBmp ) { delete m_pTrackBmp; m_pTrackBmp = NULL; }
	m_nBmpW = m_nBmpH = 0;
}

void CLoadingUI::GetRingBounds(int w, int h, REAL& x, REAL& y, REAL& size, int& thick) const
{
	int side = w < h ? w : h;
	thick = ResolveThick(side);
	int pad = thick + 1;
	int s = side - pad * 2;
	if( s < 8 ) s = 8;
	size = (REAL)s;
	x = (REAL)((w - s) / 2);
	y = (REAL)((h - s) / 2);
}

void CLoadingUI::BuildGapBmp(Bitmap* bmp, int w, int h, float sweep)
{
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	REAL x, y, size;
	int thick = 3;
	GetRingBounds(w, h, x, y, size, thick);
	Pen pen(MakeColor(ColorA()), (REAL)thick);
	pen.SetStartCap(LineCapRound);
	pen.SetEndCap(LineCapRound);
	// 固定画一段弧；缺口位置靠整图旋转改变
	g.DrawArc(&pen, x, y, size, size, -90.0f, sweep);
}

void CLoadingUI::BuildFadeBmp(Bitmap* bmp, int w, int h)
{
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	REAL x, y, size;
	int thick = 3;
	GetRingBounds(w, h, x, y, size, thick);
	const int segs = 48;
	const float totalSweep = 300.0f;
	const float segSweep = totalSweep / (float)segs;
	const float base = -90.0f - totalSweep;
	for( int i = 0; i < segs; ++i ) {
		float t = (float)(i + 1) / (float)segs;
		BYTE a = (BYTE)(ColorA() * t * t);
		if( a < 12 ) continue;
		Pen pen(MakeColor(a), (REAL)thick);
		pen.SetStartCap(LineCapRound);
		pen.SetEndCap(LineCapRound);
		g.DrawArc(&pen, x, y, size, size, base + i * segSweep, segSweep + 1.2f);
	}
}

void CLoadingUI::BuildSpokeBmp(Bitmap* bmp, int w, int h)
{
	if( m_Angles == NULL || m_Colors == NULL )
		EnsureSpokeData();
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	PointF center((REAL)w / 2, (REAL)h / 2);
	int side = w < h ? w : h;
	int outer = m_OuterCircleRadius > 0 ? m_OuterCircleRadius : (side / 2 - 2);
	int inner = m_InnerCircleRadius > 0 ? m_InnerCircleRadius : (outer * 55 / 100);
	if( outer < 4 ) outer = 4;
	if( inner >= outer ) inner = outer - 2;
	int thick = ResolveThick(side);
	if( thick > (outer - inner) ) thick = outer - inner;
	if( thick < 2 ) thick = 2;
	for( int i = 0; i < m_NumberOfSpoke; ++i ) {
		DrawSpokeLine(&g,
			Polar(center, inner, m_Angles[i]),
			Polar(center, outer, m_Angles[i]),
			m_Colors[i], thick);
	}
}

void CLoadingUI::BuildCssTrackBmp(Bitmap* bmp, int w, int h)
{
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	REAL x, y, size;
	int thick = 3;
	GetRingBounds(w, h, x, y, size, thick);
	Pen track(m_TrackColor, (REAL)thick);
	g.DrawEllipse(&track, x, y, size, size);
}

void CLoadingUI::BuildCssHeadBmp(Bitmap* bmp, int w, int h)
{
	BuildGapBmp(bmp, w, h, 75.0f);
}

void CLoadingUI::BuildDotsFrame(Bitmap* bmp, int w, int h)
{
	// 三点依次高亮：透明度 + 半径随相位错开切换
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	int side = w < h ? w : h;
	int baseR = side / 10;
	if( baseR < 5 ) baseR = 5;
	if( baseR > 10 ) baseR = 10;
	int gap = baseR * 2 + 4;
	int total = baseR * 2 * 3 + gap * 2;
	REAL baseX = (REAL)((w - total) / 2 + baseR);
	REAL cy = (REAL)(h / 2);
	double phase = (m_fAngle / 360.0) * 2.0 * kPi;
	for( int i = 0; i < 3; ++i ) {
		double t = 0.5 + 0.5 * sin(phase + i * (2.0 * kPi / 3.0)); // 0..1，错开 120°
		REAL r = (REAL)(baseR * (0.55 + 0.45 * t));
		BYTE a = (BYTE)(70 + (int)(t * (ColorA() - 70)));
		SolidBrush br(MakeColor(a));
		REAL cx = baseX + (REAL)(i * (baseR * 2 + gap));
		g.FillEllipse(&br, cx - r, cy - r, r * 2, r * 2);
	}
}

void CLoadingUI::BuildWaveFrame(Bitmap* bmp, int w, int h)
{
	// 低振幅波形轨道 + 亮段沿线流动（避免粗蛇身横爬）
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 8 || h < 4 ) return;

	int thick = ResolveThick(h);
	if( thick < 2 ) thick = 2;
	if( thick > 4 ) thick = 4;
	REAL midY = (REAL)h * 0.5f;
	REAL amp = (REAL)h * 0.18f; // 低振幅，不像蛇身扭动
	const double cycles = 2.2;
	const int n = (w > 2) ? w : 2;

	PointF* pts = new PointF[n];
	for( int x = 0; x < n; ++x ) {
		double t = (double)x / (double)(n - 1);
		double y = midY + amp * sin(t * 2.0 * kPi * cycles); // 底波完全固定
		pts[x] = PointF((REAL)x, (REAL)y);
	}

	// 淡底轨
	{
		Pen track(MakeColor((BYTE)(ColorA() * 0.28)), (REAL)thick);
		track.SetStartCap(LineCapRound);
		track.SetEndCap(LineCapRound);
		track.SetLineJoin(LineJoinRound);
		g.DrawCurve(&track, pts, n, 0.5f);
	}

	// 亮段：沿曲线取一段连续点，相位驱动位置
	const float win = 0.28f; // 亮段约占整宽 28%
	float head = (float)(m_fAngle / 360.0); // 0..1
	int count = 0;
	PointF* seg = new PointF[n];
	for( int x = 0; x < n; ++x ) {
		float t = (float)x / (float)(n - 1);
		float d = t - head;
		if( d < -0.5f ) d += 1.0f;
		if( d > 0.5f ) d -= 1.0f;
		if( d >= -win && d <= 0.0f ) {
			// 段内渐亮：靠近 head 最亮
			seg[count++] = pts[x];
		}
	}
	if( count >= 2 ) {
		Pen glow(MakeColor((BYTE)(ColorA() * 0.45)), (REAL)(thick + 2));
		glow.SetStartCap(LineCapRound);
		glow.SetEndCap(LineCapRound);
		glow.SetLineJoin(LineJoinRound);
		g.DrawCurve(&glow, seg, count, 0.5f);

		Pen headPen(MakeColor(ColorA()), (REAL)thick);
		headPen.SetStartCap(LineCapRound);
		headPen.SetEndCap(LineCapRound);
		headPen.SetLineJoin(LineJoinRound);
		g.DrawCurve(&headPen, seg, count, 0.5f);
	}
	delete[] seg;
	delete[] pts;
}

void CLoadingUI::BuildBarsFrame(Bitmap* bmp, int w, int h)
{
	// 实心信号柱（类似 WiFi/蜂窝信号条）：底对齐、高度递增，依次点亮
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 4 || h < 4 ) return;

	int n = m_NumberOfSpoke;
	if( n < 3 ) n = 4;
	if( n > 8 ) n = 8;

	int gap = (w > 80) ? 5 : 4;
	int barW = (w - gap * (n - 1)) / n;
	if( barW < 4 ) barW = 4;
	if( barW > 14 ) barW = 14;
	int used = barW * n + gap * (n - 1);
	REAL x0 = (REAL)((w - used) / 2);
	REAL bottom = (REAL)h - 2.0f;
	REAL maxH = (REAL)h - 6.0f;
	if( maxH < 8.0f ) maxH = 8.0f;

	float head = (m_fAngle / 360.0f) * (float)n;

	for( int i = 0; i < n; ++i ) {
		float level = (float)(i + 1) / (float)n;
		REAL hh = maxH * (0.28f + 0.72f * level);
		REAL x = x0 + (REAL)(i * (barW + gap));
		REAL y = bottom - hh;

		float dist = fabsf(head - (float)i);
		if( dist > n * 0.5f ) dist = (float)n - dist;
		float t = 1.0f - dist;
		if( t < 0.0f ) t = 0.0f;
		t = t * t;
		BYTE a = (BYTE)(55 + (int)(t * (ColorA() - 55)));

		SolidBrush br(MakeColor(a));
		g.FillRectangle(&br, x, y, (REAL)barW, hh);
	}
}

void CLoadingUI::BuildDropFrame(Bitmap* bmp, int w, int h)
{
	// 偏写实：水滴下落 → 压扁 → 涟漪 + 飞溅水珠
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 8 || h < 8 ) return;

	float p = m_fAngle / 360.0f;
	REAL cx = (REAL)w * 0.5f;
	REAL ground = (REAL)h * 0.82f;
	REAL dropR = (REAL)((w < h ? w : h) * 0.11);
	if( dropR < 5.0f ) dropR = 5.0f;

	const float tFall = 0.42f;
	const float tSquash = 0.52f;

	if( p < tFall ) {
		float t = EaseInQuad(p / tFall);
		REAL y = (REAL)h * 0.08f + (ground - dropR - (REAL)h * 0.08f) * t;
		// 水滴：上尖下圆
		GraphicsPath path;
		path.AddEllipse(cx - dropR * 0.85f, y, dropR * 1.7f, dropR * 1.7f);
		PointF tip[3] = {
			PointF(cx, y - dropR * 0.55f),
			PointF(cx - dropR * 0.55f, y + dropR * 0.35f),
			PointF(cx + dropR * 0.55f, y + dropR * 0.35f)
		};
		path.AddPolygon(tip, 3);
		SolidBrush br(MakeColor(ColorA()));
		g.FillPath(&br, &path);
		// 高光
		SolidBrush hi(MakeColor((BYTE)(ColorA() * 0.35)));
		g.FillEllipse(&hi, cx - dropR * 0.35f, y + dropR * 0.25f, dropR * 0.55f, dropR * 0.45f);
	}
	else if( p < tSquash ) {
		float t = (p - tFall) / (tSquash - tFall);
		REAL sx = dropR * (1.0f + 0.85f * t);
		REAL sy = dropR * (1.0f - 0.55f * t);
		SolidBrush br(MakeColor(ColorA()));
		g.FillEllipse(&br, cx - sx, ground - sy * 1.2f, sx * 2, sy * 1.6f);
	}
	else {
		float t = (p - tSquash) / (1.0f - tSquash);
		float te = EaseOutCubic(t);
		BYTE fade = (BYTE)(ColorA() * (1.0f - t * 0.85f));

		// 涟漪环
		for( int i = 0; i < 3; ++i ) {
			float delay = i * 0.12f;
			float rt = (t - delay) / (1.0f - delay);
			if( rt <= 0.0f || rt > 1.0f ) continue;
			REAL rr = dropR * (1.2f + rt * 4.5f);
			BYTE a = (BYTE)(fade * (1.0f - rt));
			if( a < 8 ) continue;
			Pen pen(MakeColor(a), 1.8f + (1.0f - rt));
			g.DrawEllipse(&pen, cx - rr, ground - rr * 0.28f, rr * 2, rr * 0.56f);
		}

		// 飞溅水珠
		for( int i = 0; i < 7; i++ ) {
			double ang = (-70.0 + i * 23.0) * kPi / 180.0;
			float speed = 0.55f + (i % 3) * 0.12f;
			REAL dx = (REAL)(cos(ang) * te * w * 0.28 * speed);
			REAL dy = (REAL)(sin(ang) * te * h * 0.38 * speed) - te * (1.0f - te) * h * 0.22f;
			REAL pr = dropR * (0.28f - 0.12f * t);
			if( pr < 1.5f ) pr = 1.5f;
			BYTE a = (BYTE)(fade * (0.9f - t * 0.5f));
			SolidBrush br(MakeColor(a));
			g.FillEllipse(&br, cx + dx - pr, ground + dy - pr, pr * 2, pr * 2);
		}
	}
}

void CLoadingUI::BuildDripFrame(Bitmap* bmp, int w, int h)
{
	// 几何卡通：圆+三角水滴，触地弹开简洁水花
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 8 || h < 8 ) return;

	float p = m_fAngle / 360.0f;
	REAL cx = (REAL)w * 0.5f;
	REAL ground = (REAL)h * 0.78f;
	REAL r = (REAL)((w < h ? w : h) * 0.13);
	if( r < 6.0f ) r = 6.0f;

	const float tFall = 0.40f;
	const float tSquash = 0.50f;

	SolidBrush br(MakeColor(ColorA()));

	if( p < tFall ) {
		float t = EaseInQuad(p / tFall);
		REAL y = (REAL)h * 0.06f + (ground - r * 1.6f - (REAL)h * 0.06f) * t;
		PointF tip[3] = {
			PointF(cx, y),
			PointF(cx - r * 0.75f, y + r * 1.1f),
			PointF(cx + r * 0.75f, y + r * 1.1f)
		};
		g.FillPolygon(&br, tip, 3);
		g.FillEllipse(&br, cx - r, y + r * 0.55f, r * 2, r * 2);
	}
	else if( p < tSquash ) {
		float t = (p - tFall) / (tSquash - tFall);
		REAL bw = r * (2.0f + 1.4f * t);
		REAL bh = r * (1.2f - 0.7f * t);
		g.FillEllipse(&br, cx - bw * 0.5f, ground - bh, bw, bh);
		// 地面短横
		Pen line(MakeColor((BYTE)(ColorA() * 0.5)), 2.0f);
		g.DrawLine(&line, cx - bw * 0.7f, ground + 1, cx + bw * 0.7f, ground + 1);
	}
	else {
		float t = (p - tSquash) / (1.0f - tSquash);
		float te = EaseOutCubic(t);
		BYTE fade = (BYTE)(ColorA() * (1.0f - t));

		// 简洁圆弧水花
		for( int i = 0; i < 2; ++i ) {
			float rt = (t - i * 0.15f);
			if( rt <= 0.0f ) continue;
			if( rt > 1.0f ) rt = 1.0f;
			REAL rr = r * (1.5f + rt * 3.2f);
			Pen pen(MakeColor((BYTE)(fade * (1.0f - rt * 0.7f))), 2.5f);
			pen.SetStartCap(LineCapRound);
			pen.SetEndCap(LineCapRound);
			g.DrawArc(&pen, cx - rr, ground - rr * 0.55f, rr * 2, rr * 1.1f, 200.0f, 140.0f);
		}

		// 对称小圆水珠
		for( int i = 0; i < 5; ++i ) {
			double ang = (-50.0 - i * 20.0) * kPi / 180.0;
			REAL dist = te * (r * (2.2f + i * 0.35f));
			REAL dx = (REAL)(cos(ang) * dist * ((i % 2) ? 1.0 : -1.0));
			if( i == 2 ) dx = 0;
			else if( i < 2 ) dx = (REAL)(-fabs(cos(ang)) * dist);
			else dx = (REAL)(fabs(cos(ang)) * dist);
			REAL dy = (REAL)(sin(ang) * dist) - te * (1.0f - te) * r * 2.5f;
			REAL pr = r * (0.35f - 0.15f * t);
			SolidBrush b2(MakeColor(fade));
			g.FillEllipse(&b2, cx + dx - pr, ground + dy - pr, pr * 2, pr * 2);
		}
	}
}

void CLoadingUI::BuildStarsFrame(Bitmap* bmp, int w, int h)
{
	// 满天星：更大的五角星，错相闪烁
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 8 || h < 8 ) return;

	struct StarPos { float x, y, size, phase; };
	static const StarPos kStars[] = {
		{ 0.22f, 0.30f, 0.85f, 0.0f },
		{ 0.72f, 0.24f, 0.70f, 1.1f },
		{ 0.50f, 0.52f, 1.15f, 2.2f },
		{ 0.28f, 0.74f, 0.78f, 3.0f },
		{ 0.76f, 0.70f, 0.90f, 4.1f },
		{ 0.48f, 0.18f, 0.55f, 5.0f },
	};
	const int n = (int)(sizeof(kStars) / sizeof(kStars[0]));
	double phase = (m_fAngle / 360.0) * 2.0 * kPi;
	REAL unit = (REAL)((w < h ? w : h) * 0.16);

	for( int i = 0; i < n; ++i ) {
		float tw = (float)(0.5 + 0.5 * sin(phase + kStars[i].phase));
		tw = tw * tw;
		BYTE a = (BYTE)(55 + (int)(tw * (ColorA() - 55)));
		REAL cx = (REAL)w * kStars[i].x;
		REAL cy = (REAL)h * kStars[i].y;
		REAL outer = unit * kStars[i].size * (0.75f + 0.40f * tw);
		FillStar5(g, cx, cy, outer, MakeColor(a));
	}
}

void CLoadingUI::BuildStarFrame(Bitmap* bmp, int w, int h)
{
	// 单颗经典五角星呼吸（无十字芒，避免背后像加号）
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 8 || h < 8 ) return;

	REAL cx = (REAL)w * 0.5f;
	REAL cy = (REAL)h * 0.5f;
	double phase = (m_fAngle / 360.0) * 2.0 * kPi;
	float breath = (float)(0.5 + 0.5 * sin(phase));

	REAL side = (REAL)(w < h ? w : h);
	REAL outer = side * (0.32f + 0.10f * breath);
	BYTE a = (BYTE)(180 + (int)(breath * (ColorA() - 180)));
	if( a > ColorA() ) a = ColorA();

	FillStar5(g, cx, cy, outer, MakeColor(a));
	FillStar5(g, cx, cy, outer * 0.40f, MakeColor((BYTE)(ColorA() * 0.35)));
}

void CLoadingUI::BuildDogFrame(Bitmap* bmp, int w, int h)
{
	// 用 SvgBox 同款 Lucide/Tabler dog 图标 + 蹦跳，一眼能认出是狗
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 12 || h < 12 ) return;

	const char* svg = LucideIcons::GetIcon(_T("dog"));
	if( svg == NULL || *svg == '\0' )
		svg = TablerOutlineIcons::GetIcon(_T("dog"));
	if( svg == NULL || *svg == '\0' ) return;

	std::unique_ptr<lunasvg::Document> doc = lunasvg::Document::loadFromData(svg);
	if( !doc ) return;

	char style[160];
	sprintf_s(style, sizeof(style),
		"* { fill: none; stroke: #%02x%02x%02x; }",
		(unsigned)m_Color.GetR(), (unsigned)m_Color.GetG(), (unsigned)m_Color.GetB());
	doc->applyStyleSheet(style);

	int side = w < h ? w : h;
	int icon = (int)(side * 0.78f);
	if( icon < 20 ) icon = 20;
	lunasvg::Bitmap svgBmp = doc->renderToBitmap(icon, icon);
	if( svgBmp.isNull() ) return;

	Bitmap dogBmp(svgBmp.width(), svgBmp.height(), (INT)svgBmp.stride(),
		PixelFormat32bppPARGB, (BYTE*)svgBmp.data());

	double phase = (m_fAngle / 360.0) * 2.0 * kPi;
	// 蹦跳：离地高度 + 落地压扁
	float hop = (float)((sin(phase) + 1.0) * 0.5); // 0..1
	float lift = hop * hop; // 滞空感
	REAL bob = -(REAL)(h * 0.10f * lift);
	REAL squashY = 1.0f - (1.0f - lift) * 0.12f;
	REAL squashX = 1.0f + (1.0f - lift) * 0.08f;
	REAL tilt = (REAL)(sin(phase) * 6.0); // 轻微前倾

	REAL cx = (REAL)w * 0.5f;
	REAL cy = (REAL)h * 0.52f + bob;
	REAL dw = (REAL)icon * squashX;
	REAL dh = (REAL)icon * squashY;

	GraphicsState st = g.Save();
	g.TranslateTransform(cx, cy);
	g.RotateTransform(tilt);
	g.TranslateTransform(-dw * 0.5f, -dh * 0.55f);
	g.DrawImage(&dogBmp, 0, 0, (INT)dw, (INT)dh);
	g.Restore(st);

	// 落地尘点（蹦跳低点更明显）
	if( lift < 0.35f ) {
		BYTE ga = (BYTE)(ColorA() * (0.45f - lift));
		SolidBrush gbr(MakeColor(ga));
		REAL gy = (REAL)h * 0.88f;
		REAL spread = (1.0f - lift) * (REAL)w * 0.12f;
		g.FillEllipse(&gbr, cx - spread - (REAL)3, gy, (REAL)5, (REAL)3);
		g.FillEllipse(&gbr, cx + spread - (REAL)2, gy + (REAL)1, (REAL)4, (REAL)2);
		g.FillEllipse(&gbr, cx - (REAL)2, gy + (REAL)2, (REAL)3, (REAL)2);
	}
}

void CLoadingUI::BuildFishFrame(Bitmap* bmp, int w, int h)
{
	// IconPark fish-one（与 SvgBox iconpark="fish-one" 同款）+ 游动
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	if( w < 12 || h < 12 ) return;

	const char* svg = IconParkIcons::GetIcon(_T("fish-one"));
	if( svg == NULL || *svg == '\0' ) return;

	std::unique_ptr<lunasvg::Document> doc = lunasvg::Document::loadFromData(svg);
	if( !doc ) return;

	// 描边鱼身 + 实心眼睛：path 保持 fill:none，circle 填色
	char style[220];
	sprintf_s(style, sizeof(style),
		"* { stroke: #%02x%02x%02x; } path { fill: none; } circle { fill: #%02x%02x%02x; stroke: none; }",
		(unsigned)m_Color.GetR(), (unsigned)m_Color.GetG(), (unsigned)m_Color.GetB(),
		(unsigned)m_Color.GetR(), (unsigned)m_Color.GetG(), (unsigned)m_Color.GetB());
	doc->applyStyleSheet(style);

	int side = w < h ? w : h;
	int icon = (int)(side * 0.82f);
	if( icon < 20 ) icon = 20;
	lunasvg::Bitmap svgBmp = doc->renderToBitmap(icon, icon);
	if( svgBmp.isNull() ) return;

	Bitmap fishBmp(svgBmp.width(), svgBmp.height(), (INT)svgBmp.stride(),
		PixelFormat32bppPARGB, (BYTE*)svgBmp.data());

	double phase = (m_fAngle / 360.0) * 2.0 * kPi;
	// 左右游 + 上下轻摆 + 头随速度微倾
	REAL swimX = (REAL)(sin(phase) * w * 0.14);
	REAL bobY = (REAL)(sin(phase * 2.0) * h * 0.06);
	REAL pitch = (REAL)(cos(phase) * 10.0);
	REAL squashX = 1.0f + (REAL)(sin(phase * 2.0) * 0.04); // 轻微身体伸缩
	REAL squashY = 1.0f - (REAL)(sin(phase * 2.0) * 0.03);

	REAL cx = (REAL)w * 0.5f + swimX;
	REAL cy = (REAL)h * 0.52f + bobY;
	REAL dw = (REAL)icon * squashX;
	REAL dh = (REAL)icon * squashY;

	// 水中环境气泡：分散上浮（非尾气）
	{
		static const float kLane[] = { 0.14f, 0.30f, 0.70f, 0.86f, 0.50f };
		static const float kSpeed[] = { 0.55f, 0.72f, 0.48f, 0.68f, 0.40f };
		static const float kSize[]  = { 1.1f, 0.85f, 1.35f, 0.95f, 0.75f };
		static const float kPhase[] = { 0.10f, 0.42f, 0.68f, 0.25f, 0.88f };
		const int nBub = 5;
		Pen bubPen(MakeColor(1), 1.0f);
		for( int i = 0; i < nBub; ++i ) {
			double life = fmod(phase / (2.0 * kPi) * kSpeed[i] + kPhase[i] + 1.0, 1.0);
			// 底部淡入、顶部淡出
			float edge = 1.0f;
			if( life < 0.12f ) edge = (float)(life / 0.12);
			else if( life > 0.78f ) edge = (float)((1.0 - life) / 0.22);
			if( edge < 0.04f ) continue;
			BYTE ba = (BYTE)(ColorA() * (0.18f + 0.22f * edge));
			bubPen.SetColor(MakeColor(ba));
			REAL br = kSize[i];
			REAL bx = (REAL)w * kLane[i] + (REAL)(sin(life * kPi * 2.0 + i * 1.7) * 2.0);
			REAL by = (REAL)h * (1.05f - (REAL)life * 1.15f);
			g.DrawEllipse(&bubPen, bx - br, by - br, br * 2, br * 2);
		}
	}

	// 朝向：向右游时正向；向左时水平翻转
	bool faceRight = cos(phase) >= 0.0;
	GraphicsState st = g.Save();
	g.TranslateTransform(cx, cy);
	g.RotateTransform(pitch);
	if( !faceRight )
		g.ScaleTransform(-1.0f, 1.0f);
	g.TranslateTransform(-dw * 0.5f, -dh * 0.5f);
	g.DrawImage(&fishBmp, 0, 0, (INT)dw, (INT)dh);
	g.Restore(st);
}

void CLoadingUI::BuildPulseFrame(Bitmap* bmp, int w, int h)
{
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	PointF center((REAL)w / 2, (REAL)h / 2);
	int maxR = m_OuterCircleRadius > 0 ? m_OuterCircleRadius : ((w < h ? w : h) / 2 - 2);
	if( maxR < 4 ) maxR = 4;
	double t = (m_fAngle / 360.0) * 2.0 * kPi;
	double k = 0.55 + 0.45 * (0.5 + 0.5 * sin(t));
	REAL r = (REAL)(maxR * k);
	BYTE a = (BYTE)(60 + (int)((1.0 - k) * (ColorA() - 60)));
	SolidBrush br(MakeColor(a));
	g.FillEllipse(&br, center.X - r, center.Y - r, r * 2, r * 2);
	REAL r2 = r * 0.45f;
	SolidBrush core(MakeColor(ColorA()));
	g.FillEllipse(&core, center.X - r2, center.Y - r2, r2 * 2, r2 * 2);
}

void CLoadingUI::BuildChaseFrame(Bitmap* bmp, int w, int h)
{
	Graphics g(bmp);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.Clear(Color(0, 0, 0, 0));
	const int n = 8;
	PointF center((REAL)w / 2, (REAL)h / 2);
	int side = w < h ? w : h;
	int R = m_OuterCircleRadius > 0 ? m_OuterCircleRadius : (side / 2 - 4);
	int r = ResolveThick(side) / 2;
	if( r < 3 ) r = 3;
	float step = 360.0f / (float)n;
	for( int i = 0; i < n; ++i ) {
		float rel = WrapDeg(i * step - m_fAngle);
		float t = 1.0f - rel / 360.0f;
		BYTE a = (BYTE)(30 + t * t * (ColorA() - 30));
		PointF p = Polar(center, R, -90.0 + i * step);
		SolidBrush br(MakeColor(a));
		g.FillEllipse(&br, p.X - r, p.Y - r, (REAL)(r * 2), (REAL)(r * 2));
	}
}

void CLoadingUI::EnsureSpinBmps(int w, int h)
{
	m_bMorphType = IsMorphType(m_eType);
	bool needRebuild = (m_pSpinBmp == NULL || m_nBmpW != w || m_nBmpH != h || m_eBmpType != m_eType);
	if( needRebuild ) {
		DestroySpinBmps();
		m_pSpinBmp = new Bitmap(w, h, PixelFormat32bppPARGB);
		m_nBmpW = w;
		m_nBmpH = h;
		m_eBmpType = m_eType;

		switch( m_eType ) {
		case LoadingCss:
			m_pTrackBmp = new Bitmap(w, h, PixelFormat32bppPARGB);
			BuildCssTrackBmp(m_pTrackBmp, w, h);
			BuildCssHeadBmp(m_pSpinBmp, w, h);
			break;
		case LoadingGap:
			BuildGapBmp(m_pSpinBmp, w, h, 270.0f);
			break;
		case LoadingFade:
			BuildFadeBmp(m_pSpinBmp, w, h);
			break;
		case LoadingArc:
			BuildGapBmp(m_pSpinBmp, w, h, 90.0f);
			break;
		case LoadingDots:
		case LoadingWave:
		case LoadingBars:
		case LoadingDrop:
		case LoadingDrip:
		case LoadingStars:
		case LoadingStar:
		case LoadingDog:
		case LoadingFish:
		case LoadingPulse:
		case LoadingChase:
			// 占位，每帧重画
			break;
		default:
			if( m_eType == LoadingSpoke || m_Angles == NULL )
				EnsureSpokeData();
			BuildSpokeBmp(m_pSpinBmp, w, h);
			break;
		}
	}

	if( m_bMorphType && m_pSpinBmp ) {
		switch( m_eType ) {
		case LoadingDots: BuildDotsFrame(m_pSpinBmp, w, h); break;
		case LoadingWave: BuildWaveFrame(m_pSpinBmp, w, h); break;
		case LoadingBars: BuildBarsFrame(m_pSpinBmp, w, h); break;
		case LoadingDrop: BuildDropFrame(m_pSpinBmp, w, h); break;
		case LoadingDrip: BuildDripFrame(m_pSpinBmp, w, h); break;
		case LoadingStars: BuildStarsFrame(m_pSpinBmp, w, h); break;
		case LoadingStar: BuildStarFrame(m_pSpinBmp, w, h); break;
		case LoadingDog: BuildDogFrame(m_pSpinBmp, w, h); break;
		case LoadingFish: BuildFishFrame(m_pSpinBmp, w, h); break;
		case LoadingPulse: BuildPulseFrame(m_pSpinBmp, w, h); break;
		case LoadingChase: BuildChaseFrame(m_pSpinBmp, w, h); break;
		default: break;
		}
		// 内容变了但指针不变：必须清 D2D 缓存，否则一直显示第一帧
		IRenderDevice* pDev = GetRenderDevice();
		if( pDev && pDev->GetBackendKind() == DUILIB_RENDER_D2D )
			static_cast<CD2dRenderDevice*>(pDev)->InvalidateBitmapCacheForImage(NULL, m_pSpinBmp);
	}
}

void CLoadingUI::RestartTimer()
{
	if( m_bStop || !m_pManager || m_nTime <= 0 ) return;
	m_pManager->KillTimer(this, kTimerLoadingId);
	m_pManager->SetTimer(this, kTimerLoadingId, (UINT)m_nTime);
}

void CLoadingUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcsicmp(pstrName, _T("type")) == 0 || _tcsicmp(pstrName, _T("variant")) == 0 ) {
		if( _tcsicmp(pstrValue, _T("css")) == 0 || _tcsicmp(pstrValue, _T("border")) == 0 )
			SetLoadingType(LoadingCss);
		else if( _tcsicmp(pstrValue, _T("gap")) == 0 || _tcsicmp(pstrValue, _T("c")) == 0 )
			SetLoadingType(LoadingGap);
		else if( _tcsicmp(pstrValue, _T("fade")) == 0 || _tcsicmp(pstrValue, _T("ring")) == 0
			|| _tcsicmp(pstrValue, _T("spin")) == 0 )
			SetLoadingType(LoadingFade);
		else if( _tcsicmp(pstrValue, _T("arc")) == 0 || _tcsicmp(pstrValue, _T("circle")) == 0 )
			SetLoadingType(LoadingArc);
		else if( _tcsicmp(pstrValue, _T("dots")) == 0 || _tcsicmp(pstrValue, _T("dot")) == 0 )
			SetLoadingType(LoadingDots);
		else if( _tcsicmp(pstrValue, _T("wave")) == 0 || _tcsicmp(pstrValue, _T("sine")) == 0 )
			SetLoadingType(LoadingWave);
		else if( _tcsicmp(pstrValue, _T("bars")) == 0 || _tcsicmp(pstrValue, _T("eq")) == 0
			|| _tcsicmp(pstrValue, _T("equalizer")) == 0 )
			SetLoadingType(LoadingBars);
		else if( _tcsicmp(pstrValue, _T("drop")) == 0 || _tcsicmp(pstrValue, _T("splash")) == 0
			|| _tcsicmp(pstrValue, _T("water")) == 0 )
			SetLoadingType(LoadingDrop);
		else if( _tcsicmp(pstrValue, _T("drip")) == 0 || _tcsicmp(pstrValue, _T("blob")) == 0
			|| _tcsicmp(pstrValue, _T("drop-flat")) == 0 )
			SetLoadingType(LoadingDrip);
		else if( _tcsicmp(pstrValue, _T("stars")) == 0 || _tcsicmp(pstrValue, _T("twinkle")) == 0 )
			SetLoadingType(LoadingStars);
		else if( _tcsicmp(pstrValue, _T("star")) == 0 || _tcsicmp(pstrValue, _T("sparkle")) == 0
			|| _tcsicmp(pstrValue, _T("shine")) == 0 )
			SetLoadingType(LoadingStar);
		else if( _tcsicmp(pstrValue, _T("dog")) == 0 || _tcsicmp(pstrValue, _T("puppy")) == 0
			|| _tcsicmp(pstrValue, _T("run")) == 0 )
			SetLoadingType(LoadingDog);
		else if( _tcsicmp(pstrValue, _T("fish")) == 0 || _tcsicmp(pstrValue, _T("fish-one")) == 0
			|| _tcsicmp(pstrValue, _T("swim")) == 0 )
			SetLoadingType(LoadingFish);
		else if( _tcsicmp(pstrValue, _T("pulse")) == 0 || _tcsicmp(pstrValue, _T("breath")) == 0 )
			SetLoadingType(LoadingPulse);
		else if( _tcsicmp(pstrValue, _T("chase")) == 0 )
			SetLoadingType(LoadingChase);
		else
			SetLoadingType(LoadingSpoke);
	}
	else if( _tcsicmp(pstrName, _T("style")) == 0 ) {
		if( _tcsicmp(pstrValue, _T("macosx")) == 0 || _tcscmp(pstrValue, _T("1")) == 0 ) {
			m_NumberOfSpoke = 12; m_SpokeThickness = 0;
			SetLoadingType(LoadingSpoke);
		}
		else if( _tcsicmp(pstrValue, _T("firefox")) == 0 || _tcscmp(pstrValue, _T("2")) == 0 ) {
			m_NumberOfSpoke = 9; m_SpokeThickness = 0;
			SetLoadingType(LoadingSpoke);
		}
		else if( _tcsicmp(pstrValue, _T("ie7")) == 0 || _tcscmp(pstrValue, _T("3")) == 0 ) {
			m_NumberOfSpoke = 24; m_SpokeThickness = 0;
			SetLoadingType(LoadingSpoke);
		}
		if( m_pManager ) EnsureSpokeData();
		DestroySpinBmps();
	}
	else if( _tcsicmp(pstrName, _T("time")) == 0 ) {
		m_nTime = _ttoi(pstrValue);
		if( m_nTime < 10 ) m_nTime = 10;
		RestartTimer();
	}
	else if( _tcsicmp(pstrName, _T("duration")) == 0 ) {
		m_nDuration = _ttoi(pstrValue);
		if( m_nDuration < 200 ) m_nDuration = 200;
	}
	else if( _tcsicmp(pstrName, _T("spoke")) == 0 ) {
		m_NumberOfSpoke = _ttoi(pstrValue);
		if( m_NumberOfSpoke < 3 ) m_NumberOfSpoke = 3;
		if( m_pManager ) EnsureSpokeData();
		DestroySpinBmps();
	}
	else if( _tcsicmp(pstrName, _T("thickness")) == 0 ) {
		m_SpokeThickness = _ttoi(pstrValue);
		if( m_SpokeThickness < 0 ) m_SpokeThickness = 0;
		DestroySpinBmps();
	}
	else if( _tcsicmp(pstrName, _T("outer-radius")) == 0 ) {
		m_OuterCircleRadius = _ttoi(pstrValue);
		DestroySpinBmps();
	}
	else if( _tcsicmp(pstrName, _T("inner-radius")) == 0 ) {
		m_InnerCircleRadius = _ttoi(pstrValue);
		DestroySpinBmps();
	}
	else if( _tcsicmp(pstrName, _T("color")) == 0 ) {
		DWORD clr = 0;
		if( ParseColorString(pstrValue, clr) ) {
			// DuiLib DWORD = RRGGBBAA，Gdiplus::Color 构造为 (A,R,G,B)
			m_Color = Color(DuiColorA(clr), DuiColorR(clr), DuiColorG(clr), DuiColorB(clr));
			if( m_eType == LoadingSpoke ) EnsureSpokeData();
			DestroySpinBmps();
		}
	}
	else if( _tcsicmp(pstrName, _T("track-color")) == 0 || _tcsicmp(pstrName, _T("trackcolor")) == 0 ) {
		DWORD clr = 0;
		if( ParseColorString(pstrValue, clr) ) {
			m_TrackColor = Color(DuiColorA(clr), DuiColorR(clr), DuiColorG(clr), DuiColorB(clr));
			DestroySpinBmps();
		}
	}
	else {
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

void CLoadingUI::PaintBackgroundImage(IRenderContext& ctx)
{
	int w = GetWidth();
	int h = GetHeight();
	if( w <= 0 || h <= 0 ) return;

	EnsureSpinBmps(w, h);
	if( m_pSpinBmp == NULL ) return;

	if( m_eType == LoadingCss && m_pTrackBmp )
		ctx.DrawGdiplusImage(m_pTrackBmp, (INT)m_rcItem.left, (INT)m_rcItem.top, w, h);

	if( m_bMorphType ) {
		// 形态变化型：已按角度重画，直接贴图
		ctx.DrawGdiplusImage(m_pSpinBmp, (INT)m_rcItem.left, (INT)m_rcItem.top, w, h);
	}
	else {
		// 圆环 / 辐条：固定图画一次，只旋转缺口位置
		ctx.DrawGdiplusImageRotated(m_pSpinBmp, m_rcItem, m_fAngle);
	}
}

void CLoadingUI::TickFrame()
{
	if( m_nDuration < 200 ) m_nDuration = 200;
	float delta = 360.0f * (float)m_nTime / (float)m_nDuration;
	m_fAngle = WrapDeg(m_fAngle + delta);
	// 与 Ring 一致：刷新父级脏区，保证旋转可见
	NeedParentUpdate();
}

void CLoadingUI::Start()
{
	if( m_nTime > 0 && m_pManager && m_bStop ) {
		m_pManager->SetTimer(this, kTimerLoadingId, (UINT)m_nTime);
	}
	m_bStop = false;
}

void CLoadingUI::Stop()
{
	m_bStop = true;
	if( m_pManager )
		m_pManager->KillTimer(this, kTimerLoadingId);
}

void CLoadingUI::Init()
{
	CControlUI::Init();
	m_bMorphType = IsMorphType(m_eType);
	if( m_eType == LoadingSpoke )
		EnsureSpokeData();
	Start();
}

void CLoadingUI::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_TIMER && event.wParam == kTimerLoadingId ) {
		TickFrame();
		return;
	}
	CControlUI::DoEvent(event);
}

CControlUI* CreateLoadingControl(LPCTSTR pstrType)
{
	if( _tcsicmp(pstrType, DUI_CTR_LOADINGCIRCLE) == 0 )
		return new CLoadingUI();
	return NULL;
}
