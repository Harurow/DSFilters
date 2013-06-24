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

class CVideoResizeBase : public CUnknown
{
private:
	friend class CVideoResizer;

private:
	CVideoResizeBase(int toWidth, int toHeight, HRESULT* phr);
	virtual ~CVideoResizeBase();

private:
	STDMETHODIMP IsSupportMediaSubType(const GUID* pMediaSubType, int* pBits);

	STDMETHODIMP_(const GUID*) GetMediaSubType() { return &m_MediaSubType; }
	STDMETHODIMP_(int) GetBitsPerPixel() { return m_nBytesPerPixel * 8; }
	STDMETHODIMP_(int) GetToWidth() { return m_nToWidth; }
	STDMETHODIMP_(int) GetToHeight() { return m_nToHeight; }
	STDMETHODIMP_(int) CalcStride(int nWidth)
		{ return ((nWidth * m_nBytesPerPixel) + 3) / 4 * 4; }
	STDMETHODIMP_(int) GetSize()
		{ return CalcStride(m_nToWidth) * abs(m_nToHeight); }

	STDMETHODIMP Transform(IMediaSample* pSrcSample,
					int nSrcWidth, int nSrcHeight, IMediaSample* pDstSample);

	STDMETHODIMP SetMediaSubType(const GUID* pMediaSubType);
	STDMETHODIMP SetupScaleTable(int nWidth, int nHeight);

private:
	const int m_nToWidth;
	const int m_nToHeight;

	GUID m_MediaSubType;
	int m_nBytesPerPixel;

	int m_nSrcWidth;
	int m_nSrcHeight;
	ULONG* m_pWScale;
	ULONG* m_pHScale;
};
