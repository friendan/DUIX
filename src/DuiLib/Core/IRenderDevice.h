#ifndef __IRENDERDEVICE_H__
#define __IRENDERDEVICE_H__

#pragma once

#include "IRenderContext.h"
#include "IRenderSurface.h"
#include "IRenderResource.h"

namespace DuiLib {

	struct tagTImageInfo;
	typedef struct tagTImageInfo TImageInfo;
	struct tagTFontInfo;
	typedef struct tagTFontInfo TFontInfo;

	// 渲染设备抽象：上下文、离屏表面、呈现、资源加载与测量。
	class UILIB_API IRenderDevice
	{
	public:
		virtual ~IRenderDevice() {}

		virtual int GetBackendKind() const { return DUILIB_RENDER_GDI; }

		// nativeTarget：GDI 下为 HDC；D2D/Skia 下可为对应 surface/target
		virtual IRenderContext* CreateContext(void* nativeTarget, CPaintManagerUI* pManager) = 0;
		virtual void DestroyContext(IRenderContext* pCtx) = 0;

		virtual IRenderSurface* CreateSurface() = 0;
		virtual void DestroySurface(IRenderSurface* pSurface) = 0;

		// 像素缓冲（拖拽图/临时离屏）；*ppNative 为后端原生句柄（GDI: HBITMAP）
		virtual bool CreatePixelBuffer(int width, int height, BYTE** ppBits, void** ppNative) = 0;
		virtual void DestroyPixelBuffer(void* pNative) = 0;

		// 图片资源（后端可填 TImageInfo::pBackend）
		virtual TImageInfo* LoadImage(LPCTSTR pStrImage, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) = 0;
		virtual TImageInfo* LoadImage(UINT nID, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) = 0;
		virtual TImageInfo* GdiplusLoadImage(LPCTSTR pStrImage, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) = 0;
		virtual TImageInfo* GdiplusLoadImage(UINT nID, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL) = 0;
		virtual void FreeImage(TImageInfo* pImageInfo, bool bDelete = true) = 0;
		// 像素被原地修改（如 HSL）后，通知后端丢弃 GPU 纹理缓存
		virtual void InvalidateImageGpu(TImageInfo* /*pImageInfo*/) {}
		// 直接按 HBITMAP 丢弃 GPU 缓存（SvgBox 等自管位图，不经 TImageInfo）
		virtual void InvalidateBitmapGpu(HBITMAP /*hBitmap*/) {}

		// 字体资源（写入 hFont 和/或 pBackend）
		virtual bool CreateNativeFont(TFontInfo* pFontInfo, int nHeightPx, void* measureNative = NULL) = 0;
		virtual void DestroyNativeFont(TFontInfo* pFontInfo) = 0;

		// 无布局 HDC 的文字测量（D2D 可用 DWrite；默认实现可走临时 Context）
		virtual SIZE MeasureText(CPaintManagerUI* pManager, LPCTSTR pstrText, int iFont, UINT uStyle);

		// 帧生命周期
		virtual void BeginFrame(IRenderContext* /*pCtx*/) {}
		virtual void EndFrame(IRenderContext* /*pCtx*/) {}
	};

	class UILIB_API CGdiRenderSurface : public IRenderSurface
	{
	public:
		CGdiRenderSurface();
		~CGdiRenderSurface();

		bool Ensure(int width, int height, void* refNative) override;
		void Destroy() override;
		bool IsValid() const override;

		int GetWidth() const override;
		int GetHeight() const override;
		BYTE* GetBits() override;
		void* GetNativeTarget() override;

		void ClearPaintRect(const RECT& rcPaint, const RECT& rcClient) override;
		void ClearAll() override;
		void FixLayeredAlpha(const RECT& rcPaint, const RECT& rcClient) override;
		void ApplyRoundCornerMask(int radiusX, int radiusY) override;
		void ApplyLayeredMask(IRenderSurface* pMask, const RECT& rcPaint, const RECT& rcClient) override;
		bool Present(const RenderPresentParams& params) override;

		HDC GetDC() const { return m_hDC; }
		HBITMAP GetBitmap() const { return m_hBitmap; }

	private:
		HDC m_hDC;
		HBITMAP m_hBitmap;
		HBITMAP m_hOldBitmap;
		BYTE* m_pBits;
		int m_nWidth;
		int m_nHeight;
		bool m_bSelected;
	};

	class UILIB_API CGdiRenderDevice : public IRenderDevice
	{
	public:
		int GetBackendKind() const override { return DUILIB_RENDER_GDI; }

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

		bool CreateNativeFont(TFontInfo* pFontInfo, int nHeightPx, void* measureNative = NULL) override;
		void DestroyNativeFont(TFontInfo* pFontInfo) override;
	};

	// 临时测量 Context：不 BeginFrame、不写入 PaintManager::SetRenderContext
	class UILIB_API CTempRenderContextScope
	{
	public:
		explicit CTempRenderContextScope(CPaintManagerUI* pManager);
		~CTempRenderContextScope();
		IRenderContext& GetContext() const { return *m_pCtx; }
		IRenderContext* operator->() const { return m_pCtx; }

	private:
		IRenderContext* m_pCtx;
		CTempRenderContextScope(const CTempRenderContextScope&);
		CTempRenderContextScope& operator=(const CTempRenderContextScope&);
	};

	// 全局默认设备（进程内单例，可替换）
	UILIB_API IRenderDevice* GetRenderDevice();
	UILIB_API void SetRenderDevice(IRenderDevice* pDevice);

	// RAII：按当前 IRenderDevice 创建并绑定上下文（切换 GDI/D2D 时主绘制路径走这里）
	class UILIB_API COwnedRenderContextScope
	{
	public:
		COwnedRenderContextScope(CPaintManagerUI* pManager, void* nativeTarget);
		~COwnedRenderContextScope();

		IRenderContext& GetContext() const { return *m_pCtx; }
		IRenderContext* operator->() const { return m_pCtx; }
		operator IRenderContext&() const { return *m_pCtx; }

	private:
		CPaintManagerUI* m_pManager;
		IRenderContext* m_pCtx;
		IRenderContext* m_pOld;
		COwnedRenderContextScope(const COwnedRenderContextScope&);
		COwnedRenderContextScope& operator=(const COwnedRenderContextScope&);
	};

} // namespace DuiLib

#endif // __IRENDERDEVICE_H__
