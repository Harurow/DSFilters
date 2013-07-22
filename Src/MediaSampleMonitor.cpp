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
#include "MediaSampleMonitor.h"
#include "DSFiltersGuids.h"


// メディアサンプルの状況を表示する
// 入力ピン
//	すべて
// 出力ピン
//  入力ピンに依存

const AMOVIESETUP_MEDIATYPE sudOpPinTypes[] =
{
	{
		&MEDIATYPE_NULL,		// Major type
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

extern const AMOVIESETUP_FILTER sudMediaSampleMonitor =
{
	&CLSID_MediaSampleMonitor,	// Filter CLSID
	L"Media Sample Monitor",	// String name
	MERIT_DO_NOT_USE,			// Filter merit
	2,							// Number pins
	sudOpPin					// Pin details
};


CUnknown * WINAPI CMediaSampleMonitor::CreateInstance(LPUNKNOWN punk,
		HRESULT *phr)
{
	CMediaSampleMonitor *pFilter = new CMediaSampleMonitor(punk, phr);
	
	if (pFilter == NULL) {
		*phr = E_OUTOFMEMORY;
	} else {
		*phr = S_OK;
	}
	
	return pFilter;
}


CMediaSampleMonitor::CMediaSampleMonitor(LPUNKNOWN punk, HRESULT* phr)
	: CTransInPlaceFilter(NAME("MediaSampleMonitor"), punk,
			CLSID_MediaSampleMonitor, phr)
{
#ifdef _DEBUG
	m_nFrameCount = 0;
	DbgSetModuleLevel(LOG_TRACE, 1);
#endif
}


CMediaSampleMonitor::~CMediaSampleMonitor()
{
#ifdef SHOW_DBG_WND
	m_dbgWnd.Destroy();
#endif
}


HRESULT CMediaSampleMonitor::CheckInputType(const CMediaType *mtIn)
{
#ifdef SHOW_DBG_WND
	DbgLog((LOG_TRACE, 1, TEXT("CMediaSampleMonitor::CheckInputType")));
	DbgLogMediaType((LOG_TRACE, 0, mtIn));
	DbgLogMediaFormat((LOG_TRACE, 0, mtIn));
	DbgLog((LOG_TRACE, 1, TEXT(" => ACCEPTED !")));
#endif
	return S_OK;
}


HRESULT CMediaSampleMonitor::Transform(IMediaSample *pSample)
{
#ifdef SHOW_DBG_WND
	m_nFrameCount++;

	DbgWndDisplay(&m_dbgWnd, this, pSample, m_nFrameCount, m_tStart);
#endif
	return S_OK;
}


HRESULT CMediaSampleMonitor::CompleteConnect(PIN_DIRECTION direction,
		IPin *pReceivePin)
{
#ifdef SHOW_DBG_WND
	{
		CAutoLock lock(&m_csFilter);
		m_dbgWnd.Create(this);
	}
#endif
	return CTransInPlaceFilter::CompleteConnect(direction, pReceivePin);
}


STDMETHODIMP CMediaSampleMonitor::Stop()
{
#ifdef SHOW_DBG_WND
	m_dbgWnd.SetTextColor(RGB(255, 0, 0));
	m_dbgWnd.DrawString(10, 0, TEXT("Stop  "));
	m_dbgWnd.SetDefaultTextColor();
	m_nFrameCount = 0;
#endif

	return CTransInPlaceFilter::Stop();
}


STDMETHODIMP CMediaSampleMonitor::Pause()
{
#ifdef SHOW_DBG_WND
	m_dbgWnd.SetTextColor(RGB(255, 0, 0));
	m_dbgWnd.DrawString(10, 0, TEXT("Paused"));
	m_dbgWnd.SetDefaultTextColor();
#endif
	return CTransInPlaceFilter::Pause();
}
