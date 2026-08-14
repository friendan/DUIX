#include "StdAfx.h"
#include "UIZip.h"

#include <time.h>

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_mem.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "ncrypt.lib")
#pragma comment(lib, "crypt32.lib")

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
		// 目录条目常带尾部 '/'，不算空段。
		while( nLen > 0 && (pszName[nLen - 1] == _T('/') || pszName[nLen - 1] == _T('\\')) )
			--nLen;
		if( nLen <= 0 ) return false;

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

	static char* DupWideToUtf8(LPCTSTR psz)
	{
		if( psz == NULL ) return NULL;
		int nNeed = ::WideCharToMultiByte(CP_UTF8, 0, psz, -1, NULL, 0, NULL, NULL);
		if( nNeed <= 0 ) return NULL;
		char* p = new char[nNeed];
		if( ::WideCharToMultiByte(CP_UTF8, 0, psz, -1, p, nNeed, NULL, NULL) <= 0 ) {
			delete[] p;
			return NULL;
		}
		return p;
	}

	static CDuiString Utf8ToWide(const char* psz)
	{
		CDuiString s;
		if( psz == NULL || *psz == '\0' ) return s;
		int nNeed = ::MultiByteToWideChar(CP_UTF8, 0, psz, -1, NULL, 0);
		if( nNeed <= 0 ) return s;
		wchar_t* p = new wchar_t[nNeed];
		if( ::MultiByteToWideChar(CP_UTF8, 0, psz, -1, p, nNeed) > 0 )
			s = p;
		delete[] p;
		return s;
	}

	static void UnixTimeToFileTime(time_t t, FILETIME& ft)
	{
		if( t <= 0 ) {
			memset(&ft, 0, sizeof(ft));
			return;
		}
		ULONGLONG ll = ((ULONGLONG)t * 10000000ULL) + 116444736000000000ULL;
		ft.dwLowDateTime = (DWORD)ll;
		ft.dwHighDateTime = (DWORD)(ll >> 32);
	}

	static LPCTSTR MzErrorText(int code)
	{
		switch( code ) {
		case MZ_OK: return _T("ok");
		case MZ_STREAM_ERROR: return _T("stream error");
		case MZ_DATA_ERROR: return _T("data error");
		case MZ_MEM_ERROR: return _T("out of memory");
		case MZ_BUF_ERROR: return _T("buffer error");
		case MZ_PARAM_ERROR: return _T("invalid argument");
		case MZ_FORMAT_ERROR: return _T("invalid zip format");
		case MZ_INTERNAL_ERROR: return _T("internal error");
		case MZ_CRC_ERROR: return _T("crc error");
		case MZ_CRYPT_ERROR: return _T("crypt error");
		case MZ_EXIST_ERROR: return _T("already exists");
		case MZ_PASSWORD_ERROR: return _T("password error");
		case MZ_SUPPORT_ERROR: return _T("unsupported feature");
		case MZ_OPEN_ERROR: return _T("open failed");
		case MZ_CLOSE_ERROR: return _T("close failed");
		case MZ_SEEK_ERROR: return _T("seek failed");
		case MZ_TELL_ERROR: return _T("tell failed");
		case MZ_READ_ERROR: return _T("read failed");
		case MZ_WRITE_ERROR: return _T("write failed");
		case MZ_END_OF_LIST: return _T("end of list");
		default: return _T("zip error");
		}
	}

	CZipFile::CZipFile()
		: m_reader(NULL)
		, m_writer(NULL)
		, m_memStream(NULL)
		, m_mode(ModeNone)
		, m_pwdA(NULL)
		, m_memBuf(NULL)
		, m_memLen(0)
		, m_bMemOwned(true)
		, m_maxMem(0)
		, m_maxUncompressed(kDefaultMaxUncompressed)
		, m_compressLevel(MZ_COMPRESS_LEVEL_DEFAULT)
		, m_cachedCount(-1)
		, m_last(MZ_OK)
	{
	}

	CZipFile::~CZipFile()
	{
		Close();
	}

	void CZipFile::SetPassword(LPCTSTR pszPassword)
	{
		if( pszPassword == NULL )
			m_password.Empty();
		else
			m_password = pszPassword;
		SyncCryptoOpts();
	}

	LPCTSTR CZipFile::GetPassword() const
	{
		return m_password.GetData();
	}

	void CZipFile::SetMaxUncompressedSize(DWORD dwBytes)
	{
		m_maxUncompressed = dwBytes;
	}

	DWORD CZipFile::GetMaxUncompressedSize() const
	{
		return m_maxUncompressed;
	}

	void CZipFile::SetCompressLevel(int nLevel)
	{
		if( nLevel < -1 ) nLevel = -1;
		if( nLevel > 9 ) nLevel = 9;
		m_compressLevel = nLevel;
		if( m_writer != NULL )
			ApplyWriterOpts(m_writer);
	}

	int CZipFile::GetCompressLevel() const
	{
		return m_compressLevel;
	}

	bool CZipFile::Ok()
	{
		m_last = MZ_OK;
		m_errText.Empty();
		return true;
	}

	bool CZipFile::Fail(int mzCode, LPCTSTR pszText)
	{
		m_last = mzCode;
		m_errText = pszText;
		return false;
	}

	bool CZipFile::PreparePassword()
	{
		// 先算出新缓冲，失败时不改 m_pwdA，避免 reader/writer 悬空指向已释放密码。
		char* neu = NULL;
		if( !m_password.IsEmpty() ) {
			LPCTSTR psz = m_password.GetData();
			int nNeed = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, psz, -1, NULL, 0, NULL, NULL);
			if( nNeed <= 1 )
				return Fail(MZ_PARAM_ERROR, _T("password utf8 conversion failed"));

			neu = new char[nNeed];
			int nGot = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, psz, -1, neu, nNeed, NULL, NULL);
			if( nGot <= 1 ) {
				delete[] neu;
				return Fail(MZ_PARAM_ERROR, _T("password utf8 conversion failed"));
			}
		}

		delete[] m_pwdA;
		m_pwdA = neu;
		// 指针已换，立刻同步到已打开的句柄，避免 copy_from_reader / 解压读到悬空密码。
		if( m_writer != NULL )
			ApplyWriterOpts(m_writer);
		if( m_reader != NULL )
			ApplyReaderOpts(m_reader);
		return true;
	}

	bool CZipFile::SyncCryptoOpts()
	{
		if( !PreparePassword() ) return false;
		return Ok();
	}

	bool CZipFile::CheckMzBufSize(unsigned int nBytes, LPCTSTR pszWhat)
	{
		if( nBytes > (unsigned int)0x7FFFFFFF ) {
			CDuiString msg = pszWhat ? pszWhat : _T("buffer");
			msg += _T(" exceeds 2GB limit");
			return Fail(MZ_PARAM_ERROR, msg.GetData());
		}
		return true;
	}

	bool CZipFile::InstallOwnedMemory(BYTE* pOwned, DWORD nLen)
	{
		if( pOwned == NULL || nLen == 0 ) {
			delete[] pOwned;
			return Fail(MZ_FORMAT_ERROR, _T("empty memory zip"));
		}
		if( !CheckMzBufSize(nLen, _T("memory zip")) ) {
			delete[] pOwned;
			return false;
		}
		if( !PreparePassword() ) {
			delete[] pOwned;
			return false;
		}
		void* reader = mz_zip_reader_create();
		if( reader == NULL ) {
			delete[] pOwned;
			return Fail(MZ_MEM_ERROR, _T("create zip reader failed"));
		}
		ApplyReaderOpts(reader);
		int32_t err = mz_zip_reader_open_buffer(reader, pOwned, (int32_t)nLen, 0);
		if( err != MZ_OK ) {
			mz_zip_reader_delete(&reader);
			delete[] pOwned;
			return Fail(err, _T("open memory zip failed"));
		}
		DestroyReader();
		DestroyWriter();
		FreeMemBuf();
		ClearWriteNames();
		m_memBuf = pOwned;
		m_memLen = nLen;
		m_bMemOwned = true;
		m_maxMem = 0;
		m_reader = reader;
		m_mode = ModeRead;
		InvalidateEntryCount();
		return Ok();
	}

	const char* CZipFile::Pwd() const
	{
		return m_pwdA;
	}

	void CZipFile::ApplyWriterOpts(void* writer)
	{
		if( writer == NULL ) return;
		if( m_compressLevel == 0 ) {
			mz_zip_writer_set_compress_method(writer, MZ_COMPRESS_METHOD_STORE);
			mz_zip_writer_set_compress_level(writer, 0);
		}
		else {
			mz_zip_writer_set_compress_method(writer, MZ_COMPRESS_METHOD_DEFLATE);
			mz_zip_writer_set_compress_level(writer, (int16_t)m_compressLevel);
		}
		if( Pwd() != NULL ) {
			mz_zip_writer_set_aes(writer, 1);
			mz_zip_writer_set_password(writer, Pwd());
		}
		else {
			mz_zip_writer_set_aes(writer, 0);
			mz_zip_writer_set_password(writer, NULL);
		}
	}

	void CZipFile::ApplyReaderOpts(void* reader)
	{
		if( reader == NULL ) return;
		if( Pwd() != NULL )
			mz_zip_reader_set_password(reader, Pwd());
		else
			mz_zip_reader_set_password(reader, NULL);
		mz_zip_reader_set_encoding(reader, MZ_ENCODING_UTF8);
	}

	void CZipFile::DestroyReader()
	{
		if( m_reader != NULL ) {
			mz_zip_reader_delete(&m_reader);
			m_reader = NULL;
		}
	}

	void CZipFile::DestroyWriter()
	{
		if( m_writer != NULL ) {
			mz_zip_writer_delete(&m_writer);
			m_writer = NULL;
		}
		if( m_memStream != NULL ) {
			mz_stream_mem_delete(&m_memStream);
			m_memStream = NULL;
		}
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
		InvalidateEntryCount();
	}

	bool CZipFile::EnsureEntryName(CDuiString& name, bool bDir)
	{
		name = bDir ? NormalizeDirName(name.GetData()) : NormalizeZipName(name.GetData());
		if( name.IsEmpty() ) return Fail(MZ_PARAM_ERROR, _T("entry name is empty"));
		if( !IsSafeZipName(name.GetData()) )
			return Fail(MZ_PARAM_ERROR, _T("unsafe zip entry path"));
		return true;
	}

	bool CZipFile::CheckUncompressed(long nSize) const
	{
		if( nSize < 0 ) return false;
		if( m_maxUncompressed == 0 ) return true;
		return (DWORD)nSize <= m_maxUncompressed;
	}

	void CZipFile::FreeMemBuf()
	{
		if( m_bMemOwned )
			delete[] m_memBuf;
		m_memBuf = NULL;
		m_memLen = 0;
		m_bMemOwned = true;
	}

	void CZipFile::InvalidateEntryCount()
	{
		m_cachedCount = -1;
	}

	void CZipFile::Close()
	{
		DestroyReader();
		DestroyWriter();
		FreeMemBuf();
		m_maxMem = 0;
		m_path.Empty();
		m_mode = ModeNone;
		InvalidateEntryCount();
		ClearWriteNames();
		delete[] m_pwdA;
		m_pwdA = NULL;
	}

	bool CZipFile::Commit()
	{
		if( m_mode == ModeWrite )
			return CommitWrite();
		if( m_mode == ModeRead && m_reader != NULL )
			return Ok();
		if( (m_memBuf != NULL && m_memLen > 0) || !m_path.IsEmpty() )
			return ReopenRead();
		return Fail(MZ_PARAM_ERROR, _T("zip not open"));
	}

	bool CZipFile::IsOpen() const
	{
		return m_reader != NULL || m_writer != NULL;
	}

	bool CZipFile::IsMemory() const
	{
		return m_path.IsEmpty() && (m_memBuf != NULL || (m_mode == ModeWrite && m_writer != NULL));
	}

	bool CZipFile::IsEncrypted()
	{
		if( !EnsureRead() ) return false;
		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		while( err == MZ_OK ) {
			mz_zip_file* info = NULL;
			if( mz_zip_reader_entry_get_info(m_reader, &info) == MZ_OK && info != NULL ) {
				if( (info->flag & MZ_ZIP_FLAG_ENCRYPTED) != 0 ) {
					Ok();
					return true;
				}
			}
			err = mz_zip_reader_goto_next_entry(m_reader);
		}
		Ok();
		return false;
	}

	LPCTSTR CZipFile::GetPath() const
	{
		return m_path.GetData();
	}

	bool CZipFile::Create(LPCTSTR pszZipPath)
	{
		if( pszZipPath == NULL || *pszZipPath == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("zip path is empty"));
		Close();
		m_path = pszZipPath;
		if( !PreparePassword() ) {
			m_path.Empty();
			return false;
		}
		char* pathUtf8 = DupWideToUtf8(m_path.GetData());
		if( pathUtf8 == NULL ) {
			m_path.Empty();
			return Fail(MZ_PARAM_ERROR, _T("path utf8 conversion failed"));
		}
		EnsureParentDir(m_path.GetData());
		m_writer = mz_zip_writer_create();
		if( m_writer == NULL ) {
			delete[] pathUtf8;
			m_path.Empty();
			return Fail(MZ_MEM_ERROR, _T("create zip writer failed"));
		}
		ApplyWriterOpts(m_writer);
		int32_t err = mz_zip_writer_open_file(m_writer, pathUtf8, 0, 0);
		delete[] pathUtf8;
		if( err != MZ_OK ) {
			DestroyWriter();
			m_path.Empty();
			return Fail(err, _T("create zip failed"));
		}
		m_mode = ModeWrite;
		InvalidateEntryCount();
		return Ok();
	}

	bool CZipFile::CreateMemory(unsigned int nMaxBytes)
	{
		Close();
		m_maxMem = nMaxBytes;
		if( !PreparePassword() ) return false;
		m_writer = mz_zip_writer_create();
		if( m_writer == NULL )
			return Fail(MZ_MEM_ERROR, _T("create zip writer failed"));
		ApplyWriterOpts(m_writer);
		m_memStream = mz_stream_mem_create();
		if( m_memStream == NULL ) {
			DestroyWriter();
			return Fail(MZ_MEM_ERROR, _T("create memory stream failed"));
		}
		mz_stream_mem_set_grow_size(m_memStream, 128 * 1024);
		int32_t err = mz_stream_open(m_memStream, NULL, MZ_OPEN_MODE_CREATE);
		if( err != MZ_OK ) {
			DestroyWriter();
			return Fail(err, _T("open memory stream failed"));
		}
		if( nMaxBytes > 0 && nMaxBytes <= (unsigned int)0x7FFFFFFF )
			mz_stream_mem_set_buffer_limit(m_memStream, (int32_t)nMaxBytes);
		err = mz_zip_writer_open(m_writer, m_memStream, 0);
		if( err != MZ_OK ) {
			DestroyWriter();
			return Fail(err, _T("create memory zip failed"));
		}
		m_mode = ModeWrite;
		InvalidateEntryCount();
		return Ok();
	}

	bool CZipFile::Open(LPCTSTR pszZipPath)
	{
		if( pszZipPath == NULL || *pszZipPath == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("zip path is empty"));
		CDuiString path(pszZipPath);
		Close();
		m_path = path;
		if( !PreparePassword() ) {
			m_path.Empty();
			return false;
		}
		char* pathUtf8 = DupWideToUtf8(m_path.GetData());
		if( pathUtf8 == NULL ) {
			m_path.Empty();
			return Fail(MZ_PARAM_ERROR, _T("path utf8 conversion failed"));
		}
		m_reader = mz_zip_reader_create();
		if( m_reader == NULL ) {
			delete[] pathUtf8;
			m_path.Empty();
			return Fail(MZ_MEM_ERROR, _T("create zip reader failed"));
		}
		ApplyReaderOpts(m_reader);
		int32_t err = mz_zip_reader_open_file(m_reader, pathUtf8);
		delete[] pathUtf8;
		if( err != MZ_OK ) {
			DestroyReader();
			m_path.Empty();
			return Fail(err, _T("open zip failed"));
		}
		m_mode = ModeRead;
		InvalidateEntryCount();
		return Ok();
	}

	bool CZipFile::OpenMemory(const void* pData, unsigned int nLen, bool bCopy)
	{
		if( pData == NULL || nLen == 0 )
			return Fail(MZ_PARAM_ERROR, _T("memory zip is empty"));
		if( !CheckMzBufSize(nLen, _T("memory zip")) ) return false;
		BYTE* pBuf = NULL;
		bool bOwned = false;
		if( bCopy ) {
			pBuf = new BYTE[nLen];
			memcpy(pBuf, pData, nLen);
			bOwned = true;
		}
		else {
			pBuf = (BYTE*)pData;
			bOwned = false;
		}
		Close();
		if( !PreparePassword() ) {
			if( bOwned ) delete[] pBuf;
			return false;
		}
		m_memBuf = pBuf;
		m_memLen = nLen;
		m_bMemOwned = bOwned;
		m_reader = mz_zip_reader_create();
		if( m_reader == NULL ) {
			FreeMemBuf();
			return Fail(MZ_MEM_ERROR, _T("create zip reader failed"));
		}
		ApplyReaderOpts(m_reader);
		int32_t err = mz_zip_reader_open_buffer(m_reader, m_memBuf, (int32_t)m_memLen, 0);
		if( err != MZ_OK ) {
			DestroyReader();
			FreeMemBuf();
			return Fail(err, _T("open memory zip failed"));
		}
		m_mode = ModeRead;
		InvalidateEntryCount();
		return Ok();
	}

	bool CZipFile::AttachMemory(const void* pData, unsigned int nLen)
	{
		return OpenMemory(pData, nLen, false);
	}

	bool CZipFile::OpenResource(HINSTANCE hInst, LPCTSTR pszName, LPCTSTR pszType)
	{
		if( pszName == NULL )
			return Fail(MZ_PARAM_ERROR, _T("resource name is empty"));
		if( pszType == NULL ) pszType = RT_RCDATA;
		if( hInst == NULL ) hInst = CPaintManagerUI::GetResourceDll();
		if( hInst == NULL ) hInst = CPaintManagerUI::GetInstance();
		if( hInst == NULL )
			return Fail(MZ_OPEN_ERROR, _T("resource module is null"));

		HRSRC hRes = ::FindResource(hInst, pszName, pszType);
		if( hRes == NULL )
			return Fail(MZ_OPEN_ERROR, _T("resource not found"));
		DWORD nLen = ::SizeofResource(hInst, hRes);
		HGLOBAL hGlob = ::LoadResource(hInst, hRes);
		if( hGlob == NULL || nLen == 0 )
			return Fail(MZ_OPEN_ERROR, _T("load resource failed"));
		const void* pData = ::LockResource(hGlob);
		if( pData == NULL )
			return Fail(MZ_OPEN_ERROR, _T("lock resource failed"));
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
			return Fail(MZ_PARAM_ERROR, _T("module path is empty"));
		HMODULE hMod = LoadPeAsDataFile(pszModulePath);
		if( hMod == NULL )
			return Fail(MZ_OPEN_ERROR, _T("load module failed"));
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
			return Fail(MZ_PARAM_ERROR, _T("module path is empty"));
		if( pszName == NULL )
			return Fail(MZ_PARAM_ERROR, _T("resource name is empty"));
		if( pszType == NULL ) pszType = RT_RCDATA;

		BYTE* pData = NULL;
		DWORD dwSize = 0;
		if( !GetMemory(&pData, &dwSize) ) return false;
		if( pData == NULL || dwSize == 0 ) {
			delete[] pData;
			return Fail(MZ_PARAM_ERROR, _T("zip is empty"));
		}

		HANDLE hUpdate = ::BeginUpdateResource(pszModulePath, FALSE);
		if( hUpdate == NULL ) {
			delete[] pData;
			return Fail(MZ_OPEN_ERROR, _T("begin update resource failed"));
		}
		BOOL bOk = ::UpdateResource(hUpdate, pszType, pszName, wLanguage, pData, dwSize);
		if( !bOk ) {
			::EndUpdateResource(hUpdate, TRUE);
			delete[] pData;
			return Fail(MZ_WRITE_ERROR, _T("update resource failed"));
		}
		bOk = ::EndUpdateResource(hUpdate, FALSE);
		delete[] pData;
		if( !bOk )
			return Fail(MZ_WRITE_ERROR, _T("commit resource failed"));
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
			return Fail(MZ_PARAM_ERROR, _T("dll path is empty"));
		if( !EnsureRead() ) return false;
		if( !WriteResourceOnlyDllStub(pszDllPath) )
			return Fail(MZ_OPEN_ERROR, _T("create resource dll failed"));
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
		DestroyReader();
		DestroyWriter();
		ClearWriteNames();
		m_mode = ModeNone;
		if( !PreparePassword() ) return false;
		if( !m_path.IsEmpty() ) {
			char* pathUtf8 = DupWideToUtf8(m_path.GetData());
			if( pathUtf8 == NULL )
				return Fail(MZ_PARAM_ERROR, _T("path utf8 conversion failed"));
			m_reader = mz_zip_reader_create();
			if( m_reader == NULL ) {
				delete[] pathUtf8;
				return Fail(MZ_MEM_ERROR, _T("create zip reader failed"));
			}
			ApplyReaderOpts(m_reader);
			int32_t err = mz_zip_reader_open_file(m_reader, pathUtf8);
			delete[] pathUtf8;
			if( err != MZ_OK ) {
				DestroyReader();
				return Fail(err, _T("reopen zip failed"));
			}
			m_mode = ModeRead;
			InvalidateEntryCount();
			return Ok();
		}
		if( m_memBuf == NULL || m_memLen == 0 )
			return Fail(MZ_PARAM_ERROR, _T("zip not open"));
		m_reader = mz_zip_reader_create();
		if( m_reader == NULL )
			return Fail(MZ_MEM_ERROR, _T("create zip reader failed"));
		ApplyReaderOpts(m_reader);
		int32_t err = mz_zip_reader_open_buffer(m_reader, m_memBuf, (int32_t)m_memLen, 0);
		if( err != MZ_OK ) {
			DestroyReader();
			return Fail(err, _T("reopen memory zip failed"));
		}
		m_mode = ModeRead;
		InvalidateEntryCount();
		return Ok();
	}

	bool CZipFile::CommitWrite()
	{
		if( m_mode != ModeWrite ) return m_reader != NULL && m_mode == ModeRead;
		if( m_writer == NULL ) {
			m_mode = ModeNone;
			return Fail(MZ_PARAM_ERROR, _T("zip not open"));
		}

		int32_t err = mz_zip_writer_close(m_writer);
		if( err != MZ_OK ) {
			DestroyWriter();
			m_mode = ModeNone;
			return Fail(err, _T("close writer failed"));
		}

		if( !m_path.IsEmpty() ) {
			DestroyWriter();
			return ReopenRead();
		}

		const void* pBuf = NULL;
		int32_t nLen = 0;
		if( m_memStream == NULL || mz_stream_mem_get_buffer(m_memStream, &pBuf) != MZ_OK ) {
			DestroyWriter();
			m_mode = ModeNone;
			return Fail(MZ_INTERNAL_ERROR, _T("get memory zip buffer failed"));
		}
		// minizip-ng 的 get_buffer_length 返回 mem->limit，会被 CreateMemory 里的
		// set_buffer_limit 钉死在容量上限上，拿不到真实已写长度；writer close 后流停在
		// 归档末尾，用 tell 取当前位置即实际长度。
		nLen = (int32_t)mz_stream_tell(m_memStream);
		BYTE* pCopy = NULL;
		if( nLen > 0 && pBuf != NULL ) {
			pCopy = new BYTE[nLen];
			memcpy(pCopy, pBuf, (size_t)nLen);
		}
		DestroyWriter();
		m_mode = ModeNone;
		if( pCopy == NULL || nLen <= 0 ) {
			delete[] pCopy;
			return Fail(MZ_FORMAT_ERROR, _T("empty memory zip"));
		}
		return InstallOwnedMemory(pCopy, (DWORD)nLen);
	}

	bool CZipFile::EnsureRead()
	{
		if( m_mode == ModeRead && m_reader != NULL ) return true;
		if( m_mode == ModeWrite ) return CommitWrite();
		if( (m_memBuf != NULL && m_memLen > 0) || !m_path.IsEmpty() )
			return ReopenRead();
		return Fail(MZ_PARAM_ERROR, _T("zip not open"));
	}

	bool CZipFile::GotoIndex(int nIndex)
	{
		if( m_reader == NULL || nIndex < 0 ) return false;
		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		int i = 0;
		while( err == MZ_OK ) {
			if( i == nIndex ) return true;
			err = mz_zip_reader_goto_next_entry(m_reader);
			++i;
		}
		return false;
	}

	bool CZipFile::FillItemFromCurrent(int nIndex, TZipItem& item)
	{
		mz_zip_file* info = NULL;
		if( mz_zip_reader_entry_get_info(m_reader, &info) != MZ_OK || info == NULL )
			return false;
		item.index = nIndex;
		item.name = NormalizeZipName(Utf8ToWide(info->filename).GetData());
		bool bDir = (mz_zip_reader_entry_is_dir(m_reader) == MZ_OK);
		if( !bDir && item.name.GetLength() > 0 && item.name.GetAt(item.name.GetLength() - 1) == _T('/') )
			bDir = true;
		item.attr = bDir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
		uint32_t winAttr = 0;
		if( mz_zip_attrib_convert(MZ_HOST_SYSTEM(info->version_madeby), info->external_fa,
			MZ_HOST_SYSTEM_WINDOWS_NTFS, &winAttr) == MZ_OK && winAttr != 0 ) {
			item.attr = winAttr;
			if( bDir ) item.attr |= FILE_ATTRIBUTE_DIRECTORY;
		}
		UnixTimeToFileTime(info->modified_date, item.mtime);
		item.comp_size = (info->compressed_size > 0x7FFFFFFF) ? 0x7FFFFFFF : (long)info->compressed_size;
		item.unc_size = (info->uncompressed_size > 0x7FFFFFFF) ? 0x7FFFFFFF : (long)info->uncompressed_size;
		if( info->uncompressed_size < 0 ) item.unc_size = -1;
		return true;
	}

	int CZipFile::GetCount()
	{
		if( !EnsureRead() ) return 0;
		if( m_cachedCount >= 0 ) {
			m_last = MZ_OK;
			m_errText.Empty();
			return m_cachedCount;
		}
		int n = 0;
		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		while( err == MZ_OK ) {
			++n;
			err = mz_zip_reader_goto_next_entry(m_reader);
		}
		m_cachedCount = n;
		m_last = MZ_OK;
		m_errText.Empty();
		return n;
	}

	bool CZipFile::GetItem(int nIndex, TZipItem& item)
	{
		item = TZipItem();
		if( !EnsureRead() ) return false;
		if( !GotoIndex(nIndex) )
			return Fail(MZ_PARAM_ERROR, _T("invalid entry index"));
		if( !FillItemFromCurrent(nIndex, item) )
			return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
		m_errText.Empty();
		m_last = MZ_OK;
		return true;
	}

	bool CZipFile::GetItem(LPCTSTR pszName, TZipItem& item)
	{
		item = TZipItem();
		int idx = Find(pszName, true);
		if( idx < 0 ) return false;
		// Find 已停在目标条目上
		if( !FillItemFromCurrent(idx, item) )
			return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
		m_errText.Empty();
		m_last = MZ_OK;
		return true;
	}

	int CZipFile::Find(LPCTSTR pszName, bool bIgnoreCase)
	{
		if( !EnsureRead() ) return -1;
		CDuiString name = NormalizeZipName(pszName);
		if( name.IsEmpty() ) {
			Fail(MZ_PARAM_ERROR, _T("entry name is empty"));
			return -1;
		}
		CDuiString nameDir = NormalizeDirName(name.GetData());
		int idx = 0;
		int32_t walk = mz_zip_reader_goto_first_entry(m_reader);
		while( walk == MZ_OK ) {
			mz_zip_file* info = NULL;
			if( mz_zip_reader_entry_get_info(m_reader, &info) == MZ_OK && info && info->filename ) {
				CDuiString item = NormalizeZipName(Utf8ToWide(info->filename).GetData());
				bool match = false;
				if( bIgnoreCase ) {
					match = (item.CompareNoCase(name.GetData()) == 0)
						|| (NormalizeDirName(item.GetData()).CompareNoCase(nameDir.GetData()) == 0);
				}
				else {
					match = (item.Compare(name.GetData()) == 0)
						|| (NormalizeDirName(item.GetData()).Compare(nameDir.GetData()) == 0);
				}
				if( match ) {
					m_last = MZ_OK;
					m_errText.Empty();
					return idx;
				}
			}
			walk = mz_zip_reader_goto_next_entry(m_reader);
			++idx;
		}
		m_last = MZ_EXIST_ERROR;
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

		CDuiString prefix = dir;
		prefix += _T('/');
		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		while( err == MZ_OK ) {
			mz_zip_file* info = NULL;
			if( mz_zip_reader_entry_get_info(m_reader, &info) != MZ_OK || info == NULL ) {
				err = mz_zip_reader_goto_next_entry(m_reader);
				continue;
			}
			CDuiString item = NormalizeZipName(Utf8ToWide(info->filename).GetData());
			CDuiString itemDir = NormalizeDirName(item.GetData());
			bool bItemDir = (mz_zip_reader_entry_is_dir(m_reader) == MZ_OK)
				|| (item.GetLength() > 0 && item.GetAt(item.GetLength() - 1) == _T('/'));
			if( itemDir.CompareNoCase(dir.GetData()) == 0 ) {
				if( bItemDir ) bDirEntry = true;
				else bFileExact = true;
			}
			else if( item.GetLength() >= prefix.GetLength() ) {
				CDuiString head = item.Left(prefix.GetLength());
				if( head.CompareNoCase(prefix.GetData()) == 0 )
					bChildren = true;
			}
			err = mz_zip_reader_goto_next_entry(m_reader);
		}
		return true;
	}

	bool CZipFile::IsDir(LPCTSTR pszName)
	{
		if( !EnsureRead() ) return false;
		bool bDirEntry = false, bChildren = false, bFileExact = false;
		if( !ClassifyDir(pszName, bDirEntry, bChildren, bFileExact) ) return false;
		m_last = MZ_OK;
		m_errText.Empty();
		return bDirEntry || bChildren;
	}

	bool CZipFile::ExtractCurrent(LPCTSTR pszDestPath, LPCTSTR pszContainRoot)
	{
		if( !SyncCryptoOpts() ) return false;
		if( pszContainRoot != NULL && *pszContainRoot != _T('\0') ) {
			if( !IsPathContained(pszContainRoot, pszDestPath) )
				return Fail(MZ_PARAM_ERROR, _T("extract path escapes destination"));
		}
		mz_zip_file* info = NULL;
		if( mz_zip_reader_entry_get_info(m_reader, &info) != MZ_OK || info == NULL )
			return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
		bool bDir = (mz_zip_reader_entry_is_dir(m_reader) == MZ_OK);
		if( bDir ) {
			if( !EnsureDirectory(pszDestPath) )
				return Fail(MZ_OPEN_ERROR, _T("create directory failed"));
			return Ok();
		}
		long unc = (info->uncompressed_size > 0x7FFFFFFF) ? 0x7FFFFFFF : (long)info->uncompressed_size;
		if( info->uncompressed_size < 0 )
			return Fail(MZ_FORMAT_ERROR, _T("unknown uncompressed size"));
		if( !CheckUncompressed(unc) )
			return Fail(MZ_MEM_ERROR, _T("entry too large"));
		if( !EnsureParentDir(pszDestPath) )
			return Fail(MZ_OPEN_ERROR, _T("create parent directory failed"));
		char* destUtf8 = DupWideToUtf8(pszDestPath);
		if( destUtf8 == NULL )
			return Fail(MZ_PARAM_ERROR, _T("path utf8 conversion failed"));
		int32_t err = mz_zip_reader_entry_save_file(m_reader, destUtf8);
		delete[] destUtf8;
		if( err != MZ_OK )
			return Fail(err, _T("extract failed"));
		return Ok();
	}

	bool CZipFile::ExtractOne(int nIndex, LPCTSTR pszDestPath, LPCTSTR pszContainRoot)
	{
		if( !GotoIndex(nIndex) )
			return Fail(MZ_PARAM_ERROR, _T("invalid entry index"));
		return ExtractCurrent(pszDestPath, pszContainRoot);
	}

	bool CZipFile::ExtractFile(LPCTSTR pszName, LPCTSTR pszDestPath)
	{
		if( pszDestPath == NULL || *pszDestPath == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("dest path is empty"));
		int idx = Find(pszName, true);
		if( idx < 0 ) return Fail(MZ_EXIST_ERROR, _T("entry not found"));
		TZipItem item;
		if( !FillItemFromCurrent(idx, item) )
			return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
		if( !IsSafeZipName(item.name.GetData()) )
			return Fail(MZ_PARAM_ERROR, _T("unsafe zip entry path"));
		return ExtractCurrent(pszDestPath, NULL);
	}

	bool CZipFile::ExtractDir(LPCTSTR pszDir, LPCTSTR pszDestDir)
	{
		if( pszDestDir == NULL || *pszDestDir == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("dest dir is empty"));
		if( !EnsureRead() ) return false;
		if( !SyncCryptoOpts() ) return false;
		CDuiString dir = NormalizeDirName(pszDir);
		if( dir.IsEmpty() ) return Fail(MZ_PARAM_ERROR, _T("directory name is empty"));
		if( !IsSafeZipName(dir.GetData()) )
			return Fail(MZ_PARAM_ERROR, _T("unsafe zip entry path"));

		bool bDirEntry = false, bChildren = false, bFileExact = false;
		if( !ClassifyDir(dir.GetData(), bDirEntry, bChildren, bFileExact) ) return false;
		if( !bDirEntry && !bChildren ) {
			if( bFileExact ) return Fail(MZ_PARAM_ERROR, _T("not a directory"));
			return Fail(MZ_EXIST_ERROR, _T("directory not found"));
		}
		if( !EnsureDirectory(pszDestDir) )
			return Fail(MZ_OPEN_ERROR, _T("create dest dir failed"));

		int nPrefix = dir.GetLength();
		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		int idx = 0;
		while( err == MZ_OK ) {
			TZipItem item;
			if( !FillItemFromCurrent(idx, item) )
				return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
			CDuiString entry = item.name;
			if( NameMatch(entry, dir, true) ) {
				if( !IsSafeZipName(entry.GetData()) )
					return Fail(MZ_PARAM_ERROR, _T("unsafe zip entry path"));
				CDuiString rel = entry;
				if( rel.GetLength() == nPrefix )
					rel.Empty();
				else if( rel.GetLength() > nPrefix + 1 )
					rel = rel.Mid(nPrefix + 1);
				else
					rel.Empty();
				CDuiString dest = rel.IsEmpty() ? CDuiString(pszDestDir) : JoinPath(pszDestDir, rel.GetData());
				if( !ExtractCurrent(dest.GetData(), pszDestDir) ) return false;
			}
			err = mz_zip_reader_goto_next_entry(m_reader);
			++idx;
		}
		return Ok();
	}

	bool CZipFile::ExtractAll(LPCTSTR pszDestDir)
	{
		if( pszDestDir == NULL || *pszDestDir == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("dest dir is empty"));
		if( !EnsureRead() ) return false;
		if( !SyncCryptoOpts() ) return false;
		if( !EnsureDirectory(pszDestDir) )
			return Fail(MZ_OPEN_ERROR, _T("create dest dir failed"));

		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		int idx = 0;
		while( err == MZ_OK ) {
			TZipItem item;
			if( !FillItemFromCurrent(idx, item) )
				return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
			CDuiString name = item.name;
			if( !name.IsEmpty() ) {
				if( !IsSafeZipName(name.GetData()) )
					return Fail(MZ_PARAM_ERROR, _T("unsafe zip entry path"));
				CDuiString dest = JoinPath(pszDestDir, name.GetData());
				if( !ExtractCurrent(dest.GetData(), pszDestDir) ) return false;
			}
			err = mz_zip_reader_goto_next_entry(m_reader);
			++idx;
		}
		return Ok();
	}

	bool CZipFile::ExtractMemory(LPCTSTR pszName, BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData ) *ppData = NULL;
		if( pdwSize ) *pdwSize = 0;
		if( ppData == NULL || pdwSize == NULL )
			return Fail(MZ_PARAM_ERROR, _T("output buffer is null"));
		if( !EnsureRead() ) return false;
		if( !SyncCryptoOpts() ) return false;
		int idx = Find(pszName, true);
		if( idx < 0 ) return Fail(MZ_EXIST_ERROR, _T("entry not found"));
		if( mz_zip_reader_entry_is_dir(m_reader) == MZ_OK )
			return Fail(MZ_PARAM_ERROR, _T("item is a directory"));
		int32_t need = mz_zip_reader_entry_save_buffer_length(m_reader);
		if( need < 0 )
			return Fail(need, _T("unknown uncompressed size"));
		if( !CheckUncompressed(need) )
			return Fail(MZ_MEM_ERROR, _T("entry too large"));
		if( need == 0 ) {
			*ppData = new BYTE[1];
			*pdwSize = 0;
			return Ok();
		}
		BYTE* p = new BYTE[need];
		int32_t err = mz_zip_reader_entry_save_buffer(m_reader, p, need);
		if( err != MZ_OK ) {
			delete[] p;
			return Fail(err, _T("extract failed"));
		}
		*ppData = p;
		*pdwSize = (DWORD)need;
		return Ok();
	}

	static int32_t ZipDiscardWrite(void* /*stream*/, const void* /*buf*/, int32_t size)
	{
		return size;
	}

	bool CZipFile::Test(int* pFailIndex)
	{
		if( pFailIndex ) *pFailIndex = -1;
		if( !EnsureRead() ) return false;
		if( !SyncCryptoOpts() ) return false;

		int idx = 0;
		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		while( err == MZ_OK ) {
			if( mz_zip_reader_entry_is_dir(m_reader) != MZ_OK ) {
				mz_zip_file* info = NULL;
				if( mz_zip_reader_entry_get_info(m_reader, &info) != MZ_OK || info == NULL ) {
					if( pFailIndex ) *pFailIndex = idx;
					return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
				}
				if( info->uncompressed_size < 0 ) {
					if( pFailIndex ) *pFailIndex = idx;
					return Fail(MZ_FORMAT_ERROR, _T("unknown uncompressed size"));
				}
				long unc = (info->uncompressed_size > 0x7FFFFFFF) ? 0x7FFFFFFF : (long)info->uncompressed_size;
				if( !CheckUncompressed(unc) ) {
					if( pFailIndex ) *pFailIndex = idx;
					return Fail(MZ_MEM_ERROR, _T("entry too large"));
				}
				int32_t saveErr = mz_zip_reader_entry_save(m_reader, NULL, ZipDiscardWrite);
				if( saveErr != MZ_OK ) {
					if( pFailIndex ) *pFailIndex = idx;
					return Fail(saveErr, _T("integrity check failed"));
				}
			}
			err = mz_zip_reader_goto_next_entry(m_reader);
			++idx;
		}
		return Ok();
	}

	int CZipFile::AddNewItem(void* writer, EMutate op, const CDuiString& name, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile)
	{
		if( writer == NULL ) return MZ_PARAM_ERROR;
		CDuiString entry = name;
		if( op == kAddDir ) {
			if( entry.GetLength() == 0 || entry.GetAt(entry.GetLength() - 1) != _T('/') )
				entry += _T('/');
		}
		char* nameUtf8 = DupWideToUtf8(entry.GetData());
		if( nameUtf8 == NULL ) return MZ_PARAM_ERROR;

		if( op == kAddFile || op == kUpdateFile ) {
			char* pathUtf8 = DupWideToUtf8(pszSrcFile);
			if( pathUtf8 == NULL ) {
				delete[] nameUtf8;
				return MZ_PARAM_ERROR;
			}
			int32_t err = mz_zip_writer_add_file(writer, pathUtf8, nameUtf8);
			delete[] pathUtf8;
			delete[] nameUtf8;
			return err;
		}

		mz_zip_file file_info;
		memset(&file_info, 0, sizeof(file_info));
		file_info.filename = nameUtf8;
		file_info.modified_date = time(NULL);
		file_info.version_madeby = (uint16_t)(MZ_HOST_SYSTEM_WINDOWS_NTFS << 8);
		file_info.compression_method = (m_compressLevel == 0)
			? MZ_COMPRESS_METHOD_STORE : MZ_COMPRESS_METHOD_DEFLATE;
		file_info.flag |= MZ_ZIP_FLAG_UTF8;
		// 有密码时必须带 aes_version，否则不会挂 wzaes，会静默写成明文。
		if( Pwd() != NULL )
			file_info.aes_version = MZ_AES_VERSION;
		if( op == kAddDir ) {
			file_info.external_fa = ((uint32_t)FILE_ATTRIBUTE_DIRECTORY) << 16;
			file_info.uncompressed_size = 0;
			char dummy = 0;
			// add_buffer 拒绝 NULL；目录条目不写载荷，占位即可。
			int32_t err = mz_zip_writer_add_buffer(writer, &dummy, 0, &file_info);
			delete[] nameUtf8;
			return err;
		}

		file_info.uncompressed_size = (int64_t)dwSize;
		file_info.external_fa = ((uint32_t)FILE_ATTRIBUTE_NORMAL) << 16;
		const void* buf = pData;
		char dummy = 0;
		if( dwSize == 0 ) buf = &dummy;
		int32_t err = mz_zip_writer_add_buffer(writer, buf, (int32_t)dwSize, &file_info);
		delete[] nameUtf8;
		return err;
	}

	bool CZipFile::AddInWriteMode(EMutate op, LPCTSTR pszZipName, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bReplace)
	{
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, op == kAddDir) ) return false;
		if( FindWriteName(name.GetData()) >= 0 ) {
			if( !bReplace ) return Fail(MZ_PARAM_ERROR, _T("entry already exists"));
			if( !CommitWrite() ) return false;
			return Mutate(op, name.GetData(), pData, dwSize, pszSrcFile, true, false);
		}
		if( !PreparePassword() ) return false;
		ApplyWriterOpts(m_writer);
		int err = AddNewItem(m_writer, op, name, pData, dwSize, pszSrcFile);
		if( (err == MZ_STREAM_ERROR || err == MZ_BUF_ERROR || err == MZ_MEM_ERROR) && m_path.IsEmpty() ) {
			if( m_writeNames.GetSize() > 0 ) {
				if( !CommitWrite() ) return Fail(err, _T("memory zip full"));
				return Mutate(op, name.GetData(), pData, dwSize, pszSrcFile, bReplace, false);
			}
			return Fail(err, _T("memory zip too small"));
		}
		if( err != MZ_OK ) return Fail(err, _T("add failed"));
		RememberWriteName(name.GetData());
		return Ok();
	}

	bool CZipFile::AddFile(LPCTSTR pszZipName, LPCTSTR pszSrcFile, bool bReplace)
	{
		if( pszSrcFile == NULL || *pszSrcFile == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("src file is empty"));
		CDuiString name;
		if( pszZipName && *pszZipName ) name = pszZipName;
		else name = FileNameOf(pszSrcFile);
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_writer )
			return AddInWriteMode(kAddFile, name.GetData(), NULL, 0, pszSrcFile, bReplace);
		return Mutate(kAddFile, name.GetData(), NULL, 0, pszSrcFile, bReplace, false);
	}

	bool CZipFile::AddMemory(LPCTSTR pszZipName, const void* pData, DWORD dwSize, bool bReplace)
	{
		if( pszZipName == NULL || *pszZipName == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("entry name is empty"));
		if( dwSize > 0 && pData == NULL )
			return Fail(MZ_PARAM_ERROR, _T("data is null"));
		if( !CheckMzBufSize(dwSize, _T("entry data")) ) return false;
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_writer )
			return AddInWriteMode(kAddMem, name.GetData(), pData, dwSize, NULL, bReplace);
		return Mutate(kAddMem, name.GetData(), pData, dwSize, NULL, bReplace, false);
	}

	bool CZipFile::AddFolder(LPCTSTR pszZipName)
	{
		if( pszZipName == NULL || *pszZipName == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("entry name is empty"));
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, true) ) return false;
		if( m_mode == ModeWrite && m_writer )
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
			return Fail(MZ_PARAM_ERROR, _T("src dir is empty"));
		DWORD attr = ::GetFileAttributes(pszSrcDir);
		if( attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0 )
			return Fail(MZ_PARAM_ERROR, _T("src is not a directory"));
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
			return Fail(MZ_PARAM_ERROR, _T("src file is empty"));
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_writer ) {
			if( !CommitWrite() ) return false;
		}
		return Mutate(kUpdateFile, name.GetData(), NULL, 0, pszSrcFile, true, false);
	}

	bool CZipFile::UpdateMemory(LPCTSTR pszZipName, const void* pData, DWORD dwSize)
	{
		if( dwSize > 0 && pData == NULL )
			return Fail(MZ_PARAM_ERROR, _T("data is null"));
		if( !CheckMzBufSize(dwSize, _T("entry data")) ) return false;
		CDuiString name = pszZipName;
		if( !EnsureEntryName(name, false) ) return false;
		if( m_mode == ModeWrite && m_writer ) {
			if( !CommitWrite() ) return false;
		}
		return Mutate(kUpdateMem, name.GetData(), pData, dwSize, NULL, true, false);
	}

	bool CZipFile::Remove(LPCTSTR pszName, bool bRecursive)
	{
		if( m_mode == ModeWrite && m_writer ) {
			if( !CommitWrite() ) return false;
		}
		return Mutate(kRemove, pszName, NULL, 0, NULL, false, bRecursive);
	}

	bool CZipFile::RemoveDir(LPCTSTR pszDir)
	{
		if( m_mode == ModeWrite && m_writer ) {
			if( !CommitWrite() ) return false;
		}
		if( !EnsureRead() ) return false;
		CDuiString dir = NormalizeDirName(pszDir);
		if( dir.IsEmpty() ) return Fail(MZ_PARAM_ERROR, _T("directory name is empty"));

		bool bDirEntry = false, bChildren = false, bFileExact = false;
		if( !ClassifyDir(dir.GetData(), bDirEntry, bChildren, bFileExact) ) return false;
		if( !bDirEntry && !bChildren ) {
			if( bFileExact ) return Fail(MZ_PARAM_ERROR, _T("not a directory"));
			return Fail(MZ_EXIST_ERROR, _T("directory not found"));
		}
		return Mutate(kRemove, dir.GetData(), NULL, 0, NULL, false, true);
	}

	bool CZipFile::Mutate(EMutate op, LPCTSTR pszZipName, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bReplace, bool bRecursive)
	{
		if( !EnsureRead() ) return false;
		CDuiString target = (op == kAddDir) ? NormalizeDirName(pszZipName) : NormalizeZipName(pszZipName);
		if( target.IsEmpty() ) return Fail(MZ_PARAM_ERROR, _T("entry name is empty"));
		if( op != kRemove && !IsSafeZipName(target.GetData()) )
			return Fail(MZ_PARAM_ERROR, _T("unsafe zip entry path"));

		bool found = false;
		int32_t err = mz_zip_reader_goto_first_entry(m_reader);
		while( err == MZ_OK ) {
			mz_zip_file* info = NULL;
			if( mz_zip_reader_entry_get_info(m_reader, &info) == MZ_OK && info && info->filename ) {
				CDuiString item = NormalizeZipName(Utf8ToWide(info->filename).GetData());
				if( NameMatch(item, target, bRecursive && op == kRemove) ) {
					found = true;
					break;
				}
			}
			err = mz_zip_reader_goto_next_entry(m_reader);
		}

		if( op == kRemove ) {
			if( !found ) return Fail(MZ_EXIST_ERROR, _T("entry not found"));
		}
		else if( op == kUpdateFile || op == kUpdateMem ) {
			if( !found ) return Fail(MZ_EXIST_ERROR, _T("entry not found"));
		}
		else {
			if( found && !bReplace ) return Fail(MZ_PARAM_ERROR, _T("entry already exists"));
		}

		return Rewrite(op, target, pData, dwSize, pszSrcFile, bRecursive);
	}

	bool CZipFile::Rewrite(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive)
	{
		if( !m_path.IsEmpty() )
			return RewriteToFile(op, target, pData, dwSize, pszSrcFile, bRecursive);
		return RewriteToMemory(op, target, pData, dwSize, pszSrcFile, bRecursive);
	}

	static bool ShouldSkipEntry(bool bRemove, const CDuiString& item, const CDuiString& target, bool bRecursive)
	{
		bool match = NameMatch(item, target, bRecursive && bRemove);
		if( !match && !bRemove )
			match = (item.CompareNoCase(target.GetData()) == 0);
		return match;
	}

	bool CZipFile::RewriteToFile(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive)
	{
		if( !PreparePassword() ) return false;
		CDuiString tmp;
		if( !MakeTempZipPath(m_path.GetData(), tmp) )
			return Fail(MZ_OPEN_ERROR, _T("create temp zip failed"));

		char* tmpUtf8 = DupWideToUtf8(tmp.GetData());
		if( tmpUtf8 == NULL )
			return Fail(MZ_PARAM_ERROR, _T("path utf8 conversion failed"));

		void* writer = mz_zip_writer_create();
		if( writer == NULL ) {
			delete[] tmpUtf8;
			return Fail(MZ_MEM_ERROR, _T("create zip writer failed"));
		}
		ApplyWriterOpts(writer);
		int32_t err = mz_zip_writer_open_file(writer, tmpUtf8, 0, 0);
		delete[] tmpUtf8;
		if( err != MZ_OK ) {
			mz_zip_writer_delete(&writer);
			::DeleteFile(tmp.GetData());
			return Fail(err, _T("create temp zip failed"));
		}

		int nCopied = 0;
		int nTotal = 0;
		const bool bRemove = (op == kRemove);
		err = mz_zip_reader_goto_first_entry(m_reader);
		while( err == MZ_OK ) {
			++nTotal;
			mz_zip_file* info = NULL;
			if( mz_zip_reader_entry_get_info(m_reader, &info) != MZ_OK || info == NULL || info->filename == NULL ) {
				mz_zip_writer_delete(&writer);
				::DeleteFile(tmp.GetData());
				return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
			}
			CDuiString item = NormalizeZipName(Utf8ToWide(info->filename).GetData());
			if( ShouldSkipEntry(bRemove, item, target, bRecursive) ) {
				err = mz_zip_reader_goto_next_entry(m_reader);
				continue;
			}
			int32_t copyErr = mz_zip_writer_copy_from_reader(writer, m_reader);
			if( copyErr != MZ_OK ) {
				mz_zip_writer_delete(&writer);
				::DeleteFile(tmp.GetData());
				return Fail(copyErr, _T("copy entry failed"));
			}
			++nCopied;
			err = mz_zip_reader_goto_next_entry(m_reader);
		}

		if( op != kRemove ) {
			int addErr = AddNewItem(writer, op, target, pData, dwSize, pszSrcFile);
			if( addErr != MZ_OK ) {
				mz_zip_writer_delete(&writer);
				::DeleteFile(tmp.GetData());
				return Fail(addErr, _T("add entry failed"));
			}
		}
		else if( nCopied == nTotal ) {
			mz_zip_writer_delete(&writer);
			::DeleteFile(tmp.GetData());
			return Fail(MZ_EXIST_ERROR, _T("entry not found"));
		}

		err = mz_zip_writer_close(writer);
		mz_zip_writer_delete(&writer);
		if( err != MZ_OK ) {
			::DeleteFile(tmp.GetData());
			return Fail(err, _T("close temp zip failed"));
		}

		DestroyReader();
		m_mode = ModeNone;
		if( !ReplaceZipAtomically(m_path.GetData(), tmp.GetData()) ) {
			::DeleteFile(tmp.GetData());
			ReopenRead();
			return Fail(MZ_OPEN_ERROR, _T("replace zip file failed"));
		}
		return ReopenRead();
	}

	bool CZipFile::RewriteToMemory(EMutate op, const CDuiString& target, const void* pData, DWORD dwSize, LPCTSTR pszSrcFile, bool bRecursive)
	{
		if( !PreparePassword() ) return false;

		void* mem = mz_stream_mem_create();
		if( mem == NULL )
			return Fail(MZ_MEM_ERROR, _T("create memory stream failed"));
		mz_stream_mem_set_grow_size(mem, 256 * 1024);
		int32_t err = mz_stream_open(mem, NULL, MZ_OPEN_MODE_CREATE);
		if( err != MZ_OK ) {
			mz_stream_mem_delete(&mem);
			return Fail(err, _T("open memory stream failed"));
		}

		void* writer = mz_zip_writer_create();
		if( writer == NULL ) {
			mz_stream_mem_delete(&mem);
			return Fail(MZ_MEM_ERROR, _T("create zip writer failed"));
		}
		ApplyWriterOpts(writer);
		err = mz_zip_writer_open(writer, mem, 0);
		if( err != MZ_OK ) {
			mz_zip_writer_delete(&writer);
			mz_stream_mem_delete(&mem);
			return Fail(err, _T("create memory zip failed"));
		}

		bool skipped = false;
		const bool bRemove = (op == kRemove);
		err = mz_zip_reader_goto_first_entry(m_reader);
		while( err == MZ_OK ) {
			mz_zip_file* info = NULL;
			if( mz_zip_reader_entry_get_info(m_reader, &info) != MZ_OK || info == NULL || info->filename == NULL ) {
				mz_zip_writer_delete(&writer);
				mz_stream_mem_delete(&mem);
				return Fail(MZ_INTERNAL_ERROR, _T("get entry info failed"));
			}
			CDuiString item = NormalizeZipName(Utf8ToWide(info->filename).GetData());
			if( ShouldSkipEntry(bRemove, item, target, bRecursive) ) {
				skipped = true;
				err = mz_zip_reader_goto_next_entry(m_reader);
				continue;
			}
			int32_t copyErr = mz_zip_writer_copy_from_reader(writer, m_reader);
			if( copyErr != MZ_OK ) {
				mz_zip_writer_delete(&writer);
				mz_stream_mem_delete(&mem);
				return Fail(copyErr, _T("copy entry failed"));
			}
			err = mz_zip_reader_goto_next_entry(m_reader);
		}

		if( op != kRemove ) {
			int addErr = AddNewItem(writer, op, target, pData, dwSize, pszSrcFile);
			if( addErr != MZ_OK ) {
				mz_zip_writer_delete(&writer);
				mz_stream_mem_delete(&mem);
				return Fail(addErr, _T("add entry failed"));
			}
		}
		else if( !skipped ) {
			mz_zip_writer_delete(&writer);
			mz_stream_mem_delete(&mem);
			return Fail(MZ_EXIST_ERROR, _T("entry not found"));
		}

		err = mz_zip_writer_close(writer);
		if( err != MZ_OK ) {
			mz_zip_writer_delete(&writer);
			mz_stream_mem_delete(&mem);
			return Fail(err, _T("close memory zip failed"));
		}

		const void* pOut = NULL;
		int32_t nOut = 0;
		if( mz_stream_mem_get_buffer(mem, &pOut) != MZ_OK ) {
			mz_zip_writer_delete(&writer);
			mz_stream_mem_delete(&mem);
			return Fail(MZ_INTERNAL_ERROR, _T("get memory zip buffer failed"));
		}
		mz_stream_mem_get_buffer_length(mem, &nOut);
		BYTE* pCopy = NULL;
		if( nOut > 0 && pOut != NULL ) {
			pCopy = new BYTE[nOut];
			memcpy(pCopy, pOut, (size_t)nOut);
		}
		mz_zip_writer_delete(&writer);
		mz_stream_mem_delete(&mem);

		if( pCopy == NULL || nOut <= 0 ) {
			delete[] pCopy;
			return Fail(MZ_FORMAT_ERROR, _T("empty memory zip"));
		}
		// 先校验新缓冲再替换；失败时保留原 reader/memBuf。
		return InstallOwnedMemory(pCopy, (DWORD)nOut);
	}

	bool CZipFile::SaveAs(LPCTSTR pszZipPath)
	{
		if( pszZipPath == NULL || *pszZipPath == _T('\0') )
			return Fail(MZ_PARAM_ERROR, _T("zip path is empty"));
		if( !EnsureRead() ) return false;
		if( !m_path.IsEmpty() ) {
			if( m_path.CompareNoCase(pszZipPath) == 0 ) return Ok();
			if( !::CopyFile(m_path.GetData(), pszZipPath, FALSE) )
				return Fail(MZ_OPEN_ERROR, _T("copy zip failed"));
			return Ok();
		}
		if( m_memBuf == NULL || m_memLen == 0 )
			return Fail(MZ_PARAM_ERROR, _T("memory zip is empty"));
		if( !WriteWholeFile(pszZipPath, m_memBuf, m_memLen) )
			return Fail(MZ_WRITE_ERROR, _T("write zip failed"));
		return Ok();
	}

	bool CZipFile::GetMemory(BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData ) *ppData = NULL;
		if( pdwSize ) *pdwSize = 0;
		if( ppData == NULL || pdwSize == NULL )
			return Fail(MZ_PARAM_ERROR, _T("output buffer is null"));
		if( !EnsureRead() ) return false;
		if( m_memBuf != NULL && m_memLen > 0 ) {
			BYTE* p = new BYTE[m_memLen];
			memcpy(p, m_memBuf, m_memLen);
			*ppData = p;
			*pdwSize = m_memLen;
			return Ok();
		}
		if( m_path.IsEmpty() ) return Fail(MZ_PARAM_ERROR, _T("zip not open"));
		if( !ReadWholeFile(m_path.GetData(), ppData, pdwSize) )
			return Fail(MZ_READ_ERROR, _T("read zip file failed"));
		return Ok();
	}

	DWORD CZipFile::GetMemorySize()
	{
		if( !EnsureRead() ) return 0;
		if( m_memBuf != NULL )
			return m_memLen;
		if( !m_path.IsEmpty() ) {
			HANDLE h = ::CreateFile(m_path.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if( h == INVALID_HANDLE_VALUE ) {
				Fail(MZ_OPEN_ERROR, _T("open zip file failed"));
				return 0;
			}
			DWORD n = ::GetFileSize(h, NULL);
			::CloseHandle(h);
			if( n == INVALID_FILE_SIZE ) {
				Fail(MZ_READ_ERROR, _T("get zip size failed"));
				return 0;
			}
			Ok();
			return n;
		}
		Fail(MZ_PARAM_ERROR, _T("zip not open"));
		return 0;
	}

	bool CZipFile::DetachMemory(BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData ) *ppData = NULL;
		if( pdwSize ) *pdwSize = 0;
		if( ppData == NULL || pdwSize == NULL )
			return Fail(MZ_PARAM_ERROR, _T("output buffer is null"));
		if( !EnsureRead() ) return false;
		if( m_memBuf == NULL || m_memLen == 0 )
			return Fail(MZ_PARAM_ERROR, _T("not a memory zip"));
		if( !m_bMemOwned )
			return Fail(MZ_PARAM_ERROR, _T("buffer not owned; use GetMemory to copy"));
		DestroyReader();
		*ppData = m_memBuf;
		*pdwSize = m_memLen;
		m_memBuf = NULL;
		m_memLen = 0;
		m_bMemOwned = true;
		m_mode = ModeNone;
		m_path.Empty();
		InvalidateEntryCount();
		ClearWriteNames();
		return Ok();
	}

	int CZipFile::GetLastResult() const
	{
		return m_last;
	}

	CDuiString CZipFile::GetErrorMessage() const
	{
		if( !m_errText.IsEmpty() ) return m_errText;
		return CDuiString(MzErrorText(m_last));
	}

	bool CZipFile::UnzipToDirectory(LPCTSTR pszZipPath, LPCTSTR pszDestDir, LPCTSTR pszPassword)
	{
		CZipFile zip;
		zip.SetPassword(pszPassword);
		if( !zip.Open(pszZipPath) ) return false;
		return zip.ExtractAll(pszDestDir);
	}

	bool CZipFile::UnzipMemoryToDirectory(const void* pData, unsigned int nLen, LPCTSTR pszDestDir, LPCTSTR pszPassword)
	{
		CZipFile zip;
		zip.SetPassword(pszPassword);
		if( !zip.OpenMemory(pData, nLen, true) ) return false;
		return zip.ExtractAll(pszDestDir);
	}
}
