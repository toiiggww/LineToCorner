
// theDialog.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "theApp.h"
#include "theDialog.h"
#include "afxdialogex.h"

#include <vector>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#define _WINSOCK_DEPRECATED_NO_WARNINGS


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
// theDialog 对话框



theDialog::theDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_THEAPP_DIALOG, pParent)
	, udpsocket(INVALID_SOCKET)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	clLine = RGB(192, 128, 64);
	clBack = RGB(32, 64, 192);
}

void theDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void theDialog::OnOK()
{
	TCHAR buf[256] = {0};
	OPENFILENAME g;
	ZeroMemory(&g, sizeof(g));
	g.lStructSize = sizeof(g);
	g.hwndOwner = m_hWnd;
	g.lpstrFile = buf;
	g.nMaxFile = 255;
	g.lpstrFilter = txt("bmp\0*.bmp");

	if (GetSaveFileName(&g))
	{
		SYSTEMTIME t;
		GetLocalTime(&t);
		CString f = g.lpstrFile;
		CString fn = PathFindFileName(f);
		CString pt = f.Mid(0, f.GetLength() - fn.GetLength());
		int ix = fn.Find(txt("."), 0);
		if (ix > 0)
		{
			fn = fn.Mid(0, ix);
		}
		CString ff;
		ff.Format(txt("%s%s-s-%02d-%02d-%02d-%02d-%02d.bmp"),pt.GetBuffer(), fn.GetBuffer(), t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
		SaveToImage(ff, false);

		ff.Format(txt("%s%s-l-%d-%02d-%02d-%02d-%02d-%02d.bmp"), pt.GetBuffer(), fn.GetBuffer(), zoom, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
		SaveToImage(ff, true);
	}
}

void theDialog::OnCancel()
{
	WSACleanup();
	CDialogEx::OnCancel();
}

void theDialog::DrawRect(CDC& dc)
{
	CRect& r = rRange;
	CPen pr(PS_SOLID, 3, RGB(127, 0, 0));
	dc.SelectObject(&pr);
	//dc.Rectangle(&r);
	int w = r.Width(), h = r.Height();
	CPoint a = pdStart,
		b(pdStart.x + (w <= h ? w : h) * (pdStart.x < pdEnd.x ? 1 : -1),
			pdStart.y + (w <= h ? w : h) * (pdStart.y < pdEnd.y ? 1 : -1));
	CPoint c;
	vector<CPoint> ps = { a, b };
	while (Next(r, a, b, c))
	{
		ps.push_back(c);
		if (ps.size() >= 10000)
		{
			ShowMessage(txt("Point count >= 10000, end to get next point"));
			break;
		}
		else
		{
			//ShowMessage(txt("pre = (%03d, %03d), p = (%03d, %03d)"), b.x, b.y, c.x, c.y);
		}
		a = b;
		b = c;
	}
	int pl = ps.size();
	ShowMessage(txt("calculated point count = %d"), pl);
	CPoint* pa = new CPoint[pl];
	for (int i = 0; i < pl; i++)
	{
		pa[i] = ps[i];
	}
	if (isDrawIndicator || pl == 10000)
	{
		CPen pt(PS_DOT, 3, RGB(128, 0, 0));
		dc.SelectObject(&pt);
		dc.MoveTo(ps[0]);
		dc.LineTo(ps[1]);
		CPen pe(PS_DOT, 3, RGB(0, 128, 0));
		dc.SelectObject(&pe);
		dc.MoveTo(ps[pl - 2]);
		dc.LineTo(ps[pl - 1]);
	}
	DWORD ls = static_cast<CComboBox*>(GetDlgItem(IDCcLineStyle))->GetCurSel();
	CPen pxl(ls, 1, clLine/*RGB(128, 128, 128)*/);
	dc.SelectObject(&pxl);
	dc.Polyline(pa, pl);
	delete[] pa;
}

void theDialog::DrawThumbnail(CDC& dc)
{
	CPen sp(PS_DOT, 2, RGB(255, 0, 0));
	dc.SelectObject(&sp);
	CRect ro(rRange.left - 2, rRange.top - 2, rRange.right + 2, rRange.bottom + 2);
	dc.DrawFocusRect(&ro);

	CPoint rlt(rRange.right + UiE.Padding, rRange.top + UiE.Padding);
	CSize rsz(rSelect.Width() * zoom, rSelect.Height() * zoom);
	CRect r(rlt, rsz);
	dc.StretchBlt(r.left, r.top, r.Width(), r.Height(), &dc, rSelect.left, rSelect.top, rSelect.Width(), rSelect.Height(), SRCCOPY);
}

void theDialog::SetEnd(const CPoint& v, CPoint& d, bool force)
{
	if (&v != &d)
	{
		d = v;
	}
	CString t;
	t.Format(txt("%d"), v.x);
	SetDlgItemText(IDCexEnd, t);

	t.Format(txt("%d"), v.y);
	SetDlgItemText(IDCeyEnd, t);

	t.Format(txt("%d"), abs(pdEnd.x - pdStart.x));
	SetDlgItemText(IDCeWidth, t);

	t.Format(txt("%d"), abs(pdEnd.y - pdStart.y));
	SetDlgItemText(IDCeHeight, t);
	if (force)
	{
		rRange.left = pdStart.x > pdEnd.x ? pdEnd.x : pdStart.x; rRange.top = pdStart.y > pdEnd.y ? pdEnd.y : pdStart.y;
		rRange.right = pdEnd.x > pdStart.x ? pdEnd.x : pdStart.x; rRange.bottom = pdEnd.y > pdStart.y ? pdEnd.y : pdStart.y;
		Invalidate();
	}
}

void theDialog::UpdateZoom()
{
	CString t;
	t.Format(txt("%d"), zoom);
	SetDlgItemText(IDCeZoom, t);
}

void theDialog::UpdatePostaion(DWORD i, LPLONG d, bool u)
{
	CString t;
	GetDlgItemText(i, t);
	ScanTo(t.GetBuffer(), txt("%d"), d);
	if (u)
	{
		Invalidate();
	}
}

void theDialog::UpdateColor(COLORREF& c, ColorPart p, BYTE v)
{
	//c ^= (v << p);
	BYTE r = GetRValue(c), g = GetGValue(c), b = GetBValue(c);
	if (p == ColorPart::CPR)
	{
		r = v;
	}
	else if (p == ColorPart::CPG)
	{
		g = v;
	}
	else if (p == ColorPart::CPB)
	{
		b = v;
	}
	c = RGB(r, g, b);
	(isLineColor ? ccLine : ccBack).SetColor(c);
	//ccLine.SetColor(c);
	Invalidate();
}

void theDialog::MoveElement(const CRect& r, CWnd* w)
{
	w->MoveWindow(&r);
	w->Invalidate();
}

bool theDialog::Next(const CRect& r, const POINT& a, const POINT& b, POINT& c)
{
	bool rv = true;
	do
	{
		if (InCorner(r, b))
		{
			rv = false;
			break;
		}
		int rm = 0;
		if (b.x == r.left || b.x == r.right)
		{
			//ShowMessage(txt("|"));
			rm = a.y < b.y ? r.bottom - b.y : b.y - r.top;
			rm = rm > r.Width() ? r.Width() : rm;
			c.x = a.x < b.x ? (b.x - rm < r.left ? r.left : b.x - rm) : (b.x + rm > r.right ? r.right : b.x + rm);
			c.y = a.y < b.y ? (b.y + rm > r.bottom ? r.bottom : b.y + rm) : (b.y - rm < r.top ? r.top : b.y - rm);
		}
		else if(b.y == r.top || b.y == r.bottom)
		{
			//ShowMessage(txt("--"));
			rm = a.x <= b.x ? r.right - b.x : b.x - r.left;
			rm = rm > r.Height() ? r.Height() : rm;
			c.x = a.x < b.x ? (b.x + rm > r.right ? r.right : b.x + rm) : (b.x - rm < r.left ? r.left : b.x - rm);
			c.y = a.y < b.y ? (b.y - rm < r.top ? r.top : b.y - rm) : (b.y + rm > r.bottom ? r.bottom : b.y + rm);
		}
		else
		{
			ShowMessage(txt("impassable : (%d, %d), (%d, %d)"), a.x, a.y, b.x, b.y);
			break;
		}
	} while (false);
	return rv;
}

bool theDialog::InCorner(const CRect& r, const POINT& p)
{
	bool rv = true;
	do
	{
		if (r.TopLeft() == p)
		{
			//ShowMessage(txt("point at top left (%d, %d) = (%d, %d)"), r.left, r.top, p.x, p.y);
			break;
		}
		if (r.BottomRight() == p)
		{
			//ShowMessage(txt("point at bottom right (%d, %d) = (%d, %d)"), r.right, r.bottom, p.x, p.y);
			break;
		}
		if (p.x == r.left && p.y == r.bottom)
		{
			//ShowMessage(txt("point at left bottom (%d, %d) = (%d, %d)"), r.left, r.bottom, p.x, p.y);
			break;
		}
		if (p.x == r.right && p.y == r.top)
		{
			//ShowMessage(txt("point at right top (%d, %d) = (%d, %d)"), r.right, r.top, p.x, p.y);
			break;
		}
		//ShowMessage(txt("point not in corner"));
		rv = false;
	} while (false);
	return rv;
}

void theDialog::ShowMessage(LPCTSTR v, ...)
{
	va_list a;
	va_start(a, v);
	SYSTEMTIME t;
	GetLocalTime(&t);
	CString m;
	m.FormatV(v, a);
	CString x;
	x.Format(txt("%02d:%02d.%03d \t"), t.wMinute, t.wSecond, t.wMilliseconds);
	x += m;
	x += NewLine;
	OutputDebugString(x);
	SetDlgItemText(IDClMessage, x);
}

void theDialog::SaveToImage(const CString& n, bool z)
{
	CDC* pdc = GetDC();
	CDC ndc;
	CImage img;
	CBitmap bmp;
	int w = rRange.Width() * (z ? zoom : 1), h = rRange.Height() * (z ? zoom : 1);

	bmp.CreateCompatibleBitmap(pdc, w, h);
	ndc.CreateCompatibleDC(pdc);
	ndc.SelectObject(bmp);
	ndc.StretchBlt(0, 0, w, h, pdc, rRange.left, rRange.top, rRange.Width(), rRange.Height(), SRCCOPY);

	img.Attach(bmp);
	img.Save(n);
	img.Detach();
}

BEGIN_MESSAGE_MAP(theDialog, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(UDN_DELTAPOS, IDCsWidth, &theDialog::OnWidthChanged)
	ON_NOTIFY(UDN_DELTAPOS, IDCsHeight, &theDialog::OnHeightChanged)
	ON_STN_DBLCLK(IDClMessage, &theDialog::OnRedraw)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(UDN_DELTAPOS, IDCsZoom, &theDialog::OnZoomChanged)
	ON_BN_CLICKED(IDCcEanbleSE, &theDialog::OnShowIndicator)
	ON_CBN_SELCHANGE(IDCcLineStyle, &theDialog::OnLinestyleChanged)
	//ON_EN_CHANGE(IDCexStart, &theDialog::OnStartXChanged)
	//ON_EN_CHANGE(IDCeyStart, &theDialog::OnStartYChanged)
	//ON_EN_CHANGE(IDCexEnd, &theDialog::OnEndXChanged)
	//ON_EN_CHANGE(IDCeyEnd, &theDialog::OnEndYChanged)
	//ON_EN_CHANGE(IDCeWidth, &theDialog::OnWidthChanged)
	//ON_EN_CHANGE(IDCeHeight, &theDialog::OnHeightChanged)
	//ON_EN_CHANGE(IDCeZoom, &theDialog::OnZoomChanged)
	ON_BN_CLICKED(IDCcIsLineColor, &theDialog::OnLineColorChecked)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDCsColorR, &theDialog::OnPosChange)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDCsColorB, &theDialog::OnPosChange)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDCsColorG, &theDialog::OnPosChange)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// theDialog 消息处理程序

BOOL theDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	zoom = 3;
	UpdateZoom();

	CRect r(0, 0, 80, 30);
	ccLine.Create(txt(""), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER | SS_SUNKEN, r, this, IDCxColorLine);
	ccLine.SetFont(GetFont());
	ccBack.Create(txt(""), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER | SS_SUNKEN, r, this, IDCxColorBack);
	ccBack.SetFont(GetFont());

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void theDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	CRect r;
	GetDesktopWindow()->GetClientRect(r);
	r.left = (r.Width() - 1024) / 2;
	r.top = (r.Height() - 768) / 2;
	r.right = r.left + 1024;
	r.bottom = r.top + 768;
	MoveWindow(&r);
	CSpinButtonCtrl* spin;
	spin = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDCsWidth));
	if (spin->GetBuddy() == NULL)
	{
		spin->SetBuddy(GetDlgItem(IDCeWidth));
	}

	spin = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDCsHeight));
	if (spin->GetBuddy() == NULL)
	{
		spin->SetBuddy(GetDlgItem(IDCeHeight));
	}

	spin = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDCsZoom));
	if (spin->GetBuddy() == NULL)
	{
		spin->SetBuddy(GetDlgItem(IDCeZoom));
	}

	CComboBox* w = static_cast<CComboBox*>(GetDlgItem(IDCcLineStyle));
	w->InsertString(w->GetCount(), txt("实线"));
	w->InsertString(w->GetCount(), txt("短横"));
	w->InsertString(w->GetCount(), txt("点"));
	w->InsertString(w->GetCount(), txt("点划"));
	w->InsertString(w->GetCount(), txt("点点划"));
	w->SetCurSel(2);

	CSliderCtrl* sc = NULL;
	sc = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorR));
	sc->SetRange(0, 255);
	sc = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorG));
	sc->SetRange(0, 255);
	sc = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorB));
	sc->SetRange(0, 255);
	OnLineColorChecked();

	cuMoving = LoadCursor(NULL, IDC_SIZEALL);
	cuDefault = LoadCursor(NULL, IDC_ARROW);

	ccBack.SetColor(clBack);
	ccLine.SetColor(clLine);

	// TODO: 在此处添加消息处理程序代码
	CString t = (txt("右键选择画图区域，左键选择放大区域"));
	SetWindowText(t);
	ShowMessage(t);
}


void theDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		ShellAbout(m_hWnd, txt("Line To Corner"), txt("ItFunly"), m_hIcon);
		ShowMessage(txt("A Line to corner application"));
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void theDialog::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文
	if (IsIconic())
	{

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		if (!isPainting)
		{
			CMemDC mdc(dc, this);
			CBitmap mbp;
			CRect r;
			GetClientRect(&r);
			mbp.CreateCompatibleBitmap(&dc, r.Width(), r.Height());
			CDC& pdc = mdc.GetDC();
			CBitmap* obp = pdc.SelectObject(&mbp);
			CBrush b;
			b.CreateSolidBrush(clBack);
			pdc.FillRect(&r, &b);
			isPainting = true;
			if (pdStart != pdEnd)
			{
				DrawRect(pdc);
			}
			if (ptStart != ptEnd)
			{
				DrawThumbnail(pdc);
			}
		}
		isPainting = false;
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR theDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void theDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	isCapture = true;
	pdStart = point;
	CString t;

	t.Format(txt("%d"), point.x);
	SetDlgItemText(IDCexStart, t);

	t.Format(txt("%d"), point.y);
	SetDlgItemText(IDCeyStart, t);
	ShowMessage(txt("\t on right button down \n"));
	CDialogEx::OnRButtonDown(nFlags, point);
}


void theDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (isCapture)
	{
		isCapture = false;
		SetEnd(point, pdEnd);
		ShowMessage(txt("\t on right button up \n"));
	}
	CDialogEx::OnRButtonUp(nFlags, point);
}


void theDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (isCapture)
	{
		SetEnd(point, pdEnd, false);
		//ShowMessage(txt("\t on move \n"));
	}
	else if (isThumbnail)
	{
		SetEnd(point, ptEnd);
		rSelect.left = (ptStart.x < ptEnd.x ? ptStart.x : ptEnd.x); rSelect.top = (ptStart.y < ptEnd.y ? ptStart.y : ptEnd.y);
		rSelect.right = (ptStart.x > ptEnd.x ? ptStart.x : ptEnd.x); rSelect.bottom = (ptStart.y > ptEnd.y ? ptStart.y : ptEnd.y);
	}
	else if (isMoving)
	{
		int dx = point.x - pmStart.x,
			dy = point.y - pmStart.y;
		ptEnd.Offset(dx, dy);
		ptStart.Offset(dx, dy);
		rSelect.OffsetRect(dx, dy);
		pmStart = point;
		Invalidate();
	}
	if (rSelect.PtInRect(point))
	{
		SetCursor(cuMoving);
	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void theDialog::OnSize(UINT nType, int cx, int cy)
{
	CWnd* w = NULL;
	w = GetDlgItem(IDOK);
	if (w->GetSafeHwnd())
	{
		RECT r;
		int uiLable = 20, uiEdit = 40;
		r.top = r.bottom = cy - UiE.Padding * 5 - UiE.LineHeight * 5;
		r.left = r.right = UiE.Padding;
		r.bottom = r.top + UiE.LineHeight;

		r.left = UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClColorR));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit + uiLable;
		MoveElement(r, GetDlgItem(IDCeColorR));

		r.left = r.right + UiE.Padding;
		r.right = r.left + UiE.Edit * 3;
		MoveElement(r, GetDlgItem(IDCsColorR));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit + uiLable;
		MoveElement(r, GetDlgItem(IDCxColorBack));

		r.top = r.bottom + UiE.Padding;
		r.bottom = r.top + UiE.LineHeight;

		r.left = UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClColorG));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit + uiLable;
		MoveElement(r, GetDlgItem(IDCeColorG));

		r.left = r.right + UiE.Padding;
		r.right = r.left + UiE.Edit * 3;
		MoveElement(r, GetDlgItem(IDCsColorG));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit + uiLable;
		MoveElement(r, GetDlgItem(IDCxColorLine));

		r.top = r.bottom + UiE.Padding;
		r.bottom = r.top + UiE.LineHeight;

		r.left = UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClColorB));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit + uiLable;
		MoveElement(r, GetDlgItem(IDCeColorB));

		r.left = r.right + UiE.Padding;
		r.right = r.left + UiE.Edit * 3;
		MoveElement(r, GetDlgItem(IDCsColorB));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCcIsLineColor));

		r.top = r.bottom + UiE.Padding;
		r.bottom = r.top + UiE.LineHeight;

		r.left = UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClxStart));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCexStart));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClxEnd));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCexEnd));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClWidth));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCeWidth));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDCsWidth));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClZoom));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit + uiLable;
		MoveElement(r, GetDlgItem(IDCeZoom));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDCsZoom));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCcEanbleSE));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit + uiLable;
		MoveElement(r, GetDlgItem(IDCcLineStyle));

		r.left = cx - UiE.Padding - UiE.Edit;
		r.right = r.left + UiE.Edit;
		r.top -= 2;
		r.bottom += 2;
		MoveElement(r, GetDlgItem(IDOK));
		r.bottom -= 2;

		r.left = r.right = UiE.Padding;
		r.top = r.bottom + UiE.Padding;
		r.bottom = r.top + UiE.LineHeight;

		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClyStart));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCeyStart));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClyEnd));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCeyEnd));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDClHeight));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiEdit;
		MoveElement(r, GetDlgItem(IDCeHeight));

		r.left = r.right + UiE.Padding;
		r.right = r.left + uiLable;
		MoveElement(r, GetDlgItem(IDCsHeight));

		r.left = r.right + UiE.Padding;
		r.right = cx - UiE.Padding * 2 - UiE.Edit;
		MoveElement(r, GetDlgItem(IDClMessage));

		r.left = cx - UiE.Padding - UiE.Edit;
		r.right = r.left + UiE.Edit;
		r.top -= 2;
		r.bottom += 2;
		MoveElement(r, GetDlgItem(IDCANCEL));
		Invalidate();
	}
	else
	{
		CDialogEx::OnSize(nType, cx, cy);
	}
	// TODO: 在此处添加消息处理程序代码
}


void theDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//ShowMessage(txt("on h scroll message"));
	BYTE v = 0;
	CString t;
	CWnd* w = NULL;
	ColorPart p;
	CSliderCtrl* sc = NULL;
	if (false) {}
	else if (pScrollBar->GetDlgCtrlID() == IDCsColorR)
	{
		sc = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorR));
		w = GetDlgItem(IDCeColorR);
		p = ColorPart::CPR;
	}
	else if (pScrollBar->GetDlgCtrlID() == IDCsColorG)
	{
		sc = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorG));
		w = GetDlgItem(IDCeColorG);
		p = ColorPart::CPG;
	}
	else if (pScrollBar->GetDlgCtrlID() == IDCsColorB)
	{
		sc = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorB));
		w = GetDlgItem(IDCeColorB);
		p = ColorPart::CPB;
	}
	if (sc)
	{
		v = sc->GetPos();
		t.Format(txt("%02X:%d"), v, v);
		w->SetWindowText(t);
		UpdateColor((isLineColor ? clLine : clBack), p, v);
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void theDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


void theDialog::OnWidthChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pud = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	pdEnd.x += pud->iDelta < 0 ? 1 : -1;
	SetEnd(pdEnd, pdEnd);
	*pResult = 0;
}


void theDialog::OnHeightChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pud = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	pdEnd.y += pud->iDelta < 0 ? 1 : -1;
	SetEnd(pdEnd, pdEnd);
	*pResult = 0;
}


void theDialog::OnRedraw()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
}


void theDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (rSelect.PtInRect(point))
	{
		isMoving = true;
		pmStart = point;
		SetCursor(cuMoving);
	}
	else
	{
		isThumbnail = true;
		ptStart = point;
		rSelect.left = rSelect.top = rSelect.right = rSelect.bottom = -1;
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}


void theDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (isMoving)
	{
		isMoving = false;
		SetCursor(cuDefault);
	}
	SetEnd(point, ptEnd);
	isThumbnail = false;
	CDialogEx::OnLButtonUp(nFlags, point);
}


void theDialog::OnZoomChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pud = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	zoom += (pud->iDelta > 0 ? -1 : 1);
	if (zoom < 1.0000001)
	{
		zoom = 1;
	}
	UpdateZoom();
	Invalidate();
	*pResult = 0;
}


void theDialog::OnShowIndicator()
{
	// TODO: 在此添加控件通知处理程序代码
	isDrawIndicator = static_cast<CButton*>(GetDlgItem(IDCcEanbleSE))->GetCheck();
	Invalidate();
}


void theDialog::OnLinestyleChanged()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
}


void theDialog::OnStartXChanged()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	UpdatePostaion(IDCexStart, &(pdStart.x));
	// TODO:  在此添加控件通知处理程序代码
}


void theDialog::OnStartYChanged()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdatePostaion(IDCeyStart, &(pdStart.y));
}


void theDialog::OnEndXChanged()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdatePostaion(IDCexEnd, &(pdEnd.x));
}


void theDialog::OnEndYChanged()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdatePostaion(IDCeyEnd, &(pdEnd.y));
}


void theDialog::OnWidthChanged()
{
	// TODO:  在此添加控件通知处理程序代码
	LONG v;
	UpdatePostaion(IDCeWidth, &v, false);
	pdEnd.x += (pdEnd.x > pdStart.x ? pdEnd.x : pdStart.x) - (pdEnd.x < pdStart.x ? pdEnd.x : pdStart.x) - v;
	Invalidate();
}


void theDialog::OnHeightChanged()
{
	// TODO:  在此添加控件通知处理程序代码
	LONG v;
	UpdatePostaion(IDCeHeight, &v, false);
	pdEnd.y += (pdEnd.y > pdStart.y ? pdEnd.y : pdStart.y) - (pdEnd.y < pdStart.y ? pdEnd.y : pdStart.y) - v;
	Invalidate();
}


void theDialog::OnZoomChanged()
{
	//try
	//{
	//	CString t;
	//	GetDlgItemText(IDCeZoom, t);
	//	ScanTo(t.GetBuffer(), txt("%.4f"), zoom);
	//	Invalidate();
	//}
	//catch (const std::exception& e)
	//{
	//	ShowMessage(txt("OnZoomChanged error = %s"), e.what());
	//}
}


void theDialog::OnLineColorChecked()
{
	// TODO: 在此添加控件通知处理程序代码
	isLineColor = static_cast<CButton*>(GetDlgItem(IDCcIsLineColor))->GetCheck();
	CSliderCtrl* w = NULL;
	CString t;
	BYTE v;

	w = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorR));
	v = GetRValue(isLineColor ? clLine : clBack);
	t.Format(txt("%02X:%d"), v, v);
	SetDlgItemText(IDCeColorR, t);
	w->SetPos(v);

	w = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorG));
	v = GetGValue(isLineColor ? clLine : clBack);
	t.Format(txt("%02X:%d"), v, v);
	SetDlgItemText(IDCeColorG, t);
	w->SetPos(v);

	w = static_cast<CSliderCtrl*>(GetDlgItem(IDCsColorB));
	v = GetBValue(isLineColor ? clLine : clBack);
	t.Format(txt("%02X:%d"), v, v);
	SetDlgItemText(IDCeColorB, t);
	w->SetPos(v);

	(isLineColor ? ccLine : ccBack).SetColor(isLineColor ? clLine : clBack);
	Invalidate();
}



void theDialog::OnPosChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	NMTRBTHUMBPOSCHANGING* pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString t;
	t.Format(txt("pos = %d"), pNMTPC->dwPos);
	ShowMessage(t);
	*pResult = 0;
}


BOOL theDialog::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE; // CDialogEx::OnEraseBkgnd(pDC);
}
