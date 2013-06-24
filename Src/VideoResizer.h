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

class CVideoResizeBase;

extern const AMOVIESETUP_FILTER sudVideoResizer;

class CVideoResizer : public CTransformFilter
{
#ifdef _DEBUG
	long m_nFrameCount;
#endif
private:
	// resize config
	int const DEST_WIDTH;
	int const DEST_HEIGHT;
	
public:
	DECLARE_IUNKNOWN;
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	CVideoResizer(LPUNKNOWN punk);
	virtual ~CVideoResizer();

	HRESULT CheckInputType(const CMediaType *mtIn);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
	
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pAp);
	HRESULT Transform(IMediaSample *pSource, IMediaSample *pDest);

	HRESULT CompleteConnect(PIN_DIRECTION direction, IPin *pReceivePin);

	STDMETHODIMP Stop();
	STDMETHODIMP Pause();

protected:
	CVideoResizeBase* m_pResizer;
	int m_nSrcWidth;
	int m_nSrcHeight;

	DECLARE_DBGWND;
};
