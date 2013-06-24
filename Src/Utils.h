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

#include "DbgWnd.h"
#include "Utils.h"

#define sign(n)		((n) < 0 ? -1 : 1)

void DbgWndDisplay(CDbgWnd* pWnd, CBaseFilter* pFilter,
				IMediaSample* pSample, long nFrame, REFERENCE_TIME rtStart);

HRESULT GetMediaTypeName(LPTSTR pszString, int cchBuf, const GUID *pGUID);
HRESULT GetMediaSubTypeName(LPTSTR pszString, int cchBuf, const GUID *pGUID);
HRESULT GetFormatName(LPTSTR pszString, int cchBuf, const GUID *pGUID);
int GetBmpBits(const GUID* pSubtype);

#ifdef DEBUG

void DbgLogMediaTypeInfo(DWORD type, DWORD level, const CMediaType* pmt);
void DbgLogMediaFormatInfo(DWORD type, DWORD level, const CMediaType* pmt);
#define DbgLogMediaType(_x_)	DbgLogMediaTypeInfo _x_
#define	DbgLogMediaFormat(_x_)	DbgLogMediaFormatInfo _x_

#else

#define DbgLogMediaType(_x_)	0
#define	DbgLogMediaFormat(_x_)	0

#endif
