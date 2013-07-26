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
#include "BaseMux.h"


extern const AMOVIESETUP_FILTER sudAudioMux;

class CRingBuffer;

class CAudioMux : public CBaseMux
{
	WORD	m_nChannels;
	DWORD	m_nSamplesPerSec;
	WORD	m_nBitsPerSample;
	WORD	m_nBlockAlign;
	LONGLONG m_nMasterMediaTime;
	LONGLONG m_nSlaveMediaTime;
	
	CRingBuffer* m_pBuf;
	
	
public:
	DECLARE_IUNKNOWN;
	static CUnknown* WINAPI CreateInstance(LPUNKNOWN punk, HRESULT* phr);

protected:
	CAudioMux(LPUNKNOWN punk, HRESULT* phr);
	~CAudioMux();

public:
	HRESULT CheckInputType(const CMediaType *mtIn);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
	HRESULT Transform(IMediaSample *pSource, IMediaSample *pDest);

	HRESULT ReceiveSlave(IMediaSample *pSample);

	HRESULT CompleteConnect(PIN_DIRECTION direction, IPin *pReceivePin);

	STDMETHODIMP Stop();
	STDMETHODIMP Pause();

protected:
	HRESULT DecideBufferSize(AM_MEDIA_TYPE* pmt, ALLOCATOR_PROPERTIES* pProp);

protected:

	DECLARE_DBGWND;
};
