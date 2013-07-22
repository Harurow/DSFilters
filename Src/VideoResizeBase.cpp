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

#include "Utils.h"
#include "VideoResizeBase.h"


CVideoResizeBase::CVideoResizeBase(int toWidth, int toHeight, HRESULT* phr)
	: CUnknown(NAME("Video Resize Base"), NULL)
	, m_nToWidth(toWidth)
	, m_nToHeight(toHeight)
	, m_MediaSubType(GUID_NULL)
	, m_nBytesPerPixel(0)
	, m_nSrcWidth(0)
	, m_nSrcHeight(0)
{
	ASSERT(phr);

	m_pWScale = new ULONG[toWidth];
	m_pHScale = new ULONG[toHeight];

	if (m_pWScale == NULL || m_pHScale == NULL) {
		*phr = E_OUTOFMEMORY;
		delete [] m_pWScale;
		delete [] m_pHScale;
		m_pWScale = NULL;
		m_pHScale = NULL;
	} else {
		*phr = S_OK;
	}
}


CVideoResizeBase::~CVideoResizeBase()
{
	delete [] m_pWScale;
	delete [] m_pHScale;
}


STDMETHODIMP CVideoResizeBase::IsSupportMediaSubType(const GUID* pMediaSubType,
													 int* pBits)
{
	CheckPointer(pMediaSubType, E_POINTER);
	CheckPointer(pBits, E_POINTER);

	int bits = GetBmpBits(pMediaSubType);
	if ((bits & 0x7) == 0) {
		*pBits = bits;
	} else {
		return E_INVALIDARG;
	}

	return S_OK;
}


STDMETHODIMP CVideoResizeBase::Transform(IMediaSample* pSrcSample,
							 int nWidth, int nHeight, IMediaSample* pDstSample)
{
	CheckPointer(pSrcSample, E_POINTER);
	CheckPointer(pDstSample, E_POINTER);

	BYTE *pSrcBuf, *pDstBuf;
	HRESULT hr = pSrcSample->GetPointer(&pSrcBuf);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pDstSample->GetPointer(&pDstBuf);
	if (FAILED(hr)) {
		return hr;
	}

	if (nWidth == m_nToWidth && abs(nHeight) == abs(m_nToHeight)) {
		if (nHeight == m_nToHeight) {
			// 単純コピー
			::CopyMemory(pDstBuf, pSrcBuf, pSrcSample->GetSize());
		} else {
			// 上下入れ替えコピー
			int nStride = CalcStride(m_nToWidth);
			int h = abs(m_nToHeight);
			for (int y = 1; y <= m_nToHeight; y++) {
				BYTE* pSrcLine = pSrcBuf + nStride * (h - y);
				::CopyMemory(pDstBuf, pSrcLine, nStride);
			}
		}

		pDstSample->SetActualDataLength(pSrcSample->GetSize());
	} else {
		// 拡大・縮小
		SetupScaleTable(nWidth, nHeight);

		int nStride = CalcStride(m_nToWidth);
		switch (m_nBytesPerPixel) {
		case 1:
			// MEDIASUBTYPE_RGB8
			for (int y = 0; y < m_nToHeight; y++) {
				LPBYTE pSrcLine = pSrcBuf + m_pHScale[y];
				LPBYTE pDstPtr = pDstBuf + nStride * y;
				for (int x = 0; x < m_nToWidth; x++) {
					LPBYTE pSrcPtr = pSrcLine + m_pWScale[x];
					*pDstPtr++ = *pSrcPtr;
				}
			}
			break;

		case 2:
			// MEDIASUBTYPE_RGB565
			// MEDIASUBTYPE_RGB555
			// MEDIASUBTYPE_ARGB1555
			// MEDIASUBTYPE_ARGB4444
			for (int y = 0; y < m_nToHeight; y++) {
				LPBYTE pSrcLine = pSrcBuf + m_pHScale[y];
				LPBYTE pDstPtr = pDstBuf + nStride * y;
				for (int x = 0; x < m_nToWidth; x++) {
					LPBYTE pSrcPtr = pSrcLine + m_pWScale[x];
					*pDstPtr++ = *pSrcPtr++;
					*pDstPtr++ = *pSrcPtr;
				}
			}
			break;

		case 3:
			// MEDIASUBTYPE_RGB24
			for (int y = 0; y < m_nToHeight; y++) {
				LPBYTE pSrcLine = pSrcBuf + m_pHScale[y];
				LPBYTE pDstPtr = pDstBuf + nStride * y;
				for (int x = 0; x < m_nToWidth; x++) {
					LPBYTE pSrcPtr = pSrcLine + m_pWScale[x];
					*pDstPtr++ = *pSrcPtr++;
					*pDstPtr++ = *pSrcPtr++;
					*pDstPtr++ = *pSrcPtr;
				}
			}
			break;

		case 4:
			// MEDIASUBTYPE_RGB32
			// MEDIASUBTYPE_ARGB32
			// MEDIASUBTYPE_A2R10G10B10
			// MEDIASUBTYPE_A2B10G10R10
			for (int y = 0; y < m_nToHeight; y++) {
				LPBYTE pSrcLine = pSrcBuf + m_pHScale[y];
				LPBYTE pDstPtr = pDstBuf + nStride * y;
				for (int x = 0; x < m_nToWidth; x++) {
					LPBYTE pSrcPtr = pSrcLine + m_pWScale[x];
					*pDstPtr++ = *pSrcPtr++;
					*pDstPtr++ = *pSrcPtr++;
					*pDstPtr++ = *pSrcPtr++;
					*pDstPtr++ = *pSrcPtr;
				}
			}
			break;

		default:
			return E_FAIL;
		}

		pDstSample->SetActualDataLength(CalcStride(m_nToWidth) * m_nToHeight);
	}

	return S_OK;
}


STDMETHODIMP CVideoResizeBase::SetMediaSubType(const GUID* pMediaSubType)
{
	if (m_MediaSubType != *pMediaSubType) {
		int nLastPixBytes = m_nBytesPerPixel;

		int bits = GetBmpBits(pMediaSubType);
		if (bits & 0x7) {
			return E_INVALIDARG;
		}

		m_nBytesPerPixel = bits / 8;;

		if (nLastPixBytes != m_nBytesPerPixel) {
			// reset scale table
			m_nSrcWidth = 0;
			m_nSrcHeight = 0;
		}

		m_MediaSubType = *pMediaSubType;
	}

	return S_OK;
}


STDMETHODIMP CVideoResizeBase::SetupScaleTable(int nWidth, int nHeight)
{
	ASSERT(m_pWScale);
	ASSERT(m_pHScale);
	ASSERT(nWidth > 0);
	ASSERT(nHeight > 0);

	if (m_nSrcWidth != nWidth || m_nSrcHeight != nHeight) {
		m_nSrcWidth = nWidth;
		m_nSrcHeight = nHeight;

		int nPixBytes = m_nBytesPerPixel;

		for(int x = 0; x < m_nToWidth; x++) {
			m_pWScale[x] = (int)((double)(nWidth * nPixBytes * x)
													/ m_nToWidth);
		}
		
		int stride = CalcStride(nWidth);
		nHeight = abs(nHeight);

		for (int y = 0; y < m_nToHeight; y++) {
			m_pHScale[y] = (int)((double)(nHeight * y) / m_nToHeight) * stride;
		}
	}

	return S_OK;
}
