#include "StdAfx.h"
#include "UIHotKeyBinder.h"
#include "UIHotKey.h"
#include "UIHotKeyBox.h"
#include "UIButton.h"
#include <commctrl.h>

namespace DuiLib {

namespace {
	const UINT_PTR kHotKeySubclassIdBase = 0x484B0001; // 'HK\0\1'
}

CHotKeyBinder::CHotKeyBinder()
	: m_pManager(NULL)
	, m_pfnHandler(NULL)
	, m_pUser(NULL)
	, m_bRequireModifier(true)
{
}

CHotKeyBinder::~CHotKeyBinder()
{
	Detach();
}

void CHotKeyBinder::SetHandler(LPHotKeyHandler fn, LPVOID pUser)
{
	m_pfnHandler = fn;
	m_pUser = pUser;
}

void CHotKeyBinder::SetRequireModifier(bool require)
{
	m_bRequireModifier = require;
}

bool CHotKeyBinder::Attach(CPaintManagerUI* pm)
{
	if( pm == NULL ) return false;
	if( m_pManager == pm ) return true;
	Detach();
	m_pManager = pm;
	m_pManager->AddPreMessageFilter(this);
	HWND hWnd = m_pManager->GetPaintWindow();
	if( hWnd != NULL )
		EnsureSubclass(hWnd);
	return true;
}

void CHotKeyBinder::Detach()
{
	UnbindAll();
	if( m_pManager != NULL ) {
		m_pManager->RemovePreMessageFilter(this);
		m_pManager = NULL;
	}
	while( m_aSubclass.GetSize() > 0 ) {
		HWND h = (HWND)m_aSubclass.GetAt(0);
		ReleaseSubclass(h);
	}
}

bool CHotKeyBinder::IsModifierVk(UINT vk)
{
	switch( vk ) {
	case VK_SHIFT: case VK_CONTROL: case VK_MENU:
	case VK_LSHIFT: case VK_RSHIFT:
	case VK_LCONTROL: case VK_RCONTROL:
	case VK_LMENU: case VK_RMENU:
	case VK_LWIN: case VK_RWIN:
		return true;
	default:
		return false;
	}
}

void CHotKeyBinder::EnsureSubclass(HWND hWnd)
{
	if( hWnd == NULL || !::IsWindow(hWnd) ) return;
	for( int i = 0; i < m_aSubclass.GetSize(); ++i ) {
		if( (HWND)m_aSubclass.GetAt(i) == hWnd )
			return;
	}
	if( ::SetWindowSubclass(hWnd, SubclassProc, kHotKeySubclassIdBase, (DWORD_PTR)this) )
		m_aSubclass.Add((LPVOID)hWnd);
}

void CHotKeyBinder::ReleaseSubclass(HWND hWnd)
{
	if( hWnd == NULL ) return;
	for( int i = 0; i < m_aSubclass.GetSize(); ++i ) {
		if( (HWND)m_aSubclass.GetAt(i) == hWnd ) {
			if( ::IsWindow(hWnd) )
				::RemoveWindowSubclass(hWnd, SubclassProc, kHotKeySubclassIdBase);
			m_aSubclass.Remove(i);
			return;
		}
	}
}

LRESULT CALLBACK CHotKeyBinder::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR /*uIdSubclass*/, DWORD_PTR dwRefData)
{
	CHotKeyBinder* pThis = (CHotKeyBinder*)dwRefData;
	if( pThis != NULL ) {
		if( uMsg == WM_HOTKEY ) {
			if( pThis->HandleHotKeyMessage(wParam, lParam) )
				return 0;
		}
		else if( uMsg == WM_NCDESTROY ) {
			// 窗体销毁：卸掉本窗全局键与子类化
			for( int i = pThis->m_aItems.GetSize() - 1; i >= 0; --i ) {
				Item* p = (Item*)pThis->m_aItems.GetAt(i);
				if( p != NULL && p->hWnd == hWnd ) {
					pThis->UnregisterGlobal(*p);
					delete p;
					pThis->m_aItems.Remove(i);
				}
			}
			pThis->ReleaseSubclass(hWnd);
		}
	}
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

int CHotKeyBinder::FindIndexById(int id) const
{
	for( int i = 0; i < m_aItems.GetSize(); ++i ) {
		Item* p = (Item*)m_aItems.GetAt(i);
		if( p != NULL && p->id == id )
			return i;
	}
	return -1;
}

bool CHotKeyBinder::UnregisterGlobal(Item& item)
{
	if( !item.bGlobalOk || item.hWnd == NULL ) {
		item.bGlobalOk = false;
		return true;
	}
	BOOL ok = ::UnregisterHotKey(item.hWnd, item.id);
	item.bGlobalOk = false;
	return ok != FALSE;
}

void CHotKeyBinder::Fire(const Item& item)
{
	if( m_pfnHandler != NULL )
		m_pfnHandler(item.id, item.vk, item.mod, item.scope, m_pUser);
}

bool CHotKeyBinder::Bind(HWND hWnd, int id, WORD vk, WORD mod, int scope)
{
	if( hWnd == NULL || !::IsWindow(hWnd) ) return false;
	if( vk == 0 && CHotKeyUI::HotKeyCompareMask(mod) == 0 ) return false;
	if( m_bRequireModifier && CHotKeyUI::IsBareLetterOrDigit(vk, mod) ) return false;

	scope = (scope == HOTKEYBOX_SCOPE_GLOBAL) ? HOTKEYBOX_SCOPE_GLOBAL : HOTKEYBOX_SCOPE_APP;

	// 同 id 先解绑
	Unbind(id);

	// 同组合键已存在则失败
	if( Find(vk, mod) ) return false;

	Item* p = new Item;
	p->id = id;
	p->hWnd = hWnd;
	p->vk = vk;
	p->mod = mod;
	p->scope = scope;
	p->bGlobalOk = false;

	if( scope == HOTKEYBOX_SCOPE_GLOBAL ) {
		UINT fs = CHotKeyUI::HotKeyToRegisterMods(mod, true);
		if( !::RegisterHotKey(hWnd, id, fs, vk) ) {
			delete p;
			return false;
		}
		p->bGlobalOk = true;
		EnsureSubclass(hWnd);
	}
	else {
		// 程序快捷键依赖 Attach 的 PreMessageFilter；仍子类化以便统一生命周期
		EnsureSubclass(hWnd);
		if( m_pManager == NULL ) {
			// 未 Attach 时尝试挂到该 HWND 的 PM
			CStdPtrArray* a = CPaintManagerUI::GetPaintManagers();
			if( a != NULL ) {
				for( int i = 0; i < a->GetSize(); ++i ) {
					CPaintManagerUI* pm = static_cast<CPaintManagerUI*>(a->GetAt(i));
					if( pm != NULL && pm->GetPaintWindow() == hWnd ) {
						Attach(pm);
						break;
					}
				}
			}
		}
	}

	m_aItems.Add(p);
	return true;
}

bool CHotKeyBinder::BindButton(CButtonUI* pButton, int id)
{
	if( pButton == NULL || !pButton->HasShortcutKey() ) return false;
	CPaintManagerUI* pm = pButton->GetManager();
	HWND hWnd = (pm != NULL) ? pm->GetPaintWindow() : NULL;
	if( hWnd == NULL ) return false;
	WORD vk = 0, mod = 0;
	int scope = HOTKEYBOX_SCOPE_APP;
	pButton->GetShortcutKey(vk, mod, scope);
	if( m_pManager == NULL && pm != NULL )
		Attach(pm);
	return Bind(hWnd, id, vk, mod, scope);
}

bool CHotKeyBinder::Unbind(int id)
{
	int idx = FindIndexById(id);
	if( idx < 0 ) return false;
	Item* p = (Item*)m_aItems.GetAt(idx);
	if( p != NULL ) {
		UnregisterGlobal(*p);
		delete p;
	}
	m_aItems.Remove(idx);
	return true;
}

void CHotKeyBinder::UnbindAll()
{
	for( int i = 0; i < m_aItems.GetSize(); ++i ) {
		Item* p = (Item*)m_aItems.GetAt(i);
		if( p != NULL ) {
			UnregisterGlobal(*p);
			delete p;
		}
	}
	m_aItems.Empty();
}

bool CHotKeyBinder::IsBound(int id) const
{
	return FindIndexById(id) >= 0;
}

bool CHotKeyBinder::Find(WORD vk, WORD mod, int* pId, int* pScope) const
{
	for( int i = 0; i < m_aItems.GetSize(); ++i ) {
		Item* p = (Item*)m_aItems.GetAt(i);
		if( p != NULL && CHotKeyUI::IsSameHotKey(vk, mod, p->vk, p->mod) ) {
			if( pId != NULL ) *pId = p->id;
			if( pScope != NULL ) *pScope = p->scope;
			return true;
		}
	}
	return false;
}

bool CHotKeyBinder::HandleHotKeyMessage(WPARAM wParam, LPARAM /*lParam*/)
{
	int id = (int)wParam;
	int idx = FindIndexById(id);
	if( idx < 0 ) return false;
	Item* p = (Item*)m_aItems.GetAt(idx);
	if( p == NULL || p->scope != HOTKEYBOX_SCOPE_GLOBAL ) return false;
	Fire(*p);
	return true;
}

LRESULT CHotKeyBinder::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if( uMsg != WM_KEYDOWN && uMsg != WM_SYSKEYDOWN )
		return 0;
	if( IsModifierVk((UINT)wParam) )
		return 0;
	// 忽略自动重复
	if( (lParam & (1L << 30)) != 0 )
		return 0;

	WORD vk = (WORD)wParam;
	WORD mod = 0;
	if( ::GetKeyState(VK_CONTROL) < 0 ) mod |= HOTKEYF_CONTROL;
	if( ::GetKeyState(VK_SHIFT) < 0 ) mod |= HOTKEYF_SHIFT;
	if( ::GetKeyState(VK_MENU) < 0 ) mod |= HOTKEYF_ALT;
	if( ::GetKeyState(VK_LWIN) < 0 || ::GetKeyState(VK_RWIN) < 0 ) mod |= HOTKEYF_WIN;
	if( (lParam & (1L << 24)) != 0 ) mod |= HOTKEYF_EXT;

	for( int i = 0; i < m_aItems.GetSize(); ++i ) {
		Item* p = (Item*)m_aItems.GetAt(i);
		if( p == NULL || p->scope != HOTKEYBOX_SCOPE_APP ) continue;
		if( !CHotKeyUI::IsSameHotKey(vk, mod, p->vk, p->mod) ) continue;
		Fire(*p);
		bHandled = true;
		return 0;
	}
	return 0;
}

} // namespace DuiLib
