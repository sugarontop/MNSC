#include "pch.h"
void DrawStockChart2(CDC* pDC, const std::vector<std::wstring>& days, const std::vector<std::wstring>& close, CSize sizeview)
{
    _ASSERT(days.size() == close.size());

    if (days.empty() || close.empty())
        return;

    // マージンの設定
    const int leftMargin = 60;
    const int rightMargin = 20;
    const int topMargin = 20;
    const int bottomMargin = 60;

    // 描画領域の計算
    CRect chartRect(leftMargin, topMargin,
        sizeview.cx - rightMargin,
        sizeview.cy - bottomMargin);

    // 終値データを数値に変換し、最大値・最小値を取得
    std::vector<double> prices;
    double minPrice = DBL_MAX;
    double maxPrice = -DBL_MAX;

    for (const auto& priceStr : close) {
        double price = _wtof(priceStr.c_str());
        prices.push_back(price);
        minPrice = min(minPrice, price);
        maxPrice = max(maxPrice, price);
    }

    // 価格範囲に余裕を持たせる
    double priceRange = maxPrice - minPrice;
    if (priceRange == 0) priceRange = 1; // ゼロ除算回避
    minPrice -= priceRange * 0.1;
    maxPrice += priceRange * 0.1;
    priceRange = maxPrice - minPrice;

    // ペンとブラシの準備
    CPen gridPen(PS_DOT, 1, RGB(200, 200, 200));
    CPen axisPen(PS_SOLID, 1, RGB(0, 0, 0));
    CPen linePen(PS_SOLID, 2, RGB(0, 100, 200));
    CBrush pointBrush(RGB(255, 0, 0));

    CPen* pOldPen = pDC->SelectObject(&axisPen);

    // 軸の描画
    pDC->MoveTo(chartRect.left, chartRect.top);
    pDC->LineTo(chartRect.left, chartRect.bottom);
    pDC->LineTo(chartRect.right, chartRect.bottom);

    // グリッドの描画
    pDC->SelectObject(&gridPen);

    // 横線（価格グリッド）
    for (int i = 1; i <= 4; i++) {
        int y = chartRect.bottom - (chartRect.Height() * i / 5);
        pDC->MoveTo(chartRect.left, y);
        pDC->LineTo(chartRect.right, y);
    }

    // 縦線（日付グリッド）
    int dataCount = static_cast<int>(days.size());
    for (int i = 1; i < dataCount; i++) {
        if (i % max(1, dataCount / 10) == 0) { // 最大10本のグリッド線
            int x = chartRect.left + (chartRect.Width() * i / (dataCount - 1));
            pDC->MoveTo(x, chartRect.top);
            pDC->LineTo(x, chartRect.bottom);
        }
    }

    // 株価線の描画
    pDC->SelectObject(&linePen);

    std::vector<CPoint> points;
    for (int i = 0; i < dataCount; i++) {
        int x = chartRect.left + (chartRect.Width() * i / max(1, dataCount - 1));
        int y = chartRect.bottom - static_cast<int>((prices[i] - minPrice) / priceRange * chartRect.Height());
        points.push_back(CPoint(x, y));
    }

    // 線の描画
    if (points.size() >= 2) {
        pDC->MoveTo(points[0]);
        for (size_t i = 1; i < points.size(); i++) {
            pDC->LineTo(points[i]);
        }
    }

    // データポイントの描画
    CBrush* pOldBrush = pDC->SelectObject(&pointBrush);
    for (const auto& pt : points) {
        pDC->Ellipse(pt.x - 3, pt.y - 3, pt.x + 3, pt.y + 3);
    }

    // ラベルの描画用フォント
    CFont font;
    font.CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    CFont* pOldFont = pDC->SelectObject(&font);

    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(RGB(0, 0, 0));

    // Y軸ラベル（価格）
    for (int i = 0; i <= 5; i++) {
        double price = minPrice + (priceRange * i / 5);
        int y = chartRect.bottom - (chartRect.Height() * i / 5);

        CString priceLabel;
        priceLabel.Format(_T("%.0f"), price);

        CSize textSize = pDC->GetTextExtent(priceLabel);
        pDC->TextOut(chartRect.left - textSize.cx - 5, y - textSize.cy / 2, priceLabel);
    }

    // X軸ラベル（日付）
    int labelStep = max(1, dataCount / 8); // 最大8個のラベル
    for (int i = 0; i < dataCount; i += labelStep) {
        int x = chartRect.left + (chartRect.Width() * i / max(1, dataCount - 1));

        // 日付文字列から月日のみ抽出（YYYY-MM-DD形式を想定）
        std::wstring dateStr = days[i];
        if (dateStr.length() >= 10) {
            dateStr = dateStr.substr(5, 5); // MM-DD部分を抽出
        }

        CSize textSize = pDC->GetTextExtent(dateStr.c_str());
        pDC->TextOut(x - textSize.cx / 2, chartRect.bottom + 5, dateStr.c_str());
    }

    // タイトルの描画
    CFont titleFont;
    titleFont.CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    pDC->SelectObject(&titleFont);

    CString title = _T("株価チャート");
    CSize titleSize = pDC->GetTextExtent(title);
    pDC->TextOut((sizeview.cx - titleSize.cx) / 2, 5, title);

    // リソースの復元
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldFont);
}

void DrawStockChart(CDC& cDC, const std::vector<VARIANT>& days, const std::vector<VARIANT>& open, const std::vector<VARIANT>& high, const std::vector<VARIANT>& low, const std::vector<VARIANT>& close, CSize viewSize)
{
    // Grok3

    _ASSERT(days.size() == open.size() && days.size() == high.size() && days.size() == low.size() && days.size() == close.size());

    if (days.empty()) return; // データが空の場合は終了

    // 描画エリアの設定
    const int margin = 50; // 上下左右の余白
    const int chartWidth = viewSize.cx - 2 * margin;
    const int chartHeight = viewSize.cy - 2 * margin;

    // データの最大・最小値を計算
    double minPrice = DBL_MAX;
    double maxPrice = -DBL_MAX;
    for (size_t i = 0; i < high.size(); ++i) {
        double h = high[i].dblVal;
        double l = low[i].dblVal;
        minPrice = min(minPrice, l);
        maxPrice = max(maxPrice, h);
    }

    // 価格のスケーリング
    double priceRange = maxPrice - minPrice;
    if (priceRange == 0) priceRange = 1.0; // ゼロ除算防止
    double priceScale = chartHeight / priceRange;

    // 日付のスケーリング
    size_t numDays = days.size();
    double barWidth = static_cast<double>(chartWidth) / numDays;
    if (barWidth < 2.0) barWidth = 2.0; // バーの最小幅

    // 背景を白でクリア
    cDC.FillSolidRect(0, 0, viewSize.cx, viewSize.cy, RGB(255, 255, 255));

    // 軸の描画
    CPen axisPen(PS_SOLID, 1, RGB(0, 0, 0));
    CPen* oldPen = cDC.SelectObject(&axisPen);
    cDC.MoveTo(margin, margin);
    cDC.LineTo(margin, viewSize.cy - margin); // Y軸
    cDC.MoveTo(margin, viewSize.cy - margin);
    cDC.LineTo(viewSize.cx - margin, viewSize.cy - margin); // X軸

    // 価格目盛りの描画（簡易的に5分割）
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
    lf.lfHeight = 20;               // フォントの高さ
    lf.lfEscapement = 450;          // 文字の傾斜角度（1/10度単位、45度 = 450）
    lf.lfOrientation = 450;         // 文字のベースラインの傾斜（1/10度単位）
    _tcscpy_s(lf.lfFaceName, _T("Arial")); // フォント名
    font45.CreateFontIndirect(&lf);



    // ローソク足の描画
    for (size_t i = 0; i < numDays; ++i) {
        double o = open[i].dblVal;
        double h = high[i].dblVal;
        double l = low[i].dblVal;
        double c = close[i].dblVal;

        // X座標（バー中心）
        int x = margin + static_cast<int>(i * barWidth + barWidth / 2);

        // Y座標を計算
        int highY = viewSize.cy - margin - static_cast<int>((h - minPrice) * priceScale);
        int lowY = viewSize.cy - margin - static_cast<int>((l - minPrice) * priceScale);
        int openY = viewSize.cy - margin - static_cast<int>((o - minPrice) * priceScale);
        int closeY = viewSize.cy - margin - static_cast<int>((c - minPrice) * priceScale);

        // ローソク足の色（陽線：赤、陰線：青）
        COLORREF color = (c >= o) ? RGB(255, 0, 0) : RGB(0, 0, 255);
        CPen candlePen(PS_SOLID, 1, color);
        CBrush candleBrush(color);
        cDC.SelectObject(&candlePen);
        cDC.SelectObject(&candleBrush);

        // 高値-安値の線（ヒゲ）
        cDC.MoveTo(x, highY);
        cDC.LineTo(x, lowY);

        // ローソク足本体
        int candleWidth = static_cast<int>(barWidth * 0.4); // バーの幅の40%
        if (candleWidth < 1) candleWidth = 1;
        CRect candleRect(x - candleWidth, min(openY, closeY), x + candleWidth, max(openY, closeY));
        cDC.Rectangle(candleRect);

        // 日付ラベルの描画（例：5つおき）
        if (i % 5 == 0) {
            
            auto oldf = cDC.SelectObject(font45);
            CString dateStr;
            //dateStr.Format(_T("%.0f"), days[i].dblVal); // VARIANTの日付を仮にdoubleとして扱う
            dateStr = days[i].bstrVal;
            cDC.TextOut(x - 20, viewSize.cy - margin + 10, dateStr);
            cDC.SelectObject(oldf);
        }
    }

    // 元のペンを復元
    cDC.SelectObject(oldPen);
}

