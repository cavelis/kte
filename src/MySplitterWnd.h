#pragma once

//#include <afxsplitterwndex.h>
//#include "ExtSplitter.h"

//////////////////////////////////////////////////
//分割ウィンドウ
//////////////////////////////////////////////////
class CMySplitterWnd : public CSplitterWnd
{
	DECLARE_DYNAMIC(CMySplitterWnd)

public:
	//コンストラクタ
	CMySplitterWnd();
	//デストラクタ
	virtual ~CMySplitterWnd();
protected:
	DECLARE_MESSAGE_MAP()
	//ウィンドウデザイン変更
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	//左クリック時に呼ばれるイベントハンドラ
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//左クリック解除時に呼ばれるイベントハンドラ
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//マウス移動時に呼ばれるイベントハンドラ
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	//列を表示
	void ShowColumn();
	//列を非表示
	void HideColumn(int colHide);
	//列が表示されているかどうか調べる
	BOOL IsShowColumn() { return (BOOL) m_nHidedCol == -1;}
	//非表示列を取得
	int  IsHideColumnIndex() {return m_nHidedCol;}


	//列のウィンドウハンドルを取得する
	//この関数は列が非表示であってもカウントされる
	CWnd* GetColumnWnd(int col);

	//行を表示
	void ShowRow();
	//行を非表示
	void HideRow(int rowHide);
	//行が表示されているかどうか調べる
	BOOL IsShowRow() { return (BOOL) m_nHidedRow == -1;}
	//非表示列番号を取得
	int  IsHideRowIndex() {return m_nHidedRow;}

	//行のウィンドウハンドルを取得する
	//この関数は列が非表示であってもカウントされる
	CWnd* GetRowWnd(int row);

	int m_nHidedCol;	//隠されている列
	int m_nHidedRow;	//隠されている行
	int m_nHidedID;		//消されている行・列のID
};


