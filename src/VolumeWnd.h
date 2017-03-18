#pragma once


// CVolumeWnd
#define VOLUME_CHANNELS         2	/*ボリュームチャンネル数*/
#define VOLUME_LEFT_MARGIN     15	/*左マージン(固定値)*/
#define VOLUME_RIGHT_MARGIN    10	/*右マージン(固定値)*/
#define VOLUME_MIN_DB       -90.0	/*ボリュームの最小値*/
#define VOLUME_FONT_SIZE       10	/*ボリュームのフォントサイズ*/
#define VOLUME_LINE_COUNT      10	/*線の数*/

class CVolumeWnd : public CWnd
{
	DECLARE_DYNAMIC(CVolumeWnd)

public:
	//コンストラクタ
	CVolumeWnd();
	//デストラクタ
	virtual ~CVolumeWnd();


protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	//デシベルデータが送られるときのメッセージハンドラ
	LRESULT OnSenddBData(WPARAM wParam,LPARAM lParam);

	BOOL CreateBackSurface(int nWidth,int nHeight);
	BOOL DeleteBackSurface();


private:
	//////////////////////////////////////////////////
	//メンバ変数(ここから)
	//CToolTipCtrl m_ToolTip;		//ダイアログに表示するツールチップ

	//裏画面
	CDC*     m_phBackDC;			//デバイスコンテキスト
	CBitmap* m_phBackBitmap;		//ビットマップ

	double m_ddB[VOLUME_CHANNELS];	//デシベルデータ(L,R)

	//各種バー領域のサイズ
	CRect m_rcChannelBar[VOLUME_CHANNELS];	//チャンネルのバーの領域(全体)
	CRect m_rcChannelColorBar[VOLUME_CHANNELS][3];	//チャンネルのバーの領域(色ごと)

	//TRIVERTEX m_DarkTriVertex[VOLUME_CHANNELS][3][2];	//音量メーターのバーのグラデーション情報(暗色)
	//TRIVERTEX m_LightTriVertex[VOLUME_CHANNELS][3][2];//音量メーターのバーのグラデーション情報(明色)
	//GRADIENT_RECT m_GradientRect;		//音量メーターのグラデーションRECT

	//目盛り周り
	double m_ddBLine[VOLUME_LINE_COUNT];	//目盛り線のデータ(デシベル)
	int    m_ndBLine[VOLUME_LINE_COUNT];	//目盛り銭のX座標(ピクセル)

	//固定色
	COLORREF m_rgbBlack;		//背景色
	COLORREF m_rgbWhite;		//文字色
	COLORREF m_rgbLightBar[4];	//バーの色(明色)
	COLORREF m_rgbDarkBar[4];	//バーの色(暗色)
	CFont m_VolumeTextFont;		//音量メーターのフォント
	//////////////////////////////////////////////////
	//メンバ変数(ここまで)

private:
	//描画の為の便利サブルーチン

	//チャンネルバーの大きさ(CRect)を得る
	inline CRect GetBar(CRect &rcWindow,BOOL bLChannel) {
		CRect rcBar(0,0,0,0);
		if (rcWindow.Width() <= VOLUME_LEFT_MARGIN + VOLUME_RIGHT_MARGIN) {
			return rcBar;
		}
		rcBar.left = VOLUME_LEFT_MARGIN;
		rcBar.right = rcWindow.Width() - VOLUME_RIGHT_MARGIN;
		if (bLChannel) {
			rcBar.top = (int)(rcWindow.Height() * 0.125);
		} else {
			rcBar.top = (int)(rcWindow.Height() * 0.625);
		}
		rcBar.bottom = rcBar.top + (int)(rcWindow.Height() * 0.25);
		return rcBar;
	}
	//バーの大きさから指定のデシベルのX座標を得る
	inline int GetdBPos(CRect &rcBar,double dB) {
		if (dB <= VOLUME_MIN_DB)
			return rcBar.left;
		else if (dB >= 0.0)
			return rcBar.right - 1;
		double dPer = rcBar.Width() / (VOLUME_MIN_DB);	//分解能
		return (int) (rcBar.right - (dB * dPer));
	}

	//描画2
	void CVolumeWnd::OnPaint2(CDC &dc);

public:
	//afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//virtual BOOL PreTranslateMessage(MSG* pMsg);
	//afx_msg BOOL OnToolTip(UINT ID_notused,NMHDR *pNMHDR,LRESULT *Result_notused);
	//afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR,LRESULT *pResult);
	//afx_msg BOOL memberFxn( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPopupEnableVolume();
};


