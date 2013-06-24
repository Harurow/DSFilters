/* The MIT Licence (MIT)
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
#include <initguid.h>

#include "Utils.h"
#include "VideoResizeBase.h"
#include "VideoResizer.h"
#include "DSFiltersGuids.h"
#include "MediaSampleMonitor.h"

// ビデオのサイズを変更するフィルター
// 入力ピン
//	Video系のビットマップ系
// 出力ピン
//	Video系のビットマップ系
//  リサイズして出力する

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
		TRUE,					// Is it an output
		FALSE,					// Allowed none
		FALSE,					// Allowed many
		&GUID_NULL,				// Connects to filter
		NULL,					// Connects to pin
		1,						// Number of types
		sudOpPinTypes			// Pin information
	},
};

extern const AMOVIESETUP_FILTER sudVideoResizer =
{
	&CLSID_VideoResizer,		// Filter CLSID
	L"Video Resizer",			// String name
	MERIT_DO_NOT_USE,			// Filter merit
	2,							// Number pins
	sudOpPin					// Pin details
};


CUnknown * WINAPI CVideoResizer::CreateInstance(LPUNKNOWN punk,
		HRESULT *phr)
{
	CVideoResizer *pFilter = new CVideoResizer(punk);
	
	if (pFilter == NULL || pFilter->m_pResizer == NULL)
	{
		*phr = E_OUTOFMEMORY;
	}
	else
	{
		*phr = S_OK;
	}
	
	return pFilter;
}


CVideoResizer::CVideoResizer(LPUNKNOWN punk)
	: CTransformFilter(NAME("VideoResizer"), punk, CLSID_VideoResizer)
	, DEST_WIDTH(320)
	, DEST_HEIGHT(240)
	, m_pResizer(NULL)
	, m_nSrcWidth(0)
	, m_nSrcHeight(0)
{
#ifdef _DEBUG
	m_nFrameCount = 0;
	DbgSetModuleLevel(LOG_TRACE, 1);
#endif

	HRESULT hr;
	m_pResizer = new CVideoResizeBase(DEST_WIDTH, DEST_HEIGHT, &hr);
	if (hr != S_OK)
	{
		m_pResizer->NonDelegatingRelease();
		m_pResizer = NULL;
	}
}


CVideoResizer::~CVideoResizer()
{
	DBGWND_DESTROY;
	if (m_pResizer)
		delete m_pResizer;
}


HRESULT CVideoResizer::CheckInputType(const CMediaType *mtIn)
{
	// supported sub types
	// MEDIASUBTYPE_RGB8
	// MEDIASUBTYPE_RGB565
	// MEDIASUBTYPE_RGB555
	// MEDIASUBTYPE_ARGB1555
	// MEDIASUBTYPE_ARGB4444
	// MEDIASUBTYPE_RGB24
	// MEDIASUBTYPE_RGB32
	// MEDIASUBTYPE_ARGB32
	// MEDIASUBTYPE_A2R10G10B10
	// MEDIASUBTYPE_A2B10G10R10

	DbgLog((LOG_TRACE, 1, TEXT("CVideoResizer::CheckInputType")));
	DbgLogMediaType((LOG_TRACE, 0, mtIn));
	DbgLogMediaFormat((LOG_TRACE, 0, mtIn));

	int nBits;
	if (mtIn->majortype != MEDIATYPE_Video
		|| m_pResizer->IsSupportMediaSubType(mtIn->Subtype(), &nBits) != S_OK
		|| mtIn->formattype != FORMAT_VideoInfo
		|| mtIn->cbFormat < sizeof(VIDEOINFOHEADER))
	{
		DbgLog((LOG_TRACE, 1, TEXT(" => NOT ACCEPTED")));
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mtIn->pbFormat);
	if ((pVih->bmiHeader.biBitCount != nBits))
	{
		DbgLog((LOG_TRACE, 1, TEXT(" => NOT ACCEPTED")));
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if (pVih->bmiHeader.biClrUsed > PALETTE_ENTRIES(pVih))
	{
		DbgLog((LOG_TRACE, 1, TEXT(" => NOT ACCEPTED")));
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	DWORD cbPalette = pVih->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	if (mtIn->cbFormat < sizeof(VIDEOINFOHEADER) + cbPalette)
	{
		DbgLog((LOG_TRACE, 1, TEXT(" => NOT ACCEPTED")));
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	m_pResizer->SetMediaSubType(mtIn->Subtype());
	m_nSrcWidth = pVih->bmiHeader.biWidth;
	m_nSrcHeight = pVih->bmiHeader.biHeight;

	DbgLog((LOG_TRACE, 1, TEXT(" => ACCEPTED !")));
	return S_OK;
}


HRESULT CVideoResizer::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	ASSERT(m_pInput->IsConnected());
	
	if (iPosition < 0)
		return E_INVALIDARG;
	
	if (iPosition != 0)
		return VFW_S_NO_MORE_ITEMS;

	CMediaType *pInMediaType = &m_pInput->CurrentMediaType();
	VIDEOINFOHEADER *pInVh = (VIDEOINFOHEADER*)pInMediaType->Format();

	// ここでIndexColorとそうでないので処理を分ける
	ULONG nInFmtLen = pInMediaType->FormatLength();
	if (nInFmtLen < sizeof(VIDEOINFOHEADER))
	{
		return E_UNEXPECTED;
	}
	
	VIDEOINFOHEADER* pvh =
					(VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer(nInFmtLen);
	if (!pvh)
	{
		return E_OUTOFMEMORY;
	}

	ZeroMemory(pvh, nInFmtLen);
	pvh->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvh->bmiHeader.biWidth = m_pResizer->GetToWidth();
	pvh->bmiHeader.biHeight = m_pResizer->GetToHeight();
	pvh->bmiHeader.biPlanes = 1;
	pvh->bmiHeader.biBitCount = m_pResizer->GetBitsPerPixel();
	pvh->bmiHeader.biCompression = BI_RGB;
	pvh->bmiHeader.biSizeImage = m_pResizer->GetSize();
	pvh->bmiHeader.biClrUsed = pInVh->bmiHeader.biClrUsed;
	pvh->bmiHeader.biClrImportant = pInVh->bmiHeader.biClrImportant;

	pMediaType->SetType(&MEDIATYPE_Video);
	pMediaType->SetSubtype(m_pResizer->GetMediaSubType());
	pMediaType->SetFormatType(&FORMAT_VideoInfo);
	pMediaType->SetSampleSize(pvh->bmiHeader.biSizeImage);
	pMediaType->SetTemporalCompression(FALSE);
	pMediaType->bFixedSizeSamples = TRUE;

	if (sizeof(VIDEOINFOHEADER) < nInFmtLen)
	{
		// パレットのコピー
		int cbPalette = nInFmtLen - sizeof(VIDEOINFOHEADER);
		LPBYTE pSrcPlt = (LPBYTE)pInVh + sizeof(VIDEOINFOHEADER);
		LPBYTE pDstPlt = (LPBYTE)pvh + sizeof(VIDEOINFOHEADER);
		::CopyMemory(pDstPlt, pSrcPlt, cbPalette);
	}

	double frameRate = 10000000.0 / pInVh->AvgTimePerFrame;
	pvh->dwBitRate = (DWORD)(pvh->bmiHeader.biSizeImage * 8 * frameRate);
	pvh->AvgTimePerFrame = pInVh->AvgTimePerFrame;
	SetRectEmpty(&(pvh->rcSource));
	SetRectEmpty(&(pvh->rcTarget));

	DbgLog((LOG_TRACE, 1, TEXT("CVideoResizer::GetMediaType")));
	DbgLogMediaType((LOG_TRACE, 0, pMediaType));
	DbgLogMediaFormat((LOG_TRACE, 0, pMediaType));

	return S_OK;
}


HRESULT CVideoResizer::CheckTransform(const CMediaType *mtIn,
										const CMediaType *mtOut)
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoResizer::CheckTransform")));
	DbgLog((LOG_TRACE, 1, TEXT(" <Media In>")));
	DbgLogMediaType((LOG_TRACE, 0, mtIn));
	DbgLogMediaFormat((LOG_TRACE, 0, mtIn));
	DbgLog((LOG_TRACE, 1, TEXT(" <Media Out>")));
	DbgLogMediaType((LOG_TRACE, 0, mtOut));
	DbgLogMediaFormat((LOG_TRACE, 0, mtOut));

	if (mtOut->majortype != MEDIATYPE_Video
		|| mtOut->subtype != *m_pResizer->GetMediaSubType())
	{
		DbgLog((LOG_TRACE, 1, TEXT(" => NOT ACCEPTED")));
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if (mtOut->formattype != FORMAT_VideoInfo
		|| mtOut->cbFormat < sizeof(VIDEOINFOHEADER))
	{
		DbgLog((LOG_TRACE, 1, TEXT(" => NOT ACCEPTED")));
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// ビットマップ情報と入力タイプを比較する。
	ASSERT(mtIn->formattype == FORMAT_VideoInfo);
	BITMAPINFOHEADER *pBmiOut = HEADER(mtOut->pbFormat);
	BITMAPINFOHEADER *pBmiIn = HEADER(mtIn->pbFormat);
	if (pBmiOut->biPlanes != 1
		|| pBmiOut->biBitCount != m_pResizer->GetBitsPerPixel()
		|| pBmiOut->biCompression != BI_RGB
		|| pBmiOut->biWidth != m_pResizer->GetToWidth()
		|| pBmiOut->biHeight != m_pResizer->GetToHeight())
	{
		DbgLog((LOG_TRACE, 1, TEXT(" => NOT ACCEPTED")));
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	DbgLog((LOG_TRACE, 1, TEXT(" => ACCEPTED !")));
	return S_OK;
}


HRESULT CVideoResizer::DecideBufferSize(IMemAllocator *pAlloc,
										ALLOCATOR_PROPERTIES *pAp)
{
	AM_MEDIA_TYPE mt;
	HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
	if (FAILED(hr))
	{
		return hr;
	}

	ASSERT(mt.formattype == FORMAT_VideoInfo);
	BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);
	
	pAp->cBuffers = 1;
	pAp->cbBuffer = m_pResizer->GetSize();
	pAp->cbAlign = 4;
	pAp->cbPrefix = 0;

	// フォーマット ブロックを解放する。
	FreeMediaType(mt);

	// アロケータ プロパティを設定する。
	ALLOCATOR_PROPERTIES actual;
	hr = pAlloc->SetProperties(pAp, &actual);
	if (FAILED(hr)) 
	{
		return hr;
	}
	
	// 成功した場合も、実際の結果を確認する。
	if (pAp->cbBuffer > actual.cbBuffer)
	{
		return E_FAIL;
	}
	
	return S_OK;
}


HRESULT CVideoResizer::Transform(IMediaSample *pSource, IMediaSample *pDest)
{
	HRESULT hr = m_pResizer->Transform(pSource, m_nSrcWidth, m_nSrcHeight,
									   pDest);

#if _DEBUG
	m_nFrameCount++;
	DbgWndDisplay(&DBGWND, this, pDest, m_nFrameCount, m_tStart);
	DBGWND_TEXT(10, 110, GetSubtypeName(m_pResizer->GetMediaSubType()));
#endif

	return hr;
}


HRESULT CVideoResizer::CompleteConnect(PIN_DIRECTION direction,
		IPin *pReceivePin)
{
	{
		CAutoLock lock(&m_csFilter);
		DBGWND_CREATE;
	}

	return CTransformFilter::CompleteConnect(direction, pReceivePin);
}


STDMETHODIMP CVideoResizer::Stop()
{
#ifdef _DEBUG
	DBGWND_RGB(255,0,0);
	DBGWND_TEXT(10, 0, TEXT("Stop  "));
	DBGWND_RESET_RGB;
	m_nFrameCount = 0;
#endif

	return CTransformFilter::Stop();
}


STDMETHODIMP CVideoResizer::Pause()
{
#ifdef _DEBUG
	DBGWND_RGB(255,0,0);
	DBGWND_TEXT(10, 0, TEXT("Paused"));
	DBGWND_RESET_RGB;
#endif

	return CTransformFilter::Pause();
}
