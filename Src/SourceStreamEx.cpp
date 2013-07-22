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

#include <streams.h>
#include <olectl.h>
#include <initguid.h>

#include "SourceStreamEx.h"


CSourceStreamEx::CSourceStreamEx(LPCTSTR pObjectName, HRESULT *phr,
								 CSource *pms, LPCWSTR pName,
								 REFERENCE_TIME defaultRepeatTime)
	: CSourceStream(pObjectName, phr, pms, pName)
	, m_bInitStreamTime(FALSE)
	, m_rtDefaultRepeatTime(defaultRepeatTime)
	, m_rtMinRepeatTime(defaultRepeatTime)
	, m_rtMaxRepeatTime(defaultRepeatTime)
	, m_rtRepeatTime(0)
	, m_rtSampleTime(0)
{
	ASSERT(phr);
	CAutoLock lock(&m_cSharedState);

	if (defaultRepeatTime < 0) {
		if (phr) {
			*phr = E_INVALIDARG;
		}
	}

	m_hStreamTime = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hStreamTime == NULL) {
		if (phr) {
			*phr = E_OUTOFMEMORY;
		}
	}
}


CSourceStreamEx::CSourceStreamEx(LPCTSTR pObjectName, HRESULT *phr,
								 CSource *pms, LPCWSTR pName,
								 REFERENCE_TIME defaultRepeatTime,
								 REFERENCE_TIME minRepeatTime,
								 REFERENCE_TIME maxRepeatTime)
	: CSourceStream(pObjectName, phr, pms, pName)
	, m_bInitStreamTime(FALSE)
	, m_rtDefaultRepeatTime(defaultRepeatTime)
	, m_rtMinRepeatTime(minRepeatTime)
	, m_rtMaxRepeatTime(maxRepeatTime)
	, m_rtRepeatTime(0)
	, m_rtSampleTime(0)
{
	ASSERT(phr);
	CAutoLock lock(&m_cSharedState);

	if (defaultRepeatTime < 0
			|| defaultRepeatTime < minRepeatTime
			|| maxRepeatTime < defaultRepeatTime) {
		if (phr) {
			*phr = E_INVALIDARG;
		}
	}

	m_hStreamTime = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hStreamTime == NULL) {
		if (phr) {
			*phr = E_OUTOFMEMORY;
		}
	}
}


STDMETHODIMP CSourceStreamEx::NonDelegatingQueryInterface(REFIID riid,
															void ** ppv)
{
	CheckPointer(ppv, E_POINTER);

	if (riid == IID_IAMPushSource) {
		return GetInterface((IAMPushSource*)(this), ppv);
	} else if (riid == IID_IAMLatency) {
		return GetInterface((IAMLatency*)(this), ppv);
	}

	return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}


CSourceStreamEx::~CSourceStreamEx()
{
	CAutoLock lock(&m_cSharedState);

	HANDLE hEvent = m_hStreamTime;
	m_hStreamTime = NULL;
	if (hEvent != NULL) {
		::SetEvent(hEvent);
		::CloseHandle(hEvent);
	}
}


// implement IAMPushSource
HRESULT STDMETHODCALLTYPE CSourceStreamEx::GetPushSourceFlags(ULONG *pFlags)
{
	*pFlags = 0;
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CSourceStreamEx::SetPushSourceFlags(ULONG Flags)
{
	UNREFERENCED_PARAMETER(Flags);
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CSourceStreamEx::SetStreamOffset(
													REFERENCE_TIME rtOffset)
{
	UNREFERENCED_PARAMETER(rtOffset);
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CSourceStreamEx::GetStreamOffset(
													REFERENCE_TIME *prtOffset)
{
	UNREFERENCED_PARAMETER(prtOffset);
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CSourceStreamEx::GetMaxStreamOffset(
												REFERENCE_TIME *prtMaxOffset)
{
	UNREFERENCED_PARAMETER(prtMaxOffset);
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CSourceStreamEx::SetMaxStreamOffset(
												REFERENCE_TIME rtMaxOffset)
{
	UNREFERENCED_PARAMETER(rtMaxOffset);
	return E_NOTIMPL;
}


// implement IAMLatency
HRESULT STDMETHODCALLTYPE CSourceStreamEx::GetLatency(
												REFERENCE_TIME *prtLatency)
{
	UNREFERENCED_PARAMETER(prtLatency);
	return E_NOTIMPL;
}


// override

HRESULT CSourceStreamEx::DecideBufferSize(IMemAllocator *pAlloc,
							 ALLOCATOR_PROPERTIES *pProperties)
{
	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pProperties, E_POINTER);

	CAutoLock lock(m_pFilter->pStateLock());
	
	HRESULT hr = DecideBufferSize(pProperties);	// set buffer info
	if (hr != S_OK) {
		return hr;
	}

	ASSERT(pProperties->cbBuffer);

	ALLOCATOR_PROPERTIES actual;
	hr = pAlloc->SetProperties(pProperties, &actual);
	if (FAILED(hr)) {
		return hr;
	}

	if (actual.cbBuffer < pProperties->cbBuffer) {
		return E_FAIL;
	}

	ASSERT(actual.cBuffers == 1);
	return NOERROR;
}


HRESULT CSourceStreamEx::Run(REFERENCE_TIME tStart)
{
	ASSERT(0 <= m_rtDefaultRepeatTime);
	
	{
		CAutoLock lock(&m_cSharedState);

		m_bInitStreamTime = TRUE;
		m_rtRepeatTime = m_rtDefaultRepeatTime;
		::SetEvent(m_hStreamTime);
		m_rtSampleTime = 0;
	}

	return CBaseOutputPin::Run(tStart);
}


HRESULT CSourceStreamEx::OnThreadCreate()
{
	ASSERT(0 <= m_rtDefaultRepeatTime);
	
	CAutoLock lock(&m_cSharedState);

	m_bInitStreamTime = FALSE;
	m_rtRepeatTime = m_rtDefaultRepeatTime;
	m_rtSampleTime = 0;
	
#ifdef _DEBUG
	m_nFrameCount = 0;
	m_nPrerollFrameCount = 0;
#endif
	
	return NOERROR;
}


HRESULT CSourceStreamEx::Inactive(void)
{
	SetEvent();
	return CSourceStream::Inactive();
}


STDMETHODIMP CSourceStreamEx::Notify(IBaseFilter * pSender, Quality q)
{
	ASSERT(0 <= m_rtDefaultRepeatTime);

	CAutoLock lock(&m_cSharedState);
	
	if (q.Proportion <= 0) {
		m_rtRepeatTime = m_rtDefaultRepeatTime;
	} else {
		m_rtRepeatTime = m_rtRepeatTime * 1000 / q.Proportion;
		
		if (m_rtMaxRepeatTime < m_rtRepeatTime) {
			m_rtRepeatTime = m_rtMaxRepeatTime;
		} else if (m_rtRepeatTime < m_rtMinRepeatTime) {
			m_rtRepeatTime = m_rtMinRepeatTime;
		}
	}

	if (0 < q.Late) {
		m_rtSampleTime += q.Late;
	}

	return NOERROR;
}


HRESULT CSourceStreamEx::SetDefaultRepeateTime(REFERENCE_TIME defaultTime,
							REFERENCE_TIME minTime, REFERENCE_TIME maxTime)
{
	CAutoLock lock(&m_cSharedState);

	if (defaultTime <= 0 || defaultTime < minTime || maxTime < defaultTime) {
		return E_INVALIDARG;
	}

	m_rtDefaultRepeatTime = defaultTime;
	m_rtMinRepeatTime = minTime;
	m_rtMaxRepeatTime = maxTime;
	m_rtRepeatTime = m_rtDefaultRepeatTime;
	::SetEvent(m_hStreamTime);

	return S_OK;
}


HRESULT CSourceStreamEx::DoBufferProcessingLoop(void)
{
	Command com;

	OnThreadStartPlay();

#ifdef _DEBUG
	m_nFrameCount = 0;
	m_nPrerollFrameCount = 0;
#endif

	do {
		while (!CheckRequest(&com)) {
			IMediaSample *pSample;
			HRESULT hr = GetDeliveryBuffer(&pSample,NULL,NULL,0);
			if (FAILED(hr)) {
				Sleep(1);
				continue;
				// go round again. Perhaps the error will go away
				// or the allocator is decommited & we will be asked to
				// exit soon.
			}

			// Virtual function user will override.
			hr = FillBuffer(pSample);

#ifdef _DEBUG
			m_nFrameCount++;
			if (!m_bInitStreamTime)
				m_nPrerollFrameCount++;
#endif

			if (hr == S_OK) {
				if (0 < pSample->GetActualDataLength()) {
					hr = Deliver(pSample);
				}
				pSample->Release();

				// downstream filter returns S_FALSE if it wants us to
				// stop or an error if it's reporting an error.
				if(hr != S_OK) {
					DbgLog((LOG_TRACE, 2,
							TEXT("Deliver() returned %08x; stopping"), hr));
					return S_OK;
				}
			} else if (hr == S_FALSE) {
				// derived class wants us to stop pushing data
				pSample->Release();
				DeliverEndOfStream();
				return S_OK;
			} else {
				// derived class encountered an error
				pSample->Release();
				DbgLog((LOG_ERROR, 1,
							TEXT("Error %08lX from FillBuffer!!!"), hr));
				DeliverEndOfStream();
				m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
				return hr;
			}
			// all paths release the sample
		}

		// For all commands sent to us there must be a Reply call!

		if (com == CMD_RUN || com == CMD_PAUSE) {
			Reply(NOERROR);
		} else if (com != CMD_STOP) {
			Reply((DWORD) E_UNEXPECTED);
			DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
		}
	} while (com != CMD_STOP);

	return S_FALSE;
}


HRESULT CSourceStreamEx::Wait(REFERENCE_TIME streamTime, DWORD timeout)
{
	DWORD_PTR dwCookie = 0;
	IReferenceClock* pClock;
	
	HRESULT hr = m_pFilter->GetSyncSource(&pClock);
	KASSERT(hr == S_OK);
	if (hr != S_OK) {
		return hr;
	}

	REFERENCE_TIME now;
	pClock->GetTime(&now);

	hr = pClock->AdviseTime(now, streamTime, (HEVENT)m_hStreamTime, &dwCookie);
	KASSERT(hr == S_OK);
	pClock->Release();

	if (hr != S_OK) {
		return hr;
	}
	
	if (::WaitForSingleObject(m_hStreamTime, timeout) == WAIT_OBJECT_0) {
		return S_OK;
	}

	return S_FALSE;
}


HRESULT CSourceStreamEx::WaitForSampleTime(DWORD timeout)
{
	if (!m_bInitStreamTime) {
		return E_FAIL;
	}

	CRefTime rtStream;
	HRESULT hr = m_pFilter->StreamTime(rtStream);
	if (hr != S_OK) {
		return hr;
	}

	if (rtStream.m_time < m_rtSampleTime) {
		REFERENCE_TIME wait = m_rtSampleTime - rtStream.m_time;
		return Wait(wait, timeout);
	}

	return S_FALSE;
}


void CSourceStreamEx::SetEvent()
{
	HANDLE hEvent = m_hStreamTime;
	if (hEvent) {
		::SetEvent(hEvent);
	}
}
