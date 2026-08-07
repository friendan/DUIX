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
- `esc-close` 仅在焦点位于抽屉子树内时生效（打开后会自动聚焦抽屉内）。
