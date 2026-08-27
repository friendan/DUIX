# RollText

| | |
|--|--|
| 类 | `CRollTextUI` |
| XML | `<RollText>` / `<Marquee>` |
| 源码 | `src/DuiLib/Control/UIRollText.*` |
| 继承属性 | 见 [Label.md](Label.md) |

跑马灯文字：继承 Label；可用 XML 自动开滚，也可用 C++ `BeginRoll` / `EndRoll`。

### 最小示例

```xml
<RollText roll="true" roll-direction="left" height="28" width="280"
          text="这是一条跟随主题的跑马灯公告……"
          color="var(--color-text)" background-color="var(--color-bg-elevated)"
          padding="0,8,0,8" />
<RollText roll="true" roll-loop="3" height="28"
          text="只循环 3 次后停下" color="var(--color-text)" />
<RollText roll="true" roll-pause-hover="false" height="28"
          text="悬停不暂停" color="var(--color-text)" />
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `roll` / `rolling` / `marquee` | `true` 时 `DoInit` 自动 `BeginRoll` | `false` |
| `roll-direction` / `direction` | `left`：从左到右读（A→AB→ABCD）；`right`：从右到左读（A→BA→DCBA，整段倒序）；另有 `up` / `down` | `left` |
| `roll-interval` / `roll-span` | 帧间隔 ms | `50` |
| `roll-duration` / `roll-timeout` | 最长秒数；`0`=不限时；到期停止并发 `textrollend` | `0` |
| `roll-loop` / `roll-count` / `loop` | 循环圈数；`0`=一直滚；`N`=完整滚过 N 圈后停止并发 `textrollend` | `0` |
| `roll-pause-hover` / `pause-on-hover` | 鼠标悬停是否暂停；移开继续（时长倒计时一并暂停） | `true` |
| `roll-step` | 每帧像素步进 | `1` |
| `text` / `color` / `font-*` / `html` 等 | 同 Label；`color` 支持 `var(--token)` 热切主题 | — |

### C++ API

```cpp
CRollTextUI* p = static_cast<CRollTextUI*>(m_pm.FindControl(_T("marquee")));
// 或：p->GetInterface(DUI_CTR_ROLLTEXT)

p->SetText(_T("公告内容……"));
p->SetColor(0x333333FF);                    // 或皮肤 color="var(--color-text)"
p->SetRollDirection(ROLLTEXT_LEFT);
p->SetRollInterval(30);
p->SetRollStep(1);
p->SetRollLoop(3);                          // 0=一直滚
p->SetRollDuration(0);                      // 秒；0=不限时
p->SetPauseOnHover(true);
p->BeginRoll();                             // 也可用 SetAutoRoll(true) 在 Init 时开滚

p->Pause();                                 // 程序暂停（与悬停暂停独立）
p->Resume();
if( p->IsRolling() && !p->IsPaused() ) { /* 正在动 */ }
int nDone = p->GetRollLoopDone();
p->EndRoll();
```

| 方法 | 说明 |
|------|------|
| `BeginRoll(nDirect, lTimeSpan, lMaxTimeLimited)` | 开始滚动；`lMaxTimeLimited<=0` 不限时 |
| `EndRoll()` | 停止并复位位置 |
| `Pause()` / `Resume()` | 程序暂停 / 继续 |
| `IsRolling()` / `IsPaused()` | 是否在滚 / 是否暂停（含悬停） |
| `SetAutoRoll` / `IsAutoRoll` | 对应 XML `roll` |
| `SetRollDirection` / `GetRollDirection` | `ROLLTEXT_LEFT/RIGHT/UP/DOWN` |
| `SetRollInterval` / `GetRollInterval` | 帧间隔 ms |
| `SetRollDuration` / `GetRollDuration` | 最长秒数 |
| `SetRollStep` / `GetRollStep` | 每帧像素 |
| `SetRollLoop` / `GetRollLoop` / `GetRollLoopDone` | 圈数上限 / 已完成圈数 |
| `SetPauseOnHover` / `IsPauseOnHover` | 悬停是否暂停 |

方向宏（`UIRollText.h`）：

| 宏 | 值 | 方向 |
|----|-----|------|
| `ROLLTEXT_LEFT` | 0 | 向左 |
| `ROLLTEXT_RIGHT` | 1 | 向右 |
| `ROLLTEXT_UP` | 2 | 向上 |
| `ROLLTEXT_DOWN` | 3 | 向下 |

### 通知

| 类型 | 说明 |
|------|------|
| `textrollend`（`DUI_MSGTYPE_TEXTROLLEND`） | `roll-loop` 滚完 N 圈，或 `roll-duration` 到期时发出，并停止滚动 |

### 定时器

滚动帧与 `roll-duration` 超时均经 `CreateTimerQueueTimer` → `UIMSG_ROLLTEXT_TICK`（**非** `SetTimer` / `WM_TIMER`；开 Shadow 主窗下后者会丢）。详见 [Messages.md](Messages.md)。
