#include "pch.h"

void DrawStockChart(CDC& cDC, const std::vector<VARIANT>& days, const std::vector<VARIANT>& open, const std::vector<VARIANT>& high, const std::vector<VARIANT>& low, const std::vector<VARIANT>& close, CSize viewSize)
{
    // generate by Grok3

    _ASSERT(days.size() == open.size() && days.size() == high.size() && days.size() == low.size() && days.size() == close.size());

    if (days.empty()) return; // �f�[�^����̏ꍇ�͏I��

    // �`��G���A�̐ݒ�
    const int margin = 50; // �㉺���E�̗]��
    const int chartWidth = viewSize.cx - 2 * margin;
    const int chartHeight = viewSize.cy - 2 * margin;

    // �f�[�^�̍ő�E�ŏ��l���v�Z
    double minPrice = DBL_MAX;
    double maxPrice = -DBL_MAX;
    for (size_t i = 0; i < high.size(); ++i) {
        double h = high[i].dblVal;
        double l = low[i].dblVal;
        minPrice = min(minPrice, l);
        maxPrice = max(maxPrice, h);
    }

    // ���i�̃X�P�[�����O
    double priceRange = maxPrice - minPrice;
    if (priceRange == 0) priceRange = 1.0; // �[�����Z�h�~
    double priceScale = chartHeight / priceRange;

    // ���t�̃X�P�[�����O
    size_t numDays = days.size();
    double barWidth = static_cast<double>(chartWidth) / numDays;
    if (barWidth < 2.0) barWidth = 2.0; // �o�[�̍ŏ���

    // �w�i�𔒂ŃN���A
    cDC.FillSolidRect(0, 0, viewSize.cx, viewSize.cy, RGB(255, 255, 255));

    // ���̕`��
    CPen axisPen(PS_SOLID, 1, RGB(0, 0, 0));
    CPen* oldPen = cDC.SelectObject(&axisPen);
    cDC.MoveTo(margin, margin);
    cDC.LineTo(margin, viewSize.cy - margin); // Y��
    cDC.MoveTo(margin, viewSize.cy - margin);
    cDC.LineTo(viewSize.cx - margin, viewSize.cy - margin); // X��

    // ���i�ڐ���̕`��i�ȈՓI��5�����j
    for (int i = 0; i <= 5; ++i) {
        double price = minPrice + (priceRange * i / 5.0);
        int y = viewSize.cy - margin - static_cast<int>((price - minPrice) * priceScale);
        cDC.MoveTo(margin - 5, y);
        cDC.LineTo(margin + 5, y);
        CString label;
        label.Format(_T("%.2f"), price);
        cDC.TextOut(margin - 40, y - 10, label);
    }

    CFont font45;
    LOGFONT lf = { 0 };
    lf.lfHeight = 20;               // �t�H���g�̍���
    lf.lfEscapement = 450;          // �����̌X�Ίp�x�i1/10�x�P�ʁA45�x = 450�j
    lf.lfOrientation = 450;         // �����̃x�[�X���C���̌X�΁i1/10�x�P�ʁj
    _tcscpy_s(lf.lfFaceName, _T("Arial")); // �t�H���g��
    font45.CreateFontIndirect(&lf);



    // ���[�\�N���̕`��
    for (size_t i = 0; i < numDays; ++i) {
        double o = open[i].dblVal;
        double h = high[i].dblVal;
        double l = low[i].dblVal;
        double c = close[i].dblVal;

        // X���W�i�o�[���S�j
        int x = margin + static_cast<int>(i * barWidth + barWidth / 2);

        // Y���W���v�Z
        int highY = viewSize.cy - margin - static_cast<int>((h - minPrice) * priceScale);
        int lowY = viewSize.cy - margin - static_cast<int>((l - minPrice) * priceScale);
        int openY = viewSize.cy - margin - static_cast<int>((o - minPrice) * priceScale);
        int closeY = viewSize.cy - margin - static_cast<int>((c - minPrice) * priceScale);

        // ���[�\�N���̐F�i�z���F�ԁA�A���F�j
        COLORREF color = (c >= o) ? RGB(255, 0, 0) : RGB(0, 0, 255);
        CPen candlePen(PS_SOLID, 1, color);
        CBrush candleBrush(color);
        cDC.SelectObject(&candlePen);
        cDC.SelectObject(&candleBrush);

        // ���l-���l�̐��i�q�Q�j
        cDC.MoveTo(x, highY);
        cDC.LineTo(x, lowY);

        // ���[�\�N���{��
        int candleWidth = static_cast<int>(barWidth * 0.4); // �o�[�̕���40%
        if (candleWidth < 1) candleWidth = 1;
        CRect candleRect(x - candleWidth, min(openY, closeY), x + candleWidth, max(openY, closeY));
        cDC.Rectangle(candleRect);

        // ���t���x���̕`��i��F5�����j
        if (i % 5 == 0) {
            
            auto oldf = cDC.SelectObject(font45);
            CString dateStr;
            //dateStr.Format(_T("%.0f"), days[i].dblVal); // VARIANT�̓��t������double�Ƃ��Ĉ���
            dateStr = days[i].bstrVal;
            cDC.TextOut(x - 20, viewSize.cy - margin + 10, dateStr);
            cDC.SelectObject(oldf);
        }
    }

    // ���̃y���𕜌�
    cDC.SelectObject(oldPen);
}

