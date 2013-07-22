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

class CToggleBuffer
{
public:
	CToggleBuffer(int cbBlockSize);
	virtual ~CToggleBuffer();

	BOOL IsValid() { return m_pBuf != NULL; }

	int GetBloskSize() { return m_cbBlockSize; }
	int GetDataIndex() { return m_nDataIndex; }
	int GetBufferIndex() { return (m_nDataIndex + 1) & 0x1; }

	CCritSec* GetLock() { return &m_csLock; }
	BYTE* GetData() { return GetPointer(m_nDataIndex); }
	BYTE* GetBuffer() { return GetPointer(GetBufferIndex()); }

	void CopyData(BYTE* pDst, int cbSize);

	void Toggle();
	void Toggle(BYTE* pBuf, int cbSize);

	void Clear();

private:
	BYTE* GetPointer(int i) { return m_pBuf + m_cbBlockSize * i; }

private:
	CCritSec m_csLock;
	BYTE* m_pBuf;
	int m_cbBlockSize;

	int m_nDataIndex;
};
