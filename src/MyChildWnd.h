#pragma once


// CMyChildWnd

class CMyChildWnd : public CWnd
{
	DECLARE_DYNAMIC(CMyChildWnd)

public:
	CMyChildWnd();
	virtual ~CMyChildWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
};


