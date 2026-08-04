# DateTime

| | |
|--|--|
| 类 | `CDateTimeUI` |
| XML | `<DateTime>` |
| 源码 | `src/DuiLib/Control/UIDateTime.*` |
| 继承 | [Label](Label.md) |

自绘日期/时间选择：点击字段弹出面板（Combo 式 `WS_POPUP`）。支持月历、月/年面板、时分秒。**不使用**系统 DateTimePicker。

### 最小示例

```xml
<DateTime width="160" height="32" format="yyyy-MM-dd"
    border="1px solid #D9D9D9" border-radius="4" padding="0,24,0,8" />
<DateTime width="200" height="32" format="yyyy-MM-dd HH:mm:ss"
    border="1px solid #D9D9D9" border-radius="4" padding="0,24,0,8" />
<DateTime width="120" height="32" format="HH:mm"
    border="1px solid #D9D9D9" border-radius="4" padding="0,24,0,8" />
```

### format（驱动面板内容）

| `format` | 日期 | 时间 | 秒 |
|----------|------|------|----|
| `yyyy-MM-dd` | ✓ | | |
| `yyyy-MM-dd HH:mm` | ✓ | ✓ | |
| `yyyy-MM-dd HH:mm:ss` | ✓ | ✓ | ✓ |
| `HH:mm` | | ✓ | |
| `HH:mm:ss` | | ✓ | ✓ |

也可用 `show-date` / `show-time` / `show-seconds` 覆盖（设后不再跟 format 自动推导）。

### 弹层交互

| 操作 | 说明 |
|------|------|
| 点标题 | 日 → 月 → 年；选年回月，选月回日 |
| `<` `>` | 翻月 / 翻年 / 翻十年 |
| 点日期 | 仅日期：选中并关闭；带时间：选中日期，点「确定」关闭 |
| 时分秒 | ▲▼ 或滚轮；带时间时底部有「确定」 |
| 今天 | 跳到今日（带时间时保留当前时分秒，需再「确定」） |
| Esc / 失焦 | 关闭 |

### 其它属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `readonly` | 不弹出 | `false` |
| `show-today` | 「今天」按钮 | `true` |
| `first-day-of-week` | `0`/`sun` 或 `1`/`mon` | `0` |
| `selected-background-color` | 选中格 | `#1677FFFF` |
| `day-hover-background-color` | 悬停 | `#E6F4FFFF` |
| `today-color` | 今日字 / 按钮 | `#1677FFFF` |
| `other-month-color` | 邻月 / 十年外 | `#BFBFBFFF` |

### 通知

| 类型 | 说明 |
|------|------|
| `valuechanged` | 日期或时间相对上次有变化（选日、调时、确定、今天） |

### C++ API

`GetTime` / `SetTime`、`SetFormat`、`SetShowTime` / `SetShowSeconds`、`ActivateDropDown` / `CloseDropDown`、`SetReadOnly`。
