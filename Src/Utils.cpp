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

#include <Dvdmedia.h>

#include "Utils.h"

#define TIME_FACTOR	(10000)

struct NamedGuid
{
	const GUID *pguid;
	const TCHAR *psz;
};

static const NamedGuid g_MediaTypes[] =
{
	{&MEDIATYPE_NULL, TEXT("NULL\0")},
	{&MEDIATYPE_Video, TEXT("Video\0")},
	{&MEDIATYPE_Audio, TEXT("Audio\0")},
	{&MEDIATYPE_Text, TEXT("Text\0")},
	{&MEDIATYPE_Midi, TEXT("Midi\0")},
	{&MEDIATYPE_Stream, TEXT("Stream\0")},
	{&MEDIATYPE_Interleaved, TEXT("Interleaved\0")},
	{&MEDIATYPE_File, TEXT("File\0")},
	{&MEDIATYPE_ScriptCommand, TEXT("ScriptCommand\0")},
	{&MEDIATYPE_AUXLine21Data, TEXT("AUXLine21Data\0")},
	{&MEDIATYPE_DTVCCData, TEXT("DTVCCData\0")},
	{&MEDIATYPE_MSTVCaption, TEXT("MSTVCaption\0")},
	{&MEDIATYPE_VBI, TEXT("VBI\0")},
	{&MEDIATYPE_Timecode, TEXT("Timecode\0")},
	{&MEDIATYPE_LMRT, TEXT("LMRT\0")},
	{&MEDIATYPE_URL_STREAM, TEXT("URL_STREAM\0")},
	{&MEDIATYPE_MPEG1SystemStream, TEXT("MPEG1SystemStream\0")},
	{&MEDIATYPE_AnalogVideo, TEXT("AnalogVideo\0")},
	{&MEDIATYPE_AnalogAudio, TEXT("AnalogAudio\0")},
	{0, 0},
};


static const NamedGuid g_MediaSubTypes[] =
{
	{&MEDIASUBTYPE_NULL, TEXT("NULL\0")},
	{&MEDIASUBTYPE_None, TEXT("None\0")},
	{&MEDIASUBTYPE_CLPL, TEXT("CLPL\0")},
	{&MEDIASUBTYPE_YUYV, TEXT("YUYV\0")},
	{&MEDIASUBTYPE_IYUV, TEXT("IYUV\0")},
	{&MEDIASUBTYPE_YVU9, TEXT("YVU9\0")},
	{&MEDIASUBTYPE_Y411, TEXT("Y411\0")},
	{&MEDIASUBTYPE_Y41P, TEXT("Y41P\0")},
	{&MEDIASUBTYPE_YUY2, TEXT("YUY2\0")},
	{&MEDIASUBTYPE_YVYU, TEXT("YVYU\0")},
	{&MEDIASUBTYPE_UYVY, TEXT("UYVY\0")},
	{&MEDIASUBTYPE_Y211, TEXT("Y211\0")},
	{&MEDIASUBTYPE_CLJR, TEXT("CLJR\0")},
	{&MEDIASUBTYPE_IF09, TEXT("IF09\0")},
	{&MEDIASUBTYPE_CPLA, TEXT("CPLA\0")},
	{&MEDIASUBTYPE_MJPG, TEXT("MJPG\0")},
	{&MEDIASUBTYPE_TVMJ, TEXT("TVMJ\0")},
	{&MEDIASUBTYPE_WAKE, TEXT("WAKE\0")},
	{&MEDIASUBTYPE_CFCC, TEXT("CFCC\0")},
	{&MEDIASUBTYPE_IJPG, TEXT("IJPG\0")},
	{&MEDIASUBTYPE_Plum, TEXT("Plum\0")},
	{&MEDIASUBTYPE_DVCS, TEXT("DVCS\0")},
	{&MEDIASUBTYPE_H264, TEXT("H264\0")},
	{&MEDIASUBTYPE_DVSD, TEXT("DVSD\0")},
	{&MEDIASUBTYPE_MDVF, TEXT("MDVF\0")},
	{&MEDIASUBTYPE_RGB1, TEXT("RGB1\0")},
	{&MEDIASUBTYPE_RGB4, TEXT("RGB4\0")},
	{&MEDIASUBTYPE_RGB8, TEXT("RGB8\0")},
	{&MEDIASUBTYPE_RGB565, TEXT("RGB565\0")},
	{&MEDIASUBTYPE_RGB555, TEXT("RGB555\0")},
	{&MEDIASUBTYPE_RGB24, TEXT("RGB24\0")},
	{&MEDIASUBTYPE_RGB32, TEXT("RGB32\0")},
	{&MEDIASUBTYPE_ARGB1555, TEXT("ARGB1555\0")},
	{&MEDIASUBTYPE_ARGB4444, TEXT("ARGB4444\0")},
	{&MEDIASUBTYPE_ARGB32, TEXT("ARGB32\0")},
	{&MEDIASUBTYPE_A2R10G10B10, TEXT("A2R10G10B10\0")},
	{&MEDIASUBTYPE_A2B10G10R10, TEXT("A2B10G10R10\0")},
	{&MEDIASUBTYPE_AYUV, TEXT("AYUV\0")},
	{&MEDIASUBTYPE_AI44, TEXT("AI44\0")},
	{&MEDIASUBTYPE_IA44, TEXT("IA44\0")},
	{&MEDIASUBTYPE_RGB32_D3D_DX7_RT, TEXT("RGB32_D3D_DX7_RT\0")},
	{&MEDIASUBTYPE_RGB16_D3D_DX7_RT, TEXT("RGB16_D3D_DX7_RT\0")},
	{&MEDIASUBTYPE_ARGB32_D3D_DX7_RT, TEXT("ARGB32_D3D_DX7_RT\0")},
	{&MEDIASUBTYPE_ARGB4444_D3D_DX7_RT, TEXT("ARGB4444_D3D_DX7_RT\0")},
	{&MEDIASUBTYPE_ARGB1555_D3D_DX7_RT, TEXT("ARGB1555_D3D_DX7_RT\0")},
	{&MEDIASUBTYPE_RGB32_D3D_DX9_RT, TEXT("RGB32_D3D_DX9_RT\0")},
	{&MEDIASUBTYPE_RGB16_D3D_DX9_RT, TEXT("RGB16_D3D_DX9_RT\0")},
	{&MEDIASUBTYPE_ARGB32_D3D_DX9_RT, TEXT("ARGB32_D3D_DX9_RT\0")},
	{&MEDIASUBTYPE_ARGB4444_D3D_DX9_RT, TEXT("ARGB4444_D3D_DX9_RT\0")},
	{&MEDIASUBTYPE_ARGB1555_D3D_DX9_RT, TEXT("ARGB1555_D3D_DX9_RT\0")},
	{&MEDIASUBTYPE_YV12, TEXT("YV12\0")},
	{&MEDIASUBTYPE_NV12, TEXT("NV12\0")},
	{&MEDIASUBTYPE_NV24, TEXT("NV24\0")},
	{&MEDIASUBTYPE_IMC1, TEXT("IMC1\0")},
	{&MEDIASUBTYPE_IMC2, TEXT("IMC2\0")},
	{&MEDIASUBTYPE_IMC3, TEXT("IMC3\0")},
	{&MEDIASUBTYPE_IMC4, TEXT("IMC4\0")},
	{&MEDIASUBTYPE_S340, TEXT("S340\0")},
	{&MEDIASUBTYPE_S342, TEXT("S342\0")},
	{&MEDIASUBTYPE_Overlay, TEXT("Overlay\0")},
	{&MEDIASUBTYPE_MPEG1Packet, TEXT("MPEG1Packet\0")},
	{&MEDIASUBTYPE_MPEG1Payload, TEXT("MPEG1Payload\0")},
	{&MEDIASUBTYPE_MPEG1AudioPayload, TEXT("MPEG1AudioPayload\0")},
	{&MEDIASUBTYPE_MPEG1System, TEXT("MPEG1System\0")},
	{&MEDIASUBTYPE_MPEG1VideoCD, TEXT("MPEG1VideoCD\0")},
	{&MEDIASUBTYPE_MPEG1Video, TEXT("MPEG1Video\0")},
	{&MEDIASUBTYPE_MPEG1Audio, TEXT("MPEG1Audio\0")},
	{&MEDIASUBTYPE_Avi, TEXT("Avi\0")},
	{&MEDIASUBTYPE_Asf, TEXT("Asf\0")},
	{&MEDIASUBTYPE_QTMovie, TEXT("QTMovie\0")},
	{&MEDIASUBTYPE_QTRpza, TEXT("QTRpza\0")},
	{&MEDIASUBTYPE_QTSmc, TEXT("QTSmc\0")},
	{&MEDIASUBTYPE_QTRle, TEXT("QTRle\0")},
	{&MEDIASUBTYPE_QTJpeg, TEXT("QTJpeg\0")},
	{&MEDIASUBTYPE_PCMAudio_Obsolete, TEXT("PCMAudio_Obsolete\0")},
	{&MEDIASUBTYPE_PCM, TEXT("PCM\0")},
	{&MEDIASUBTYPE_WAVE, TEXT("WAVE\0")},
	{&MEDIASUBTYPE_AU, TEXT("AU\0")},
	{&MEDIASUBTYPE_AIFF, TEXT("AIFF\0")},
	{&MEDIASUBTYPE_dvsd, TEXT("dvsd\0")},
	{&MEDIASUBTYPE_dvhd, TEXT("dvhd\0")},
	{&MEDIASUBTYPE_dvsl, TEXT("dvsl\0")},
	{&MEDIASUBTYPE_dv25, TEXT("dv25\0")},
	{&MEDIASUBTYPE_dv50, TEXT("dv50\0")},
	{&MEDIASUBTYPE_dvh1, TEXT("dvh1\0")},
	{&MEDIASUBTYPE_Line21_BytePair, TEXT("Line21_BytePair\0")},
	{&MEDIASUBTYPE_Line21_GOPPacket, TEXT("Line21_GOPPacket\0")},
	{&MEDIASUBTYPE_Line21_VBIRawData, TEXT("Line21_VBIRawData\0")},
	{&MEDIASUBTYPE_708_608Data, TEXT("708_608Data\0")},
	{&MEDIASUBTYPE_DtvCcData, TEXT("DtvCcData\0")},
	{&MEDIASUBTYPE_TELETEXT, TEXT("TELETEXT\0")},
	{&MEDIASUBTYPE_WSS, TEXT("WSS\0")},
	{&MEDIASUBTYPE_VPS, TEXT("VPS\0")},
	{&MEDIASUBTYPE_DRM_Audio, TEXT("DRM_Audio\0")},
	{&MEDIASUBTYPE_IEEE_FLOAT, TEXT("IEEE_FLOAT\0")},
	{&MEDIASUBTYPE_DOLBY_AC3_SPDIF, TEXT("DOLBY_AC3_SPDIF\0")},
	{&MEDIASUBTYPE_RAW_SPORT, TEXT("RAW_SPORT\0")},
	{&MEDIASUBTYPE_SPDIF_TAG_241h, TEXT("SPDIF_TAG_241h\0")},
	{&MEDIASUBTYPE_DssVideo, TEXT("DssVideo\0")},
	{&MEDIASUBTYPE_DssAudio, TEXT("DssAudio\0")},
	{&MEDIASUBTYPE_VPVideo, TEXT("VPVideo\0")},
	{&MEDIASUBTYPE_VPVBI, TEXT("VPVBI\0")},
	{&MEDIASUBTYPE_AnalogVideo_NTSC_M, TEXT("AnalogVideo_NTSC_M\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_B, TEXT("AnalogVideo_PAL_B\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_D, TEXT("AnalogVideo_PAL_D\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_G, TEXT("AnalogVideo_PAL_G\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_H, TEXT("AnalogVideo_PAL_H\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_I, TEXT("AnalogVideo_PAL_I\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_M, TEXT("AnalogVideo_PAL_M\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_N, TEXT("AnalogVideo_PAL_N\0")},
	{&MEDIASUBTYPE_AnalogVideo_PAL_N_COMBO, TEXT("AnalogVideo_PAL_N_COMBO\0")},
	{&MEDIASUBTYPE_AnalogVideo_SECAM_B, TEXT("AnalogVideo_SECAM_B\0")},
	{&MEDIASUBTYPE_AnalogVideo_SECAM_D, TEXT("AnalogVideo_SECAM_D\0")},
	{&MEDIASUBTYPE_AnalogVideo_SECAM_G, TEXT("AnalogVideo_SECAM_G\0")},
	{&MEDIASUBTYPE_AnalogVideo_SECAM_H, TEXT("AnalogVideo_SECAM_H\0")},
	{&MEDIASUBTYPE_AnalogVideo_SECAM_K, TEXT("AnalogVideo_SECAM_K\0")},
	{&MEDIASUBTYPE_AnalogVideo_SECAM_K1, TEXT("AnalogVideo_SECAM_K1\0")},
	{&MEDIASUBTYPE_AnalogVideo_SECAM_L, TEXT("AnalogVideo_SECAM_L\0")},
	{0, 0},
};


static const NamedGuid g_MediaFormats[] = 
{
	{&FORMAT_None, TEXT("None\0")},
	{&FORMAT_VideoInfo, TEXT("VideoInfo\0")},
	{&FORMAT_VideoInfo2, TEXT("VideoInfo2\0")},
	{&FORMAT_WaveFormatEx, TEXT("WaveFormatEx\0")},
	{&FORMAT_MPEGVideo, TEXT("MPEGVideo\0")},
	{&FORMAT_MPEGStreams, TEXT("MPEGStreams\0")},
	{&FORMAT_DvInfo, TEXT("DvInfo\0")},
	{&FORMAT_525WSS, TEXT("525WSS\0")},
	{&FORMAT_AnalogVideo, TEXT("AnalogVideo\0")},
	{0, 0},
};


void DbgWndDisplay(CDbgWnd* pWnd, CBaseFilter* pFilter, IMediaSample* pSample,
				long nFrame, REFERENCE_TIME rtStart)
{
	ASSERT(pWnd);
	ASSERT(pFilter);
	ASSERT(pSample);

	FILTER_STATE state;
	pFilter->GetState(0, &state);

	pWnd->SetTextColor(RGB(255, 0, 0));
	pWnd->DrawString(10, 0, TEXT("%s"),
				((state == State_Paused) ? TEXT("Paused") : TEXT("      ")));
	pWnd->SetDefaultTextColor();

	CRefTime rtNow;
	pFilter->StreamTime(rtNow);
	pWnd->DrawString(10, 10, TEXT("Now       : %010lld(%010lld), %05ld"),
				rtNow.m_time/TIME_FACTOR,
				(rtNow.m_time + rtStart)/TIME_FACTOR, nFrame);
	
	// sample time
	REFERENCE_TIME rtStt = 0, rtEnd = 0;
	pSample->GetTime(&rtStt, &rtEnd);
	pWnd->DrawString(10, 30, TEXT("time      : %010lld + %05lld"),
							rtStt/TIME_FACTOR, (rtEnd - rtStt)/TIME_FACTOR);

	LONGLONG mtStt = 0, mtEnd = 0;
	pSample->GetMediaTime(&mtStt, &mtEnd);
	pWnd->DrawString(10, 50, TEXT("media-time: %06lld + %06lld"),
							mtStt, mtEnd - mtStt);

	LONG nSize = pSample->GetSize();
	LONG nActualSize = pSample->GetActualDataLength();
	pWnd->DrawString(10, 70, TEXT("data-size : %05ld/%05ld"), nActualSize,
							nSize);

	pWnd->DrawString(10, 90,
		TEXT("Sync : %s, Preroll : %s, Discont : %s"),
		((pSample->IsSyncPoint() == S_OK) ? TEXT("Yes") : TEXT("No ")),
		((pSample->IsPreroll() == S_OK) ? TEXT("Yes") : TEXT("No ")),
		((pSample->IsDiscontinuity() == S_OK) ? TEXT("Yes") : TEXT("No ")));
}


const LPCTSTR FindName(const NamedGuid* pTable, const GUID *pGuid)
{
	for (int i = 0; pTable[i].pguid != 0; i++) {
		if (*pGuid == *(pTable[i].pguid)) {
			return pTable[i].psz;
		}
	}

	return NULL;
}


HRESULT GetName(const NamedGuid* pTable, LPTSTR pszString, int cchBuf,
				const GUID *pGuid)
{
	ASSERT(pTable);
	ASSERT(pszString);
	ASSERT(pGuid);

	if (cchBuf < 1) {
		return E_INVALIDARG;
	}

	HRESULT hr = E_FAIL;

	pszString[0] = TEXT('\0');

	const LPCTSTR pName = FindName(pTable, pGuid);
	if (pName) 	{
		if (_tcscpy_s(pszString, cchBuf, pName) == 0) {
			return S_OK;
		}
	}

	if (FAILED(hr)) {
		hr = StringFromGUID2(*pGuid, pszString, cchBuf);
	}

	return hr;
}


HRESULT GetMediaTypeName(LPTSTR pszString, int cchBuf, const GUID *pGuid)
{
	return GetName(g_MediaTypes, pszString, cchBuf, pGuid);
}


HRESULT GetMediaSubTypeName(LPTSTR pszString, int cchBuf, const GUID *pGuid)
{
	return GetName(g_MediaSubTypes, pszString, cchBuf, pGuid);
}


HRESULT GetFormatName(LPTSTR pszString, int cchBuf, const GUID *pGuid)
{
	return GetName(g_MediaFormats, pszString, cchBuf, pGuid);
}

int GetBmpBits(const GUID* pSubtype)
{
	if (*pSubtype == MEDIASUBTYPE_RGB1) {
		return 1;
	} else if (*pSubtype == MEDIASUBTYPE_RGB4) {
		return 4;
	} else if (*pSubtype == MEDIASUBTYPE_RGB8) {
		return 8;
	} else if (*pSubtype == MEDIASUBTYPE_RGB565
			|| *pSubtype == MEDIASUBTYPE_RGB555
			|| *pSubtype == MEDIASUBTYPE_ARGB1555
			|| *pSubtype == MEDIASUBTYPE_ARGB4444)	{
		return 16;
	} else if (*pSubtype == MEDIASUBTYPE_RGB24) {
		return 24;
	} else if (*pSubtype == MEDIASUBTYPE_RGB32
			|| *pSubtype == MEDIASUBTYPE_ARGB32
			|| *pSubtype == MEDIASUBTYPE_A2R10G10B10
			|| *pSubtype == MEDIASUBTYPE_A2B10G10R10) { 
		return 32;
	}

	return -1;
}


#ifdef DEBUG

void DbgLogMediaTypeInfo(DWORD type, DWORD level, const CMediaType* pmt)
{
	DbgLogInfo(type, level,
		TEXT(" MediaType Info : %s %s %s %s SampleSize=%ld"),
			FindName(g_MediaTypes, &pmt->majortype),
			FindName(g_MediaSubTypes, &pmt->subtype),
			(pmt->bFixedSizeSamples 
				? TEXT("FixedSamples")
				: TEXT("NotFixedSamples")),
			(pmt->bTemporalCompression
				? TEXT("TemporalCompression")
				: TEXT("NotTemporalCompression")),
			pmt->lSampleSize);
}


void DbgLogMediaFormatInfo(DWORD type, DWORD level, const CMediaType* pmt)
{
	DbgLogInfo(type, level,
		TEXT(" Format Info : %s FormatSize=%ld"),
			FindName(g_MediaFormats, &pmt->formattype),
			pmt->cbFormat);

	if (pmt->formattype == FORMAT_VideoInfo) {
		if (pmt->pbFormat == NULL || pmt->cbFormat < sizeof(VIDEOINFOHEADER)) {
			DbgLogInfo(type, level, TEXT("  !! INVALID FORMAT !!"));
			return;
		}

		VIDEOINFOHEADER* pvh = (VIDEOINFOHEADER*)pmt->pbFormat;
		DbgLogInfo(type, level, TEXT("  Source : %ld, %ld, %ld, %ld"),
				pvh->rcSource.left, pvh->rcSource.top,
				pvh->rcSource.right, pvh->rcSource.bottom);
		DbgLogInfo(type, level, TEXT("  Target : %ld, %ld, %ld, %ld"),
				pvh->rcTarget.left, pvh->rcTarget.top,
				pvh->rcTarget.right, pvh->rcTarget.bottom);
		DbgLogInfo(type, level, TEXT("  BitRate / BitErrorRate : %lu / %lu"),
				pvh->dwBitRate, pvh->dwBitErrorRate);
		DbgLogInfo(type, level, TEXT("  AvgTimePerFrame : %lld"),
				pvh->AvgTimePerFrame);
		DbgLogInfo(type, level, TEXT("  Header Size=%lu"),
				pvh->bmiHeader.biSize);
		DbgLogInfo(type, level, TEXT("   Width x Height = %ld x %ld"),
				pvh->bmiHeader.biWidth, pvh->bmiHeader.biHeight);
		DbgLogInfo(type, level, TEXT("   Planes, BitCount = %u, %u"),
				pvh->bmiHeader.biPlanes, pvh->bmiHeader.biBitCount);
		DbgLogInfo(type, level, TEXT("   Compression = %lu"),
				pvh->bmiHeader.biCompression);
		DbgLogInfo(type, level, TEXT("   SizeImage = %lu"),
				pvh->bmiHeader.biSizeImage);
		DbgLogInfo(type, level, TEXT("   PelsPerMeter = %ld x %ld"),
				pvh->bmiHeader.biXPelsPerMeter,
				pvh->bmiHeader.biYPelsPerMeter);
		DbgLogInfo(type, level, TEXT("   ColorUsed(Important) = %lu(%lu)"),
				pvh->bmiHeader.biClrUsed, pvh->bmiHeader.biClrImportant);
	} else if (pmt->formattype == FORMAT_VideoInfo2) {
		if (pmt->pbFormat == NULL 
				|| pmt->cbFormat < sizeof(VIDEOINFOHEADER2)) {
			DbgLogInfo(type, level, TEXT("  !! INVALID FORMAT !!"));
			return;
		}

		VIDEOINFOHEADER2* pvh = (VIDEOINFOHEADER2*)pmt->pbFormat;
		DbgLogInfo(type, level, TEXT("  Source : %ld, %ld, %ld, %ld"),
				pvh->rcSource.left, pvh->rcSource.top,
				pvh->rcSource.right, pvh->rcSource.bottom);
		DbgLogInfo(type, level, TEXT("  Target : %ld, %ld, %ld, %ld"),
				pvh->rcTarget.left, pvh->rcTarget.top,
				pvh->rcTarget.right, pvh->rcTarget.bottom);
		DbgLogInfo(type, level, TEXT("  BitRate / BitErrorRate : %lu / %lu"),
				pvh->dwBitRate, pvh->dwBitErrorRate);
		DbgLogInfo(type, level, TEXT("  AvgTimePerFrame : %lld"),
				pvh->AvgTimePerFrame);
		DbgLogInfo(type, level, TEXT("  InterlaceFlags : 0x%08x"),
				pvh->dwInterlaceFlags);
		DbgLogInfo(type, level, TEXT("  CopyProtectFlags : 0x%08x"),
				pvh->dwCopyProtectFlags);
		DbgLogInfo(type, level, TEXT("  PictAspectRatio : %lu x %lu"),
				pvh->dwPictAspectRatioX, pvh->dwPictAspectRatioY);
		DbgLogInfo(type, level, TEXT("  ControlFlags : 0x%08x"),
				pvh->dwControlFlags);
		DbgLogInfo(type, level, TEXT("  Header Size=%lu"),
				pvh->bmiHeader.biSize);
		DbgLogInfo(type, level, TEXT("   Width x Height = %ld x %ld"),
				pvh->bmiHeader.biWidth, pvh->bmiHeader.biHeight);
		DbgLogInfo(type, level, TEXT("   Planes, BitCount = %u, %u"),
				pvh->bmiHeader.biPlanes, pvh->bmiHeader.biBitCount);
		DbgLogInfo(type, level, TEXT("   Compression = %lu"),
				pvh->bmiHeader.biCompression);
		DbgLogInfo(type, level, TEXT("   SizeImage = %lu"),
				pvh->bmiHeader.biSizeImage);
		DbgLogInfo(type, level, TEXT("   PelsPerMeter = %ld x %ld"),
				pvh->bmiHeader.biXPelsPerMeter,
				pvh->bmiHeader.biYPelsPerMeter);
		DbgLogInfo(type, level, TEXT("   ColorUsed(Important) = %lu(%lu)"),
				pvh->bmiHeader.biClrUsed, pvh->bmiHeader.biClrImportant);
	} else if (pmt->formattype == FORMAT_WaveFormatEx) {
		if (pmt->pbFormat == NULL || sizeof(WAVEFORMATEX) < pmt->cbFormat) {
			DbgLogInfo(type, level, TEXT("  !! INVALID FORMAT !!"));
			return;
		}

		WAVEFORMATEX * pvh = (WAVEFORMATEX *)pmt->pbFormat;
		DbgLogInfo(type, level, TEXT("  FormatTag : %u"), pvh->wFormatTag);
		DbgLogInfo(type, level, TEXT("  Channels : %u"), pvh->nChannels);
		DbgLogInfo(type, level, TEXT("  SamplesPerSec : %lu"),
				pvh->nSamplesPerSec);
		DbgLogInfo(type, level, TEXT("  AvgBytesPerSec : %lu"),
				pvh->nAvgBytesPerSec);
		DbgLogInfo(type, level, TEXT("  BlockAlign : %u"),
				pvh->nBlockAlign);
		DbgLogInfo(type, level, TEXT("  BitsPerSample : %u"),
				pvh->wBitsPerSample);
		DbgLogInfo(type, level, TEXT("  Size : %u"),
				pvh->cbSize);
	}

	if (pmt->formattype == FORMAT_MPEGVideo) {
		if (pmt->pbFormat == NULL || sizeof(MPEG1VIDEOINFO) < pmt->cbFormat) {
			DbgLogInfo(type, level, TEXT("  !! INVALID FORMAT !!"));
			return;
		}

		MPEG1VIDEOINFO* pvh = (MPEG1VIDEOINFO*)pmt->pbFormat;
		DbgLogInfo(type, level, TEXT("  Source : %ld, %ld, %ld, %ld"),
				pvh->hdr.rcSource.left, pvh->hdr.rcSource.top,
				pvh->hdr.rcSource.right, pvh->hdr.rcSource.bottom);
		DbgLogInfo(type, level, TEXT("  Target : %ld, %ld, %ld, %ld"),
				pvh->hdr.rcTarget.left, pvh->hdr.rcTarget.top,
				pvh->hdr.rcTarget.right, pvh->hdr.rcTarget.bottom);
		DbgLogInfo(type, level, TEXT("  BitRate / BitErrorRate : %lu / %lu"),
				pvh->hdr.dwBitRate, pvh->hdr.dwBitErrorRate);
		DbgLogInfo(type, level, TEXT("  AvgTimePerFrame : %lld"),
				pvh->hdr.AvgTimePerFrame);
		DbgLogInfo(type, level, TEXT("  Header Size=%lu"),
				pvh->hdr.bmiHeader.biSize);
		DbgLogInfo(type, level, TEXT("   Width x Height = %ld x %ld"),
				pvh->hdr.bmiHeader.biWidth, pvh->hdr.bmiHeader.biHeight);
		DbgLogInfo(type, level, TEXT("   Planes, BitCount = %u, %u"),
				pvh->hdr.bmiHeader.biPlanes, pvh->hdr.bmiHeader.biBitCount);
		DbgLogInfo(type, level, TEXT("   Compression = %lu"),
				pvh->hdr.bmiHeader.biCompression);
		DbgLogInfo(type, level, TEXT("   SizeImage = %lu"),
				pvh->hdr.bmiHeader.biSizeImage);
		DbgLogInfo(type, level, TEXT("   PelsPerMeter = %ld x %ld"),
				pvh->hdr.bmiHeader.biXPelsPerMeter,
				pvh->hdr.bmiHeader.biYPelsPerMeter);
		DbgLogInfo(type, level, TEXT("   ColorUsed(Important) = %lu(%lu)"),
				pvh->hdr.bmiHeader.biClrUsed,
				pvh->hdr.bmiHeader.biClrImportant);
		DbgLogInfo(type, level, TEXT("   StartTimeCode = %lu"),
				pvh->dwStartTimeCode);
		DbgLogInfo(type, level, TEXT("   SequenceHeader = %lu"),
				pvh->cbSequenceHeader);
	}

	// TODO:残りのフォーマット対応
	// FORMAT_MPEGStreams
	// FORMAT_DvInfo
	// FORMAT_525WSS
	// FORMAT_AnalogVideo
}
#endif
