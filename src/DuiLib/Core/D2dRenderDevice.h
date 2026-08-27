#ifndef __D2DRENDERDEVICE_H__
#define __D2DRENDERDEVICE_H__

#pragma once

#include "IRenderDevice.h"
#include "GdiRenderContext.h"

struct ID2D1Factory;
struct ID2D1Factory1;
struct IDWriteFactory;
struct IDWriteTextFormat;
struct ID2D1RenderTarget;
struct ID2D1DCRenderTarget;
struct ID2D1BitmapRenderTarget;
struct ID2D1HwndRenderTarget;
struct ID2D1SolidColorBrush;
struct ID2D1Bitmap;
struct ID2D1Geometry;
struct ID2D1GdiInteropRenderTarget;
struct ID2D1Device;
struct ID2D1DeviceContext;
struct ID2D1Bitmap1;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGIDevice;
struct IDXGISwapChain1;
struct IDCompositionDevice;
struct IDCompositionTarget;
struct IDCompositionVisual;

namespace DuiLib {

	struct TD2dBitmapCache
	{
		HBITMAP hBitmap;
		void* pGdiplusKey;
		ID2D1RenderTarget* pRT; // 创建该 bitmap 的 RT（不 AddRef，仅作身份键）
		ID2D1Bitmap* pBitmap;
	};

	struct TD2dClipEntry
	{
		RECT rcBound;   // 内容范围（通常为脏区∩控件）
		RECT rcRound;   // 圆角几何所依据的完整控件矩形（与 GDI GenerateRoundClip 的 rcItem 一致）
		bool bRound;
		float radiusX;
		float radiusY;
		bool bLayer; // true=PushLayer 圆角；false=PushAxisAlignedClip
	};

	class CD2dRenderDevice;

	// Direct2D 上下文：优先使用 Surface 的 Bitmap RT；否则 BindDC。复杂路径可 Flush 后走 GDI。
	class UILIB_API CD2dRenderContext : public IRenderContext
	{
	public:
		CD2dRenderContext(HDC hDC, CPaintManagerUI* pManager, ID2D1Factory* pFactory, IDWriteFactory* pDWrite);
		~CD2dRenderContext();

		void OnBeginFrame();
		void OnEndFrame();

		HDC GetDC() const override;
		CPaintManagerUI* GetManager() const override;

		void PushClip(const RECT& rc) override;
		void PushRoundClip(const RECT& rcClip, const RECT& rcRound, int width, int height) override;
		void PopClip() override;
		void SuspendClip() override;
		void ResumeClip() override;

		int SaveState() override;
		void RestoreState(int nSaved) override;

		void DrawColor(const RECT& rc, DWORD color) override;
		void DrawGradient(const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps) override;
		void DrawLine(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID) override;
		void DrawRect(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID) override;
		void DrawRoundRect(const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle = PS_SOLID) override;
		void FillRoundRect(const RECT& rc, int width, int height, DWORD dwColor) override;

		void DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle) override;
		void DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle, DWORD dwTextBKColor) override;
		void DrawHtmlText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle) override;
		SIZE GetTextSize(LPCTSTR pstrText, int iFont, UINT uStyle) override;

		bool DrawImageInfo(const RECT& rcItem, const RECT& rcPaint, const TDrawInfo* pDrawInfo, HINSTANCE instance = NULL) override;
		bool DrawImageString(const RECT& rcItem, const RECT& rcPaint, LPCTSTR pStrImage, LPCTSTR pStrModify = NULL, HINSTANCE instance = NULL) override;
#if DUILIB_GDI_INTEROP
		void DrawImage(HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false) override;
#endif
		void DrawImage(const TImageInfo* pImageInfo, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false) override;

		void DrawGdiplusImage(void* pGdiplusImage, int x, int y, int cx, int cy) override;
		void DrawGdiplusImageRotated(void* pGdiplusImage, const RECT& rc, float angleDegrees) override;
		void StretchBlit(void* srcNative, int x, int y, int cx, int cy, int xSrc, int ySrc, int cxSrc, int cySrc, int mode = HALFTONE) override;

		ID2D1RenderTarget* GetRenderTarget() const;

	private:
		IRenderSurface* FindBoundSurface() const;
		bool EnsureDCRenderTarget();
		bool EnsureD2dDraw();
		void FlushToGdi(bool bSyncPixels = true) const;
		void ApplyClipEntry(const TD2dClipEntry& entry);
		void PopOneClip();
		void ReleaseHwndInteropDC();
		HDC AcquireHwndInteropDC();
		ID2D1SolidColorBrush* GetBrush(DWORD argb);
		void ClearBitmapCache();
		ID2D1Bitmap* GetOrCreateBitmap(HBITMAP hBitmap, int nX, int nY, LPBYTE pBits, bool bAlpha);
		ID2D1Bitmap* GetOrCreateBitmapFromGdiplus(void* pGdiplusImage);
		// 一次性上传（不进 Device 缓存；源像素常被原地改写，如 ColorPalette）
		ID2D1Bitmap* CreateTempBitmapFromHBitmap(HBITMAP hBitmap, bool bAlpha);
		void DrawBitmapRect(ID2D1Bitmap* pBitmap, const RECT& rcDest, const RECT& rcSrc, float opacity);
		bool DrawImageD2d(ID2D1Bitmap* pBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, UINT uFade, bool hole, bool xtiled, bool ytiled);
		bool MeasureTextDWrite(RECT& rc, LPCTSTR pstrText, IDWriteTextFormat* pFormat, UINT uStyle) const;
		IDWriteTextFormat* CreateTextFormat(int iFont) const;
		static bool TextLooksLikeHtml(LPCTSTR pstrText);
		bool DrawHtmlTextDWrite(RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle);

		CGdiRenderContext m_gdiFallback;
		ID2D1Factory* m_pFactory;
		IDWriteFactory* m_pDWrite;
		ID2D1DCRenderTarget* m_pDCRT;       // 无 Surface Bitmap RT 时的回退
		ID2D1RenderTarget* m_pRT;           // 当前绘制目标（Surface RT 或 DC RT）
		IRenderSurface* m_pBoundSurface;    // 非空表示使用 Surface Bitmap RT
		ID2D1SolidColorBrush* m_pBrush;
		mutable bool m_bInDraw;
		bool m_bPixelsDirty;               // GetDC/GDI 写过，再进 D2D 前需 CopyPixelsToBackend
		ID2D1GdiInteropRenderTarget* m_pHwndInterop; // HWND RT 上借出的 GDI DC
		HDC m_hHwndInteropDC;
		int m_nD2dClipDepth;
		bool m_d2dClipIsLayer[64]; // 实际 Push 到 RT 的类型，与 m_clipStack[].bLayer 解耦
		int m_nBindWidth;
		int m_nBindHeight;
		TD2dClipEntry m_clipStack[64];
		int m_nClipCount;
	};

	// Direct2D 表面：离屏 Bitmap RT + 可选 HWND RT；分层 Present 优先 DXGI/DirectComposition
	class UILIB_API CD2dRenderSurface : public IRenderSurface
	{
	public:
		explicit CD2dRenderSurface(CD2dRenderDevice* pDevice);
		~CD2dRenderSurface();

		bool Ensure(int width, int height, void* refNative) override;
		void Destroy() override;
		bool IsValid() const override;

		int GetWidth() const override;
		int GetHeight() const override;
		BYTE* GetBits() override;
		void* GetNativeTarget() override;
		void* GetBackendTarget() override;

		void SetDirtyRect(const RECT& rc) override;
		void GetDirtyRect(RECT& rc) const override;
		bool CopyBackendToPixels() override;
		bool CopyPixelsToBackend() override;
		// EndDraw 之后用 GetBitmap→DC RT 回写 GDI（不依赖 GdiInterop::GetDC）
		bool CopyBackendToPixelsViaBitmap() override;

		bool PrepareLayeredComposition(HWND hWnd, int width, int height) override;
		bool IsLayeredComposition() const override { return m_bCompDirectDraw; }
		void SetLayeredCompositionEnabled(bool bEnable) override;
		void OnBackendBeginDraw() override;

		bool EnsureWindowTarget(HWND hWnd, int width, int height) override;
		bool IsWindowTarget() const override { return m_pHwndRT != NULL; }
		void DisableWindowTarget() override;
		void ClearPaintRect(const RECT& rcPaint, const RECT& rcClient) override;
		void ClearAll() override;
		void FixLayeredAlpha(const RECT& rcPaint, const RECT& rcClient) override;
		void ApplyRoundCornerMask(int radiusX, int radiusY) override;
		void ApplyLayeredMask(IRenderSurface* pMask, const RECT& rcPaint, const RECT& rcClient) override;

		bool Present(const RenderPresentParams& params) override;

	private:
		bool EnsureBitmapRT();
		void DestroyBitmapRT();
		void DestroyHwndRT();
		void DestroyComposition();
		bool EnsureComposition(HWND hWnd, int width, int height);
		bool BindCompositionTarget();
		void UnbindCompositionTarget();
		bool CopyCompToGdi(const RECT& rcDirty);
		bool CopyGdiToComp(const RECT& rcDirty);
		bool PresentLayeredDComp(const RenderPresentParams& params);
		bool PresentLayeredGdi(const RenderPresentParams& params);
		bool PresentLayered(const RenderPresentParams& params);
		bool ApplyLayeredMaskD2d(IRenderSurface* pMask, const RECT& rcPaint, const RECT& rcClient);
		void ResolveDirtyRect(RECT& rc) const;
		static bool CreateTopDownBitmap(ID2D1RenderTarget* pRT, int width, int height, const BYTE* pBottomUpBits, ID2D1Bitmap** ppBitmap);
		static void ApplyCompositionWindowStyle(HWND hWnd);
		static void ApplyLayeredWindowStyle(HWND hWnd);

		CD2dRenderDevice* m_pDevice;
		CGdiRenderSurface m_gdiFallback;
		ID2D1Factory* m_pFactory;
		ID2D1BitmapRenderTarget* m_pBitmapRT;
		ID2D1HwndRenderTarget* m_pHwndRT;
		HWND m_hWndTarget;
		bool m_bWindowTargetDisabled;

		// DXGI + DirectComposition 分层 Present / 零拷贝直绘
		IDXGISwapChain1* m_pSwapChain;
		ID2D1DeviceContext* m_pCompCtx;
		ID2D1Bitmap1* m_pCompTargetBmp;
		IDCompositionTarget* m_pCompTarget;
		IDCompositionVisual* m_pCompVisual;
		HWND m_hCompWnd;
		int m_nCompWidth;
		int m_nCompHeight;
		bool m_bCompDisabled;
		bool m_bCompDirectDraw;   // GetBackendTarget → CompCtx
		bool m_bCompFrameActive;  // 本帧已 Bind DXGI target，待 Present
		bool m_bPendingClear;     // 首次 BeginDraw 时 Clear 脏区
		bool m_bPostProcessOnGdi; // 遮罩等已改 GDI，Present 前需回传 DXGI
		RECT m_rcDirty;
		bool m_bDirtyValid;
	};

	// Direct2D 设备（含可选 D3D11 / DXGI / DirectComposition）
	class UILIB_API CD2dRenderDevice : public IRenderDevice
	{
	public:
		CD2dRenderDevice();
		~CD2dRenderDevice();

		bool Initialize();
		void Shutdown();
		bool IsReady() const { return m_bReady; }

		// 分层 DComp Present 所需 GPU 对象；失败不影响普通 D2D
		bool EnsureGpu();
		bool IsGpuReady() const { return m_bGpuReady; }

		ID2D1Factory* GetD2dFactory() const { return m_pFactory; }
		ID2D1Factory1* GetD2dFactory1() const { return m_pFactory1; }
		IDWriteFactory* GetDWriteFactory() const { return m_pDWriteFactory; }
		ID2D1Device* GetD2dDevice() const { return m_pD2dDevice; }
		ID3D11Device* GetD3DDevice() const { return m_pD3DDevice; }
		IDXGIDevice* GetDxgiDevice() const { return m_pDxgiDevice; }
		IDCompositionDevice* GetDCompDevice() const { return m_pDCompDevice; }

		int GetBackendKind() const override { return DUILIB_RENDER_D2D; }

		IRenderContext* CreateContext(void* nativeTarget, CPaintManagerUI* pManager) override;
		void DestroyContext(IRenderContext* pCtx) override;
		IRenderSurface* CreateSurface() override;
		void DestroySurface(IRenderSurface* pSurface) override;

		bool CreatePixelBuffer(int width, int height, BYTE** ppBits, void** ppNative) override;
		void DestroyPixelBuffer(void* pNative) override;

		TImageInfo* LoadImage(LPCTSTR pStrImage, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) override;
		TImageInfo* LoadImage(UINT nID, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) override;
		TImageInfo* GdiplusLoadImage(LPCTSTR pStrImage, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) override;
		TImageInfo* GdiplusLoadImage(UINT nID, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) override;
		void FreeImage(TImageInfo* pImageInfo, bool bDelete = true) override;
		void InvalidateImageGpu(TImageInfo* pImageInfo) override;
		void InvalidateBitmapGpu(HBITMAP hBitmap) override;

		bool CreateNativeFont(TFontInfo* pFontInfo, int nHeightPx, void* measureNative = NULL) override;
		void DestroyNativeFont(TFontInfo* pFontInfo) override;

		void BeginFrame(IRenderContext* pCtx) override;
		void EndFrame(IRenderContext* pCtx) override;

		// 跨帧复用的纹理缓存（Context 每帧销毁，缓存必须挂在 Device 上）
		ID2D1Bitmap* GetOrCreateBitmap(ID2D1RenderTarget* pRT, HBITMAP hBitmap, int nX, int nY, LPBYTE pBits, bool bAlpha);
		ID2D1Bitmap* GetOrCreateBitmapFromGdiplus(ID2D1RenderTarget* pRT, void* pGdiplusImage);
		void InvalidateBitmapCacheForImage(HBITMAP hBitmap, void* pGdiplusKey = NULL);
		void InvalidateBitmapCacheForRT(ID2D1RenderTarget* pRT);
		void ClearBitmapCache();

	private:
		void ShutdownGpu();

		ID2D1Factory* m_pFactory;
		ID2D1Factory1* m_pFactory1;
		IDWriteFactory* m_pDWriteFactory;
		ID3D11Device* m_pD3DDevice;
		ID3D11DeviceContext* m_pD3DContext;
		IDXGIDevice* m_pDxgiDevice;
		ID2D1Device* m_pD2dDevice;
		IDCompositionDevice* m_pDCompDevice;
		bool m_bReady;
		bool m_bGpuReady;
		bool m_bGpuFailed;
		CStdPtrArray m_aBitmapCache;
	};

	UILIB_API CD2dRenderDevice* GetD2dRenderDevice();
	UILIB_API bool EnableD2dRenderDevice();
	UILIB_API void EnableGdiRenderDevice();

	/// 任务栏只用窗口图标，禁用 Live Preview（悬停不再抓客户区，避免整栏闪白）。
	UILIB_API void DisableTaskbarLivePreview(HWND hWnd);
	UILIB_API void HandleTaskbarIconicThumbnail(HWND hWnd, int cx, int cy);
	UILIB_API void HandleTaskbarIconicLivePreview(HWND hWnd);
	UILIB_API void ClearTaskbarLivePreview(HWND hWnd);

} // namespace DuiLib

#endif // __D2DRENDERDEVICE_H__
