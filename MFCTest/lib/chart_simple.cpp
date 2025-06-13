#include "pch.h"
void DrawStockChart2(CDC* pDC, const std::vector<std::wstring>& days, const std::vector<std::wstring>& close, CSize sizeview)
{
    _ASSERT(days.size() == close.size());

    if (days.empty() || close.empty())
        return;

    // �}�[�W���̐ݒ�
    const int leftMargin = 60;
    const int rightMargin = 20;
    const int topMargin = 20;
    const int bottomMargin = 60;

    // �`��̈�̌v�Z
    CRect chartRect(leftMargin, topMargin,
        sizeview.cx - rightMargin,
        sizeview.cy - bottomMargin);

    // �I�l�f�[�^�𐔒l�ɕϊ����A�ő�l�E�ŏ��l���擾
    std::vector<double> prices;
    double minPrice = DBL_MAX;
    double maxPrice = -DBL_MAX;

    for (const auto& priceStr : close) {
        double price = _wtof(priceStr.c_str());
        prices.push_back(price);
        minPrice = min(minPrice, price);
        maxPrice = max(maxPrice, price);
    }

    // ���i�͈͂ɗ]�T����������
    double priceRange = maxPrice - minPrice;
    if (priceRange == 0) priceRange = 1; // �[�����Z���
    minPrice -= priceRange * 0.1;
    maxPrice += priceRange * 0.1;
    priceRange = maxPrice - minPrice;

    // �y���ƃu���V�̏���
    CPen gridPen(PS_DOT, 1, RGB(200, 200, 200));
    CPen axisPen(PS_SOLID, 1, RGB(0, 0, 0));
    CPen linePen(PS_SOLID, 2, RGB(0, 100, 200));
    CBrush pointBrush(RGB(255, 0, 0));

    CPen* pOldPen = pDC->SelectObject(&axisPen);

    // ���̕`��
    pDC->MoveTo(chartRect.left, chartRect.top);
    pDC->LineTo(chartRect.left, chartRect.bottom);
    pDC->LineTo(chartRect.right, chartRect.bottom);

    // �O���b�h�̕`��
    pDC->SelectObject(&gridPen);

    // �����i���i�O���b�h�j
    for (int i = 1; i <= 4; i++) {
        int y = chartRect.bottom - (chartRect.Height() * i / 5);
        pDC->MoveTo(chartRect.left, y);
        pDC->LineTo(chartRect.right, y);
    }

    // �c���i���t�O���b�h�j
    int dataCount = static_cast<int>(days.size());
    for (int i = 1; i < dataCount; i++) {
        if (i % max(1, dataCount / 10) == 0) { // �ő�10�{�̃O���b�h��
            int x = chartRect.left + (chartRect.Width() * i / (dataCount - 1));
            pDC->MoveTo(x, chartRect.top);
            pDC->LineTo(x, chartRect.bottom);
        }
    }

    // �������̕`��
    pDC->SelectObject(&linePen);

    std::vector<CPoint> points;
    for (int i = 0; i < dataCount; i++) {
        int x = chartRect.left + (chartRect.Width() * i / max(1, dataCount - 1));
        int y = chartRect.bottom - static_cast<int>((prices[i] - minPrice) / priceRange * chartRect.Height());
        points.push_back(CPoint(x, y));
    }

    // ���̕`��
    if (points.size() >= 2) {
        pDC->MoveTo(points[0]);
        for (size_t i = 1; i < points.size(); i++) {
            pDC->LineTo(points[i]);
        }
    }

    // �f�[�^�|�C���g�̕`��
    CBrush* pOldBrush = pDC->SelectObject(&pointBrush);
    for (const auto& pt : points) {
        pDC->Ellipse(pt.x - 3, pt.y - 3, pt.x + 3, pt.y + 3);
    }

    // ���x���̕`��p�t�H���g
    CFont font;
    font.CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    CFont* pOldFont = pDC->SelectObject(&font);

    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(RGB(0, 0, 0));

    // Y�����x���i���i�j
    for (int i = 0; i <= 5; i++) {
        double price = minPrice + (priceRange * i / 5);
        int y = chartRect.bottom - (chartRect.Height() * i / 5);

        CString priceLabel;
        priceLabel.Format(_T("%.0f"), price);

        CSize textSize = pDC->GetTextExtent(priceLabel);
        pDC->TextOut(chartRect.left - textSize.cx - 5, y - textSize.cy / 2, priceLabel);
    }

    // X�����x���i���t�j
    int labelStep = max(1, dataCount / 8); // �ő�8�̃��x��
    for (int i = 0; i < dataCount; i += labelStep) {
        int x = chartRect.left + (chartRect.Width() * i / max(1, dataCount - 1));

        // ���t�����񂩂猎���̂ݒ��o�iYYYY-MM-DD�`����z��j
        std::wstring dateStr = days[i];
        if (dateStr.length() >= 10) {
            dateStr = dateStr.substr(5, 5); // MM-DD�����𒊏o
        }

        CSize textSize = pDC->GetTextExtent(dateStr.c_str());
        pDC->TextOut(x - textSize.cx / 2, chartRect.bottom + 5, dateStr.c_str());
    }

    // �^�C�g���̕`��
    CFont titleFont;
    titleFont.CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    pDC->SelectObject(&titleFont);

    CString title = _T("�����`���[�g");
    CSize titleSize = pDC->GetTextExtent(title);
    pDC->TextOut((sizeview.cx - titleSize.cx) / 2, 5, title);

    // ���\�[�X�̕���
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldFont);
}

void DrawStockChart(CDC& cDC, const std::vector<VARIANT>& days, const std::vector<VARIANT>& open, const std::vector<VARIANT>& high, const std::vector<VARIANT>& low, const std::vector<VARIANT>& close, CSize viewSize)
{
    // Grok3

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

