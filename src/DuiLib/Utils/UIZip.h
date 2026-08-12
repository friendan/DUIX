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

	enum ZipPasswordEncoding
	{
		ZIP_PWD_ASCII = 0,     // 仅 7-bit ASCII，含非 ASCII 则失败
		ZIP_PWD_UNICODE = 1    // LPCTSTR 自动转 UTF-8（与环境无关；需要兼容旧包时可指定 CP_ACP）
	};

	/// ZIP CRUD 封装。创建阶段连续 Add 走原生写入；已打开的包上 Add/Update/Remove 会重写整包。
	class UILIB_API CZipFile
	{
	public:
		CZipFile();
		~CZipFile();

		void SetPassword(LPCTSTR pszPassword);
		void SetPassword(LPCTSTR pszPassword, ZipPasswordEncoding encoding);
		LPCTSTR GetPassword() const;
		/// UNICODE 时 uCodePage：0 = CP_UTF8。一般不必改；仅打开旧 ANSI 包时传 CP_ACP。ASCII 模式忽略代码页。
		void SetPasswordEncoding(ZipPasswordEncoding encoding, UINT uCodePage = 0);
		ZipPasswordEncoding GetPasswordEncoding() const;
		UINT GetPasswordCodePage() const;

		/// 单条解压/重写时未压缩大小上限；默认 512MB。0 = 不限制（仅可信包）。
		void SetMaxUncompressedSize(DWORD dwBytes);
		DWORD GetMaxUncompressedSize() const;

		bool Create(LPCTSTR pszZipPath);
		bool CreateMemory(unsigned int nMaxBytes = 32 * 1024 * 1024);
		bool Open(LPCTSTR pszZipPath);
		bool OpenMemory(const void* pData, unsigned int nLen);
		/// 打开已加载模块里的 ZIP 资源（默认 RT_RCDATA）。内部拷贝一份。hInst=NULL 用资源 DLL / 本模块。
		bool OpenResource(HINSTANCE hInst, LPCTSTR pszName, LPCTSTR pszType = RT_RCDATA);
		bool OpenResource(HINSTANCE hInst, UINT nID, LPCTSTR pszType = RT_RCDATA);
		/// 打开磁盘上 EXE/DLL 文件里的 ZIP 资源（当数据加载，不执行其代码）。
		bool OpenResourceFile(LPCTSTR pszModulePath, LPCTSTR pszName, LPCTSTR pszType = RT_RCDATA);
		bool OpenResourceFile(LPCTSTR pszModulePath, UINT nID, LPCTSTR pszType = RT_RCDATA);
		void Close();
		bool IsOpen() const;
		bool IsMemory() const;
		LPCTSTR GetPath() const;

		int GetCount();
		bool GetItem(int nIndex, TZipItem& item);
		int Find(LPCTSTR pszName, bool bIgnoreCase = true);
		bool Exists(LPCTSTR pszName);
		bool IsDir(LPCTSTR pszName);

		bool ExtractFile(LPCTSTR pszName, LPCTSTR pszDestPath);
		bool ExtractDir(LPCTSTR pszDir, LPCTSTR pszDestDir);
		bool ExtractAll(LPCTSTR pszDestDir);
		/// 成功时 *ppData 为 new BYTE[]，调用方 delete[]；空文件 *pdwSize=0 且 *ppData 非空。
		bool ExtractMemory(LPCTSTR pszName, BYTE** ppData, DWORD* pdwSize);

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

		ZRESULT GetLastResult() const;
		CDuiString GetErrorMessage() const;

		static bool UnzipToDirectory(LPCTSTR pszZipPath, LPCTSTR pszDestDir, LPCTSTR pszPassword = NULL, ZipPasswordEncoding encoding = ZIP_PWD_UNICODE);

	private:
		CZipFile(const CZipFile&);
		CZipFile& operator=(const CZipFile&);

		enum EMode { ModeNone = 0, ModeWrite, ModeRead };
		enum EMutate { kAddFile, kAddMem, kAddDir, kUpdateFile, kUpdateMem, kRemove };

		bool Ok();
		bool Fail(ZRESULT r, LPCTSTR pszText = NULL);
		bool PreparePassword();
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
		ZRESULT CopyItem(HZIP hzSrc, HZIP hzDst, int nIndex);
		ZRESULT AddNewItem(HZIP hzDst, EMutate op, const CDuiString& name, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile);
		unsigned int EstimateMemCap(DWORD dwExtra) const;
		bool ReopenRead();
		bool ClassifyDir(LPCTSTR pszDir, bool& bDirEntry, bool& bChildren, bool& bFileExact);
		bool AddDirWalk(const CDuiString& zipDir, LPCTSTR pszSrcDir, bool bRecursive);
		bool ExtractOne(int nIndex, const ZIPENTRY& ze, LPCTSTR pszDestPath, LPCTSTR pszContainRoot);

		HZIP m_hz;
		EMode m_mode;
		CDuiString m_path;
		CDuiString m_password;
		CDuiString m_errText;
		char* m_pwdA;
		ZipPasswordEncoding m_pwdEncoding;
		UINT m_pwdCodePage;
		BYTE* m_memBuf;
		DWORD m_memLen;
		unsigned int m_maxMem;
		DWORD m_maxUncompressed;
		ZRESULT m_last;
		CStdPtrArray m_writeNames;
	};
}

#endif
