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

#include <streams.h>
#include <olectl.h>
#include <initguid.h>

#include "DSFiltersGuids.h"
#include "Utils.h"
#include "VideoMux.h"
#include "ToggleBuffer.h"


const AMOVIESETUP_MEDIATYPE sudOpPinTypes[] =
{
	{
		&MEDIATYPE_Video,		// Major type
		&MEDIASUBTYPE_NULL		// Minor type
	}
};

const AMOVIESETUP_PIN sudOpPin[] =
{
	{
		L"",					// Pin string name
		FALSE,					// Is it rendered
		FALSE,					// Is it an output
		FALSE,					// Allowed none
		FALSE,					// Allowed many
		&GUID_NULL,				// Connects to filter
		NULL,					// Connects to pin
		1,						// Number of types
		sudOpPinTypes			// Pin information
	},
	{
		L"",					// Pin string name
		FALSE,					// Is it rendered
		FALSE,					// Is it an output
		FALSE,					// Allowed none
		FALSE,					// Allowed many
		&GUID_NULL,				// Connects to filter
		NULL,					// Connects to pin
		1,						// Number of types
		sudOpPinTypes			// Pin information
	},
	{
		L"",					// Pin string name
		FALSE,					// Is it rendered
		TRUE,					// Is it an output
		FALSE,					// Allowed none
		FALSE,					// Allowed many
		&GUID_NULL,				// Connects to filter
		NULL,					// Connects to pin
		1,						// Number of types
		sudOpPinTypes			// Pin information
	},
};

extern const AMOVIESETUP_FILTER sudVideoMux =
{
	&CLSID_VideoMux,			// Filter CLSID
	L"Video Mux",				// String name
	MERIT_DO_NOT_USE,			// Filter merit
	3,							// Number pins
	sudOpPin					// Pin details
};


CUnknown* WINAPI CVideoMux::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
	CVideoMux* pMux = new CVideoMux(punk, phr, 320, 240);
	if (pMux == NULL) {
		*phr = E_OUTOFMEMORY;
	}
	return pMux;
}


CVideoMux::CVideoMux(LPUNKNOWN punk, HRESULT* phr, int nWidth, int nHeight)
	: CBaseMux(NAME("Video Mux"), punk, CLSID_VideoMux)
	, m_nWidth(nWidth)
	, m_nHeight(nHeight)
	, m_pBuf(NULL)
	, m_nPixelPerBytes(0)
{
	ASSERT(nWidth > 0);
	ASSERT(nHeight > 0);

	*phr = S_OK;
}


CVideoMux::~CVideoMux()
{
	delete m_pBuf;
	DBGWND_DESTROY;
}


HRESULT CVideoMux::CheckInputType(const CMediaType *mtIn)
{
	if (mtIn->majortype != MEDIATYPE_Video
			|| mtIn->formattype != FORMAT_VideoInfo
			|| mtIn->cbFormat < sizeof(VIDEOINFOHEADER)) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mtIn->pbFormat);

	const GUID* pConnectedGuid = NULL;
	if (m_pInput->IsConnected()) {
		CMediaType& mt = m_pInput->CurrentMediaType();
		pConnectedGuid = mt.Subtype();
	} else if (m_pSlaveInput->IsConnected()) {
		CMediaType& mt = m_pSlaveInput->CurrentMediaType();
		pConnectedGuid = mt.Subtype();
	}

	int bits;
	if (pConnectedGuid) {
		if (mtIn->subtype != *pConnectedGuid) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
		bits = GetBmpBits(pConnectedGuid);
	} else {
		bits = GetBmpBits(&mtIn->subtype);
		if (bits & 0x7 || bits == 8) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	if ((pVih->bmiHeader.biBitCount != bits) ||
		(pVih->bmiHeader.biWidth != m_nWidth) ||
		(pVih->bmiHeader.biHeight != m_nHeight))
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return S_OK;
}


HRESULT CVideoMux::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	ASSERT(m_pInput->IsConnected());

	if (iPosition < 0)
		return E_INVALIDARG;

	if (iPosition != 0)
		return VFW_S_NO_MORE_ITEMS;

	HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
	if (FAILED(hr))
		return hr;

	CMediaType* pMtIn = &m_pInput->CurrentMediaType();
	VIDEOINFOHEADER *pInVih = (VIDEOINFOHEADER*)pMtIn->Format();

	int bits = GetBmpBits(&pMtIn->subtype);
	DWORD cbSize = CalcStride(m_nWidth * 2, bits/8) * m_nHeight;

	pMediaType->SetType(&MEDIATYPE_Video);
	pMediaType->SetSubtype(&pMtIn->subtype);
	pMediaType->SetFormatType(&FORMAT_VideoInfo);
	pMediaType->SetSampleSize(cbSize);
	pMediaType->SetTemporalCompression(FALSE);
	pMediaType->bFixedSizeSamples = TRUE;

	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pMediaType->
								AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
	if (!pVih) {
		return E_OUTOFMEMORY;
	}
	
	CToggleBuffer* pTglBuf = new CToggleBuffer(cbSize);
	if (!pTglBuf) {
		return E_OUTOFMEMORY;
	}
	delete m_pBuf;
	m_pBuf = pTglBuf;
	m_nPixelPerBytes = bits/8;

	ZeroMemory(pVih, sizeof(VIDEOINFOHEADER));
	pVih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pVih->bmiHeader.biWidth = m_nWidth * 2;
	pVih->bmiHeader.biHeight = m_nHeight;
	pVih->bmiHeader.biPlanes = 1;
	pVih->bmiHeader.biBitCount = bits;
	pVih->bmiHeader.biCompression = BI_RGB;
	pVih->bmiHeader.biSizeImage = cbSize;
	pVih->bmiHeader.biClrImportant = 0;

	pVih->dwBitRate = pInVih->dwBitRate * 2;
	pVih->dwBitErrorRate = pInVih->dwBitErrorRate * 2;
	pVih->AvgTimePerFrame = pInVih->AvgTimePerFrame;

	SetRectEmpty(&(pVih->rcSource));
	SetRectEmpty(&(pVih->rcTarget));

	return S_OK;
}


HRESULT CVideoMux::CheckTransform(const CMediaType *mtIn,
								  const CMediaType *mtOut)
{
	ASSERT(mtIn);
	ASSERT(mtOut);

	if (mtOut->majortype != mtIn->majortype
			|| mtOut->subtype != mtIn->subtype) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if (mtOut->formattype != FORMAT_VideoInfo
			|| mtOut->cbFormat < sizeof(VIDEOINFOHEADER)) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// ビットマップ情報と入力タイプを比較する。
	ASSERT(mtIn->formattype == FORMAT_VideoInfo);
	BITMAPINFOHEADER *pBmiOut = HEADER(mtOut->pbFormat);
	BITMAPINFOHEADER *pBmiIn = HEADER(mtIn->pbFormat);
	if (pBmiOut->biPlanes != pBmiIn->biPlanes
			|| pBmiOut->biCompression != pBmiIn->biCompression
			|| pBmiOut->biWidth != pBmiIn->biWidth * 2
			|| pBmiOut->biHeight != pBmiIn->biHeight) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// 転送元矩形と転送先矩形を比較する。
	RECT rcSrc, rcDst;
	SetRect(&rcSrc, 0, 0, pBmiIn->biWidth, pBmiIn->biHeight);
	SetRect(&rcDst, 0, 0, pBmiOut->biWidth, pBmiOut->biHeight);
	RECT *prcSrc = &((VIDEOINFOHEADER*)(mtIn->pbFormat))->rcSource;
	RECT *prcTarget = &((VIDEOINFOHEADER*)(mtOut->pbFormat))->rcTarget;
	if (!IsRectEmpty(prcSrc) && !EqualRect(prcSrc, &rcSrc)) {
		return VFW_E_INVALIDMEDIATYPE;
	}
	if (!IsRectEmpty(prcTarget) && !EqualRect(prcTarget, &rcDst)) {
		return VFW_E_INVALIDMEDIATYPE;
	}

	return S_OK;
}


HRESULT CVideoMux::Transform(IMediaSample *pSource, IMediaSample *pDest)
{
	HRESULT hr;

	BYTE* pSrcBuf;
	hr = pSource->GetPointer(&pSrcBuf);
	if (FAILED(hr))
		return hr;

	BYTE* pDstBuf;
	hr = pDest->GetPointer(&pDstBuf);
	if (FAILED(hr))
		return hr;

	int nSrcLineBytes = m_nWidth * m_nPixelPerBytes;
	int nSrcStride = CalcStride(m_nWidth, m_nPixelPerBytes);
	int nDstStride = CalcStride(m_nWidth * 2, m_nPixelPerBytes);
	int cbSrcSize = nSrcStride * m_nHeight;
	int cbDstSize = nDstStride * m_nHeight;

	BYTE* pSrc = pSrcBuf;
	BYTE* pDst = pDstBuf;
	for (int y = 0; y < m_nHeight; y++) {
		::CopyMemory(pDst, pSrc, nSrcLineBytes);
		pSrc += nSrcStride;
		pDst += nDstStride;
	}

	if (m_pSlaveInput->IsConnected()) {
		CAutoLock lock(m_pBuf->GetLock());
		ASSERT(m_pBuf->GetBloskSize() >= cbSrcSize);
		pSrc = m_pBuf->GetData();
		pDst = pDstBuf + nSrcLineBytes;
		for (int y = 0; y < m_nHeight; y++) {
			::CopyMemory(pDst, pSrc, nSrcLineBytes);
			pSrc += nSrcStride;
			pDst += nDstStride;
		}
	}

	pDest->SetActualDataLength(cbDstSize);

#ifdef DEBUG
	m_nFrameCount++;
	DbgWndDisplay(&DBGWND, this, pDest, m_nFrameCount, m_tStart);
#endif

	return hr;
}


HRESULT CVideoMux::ReceiveSlave(IMediaSample *pSample)
{
	ASSERT(m_pBuf);
	ASSERT(pSample);

	CAutoLock lock(&m_csReceive);

	BYTE* pBuf;
	HRESULT hr = pSample->GetPointer(&pBuf);
	if (hr == S_OK && pBuf) {
		long cbSize = pSample->GetActualDataLength();
		if (cbSize <= m_pBuf->GetBloskSize()) {
			m_pBuf->Toggle(pBuf, cbSize);
		}
	}

	return hr;
}


HRESULT CVideoMux::CompleteConnect(PIN_DIRECTION direction, IPin *pReceivePin)
{
	{
		CAutoLock lock(&m_csFilter);
		DBGWND_CREATE;
	}

	return CBaseMux::CompleteConnect(direction, pReceivePin);
}


STDMETHODIMP CVideoMux::Stop()
{
#ifdef _DEBUG
	DBGWND_RGB(255,0,0);
	DBGWND_TEXT(10, 0, TEXT("Stop  "));
	DBGWND_RESET_RGB;
	m_nFrameCount = 0;
#endif

	m_pBuf->Clear();

	return CBaseMux::Stop();
}


STDMETHODIMP CVideoMux::Pause()
{
#ifdef _DEBUG
	DBGWND_RGB(255,0,0);
	DBGWND_TEXT(10, 0, TEXT("Paused"));
	DBGWND_RESET_RGB;
#endif

	return CBaseMux::Pause();
}


HRESULT CVideoMux::DecideBufferSize(AM_MEDIA_TYPE* pmt,
									ALLOCATOR_PROPERTIES* pProp)
{
	ASSERT(m_pBuf);
	ASSERT(m_nPixelPerBytes > 0);

	pProp->cbBuffer = CalcStride(m_nWidth * 2, m_nPixelPerBytes) * m_nHeight;
	pProp->cbAlign = 4;

	return S_OK;
}
