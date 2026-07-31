#ifndef __IRENDERSURFACE_H__
#define __IRENDERSURFACE_H__

#pragma once

namespace DuiLib {

	class CPaintManagerUI;
	class IRenderContext;

	// Present 参数：将窗口像素缓冲提交到屏幕
	struct UILIB_API RenderPresentParams
	{
		HWND hWnd;
		HDC hWindowDC;       // 窗口 DC（GDI BitBlt / UpdateLayeredWindow 目标）
		RECT rcPaint;        // 脏区（非分层 BitBlt 用）
		RECT rcClient;       // 客户区尺寸
		bool bLayered;
		BYTE nOpacity;
	};

	// 离屏绘制表面：GDI 为 ARGB32 HBITMAP；D2D/Skia 可为对应 render target / bitmap
	class UILIB_API IRenderSurface
	{
	public:
		virtual ~IRenderSurface() {}

		virtual bool Ensure(int width, int height, void* refNative) = 0;
		virtual void Destroy() = 0;
		virtual bool IsValid() const = 0;

		virtual int GetWidth() const = 0;
		virtual int GetHeight() const = 0;
		virtual BYTE* GetBits() = 0;
		// GDI：HDC；其他后端：native target 指针
		virtual void* GetNativeTarget() = 0;

		// D2D/Skia：后端 RT（如 ID2D1BitmapRenderTarget*）；GDI 返回 NULL
		virtual void* GetBackendTarget() { return NULL; }
		// 本帧脏区（CopyBackend/Pixels、Clear 使用）；空矩形表示整表面
		virtual void SetDirtyRect(const RECT& rc) { (void)rc; }
		virtual void GetDirtyRect(RECT& rc) const { ::SetRect(&rc, 0, 0, GetWidth(), GetHeight()); }
		// 帧末 / GetDC 前：后端像素 → GetBits；GetDC 混绘后：GetBits → 后端（按脏区）
		virtual bool CopyBackendToPixels() { return true; }
		virtual bool CopyPixelsToBackend() { return true; }
		// EndDraw 后回写（D2D：GetBitmap→GDI；其它后端默认等同 CopyBackendToPixels）
		virtual bool CopyBackendToPixelsViaBitmap() { return CopyBackendToPixels(); }

		// 分层 DComp：在绘制前绑定 DXGI/Composition，使 GetBackendTarget 直出 SwapChain
		virtual bool PrepareLayeredComposition(HWND /*hWnd*/, int /*width*/, int /*height*/) { return false; }
		virtual bool IsLayeredComposition() const { return false; }
		// 后端 BeginDraw 后回调（清脏区等）
		virtual void OnBackendBeginDraw() {}

		// 非分层直出：绑定 HWND RenderTarget（D2D）；失败则继续离屏 Present
		virtual bool EnsureWindowTarget(HWND /*hWnd*/, int /*width*/, int /*height*/) { return false; }
		virtual bool IsWindowTarget() const { return false; }
		virtual void DisableWindowTarget() {}

		// 分层脏区清零（GDI bits；D2D 同步清 Bitmap RT，避免只清 GDI 导致 Flush 回写脏数据）
		virtual void ClearPaintRect(const RECT& rcPaint, const RECT& rcClient) {}
		// 整表面清零（分层布局变更等）
		virtual void ClearAll() {}
		// 分层窗口：RGB 非零但 A=0 时补 A=255（原 UIManager 像素循环）
		virtual void FixLayeredAlpha(const RECT& rcPaint, const RECT& rcClient) {}
		// 分层遮罩：用 pMask 的 alpha 调制本表面脏区像素（原 UIManager 双重循环）
		virtual void ApplyLayeredMask(IRenderSurface* pMask, const RECT& rcPaint, const RECT& rcClient) {}

		virtual bool Present(const RenderPresentParams& params) = 0;
	};

} // namespace DuiLib

#endif // __IRENDERSURFACE_H__
