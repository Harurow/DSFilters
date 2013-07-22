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

#include <initguid.h>
#include "DSFiltersGuids.h"

// include filter/interface headers
#include "MediaSampleMonitor.h"
#include "VideoResizer.h"
#include "VideoMux.h"


// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {
	{
		L"Media Sample Monitor",
		&CLSID_MediaSampleMonitor,
		CMediaSampleMonitor::CreateInstance,
		NULL,
		&sudMediaSampleMonitor
	},
	{
		L"Video Resizer",
		&CLSID_VideoResizer,
		CVideoResizer::CreateInstance,
		NULL,
		&sudVideoResizer
	},
	{
		L"Video Mux",
		&CLSID_VideoMux,
		CVideoMux::CreateInstance,
		NULL,
		&sudVideoMux
	},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
