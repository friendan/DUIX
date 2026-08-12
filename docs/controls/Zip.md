# ZIP

对外请用 **`CZipFile`**（CRUD）。底层仍是 Lucian Wischik Zip Utils；改/删没有原地 API，封装里会重写整包。

| | |
|--|--|
| 封装 | `CZipFile` / `TZipItem` |
| 头文件 | `src/DuiLib/Utils/UIZip.h`（`UIlib.h` 已包含） |
| 源码 | `src/DuiLib/Utils/UIZip.cpp` |
| 底层 | `zip.h` / `unzip.h` |
| 皮肤 | `CPaintManagerUI::SetResourceZip` / `LoadResourceData` |

---

## CZipFile（推荐）

密码用 `SetPassword`（`LPCTSTR`）。默认 **UNICODE → UTF-8**，不随系统代码页变化。`ZIP_PWD_ASCII` 则要求纯 ASCII。打开与压缩必须同一编码。

```cpp
zip.SetPassword(_T("secret"));                     // 默认 UNICODE / UTF-8
zip.SetPassword(_T("中文密码"));                    // 宽字符 → UTF-8 字节
zip.SetPasswordEncoding(ZIP_PWD_ASCII);            // 含非 ASCII 则失败
zip.SetPasswordEncoding(ZIP_PWD_UNICODE, CP_ACP);  // 仅用于打开旧的系统 ANSI 包
```

| 操作 | 方法 | 说明 |
|------|------|------|
| **C** 创建 | `Create` / `CreateMemory` + `AddFile` / `AddMemory` / `AddFolder` / `AddDir` | 创建会话里连续 Add 走原生写入 |
| **R** 读取 | `Open` / `OpenMemory` / `OpenResource` + `GetCount` / `GetItem` / `Find` / `Exists` / `IsDir` | | |
| **R** 解压 | `ExtractFile` / `ExtractDir` / `ExtractAll` / `ExtractMemory` | `ExtractMemory` 的缓冲 `delete[]` |
| **U** 更新 | `UpdateFile` / `UpdateMemory`，或 `Add*(..., bReplace=true)` | 已打开的包会重写整包 |
| **D** 删除 | `Remove(name)` / `RemoveDir(dir)` | `Remove` 删单条；`RemoveDir` 删目录及 `dir/` 下全部子路径 |

创建会话尚未 `Close` 时调用读/改/删，会先提交再打开。已存在条目且 `bReplace=false` 则 Add 失败。

包内名支持多层目录，`/` 与 `\` 等价；**前导 `/` 会去掉**（ZIP 里存相对路径）。不必先 `AddFolder` 父目录。`ExtractAll` / `ExtractDir` 会按层级建目录。查找/删除时带不带前导 `/` 都可以。不支持通配符（`*.txt`、`**`）。

安全约定：

- 条目名禁止 `..`、`.` 段、盘符、`:` `*` `?` `"<>|`、控制字符、Windows 设备名（`CON`/`NUL`/`COM1` 等）
- `ExtractAll` / `ExtractDir` 解压后路径必须仍在目标目录内（防 zip-slip）
- 单条未压缩大小默认上限 **512MB**（`SetMaxUncompressedSize`；`0` = 不限制，仅可信包）
- 改包时用临时文件 + `ReplaceFile`，失败不删原包
- 密码：默认 UNICODE → UTF-8；`ZIP_PWD_ASCII` 才要求纯 ASCII。`AddDir` 不跟随交接点（reparse point）

```cpp
zip.AddFile(_T("/ssss/sss/aa/sss.txt"), _T("c:\\data\\sss.txt"));
// 实际条目名：ssss/sss/aa/sss.txt
zip.Exists(_T("/ssss/sss/aa/sss.txt"));          // true
zip.ExtractFile(_T("ssss/sss/aa/sss.txt"), _T("c:\\tmp\\sss.txt"));
zip.ExtractDir(_T("ssss/sss"), _T("c:\\tmp\\sss")); // 目录内容解到目标（去掉前缀）
zip.RemoveDir(_T("/ssss/sss"));                  // 删目录 ssss/sss 及其下全部文件
zip.Remove(_T("old.png"));                       // 只删这一条
zip.AddDir(_T("skin"), _T("c:\\skin"), true);    // 磁盘目录打进包
```

```cpp
CZipFile zip;
zip.Create(_T("c:\\out\\skin.zip"));
zip.AddFile(_T("main.xml"), _T("c:\\skin\\main.xml"));
zip.AddFile(_T("img/btn.png"), _T("c:\\skin\\img\\btn.png"));
zip.AddMemory(_T("ver.txt"), "1.0", 3);
zip.Close();

zip.Open(_T("c:\\out\\skin.zip"));
zip.UpdateFile(_T("main.xml"), _T("c:\\skin\\main.xml"));
zip.AddMemory(_T("ver.txt"), "1.1", 3);     // 已存在则替换
zip.Remove(_T("old.png"));
zip.RemoveDir(_T("cache"));                 // 目录及子条目

int n = zip.GetCount();
TZipItem item;
for( int i = 0; i < n; ++i ) {
    zip.GetItem(i, item);
    // item.name / item.unc_size / item.IsDirectory()
}

zip.ExtractAll(_T("c:\\tmp\\skin"));
BYTE* p = NULL; DWORD cb = 0;
if( zip.ExtractMemory(_T("main.xml"), &p, &cb) ) {
    // 使用 p[0 .. cb)
    delete[] p;
}

// 内存包
CZipFile mem;
mem.CreateMemory(8 * 1024 * 1024);
mem.AddMemory(_T("a.txt"), "hello", 5);
BYTE* pZip = NULL; DWORD nZip = 0;
mem.GetMemory(&pZip, &nZip);
CPaintManagerUI::SetResourceZip(pZip, nZip);
delete[] pZip;

// 一句解压
CZipFile::UnzipToDirectory(_T("skin.zip"), _T("c:\\tmp\\skin"));
```

| 方法 | 说明 |
|------|------|
| `SetPassword` / `GetPassword` | 整包同一密码；空 = 无密码 |
| `SetPasswordEncoding` | `ZIP_PWD_ASCII` 或 `ZIP_PWD_UNICODE`（默认 UTF-8；旧包可传 `CP_ACP`） |
| `Create(路径)` | 新建/覆盖磁盘 ZIP，进入写入会话 |
| `CreateMemory(nMaxBytes)` | 内存 ZIP，`nMaxBytes` 为**压缩后整包上限** |
| `Open` / `OpenMemory` / `OpenResource` / `OpenResourceFile` | 打开 zip 文件 / 内存 / 已加载模块 / 磁盘 EXE、DLL 资源 |
| `Close` / `IsOpen` / `IsMemory` / `GetPath` | |
| `SaveAs` / `SaveResource` / `SaveResourceDll` | 另存 zip；写回已有 EXE/DLL 资源；**新建**仅含资源的 DLL |
| `GetMemory` | 整包字节，`delete[]` |
| `AddDir` / `ExtractDir` / `IsDir` | 打包磁盘目录；解出目录（去掉前缀）；判断是否为目录 |
| `Remove` / `RemoveDir` | 删文件；删目录（含 `dir/` 子路径）。目录不存在或目标是文件则失败 |
| `SetMaxUncompressedSize` | 单条解压上限，默认 512MB；`0` 不限制 |
| `GetLastResult` / `GetErrorMessage` | `ZRESULT` 与说明 |

---

## 皮肤资源 ZIP

`LoadResourceData` 查找顺序：

1. 磁盘 `GetResourcePath() + 相对路径`（开发热更新）
2. `SetResourceZip` 的包
3. 再试绝对路径

成功时 `*ppData` 为 `new BYTE[]`，调用方 `delete[]`。

```cpp
CPaintManagerUI::SetResourcePath(_T("c:\\app\\skin\\"));
CPaintManagerUI::SetResourceZip(_T("skin.zip"), true, NULL);
// 或：SetResourceZip(pBuf, nLen, NULL);

BYTE* p = NULL; DWORD n = 0;
if( CPaintManagerUI::LoadResourceData(_T("main.xml"), &p, &n) ) {
    delete[] p;
}
```

包内路径要和 XML 相对路径一致（`bkimage="img/btn.png"` → ZIP 内 `img/btn.png`）。`CZipFile` 打出来的包可直接给 `SetResourceZip`。

EXE/DLL 里嵌入的 ZIP（一般为 `RT_RCDATA`）用 `OpenResource`（已加载模块）或 `OpenResourceFile`（磁盘上的外部文件）。写回用 `SaveResource`。写的是**磁盘文件**，目标不能正在运行/被占用；改资源会作废该文件的 Authenticode 签名。

```cpp
CZipFile zip;
zip.SetPassword(_T("中文密码"));

// 本模块
zip.OpenResource(NULL, IDR_SKIN_ZIP);

// 外部 EXE / DLL（只当数据文件加载，不执行）
zip.OpenResourceFile(_T("c:\\app\\player.exe"), IDR_SKIN_ZIP);
zip.OpenResourceFile(_T("c:\\app\\skin.dll"), _T("SKIN"), RT_RCDATA);

zip.Remove(_T("old.png"));
zip.AddFile(_T("main.xml"), _T("c:\\skin\\main.xml"));

// 写回外部文件（player.exe 不能正在运行）
if( !zip.SaveResource(_T("c:\\app\\player.exe"), IDR_SKIN_ZIP) ) {
    // GetErrorMessage()：文件占用 / 权限 / 不是 PE
}
zip.SaveAs(_T("c:\\tmp\\skin.zip"));   // 也可以另存为独立 zip

// 动态生成只含资源的 DLL（无 DllMain，架构与当前程序一致）
zip.SaveResourceDll(_T("c:\\app\\skin.dll"), IDR_SKIN_ZIP);
zip.SaveResourceDll(_T("c:\\app\\skin.dll"), _T("SKIN"), RT_RCDATA);
```

---

## 底层 API

一般不必直接用。创建与打开是两种 `HZIP`，**不能混用**（`ZR_ZMODE`）。没有原地改/删。

```cpp
HZIP hz = CreateZip(_T("a.zip"), NULL);
ZipAdd(hz, _T("a.txt"), _T("c:\\a.txt"));
CloseZip(hz);

hz = OpenZip(_T("a.zip"), NULL);
ZIPENTRY ze = {};
GetZipItem(hz, -1, &ze);
int n = ze.index;
FindZipItem(hz, _T("a.txt"), true, &i, &ze);
UnzipItem(hz, i, ze.name);
CloseZip(hz);
```

密码参数是 `const char*`（ASCII）。`CloseZip` 按句柄分发到创建侧或解压侧。

常用结果码：`ZR_OK`、`ZR_MORE`（解压缓冲不够）、`ZR_NOTFOUND`、`ZR_PASSWORD`、`ZR_CORRUPT`、`ZR_NOFILE`、`ZR_MEMSIZE`、`ZR_ZMODE`。`FormatZipMessage` 可转成字符串。

首次把 `zip.cpp` / `UIZip.cpp` 加进工程后需 **CMake 重新 configure**（`aux_source_directory(Utils)` 只在 configure 时扫文件）。
