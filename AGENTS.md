# DuiLib Ultimate 构建指南

## 环境依赖

- Windows 10+
- Visual Studio 2022（Community 或 Professional）
- Clang-cl（随 VS 安装的 LLVM 工具链）
- CMake 3.10+
- Ninja
- （可选）WebView2 SDK：默认 `src/3rd/webview2/pkg`；或设 `WEBVIEW2_SDK_PATH`。运行时需 Edge WebView2 Runtime。CMake：`DUILIB_USE_WEBVIEW2`（默认 ON，静态链接 `WebView2LoaderStatic`）

## 目录结构

```
src/           CMake 源码目录（顶层 CMakeLists.txt）
  DuiLib/      DuiLib 库源码
  Demos/       示例程序（duidemo, HiDPITest 等）
bin/           编译输出 + 运行时资源（skin 目录）
docs/controls/ 控件用法知识库（按控件一篇，勿堆本文件）
```

## 控件文档

控件 / 窗口**属性**、通知与行为说明一律放在 **[docs/controls/](docs/controls/README.md)**：

- 属性总览与盒模型约定：[Attributes.md](docs/controls/Attributes.md)
- 窗口级属性：[Window.md](docs/controls/Window.md)
- 各控件：同目录下对应 `*.md`（如 [TabBar](docs/controls/TabBar.md)）
- 颜色主题：[Theme.md](docs/controls/Theme.md)（`CTheme` / 内置 azure 等）

**本文件只保留构建、环境与渲染硬约束**，勿再往此处堆属性清单。

## 首次生成工程

默认使用 **Debug**：

```bat
build_clang_ninja_debug_init.bat
```

Release：

```bat
build_clang_ninja_release_init.bat
```

执行后会在项目根目录生成 `build_clang_ninja_debug` 或 `build_clang_ninja_release` 目录。

## 编译

默认增量编译 Debug：

```bat
build_clang_ninja_debug.bat
```

Release：

```bat
build_clang_ninja_release.bat
```

产物输出到 `bin/` 目录：Debug 静态 CRT `/MTd`、后缀 `_mtd`；Release 静态 CRT `/MT`、后缀 `_mt`。

## init_env.bat

自动查找 vcvarsall.bat 并初始化 x64 编译环境。默认搜索路径：

1. `C:\Program Files\Microsoft Visual Studio\18\Community`
2. `C:\Program Files\Microsoft Visual Studio\2022\Professional`

如果 VS 安装路径不同，修改 `init_env.bat` 中的路径。

## 编译选项

- 编译器：clang-cl（MSVC 兼容模式）
- 构建系统：Ninja
- C++ 标准：C++11
- CRT：Debug `/MTd`，Release `/MT`
- 预定义宏：`CMAKE`, `UNICODE`, `_UNICODE`
- 输出目录：`bin/`（exe、dll、lib 统一输出；后缀 `_mtd` / `_mt`）

## 渲染后端

- 默认：Direct2D（`DUILIB_USE_D2D=ON` → `DUILIB_RENDER_BACKEND=1`）
- 切回 GDI：CMake 配置时加 `-DDUILIB_USE_D2D=OFF` 后重新 init
- 运行时仍可用 `EnableD2dRenderDevice()` / `EnableGdiRenderDevice()` 切换
- 布局测量走 `RenderMeasureText` / `RenderMeasureHtmlText`（经当前 Device），勿再写死 `CGdiRenderContext`
- 非分层 + D2D：优先 HWND RenderTarget；GdiInterop 失败会 `DisableWindowTarget` 回退离屏
- 分层：脏区用 `ClearPaintRect`（GDI bits + D2D Bitmap RT 同步清）；子窗口经 `GetDC()` 混绘后再 Present
- 分层 Present：优先 DXGI SwapChain + DirectComposition 零拷贝直绘（脏区同步）；失败回退 `UpdateLayeredWindow`
- 分层：脏区用 `ClearPaintRect` / `SetDirtyRect`；`CopyBackendToPixels` / `CopyPixelsToBackend` 只同步脏区
- 分层遮罩：D2D `FillOpacityMask`，失败回退 CPU 像素合成
- Html（DWrite）：`b/i/u/c/a/n/r/s/f/p/x/y`、字体名、`<i name num idx>` 图集；复杂标签仍可 Flush→GDI
- 皮肤图 GPU 缓存：挂在 `CD2dRenderDevice`（键 `(hBitmap|Gdiplus, RT)`）；Context 每帧销毁，勿再把缓存放回 Context。`FreeImage` / HSL `AdjustImage` 经 `InvalidateImageGpu` 失效

### D2D 硬约束（易改出黑屏 / 花屏）

改 `OnEndFrame` / `FlushToGdi` / `Present` / `GetDC` / `CopyBackend*` 前必读：

1. **非分层必须 EndFrame 同步到 GDI，再 BitBlt Present**  
   离屏内容在 BitmapRT（预乘 alpha）。若跳过 `CopyBackendToPixels*`，再用 `DrawBitmap` 直绘窗口 DC，常见整窗黑屏。  
   正确路径：`EndFrame` → sync → `m_gdiFallback.Present`（BitBlt）。
2. **分层 Comp 可跳过 sync**  
   仅当 `IsLayeredComposition()` 且本帧未 `GetDC`/GDI 脏（`!m_bPixelsDirty`）时可只 `EndDraw`、Present 走 DComp。有子窗混绘 / 遮罩改过 GDI 时必须回写。
3. **HWND RT**  
   EndFrame 只需 `EndDraw`（内容已在窗口）；不要为「省同步」去改非分层 BitmapRT 路径去套 HWND 逻辑。
4. **`GetDC` / Flush**  
   RichEdit、子窗 `WM_PRINT`、未知 Html 等会 Flush 或借 GdiInterop DC。改裁剪栈 / `PushLayer` / Interop 顺序极易空白或花屏；优先定点修，勿动总路径「少 Flush」。
5. **经典 `ID2D1Bitmap` 绑定创建它的 RT**  
   不能把一张 bitmap 挂到 `TImageInfo::pBackend` 给所有 RT 共用；Device 级按 RT 键控缓存是正确做法。

### 改动后冒烟清单

改 Present / Flush / GetDC / 裁剪 / 分层 / 图缓存后，至少手工过一遍（Debug：`bin\*_mtd.exe`）：

| # | 程序 | 检查项 |
|---|------|--------|
| 1 | `duidemo_mtd.exe` | 启动非黑屏；主界面皮肤/按钮圆角/文字正常；切换 Tab/列表滚动无明显花屏 |
| 2 | `duidemo_mtd.exe` | 含 Html 文本、图片按钮的页面；悬停/按下态图正常 |
| 3 | `HiDPITest_mtd.exe` | 启动有内容（非大块空白）；改 DPI/缩放后控件与文字不错位、不整区空白 |
| 4 | （若有）ColorPalette 相关页 | 色板/滑条绘制正常（走 D2D `StretchBlit`，勿再整帧 Flush） |

**失败信号**：整窗黑色、客户区大块空白、分层变不透明黑块、悬停图不刷新、HiDPI 裁剪后空白 → 先怀疑 EndFrame sync / Present / GetDC Interop / RoundClip，而不是单个控件逻辑。

建议顺序：先 1（最快暴露黑屏），再 3，最后 2/4。
