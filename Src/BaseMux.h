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


class CBaseMux;

/////////////////////////////////////////////////////////////////////////////
// CBaseMuxInputPin

class CBaseMuxInputPin : public CTransformInputPin
{
public:
	CBaseMuxInputPin(
		LPCTSTR pObjectName,
		CBaseMux *pFilter,
		HRESULT * phr,
		LPCWSTR pName,
		BOOL bSync);
#ifdef UNICODE
	CBaseMuxInputPin(
		LPCSTR pObjectName,
		CBaseMux *pFilter,
		HRESULT * phr,
		LPCWSTR pName,
		BOOL bSync);
#endif

	STDMETHODIMP Receive(IMediaSample * pSample);

private:
	friend class CBaseMux;

	CBaseMux* m_pMux;
	BOOL m_bSync;

protected:
	BOOL IsSyncPin() { return m_bSync; }
	void SetSyncPin(BOOL sync) { m_bSync = sync; }
};


/////////////////////////////////////////////////////////////////////////////
// CBaseMux

class CBaseMux : public CTransformFilter
{
#ifdef _DEBUG
protected:
	long m_nFrameCount;
#endif
public:
	CBaseMux(LPCTSTR pName, LPUNKNOWN pUnk, REFCLSID clsid);
#ifdef UNICODE
	CBaseMux(LPCSTR pName, LPUNKNOWN pUnk, REFCLSID clsid);
#endif
	~CBaseMux();

public:
	// override
	int GetPinCount() { return 3; }
	CBasePin * GetPin(int n);
	STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

	STDMETHODIMP Stop();

	HRESULT DecideBufferSize(IMemAllocator *pAlloc,
							 ALLOCATOR_PROPERTIES *pProp);

	virtual HRESULT ReceiveSlave(IMediaSample *pSample) PURE;

protected:
	HRESULT BuildPins();
	virtual CBaseMuxInputPin* CreateInputPin(BOOL bMaster, HRESULT* phr);
	virtual CTransformOutputPin* CreateOutputPin(HRESULT* phr);
	virtual HRESULT DecideBufferSize(AM_MEDIA_TYPE* pmt,
									 ALLOCATOR_PROPERTIES* pProp) PURE;

protected:
	friend class CBaseMuxInputPin;
	CBaseMuxInputPin *m_pSlaveInput;
};
