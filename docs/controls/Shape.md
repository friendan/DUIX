# Shape（异形）



| | |

|--|--|

| 类 | `CShapeButtonUI` / `CShapeBoxUI`；窗口 `shape-image` |

| XML | `<ShapeButton>` / `<ShapeBox>`；`<html shape-image="…">` |

| 源码 | `Utils/UIShape.*`、`Control/UIShapeButton.*`、`WinImplBase::FitToShapeImage` |



**不改**基类 `FindControl`：仅新控件 / 可选窗口属性。



### ShapeButton



```xml

<ShapeButton shape-image="shape_circle.png" width="96" height="96" text="圆" />

<!-- 命中与绘制分离 -->

<ShapeButton shape-mask="mask.png" image="skin.png" image-hover="skin_hot.png" width="96" height="96" />

```



| 属性 | 说明 |

|------|------|

| `shape-image` / `src` | 异形图；未写 `image` 时兼作常态绘制 |

| `shape-mask` | **仅命中**用的 alpha 图；省略则用 `shape-image` |

| `shape-alpha-threshold` | alpha ≥ 此值算实体，默认 `16` |

| `image` / `image-hover` / … | 同 [Button](Button.md)，与 mask 独立 |



外形外点不中（可点穿）。默认手型光标。



### ShapeBox



```xml

<ShapeBox shape-image="shape_star.png" width="120" height="120">

  <Label text="仅星内可点子控件" align="center" />

</ShapeBox>

```



`shape-mask` 规则同按钮；底图为 `shape-image`。



### 异形窗口



```xml

<html layered="true" shape-image="apple.png" shape-drag="true"

    background-color="#00000000" background-image="apple.png">

```



| 属性 / API | 说明 |

|------------|------|

| `shape-image` | 外形参考图（分层靠像素 alpha；非分层可 RGN） |

| `shape-mask` | 非分层 RGN / 命中参考；省略用 `shape-image` |

| `shape-alpha-threshold` | 默认 `16` |

| `shape-drag` | 默认 `true`：未写 `action` 时自动 `move` |

| `FitToShapeImage()` | `WindowImplBase`：按图像素 `ResizeClient`，钳工作区并居中 |

| `CalcShapeWindowClientSize` / `FitToShapeImage(HWND)` | `CPaintManagerUI` |

| `SetShapeImageFromMemory` / `SetShapeMaskFromMemory` | 内存 PNG |



- **分层（推荐）**：不设 `SetWindowRgn`（防 AA「裂开」）

- **非分层**：`ApplyWindowShapeRgn` 用 hit 图建 RGN

- Demo：`FitToShapeImage()`；`shapedemo.html` + `apple.png`



### 工具 API



见 `Utils/UIShape.h`（`CreateRegionFromAlphaImage`、`HitTestAlphaInDestRect` 等）。



### Demo



`duidemo` → 基础 → **Shape（异形）**（含 ShapeBox）；「打开异形窗」。


