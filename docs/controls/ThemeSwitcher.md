# ThemeSwitcher



| | |

|--|--|

| 类 | `CThemeSwitcherUI` / `CThemePickerWnd` |

| XML | `<ThemeSwitcher>` / `themeswitcher` |

| 源码 | `src/DuiLib/Control/UIThemeSwitcher.*` |

| 预览窗皮肤 | 内嵌于 `UIThemeSwitcher.cpp`（宽字面量 `LR"dui(...)"dui`；工程仅 Unicode），无需再附带 html |

| 继承 | Button |



点击后弹出**两列主题窗**：左主题名、右色值预览；可新建主题、**色系一键 / 简易种子 / 高级全量**、导入/另存 `:root` 主题文件。正式切换经 `CThemeManager` 通知外部，便于持久化当前主题 id。



默认显示 **Lucide `palette` 调色板图标**（无字）。图标色默认随主题刷新（`tint-auto=true`）：标题栏内跟系统按钮一致（`titlebar-text` + 半透明悬停底，避免 primary 与标题栏同色时隐身）；其它位置跟 `color-text`，悬停用 `color-primary`。可用 `lucide` / `tabler-outline` 等覆盖图标；`tint-auto="false"` 时保留皮肤里的 `icon-tint*`。



### 最小示例



```xml

<TitleBar height="40" ...>

  <Spacer />

  <ThemeSwitcher name="themeSwitch" width="36" height="40" margin="0,8,0,0" />

</TitleBar>

```



```cpp

// 进程级：落盘当前主题 id + 用户主题文件路径列表

class CThemePersist : public IThemeNotifyUI {

public:

	virtual void OnThemeChanged(LPCTSTR /*oldId*/, LPCTSTR newId, bool bPreview) {

		if( bPreview ) return;

		::WritePrivateProfileString(_T("ui"), _T("theme"), newId, m_ini);

	}

	CDuiString m_ini;

};



// 启动：先加载用户主题文件，再 Apply 上次 id

CThemeManager* tm = CThemeManager::GetInstance();

CThemeSwitcherUI* pSw = ...; // FindControl

// 读取 ini 里保存的路径列表，逐个加入显示列表（不切换）

pSw->AddThemeFile(_T("themes\\myblue.css"));

pSw->AddThemeFile(_T("themes\\night.css"));

tm->ApplyTheme(::读出的 id); // 或 pSw 所在窗 Init 后再 Apply

```



另存为成功时 ThemeSwitcher 发 `themefilesaved`：`wParam`=主题 id，`lParam`=文件路径（回调内立刻拷贝）。可把路径写入 ini，下次启动 `AddThemeFile`。



### 预览窗



| 区域 | 说明 |

|------|------|

| 新建主题 | 从当前主题拷贝色板并注册，进入可编辑态 |

| 色系 / 简易 / 高级 | 三段模式。默认**色系**：选绿/青/蓝/紫/橙/红/灰/深 + **色相微调** + **明暗滑块** |

| 色系 | 点色系 + 色相微调（±30°）+ 明暗；换色系重置色相 |

| 简易 | 7 个种子色；改色即生成整盘 |

| 高级 | 全量 token 表 |

| 导入… | `LoadThemeFile` + 加入显示列表（预览选中，不强制正式 Apply） |

| 另存为… | `SaveThemeFile`；发 `themefilesaved` |

| 左列 | 已注册且通过白名单的主题 |

| 预览寿命 | 直到 **确定** / **取消·关窗** |



改内置主题时自动分叉为 `custom_*`。无 Esc 关窗。



### 属性



| 属性 | 说明 |

|------|------|

| `themes` | 逗号分隔 id 白名单；**空=全部已注册** |

| `tint-auto` | 默认 `true` |

| `modal` | 默认 `false`（非模态、不设 owner，**显示最小化**）；`true` 则 ShowModal 禁用主窗并隐藏最小化 |



### API（持久化用户主题）



| 方法 | 说明 |

|------|------|

| `AddTheme(CTheme*, bOwn)` | `RegisterTheme`；白名单非空时 `IncludeTheme` |

| `AddThemeFile(path, idOverride)` | `LoadThemeFile`（只注册不切换）+ `IncludeTheme`；失败返回 NULL |

| `IncludeTheme(id)` / `ExcludeTheme(id)` | 维护 `themes` 白名单（空白名单表示显示全部，`Include` 为 no-op） |

| `IsThemeListed(id)` | 是否会出现在选择窗列表 |



`CThemeManager::LoadThemeFile`：只注册；`ApplyThemeFile`：注册并正式切换。



### 通知



| 类型 | 时机 |

|------|------|

| `selectchanged` | 预览窗确定后 |

| `themefilesaved` | 另存为成功；`wParam`=id，`lParam`=路径 |

| `IThemeNotifyUI::OnThemeChanged` | 正式 `ApplyTheme`；`bPreview` 区分预览 |



另见 [Theme.md](Theme.md)。


