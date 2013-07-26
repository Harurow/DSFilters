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
#include "AudioMux.h"
#include "StringBuffer.h"


const AMOVIESETUP_MEDIATYPE sudOpPinTypes[] =
{
	{
		&MEDIATYPE_Audio,		// Major type
		&MEDIASUBTYPE_PCM		// Minor type
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

extern const AMOVIESETUP_FILTER sudAudioMux =
{
	&CLSID_AudioMux,			// Filter CLSID
	L"Audio Mux",				// String name
	MERIT_DO_NOT_USE,			// Filter merit
	3,							// Number pins
	sudOpPin					// Pin details
};


CUnknown* WINAPI CAudioMux::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
	CAudioMux* pMux = new CAudioMux(punk, phr);
	if (pMux == NULL) {
		*phr = E_OUTOFMEMORY;
	}
	return pMux;
}


CAudioMux::CAudioMux(LPUNKNOWN punk, HRESULT* phr)
	: CBaseMux(NAME("Audio Mux"), punk, CLSID_AudioMux)
	, m_pBuf(NULL)
	, m_nChannels(0)
	, m_nSamplesPerSec(0)
	, m_nBitsPerSample(0)
	, m_nnBlockAlign(0)
	, m_nMasterMediaTime(0)
	, m_nSlaveMediaTime(0)
{
	*phr = S_OK;
}


CAudioMux::~CAudioMux()
{
	delete m_pBuf;
	DBGWND_DESTROY;
}


HRESULT CAudioMux::CheckInputType(const CMediaType *mtIn)
{
	if (mtIn->majortype != MEDIATYPE_Audio
			|| mtIn->subtype != MEDIASUBTYPE_PCM
			|| mtIn->formattype != FORMAT_WaveFormatEx
			|| mtIn->cbFormat < sizeof(WAVEFORMATEX)) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	WAVEFORMATEX *pinwf = NULL;
	if (m_pInput->IsConnected()) {
		CMediaType& mt = m_pInput->CurrentMediaType();
		pinwf = reinterpret_cast<WAVEFORMATEX*>(&mt.pbFormat);
	} else if (m_pSlaveInput->IsConnected()) {
		CMediaType& mt = m_pSlaveInput->CurrentMediaType();
		pinwf = reinterpret_cast<WAVEFORMATEX*>(&mt.pbFormat);
	}

	WAVEFORMATEX *pwf = reinterpret_cast<WAVEFORMATEX*>(mtIn->pbFormat);
	if (pinwf) {
		if (pwf->wFormatTag != pinwf->wFormatTag
				|| pwf->nChannels != pinwf->nChannels
				|| pwf->nSamplesPerSec != pinwf->nSamplesPerSec
				|| pwf->wBitsPerSample != pinwf->wBitsPerSample
				|| pwf->nBlockAlign != pinwf->nBlockAlign) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
		return S_OK;
	}

	if (pwf->wFormatTag != WAVE_FORMAT_PCM
		|| (pwf->nChannels != 1 && pwf->nChannels != 2)
		|| (pwf->nSamplesPerSec < 8000)
		|| (pwf->wBitsPerSample != 8 && pwf->wBitsPerSample != 16
				&& pwf->wBitsPerSample != 32)
		|| (pwf->nBlockAlign != pwf->nChannels * (pwf->wBitsPerSample/8)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	
	m_nChannels = pwf->nChannels;
	m_nSamplesPerSec = pwf->nSamplesPerSec;
	m_nBitsPerSample = pwf->wBitsPerSample;
	m_nBlockAlign = pwf->nBlockAlign;

	return S_OK;
}


HRESULT CAudioMux::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	ASSERT(m_pInput->IsConnected());

	if (iPosition < 0)
		return E_INVALIDARG;

	if (iPosition != 0)
		return VFW_S_NO_MORE_ITEMS;

	HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
	if (FAILED(hr))
		return hr;

	CMediaType *pInMediaType = &m_pInput1->CurrentMediaType();
	WAVEFORMATEX *pInWf = (WAVEFORMATEX*)pInMediaType->Format();

	pMediaType->SetType(&MEDIATYPE_Audio);
	pMediaType->SetSubtype(&MEDIASUBTYPE_PCM);
	pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
	pMediaType->SetTemporalCompression(FALSE);
	pMediaType->SetSampleSize(m_nSampleSize);
	pMediaType->bFixedSizeSamples = TRUE;

	WAVEFORMATEX* pwf = (WAVEFORMATEX*) pMediaType->
									AllocFormatBuffer(sizeof(WAVEFORMATEX));
	if (!pwf)
		return E_OUTOFMEMORY;
	
	ZeroMemory(pwf, sizeof(WAVEFORMATEX));
	pwf->wFormatTag = WAVE_FORMAT_PCM;
	pwf->nChannels = m_nChannels;
	pwf->nSamplesPerSec = m_nSamplesPerSec;
	pwf->nAvgBytesPerSec = m_nBlockAlign * m_nSamplesPerSec;
	pwf->nBlockAlign = m_nBlockAlign;
	pwf->wBitsPerSample = m_nBitsPerSample;
	pwf->cbSize = 0;

	return S_OK;
}


HRESULT CAudioMux::CheckTransform(const CMediaType *mtIn,
								  const CMediaType *mtOut)
{
	ASSERT(mtIn);
	ASSERT(mtOut);

	if (mtOut->majortype != MEDIATYPE_Audio
		|| mtOut->subtype != MEDIASUBTYPE_PCM)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if ((mtOut->formattype != FORMAT_WaveFormatEx) ||
		(mtOut->cbFormat < sizeof(WAVEFORMATEX)))
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	ASSERT(mtIn->formattype == FORMAT_WaveFormatEx);
	WAVEFORMATEX* pOutWf = (WAVEFORMATEX *) mtOut->Format();

	if (pOutWf->wFormatTag != WAVE_FORMAT_PCM
		|| pOutWf->nChannels != m_nChannels
		|| pOutWf->nSamplesPerSec != m_nSamplesPerSec
		|| pOutWf->nAvgBytesPerSec != m_nBlockAlign * m_nSamplesPerSec
		|| pOutWf->nBlockAlign != m_nBlockAlign
		|| pOutWf->wBitsPerSample != m_nBitsPerSample
		|| pOutWf->cbSize != 0)
	{
		return VFW_E_INVALIDMEDIATYPE;
	}

	return S_OK;
}


HRESULT CAudioMux::Transform(IMediaSample *pSource, IMediaSample *pDest)
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

	//:TODO バッファとミックスする



	pDest->SetActualDataLength(cbDstSize);

#ifdef DEBUG
	m_nFrameCount++;
	DbgWndDisplay(&DBGWND, this, pDest, m_nFrameCount, m_tStart);
#endif

	return hr;
}


HRESULT CAudioMux::ReceiveSlave(IMediaSample *pSample)
{
	ASSERT(m_pBuf);
	ASSERT(pSample);

	CAutoLock lock(&m_csReceive);

	if (m_pBuf == NULL || m_pBuf->GetBlockSize() < pSample->GetSize()) {
		delete m_pBuf;
		int cbBufSize = pSample->GetSize();
		DWORD cbBytesPerSec = m_nSamplesPerSec * m_nBlockAlign;
		int nBlockCount = 2;
		if (cbBufSize < cbBytesPerSec) {
			nBlockCount = (cbBytesPerSec / cbBufSize) + 1;
		}
		cbBufSize += sizeof(LONGLONG) + sizeof(long);
		m_pBuf = new CRingBuffer(cbBufSize, nBlockCount);
		if (m_pBuf == NULL) {
			return E_OUTOFMEMORY;
		}
	}

	BYTE* pBuf;
	HRESULT hr = pSample->GetPointer(&pBuf);
	if (hr == S_OK && pBuf) {
		long cbSize = pSample->GetActualDataLength();

		// TODO: buffer stock
	}
	m_nSlaveMediaTime += cbSize / m_nBlockAlign;

	return hr;
}


HRESULT CAudioMux::CompleteConnect(PIN_DIRECTION direction, IPin *pReceivePin)
{
	{
		CAutoLock lock(&m_csFilter);
		DBGWND_CREATE;
	}

	return CBaseMux::CompleteConnect(direction, pReceivePin);
}


STDMETHODIMP CAudioMux::Stop()
{
#ifdef _DEBUG
	DBGWND_RGB(255,0,0);
	DBGWND_TEXT(10, 0, TEXT("Stop  "));
	DBGWND_RESET_RGB;
	m_nFrameCount = 0;
#endif

	delete m_pBuf;
	m_pBuf = NULL;
	m_nMasterMediaTime = 0;
	m_nSlaveMediaTime = 0;

	return CBaseMux::Stop();
}


STDMETHODIMP CAudioMux::Pause()
{
#ifdef _DEBUG
	DBGWND_RGB(255,0,0);
	DBGWND_TEXT(10, 0, TEXT("Paused"));
	DBGWND_RESET_RGB;
#endif

	return CBaseMux::Pause();
}


HRESULT CAudioMux::DecideBufferSize(AM_MEDIA_TYPE* pmt,
									ALLOCATOR_PROPERTIES* pProp)
{
	ASSERT(m_pBuf);
	ASSERT(m_nPixelPerBytes > 0);

	pProp->cbAlign = m_nBlockAlign;

	return S_OK;
}
