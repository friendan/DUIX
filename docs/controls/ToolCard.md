# ToolCard

| | |
|--|--|
| 类 | `CToolCardUI` / `CToolCardHeaderUI` / `CToolCardBodyUI` |
| XML | `<ToolCard>` / `<ToolCardHeader>` / `<ToolCardBody>`（`toolcard` 亦可） |
| 源码 | `src/DuiLib/Control/UIToolCard.*` |
| 继承 | [Container](Container.md) → VerticalLayout |

聊天工具卡（对齐 Cursor Agent 工具结果块）：**标题栏 + 内容区**。

### 布局

```
[ ▾ | badge | title 弹性 ][ ToolCardHeader 自定义槽 ]  ← 标题栏
[ ToolCardBody ]                                      ← 内容区
```

- **内建 chrome**（箭头 / badge / 标题）：在 `ToolCard` 上用属性或 `GetChevron()` / `GetKindBadge()` / `GetTitleLabel()` / `GetHeaderChrome()` 改
- `<ToolCardHeader>` → **仅右侧自定义槽**（不能替换内建箭头/badge）
- `<ToolCardBody>` → 内容区；未写时普通子节点仍进内建 Body

### 交互

| 区域 | 行为 |
|------|------|
| 箭头 / badge / 标题栏空白 | 展开 / 折叠（`itemexpand` / `itemcollapse`） |
| 蓝色文件名（`kind=file`） | `toolcardopen`；亦可用 `OpenFile()` |
| `ToolCardHeader` 内按钮 | 自己吃点击 |
| `ToolCardBody` 右键 | 默认 **全选 / 复制**（`menu="false"` 关闭）；Ctrl+A / Ctrl+C |

### 最小示例

```xml
<ToolCard kind="cmd" command="build.bat" expanded="true" body-max-height="500"
    show-badge="true" badge-text="RUN" header-bk="#F1F3F5FF">
  <ToolCardHeader gap="6">
    <Label text="done" color="#198754FF" width="40" height="20" />
  </ToolCardHeader>
  <ToolCardBody>
    <Label text="编译输出…" color="#495057FF" />
  </ToolCardBody>
</ToolCard>
```

### 属性（XML）

| 属性 | 说明 | 默认 |
|------|------|------|
| `kind` | `file` / `cmd` / `command` / `shell` / `generic` | `generic` |
| `title` / `text` | 覆盖标题；空则 file→path、cmd→command | |
| `path` / `file` | 文件路径 | |
| `command` / `cmd` | 命令行文案 | |
| `expanded` | 是否展开 | `true` |
| `header-height` | 标题栏高度 | `32` |
| `show-chevron` | 显示展开箭头 | `true` |
| `show-badge` / `show-kind-badge` | 显示 kind 徽章 | `true` |
| `show-title` | 显示标题 | `true` |
| `header-bk` / `header-background-color` | 标题栏底色 | `#F1F3F5FF` |
| `header-hover-bk` | 悬停底色 | `#E9ECEFFF` |
| `chevron-color` | 箭头颜色 | `#6C757DFF` |
| `title-color` | 普通标题色 | `#212529FF` |
| `title-link-color` | file 文件名链接色 | `#0D6EFDFF` |
| `badge-text` | 徽章文案；空=按 kind 自动 | |
| `badge-color` / `badge-text-color` | 徽章字色 | `#FFFFFFFF` |
| `badge-bk` | 徽章底色；`auto`/`0`=按 kind | 自动 |
| `badge-width` | 徽章宽度 | `44` |
| `body-height` | 内容区固定视口高；`0`=估高 | `0` |
| `body-min-height` / `min-body-height` | 最小高度；`0`=不强制 | `100` |
| `body-max-height` / `max-body-height` | 最大高度；超出滚动；`0`=不限 | `500` |
| `body` / `body-text` | 无子项时的预览文本 | |
| Body `menu` / `contextmenu` | 右键菜单 | `true` |

高度：`clamp(内容或 body-height, min, max)`。

### C++ API

| 方法 | 说明 |
|------|------|
| `SetCardKind` / `GetCardKind` | 枚举 |
| `SetCardKindString` / `GetCardKindString` | `file` / `cmd` / `generic` |
| `SetTitle` / `GetTitle` | 标题 |
| `SetPath` / `GetPath` | 路径；generic 时设 path 会升为 file |
| `SetCommand` / `GetCommand` | 命令；generic 时设 command 会升为 cmd |
| `SetExpanded` / `IsExpanded` / `ToggleExpanded` | 展开 |
| `OpenFile` | 发 `toolcardopen`（仅 file） |
| `SetHeaderHeight` / `GetHeaderHeight` | 标题栏高 |
| `SetShowChevron` / `SetShowKindBadge` / `SetShowTitle` | 内建显隐 |
| `SetHeaderBkColor` / `SetHeaderHoverBkColor` | 标题栏底色 |
| `SetChevronColor` / `SetTitleColor` / `SetTitleLinkColor` | 字色 |
| `SetKindBadgeText` / `SetKindBadgeBkColor` / `SetKindBadgeTextColor` / `SetKindBadgeWidth` | 徽章 |
| `GetHeaderChrome` / `GetChevron` / `GetKindBadge` / `GetTitleLabel` | 内建控件指针（可再改 font 等） |
| `GetHeaderSlot` / `GetBody` | 自定义槽 |
| `SetBodyHeight` / `GetBodyHeight` | 固定视口 |
| `SetBodyMinHeight` / `GetBodyMinHeight` | 最小 |
| `SetBodyMaxHeight` / `GetBodyMaxHeight` | 最大 |
| `SetBodyText` / `GetBodyText` | 内建预览 Label 文本 |
| `AppendBodyLine` | 追加一行日志 Label |
| `ClearBody` | 清空内容区 |
| `CollectBodyText` | 收集全部可见文本 |
| `SelectAllBody` / `CopyBodyText` | 全选高亮 / 复制到剪贴板 |
### 通知

| 宏 | 字符串 | 说明 |
|----|--------|------|
| `DUI_MSGTYPE_TOOLCARDOPEN` | `toolcardopen` | file 打开；`lParam`=`CToolCardUI*`，路径 `GetPath()` |
| `DUI_MSGTYPE_ITEMEXPAND` | `itemexpand` | 展开 |
| `DUI_MSGTYPE_ITEMCOLLAPSE` | `itemcollapse` | 折叠 |

```cpp
if( msg.sType == DUI_MSGTYPE_TOOLCARDOPEN ) {
  CToolCardUI* p = static_cast<CToolCardUI*>(msg.pSender->GetInterface(DUI_CTR_TOOLCARD));
  // ShellExecute / 业务打开 p->GetPath()
}
```

### Demo

`duidemo` → Accordion「ToolCard」。

