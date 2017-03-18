// MySplitterWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MySplitterWnd.h"


// CMySplitterWnd

IMPLEMENT_DYNAMIC(CMySplitterWnd, CSplitterWnd)

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CMySplitterWnd::CMySplitterWnd()
		:
	m_nHidedCol(-1),
	m_nHidedRow(-1),
	m_nHidedID(-1)
{
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CMySplitterWnd::~CMySplitterWnd()
{
}

//メッセージマップ開始
BEGIN_MESSAGE_MAP(CMySplitterWnd, CSplitterWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()
//メッセージマップ終了

//////////////////////////////////////////////////
//ウィンドウスタイルの変更を行う
//////////////////////////////////////////////////
BOOL CMySplitterWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	//cs.style &= WS_CLIPCHILDREN;
	return CSplitterWnd::PreCreateWindow(cs);
}

//////////////////////////////////////////////////
//左クリック時
//////////////////////////////////////////////////
void CMySplitterWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	//左クリック時を処理しないことで分割ウィンドウの移動を制限する
	CSplitterWnd::OnLButtonDown(nFlags, point);
}
//////////////////////////////////////////////////
//左マウスボタンをあげる
//////////////////////////////////////////////////
void CMySplitterWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	CSplitterWnd::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////
//マウス移動時
//////////////////////////////////////////////////
void CMySplitterWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	//マウス移動を検知しないことでマウスアイコンの変更を抑制できる
	CSplitterWnd::OnMouseMove(nFlags, point);
}
//////////////////////////////////////////////////
//列を表示する
//////////////////////////////////////////////////
void CMySplitterWnd::ShowColumn()
{
   ASSERT_VALID(this);
   //ASSERT(m_nCols < m_nMaxCols);
   //ASSERT(m_nHidedCol != -1);
	if (m_nCols == -1)
		return;

   int colNew = m_nHidedCol;
   m_nHidedCol = -1;
   int cxNew = m_pColInfo[m_nCols].nCurSize;
   m_nCols++;    // add a column
   ASSERT(m_nCols == m_nMaxCols);

   // fill the hidden column
   int col;
   for (int row = 0; row < m_nRows; row++)
   {
      CWnd* pPaneShow = GetDlgItem(
         AFX_IDW_PANE_FIRST + row * 16 + m_nCols);
      ASSERT(pPaneShow != NULL);
      pPaneShow->ShowWindow(SW_SHOWNA);
	  m_nHidedID = -1;	//IDリセット

      for (col = m_nCols - 2; col >= colNew; col--)
      {
         CWnd* pPane = GetPane(row, col);
         ASSERT(pPane != NULL);
         pPane->SetDlgCtrlID(IdFromRowCol(row, col + 1));
      }

      pPaneShow->SetDlgCtrlID(IdFromRowCol(row, colNew));
   }

   // new panes have been created -- recalculate layout
   for (col = colNew + 1; col < m_nCols; col++)
      m_pColInfo[col].nIdealSize = m_pColInfo[col - 1].nCurSize;
   m_pColInfo[colNew].nIdealSize = cxNew;
   RecalcLayout();
}
//////////////////////////////////////////////////
//列を隠す
//////////////////////////////////////////////////
void CMySplitterWnd::HideColumn(int colHide)
{
   ASSERT_VALID(this);
   ASSERT(m_nCols > 1);
   ASSERT(colHide < m_nCols);
   ASSERT(m_nHidedCol == -1);
   m_nHidedCol = colHide;

   // if the column has an active window -- change it
   /*
   int rowActive = -1;
   int colActive = -1;
   if (GetActivePane(&rowActive, &colActive) != NULL &&
       colActive == colHide)
   {
      if (++colActive >= m_nCols)
         colActive = 0;
      SetActivePane(rowActive, colActive);
   }*/

   // hide all column panes
   for (int row = 0; row < m_nRows; row++)
   {
      CWnd* pPaneHide = GetPane(row, colHide);
      ASSERT(pPaneHide != NULL);
      pPaneHide->ShowWindow(SW_HIDE);

	  //IDのセット
	  m_nHidedID = AFX_IDW_PANE_FIRST + row * 16 + m_nCols;
      pPaneHide->SetDlgCtrlID(m_nHidedID);

      for (int col = colHide + 1; col < m_nCols; col++)
      {
         CWnd* pPane = GetPane(row, col);
         ASSERT(pPane != NULL);
         pPane->SetDlgCtrlID(IdFromRowCol(row, col - 1));
      }
   }
   m_nCols--;
   m_pColInfo[m_nCols].nCurSize = m_pColInfo[colHide].nCurSize;
   RecalcLayout();
}

//////////////////////////////////////////////////
//列のウィンドウハンドルを取得する
//この関数は列が非表示であってもカウントされる
//////////////////////////////////////////////////
CWnd* CMySplitterWnd::GetColumnWnd(int col)
{
	ASSERT_VALID(this);

	//列が表示されている場合
	if (m_nHidedCol == -1)
		return CSplitterWnd::GetPane(0,col);

	//列が表示されていない場合で非表示列と一致している場合
	if (col == m_nHidedCol)
		return GetDlgItem(m_nHidedID);
	if(col > m_nHidedCol)
		return GetPane(0,col - 1);
	return GetPane(0,col);
}



//////////////////////////////////////////////////
//行を表示
//////////////////////////////////////////////////
void CMySplitterWnd::ShowRow()
{
	ASSERT_VALID(this);
	//ASSERT(m_nRows < m_nMaxRows);
	//ASSERT(m_nHidedRow != -1);
	if (m_nHidedRow == -1)
		return;

	int rowNew = m_nHidedRow;
	m_nHidedRow = -1;
	int cyNew = m_pRowInfo[m_nRows].nCurSize;
	m_nRows++;	//行の追加
	
	ASSERT(m_nRows == m_nMaxRows);

	//隠されていた行の表示
	int row;
	for (int col = 0; col<m_nCols; col++)
	{
		//TRACE2("GetDlgItem(%d,%d)\n",m_nRows,col);
		CWnd* pPaneShow = GetDlgItem(
			AFX_IDW_PANE_FIRST + m_nRows * 16 + col);
		ASSERT(pPaneShow != NULL);
		pPaneShow->ShowWindow(SW_SHOWNA);
		m_nHidedID = -1;

		for (row = m_nRows - 2; row >= rowNew; row--)
		{
			CWnd* pPane = GetPane(row,col);
			pPane->SetDlgCtrlID(IdFromRowCol(row+1,col));
		}
		pPaneShow->SetDlgCtrlID(IdFromRowCol(rowNew,col));
	}

	//レイアウトの再計算
	for (row = rowNew + 1; row < m_nRows; row++) {
		m_pRowInfo[row].nIdealSize = m_pRowInfo[row - 1].nCurSize;
	}
	m_pRowInfo[rowNew].nIdealSize = cyNew;
	RecalcLayout();
}
//////////////////////////////////////////////////
//行を隠す
//////////////////////////////////////////////////
void CMySplitterWnd::HideRow(int rowHide)
{
	ASSERT_VALID(this);
	ASSERT(m_nRows > 1);
	ASSERT(rowHide < m_nRows);
	ASSERT(m_nHidedRow == -1);
	m_nHidedRow = rowHide;

	//アクティブウィンドウであれば変更する
	/*
	int rowActive = -1;
	int colActive = -1;
	if (GetActivePane(&rowActive,&colActive) != NULL &&
		rowActive == rowHide)
	{
		if (++rowActive >= m_nRows)
			rowActive = 0;
		SetActivePane(rowActive, colActive);
	}*/

	//全ての列ペインを隠す
	for (int col = 0; col < m_nCols; col++)
	{
		CWnd* pPaneHide = GetPane(rowHide, col);
		ASSERT(pPaneHide != NULL);
		pPaneHide->ShowWindow(SW_HIDE);
		//TRACE2("SetDlgCtrlID(%d,%d)\n",m_nRows,col);

		//IDのセット
		m_nHidedID = AFX_IDW_PANE_FIRST + m_nRows * 16 + col;
		pPaneHide->SetDlgCtrlID(m_nHidedID);

		for (int row = rowHide + 1; row <m_nRows; row++)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			pPane->SetDlgCtrlID(IdFromRowCol(row-1,col));
		}
	}
	m_nRows--;
	m_pRowInfo[m_nRows].nCurSize = m_pRowInfo[rowHide].nCurSize;
	RecalcLayout();
}

//////////////////////////////////////////////////
//行のウィンドウハンドルを取得する
//この関数は列が非表示であってもカウントされる
//////////////////////////////////////////////////
CWnd* CMySplitterWnd::GetRowWnd(int row)
{
	ASSERT_VALID(this);

	//列が表示されている場合
	if (m_nHidedRow == -1)
		return CSplitterWnd::GetPane(row,0);

	//列が表示されていない場合で非表示列と一致している場合
	if (row == m_nHidedRow)
		return GetDlgItem(m_nHidedID);
	if(row > m_nHidedRow)
		return GetPane(row - 1,0);
	return GetPane(row,0);
}
