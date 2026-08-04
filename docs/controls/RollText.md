# RollText

| | |
|--|--|
| 类 | `CRollTextUI` |
| XML | `<RollText>` |
| 源码 | `src/DuiLib/Control/UIRollText.*` |
| 继承属性 | 见 [Label.md](Label.md) |

跑马灯文字：继承 Label 的 `text` / `color` / `font-*` / `html` 等；**滚动需 C++ 调用** `BeginRoll` / `EndRoll`，无对应 XML 属性。

### 最小示例

```xml
<RollText name="marquee" text="超长公告内容……" height="28" color="#333333FF" />
```

```cpp
CRollTextUI* p = static_cast<CRollTextUI*>(m_pm.FindControl(_T("marquee")));
// 方向：ROLLTEXT_LEFT/RIGHT/UP/DOWN；步进定时默认 50ms；最长滚动秒数默认 60
p->BeginRoll(ROLLTEXT_LEFT, 50, 60);
// …
p->EndRoll();
```

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `text` / `color` / `font-*` / `html` 等 | 同 Label |

### C++ API

| 方法 | 说明 |
|------|------|
| `BeginRoll(nDirect, lTimeSpan, lMaxTimeLimited)` | 开始滚动；`lTimeSpan` 为帧间隔 ms；`lMaxTimeLimited` 为最长秒数 |
| `EndRoll()` | 停止 |

方向宏（`UIRollText.h`）：

| 宏 | 值 | 方向 |
|----|-----|------|
| `ROLLTEXT_LEFT` | 0 | 向左 |
| `ROLLTEXT_RIGHT` | 1 | 向右 |
| `ROLLTEXT_UP` | 2 | 向上 |
| `ROLLTEXT_DOWN` | 3 | 向下 |

内部步进 `m_nStep` 固定为 5（像素级逻辑偏移，无 XML）。

### 通知

| 类型 | 说明 |
|------|------|
| `textrollend`（`DUI_MSGTYPE_TEXTROLLEND`） | 达到 `lMaxTimeLimited` 时发出（未必已 `EndRoll`） |
