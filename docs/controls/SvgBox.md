# SvgBox

| | |
|--|--|
| 类 | `CSvgBoxUI` |
| XML | `<SvgBox>` |
| 源码 | `src/DuiLib/Control/UISvgBox.*` |
| 继承属性 | 见 [Control.md](Control.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `src` | — |
| `color` | — |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `color-hover` | — | 无标准等价 |
| `color-active` | — | 无标准等价 |
| `color-disabled` | — | 无标准等价 |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `data` | — | 无（内联 SVG） |
| `bsicon` | — | 无（图标库） |
| `iconpark` | — | 无 |
| `lucide` | — | 无 |
| `tabler-filled` | — | 无 |
| `tabler-outline` | — | 无 |
| `remixicon` | — | 无 |
| `twicon` | — | 无 |

`src`/`color` 接近 SVG/CSS；图标库名属性与 `data` 内联均无 HTML 标准。伪类可用 `color-hover` 等或 CSS `:hover { color }` 改写。

### 枚举图标库（给使用者展示/选择）

每个内置图标库都提供 3 个静态接口，方便你列出全部图标名（如做成图标选择器）、预览、并按名字回填到 `SvgBox`。

| 图标库（XML 属性） | C++ 类 | 个数 |
|------|------|------|
| `bsicon` | `DuiLib::BootstrapIcons` | 2079 |
| `iconpark` | `DuiLib::IconParkIcons` | 2658 |
| `lucide` | `DuiLib::LucideIcons` | 1748 |
| `remixicon` | `DuiLib::RemixIconIcons` | 3229 |
| `tabler-outline` | `DuiLib::TablerOutlineIcons` | 5112 |
| `tabler-filled` | `DuiLib::TablerFilledIcons` | 1054 |
| `twicon` | `DuiLib::TwemojiIcons` | 3689 |

四个接口：`GetIconCount()` / `GetNameByIndex(i)` / `GetDataByIndex(i)` / `GetIndexByName(name)`。

```cpp
#include <UIlib.h>
// 枚举 Tabler Outline 全部图标（name → SVG），供选择器展示/预览
int n = DuiLib::TablerOutlineIcons::GetIconCount();
for( int i = 0; i < n; ++i ) {
    const wchar_t* name = DuiLib::TablerOutlineIcons::GetNameByIndex(i);
    const char* svg    = DuiLib::TablerOutlineIcons::GetDataByIndex(i);
    // name/svg 均属于库数组，勿 free；把 name 交给 SvgBox 显示：
    svgbox->SetAttribute(_T("tabler-outline"), name);   // 或 LoadFromUtf8Data(svg) 直接给 SVG 字节预览
}
```

- `GetNameByIndex(i)` 返回 `wchar*`（图标名，如 `_T("settings")`）；`GetDataByIndex(i)` 返回 `char*` UTF-8 SVG。两者都属库者数组、勿释放；越界返回 `NULL`。
- `GetIcon(name)` 按名取 SVG，`GetIndexByName(name)` 按名取下标（与 `GetNameByIndex` 互逆），找不到返回 `NULL` / `-1`。**存储建议只存图标名**，读取时用 `GetIcon(name)` / `SetAttribute(库名, name)` 即可实时取回。
- **回填到 `SvgBox` 的两条路**：① `SetAttribute(_T("tabler-outline"), name)` 指向图标库（和你 XML 写 `<SvgBox tabler-outline="settings">` 等效）；② `LoadFromUtf8Data(svg)` 直接塞 SVG 字节预览（适合选择器网格展示，不依赖名字映射）。
- 展示网格建议用 `LoadFromUtf8Data(svg)` 预览（可控尺寸/着色/缓存），选中后再把 `name` 存回业务配置，后续用 `SetAttribute`/XML 方式加载。

#### 统一入口 `CIconLibrary`（不用记 7 个库类名）

`#include <UIlib.h>` 下可直接用 `DuiLib::CIconLibrary`，按库名（与 XML 属性同名）访问，**不用记 7 个库类名**。库名：`bsicon`/`iconpark`/`lucide`/`remixicon`/`tabler-outline`/`tabler-filled`/`twicon`；未知库返回 `0`/`NULL`。

```cpp
#include <UIlib.h>
// 按库名枚举全部图标（name → SVG），入口统一，无需关心底层的 BootstrapIcons 等类
int n = DuiLib::CIconLibrary::GetIconCount(_T("tabler-outline"));
for( int i = 0; i < n; ++i ) {
    const wchar_t* name = DuiLib::CIconLibrary::GetNameByIndex(_T("tabler-outline"), i);
    const char*    svg  = DuiLib::CIconLibrary::GetDataByIndex(_T("tabler-outline"), i);
    // name/svg 均属库的静态数组，勿 free
    svgbox->SetAttribute(_T("tabler-outline"), name);      // 回填到 SvgBox（等价 XML 属性）
    // 或 svgbox->LoadFromUtf8Data(svg);                     // 直接给 SVG 字节预览
}

// 存储时只存图标名，读取时按名取回（不再依赖下标）
const char* svg = DuiLib::CIconLibrary::GetDataByName(_T("tabler-outline"), _T("settings"));  // 找不到返回 NULL
int idx = DuiLib::CIconLibrary::GetIndexByName(_T("tabler-outline"), _T("settings"));         // 找不到返回 -1
```

- `GetIconCount(库名)` 返回该库图标总数；`GetNameByIndex(库名, i)` 返回 `wchar_t*` 图标名；`GetDataByIndex(库名, i)` 返回 `char*` UTF-8 SVG。三者都在 `DuiLib::CIconLibrary` 命名下按库名分发到底层 7 个库类，未知库名返回 `0`/`NULL`。
- `GetDataByName(库名, 图标名)` 按名取图标 SVG（等同各库 `GetIcon`），`GetIndexByName(库名, 图标名)` 按名取下标（与 `GetNameByIndex` 互逆）。均按库名分发，未知库名 / 找不到图标名返回 `NULL` / `-1`。
- 其余用法（回调、坐标、着色）与上面枚举单库一致，只是把「类名 + 实例接口」换成「库名字符串 + 静态接口」。

### `color` 着色策略

按 SVG 内容自动选择，避免填充图标被强制加 `stroke` 后「变粗」：

| 判定 | 样式 | 典型来源 |
|------|------|----------|
| `fill="#…"` 且无 `currentColor` | 不着色（保留多色） | Twemoji |
| `stroke="currentColor"` 或 `fill="none"`+有 stroke | `fill:none; stroke:#rgb` | Lucide / Tabler Outline / IconPark |
| `fill="currentColor"` 或无 stroke | `fill:#rgb; stroke:none` | Bootstrap / Remix / Tabler Filled |
| 其它 | fill+stroke 同色 | 通用回退 |

`PreferClientHit`：配置了 `color-hover` / `color-active`（或基类热态）时，不继承 `html { action: title }` 的标题拖拽，悬停才能生效。有悬停/按下视觉时默认 `cursor=hand`（仍可用 `cursor` 覆盖）。

悬停反馈建议同时用基类背景（不必另加开关）：

```xml
<SvgBox lucide="settings" width="32" height="32"
        color="var(--color-text-secondary)" color-hover="var(--color-primary)"
        background-color-hover="var(--color-bg-hover-medium)" border-radius="6" />
```

`background-color-hover` / `background-color-active` / `border-color-hover` 等见 [Control.md](Control.md)；图标色仍用本页 `color-*`。颜色可用 `var(--token)`（热切主题重解）；SvgBox **不会**像 FontIcon 那样自动套 chrome，需皮肤显式写 token。

悬停底色三档（见 [Theme.md](Theme.md)）：

| Token | 强度 | 典型用途 |
|-------|------|----------|
| `color-bg-hover` | 轻 | 列表行、大面积 |
| `color-bg-hover-medium` | 中 | 工具栏 / 图标按钮（推荐） |
| `color-bg-hover-primary` | 强 | 主色倾向；图标可用 `color-primary-text` |

### 本地位图 / favicon（.ico/.png/.jpg/.bmp/.gif/.webp）

`CSvgBoxUI` 除了 SVG，也能直接显示本地位图，**同一个 `src` / `LoadFromFile` 入口按扩展名透明分流**：

- `src` 指向 `.ico/.png/.jpg/.jpeg/.bmp/.gif/.webp` → **按位图**解码显示（含 favicon）
- `src` 指向 `.svg` / 内联 `data` / 图标库 → 按 SVG 显示（不变）

#### C++

```cpp
#include <UIlib.h>
// 1) 本地位图文件
svgbox->LoadFromFile(_T("c:/favicon.ico"));
svgbox->LoadFromFile(_T("skin/site1.png"));

// 2) 内存（二进制位图字节）
const BYTE* pBits = ...; size_t nLen = ...;   // 来自网络/下载/内存
svgbox->LoadFromMemory(pBits, nLen);

// 3) 资源（.rc 里的位图，默认 RT_RCDATA；可给类型）
//    .rc:  IDR_FAV ZIPTYPE "fav.ico"   → LoadFromResource(_T("ZIPTYPE"), IDR_FAV)
svgbox->LoadFromResource(_T("ZIPTYPE"), IDR_FAV);
svgbox->LoadFromResource(NULL, IDR_FAV);            // RT_RCDATA
```

#### XML

```xml
<!-- src 指向本地图片文件，自动按位图显示 -->
<SvgBox src="skin/favicons/site1.ico" width="24" height="24" />
<SvgBox src="c:/favicon.png" width="32" height="32" />
```

#### 行为约定

- **透明分流**：`LoadFromFile(path)` / `src=path` 按扩展名自动选择位图或 SVG；`LoadFromMemory` / `LoadFromResource` 固定按位图。
- **等比缩放 contain**：按控件尺寸等比、居中、不拉伸变形；`width/height`/布局里的尺寸变化会重建缓存（自动失效重载）。
- **位图不着色**：保持原色；`color`/`color-*` 只作用于 SVG。按位图处理的扩展名为 `.ico/.png/.jpg/.jpeg/.bmp/.gif/.webp`（stb_image 解码）。
- **favicon 缓存保持 `.ico` 即可**，无需预转 PNG；位于 `src` 或 `LoadFromFile` 直接写 `.ico` 路径即可。
- 图标库 SVG（`tabler-outline` 等）仍走 lunasvg，与本功能并存、互不影响。

### 导出

```cpp
CSvgBoxUI* p = ...;
p->ExportToFile(_T("C:\\out\\icon.png"));
p->ExportToFile(_T("C:\\out\\icon.jpg"), 128, 128, (DWORD)-1, 90);
p->ExportToFile(_T("C:\\out\\raw.bmp"), 48, 48, 0); // 0=不着色

// ICO 专用（推荐）：默认 16/24/32/48/64/128/256，PNG-in-ICO，保留透明
p->ExportToIcoFile(_T("C:\\out\\app.ico"));
const int sizes[] = { 16, 32, 48, 256 }; // 也可自定义（单边≤512）
p->ExportToIcoFile(_T("C:\\out\\app.ico"), sizes, 4);
```

| API | 说明 |
|-----|------|
| `ExportToFile` | PNG / JPG / BMP；若路径为 `.ico`：未指定尺寸 → 等同 `ExportToIcoFile` 默认多尺寸；指定宽或高 → 单尺寸正方形 |
| `ExportToIcoFile(path)` | **Windows 壳图标推荐**；标准七档 16/24/32/48/64/128/256，系统按场景选用 |
| `ExportToIcoFile(path, sizes, count)` | 自定义边长列表（≤512，去重；≥256 在目录项里宽高写 0，真实尺寸在 PNG） |

`dwTintColor`：`(DWORD)-1` 用 `GetPaintColor()`；`0` 不着色。JPEG 叠白底无透明；ICO/PNG 保留 alpha。
