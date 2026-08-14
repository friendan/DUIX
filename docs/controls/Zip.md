# ZIP

对外请用 **`CZipFile`**（CRUD）。底层为 **minizip-ng + zlib**；改/删没有原地 API，封装里会重写整包。密码加密为 **WinZip AES**（不兼容旧 XZip / ZipCrypto）。

| | |
|--|--|
| 封装 | `CZipFile` / `TZipItem` |
| 头文件 | `src/DuiLib/Utils/UIZip.h`（`UIlib.h` 已包含） |
| 源码 | `src/DuiLib/Utils/UIZip.cpp` |
| 底层 | minizip-ng 4.2.2 + zlib 1.3.2（`src/3rd/`，链接 `minizip_ng::minizip_ng`） |
| 皮肤 | `CPaintManagerUI::SetResourceZip` / `LoadResourceData` |

---

## CZipFile（推荐）

密码用 `SetPassword`（`LPCTSTR`）。宽字符**固定转 UTF-8**（与系统代码页无关）。有密码时写 **WinZip AES**；未设密码（或空 / NULL）则普通明文包。可在 `Open` / `Create` **之后**再改密码（会同步到当前 reader/writer）。

`SetCompressLevel(-1..9)`：`-1` 默认 deflate，`0` 仅存储（适合 png/jpg），`1..9` 为 deflate 级别。

内存包 / 单条 `AddMemory` 长度上限为 **2GB-1**（minizip `int32_t`）；超限直接失败。

```cpp
zip.SetPassword(_T("secret"));
zip.SetPassword(_T("中文密码"));   // → UTF-8
zip.SetCompressLevel(0);           // STORE
if( zip.IsEncrypted() ) { /* 需密码 */ }
TZipItem item;
zip.GetItem(_T("main.xml"), item);
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
- 密码：宽字符固定 UTF-8。`AddDir` 不跟随交接点（reparse point）

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
mem.CreateMemory(0);
mem.AddMemory(_T("a.txt"), "hello", 5);
mem.Commit();                               // 可选：显式结束写入
DWORD nZip = mem.GetMemorySize();
BYTE* pZip = NULL;
mem.DetachMemory(&pZip, &nZip);             // 零额外拷贝交出整包；之后 mem 已关闭
CPaintManagerUI::SetResourceZip(pZip, nZip);
delete[] pZip;

// 零拷贝打开已有缓冲（缓冲须保持有效）
CZipFile view;
view.AttachMemory(pExisting, nExisting);

// 一句解压
CZipFile::UnzipToDirectory(_T("skin.zip"), _T("c:\\tmp\\skin"));
CZipFile::UnzipMemoryToDirectory(pZip, nZip, _T("c:\\tmp\\skin2"));
```

| 方法 | 说明 |
|------|------|
| `SetPassword` / `GetPassword` | 整包同一密码（UTF-8）；空 / NULL = 无密码；打开后可改 |
| `SetCompressLevel` / `GetCompressLevel` | `-1` 默认；`0` STORE；`1..9` deflate |
| `IsEncrypted` | 是否含加密条目（不解压） |
| `Create(路径)` | 新建/覆盖磁盘 ZIP，进入写入会话 |
| `CreateMemory(nMaxBytes)` | 内存 ZIP；`nMaxBytes` 为**可选软上限**（可增长）；`0` = 不限制 |
| `Open` / `OpenMemory` / `AttachMemory` / `OpenResource` / `OpenResourceFile` | 打开 zip；`OpenMemory` 默认拷贝；`AttachMemory` / `OpenMemory(..., false)` 零拷贝（缓冲须保持有效） |
| `Close` / `Commit` / `IsOpen` / `IsMemory` / `GetPath` | `Commit` 显式结束写入会话 |
| `SaveAs` / `SaveResource` / `SaveResourceDll` | 另存 zip；写回已有 EXE/DLL 资源；**新建**仅含资源的 DLL |
| `GetMemory` / `GetMemorySize` / `DetachMemory` | 整包拷贝；只取长度；交出内部缓冲所有权（`delete[]`） |
| `GetCount` / `GetItem` / `Find` | `GetItem` 支持下标或条目名；`GetCount` 有缓存 |
| `AddDir` / `ExtractDir` / `IsDir` | 打包磁盘目录；解出目录（去掉前缀）；判断是否为目录 |
| `Remove` / `RemoveDir` | 删文件；删目录（含 `dir/` 子路径）。目录不存在或目标是文件则失败 |
| `Test` | 解压校验全部条目（不落盘）；可选返回失败下标 |
| `SetMaxUncompressedSize` | 单条解压上限，默认 512MB；`0` 不限制 |
| `GetLastResult` / `GetErrorMessage` | minizip `MZ_*` 结果码（`MZ_OK=0`）与说明 |
| `UnzipToDirectory` / `UnzipMemoryToDirectory` | 静态：文件/内存包解到目录 |

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

## 兼容说明

- 旧 Lucian Wischik `zip.h` / `unzip.h` / ZipCrypto 已移除；请只用 `CZipFile`。
- 带密码的新包为 AES；旧 ZipCrypto 包无法用当前后端打开。
- 首次改动 `UIZip.cpp` / `src/3rd` 后需 **CMake 重新 configure**（`aux_source_directory` / 第三方目标只在 configure 时生效）。
