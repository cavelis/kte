// MyChildWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MyChildWnd.h"


// CMyChildWnd

IMPLEMENT_DYNAMIC(CMyChildWnd, CWnd)

CMyChildWnd::CMyChildWnd()
{
}

CMyChildWnd::~CMyChildWnd()
{
}

BEGIN_MESSAGE_MAP(CMyChildWnd, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CMyChildWnd メッセージ ハンドラ

void CMyChildWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd* parent = this->GetParent();
	assert(parent);
	parent->SendMessage(WM_LBUTTONDOWN,nFlags,MAKELPARAM(point.x,point.y));
	CWnd::OnLButtonDown(nFlags, point);
}
void CMyChildWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	CWnd* parent = this->GetParent();
	assert(parent);
	parent->SendMessage(WM_RBUTTONDOWN,nFlags,MAKELPARAM(point.x,point.y));
	CWnd::OnRButtonDown(nFlags, point);
}

BOOL CMyChildWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。

	return CWnd::PreCreateWindow(cs);
}

void CMyChildWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	//TRACE0("CMyChildWnd::OnSetFocus\n");
	// TODO: ここにメッセージ ハンドラ コードを追加します。
}


BOOL CMyChildWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}


void CMyChildWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: ここにメッセージ ハンドラー コードを追加します。
	// 描画メッセージで CWnd::OnPaint() を呼び出さないでください。
}
