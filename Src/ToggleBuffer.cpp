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

#include "ToggleBuffer.h"



// �Œ�T�C�Y�̃o�b�t�@��2�쐬���A��/���o�b�t�@�Ƃ��č쐬�Ǘ�����N���X�B
// �e�o�b�t�@�͓����T�C�Y(cbBlockSize)�B

CToggleBuffer::CToggleBuffer(int cbBlockSize)
	: m_cbBlockSize(cbBlockSize)
	, m_nDataIndex(0)
{
	ASSERT(cbBlockSize > 0);

	// �u���b�N�T�C�Y�ƌ��ŘA�������̈���m��
	m_pBuf = new BYTE[cbBlockSize * 2];
	if (m_pBuf == NULL) {
		m_cbBlockSize = 0;
	}
}


CToggleBuffer::~CToggleBuffer()
{
	delete [] m_pBuf;
}


void CToggleBuffer::CopyData(BYTE* pDst, int cbSize)
{
	CAutoLock lock(&m_csLock);
	::CopyMemory(pDst, GetData(), cbSize);
}


void CToggleBuffer::Toggle()
{
	CAutoLock lock(&m_csLock);
	m_nDataIndex = GetBufferIndex();
}


void CToggleBuffer::Toggle(BYTE* pBuf, int cbSize)
{
	ASSERT(pBuf);
	ASSERT(cbSize <= m_cbBlockSize);

	CAutoLock lock(&m_csLock);

	int cbCopy = min(cbSize, m_cbBlockSize);
	
	::CopyMemory(GetBuffer(), pBuf, cbCopy);

	m_nDataIndex = GetBufferIndex();
}


void CToggleBuffer::Clear()
{
	CAutoLock lock(&m_csLock);

	memset(m_pBuf, 0x00, m_cbBlockSize * 2);
}