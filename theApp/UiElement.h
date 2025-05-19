#pragma once

class ColorControl : public CStatic
{
public:
    void SetColor(const COLORREF&);
protected:
    DECLARE_DYNAMIC(ColorControl)
    DECLARE_MESSAGE_MAP()

    COLORREF color, invColor;

    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
};

struct UiElement
{
    int Padding;
    int Lable;
    int Edit;
    int LineHeight;
    int Edge;
    int Boder;
};

extern UiElement UiE;
