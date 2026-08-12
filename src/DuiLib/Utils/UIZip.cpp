#include "StdAfx.h"
#include "UIZip.h"

namespace DuiLib
{
	static const DWORD kDefaultMaxUncompressed = 512u * 1024u * 1024u;

	static CDuiString NormalizeZipName(LPCTSTR pszName)
	{
		CDuiString s(pszName);
		s.Replace(_T("\\"), _T("/"));
		while( s.GetLength() > 0 && s.GetAt(0) == _T('/') )
			s = s.Mid(1);
		CDuiString out;
		for( int i = 0; i < s.GetLength(); ++i ) {
			TCHAR c = s.GetAt(i);
			if( c == _T('/') ) {
				int n = out.GetLength();
				if( n == 0 ) continue;
				if( out.GetAt(n - 1) == _T('/') ) continue;
			}
			out += c;
		}
		return out;
	}

	static CDuiString NormalizeDirName(LPCTSTR pszDir)
	{
		CDuiString s = NormalizeZipName(pszDir);
		while( s.GetLength() > 0 && s.GetAt(s.GetLength() - 1) == _T('/') )
			s = s.Left(s.GetLength() - 1);
		return s;
	}

	static CDuiString FileNameOf(LPCTSTR pszPath)
	{
		CDuiString s(pszPath);
		int a = s.ReverseFind(_T('\\'));
		int b = s.ReverseFind(_T('/'));
		int p = (a > b) ? a : b;
		if( p >= 0 ) return s.Mid(p + 1);
		return s;
	}

	static bool NameMatch(const CDuiString& item, const CDuiString& target, bool bRecursive)
	{
		CDuiString left = NormalizeDirName(item.GetData());
		CDuiString right = NormalizeDirName(target.GetData());
		if( left.CompareNoCase(right.GetData()) == 0 ) return true;
		if( !bRecursive ) return false;
		CDuiString prefix = right;
		prefix += _T('/');
		CDuiString raw = NormalizeZipName(item.GetData());
		if( raw.GetLength() < prefix.GetLength() ) return false;
		CDuiString head = raw.Left(prefix.GetLength());
		return head.CompareNoCase(prefix.GetData()) == 0;
	}

	static bool IsWinReservedSegment(LPCTSTR psz, int nLen)
	{
		if( psz == NULL || nLen <= 0 ) return false;
		TCHAR buf[16];
		int n = nLen;
		int dot = nLen;
		for( int i = 0; i < nLen; ++i ) {
			if( psz[i] == _T('.') ) { dot = i; break; }
		}
		n = dot;
		while( n > 0 && (psz[n - 1] == _T(' ') || psz[n - 1] == _T('.')) ) --n;
		if( n <= 0 || n >= (int)(sizeof(buf) / sizeof(buf[0])) ) return false;
		for( int i = 0; i < n; ++i ) {
			TCHAR c = psz[i];
			if( c >= _T('a') && c <= _T('z') ) c = (TCHAR)(c - _T('a') + _T('A'));
			buf[i] = c;
		}
		buf[n] = _T('\0');
		static const TCHAR* kNames[] = {
			_T("CON"), _T("PRN"), _T("AUX"), _T("NUL"), _T("CLOCK$"),
			_T("COM1"), _T("COM2"), _T("COM3"), _T("COM4"), _T("COM5"),
			_T("COM6"), _T("COM7"), _T("COM8"), _T("COM9"),
			_T("LPT1"), _T("LPT2"), _T("LPT3"), _T("LPT4"), _T("LPT5"),
			_T("LPT6"), _T("LPT7"), _T("LPT8"), _T("LPT9"), NULL
		};
		for( int i = 0; kNames[i]; ++i ) {
			if( _tcscmp(buf, kNames[i]) == 0 ) return true;
		}
		return false;
	}

	static bool IsSafeZipName(LPCTSTR pszName)
	{
		if( pszName == NULL || *pszName == _T('\0') ) return false;
		int nLen = (int)_tcslen(pszName);
		if( nLen <= 0 || nLen >= MAX_PATH ) return false;
		if( pszName[0] == _T('/') || pszName[0] == _T('\\') ) return false;

		int segStart = 0;
		for( int i = 0; i <= nLen; ++i ) {
			TCHAR c = (i < nLen) ? pszName[i] : _T('/');
			if( c == _T('/') || c == _T('\\') ) {
				int segLen = i - segStart;
				if( segLen <= 0 ) return false;
				if( segLen == 1 && pszName[segStart] == _T('.') ) return false;
				if( segLen == 2 && pszName[segStart] == _T('.') && pszName[segStart + 1] == _T('.') )
					return false;
				if( IsWinReservedSegment(pszName + segStart, segLen) ) return false;
				segStart = i + 1;
				continue;
			}
			if( i >= nLen ) break;
			if( (unsigned)c < 32 ) return false;
			if( c == _T(':') || c == _T('*') || c == _T('?') || c == _T('"') ||
				c == _T('<') || c == _T('>') || c == _T('|') )
				return false;
		}
		return true;
	}

	static bool EnsureDirectory(LPCTSTR pszDir)
	{
		if( pszDir == NULL || *pszDir == _T('\0') ) return false;
		DWORD attr = ::GetFileAttributes(pszDir);
		if( attr != INVALID_FILE_ATTRIBUTES )
			return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;

		CDuiString s(pszDir);
		int n = s.GetLength();
		while( n > 0 && (s.GetAt(n - 1) == _T('\\') || s.GetAt(n - 1) == _T('/')) ) {
			s = s.Left(n - 1);
			n = s.GetLength();
		}
		if( s.IsEmpty() ) return true;
		if( n == 2 && s.GetAt(1) == _T(':') ) return true;

		int slash = -1;
		for( int i = n - 1; i >= 0; --i ) {
			TCHAR c = s.GetAt(i);
			if( c == _T('\\') || c == _T('/') ) { slash = i; break; }
		}
		if( slash > 0 ) {
			CDuiString parent = s.Left(slash);
			if( !EnsureDirectory(parent.GetData()) ) return false;
		}
		if( ::CreateDirectory(s.GetData(), NULL) ) return true;
		return ::GetLastError() == ERROR_ALREADY_EXISTS;
	}

	static bool EnsureParentDir(LPCTSTR pszFile)
	{
		CDuiString s(pszFile);
		int a = s.ReverseFind(_T('\\'));
		int b = s.ReverseFind(_T('/'));
		int p = (a > b) ? a : b;
		if( p <= 0 ) return true;
		return EnsureDirectory(s.Left(p).GetData());
	}

	static CDuiString JoinPath(LPCTSTR pszDir, LPCTSTR pszRel)
	{
		CDuiString s(pszDir);
		int n = s.GetLength();
		if( n > 0 ) {
			TCHAR c = s.GetAt(n - 1);
			if( c != _T('\\') && c != _T('/') ) s += _T('\\');
		}
		CDuiString r(pszRel);
		r.Replace(_T("/"), _T("\\"));
		s += r;
		return s;
	}

	static bool GetFullPathStr(LPCTSTR pszIn, CDuiString& out)
	{
		TCHAR buf[MAX_PATH];
		DWORD n = ::GetFullPathName(pszIn, MAX_PATH, buf, NULL);
		if( n == 0 || n >= MAX_PATH ) return false;
		out = buf;
		return true;
	}

	static bool IsPathContained(LPCTSTR pszRoot, LPCTSTR pszPath)
	{
		CDuiString root, path;
		if( !GetFullPathStr(pszRoot, root) || !GetFullPathStr(pszPath, path) )
			return false;
		int n = root.GetLength();
		if( n <= 0 ) return false;
		TCHAR c = root.GetAt(n - 1);
		if( c != _T('\\') && c != _T('/') ) root += _T('\\');
		if( path.GetLength() < root.GetLength() ) return false;
		CDuiString head = path.Left(root.GetLength());
		return head.CompareNoCase(root.GetData()) == 0;
	}

	static bool MakeTempZipPath(LPCTSTR pszZipPath, CDuiString& out)
	{
		CDuiString dir(pszZipPath);
		int a = dir.ReverseFind(_T('\\'));
		int b = dir.ReverseFind(_T('/'));
		int p = (a > b) ? a : b;
		CDuiString folder = (p >= 0) ? dir.Left(p) : CDuiString(_T("."));
		TCHAR buf[MAX_PATH];
		if( ::GetTempFileName(folder.GetData(), _T("dz"), 0, buf) == 0 )
			return false;
		out = buf;
		return true;
	}

	static bool ReplaceZipAtomically(LPCTSTR pszDst, LPCTSTR pszSrc)
	{
		if( ::ReplaceFile(pszDst, pszSrc, NULL, 0, NULL, NULL) )
			return true;
		if( ::CopyFile(pszSrc, pszDst, FALSE) ) {
			::DeleteFile(pszSrc);
			return true;
		}
		return false;
	}

	static bool ReadWholeFile(LPCTSTR pszPath, BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData ) *ppData = NULL;
		if( pdwSize ) *pdwSize = 0;
		HANDLE h = ::CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( h == INVALID_HANDLE_VALUE ) return false;
		DWORD dwSize = ::GetFileSize(h, NULL);
		if( dwSize == INVALID_FILE_SIZE ) {
			::CloseHandle(h);
			return false;
		}
		BYTE* p = NULL;
		if( dwSize > 0 ) {
			p = new BYTE[dwSize];
			DWORD dwRead = 0;
			BOOL ok = ::ReadFile(h, p, dwSize, &dwRead, NULL);
			::CloseHandle(h);
			if( !ok || dwRead != dwSize ) {
				delete[] p;
				return false;
			}
		}
		else {
			::CloseHandle(h);
			p = new BYTE[1];
		}
		*ppData = p;
		*pdwSize = dwSize;
		return true;
	}

	static bool WriteWholeFile(LPCTSTR pszPath, const void* pData, DWORD dwSize)
	{
		EnsureParentDir(pszPath);
		HANDLE h = ::CreateFile(pszPath, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if( h == INVALID_HANDLE_VALUE ) return false;
		DWORD dwWrit = 0;
		BOOL ok = TRUE;
		if( dwSize > 0 )
			ok = ::WriteFile(h, pData, dwSize, &dwWrit, NULL);
		::CloseHandle(h);
		return ok && dwWrit == dwSize;
	}

	CZipFile::CZipFile()
		: m_hz(NULL)
		, m_mode(ModeNone)
		, m_pwdA(NULL)
		, m_pwdEncoding(ZIP_PWD_UNICODE)
		, m_pwdCodePage(CP_UTF8)
		, m_memBuf(NULL)
		, m_memLen(0)
		, m_maxMem(0)
		, m_maxUncompressed(kDefaultMaxUncompressed)
		, m_last(ZR_OK)
	{
	}

	CZipFile::~CZipFile()
	{
		Close();
	}

	void CZipFile::SetPassword(LPCTSTR pszPassword)
	{
		m_password = pszPassword;
	}

	void CZipFile::SetPassword(LPCTSTR pszPassword, ZipPasswordEncoding encoding)
	{
		m_password = pszPassword;
		SetPasswordEncoding(encoding, 0);
	}

	LPCTSTR CZipFile::GetPassword() const
	{
		return m_password.GetData();
	}

	void CZipFile::SetPasswordEncoding(ZipPasswordEncoding encoding, UINT uCodePage)
	{
		m_pwdEncoding = encoding;
		if( encoding == ZIP_PWD_ASCII )
			m_pwdCodePage = 0;
		else
			m_pwdCodePage = (uCodePage == 0) ? CP_UTF8 : uCodePage;
	}

	ZipPasswordEncoding CZipFile::GetPasswordEncoding() const
	{
		return m_pwdEncoding;
	}

	UINT CZipFile::GetPasswordCodePage() const
	{
		return m_pwdCodePage;
	}

	void CZipFile::SetMaxUncompressedSize(DWORD dwBytes)
	{
		m_maxUncompressed = dwBytes;
	}

	DWORD CZipFile::GetMaxUncompressedSize() const
	{
		return m_maxUncompressed;
	}

	bool CZipFile::Ok()
	{
		m_last = ZR_OK;
		m_errText.Empty();
		return true;
	}

	bool CZipFile::Fail(ZRESULT r, LPCTSTR pszText)
	{
		m_last = r;
		m_errText = pszText;
		return false;
	}

	bool CZipFile::PreparePassword()
	{
		delete[] m_pwdA;
		m_pwdA = NULL;
		if( m_password.IsEmpty() ) return true;

		LPCTSTR psz = m_password.GetData();
		if( m_pwdEncoding == ZIP_PWD_ASCII ) {
			int n = m_password.GetLength();
			for( int i = 0; i < n; ++i ) {
				if( (unsigned)psz[i] > 127 )
					return Fail(ZR_ARGS, _T("password must be ASCII"));
			}
			m_pwdA = new char[n + 1];
			for( int i = 0; i < n; ++i )
				m_pwdA[i] = (char)psz[i];
			m_pwdA[n] = '\0';
			return true;
		}

		UINT cp = (m_pwdCodePage == 0) ? CP_UTF8 : m_pwdCodePage;
		DWORD dwFlags = 0;
		BOOL bUsedDefault = FALSE;
		BOOL* pUsedDefault = NULL;
		if( cp == CP_UTF8 )
			dwFlags = WC_ERR_INVALID_CHARS;
		else if( cp != 65000 && cp != 65001 )
			dwFlags = WC_NO_BEST_FIT_CHARS;

		int nNeed = ::WideCharToMultiByte(cp, dwFlags, psz, -1, NULL, 0, NULL, NULL);
		if( nNeed <= 1 )
			return Fail(ZR_ARGS, _T("password encoding conversion failed"));

		m_pwdA = new char[nNeed];
		if( dwFlags == WC_NO_BEST_FIT_CHARS )
			pUsedDefault = &bUsedDefault;
		int nGot = ::WideCharToMultiByte(cp, dwFlags, psz, -1, m_pwdA, nNeed, NULL, pUsedDefault);
		if( nGot <= 1 || bUsedDefault ) {
			delete[] m_pwdA;
			m_pwdA = NULL;
			return Fail(ZR_ARGS, _T("password encoding conversion failed"));
		}
		return true;
	}

	const char* CZipFile::Pwd() const
	{
		return m_pwdA;
	}

	void CZipFile::ClearWriteNames()
	{
		for( int i = 0; i < m_writeNames.GetSize(); ++i ) {
			CDuiString* p = (CDuiString*)m_writeNames.GetAt(i);
			delete p;
		}
		m_writeNames.Empty();
	}

	int CZipFile::FindWriteName(LPCTSTR pszName) const
	{
		CDuiString n = NormalizeZipName(pszName);
		for( int i = 0; i < m_writeNames.GetSize(); ++i ) {
			CDuiString* p = (CDuiString*)m_writeNames.GetAt(i);
			if( p && p->CompareNoCase(n.GetData()) == 0 ) return i;
		}
		return -1;
	}

	void CZipFile::RememberWriteName(LPCTSTR pszName)
	{
		m_writeNames.Add(new CDuiString(NormalizeZipName(pszName)));
	}

	bool CZipFile::EnsureEntryName(CDuiString& name, bool bDir)
	{
		name = bDir ? NormalizeDirName(name.GetData()) : NormalizeZipName(name.GetData());
		if( name.IsEmpty() ) return Fail(ZR_ARGS, _T("entry name is empty"));
		if( !IsSafeZipName(name.GetData()) )
			return Fail(ZR_ARGS, _T("unsafe zip entry path"));
		return true;
	}

	bool CZipFile::CheckUncompressed(long nSize) const
	{
		if( nSize < 0 ) return false;
		if( m_maxUncompressed == 0 ) return true;
		return (DWORD)nSize <= m_maxUncompressed;
	}

	void CZipFile::Close()
	{
		if( m_hz ) {
			CloseZip(m_hz);
			m_hz = NULL;
		}
		delete[] m_memBuf;
		m_memBuf = NULL;
		m_memLen = 0;
		m_maxMem = 0;
		m_path.Empty();
		m_mode = ModeNone;
		ClearWriteNames();
		delete[] m_pwdA;
		m_pwdA = NULL;
	}

	bool CZipFile::IsOpen() const
	{
		return m_hz != NULL;
	}

	bool CZipFile::IsMemory() const
	{
		return m_path.IsEmpty() && (m_memBuf != NULL || (m_mode == ModeWrite && m_hz != NULL));
	}

	LPCTSTR CZipFile::GetPath() const
	{
		return m_path.GetData();
	}

	bool CZipFile::Create(LPCTSTR pszZipPath)
	{
		if( pszZipPath == NULL || *pszZipPath == _T('\0') )
			return Fail(ZR_ARGS, _T("zip path is empty"));
		Close();
		m_path = pszZipPath;
		if( !PreparePassword() ) {
			m_path.Empty();
			return false;
		}
		m_hz = CreateZip(m_path.GetData(), Pwd());
		if( m_hz == NULL ) {
			m_path.Empty();
			return Fail(ZR_RECENT, _T("create zip failed"));
		}
		m_mode = ModeWrite;
		return Ok();
	}

	bool CZipFile::CreateMemory(unsigned int nMaxBytes)
	{
		if( nMaxBytes < 1024 ) nMaxBytes = 1024;
		Close();
		m_maxMem = nMaxBytes;
		if( !PreparePassword() ) return false;
		m_hz = CreateZip(0, nMaxBytes, Pwd());
		if( m_hz == NULL )
			return Fail(ZR_RECENT, _T("create memory zip failed"));
		m_mode = ModeWrite;
		return Ok();
	}

	bool CZipFile::Open(LPCTSTR pszZipPath)
	{
		if( pszZipPath == NULL || *pszZipPath == _T('\0') )
			return Fail(ZR_ARGS, _T("zip path is empty"));
		CDuiString path(pszZipPath);
		Close();
		m_path = path;
		if( !PreparePassword() ) {
			m_path.Empty();
			return false;
		}
		m_hz = OpenZip(m_path.GetData(), Pwd());
		if( m_hz == NULL ) {
			m_path.Empty();
			return Fail(ZR_RECENT, _T("open zip failed"));
		}
		m_mode = ModeRead;
		return Ok();
	}

	bool CZipFile::OpenMemory(const void* pData, unsigned int nLen)
	{
		if( pData == NULL || nLen == 0 )
			return Fail(ZR_ARGS, _T("memory zip is empty"));
		BYTE* pCopy = new BYTE[nLen];
		memcpy(pCopy, pData, nLen);
		Close();
		if( !PreparePassword() ) {
			delete[] pCopy;
			return false;
		}
		m_memBuf = pCopy;
		m_memLen = nLen;
		m_hz = OpenZip(m_memBuf, m_memLen, Pwd());
		if( m_hz == NULL ) {
			delete[] m_memBuf;
			m_memBuf = NULL;
			m_memLen = 0;
			return Fail(ZR_RECENT, _T("open memory zip failed"));
		}
		m_mode = ModeRead;
		return Ok();
	}

	bool CZipFile::OpenResource(HINSTANCE hInst, LPCTSTR pszName, LPCTSTR pszType)
	{
		if( pszName == NULL )
			return Fail(ZR_ARGS, _T("resource name is empty"));
		if( pszType == NULL ) pszType = RT_RCDATA;
		if( hInst == NULL ) hInst = CPaintManagerUI::GetResourceDll();
		if( hInst == NULL ) hInst = CPaintManagerUI::GetInstance();
		if( hInst == NULL )
			return Fail(ZR_NOFILE, _T("resource module is null"));

		HRSRC hRes = ::FindResource(hInst, pszName, pszType);
		if( hRes == NULL )
			return Fail(ZR_NOFILE, _T("resource not found"));
		DWORD nLen = ::SizeofResource(hInst, hRes);
		HGLOBAL hGlob = ::LoadResource(hInst, hRes);
		if( hGlob == NULL || nLen == 0 )
			return Fail(ZR_NOFILE, _T("load resource failed"));
		const void* pData = ::LockResource(hGlob);
		if( pData == NULL )
			return Fail(ZR_NOFILE, _T("lock resource failed"));
		return OpenMemory(pData, nLen);
	}

	bool CZipFile::OpenResource(HINSTANCE hInst, UINT nID, LPCTSTR pszType)
	{
		return OpenResource(hInst, MAKEINTRESOURCE(nID), pszType);
	}

	static HMODULE LoadPeAsDataFile(LPCTSTR pszModulePath)
	{
		if( pszModulePath == NULL || *pszModulePath == _T('\0') ) return NULL;
		HMODULE h = ::LoadLibraryEx(pszModulePath, NULL,
			LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
		if( h != NULL ) return h;
		return ::LoadLibraryEx(pszModulePath, NULL, LOAD_LIBRARY_AS_DATAFILE);
	}

	bool CZipFile::OpenResourceFile(LPCTSTR pszModulePath, LPCTSTR pszName, LPCTSTR pszType)
	{
		if( pszModulePath == NULL || *pszModulePath == _T('\0') )
			return Fail(ZR_ARGS, _T("module path is empty"));
		HMODULE hMod = LoadPeAsDataFile(pszModulePath);
		if( hMod == NULL )
			return Fail(ZR_NOFILE, _T("load module failed"));
		bool bOk = OpenResource((HINSTANCE)hMod, pszName, pszType);
		::FreeLibrary(hMod);
		return bOk;
	}

	bool CZipFile::OpenResourceFile(LPCTSTR pszModulePath, UINT nID, LPCTSTR pszType)
	{
		return OpenResourceFile(pszModulePath, MAKEINTRESOURCE(nID), pszType);
	}

	bool CZipFile::SaveResource(LPCTSTR pszModulePath, LPCTSTR pszName, LPCTSTR pszType, WORD wLanguage)
	{
		if( pszModulePath == NULL || *pszModulePath == _T('\0') )
			return Fail(ZR_ARGS, _T("module path is empty"));
		if( pszName == NULL )
			return Fail(ZR_ARGS, _T("resource name is empty"));
		if( pszType == NULL ) pszType = RT_RCDATA;

		BYTE* pData = NULL;
		DWORD dwSize = 0;
		if( !GetMemory(&pData, &dwSize) ) return false;
		if( pData == NULL || dwSize == 0 ) {
			delete[] pData;
			return Fail(ZR_ARGS, _T("zip is empty"));
		}

		HANDLE hUpdate = ::BeginUpdateResource(pszModulePath, FALSE);
		if( hUpdate == NULL ) {
			delete[] pData;
			return Fail(ZR_NOFILE, _T("begin update resource failed"));
		}
		BOOL bOk = ::UpdateResource(hUpdate, pszType, pszName, wLanguage, pData, dwSize);
		if( !bOk ) {
			::EndUpdateResource(hUpdate, TRUE);
			delete[] pData;
			return Fail(ZR_WRITE, _T("update resource failed"));
		}
		bOk = ::EndUpdateResource(hUpdate, FALSE);
		delete[] pData;
		if( !bOk )
			return Fail(ZR_WRITE, _T("commit resource failed"));
		return Ok();
	}

	bool CZipFile::SaveResource(LPCTSTR pszModulePath, UINT nID, LPCTSTR pszType, WORD wLanguage)
	{
		return SaveResource(pszModulePath, MAKEINTRESOURCE(nID), pszType, wLanguage);
	}

	static DWORD AlignUpDw(DWORD v, DWORD a)
	{
		if( a <= 1 ) return v;
		return (v + a - 1) & ~(a - 1);
	}

	static bool WriteResourceOnlyDllStub(LPCTSTR pszPath)
	{
		const DWORD kFileAlign = 0x200;
		const DWORD kSectAlign = 0x1000;

#ifdef _WIN64
		IMAGE_NT_HEADERS64 nt;
		ZeroMemory(&nt, sizeof(nt));
		nt.FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
		nt.FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
		nt.FileHeader.Characteristics = IMAGE_FILE_DLL | IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE;
		nt.OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
		nt.OptionalHeader.ImageBase = 0x180000000ULL;
		nt.OptionalHeader.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE
			| IMAGE_DLLCHARACTERISTICS_NX_COMPAT
			| IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA;
		nt.OptionalHeader.SizeOfStackReserve = 0x100000;
		nt.OptionalHeader.SizeOfStackCommit = 0x1000;
		nt.OptionalHeader.SizeOfHeapReserve = 0x100000;
		nt.OptionalHeader.SizeOfHeapCommit = 0x1000;
#else
		IMAGE_NT_HEADERS32 nt;
		ZeroMemory(&nt, sizeof(nt));
		nt.FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
		nt.FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
		nt.FileHeader.Characteristics = IMAGE_FILE_DLL | IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE;
		nt.OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
		nt.OptionalHeader.ImageBase = 0x10000000;
		nt.OptionalHeader.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE
			| IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
		nt.OptionalHeader.SizeOfStackReserve = 0x100000;
		nt.OptionalHeader.SizeOfStackCommit = 0x1000;
		nt.OptionalHeader.SizeOfHeapReserve = 0x100000;
		nt.OptionalHeader.SizeOfHeapCommit = 0x1000;
#endif
		nt.Signature = IMAGE_NT_SIGNATURE;
		nt.FileHeader.NumberOfSections = 1;
		nt.OptionalHeader.MajorLinkerVersion = 14;
		nt.OptionalHeader.AddressOfEntryPoint = 0;
		nt.OptionalHeader.SectionAlignment = kSectAlign;
		nt.OptionalHeader.FileAlignment = kFileAlign;
		nt.OptionalHeader.MajorOperatingSystemVersion = 6;
		nt.OptionalHeader.MinorOperatingSystemVersion = 0;
		nt.OptionalHeader.MajorSubsystemVersion = 6;
		nt.OptionalHeader.MinorSubsystemVersion = 0;
		nt.OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
		nt.OptionalHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

		IMAGE_DOS_HEADER dos;
		ZeroMemory(&dos, sizeof(dos));
		dos.e_magic = IMAGE_DOS_SIGNATURE;
		dos.e_cblp = 0x90;
		dos.e_cp = 3;
		dos.e_cparhdr = 4;
		dos.e_maxalloc = 0xFFFF;
		dos.e_sp = 0xB8;
		dos.e_lfarlc = 0x40;
		dos.e_lfanew = 0x80;

		const DWORD ntOff = (DWORD)dos.e_lfanew;
		const DWORD sectOff = ntOff + (DWORD)sizeof(nt);
		const DWORD sizeOfHeaders = AlignUpDw(sectOff + (DWORD)sizeof(IMAGE_SECTION_HEADER), kFileAlign);
		const DWORD rsrcRva = kSectAlign;
		const DWORD rsrcRaw = sizeOfHeaders;
		const DWORD rsrcRawSize = kFileAlign;
		const DWORD sizeOfImage = rsrcRva + kSectAlign;

		nt.OptionalHeader.SizeOfHeaders = sizeOfHeaders;
		nt.OptionalHeader.SizeOfImage = sizeOfImage;
		nt.OptionalHeader.SizeOfInitializedData = rsrcRawSize;
		nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = rsrcRva;
		nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = (DWORD)sizeof(IMAGE_RESOURCE_DIRECTORY);

		IMAGE_SECTION_HEADER sec;
		ZeroMemory(&sec, sizeof(sec));
		memcpy(sec.Name, ".rsrc", 5);
		sec.Misc.VirtualSize = (DWORD)sizeof(IMAGE_RESOURCE_DIRECTORY);
		sec.VirtualAddress = rsrcRva;
		sec.SizeOfRawData = rsrcRawSize;
		sec.PointerToRawData = rsrcRaw;
		sec.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;

		IMAGE_RESOURCE_DIRECTORY emptyRsrc;
		ZeroMemory(&emptyRsrc, sizeof(emptyRsrc));

		const DWORD cbTotal = sizeOfHeaders + rsrcRawSize;
		BYTE* pBuf = new BYTE[cbTotal];
		ZeroMemory(pBuf, cbTotal);
		memcpy(pBuf, &dos, sizeof(dos));
		memcpy(pBuf + ntOff, &nt, sizeof(nt));
		memcpy(pBuf + sectOff, &sec, sizeof(sec));
		memcpy(pBuf + rsrcRaw, &emptyRsrc, sizeof(emptyRsrc));
		bool bOk = WriteWholeFile(pszPath, pBuf, cbTotal);
		delete[] pBuf;
		return bOk;
	}

	bool CZipFile::SaveResourceDll(LPCTSTR pszDllPath, LPCTSTR pszName, LPCTSTR pszType, WORD wLanguage)
	{
		if( pszDllPath == NULL || *pszDllPath == _T('\0') )
			return Fail(ZR_ARGS, _T("dll path is empty"));
		if( !EnsureRead() ) return false;
		if( !WriteResourceOnlyDllStub(pszDllPath) )
			return Fail(ZR_NOFILE, _T("create resource dll failed"));
		if( !SaveResource(pszDllPath, pszName, pszType, wLanguage) ) {
			::DeleteFile(pszDllPath);
			return false;
		}
		return Ok();
	}

	bool CZipFile::SaveResourceDll(LPCTSTR pszDllPath, UINT nID, LPCTSTR pszType, WORD wLanguage)
	{
		return SaveResourceDll(pszDllPath, MAKEINTRESOURCE(nID), pszType, wLanguage);
	}

	bool CZipFile::ReopenRead()
	{
		if( m_hz ) {
			CloseZip(m_hz);
			m_hz = NULL;
		}
		ClearWriteNames();
		m_mode = ModeNone;
		if( !PreparePassword() ) return false;
		if( !m_path.IsEmpty() ) {
			m_hz = OpenZip(m_path.GetData(), Pwd());
			if( m_hz == NULL ) return Fail(ZR_RECENT, _T("reopen zip failed"));
			m_mode = ModeRead;
			return Ok();
		}
		if( m_memBuf == NULL || m_memLen == 0 )
			return Fail(ZR_ARGS, _T("zip not open"));
		m_hz = OpenZip(m_memBuf, m_memLen, Pwd());
		if( m_hz == NULL ) return Fail(ZR_RECENT, _T("reopen memory zip failed"));
		m_mode = ModeRead;
		return Ok();
	}

	bool CZipFile::CommitWrite()
	{
		if( m_mode != ModeWrite ) return m_hz != NULL && m_mode == ModeRead;
		if( m_hz == NULL ) return Fail(ZR_ARGS, _T("zip not open"));

		if( !m_path.IsEmpty() ) {
			CloseZip(m_hz);
			m_hz = NULL;
			return ReopenRead();
		}

		void* p = NULL;
		unsigned long n = 0;
		ZRESULT r = ZipGetMemory(m_hz, &p, &n);
		if( r != ZR_OK ) return Fail(r, _T("ZipGetMemory failed"));
		BYTE* pCopy = NULL;
		if( n > 0 ) {
			pCopy = new BYTE[n];
			memcpy(pCopy, p, n);
		}
		CloseZip(m_hz);
		m_hz = NULL;
		delete[] m_memBuf;
		m_memBuf = pCopy;
		m_memLen = n;
		m_maxMem = 0;
		if( m_memBuf == NULL || m_memLen == 0 )
			return Fail(ZR_CORRUPT, _T("empty memory zip"));
		return ReopenRead();
	}

	bool CZipFile::EnsureRead()
	{
		if( m_mode == ModeRead && m_hz != NULL ) return true;
		if( m_mode == ModeWrite ) return CommitWrite();
		return Fail(ZR_ARGS, _T("zip not open"));
	}

	int CZipFile::GetCount()
	{
		if( !EnsureRead() ) return 0;
		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		m_last = GetZipItem(m_hz, -1, &ze);
		if( m_last != ZR_OK ) return 0;
		m_errText.Empty();
		return ze.index;
	}

	bool CZipFile::GetItem(int nIndex, TZipItem& item)
	{
		item = TZipItem();
		if( !EnsureRead() ) return false;
		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		m_last = GetZipItem(m_hz, nIndex, &ze);
		if( m_last != ZR_OK ) return false;
		item.index = ze.index;
		item.name = NormalizeZipName(ze.name);
		item.attr = ze.attr;
		item.mtime = ze.mtime;
		item.comp_size = ze.comp_size;
		item.unc_size = ze.unc_size;
		m_errText.Empty();
		return true;
	}

	int CZipFile::Find(LPCTSTR pszName, bool bIgnoreCase)
	{
		if( !EnsureRead() ) return -1;
		CDuiString name = NormalizeZipName(pszName);
		if( name.IsEmpty() ) {
			Fail(ZR_ARGS, _T("entry name is empty"));
			return -1;
		}
		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		int idx = -1;
		m_last = FindZipItem(m_hz, name.GetData(), bIgnoreCase, &idx, &ze);
		if( m_last == ZR_OK && idx >= 0 ) {
			m_errText.Empty();
			return idx;
		}

		memset(&ze, 0, sizeof(ze));
		if( GetZipItem(m_hz, -1, &ze) != ZR_OK ) return -1;
		int n = ze.index;
		CDuiString nameDir = NormalizeDirName(name.GetData());
		for( int i = 0; i < n; ++i ) {
			memset(&ze, 0, sizeof(ze));
			if( GetZipItem(m_hz, i, &ze) != ZR_OK ) continue;
			CDuiString item = NormalizeZipName(ze.name);
			if( bIgnoreCase ) {
				if( item.CompareNoCase(name.GetData()) == 0 ) {
					m_last = ZR_OK;
					m_errText.Empty();
					return i;
				}
				if( NormalizeDirName(item.GetData()).CompareNoCase(nameDir.GetData()) == 0 ) {
					m_last = ZR_OK;
					m_errText.Empty();
					return i;
				}
			}
			else if( item.Compare(name.GetData()) == 0 ) {
				m_last = ZR_OK;
				m_errText.Empty();
				return i;
			}
		}
		m_last = ZR_NOTFOUND;
		m_errText = _T("entry not found");
		return -1;
	}

	bool CZipFile::Exists(LPCTSTR pszName)
	{
		return Find(pszName, true) >= 0;
	}

	bool CZipFile::ClassifyDir(LPCTSTR pszDir, bool& bDirEntry, bool& bChildren, bool& bFileExact)
	{
		bDirEntry = false;
		bChildren = false;
		bFileExact = false;
		CDuiString dir = NormalizeDirName(pszDir);
		if( dir.IsEmpty() ) return false;

		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		if( GetZipItem(m_hz, -1, &ze) != ZR_OK ) return false;
		int n = ze.index;
		CDuiString prefix = dir;
		prefix += _T('/');
		for( int i = 0; i < n; ++i ) {
			memset(&ze, 0, sizeof(ze));
			GetZipItem(m_hz, i, &ze);
			CDuiString item = NormalizeZipName(ze.name);
			CDuiString itemDir = NormalizeDirName(item.GetData());
			bool bItemDir = (ze.attr & FILE_ATTRIBUTE_DIRECTORY) != 0
				|| (item.GetLength() > 0 && item.GetAt(item.GetLength() - 1) == _T('/'));
			if( itemDir.CompareNoCase(dir.GetData()) == 0 ) {
				if( bItemDir ) bDirEntry = true;
				else bFileExact = true;
				continue;
			}
			if( item.GetLength() >= prefix.GetLength() ) {
				CDuiString head = item.Left(prefix.GetLength());
				if( head.CompareNoCase(prefix.GetData()) == 0 )
					bChildren = true;
			}
		}
		return true;
	}

	bool CZipFile::IsDir(LPCTSTR pszName)
	{
		if( !EnsureRead() ) return false;
		bool bDirEntry = false, bChildren = false, bFileExact = false;
		if( !ClassifyDir(pszName, bDirEntry, bChildren, bFileExact) ) return false;
		m_last = ZR_OK;
		m_errText.Empty();
		return bDirEntry || bChildren;
	}

	bool CZipFile::ExtractOne(int nIndex, const ZIPENTRY& ze, LPCTSTR pszDestPath, LPCTSTR pszContainRoot)
	{
		if( pszContainRoot != NULL && *pszContainRoot != _T('\0') ) {
			if( !IsPathContained(pszContainRoot, pszDestPath) )
				return Fail(ZR_ARGS, _T("extract path escapes destination"));
		}
		if( ze.attr & FILE_ATTRIBUTE_DIRECTORY ) {
			if( !EnsureDirectory(pszDestPath) )
				return Fail(ZR_NOFILE, _T("create directory failed"));
			return Ok();
		}
		if( !CheckUncompressed(ze.unc_size) )
			return Fail(ZR_MEMSIZE, _T("entry too large"));
		if( !EnsureParentDir(pszDestPath) )
			return Fail(ZR_NOFILE, _T("create parent directory failed"));
		m_last = UnzipItem(m_hz, nIndex, pszDestPath);
		if( m_last != ZR_OK )
			return Fail(m_last, _T("extract failed"));
		return Ok();
	}

	bool CZipFile::ExtractFile(LPCTSTR pszName, LPCTSTR pszDestPath)
	{
		if( pszDestPath == NULL || *pszDestPath == _T('\0') )
			return Fail(ZR_ARGS, _T("dest path is empty"));
		int idx = Find(pszName, true);
		if( idx < 0 ) return Fail(ZR_NOTFOUND, _T("entry not found"));
		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		m_last = GetZipItem(m_hz, idx, &ze);
		if( m_last != ZR_OK ) return false;
		CDuiString name = NormalizeZipName(ze.name);
		if( !IsSafeZipName(name.GetData()) )
			return Fail(ZR_ARGS, _T("unsafe zip entry path"));
		return ExtractOne(idx, ze, pszDestPath, NULL);
	}

	bool CZipFile::ExtractDir(LPCTSTR pszDir, LPCTSTR pszDestDir)
	{
		if( pszDestDir == NULL || *pszDestDir == _T('\0') )
			return Fail(ZR_ARGS, _T("dest dir is empty"));
		if( !EnsureRead() ) return false;
		CDuiString dir = NormalizeDirName(pszDir);
		if( dir.IsEmpty() ) return Fail(ZR_ARGS, _T("directory name is empty"));
		if( !IsSafeZipName(dir.GetData()) )
			return Fail(ZR_ARGS, _T("unsafe zip entry path"));

		bool bDirEntry = false, bChildren = false, bFileExact = false;
		if( !ClassifyDir(dir.GetData(), bDirEntry, bChildren, bFileExact) ) return false;
		if( !bDirEntry && !bChildren ) {
			if( bFileExact ) return Fail(ZR_ARGS, _T("not a directory"));
			return Fail(ZR_NOTFOUND, _T("directory not found"));
		}
		if( !EnsureDirectory(pszDestDir) )
			return Fail(ZR_NOFILE, _T("create dest dir failed"));

		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		m_last = GetZipItem(m_hz, -1, &ze);
		if( m_last != ZR_OK ) return false;
		int n = ze.index;
		int nPrefix = dir.GetLength();
		for( int i = 0; i < n; ++i ) {
			memset(&ze, 0, sizeof(ze));
			m_last = GetZipItem(m_hz, i, &ze);
			if( m_last != ZR_OK ) return false;
			CDuiString item = NormalizeZipName(ze.name);
			if( !NameMatch(item, dir, true) ) continue;
			if( !IsSafeZipName(item.GetData()) )
				return Fail(ZR_ARGS, _T("unsafe zip entry path"));
			CDuiString rel = item;
			if( rel.GetLength() == nPrefix )
				rel.Empty();
			else if( rel.GetLength() > nPrefix + 1 )
				rel = rel.Mid(nPrefix + 1);
			else
				rel.Empty();
			CDuiString dest = rel.IsEmpty() ? CDuiString(pszDestDir) : JoinPath(pszDestDir, rel.GetData());
			if( !ExtractOne(i, ze, dest.GetData(), pszDestDir) ) return false;
		}
		return Ok();
	}

	bool CZipFile::ExtractAll(LPCTSTR pszDestDir)
	{
		if( pszDestDir == NULL || *pszDestDir == _T('\0') )
			return Fail(ZR_ARGS, _T("dest dir is empty"));
		if( !EnsureRead() ) return false;
		if( !EnsureDirectory(pszDestDir) )
			return Fail(ZR_NOFILE, _T("create dest dir failed"));

		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		m_last = GetZipItem(m_hz, -1, &ze);
		if( m_last != ZR_OK ) return false;
		int n = ze.index;
		for( int i = 0; i < n; ++i ) {
			memset(&ze, 0, sizeof(ze));
			m_last = GetZipItem(m_hz, i, &ze);
			if( m_last != ZR_OK ) return false;
			CDuiString name = NormalizeZipName(ze.name);
			if( name.IsEmpty() ) continue;
			if( !IsSafeZipName(name.GetData()) )
				return Fail(ZR_ARGS, _T("unsafe zip entry path"));
			CDuiString dest = JoinPath(pszDestDir, name.GetData());
			if( !ExtractOne(i, ze, dest.GetData(), pszDestDir) ) return false;
		}
		return Ok();
	}

	bool CZipFile::ExtractMemory(LPCTSTR pszName, BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData ) *ppData = NULL;
		if( pdwSize ) *pdwSize = 0;
		if( ppData == NULL || pdwSize == NULL )
			return Fail(ZR_ARGS, _T("output buffer is null"));
		int idx = Find(pszName, true);
		if( idx < 0 ) return Fail(ZR_NOTFOUND, _T("entry not found"));
		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		m_last = GetZipItem(m_hz, idx, &ze);
		if( m_last != ZR_OK ) return false;
		if( ze.attr & FILE_ATTRIBUTE_DIRECTORY )
			return Fail(ZR_ARGS, _T("item is a directory"));
		if( ze.unc_size < 0 ) return Fail(ZR_CORRUPT, _T("unknown uncompressed size"));
		if( !CheckUncompressed(ze.unc_size) )
			return Fail(ZR_MEMSIZE, _T("entry too large"));
		if( ze.unc_size == 0 ) {
			*ppData = new BYTE[1];
			*pdwSize = 0;
			return Ok();
		}
		BYTE* p = new BYTE[ze.unc_size];
		m_last = UnzipItem(m_hz, idx, p, ze.unc_size);
		if( m_last != ZR_OK ) {
			delete[] p;
			return Fail(m_last, _T("extract failed"));
		}
		*ppData = p;
		*pdwSize = (DWORD)ze.unc_size;
		return Ok();
	}

	bool CZipFile::AddInWriteMode(EMutate op, LPCTSTR pszZipName, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bReplace)
	{
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, op == kAddDir) ) return false;
		if( FindWriteName(name.GetData()) >= 0 ) {
			if( !bReplace ) return Fail(ZR_ARGS, _T("entry already exists"));
			if( !CommitWrite() ) return false;
			return Mutate(op, name.GetData(), pData, dwSize, pszSrcFile, true, false);
		}
		ZRESULT r = AddNewItem(m_hz, op, name, pData, dwSize, pszSrcFile);
		if( r != ZR_OK ) return Fail(r, _T("add failed"));
		RememberWriteName(name.GetData());
		return Ok();
	}

	bool CZipFile::AddFile(LPCTSTR pszZipName, LPCTSTR pszSrcFile, bool bReplace)
	{
		if( pszSrcFile == NULL || *pszSrcFile == _T('\0') )
			return Fail(ZR_ARGS, _T("src file is empty"));
		CDuiString name;
		if( pszZipName && *pszZipName ) name = pszZipName;
		else name = FileNameOf(pszSrcFile);
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_hz )
			return AddInWriteMode(kAddFile, name.GetData(), NULL, 0, pszSrcFile, bReplace);
		return Mutate(kAddFile, name.GetData(), NULL, 0, pszSrcFile, bReplace, false);
	}

	bool CZipFile::AddMemory(LPCTSTR pszZipName, const void* pData, DWORD dwSize, bool bReplace)
	{
		if( pszZipName == NULL || *pszZipName == _T('\0') )
			return Fail(ZR_ARGS, _T("entry name is empty"));
		if( dwSize > 0 && pData == NULL )
			return Fail(ZR_ARGS, _T("data is null"));
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_hz )
			return AddInWriteMode(kAddMem, name.GetData(), pData, dwSize, NULL, bReplace);
		return Mutate(kAddMem, name.GetData(), pData, dwSize, NULL, bReplace, false);
	}

	bool CZipFile::AddFolder(LPCTSTR pszZipName)
	{
		if( pszZipName == NULL || *pszZipName == _T('\0') )
			return Fail(ZR_ARGS, _T("entry name is empty"));
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, true) ) return false;
		if( m_mode == ModeWrite && m_hz )
			return AddInWriteMode(kAddDir, name.GetData(), NULL, 0, NULL, true);
		return Mutate(kAddDir, name.GetData(), NULL, 0, NULL, true, false);
	}

	bool CZipFile::AddDirWalk(const CDuiString& zipDir, LPCTSTR pszSrcDir, bool bRecursive)
	{
		CDuiString pattern(pszSrcDir);
		int n = pattern.GetLength();
		if( n > 0 ) {
			TCHAR c = pattern.GetAt(n - 1);
			if( c != _T('\\') && c != _T('/') ) pattern += _T('\\');
		}
		pattern += _T('*');

		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(pattern.GetData(), &fd);
		if( hFind == INVALID_HANDLE_VALUE ) return Ok();
		do {
			if( fd.cFileName[0] == _T('.') &&
				(fd.cFileName[1] == _T('\0') || (fd.cFileName[1] == _T('.') && fd.cFileName[2] == _T('\0'))) )
				continue;
			CDuiString childSrc(pszSrcDir);
			int ns = childSrc.GetLength();
			if( ns > 0 ) {
				TCHAR c = childSrc.GetAt(ns - 1);
				if( c != _T('\\') && c != _T('/') ) childSrc += _T('\\');
			}
			childSrc += fd.cFileName;
			CDuiString childZip = zipDir;
			if( !childZip.IsEmpty() ) childZip += _T('/');
			childZip += fd.cFileName;
			if( fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
				continue;
			if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				if( !bRecursive ) continue;
				if( !AddFolder(childZip.GetData()) ) {
					::FindClose(hFind);
					return false;
				}
				if( !AddDirWalk(childZip, childSrc.GetData(), true) ) {
					::FindClose(hFind);
					return false;
				}
			}
			else {
				if( !AddFile(childZip.GetData(), childSrc.GetData(), true) ) {
					::FindClose(hFind);
					return false;
				}
			}
		} while( ::FindNextFile(hFind, &fd) );
		::FindClose(hFind);
		return Ok();
	}

	bool CZipFile::AddDir(LPCTSTR pszZipDir, LPCTSTR pszSrcDir, bool bRecursive)
	{
		if( pszSrcDir == NULL || *pszSrcDir == _T('\0') )
			return Fail(ZR_ARGS, _T("src dir is empty"));
		DWORD attr = ::GetFileAttributes(pszSrcDir);
		if( attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0 )
			return Fail(ZR_ARGS, _T("src is not a directory"));
		CDuiString zipDir;
		if( pszZipDir && *pszZipDir ) {
			zipDir = pszZipDir;
			if( !EnsureEntryName(zipDir, true) ) return false;
			if( !AddFolder(zipDir.GetData()) ) return false;
		}
		return AddDirWalk(zipDir, pszSrcDir, bRecursive);
	}

	bool CZipFile::UpdateFile(LPCTSTR pszZipName, LPCTSTR pszSrcFile)
	{
		if( pszSrcFile == NULL || *pszSrcFile == _T('\0') )
			return Fail(ZR_ARGS, _T("src file is empty"));
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_hz ) {
			if( !CommitWrite() ) return false;
		}
		return Mutate(kUpdateFile, name.GetData(), NULL, 0, pszSrcFile, true, false);
	}

	bool CZipFile::UpdateMemory(LPCTSTR pszZipName, const void* pData, DWORD dwSize)
	{
		if( dwSize > 0 && pData == NULL )
			return Fail(ZR_ARGS, _T("data is null"));
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_hz ) {
			if( !CommitWrite() ) return false;
		}
		return Mutate(kUpdateMem, name.GetData(), pData, dwSize, NULL, true, false);
	}

	bool CZipFile::Remove(LPCTSTR pszName, bool bRecursive)
	{
		if( m_mode == ModeWrite && m_hz ) {
			if( !CommitWrite() ) return false;
		}
		return Mutate(kRemove, pszName, NULL, 0, NULL, false, bRecursive);
	}

	bool CZipFile::RemoveDir(LPCTSTR pszDir)
	{
		if( m_mode == ModeWrite && m_hz ) {
			if( !CommitWrite() ) return false;
		}
		if( !EnsureRead() ) return false;
		CDuiString dir = NormalizeDirName(pszDir);
		if( dir.IsEmpty() ) return Fail(ZR_ARGS, _T("directory name is empty"));

		bool bDirEntry = false, bChildren = false, bFileExact = false;
		if( !ClassifyDir(dir.GetData(), bDirEntry, bChildren, bFileExact) ) return false;
		if( !bDirEntry && !bChildren ) {
			if( bFileExact ) return Fail(ZR_ARGS, _T("not a directory"));
			return Fail(ZR_NOTFOUND, _T("directory not found"));
		}
		return Mutate(kRemove, dir.GetData(), NULL, 0, NULL, false, true);
	}

	bool CZipFile::Mutate(EMutate op, LPCTSTR pszZipName, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bReplace, bool bRecursive)
	{
		if( !EnsureRead() ) return false;
		CDuiString target = (op == kAddDir) ? NormalizeDirName(pszZipName) : NormalizeZipName(pszZipName);
		if( target.IsEmpty() ) return Fail(ZR_ARGS, _T("entry name is empty"));
		if( op != kRemove && !IsSafeZipName(target.GetData()) )
			return Fail(ZR_ARGS, _T("unsafe zip entry path"));

		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		m_last = GetZipItem(m_hz, -1, &ze);
		if( m_last != ZR_OK ) return false;
		int n = ze.index;
		bool found = false;
		for( int i = 0; i < n; ++i ) {
			memset(&ze, 0, sizeof(ze));
			GetZipItem(m_hz, i, &ze);
			if( NameMatch(NormalizeZipName(ze.name), target, bRecursive && op == kRemove) ) {
				found = true;
				break;
			}
		}

		if( op == kRemove ) {
			if( !found ) return Fail(ZR_NOTFOUND, _T("entry not found"));
		}
		else if( op == kUpdateFile || op == kUpdateMem ) {
			if( !found ) return Fail(ZR_NOTFOUND, _T("entry not found"));
		}
		else {
			if( found && !bReplace ) return Fail(ZR_ARGS, _T("entry already exists"));
		}

		return Rewrite(op, target, pData, dwSize, pszSrcFile, bRecursive);
	}

	ZRESULT CZipFile::CopyItem(HZIP hzSrc, HZIP hzDst, int nIndex)
	{
		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		ZRESULT r = GetZipItem(hzSrc, nIndex, &ze);
		if( r != ZR_OK ) return r;
		CDuiString name = NormalizeZipName(ze.name);
		if( name.IsEmpty() || !IsSafeZipName(name.GetData()) ) return ZR_ARGS;
		if( ze.attr & FILE_ATTRIBUTE_DIRECTORY )
			return ZipAddFolder(hzDst, name.GetData());
		if( ze.unc_size < 0 ) return ZR_CORRUPT;
		if( !CheckUncompressed(ze.unc_size) ) return ZR_MEMSIZE;
		if( ze.unc_size == 0 ) {
			char dummy = 0;
			return ZipAdd(hzDst, name.GetData(), &dummy, 0);
		}
		BYTE* p = new BYTE[ze.unc_size];
		r = UnzipItem(hzSrc, nIndex, p, ze.unc_size);
		if( r != ZR_OK ) {
			delete[] p;
			return r;
		}
		r = ZipAdd(hzDst, name.GetData(), p, ze.unc_size);
		delete[] p;
		return r;
	}

	ZRESULT CZipFile::AddNewItem(HZIP hzDst, EMutate op, const CDuiString& name, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile)
	{
		if( op == kAddDir )
			return ZipAddFolder(hzDst, name.GetData());
		if( op == kAddFile || op == kUpdateFile )
			return ZipAdd(hzDst, name.GetData(), pszSrcFile);
		char dummy = 0;
		if( dwSize == 0 )
			return ZipAdd(hzDst, name.GetData(), &dummy, 0);
		return ZipAdd(hzDst, name.GetData(), (void*)pData, dwSize);
	}

	bool CZipFile::Rewrite(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive)
	{
		if( !m_path.IsEmpty() )
			return RewriteToFile(op, target, pData, dwSize, pszSrcFile, bRecursive);
		return RewriteToMemory(op, target, pData, dwSize, pszSrcFile, bRecursive);
	}

	bool CZipFile::RewriteToFile(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive)
	{
		if( !PreparePassword() ) return false;
		CDuiString tmp;
		if( !MakeTempZipPath(m_path.GetData(), tmp) )
			return Fail(ZR_NOFILE, _T("create temp zip failed"));

		HZIP hzNew = CreateZip(tmp.GetData(), Pwd());
		if( hzNew == NULL ) {
			::DeleteFile(tmp.GetData());
			return Fail(ZR_RECENT, _T("create temp zip failed"));
		}

		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		ZRESULT r = GetZipItem(m_hz, -1, &ze);
		if( r != ZR_OK ) {
			CloseZip(hzNew);
			::DeleteFile(tmp.GetData());
			return Fail(r);
		}
		int n = ze.index;
		int nCopied = 0;
		for( int i = 0; i < n; ++i ) {
			memset(&ze, 0, sizeof(ze));
			r = GetZipItem(m_hz, i, &ze);
			if( r != ZR_OK ) {
				CloseZip(hzNew);
				::DeleteFile(tmp.GetData());
				return Fail(r);
			}
			CDuiString item = NormalizeZipName(ze.name);
			bool match = NameMatch(item, target, bRecursive && op == kRemove);
			if( !match && op != kRemove )
				match = (item.CompareNoCase(target.GetData()) == 0);
			if( match ) continue;
			r = CopyItem(m_hz, hzNew, i);
			if( r != ZR_OK ) {
				CloseZip(hzNew);
				::DeleteFile(tmp.GetData());
				return Fail(r, _T("copy entry failed"));
			}
			++nCopied;
		}

		if( op != kRemove ) {
			r = AddNewItem(hzNew, op, target, pData, dwSize, pszSrcFile);
			if( r != ZR_OK ) {
				CloseZip(hzNew);
				::DeleteFile(tmp.GetData());
				return Fail(r, _T("add entry failed"));
			}
		}
		else if( nCopied == n ) {
			CloseZip(hzNew);
			::DeleteFile(tmp.GetData());
			return Fail(ZR_NOTFOUND, _T("entry not found"));
		}

		CloseZip(hzNew);
		CloseZip(m_hz);
		m_hz = NULL;
		m_mode = ModeNone;
		if( !ReplaceZipAtomically(m_path.GetData(), tmp.GetData()) ) {
			::DeleteFile(tmp.GetData());
			ReopenRead();
			return Fail(ZR_NOFILE, _T("replace zip file failed"));
		}
		return ReopenRead();
	}

	unsigned int CZipFile::EstimateMemCap(DWORD dwExtra) const
	{
		unsigned int cap = 65536 + dwExtra + 4096;
		if( m_hz == NULL ) return cap;
		ZIPENTRY ze;
		memset(&ze, 0, sizeof(ze));
		if( GetZipItem(m_hz, -1, &ze) != ZR_OK ) return cap;
		int n = ze.index;
		for( int i = 0; i < n; ++i ) {
			memset(&ze, 0, sizeof(ze));
			if( GetZipItem(m_hz, i, &ze) != ZR_OK ) continue;
			long u = ze.unc_size > 0 ? ze.unc_size : 0;
			unsigned int add = (unsigned int)u + 512;
			if( cap > 0xFFFFFFFFu - add ) return 0xFFFFFFFFu;
			cap += add;
		}
		if( cap < 256 * 1024 ) cap = 256 * 1024;
		return cap;
	}

	bool CZipFile::RewriteToMemory(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive)
	{
		if( !PreparePassword() ) return false;
		unsigned int cap = EstimateMemCap(dwSize);
		HZIP hzNew = NULL;
		ZRESULT r = ZR_OK;
		for( int attempt = 0; attempt < 5; ++attempt ) {
			hzNew = CreateZip(0, cap, Pwd());
			if( hzNew == NULL )
				return Fail(ZR_RECENT, _T("create memory zip failed"));

			ZIPENTRY ze;
			memset(&ze, 0, sizeof(ze));
			r = GetZipItem(m_hz, -1, &ze);
			if( r != ZR_OK ) {
				CloseZip(hzNew);
				return Fail(r);
			}
			int n = ze.index;
			bool skipped = false;
			bool memFail = false;
			for( int i = 0; i < n; ++i ) {
				memset(&ze, 0, sizeof(ze));
				r = GetZipItem(m_hz, i, &ze);
				if( r != ZR_OK ) {
					CloseZip(hzNew);
					return Fail(r);
				}
				CDuiString item = NormalizeZipName(ze.name);
				bool match = NameMatch(item, target, bRecursive && op == kRemove);
				if( !match && (op == kAddFile || op == kAddMem || op == kAddDir || op == kUpdateFile || op == kUpdateMem) )
					match = (item.CompareNoCase(target.GetData()) == 0);
				if( match ) {
					skipped = true;
					continue;
				}
				r = CopyItem(m_hz, hzNew, i);
				if( r == ZR_MEMSIZE ) { memFail = true; break; }
				if( r != ZR_OK ) {
					CloseZip(hzNew);
					return Fail(r, _T("copy entry failed"));
				}
			}
			if( !memFail && op != kRemove ) {
				r = AddNewItem(hzNew, op, target, pData, dwSize, pszSrcFile);
				if( r == ZR_MEMSIZE ) memFail = true;
				else if( r != ZR_OK ) {
					CloseZip(hzNew);
					return Fail(r, _T("add entry failed"));
				}
			}
			else if( !memFail && op == kRemove && !skipped ) {
				CloseZip(hzNew);
				return Fail(ZR_NOTFOUND, _T("entry not found"));
			}

			if( memFail ) {
				CloseZip(hzNew);
				hzNew = NULL;
				if( cap > 0x7FFFFFFF / 2 ) return Fail(ZR_MEMSIZE, _T("memory zip too small"));
				cap *= 2;
				continue;
			}

			void* p = NULL;
			unsigned long nOut = 0;
			r = ZipGetMemory(hzNew, &p, &nOut);
			if( r != ZR_OK ) {
				CloseZip(hzNew);
				return Fail(r, _T("ZipGetMemory failed"));
			}
			BYTE* pCopy = NULL;
			if( nOut > 0 ) {
				pCopy = new BYTE[nOut];
				memcpy(pCopy, p, nOut);
			}
			CloseZip(hzNew);
			CloseZip(m_hz);
			m_hz = NULL;
			delete[] m_memBuf;
			m_memBuf = pCopy;
			m_memLen = nOut;
			m_mode = ModeNone;
			if( m_memBuf == NULL || m_memLen == 0 )
				return Fail(ZR_CORRUPT, _T("empty memory zip"));
			return ReopenRead();
		}
		return Fail(ZR_MEMSIZE, _T("memory zip too small"));
	}

	bool CZipFile::SaveAs(LPCTSTR pszZipPath)
	{
		if( pszZipPath == NULL || *pszZipPath == _T('\0') )
			return Fail(ZR_ARGS, _T("zip path is empty"));
		if( !EnsureRead() ) return false;
		if( !m_path.IsEmpty() ) {
			if( m_path.CompareNoCase(pszZipPath) == 0 ) return Ok();
			if( !::CopyFile(m_path.GetData(), pszZipPath, FALSE) )
				return Fail(ZR_NOFILE, _T("copy zip failed"));
			return Ok();
		}
		if( m_memBuf == NULL || m_memLen == 0 )
			return Fail(ZR_ARGS, _T("memory zip is empty"));
		if( !WriteWholeFile(pszZipPath, m_memBuf, m_memLen) )
			return Fail(ZR_WRITE, _T("write zip failed"));
		return Ok();
	}

	bool CZipFile::GetMemory(BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData ) *ppData = NULL;
		if( pdwSize ) *pdwSize = 0;
		if( ppData == NULL || pdwSize == NULL )
			return Fail(ZR_ARGS, _T("output buffer is null"));
		if( !EnsureRead() ) return false;
		if( m_memBuf != NULL && m_memLen > 0 ) {
			BYTE* p = new BYTE[m_memLen];
			memcpy(p, m_memBuf, m_memLen);
			*ppData = p;
			*pdwSize = m_memLen;
			return Ok();
		}
		if( m_path.IsEmpty() ) return Fail(ZR_ARGS, _T("zip not open"));
		if( !ReadWholeFile(m_path.GetData(), ppData, pdwSize) )
			return Fail(ZR_READ, _T("read zip file failed"));
		return Ok();
	}

	ZRESULT CZipFile::GetLastResult() const
	{
		return m_last;
	}

	CDuiString CZipFile::GetErrorMessage() const
	{
		if( !m_errText.IsEmpty() ) return m_errText;
		TCHAR buf[512];
		buf[0] = _T('\0');
		FormatZipMessage(m_last, buf, 512);
		return CDuiString(buf);
	}

	bool CZipFile::UnzipToDirectory(LPCTSTR pszZipPath, LPCTSTR pszDestDir, LPCTSTR pszPassword, ZipPasswordEncoding encoding)
	{
		CZipFile zip;
		zip.SetPassword(pszPassword, encoding);
		if( !zip.Open(pszZipPath) ) return false;
		return zip.ExtractAll(pszDestDir);
	}
}
