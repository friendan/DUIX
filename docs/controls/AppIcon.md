# AppIcon

| | |
|--|--|
| 类 | `CAppIconUI` |
| XML | `<AppIcon>` / `appicon` |
| 源码 | `src/DuiLib/Control/UIAppIcon.*` |
| 继承 | [Button](Button.md) → [Label](Label.md) → [Control](Control.md) |

手机桌面风格应用图标：**图标在上、标题在下**，默认可点（`click`）。在 Button 上固定了桌面布局默认值，并可选内置角标（数字 / 小红点）。

与 [FontIcon](FontIcon.md) 区别：FontIcon 是单字色块；AppIcon 是「应用图标 + 名称」，也支持无图时的文字图标（仅图标区）。
与外包一层 [Badge](Badge.md) 的区别：本控件用 `badge-*` 属性即可，不必再包容器。

### 最小示例

```xml
<AppIcon text="邮件" lucide="mail" />
<AppIcon text="设置" iconpark="setting" icon-size="48" width="80" height="96" />
<AppIcon name="app_msg" text="消息" lucide="message-circle" badge-count="3" />
<AppIcon text="动态" lucide="bell" badge-dot="true" />
<!-- 纯图标：省略 text；未写宽高时自动正方形（icon-size+16） -->
<AppIcon lucide="search" tooltip="搜索" />
<AppIcon lucide="plus" badge-count="2" />
<!-- 文字图标：无 lucide/icon，仅 text → 区内换行裁切；全文自动 tooltip -->
<AppIcon text="邮件" />
<AppIcon text="我的云盘应用" />
<AppIcon text="云盘" background-color="#722ED1FF" />
<AppIcon text="OK" kind="success" />
```

### 默认值（相对 Button）

| 项 | 默认 |
|----|------|
| `icon-position` | `top` |
| `icon-size` | `56` |
| `icon-gap` | `4` |
| `kind` | `none`（常态透明；悬停/按下跟主题 `color-bg-hover-medium` / `color-bg-hover-primary`） |
| `border-radius` | `12`（文字图标的图标区圆角也跟它，未设时约 `icon-size/5`） |
| `width` / `height` | 未写时：有图标+标题约 `72×88`；**纯图标 / 文字图标**收成正方形 `72×72`（随 `icon-size`） |
| 文字 | 可空（纯图标）；有字无图标时走**文字图标**；chrome 下标题字色跟 `color-text` |
| 光标 | 手型 |

### 文字图标（无 `lucide` / `icon` / …）

仅设 `text`、未设图标库/路径时：

1. **只画图标区**：圆角底 + 全文（可换行、居中）；一行放不下则换行，超出区域裁切（不截前 N 字、不加省略号）
2. **无下方标题行**（避免与区内文字重复）
3. **tooltip**：未手写 `tooltip` 时自动用完整 `text`
4. **底色**：未设时跟主题 `color-primary`（悬停/按下跟 primary-hover/active）；`kind` 非 `none` 时跟 kind 色板
5. **区内字色**：默认 `color-primary-text`；可用 `icon-color` 覆盖

| 属性 | 说明 |
|------|------|
| `background-color` / `bkcolor` | **整格**底色（圆角随 `border-radius`）。有图标时铺满格子；文字图标时图标区也跟随该色（含悬停/按下） |
| `icon-background` / `icon-bk` / `text-icon-background` | **仅**文字图标的图标区底色（不铺整格）；优先于 `background-color` |
| `icon-color` / `text-icon-color` | 图标区内文字色 |
| `tooltip` | 手写后不再被自动 tip 覆盖 |

`theme="chrome"` 子树内会套：标题色、悬停/按下底、角标 `color-danger`。可用 `background-color-hover` 等覆盖；`theme="none"` 跳过 chrome。`kind` 非 `none` 时悬停跟 Button 的 kind 色板。

图标用法与 Button 相同：`lucide` / `bsicon` / `iconpark` / `remixicon` / `tabler-*` / `twicon`、`icon` / `icon-src`。

另支持**文件 / EXE 图标**（`file` / `exe`）：

```xml
<!-- PNG/JPG/BMP/GIF/WebP：显示图片内容（皮肤相对路径或绝对路径；盘符路径建议用 / 避免 \n 等歧义） -->
<AppIcon text="相册" file="photo.png" />
<AppIcon text="壁纸" file="C:/Windows/Web/Wallpaper/Windows/img0.jpg" />
<AppIcon text="WebP" file="img/demo.webp" />
<!-- SVG：渲染矢量内容（同 icon-src）；亦可 icon-src="x.svg" -->
<AppIcon text="矢量" file="icons/app.svg" />
<!-- EXE/DLL/ICO：内嵌或外壳图标 -->
<AppIcon text="记事本" file="C:/Windows/System32/notepad.exe" />
<AppIcon text="画图" exe="C:/Windows/System32/mspaint.exe" />
<!-- 其它文件 / 目录：系统外壳关联图标；缺失路径静默回退类型图标 -->
<AppIcon text="hosts" file="C:/Windows/System32/drivers/etc/hosts" />
<AppIcon text="缺文件" file="C:/path/not_exist.exe" />
```

| 属性 | 说明 |
|------|------|
| `file` / `file-icon` / `icon-file` | 文件路径。PNG/JPG/BMP/GIF/**WebP**/SVG 显示内容；`.exe`/`.dll`/`.ico`/其它取外壳或内嵌图标 |
| `exe` / `exe-icon` | 同 `file`（语义偏向可执行文件） |

C++：`SetFileIcon(path)` / `GetFileIcon()`；`SetIconFromMemory(bytes, size)`（网站 favicon 等）。改 `icon-size`（XML 或 `SetIconSize`）会按新尺寸**重取**外壳/EXE 图标。与 `lucide` / `icon-src` / `ClearIcon` / `SetIconFromMemory` **互斥**（后设的覆盖并清空 `file` 路径；C++ 虚函数与 XML 一致）。DPI 变化后若 `RemoveAllImages`，会经 `OnResetDpiAssets` 自动 `ApplyFileIcon` 重建。

```cpp
// 下载 favicon 后：
pApp->SetIconFromMemory(pPngBytes, cbSize);
// 或已有 HBITMAP：
pApp->SetIconBitmap(hBmp, w, h, true);
```

支持内存格式：PNG / JPG / BMP / GIF / WEBP、ICO、SVG。数据无效时返回 `false`，不弹错。未挂 Manager 时先缓存，挂上后自动应用。

**文件已删 / 不可读**：不弹错、不崩溃。PNG/SVG 等先试加载内容，失败则按扩展名显示外壳类型图标；仍失败则清空图标（有 `text` 时退回文字图标）。

### 文件 / EXE 图标实现要点（排障）

加载与转换在 `CAppIconUI::SetFileIcon` / `ApplyFileIcon` → `LoadFileHIcon` → `CButtonUI::CreateBitmapFromHIcon` → `SetIconBitmap`。

| 步骤 | 正确做法 | 易错 |
|------|----------|------|
| 光栅路径 | JPG/PNG/**WebP**/GIF 等走 `SetIconSrc` / `GetImageEx`（`LoadImageFromMemory`：stb 失败则 **WIC**） | `IsRasterImagePath` 未含 `.webp` → 误走外壳图标 |
| EXE/DLL | `PrivateExtractIcons(..., 256, 256, …)` 取资源高清图 | 只用外壳 `SHIL_EXTRALARGE`(48) 再放大到 56 → **发糊** |
| 其它文件 | `SHGetImageList`：目标边长 `>48` 用 `SHIL_JUMBO`(256)，`>32` 用 EXTRALARGE | `>=48` 误选 48 再放大 |
| HICON→位图 | `DrawIconEx` 到固有尺寸 DIB（必要时黑白双缓冲重建 alpha）→ **Gdiplus 高品质缩到 icon-size**；透明边可裁后等比放入 | `Gdiplus::Bitmap::FromHICON`：Win10 上常把透明区写成**不透明黑**（文本类型图标黑底）；`DrawIconEx` 直接缩 256→56 → **发糊** |
| 登记绘制 | `SetIconBitmap` → `AddImage(memKey)`；`RefreshRasterIconImage` 写 `file='_dui_btn_icon_…' dest='0,0,逻辑边长,逻辑边长'` | 写成裸 `memKey dest='…'`：`TDrawInfo::Parse` 把**整串**当图片名 → GetImage 失败 → **空白**（壁纸 `file=` 正常、EXE 空白） |
| dest DPI | dest 填**逻辑** `icon-size`，交给 `TDrawInfo::Parse` **一次** Scale | 预 Scale 后再 Parse Scale → 二次放大发糊 |
| icon-size | 虚 `SetIconSize`：有 `file` 时重取外壳图；**勿**在属性分支里因 icon-size 清空 `m_sFileIcon` | XML `file` 后再写 `icon-size` 路径被清 → 无法重取 |
| 互斥 | `ClearIcon` / `SetIconLib` / `SetIconSrc` / `SetIconFromMemory` 虚函数内清空 `m_sFileIcon` | 仅 XML 清路径、C++ `ClearIcon` 后 `SetIconSize` 又刷回旧文件图 |
| DPI | `ResetDPIAssets` 遍历控件 `OnResetDpiAssets` → AppIcon 再 `ApplyFileIcon` | `RemoveAllImages` 后 mem key 失效、EXE 图标空白 |

失败信号对照：

| 现象 | 优先查 |
|------|--------|
| 只有 JPG/PNG 正常，EXE/外壳空白（像背景色） | mem 图串是否缺 `file='…'`；或 alpha 全 0 / FromHICON 黑底被当透明 |
| EXE 能显示但发糊 | 是否从 48 放大；是否 `DrawIconEx` 硬缩而非 HQ 缩小；dest 是否二次 DPI |
| 文本/类型图标黑底、内容偏小 | 是否仍用 `FromHICON`；裁透明边 + contain 铺格 |

路径约定：盘符建议 `C:/...`（`SetFileIcon` 也会把 `\` 换成 `/`）。C++ 亦可 `SetIconBitmap` / `SetIconFromMemory`（Button 同路径）。

### 专有属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `iconimage` / `icon-image` | 同 `icon-src`（光栅或 SVG 路径） | — |
| `file` / `file-icon` / `icon-file` / `exe` / `exe-icon` | 见上「文件 / EXE 图标」 | — |
| `background-color` / `bkcolor` | 整格底色；文字图标时图标区跟随 | — |
| `icon-bk` / `icon-background` | 仅文字图标区底色 | — |
| `badge` / `badge-count` | 角标数字；`0` 默认隐藏 | `0` |
| `badge-overflow` / `overflow-count` | 超过则显示 `N+` | `99` |
| `badge-show-zero` / `show-zero` | `0` 是否仍显示 | `false` |
| `badge-dot` / `dot` | 小红点（忽略数字文案） | `false` |
| `badge-hang` | 半悬在图标右上；超出本控件时会夹回 | `true` |
| `badge-offset` | `x,y` 逻辑像素偏移（非 0 时按右上绝对偏移） | `0,0` |
| `badge-color` | 角标底色 | `#4D4FFF` |
| `badge-text-color` | 角标文字色 | `#FFFFFF` |

仍可用外包 `<Badge>`；与内置角标不要叠用。

### 通知

与 Button 相同：点击发 `click`（`DUI_MSGTYPE_CLICK`）。

```cpp
CAppIconUI* p = static_cast<CAppIconUI*>(
    m_pm.FindControl(_T("app_msg"))->GetInterface(DUI_CTR_APPICON));
p->SetBadgeCount(12);
```

### API

| 方法 | 说明 |
|------|------|
| `IsTextIcon` | 当前是否为文字图标模式（无图 + 有 text） |
| `SetTextIconBackground` / `GetTextIconBackground` | 文字图标区底色；`0` 清自定义、回主题 |
| `SetTextIconColor` / `GetTextIconColor` | 文字图标区内字色 |
| `RefreshLayout` | C++ 调用 `ClearIcon` / `SetIconLib` / `SetIconSrc` / `SetFileIcon` 后刷新自动尺寸与 tip |
| `SetFileIcon` / `GetFileIcon` | 文件 / EXE / 外壳图标（见上） |
| `SetIconFromMemory` | 内存字节图标（favicon 等；清空 `file` 并 `RefreshLayout`） |
| `ClearIcon` / `SetIconLib` / `SetIconSrc` / `SetIconSize` | 虚函数；清图标源会清空 `file`；改尺寸会重取外壳图 |
| `OnResetDpiAssets` | DPI/`RemoveAllImages` 后重建 `file` 图标 |
| `SetBadgeCount` / `GetBadgeCount` | 角标数字 |
| `SetBadgeOverflow` / `GetBadgeOverflow` | 溢出阈值 |
| `SetBadgeShowZero` / `IsBadgeShowZero` | 零是否显示 |
| `SetBadgeDot` / `IsBadgeDot` | 小红点 |
| `SetBadgeHang` / `IsBadgeHang` | 半悬 |
| `SetBadgeOffset` / `GetBadgeOffset` | 偏移 |
| `SetBadgeColor` / `GetBadgeColor` | 底色 |
| `SetBadgeTextColor` / `GetBadgeTextColor` | 字色 |

继承 Button：`SetIconLib` / `SetIconSrc` / `ClearIcon`、`click`、kind、悬停底等。动态改图标后请 `RefreshLayout()`。

Demo：Accordion → **AppIcon / AppGrid**（样例排在网格前排，搜索「记事本」「邮件」等可定位）。
