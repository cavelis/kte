#pragma once

#include "MyChildWnd.h"

//////////////////////////////////////////////////
//プレビューウィンドウ
//////////////////////////////////////////////////
class CPreviewWnd : public CWnd
{
	DECLARE_DYNAMIC(CPreviewWnd)

public:
	//コンストラクタ
	CPreviewWnd();
	//デストラクタ
	virtual ~CPreviewWnd();

protected:
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

private:

	//////////////////////////////////////////////////
	//メンバ変数
	BOOL m_bFirstFlag;			//初期化フラグ
	CMyChildWnd m_wndChild;		//プレビュー画面本体の子ウィンドウ(このウィンドウにキャプチャ画面が表示される)
	CSize m_ClientSize;			//クライアントウィンドウのサイズ
	CSize m_ChildSize;			//子ウィンドウのサイズ
	CPoint m_ChildPos;			//子ウィンドウの場所
	BOOL m_bChildShow;			//子ウィンドウを表示するか否か

	//スクロールバー関連
	BOOL m_bShowHScroll;		//水平スクロールバーの表示
	BOOL m_bShowVScroll;		//垂直スクロールバーの表示
	BOOL m_bRecalcWindowCall;	//スクロールバーの編集中フラグ
	int m_nScrollBarWidth;		//垂直スクロールバーの幅(固定値)
	int m_nScrollBarHeight;		//水平スクロールバーの高さ(固定値)

	COLORREF m_rgbBack;			//背景色
	COLORREF m_rgbLine;			//枠線の色

	//画面の拡大縮小倍率
	double m_dTimes;			//拡大倍率(単位はパーセント)
	int m_nTimesID;				//現在の倍率のMenuID
	BOOL m_bFixByClient;		//プレイヤーの大きさをプレビューにあわせる
	BOOL m_bFixByWindow;		//ウィンドウのサイズに合わせる

	//////////////////////////////////////////////////
	//メンバ変数(ここまで)

	/*
	HHOOK m_hMouseHook;			//マウスフック用のハンドル
	HHOOK GetHook() {return m_hMouseHook;};
	CWnd* GetChildWnd() {return &m_wndChild;}
	static LRESULT CALLBACK MyMouseHookProc(int nCode, WPARAM wp, LPARAM lp);
	*/

	//プレイヤーのサイズをプレビューに合わせる
	void FixWindowSize();
	//プレビューの倍率を現在のサイズに合わせる
	void FixPreviewTimes(int nClientWidth ,int nClientHeight);

	//ステータスバーに拡大倍率を書き込む
	void SetStatusTimes(double dTimes);

public:

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
private:
	void MoveChildWindow();
public:
	//プレビューウィンドウを割り当てる
	void SetPreviewWnd();
	//プレビューサイズを設定する
	void SetPreviewSize(BOOL bSizeUpdateFlag);

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);

	//プロファイルの変更通知
	BOOL ChangeProfileNotify();

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();

	//ウィンドウ配置の計算
	BOOL RecalcWindowArrangement(BOOL bOnSize);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnViewPopup(UINT nID);
	afx_msg void OnPopupFixwindowsize();
	afx_msg void OnPopupFixpreviewsize();
	afx_msg void OnPopupShowpreview();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


