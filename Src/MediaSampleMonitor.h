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

#define SHOW_DBG_WND

extern const AMOVIESETUP_FILTER sudMediaSampleMonitor;

class CMediaSampleMonitor : public CTransInPlaceFilter
{
private:
#ifdef SHOW_DBG_WND
	long m_nFrameCount;
#endif

public:
	DECLARE_IUNKNOWN;
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	CMediaSampleMonitor(LPUNKNOWN punk, HRESULT* phr);
	virtual ~CMediaSampleMonitor();

	HRESULT CheckInputType(const CMediaType *mtIn);

	HRESULT Transform(IMediaSample *pSample);
	
	HRESULT CompleteConnect(PIN_DIRECTION direction, IPin *pReceivePin);

	STDMETHODIMP Stop();
	STDMETHODIMP Pause();

protected:
#ifdef SHOW_DBG_WND
	CDbgWnd m_dbgWnd;
#endif
};
