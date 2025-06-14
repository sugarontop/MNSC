#include "pch.h"

void DrawStockChart(CDC& cDC, const std::vector<VARIANT>& days, const std::vector<VARIANT>& open, const std::vector<VARIANT>& high, const std::vector<VARIANT>& low, const std::vector<VARIANT>& close, CSize viewSize)
{
    // generate by Grok3

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

