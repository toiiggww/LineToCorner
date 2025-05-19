
// theDialog.h: 头文件
//

#pragma once

#include "UiElement.h"

enum ColorPart
{
	CPR = 16,
	CPG = 8,
	CPB = 0,
};
// theDialog 对话框
class theDialog : public CDialogEx
{
// 构造
public:
	theDialog(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DrawRect(CDC&);
	virtual void DrawThumbnail(CDC&);
	virtual void SetEnd(const CPoint&, CPoint&, bool force = true);
	virtual void UpdateZoom();
	virtual void UpdatePostaion(DWORD, LPLONG, bool u = true);
	virtual void UpdateColor(COLORREF&, ColorPart, BYTE);
	virtual void MoveElement(const CRect&, CWnd*);
	virtual bool Next(const CRect&, const POINT&, const POINT&, POINT&);
	virtual bool InCorner(const CRect&, const POINT&);
	virtual void ShowMessage(LPCTSTR, ...);

// 实现
protected:
	HICON m_hIcon;
	SCROLLINFO mScroller;
	bool isCapture, isSocketStartup, isThumbnail, isDrawIndicator, isMoving, isPainting, isLineColor;
	CPoint pdStart, pdEnd, ptStart, ptEnd, pmStart;
	CRect rRange, rSelect;
	SOCKET udpsocket;
	float zoom;
	COLORREF clLine, clBack;
	ColorControl ccLine;
	ColorControl ccBack;
	HCURSOR cuMoving, cuDefault;

	DECLARE_MESSAGE_MAP()

protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnWidthChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeightChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRedraw();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnZoomChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowIndicator();
	afx_msg void OnLinestyleChanged();
	afx_msg void OnStartXChanged();
	afx_msg void OnStartYChanged();
	afx_msg void OnEndXChanged();
	afx_msg void OnEndYChanged();
	afx_msg void OnWidthChanged();
	afx_msg void OnHeightChanged();
	afx_msg void OnZoomChanged();
	afx_msg void OnLineColorChecked();
	afx_msg void OnPosChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
