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

#ifndef __DSFILTERS_GUIDS_H__
#define __DSFILTERS_GUIDS_H__


///////////////////////////////////////////////////////////////////////////////
// Debug/Monitor

// MediaSampleMonitor
// {E9F87D3F-F025-4190-860D-A61C3A1A7DCF}
DEFINE_GUID(CLSID_MediaSampleMonitor,
0xe9f87d3f, 0xf025, 0x4190, 0x86, 0xd, 0xa6, 0x1c, 0x3a, 0x1a, 0x7d, 0xcf);


///////////////////////////////////////////////////////////////////////////////
// Video

// Video Resizer
// {D115E8CE-C27D-4CF8-AF94-102716F06772}
DEFINE_GUID(CLSID_VideoResizer,
0xd115e8ce, 0xc27d, 0x4cf8, 0xaf, 0x94, 0x10, 0x27, 0x16, 0xf0, 0x67, 0x72);


// Video Mux
// {7ABCCD4B-ACDF-450e-84C7-D60C97FA31A2}
DEFINE_GUID(CLSID_VideoMux, 
0x7abccd4b, 0xacdf, 0x450e, 0x84, 0xc7, 0xd6, 0xc, 0x97, 0xfa, 0x31, 0xa2);



///////////////////////////////////////////////////////////////////////////////
// Audio

// Audio Mux
// {2EF59C17-B855-4627-9A32-D5E87E108656}
DEFINE_GUID(CLSID_AudioMux, 
0x2ef59c17, 0xb855, 0x4627, 0x9a, 0x32, 0xd5, 0xe8, 0x7e, 0x10, 0x86, 0x56);



///////////////////////////////////////////////////////////////////////////////
// Net



#endif // __DSFILTERS_GUIDS_H__
