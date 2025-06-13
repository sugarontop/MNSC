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
		timeInfo.tm_mon + 1,     // ����0-11�Ȃ̂�+1
		timeInfo.tm_mday);       // ��
	
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

	// ���݂̃N���b�s���O�̈��ۑ�
	CRgn clipRgn;
	clipRgn.CreateRectRgnIndirect(&clipRect);

	// �V�����N���b�s���O�̈��ݒ�
	pDC->SelectClipRgn(&clipRgn, RGN_AND);

	return TRUE;
}

// �N���b�s���O�̈���|�b�v�i���ɖ߂��j����֐�
BOOL PopAxisAlignedClip(CDC* pDC)
{
	if (!pDC)
		return FALSE;

	// �N���b�s���O�̈�������i���̏�Ԃɖ߂��j
	pDC->SelectClipRgn(NULL);

	return TRUE;
}