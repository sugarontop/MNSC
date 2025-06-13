#include "pch.h"
#include "x6.h"



VARIANT UnixTime(VARIANT v)
{
	auto number = v.llVal;
	
	time_t unixTime  = (time_t)number;
	struct tm timeInfo;
	localtime_s(&timeInfo, &unixTime);

	WCHAR cb[64];

	wsprintf(cb,L"%02d/%02d",		
		timeInfo.tm_mon + 1,     // 月は0-11なので+1
		timeInfo.tm_mday);       // 日
	
	_variant_t ret(cb);
	return ret.Detach();
}


std::vector<VARIANT> Convert(IVARIANTArray& sar)
{
	std::vector<VARIANT> ar(sar.Count());

	for (int i = 0; i < sar.Count(); i++)
	{
		VARIANT v = {};
		sar.Get(i, &v);
		ar[i] = v;
	}
	return ar;
}
std::vector<VARIANT> ConvertUnixDate(IVARIANTArray& sar)
{
	std::vector<VARIANT> ar(sar.Count());

	for (int i = 0; i < sar.Count(); i++)
	{
		VARIANT v = {};
		sar.Get(i, &v);

		VARIANT vd = UnixTime(v);
		ar[i] = vd;
	}
	return ar;
}

BOOL PushAxisAlignedClip(CDC* pDC, const CRect& clipRect)
{
	if (!pDC)
		return FALSE;

	// 現在のクリッピング領域を保存
	CRgn clipRgn;
	clipRgn.CreateRectRgnIndirect(&clipRect);

	// 新しいクリッピング領域を設定
	pDC->SelectClipRgn(&clipRgn, RGN_AND);

	return TRUE;
}

// クリッピング領域をポップ（元に戻す）する関数
BOOL PopAxisAlignedClip(CDC* pDC)
{
	if (!pDC)
		return FALSE;

	// クリッピング領域を解除（元の状態に戻す）
	pDC->SelectClipRgn(NULL);

	return TRUE;
}