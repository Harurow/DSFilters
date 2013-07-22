/* The MIT License (MIT)
 * 
 * Copyright (c) 2013 Motoharu Tsubaki.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <streams.h>
#include <olectl.h>

#include "DbgWnd.h"
#include <cstdarg>

class CWndHandle
{
private:
	HWND m_hWnd;
	CDbgWnd* m_pDbgWnd;
	CWndHandle*	m_pPrev;
	CWndHandle*	m_pNext;

	CWndHandle(CDbgWnd * pWnd) {
		m_pDbgWnd = pWnd;
		m_hWnd = pWnd->m_hWnd;
		m_pPrev = NULL;
		m_pNext = NULL;
	}

public:
	HWND		GetWnd() { return m_hWnd; }
	CDbgWnd*	GetDbgWnd() { return m_pDbgWnd; }
	CWndHandle*	Next() { return m_pNext; }

private:
	friend class CWndList;
};


class CWndList
{
private:
	CCritSec	m_csLock;
	CWndHandle* m_pHead;
	CWndHandle* m_pTail;

public:
	CWndList() {
		m_pHead = NULL;
		m_pTail = NULL;
	}

	CCritSec* GetLock() { return &m_csLock; }

	void Add(CDbgWnd * pDbgWnd)
	{
		CWndHandle* pWnd = new CWndHandle(pDbgWnd);
		if (m_pTail == NULL) {
			ASSERT(m_pHead == NULL);
			m_pHead = pWnd;
			m_pTail = pWnd;
		} else {
			m_pTail->m_pNext = pWnd;
			pWnd->m_pPrev = m_pTail;
			m_pTail = pWnd;
		}
	}

	void Remove(HWND hWnd)
	{
		CWndHandle* pWnd = GetWnd(hWnd);
		if (pWnd) {
			if (m_pHead == pWnd) {
				m_pHead = pWnd->m_pNext;
			} 
			if (m_pTail == pWnd) {
				m_pTail = pWnd->m_pPrev;
			}
			if (pWnd->m_pPrev) {
				pWnd->m_pPrev->m_pNext = pWnd->m_pNext;
			}
			if (pWnd->m_pNext) {
				pWnd->m_pNext->m_pPrev = pWnd->m_pPrev;
			}
			delete pWnd;
		}
	}

	CWndHandle* GetWnd(HWND hWnd)
	{
		CWndHandle* pWnd = m_pHead;
		while (pWnd) {
			if (pWnd->m_hWnd == hWnd) {
				return pWnd;
			}
			pWnd = pWnd->m_pNext;
		}
		return NULL;
	}

	CWndHandle* GetHead() { return m_pHead; }
};


static CWndList* g_pList = new CWndList;


extern "C" LRESULT CALLBACK DebugWndProc( HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam )
{
	if (message == WM_CLOSE) {
		CAutoLock lock2(g_pList->GetLock());
		CWndHandle* pWnd = g_pList->GetWnd(hWnd);
		if (pWnd) {
			CDbgWnd* pDbgWnd = pWnd->GetDbgWnd();

			CAutoLock lock(&pDbgWnd->m_csLock);
			if (pDbgWnd->m_hWnd != NULL) {
				pDbgWnd->m_hWnd = NULL;
			}

			g_pList->Remove(hWnd);
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


CDbgWnd::CDbgWnd()
 : m_hWnd(NULL)
{
	SetDefaultTextColor();
}


CDbgWnd::~CDbgWnd() 
{
	Destroy();
}


void CDbgWnd::Create(CBasePin* pPin)
{
	PIN_INFO pinInfo;
	pPin->QueryPinInfo(&pinInfo);

	FILTER_INFO info;
	pinInfo.pFilter->QueryFilterInfo(&info);

	Create(info.achName);
}


void CDbgWnd::Create(IBaseFilter* pFilter)
{
	FILTER_INFO info;
	pFilter->QueryFilterInfo(&info);

	Create(info.achName);
}


void CDbgWnd::Create(LPCTSTR lpszCaption)
{
	const int width = 360;
	const int height = 180;

	{
		CAutoLock lock(&m_csLock);

		if (!::IsWindow(m_hWnd)) {
			WNDCLASSEX wcex		= { sizeof(WNDCLASSEX) };
			wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcex.lpfnWndProc	= (WNDPROC)DebugWndProc;
			wcex.cbClsExtra		= 0;
			wcex.cbWndExtra		= 0;
			wcex.hInstance		= GetModuleHandle( NULL );
			wcex.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
			wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
			wcex.hbrBackground	= (HBRUSH)CreateSolidBrush(0);
			wcex.lpszClassName	= TEXT("DSFilterDebugWndClass");
			ATOM a = RegisterClassEx( &wcex );

			m_hWnd = CreateWindowEx(
						WS_EX_OVERLAPPEDWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
						TEXT("DSFilterDebugWndClass"),
						lpszCaption,
						WS_OVERLAPPEDWINDOW,
						0,
						0,
						width,
						height,
						NULL,
						NULL,
						NULL,
						NULL);

			::SetLayeredWindowAttributes(m_hWnd, 0, 192, LWA_ALPHA);
			::UpdateWindow(m_hWnd);

			CAutoLock lock2(g_pList->GetLock());
			g_pList->Add(this);
		}
	}
	
	// ‡”Ô‚É•À‚×‚é
	int sc_w = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int sc_h = ::GetSystemMetrics(SM_CYFULLSCREEN);
	int x = 0, y = sc_h - height;

	CWndHandle* pWnd = g_pList->GetHead();
	while (pWnd) {
		HWND hWnd = pWnd->GetWnd();

		if (::IsWindow(hWnd)) {
			::MoveWindow(hWnd, x, y, width, height, TRUE);
#ifndef HIDE_DBGWND
			::ShowWindow(hWnd, SW_SHOW);
#endif
			::UpdateWindow(hWnd);

			x += width;
			if (sc_w - width < x) {
				x = 0;
				y -= height;
				if (y < 0) {
					y = sc_h - height;
				}
			}
		}

		pWnd = pWnd->Next();
	}
}


void CDbgWnd::Destroy()
{
	CAutoLock lock(&m_csLock);

	if (m_hWnd) {
		HWND hWnd = m_hWnd;
		m_hWnd = NULL;
		::DestroyWindow(hWnd);

		CAutoLock lock2(g_pList->GetLock());
		g_pList->Remove(hWnd);
	}
}


void CDbgWnd::Clear()
{
	if (::IsWindow(m_hWnd)) {
		::InvalidateRect(m_hWnd, NULL, TRUE);
	}
}


void CDbgWnd::DrawString(int x, int y, const TCHAR * format, ...)
{
	if (::IsWindow(m_hWnd)) {
		HDC hdc = ::GetDC(m_hWnd);
		
		TCHAR buf[256];
		::memset(buf, 0, sizeof(buf));

		va_list args;
		va_start(args, format);
		int size = _vsntprintf_s(buf, 256, _TRUNCATE, format, args);

		::SelectObject(hdc, GetStockFont(ANSI_FIXED_FONT));
		::SetTextColor(hdc, m_rgbText);
		::SetBkColor(hdc, RGB(0,0,0));
		::TextOut(hdc, x, y, buf, (int)_tcsclen(buf));

		::ReleaseDC(m_hWnd, hdc);
	}
}
