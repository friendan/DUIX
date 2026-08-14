#ifndef __UIZIP_H__
#define __UIZIP_H__
#pragma once

namespace DuiLib
{
	struct TZipItem
	{
		int index;
		CDuiString name;
		DWORD attr;
		FILETIME mtime;
		long comp_size;
		long unc_size;

		TZipItem()
		{
			index = -1;
			attr = 0;
			comp_size = 0;
			unc_size = 0;
			memset(&mtime, 0, sizeof(mtime));
		}

		bool IsDirectory() const { return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	};

	/// ZIP CRUD 封装。创建阶段连续 Add 走原生写入；已打开的包上 Add/Update/Remove 会重写整包。
	/// 后端：minizip-ng + zlib；加密为 WinZip AES（无 ZipCrypto）。
	class UILIB_API CZipFile
	{
	public:
		CZipFile();
		~CZipFile();

		/// 可在 Open/Create 之后调用；会同步到当前 reader/writer。空或 NULL = 无密码。
		/// 宽字符密码固定转为 UTF-8（不随系统代码页）。
		void SetPassword(LPCTSTR pszPassword);
		LPCTSTR GetPassword() const;

		/// 单条解压/重写时未压缩大小上限；默认 512MB。0 = 不限制（仅可信包）。
		void SetMaxUncompressedSize(DWORD dwBytes);
		DWORD GetMaxUncompressedSize() const;

		bool Create(LPCTSTR pszZipPath);
		/// nMaxBytes：可选软上限（mz 内存流 buffer_limit）；0 = 不限制。默认值仅作软上限提示。
		bool CreateMemory(unsigned int nMaxBytes = 32 * 1024 * 1024);
		bool Open(LPCTSTR pszZipPath);
		/// bCopy=true（默认）拷贝缓冲；false = 零拷贝挂接，缓冲须在 CZipFile 使用期间保持有效。
		bool OpenMemory(const void* pData, unsigned int nLen, bool bCopy = true);
		/// 等同 OpenMemory(pData, nLen, false)。
		bool AttachMemory(const void* pData, unsigned int nLen);
		/// 打开已加载模块里的 ZIP 资源（默认 RT_RCDATA）。内部拷贝一份。hInst=NULL 用资源 DLL / 本模块。
		bool OpenResource(HINSTANCE hInst, LPCTSTR pszName, LPCTSTR pszType = RT_RCDATA);
		bool OpenResource(HINSTANCE hInst, UINT nID, LPCTSTR pszType = RT_RCDATA);
		/// 打开磁盘上 EXE/DLL 文件里的 ZIP 资源（当数据加载，不执行其代码）。
		bool OpenResourceFile(LPCTSTR pszModulePath, LPCTSTR pszName, LPCTSTR pszType = RT_RCDATA);
		bool OpenResourceFile(LPCTSTR pszModulePath, UINT nID, LPCTSTR pszType = RT_RCDATA);
		void Close();
		/// 写入会话显式收尾（内存包提交到内部缓冲并切到只读；文件包关闭 writer 再打开）。
		bool Commit();
		bool IsOpen() const;
		bool IsMemory() const;
		/// 当前包是否含加密条目（读中央目录标志，不解压）。
		bool IsEncrypted();
		LPCTSTR GetPath() const;

		/// 压缩级别：-1=默认，0=仅存储(STORE)，1..9=deflate。Create/Open 后也可改，影响后续写入。
		void SetCompressLevel(int nLevel);
		int GetCompressLevel() const;

		int GetCount();
		bool GetItem(int nIndex, TZipItem& item);
		bool GetItem(LPCTSTR pszName, TZipItem& item);
		int Find(LPCTSTR pszName, bool bIgnoreCase = true);
		bool Exists(LPCTSTR pszName);
		bool IsDir(LPCTSTR pszName);

		bool ExtractFile(LPCTSTR pszName, LPCTSTR pszDestPath);
		bool ExtractDir(LPCTSTR pszDir, LPCTSTR pszDestDir);
		bool ExtractAll(LPCTSTR pszDestDir);
		/// 成功时 *ppData 为 new BYTE[]，调用方 delete[]；空文件 *pdwSize=0 且 *ppData 非空。
		bool ExtractMemory(LPCTSTR pszName, BYTE** ppData, DWORD* pdwSize);
		/// 解压校验全部条目（不落盘，校验 CRC/密码）。失败时 *pFailIndex 为出错条目下标（可 NULL）。
		bool Test(int* pFailIndex = NULL);

		/// pszZipName 为空则用源文件名。已存在且 bReplace=false 则失败。
		bool AddFile(LPCTSTR pszZipName, LPCTSTR pszSrcFile, bool bReplace = true);
		bool AddMemory(LPCTSTR pszZipName, const void* pData, DWORD dwSize, bool bReplace = true);
		bool AddFolder(LPCTSTR pszZipName);
		/// 把磁盘目录打进包。pszZipDir 空 = 包根。
		bool AddDir(LPCTSTR pszZipDir, LPCTSTR pszSrcDir, bool bRecursive = true);

		bool UpdateFile(LPCTSTR pszZipName, LPCTSTR pszSrcFile);
		bool UpdateMemory(LPCTSTR pszZipName, const void* pData, DWORD dwSize);

		/// 删单条。bRecursive：同时删掉 name/ 前缀的子条目。
		bool Remove(LPCTSTR pszName, bool bRecursive = false);
		/// 删目录：去掉目录条目，以及 dir/ 下全部子路径。dir 只是文件则失败。
		bool RemoveDir(LPCTSTR pszDir);

		bool SaveAs(LPCTSTR pszZipPath);
		/// 把当前 ZIP 写进磁盘上的 EXE/DLL 资源。文件不能被占用；会破坏该文件的数字签名。wLanguage=0 为中性语言。
		bool SaveResource(LPCTSTR pszModulePath, LPCTSTR pszName, LPCTSTR pszType = RT_RCDATA, WORD wLanguage = 0);
		bool SaveResource(LPCTSTR pszModulePath, UINT nID, LPCTSTR pszType = RT_RCDATA, WORD wLanguage = 0);
		/// 动态生成仅含资源的 DLL（无 DllMain），并把当前 ZIP 写入其中。覆盖已有文件。
		bool SaveResourceDll(LPCTSTR pszDllPath, LPCTSTR pszName, LPCTSTR pszType = RT_RCDATA, WORD wLanguage = 0);
		bool SaveResourceDll(LPCTSTR pszDllPath, UINT nID, LPCTSTR pszType = RT_RCDATA, WORD wLanguage = 0);
		/// 整包字节；*ppData 为 new BYTE[]，调用方 delete[]。
		bool GetMemory(BYTE** ppData, DWORD* pdwSize);
		/// 整包长度（不拷贝）。写入会话会先 Commit。失败返回 0。
		DWORD GetMemorySize();
		/// 交出内部整包所有权（须为本对象拥有的内存缓冲）。成功后对象关闭；*ppData 由调用方 delete[]。
		bool DetachMemory(BYTE** ppData, DWORD* pdwSize);

		/// minizip-ng 结果码（MZ_OK=0）。
		int GetLastResult() const;
		CDuiString GetErrorMessage() const;

		static bool UnzipToDirectory(LPCTSTR pszZipPath, LPCTSTR pszDestDir, LPCTSTR pszPassword = NULL);
		static bool UnzipMemoryToDirectory(const void* pData, unsigned int nLen, LPCTSTR pszDestDir, LPCTSTR pszPassword = NULL);

	private:
		CZipFile(const CZipFile&);
		CZipFile& operator=(const CZipFile&);

		enum EMode { ModeNone = 0, ModeWrite, ModeRead };
		enum EMutate { kAddFile, kAddMem, kAddDir, kUpdateFile, kUpdateMem, kRemove };

		bool Ok();
		bool Fail(int mzCode, LPCTSTR pszText = NULL);
		bool PreparePassword();
		/// 把当前密码应用到已打开的 reader/writer（Open 后改密码用）。
		bool SyncCryptoOpts();
		bool CheckMzBufSize(unsigned int nBytes, LPCTSTR pszWhat);
		/// 校验并接管自有内存包；失败时 delete[] pOwned，且不改动当前已打开状态。
		bool InstallOwnedMemory(BYTE* pOwned, DWORD nLen);
		const char* Pwd() const;
		void ClearWriteNames();
		int FindWriteName(LPCTSTR pszName) const;
		void RememberWriteName(LPCTSTR pszName);
		bool EnsureRead();
		bool CommitWrite();
		bool EnsureEntryName(CDuiString& name, bool bDir);
		bool CheckUncompressed(long nSize) const;
		bool AddInWriteMode(EMutate op, LPCTSTR pszZipName, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bReplace);
		bool Mutate(EMutate op, LPCTSTR pszZipName, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bReplace, bool bRecursive);
		bool Rewrite(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive);
		bool RewriteToFile(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive);
		bool RewriteToMemory(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive);
		int AddNewItem(void* writer, EMutate op, const CDuiString& name, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile);
		bool ReopenRead();
		bool ClassifyDir(LPCTSTR pszDir, bool& bDirEntry, bool& bChildren, bool& bFileExact);
		bool AddDirWalk(const CDuiString& zipDir, LPCTSTR pszSrcDir, bool bRecursive);
		bool ExtractOne(int nIndex, LPCTSTR pszDestPath, LPCTSTR pszContainRoot);
		/// 解压 reader 当前条目（不再 GotoIndex）。
		bool ExtractCurrent(LPCTSTR pszDestPath, LPCTSTR pszContainRoot);
		bool GotoIndex(int nIndex);
		bool FillItemFromCurrent(int nIndex, TZipItem& item);
		void InvalidateEntryCount();
		void ApplyWriterOpts(void* writer);
		void ApplyReaderOpts(void* reader);
		void DestroyReader();
		void DestroyWriter();
		void FreeMemBuf();

		void* m_reader;
		void* m_writer;
		void* m_memStream;
		EMode m_mode;
		CDuiString m_path;
		CDuiString m_password;
		CDuiString m_errText;
		char* m_pwdA;
		BYTE* m_memBuf;
		DWORD m_memLen;
		bool m_bMemOwned;
		unsigned int m_maxMem;
		DWORD m_maxUncompressed;
		int m_compressLevel;
		int m_cachedCount;
		int m_last;
		CStdPtrArray m_writeNames;
	};
}

#endif
