#include "pch.h"
#include "UiElement.h"

UiElement UiE{ 3, 45, 80, 24, 10, 2 };

BEGIN_MESSAGE_MAP(ColorControl, CStatic)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(ColorControl, CStatic)

void ColorControl::SetColor(const COLORREF& v)
{
    if (GetSafeHwnd())
    {
        color = v;
        invColor = RGB(255 - GetRValue(v), 255 - GetGValue(v), 255 - GetBValue(v));
        Invalidate(TRUE);
    }
}

BOOL ColorControl::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void ColorControl::OnPaint()
{
    if (GetSafeHwnd())
    {
        CPaintDC dc(this);
        CMemDC mdc(dc, this);
        CRect r;
        GetClientRect(&r);
        CBitmap bmp;
        bmp.CreateCompatibleBitmap(&dc, r.Width(), r.Height());
        CDC& pdc = mdc.GetDC();
        CBrush b;
        b.CreateSolidBrush(color);
        pdc.FillRect(&r, &b);
        CString t;
        t.Format(txt("%06X"), color);
        CSize sz;
        GetTextExtentPoint(pdc, t, t.GetLength(), &sz);
        r.left = (r.Width() - sz.cx) / 2;
        r.top = (r.Height() - sz.cy) / 2;
        r.right = r.left + sz.cx;
        r.bottom = r.top + sz.cy;
        pdc.SetBkColor(color);
        pdc.SetTextColor(invColor);
        pdc.TextOut(r.left, r.top, t);
    }
}
