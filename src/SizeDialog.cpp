// SizeDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "SizeDialog.h"
#include "afxdialogex.h"


// CSizeDialog ダイアログ

IMPLEMENT_DYNAMIC(CSizeDialog, CDialog)

CSizeDialog::CSizeDialog(int nID,CWnd* pParent /*=NULL*/)
	: CDialog(nID, pParent)
{

}

CSizeDialog::~CSizeDialog()
{
}

void CSizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSizeDialog, CDialog)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

// CSizeDialog メッセージ ハンドラー

BOOL CSizeDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	//画面全体のサイズの取得
	GetWindowRect(m_ScorllRect);
	//スクロールバーの現在位置の取得
	m_ScrollPos.SetPoint(0,0);
	return TRUE;
}
//////////////////////////////////////////////////
//水平スクロール
//////////////////////////////////////////////////
void CSizeDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	/*
	// TODO: Add your message handler code here and/or call default.
	int nDelta;
	int nMaxPos = m_ScorllRect.Width() - m_ScrollSize.cx;

	switch (nSBCode)
	{
	case SB_LINEDOWN:
		if (m_ScrollPos.x >= nMaxPos)
			return;
		nDelta = min(nMaxPos/100,nMaxPos-m_ScrollPos.x);
		break;

	case SB_LINEUP:
		if (m_ScrollPos.x <= 0)
			return;
		nDelta = -min(nMaxPos/100,m_ScrollPos.x);
		break;

    case SB_PAGEDOWN:
		if (m_ScrollPos.x >= nMaxPos)
			return;
		nDelta = min(nMaxPos/10,nMaxPos-m_ScrollPos.x);
		break;

	case SB_THUMBPOSITION:
		nDelta = (int)nPos - m_ScrollPos.x;
		break;

	case SB_PAGEUP:
		if (m_ScrollPos.x <= 0)
			return;
		nDelta = -min(nMaxPos/10,m_ScrollPos.x);
		break;
	
    default:
		return;
	}
	m_ScrollPos.x += nDelta;
	SetScrollPos(SB_HORZ,m_ScrollPos.x,TRUE);
	ScrollWindow(0,-nDelta);
	*/
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
//////////////////////////////////////////////////
//垂直スクロール
//////////////////////////////////////////////////
void CSizeDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	/*
	// TODO: Add your message handler code here and/or call default.
	int nDelta = 0;
	int nMaxPos = m_ScorllRect.Height() - m_ScrollSize.cy;

	switch (nSBCode)
	{
	case SB_LINEDOWN:
		if (m_ScrollPos.y >= nMaxPos)
			return;
		nDelta = min(nMaxPos/100,nMaxPos-m_ScrollPos.y);
		break;
	case SB_LINEUP:
		if (m_ScrollPos.y <= 0)
			return;
		nDelta = -min(nMaxPos/100,m_ScrollPos.y);
		break;
	case SB_PAGEDOWN:
		if (m_ScrollPos.y >= nMaxPos)
			return;
		nDelta = min(nMaxPos/10,nMaxPos-m_ScrollPos.y);
		break;
	case SB_THUMBPOSITION:
		nDelta = (int)nPos - m_ScrollPos.y;
		break;
	case SB_PAGEUP:
		if (m_ScrollPos.y <= 0)
			return;
		nDelta = -min(nMaxPos/10,m_ScrollPos.y);
		break;
	default:
		return;
	}
	m_ScrollPos.y += nDelta;
	SetScrollPos(SB_VERT,m_ScrollPos.y,TRUE);

	//画面を動かす
	ScrollWindow(0,-nDelta);
	*/

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
//////////////////////////////////////////////////
//ダイアログのサイズ変更
//////////////////////////////////////////////////
void CSizeDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	/*
	//m_ScrollSize.cx = cx;
	m_ScrollSize.cy = cy;

	int nScrollMaxX,nScrollMaxY;
	if (cx < m_ScorllRect.Width())
	     nScrollMaxX = m_ScorllRect.Width() - cx;
	else
	     nScrollMaxX = 0;
	if (cy < m_ScorllRect.Height())
	     nScrollMaxY = m_ScorllRect.Height() - cy;
	else
	     nScrollMaxY = 0;

	nScrollMaxY = 0;

	SCROLLINFO si;
	ZeroMemory(&si,sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = nScrollMaxY;//  + si.nPage;
	si.nPos = 0;
	si.nPage = (int) (nScrollMaxY * ( (double) (cy) / (double)m_ScorllRect.Height()));
	SetScrollInfo(SB_VERT, &si, TRUE);
	//TODO スクロールの範囲がおかしいかもしれないけど、たぶん普通に動いている
	*/
}



