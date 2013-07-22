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
#include <measure.h>

#include <Strsafe.h>
#include "BaseMux.h"


#define MUX_MAX_PINS	(1000)

//////////////////////////////////////////////////////////////////////////////
// CBaseMuxInputPin

CBaseMuxInputPin::CBaseMuxInputPin(LPCTSTR pObjectName,
								   CBaseMux *pFilter, HRESULT* phr,
								   LPCWSTR pName, BOOL bSync)
	: CTransformInputPin(pObjectName, pFilter, phr, pName)
	, m_pMux(pFilter)
	, m_bSync(bSync)
{
}


#ifdef UNICODE
CBaseMuxInputPin::CBaseMuxInputPin(LPCSTR pObjectName,
								   CBaseMux *pFilter, HRESULT* phr,
								   LPCWSTR pName, BOOL bSync)
	: CTransformInputPin(pObjectName, pFilter, phr, pName)
	, m_pMux(pFilter)
	, m_bSync(bSync)
{
}
#endif


STDMETHODIMP CBaseMuxInputPin::Receive(IMediaSample * pSample)
{
	ASSERT(pSample);

	// check all is well with the base class
	HRESULT hr = CBaseInputPin::Receive(pSample);

	if (S_OK == hr) {
		if (m_bSync) {
			CAutoLock lck(&m_pMux->m_csReceive);
			hr = m_pMux->Receive(pSample);
		} else {
			hr = m_pMux->ReceiveSlave(pSample);
		}
	}

	return hr;
}


//////////////////////////////////////////////////////////////////////////////
// CBaseMux


CBaseMux::CBaseMux(LPCTSTR pName, LPUNKNOWN pUnk, REFCLSID clsid)
	: CTransformFilter(pName, pUnk, clsid)
	, m_nFrameCount(0)
{
}


#ifdef UNICODE
CBaseMux::CBaseMux(LPCSTR pName, LPUNKNOWN pUnk, REFCLSID clsid)
	: CTransformFilter(pName, pUnk, clsid)
	, m_nFrameCount(0)
{
}
#endif


CBaseMux::~CBaseMux()
{
	delete m_pSlaveInput;
}


CBasePin * CBaseMux::GetPin(int n)
{
	if (n < 0)
		return NULL;

	if (m_pInput == NULL) {
		if (BuildPins() != S_OK) {
			return NULL;
		}
	}

	switch(n) {
	case 0:	return m_pInput;
	case 1: return m_pSlaveInput;
	case 2: return m_pOutput;
	}
	return NULL;
}


STDMETHODIMP CBaseMux::FindPin(LPCWSTR Id, IPin **ppPin)
{
	CheckPointer(ppPin, E_POINTER);
	ValidateReadWritePtr(ppPin, sizeof(IPin *));

	if (0==lstrcmpW(Id,L"In") || 0==lstrcmpW(Id,L"In1")) {
		*ppPin = GetPin(0);
	} else if (0==lstrcmpW(Id,L"In2")) {
		*ppPin = GetPin(1);
	} else if (0==lstrcmpW(Id,L"Out")) {
		*ppPin = GetPin(2);
	} else {
		*ppPin = NULL;
		return VFW_E_NOT_FOUND;
	}

	HRESULT hr = NOERROR;
	if (*ppPin) {
		(*ppPin)->AddRef();
	} else {
		hr = E_OUTOFMEMORY;
	}
	return hr;
}


STDMETHODIMP CBaseMux::Stop()
{
#ifdef _DEBUG
	m_nFrameCount = 0;
#endif

	CAutoLock lck1(&m_csFilter);
	if (m_State == State_Stopped) {
		return NOERROR;
	}

	// Succeed the Stop if we are not completely connected

	ASSERT(m_pInput == NULL || m_pOutput != NULL);
	if (m_pInput == NULL || m_pInput->IsConnected() == FALSE ||
				m_pOutput->IsConnected() == FALSE) {
		m_State = State_Stopped;
		m_bEOSDelivered = FALSE;
		return NOERROR;
	}

	ASSERT(m_pInput);
	ASSERT(m_pSlaveInput);
	ASSERT(m_pOutput);

	// decommit the input pin before locking or we can deadlock
	m_pInput->Inactive();
	m_pSlaveInput->Inactive();

	// synchronize with Receive calls

	CAutoLock lck2(&m_csReceive);
	m_pOutput->Inactive();

	// allow a class derived from CTransformFilter
	// to know about starting and stopping streaming

	HRESULT hr = StopStreaming();
	if (SUCCEEDED(hr)) {
		// complete the state transition
		m_State = State_Stopped;
		m_bEOSDelivered = FALSE;
	}
	return hr;
}


HRESULT CBaseMux::DecideBufferSize(IMemAllocator *pAlloc, 
								   ALLOCATOR_PROPERTIES* pProp)
{
	AM_MEDIA_TYPE mt;
	HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
	if (FAILED(hr)) {
		return hr;
	}

	pProp->cBuffers = 1;
	pProp->cbBuffer = 0;
	pProp->cbAlign = 1;
	pProp->cbPrefix = 0;

	hr = DecideBufferSize(&mt, pProp);

	ASSERT(pProp->cBuffers >= 1);
	ASSERT(pProp->cbBuffer > 0);
	ASSERT(pProp->cbAlign >= 1);
	ASSERT(pProp->cbPrefix >= 0);

	FreeMediaType(mt);

	if (FAILED(hr)) {
		return hr;
	}

	ALLOCATOR_PROPERTIES actual;
	hr = pAlloc->SetProperties(pProp, &actual);
	if (FAILED(hr)) {
		return hr;
	}

	if (pProp->cbBuffer > actual.cbBuffer) {
		return E_FAIL;
	}
	
	return S_OK;
}


HRESULT CBaseMux::BuildPins()
{
	HRESULT hr = S_OK;
	m_pInput = CreateInputPin(TRUE, &hr);
	if (m_pInput == NULL || hr != S_OK) {
		delete m_pInput;
		m_pInput = NULL;
		return hr;
	}

	m_pSlaveInput = CreateInputPin(FALSE, &hr);
	if (m_pSlaveInput == NULL || hr != S_OK) {
		delete m_pInput;
		m_pInput = NULL;
		delete m_pSlaveInput;
		m_pSlaveInput = NULL;
		return hr;
	}

	m_pOutput = CreateOutputPin(&hr);
	if (m_pOutput == NULL || hr != S_OK) {
		delete m_pInput;
		m_pInput = NULL;
		delete m_pSlaveInput;
		m_pSlaveInput = NULL;
		delete m_pOutput;
		m_pOutput = NULL;
		return hr;
	}

	return hr;
}


CBaseMuxInputPin* CBaseMux::CreateInputPin(BOOL bMaster, HRESULT* phr)
{
	LPCWSTR pMaster = L"Master In";
	LPCWSTR pSlave = L"Slave In";

	return new CBaseMuxInputPin(NAME("BaseMuxInputPin"), this, phr,
								(bMaster ? pMaster : pSlave), bMaster);
}


CTransformOutputPin* CBaseMux::CreateOutputPin(HRESULT* phr)
{
	return new CTransformOutputPin(NAME("TransformOutputPin"), this, phr,
									   L"XForm Out");
}
