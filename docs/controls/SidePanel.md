# SidePanel

树内侧滑抽屉：半透明遮罩 + 左/右/上/下面板滑入，XML 子节点为面板内容。

| | |
|--|--|
| 类 | `CSidePanelUI` |
| 标签 | `<SidePanel>` / `sidepanel` |
| 源码 | `src/DuiLib/Control/UISidePanel.*` |
| Demo | Accordion → SidePanel（反馈） |

与 [Modal](Modal.md) 不同：不另开 HWND，挂在控件树（通常 root 下 `position="0,0,1,1"`）。

---

## 最小示例

```xml
<html theme="chrome">
  <VBox name="root">
    <!-- …主界面… -->
    <SidePanel name="drawer" position="0,0,1,1" placement="right"
        panel-width="40%" panel-height="45%" title="设置" closable="true"
        mask="true" mask-color="#00000060" click-mask-close="true"
        esc-close="true" duration="200" visible="false">
      <Label text="内容" height="28" />
      <Button name="btn_close_drawer" text="关闭" kind="primary" height="32" />
    </SidePanel>
  </VBox>
</html>
```

```cpp
CSidePanelUI* p = static_cast<CSidePanelUI*>(pm.FindControl(_T("drawer")));
p->SetPlacement(CSidePanelUI::PlacementBottom);
p->Show(true);
// p->Hide(true);  p->Toggle(true);
```

---

## 属性

| 属性 | 默认 | 说明 |
|------|------|------|
| `placement` | `right` | `left` / `right` / `top` / `bottom` |
| `panel-width` / `width` | `320` | 左/右面板宽；支持 `40%`（相对宿主） |
| `panel-height` / `height` | `280` | 上/下面板高；支持 `45%` |
| `mask` | `true` | 是否显示遮罩 |
| `mask-color` | `#00000060` | 遮罩色（含 alpha） |
| `click-mask-close` | `true` | 点遮罩关闭 |
| `esc-close` | `false` | 按 Esc 关闭（默认关） |
| `closable` / `show-close` | `true` | 标题栏关闭钮 |
| `title` | 空 | 有标题或 closable 时建 header |
| `header-action` | `title` | 标题栏行为：`title` / `movewindow` 拖主窗；`none` / `false` 取消 |
| `header-drag` | `true` | 快捷开关；`false` 等价 `header-action="none"` |
| `fill-host` | `false` | 铺满宿主区（厚=100%）；默认关遮罩；适合「设置」盖满主窗 |
| `host-resize` | 随 fill-host | 铺满时面板边缘缩放**宿主 HWND**（需窗口 `size-box`；最大化跳过） |
| `duration` | `200` | 动画毫秒 |
| `position` | 建议 `0,0,1,1` | absolute 铺满父容器 |
| `visible` | `false` | 初始关闭；开启动画请用 `Show()` |

---

## API

| 方法 | 说明 |
|------|------|
| `Show` / `Hide` / `Toggle` | 开/关（可带动画） |
| `IsOpen` | 是否处于打开态 |
| `SetPlacement` | `PlacementLeft/Right/Top/Bottom` |
| `SetPanelWidth` / `SetPanelWidthPercent` | 左右宽（像素 / 0~1） |
| `SetPanelHeight` / `SetPanelHeightPercent` | 上下高（像素 / 0~1） |
| `SetEscClose` | 是否响应 Esc |
| `SetDuration` | 动画时长 |
| `SetHeaderAction` | 标题栏拖窗：`UIACTION_TITLE` / `NONE` 等 |
| `SetFillHost` / `IsFillHost` | 铺满宿主；开时默认 `host-resize`、关遮罩、宽高 100% |
| `SetHostResize` / `IsHostResize` | 铺满时边缘是否缩放宿主 |
| `HitHostResize` | 供 `WindowImplBase::OnNcHitTest` 命中宿主边 |
| `ApplyThemeChrome` | 主题套面板底/边/标题色 |

通知：`sidepanelopen`（`Show` 开始）、`sidepanelclose`（关闭完成）。

打开时：优先把焦点放在关闭钮，否则内容区第一个 `TABSTOP` 控件；关闭后尝试恢复原先焦点。

---

## 主题

`theme="chrome"` 子树内：面板底 ← `color-bg-elevated`，边框 ← `color-border`，标题字 ← `color-text`。遮罩色保留皮肤属性。

---

## 注意

- 关闭且非动画中 `visible=false`，不挡下层点击。
- 子节点经 `Add` 进内容区（header/遮罩为内建 chrome）。
- 宿主在 `html{action:title}` 下，遮罩与关闭钮已做 `PreferClientHit`，避免误成拖窗。
- 标题栏默认可拖主窗口；`header-drag="false"` 或 `header-action="none"` 可取消；关闭钮仍可点。
- `fill-host`：面板铺满宿主（与 `WindowImplBase::SyncOwner*` 不同——无独立 HWND，直接拖/缩**本窗**）。打开端点用整数宿主矩形，避免右/下负向插值露 1px。Demo：Accordion → SidePanel →「铺满·右/左/上/下」。
- `esc-close` 仅在焦点位于抽屉子树内时生效（打开后会自动聚焦抽屉内）。
- **独立 HWND 内容盖不住**：SidePanel 画在主窗客户区，[WebBrowser](WebBrowser.md)（`host=window` / `composition`）等子 HWND 会压在遮罩之上。库不自动挂起；业务在 `sidepanelopen` / `Show` 时对当前页 `WebBrowser::SetVisible(false)`，在 `sidepanelclose` / `Hide` 完成后再恢复（与 TabLayout 切页显隐同路径）。Modal/Toast 因另开 popup HWND，一般无此问题。
