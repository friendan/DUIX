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
- 窗口基类 / 消息映射 / override：[WinImplBase.md](docs/controls/WinImplBase.md)（独立工程可复制摘要进自有 AGENTS）
- HWND 自定义消息：[Messages.md](docs/controls/Messages.md)
- 各控件：同目录下对应 `*.md`（如 [TabBar](docs/controls/TabBar.md)）
- 颜色主题：[Theme.md](docs/controls/Theme.md)（`CTheme` / 内置 azure 等）
- 自定义控件：[CustomControl.md](docs/controls/CustomControl.md)

**本文件只保留构建、环境与渲染硬约束**，勿再往此处堆属性清单。

## 换行符（硬约束）

- **一律使用 Windows CRLF（`\r\n`）**，禁止把仓库文件改成 LF（Linux/Unix）。
- Agent / 编辑器新建或改写任何文本文件时，必须保持或写出 CRLF；不得因「规范化」批量改换行。
- 根目录 `.editorconfig` 已约定 `end_of_line = crlf`；改文件后若 diff 只剩 `^M`/换行差异，应还原为 CRLF，勿提交纯换行变更。

## 调试日志（硬约束）

- 临时排查用日志**只写文件**（建议 `bin/<topic>_debug.log`，与 exe 同目录），方便用户直接打开/粘贴。
- **禁止**用 `OutputDebugString` / DebugView 作为排查日志通道（不便查看与收集）。
- 日志需带时间戳与关键上下文；高频路径（Tick/Paint）应抽样，避免刷爆文件。
- 问题定位完成后应及时拆除或关断临时日志，勿长期留在主干。

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
- **字符集：仅 Unicode**。CMake 已强制 `-DUNICODE -D_UNICODE`；[`src/DuiLib/StdAfx.h`](src/DuiLib/StdAfx.h) 对非 Unicode 直接 `#error`。禁止再维护 ANSI / MultiByte 皮肤或字符串副本（内嵌皮肤用 `LR"..."`）
- 输出目录：`bin/`（exe、dll、lib 统一输出；后缀 `_mtd` / `_mt`）

## C++ 代码规范（硬约束）

clang-cl 使用 **`/W3`**（[`src/CMakeLists.txt`](src/CMakeLists.txt)），会对未标 `override` 的覆写报 **`-Winconsistent-missing-override`**。**凡覆写基类虚函数，声明末尾必须写 `override`**，否则每个包含该头文件的 `.cpp` 都会重复报 warning，整库编译日志会被刷爆。

### 必须带 `override` 的常见虚函数

| 类别 | 典型函数 |
|------|----------|
| 控件身份 | `GetClass`、`GetInterface` |
| 属性 / 布局 | `SetAttribute`、`SetPos`、`EstimateSize` |
| 生命周期 / 事件 | `DoInit`、`DoEvent`、`SetVisible`、`SetEnabled` |
clang 绘制 | `Paint`、`PaintText`、`PaintBackgroundImage` 等 `Paint*` |
| 窗口基类 | 见下「消息映射 / WindowImplBase」 |

### 消息映射 / WindowImplBase（硬约束）

独立工程窗体若用 clang-cl `/W3`，下列写法会触发 **`-Winconsistent-missing-override`**（每个包含该头的 `.cpp` 刷爆日志）。

| 场景 | 正确写法 |
|------|----------|
| 根类 `CNotifyPump` | `DUI_DECLARE_MESSAGE_MAP_BASE()`（首次引入 `virtual GetMessageMap`） |
| 派生窗 / `WindowImplBase` 子类 | `DUI_DECLARE_MESSAGE_MAP()`（宏内已是 `GetMessageMap() const override`） |
| 覆写 `CWindowWnd` / `INotifyUI` / `IMessageFilterUI` 等 | 声明末尾写 `override`：如 `OnFinalMessage`、`Notify`、`HandleMessage`、`MessageHandler`、`GetWindowClassName`、`GetClassStyle`、`CreateControl`、`QueryControlText` |
| 仅在 `WindowImplBase` **首次引入**的虚函数 | 继续 `virtual`，**不要**误标 `override`：如 `InitWindow`、`OnClick`、`GetSkinFile`、`InitResource` |

用法与可复制到业务工程 `AGENTS.md` 的摘要见 **[docs/controls/WinImplBase.md](docs/controls/WinImplBase.md)**。

### 新增 / 修改控件时

1. **新控件头文件**：参照 [`UILoading.h`](src/DuiLib/Control/UILoading.h)、[`UIEdit.h`](src/DuiLib/Control/UIEdit.h)——凡 override 基类虚函数的一律加 `override`；不要只给部分函数加（如 `UIRing.h` 曾混用，导致 130 个 TU × 3 条 warning）。
2. **改已有 `.h/.cpp`**：动到类声明时，**顺带**给该类仍缺 `override` 的覆写补上；勿为「清 warning」无功能需求地整库批量改声明。
3. **提交前**：本地编译后看 `build.log`（或终端输出），**不得新增 warning**；若动到的文件仍报 `-Winconsistent-missing-override`，先修再交。
4. **禁止**为压 warning 去掉基类 `virtual` 或改用 `#pragma` 关警告；应补 `override` 或改签名与基类一致。
5. **禁止**用脚本/正则「全库批量」把 `virtual` 换成 `override` 或给所有声明补 `override`。基类里**非 virtual 的默认实现**（如 `IRenderSurface::GetBackendTarget`）、**不同签名的重载**、**带函数体的 inline 声明**、接口/引擎里**首次出现的 virtual**，误标 `override` 会直接编译失败。历史 warning 只修**编译输出指向的那一个头文件/声明**。

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
6. **非分层 + 原生 `WC_EDIT` 插入符**  
   Present `BitBlt` 会盖住子窗 XOR 光标（能输入、`hwndCaret` 有值但看不见）。现行：Present 前 `ExcludeClipRect` Edit 子窗；Present 后焦点在 Edit 且**未组字**时再 `RedrawWindow`。细节与禁改项见 [docs/controls/Edit.md](docs/controls/Edit.md#聚焦原生-wc_edit插入符与中文输入法排障)。
7. **任务栏悬停整栏图标闪白**  
   GDI 兼容 RT 必须 `TYPE_SOFTWARE`（`GdiCompatRtProps`）；`WindowImplBase` 创建后 `DisableTaskbarLivePreview`（图标化 + 缓存响应 `WM_DWMSENDICONIC*`）。勿把 GDI 兼容 RT 改回 `TYPE_DEFAULT`、勿每帧 `FREEZE`、勿 `ExtendFrame(-1)`。阴影勿在 `NCACTIVATE`/`ACTIVATEAPP` 里 `SetWindowPos`。排障全文见 [docs/controls/Window.md](docs/controls/Window.md#任务栏悬停整栏图标闪白排障)。

### 改动后冒烟清单

改 Present / Flush / GetDC / 裁剪 / 分层 / 图缓存 / 阴影 / DWM 任务栏属性后，至少手工过一遍（Debug：`bin\*_mtd.exe`）：

| # | 程序 | 检查项 |
|---|------|--------|
| 1 | `duidemo_mtd.exe` | 启动非黑屏；主界面皮肤/按钮圆角/文字正常；切换 Tab/列表滚动无明显花屏 |
| 2 | `duidemo_mtd.exe` | 含 Html 文本、图片按钮的页面；悬停/按下态图正常 |
| 3 | `HiDPITest_mtd.exe` | 启动有内容（非大块空白）；改 DPI/缩放后控件与文字不错位、不整区空白 |
| 4 | （若有）ColorPalette 相关页 | 色板/滑条绘制正常（走 D2D `StretchBlit`，勿再整帧 Flush） |
| 5 | `duidemo_mtd.exe` 表单 Edit / EditBox | 点击后系统闪烁光标可见；EditBox 中文候选在输入框附近（勿无条件 Present 后 Redraw 冲掉 IME） |
| 6 | `duidemo_mtd.exe` 任务栏 | 启动后快速在本应用任务栏按钮上划动：整栏图标不闪白；悬停预览为图标（非实时抓窗）属预期 |

**失败信号**：整窗黑色、客户区大块空白、分层变不透明黑块、悬停图不刷新、HiDPI 裁剪后空白 → 先怀疑 EndFrame sync / Present / GetDC Interop / RoundClip，而不是单个控件逻辑。  
Edit **能输入但无光标** → 先查 Present 子窗 ExcludeClip / 未组字 RedrawWindow（见 Edit.md），勿先上自绘 caret。  
任务栏**整栏图标闪白** → 先查 SOFTWARE RT + `DisableTaskbarLivePreview` + 阴影勿 thrash Z 序（见 Window.md），勿先关阴影 / 切整窗 GDI。
建议顺序：先 1（最快暴露黑屏），再 6（任务栏），再 3，最后 2/4。
