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

#include "RingBuffer.h"



// 固定サイズのバッファを複数作成し、リングバッファとして作成管理するクラス。
// 各バッファは同じサイズ(cbBlockSize)。
// バッファ数(nBlockCount)は2の乗数である必要がある。

CRingBuffer::CRingBuffer(int cbBlockSize, int nBlockCount)
	: m_nStart(0)
	, m_nEnd(0)
	, m_nDataCount(0)
{
	ASSERT(cbBlockSize > 0);
	ASSERT(nBlockCount > 0);

	if ((cbBlockSize & (cbBlockSize-1)) == 0) {
		// nSizeは2の乗数であること
		m_cbBlockSize = cbBlockSize;
		m_nMask = cbBlockSize - 1;
		m_nBlockCount = nBlockCount;

		// ブロックサイズと個数で連続した領域を確保
		m_pBuf = new BYTE[cbBlockSize * nBlockCount];
		if (m_pBuf == NULL) {
			m_cbBlockSize = 0;
			m_nMask = 0;
			m_nBlockCount = 0;
		}
	} else {
		ASSERT(FALSE);
		m_pBuf = NULL;
		m_cbBlockSize = 0;
		m_nMask = 0;
		m_nBlockCount = 0;
	}
}


CRingBuffer::~CRingBuffer()
{
	delete [] m_pBuf;
}


BOOL CRingBuffer::IsEmpty()
{
	CAutoLock lock(&m_csLock);
	return m_nDataCount == 0;
}


BOOL CRingBuffer::IsFull()
{
	CAutoLock lock(&m_csLock);
	return m_nDataCount == m_nBlockCount;
}


int CRingBuffer::GetDataIndex()
{
	CAutoLock lock(&m_csLock);
	return IsEmpty() ? -1 : m_nStart;
}


int CRingBuffer::GetBufferIndex()
{
	CAutoLock lock(&m_csLock);
	return IsFull() ? -1 : m_nEnd;
}


BOOL CRingBuffer::Enqueue(BYTE* pData, int cbSize)
{
	CAutoLock lock(&m_csLock);

	if (m_cbBlockSize < cbSize) {
		ASSERT(FALSE);
		return FALSE;
	}

	if (IsFull()) {
		return FALSE;
	}

	BYTE* pBuf = GetPointer(m_nEnd);
	::CopyMemory(pBuf, pData, cbSize);

	m_nEnd = NextIndex(m_nEnd);
	m_nDataCount++;

	return TRUE;
}


BOOL CRingBuffer::EnforceEnqueue(BYTE* pData, int cbSize)
{
	CAutoLock lock(&m_csLock);

	if (m_cbBlockSize < cbSize) {
		ASSERT(FALSE);
		return FALSE;
	}

	BYTE* pBuf = GetPointer(m_nEnd);
	::CopyMemory(pBuf, pData, cbSize);

	if (IsFull()) {
		m_nStart = NextIndex(m_nStart);
	} else {
		m_nDataCount++;
	}
	m_nEnd = NextIndex(m_nEnd);

	return TRUE;
}


BYTE* CRingBuffer::Peek()
{
	CAutoLock lock(&m_csLock);

	if (IsEmpty()) {
		return NULL;
	}

	return GetPointer(m_nStart);
}


BOOL CRingBuffer::Dequeue(BYTE** ppBuf)
{
	ASSERT(ppBuf);

	CAutoLock lock(&m_csLock);

	if (IsEmpty()) {
		*ppBuf = NULL;
		return FALSE;
	}

	*ppBuf = GetPointer(m_nStart);
	m_nStart = NextIndex(m_nStart);
	m_nDataCount--;

	return TRUE;
}


void CRingBuffer::Clear()
{
	CAutoLock lock(&m_csLock);
	m_nStart = m_nEnd = m_nDataCount = 0;
}
