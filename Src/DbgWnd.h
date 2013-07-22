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

#pragma once

#include <stdio.h>
#include <tchar.h>

class CDbgWnd
{
public:
	CCritSec	m_csLock;
	HWND		m_hWnd;
	COLORREF	m_rgbText;

public:
	CDbgWnd();
	~CDbgWnd();
	
	void Create(CBasePin* pPin);
	void Create(IBaseFilter* pFilter);
	void Create(LPCTSTR lpszCaption);
	void Destroy();

	COLORREF GetTextColor() { return m_rgbText; }
	void SetTextColor(COLORREF rgb) { m_rgbText = rgb; }
	void SetDefaultTextColor() { m_rgbText = RGB(0, 255, 0); }
	
	void Clear();
	void DrawString(int x, int y, const TCHAR * format, ...);
};


#if defined(_DEBUG)

#define _WND							m_dbgWnd
#define DECLARE_DBGWND					CDbgWnd _WND
#define DBGWND_CREATE					_WND.Create(this)
#define DBGWND_DESTROY					_WND.Destroy()
#define DBGWND_TEXT(x,y,t)				_WND.DrawString(x,y,t)
#define DBGWND_TEXT0(x,y,t)				_WND.DrawString(x,y,t)
#define DBGWND_TEXT1(x,y,t,a1)			_WND.DrawString(x,y,t,a1)
#define DBGWND_TEXT2(x,y,t,a1,a2)		_WND.DrawString(x,y,t,a1,a2)
#define DBGWND_TEXT3(x,y,t,a1,a2,a3)	_WND.DrawString(x,y,t,a1,a2,a3)
#define DBGWND_TEXT4(x,y,t,a1,a2,a3,a4)	_WND.DrawString(x,y,t,a1,a2,a3,a4)
#define DBGWND_RGB(r,g,b)				_WND.SetTextColor(RGB(r,g,b))
#define DBGWND_RESET_RGB				_WND.SetDefaultTextColor()
#define DBGWND_CLEAR					_WND.Clear()
#define DBGWND							_WND
#undef _WND

#else

#define DECLARE_DBGWND
#define DBGWND_CREATE
#define DBGWND_DESTROY
#define DBGWND_TEXT(x, y, t)
#define DBGWND_TEXT0(x, y, t)
#define DBGWND_TEXT1(x, y, t, a1)
#define DBGWND_TEXT2(x, y, t, a1, a2)
#define DBGWND_TEXT3(x, y, t, a1, a2, a3)
#define DBGWND_TEXT4(x, y, t, a1, a2, a3, a4)
#define DBGWND_RGB(r,g,b)
#define DBGWND_RESET_RGB
#define DBGWND_CLEAR
#define DBGWND

#endif
