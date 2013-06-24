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

class CSourceStreamEx
	: public CSourceStream
	, public IAMPushSource
{
#ifdef _DEBUG
protected:
	long m_nFrameCount;
	long m_nPrerollFrameCount;
#endif
public:
	DECLARE_IUNKNOWN;

	CSourceStreamEx(LPCTSTR pObjectName, HRESULT *phr, CSource *pms,
					LPCWSTR pName, REFERENCE_TIME defaultRepeatTime);
	CSourceStreamEx(LPCTSTR pObjectName, HRESULT *phr, CSource *pms,
					LPCWSTR pName, REFERENCE_TIME defaultRepeatTime,
					REFERENCE_TIME minRepeatTime,
					REFERENCE_TIME maxRepeatTime);

	virtual ~CSourceStreamEx();

	// add support interface
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// IAMPushSource method
	HRESULT STDMETHODCALLTYPE GetPushSourceFlags(ULONG *pFlags);
	HRESULT STDMETHODCALLTYPE SetPushSourceFlags(ULONG Flags);
	HRESULT STDMETHODCALLTYPE SetStreamOffset(REFERENCE_TIME rtOffset);
	HRESULT STDMETHODCALLTYPE GetStreamOffset(REFERENCE_TIME *prtOffset);
	HRESULT STDMETHODCALLTYPE GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset);
	HRESULT STDMETHODCALLTYPE SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset);

	// IAMLatency method
	HRESULT STDMETHODCALLTYPE GetLatency(REFERENCE_TIME *prtLatency);

	// over ride
	HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
							 ALLOCATOR_PROPERTIES *pProperties);

	HRESULT Run(REFERENCE_TIME tStart);

	HRESULT OnThreadCreate(void);

	HRESULT Inactive(void);

	STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

	HRESULT SetDefaultRepeateTime(REFERENCE_TIME defaultTime,
						REFERENCE_TIME minTime, REFERENCE_TIME maxTime);

	virtual HRESULT DecideBufferSize(ALLOCATOR_PROPERTIES *pProperties) PURE;

protected:
	HRESULT DoBufferProcessingLoop(void);

	HRESULT Wait(REFERENCE_TIME streamTime, DWORD timeout);
	HRESULT WaitForSampleTime(DWORD timeout);
	void SetEvent();

protected:
	CCritSec m_cSharedState;
	BOOL m_bInitStreamTime;

	HANDLE m_hStreamTime;
	REFERENCE_TIME m_rtDefaultRepeatTime;
	REFERENCE_TIME m_rtMinRepeatTime;
	REFERENCE_TIME m_rtMaxRepeatTime;
	REFERENCE_TIME m_rtRepeatTime;
	REFERENCE_TIME m_rtSampleTime;
};
