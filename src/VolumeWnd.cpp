// VolumeWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "VolumeWnd.h"
#include "MainFrm.h"

// CVolumeWnd

IMPLEMENT_DYNAMIC(CVolumeWnd, CWnd)

BEGIN_MESSAGE_MAP(CVolumeWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_MESSAGE(WM_SENDDBDATA,&CVolumeWnd::OnSenddBData)
	ON_WM_SIZE()
	//ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	//ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	//ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	//ON_NOTIFY_EX(TTN_NEEDTEXT,0,CVolumeWnd::memberFxn)
	//ON_NOTIFY_EX(TTN_GETDISPINFO,0,CVolumeWnd::OnToolTip)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_POPUP_ENABLEVOLUME, &CVolumeWnd::OnPopupEnableVolume)
END_MESSAGE_MAP()


//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CVolumeWnd::CVolumeWnd()
{
	m_phBackDC = NULL;
	m_phBackBitmap = NULL;

	m_ddB[0] = -90.0;
	m_ddB[1] = -90.0;
	m_rgbBlack       = RGB(0,0,0);
	m_rgbWhite       = RGB(255,255,255);

	m_rgbLightBar[0] = RGB(0,255,0);
	m_rgbLightBar[1] = RGB(255,181,24);
	m_rgbLightBar[2] = RGB(255,64,64);
	m_rgbLightBar[3] = RGB(255,64,64);

	m_rgbDarkBar[0] = RGB(0,89,0);
	m_rgbDarkBar[1] = RGB(89,63,8);
	m_rgbDarkBar[2] = RGB(89,22,22);
	m_rgbDarkBar[3] = RGB(89,22,22);

	for (int i=0;i<VOLUME_CHANNELS;i++) {
		for (int j=0;j<3;j++) {
			m_rcChannelColorBar[i][j].SetRect(0,0,0,0);
		}
		m_rcChannelBar[i].SetRect(0,0,0,0);
	}
	m_ddBLine[0] =-90.0;
	m_ddBLine[1] =-72.0;
	m_ddBLine[2] =-60.0;
	m_ddBLine[3] =-48.0;
	m_ddBLine[4] =-36.0;
	m_ddBLine[5] =-30.0;
	m_ddBLine[6] =-20.0;
	m_ddBLine[7] =-12.0;
	m_ddBLine[8] =-6.0;
	m_ddBLine[9] =-0.0;
	for (int i=0;i<VOLUME_LINE_COUNT;i++) {
		m_ndBLine[i] = 0;
	}
	/*
	for (int i=0;i<VOLUME_CHANNELS;i++) {
		for (int j=0;j<3;j++) {
			ZeroMemory(&m_DarkTriVertex[i][j][0],sizeof(TRIVERTEX));
			ZeroMemory(&m_DarkTriVertex[i][j][1],sizeof(TRIVERTEX));
			ZeroMemory(&m_LightTriVertex[i][j][0],sizeof(TRIVERTEX));
			ZeroMemory(&m_LightTriVertex[i][j][1],sizeof(TRIVERTEX));
			m_DarkTriVertex[i][j][0].Red   = GetRValue(m_rgbDarkBar[j]) << 8;
			m_DarkTriVertex[i][j][0].Green = GetGValue(m_rgbDarkBar[j]) << 8;
			m_DarkTriVertex[i][j][0].Blue  = GetBValue(m_rgbDarkBar[j]) << 8;
			m_DarkTriVertex[i][j][1].Red   = GetRValue(m_rgbDarkBar[j+1]) << 8;
			m_DarkTriVertex[i][j][1].Green = GetGValue(m_rgbDarkBar[j+1]) << 8;
			m_DarkTriVertex[i][j][1].Blue  = GetBValue(m_rgbDarkBar[j+1]) << 8;
			m_LightTriVertex[i][j][0].Red   = GetRValue(m_rgbLightBar[j]) << 8;
			m_LightTriVertex[i][j][0].Green = GetGValue(m_rgbLightBar[j]) << 8;
			m_LightTriVertex[i][j][0].Blue  = GetBValue(m_rgbLightBar[j]) << 8;
			//以下の情報は描画時に決定
			//m_LightTriVertex[i][j][1].Red   = GetRValue(m_rgbLightBar[j+1]) << 8;
			//m_LightTriVertex[i][j][1].Green = GetGValue(m_rgbLightBar[j+1]) << 8;
			//m_LightTriVertex[i][j][1].Blue  = GetBValue(m_rgbLightBar[j+1]) << 8;
		}
	}
	m_GradientRect.UpperLeft = 0;
	m_GradientRect.LowerRight = 1;
	*/
}
//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CVolumeWnd::~CVolumeWnd()
{
}



//////////////////////////////////////////////////
//ウィンドウ生成時のオプション変更
//////////////////////////////////////////////////
BOOL CVolumeWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。

	//子ウィンドウをクリップする
	cs.style |= WS_CLIPCHILDREN;

	//cs.style |= CBRS_TOOLTIPS;

	cs.style &= ~CS_VREDRAW;
	cs.style &= ~CS_HREDRAW;

	//if (!CWnd::PreCreateWindow(cs))
	//	return FALSE;
	//cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS, 
	//	::LoadCursor(NULL, IDC_ARROW),(HBRUSH)GetStockObject(NULL_BRUSH), NULL);
	return CWnd::PreCreateWindow(cs);
}

//////////////////////////////////////////////////
//ウィンドウが生成されたとき(WM_CREATE)に呼ばれるイベントハンドら
//////////////////////////////////////////////////
int CVolumeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//ウィンドウハンドルを与える
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	pDSA->SetAudioVolumeHWnd(this->GetSafeHwnd());

	//フォントの作成
	m_VolumeTextFont.CreateFont(
		VOLUME_FONT_SIZE,	//フォントの高さ(大きさ)
		0,					//フォントの幅。普通0
		0,					//角度
		0,					//同じく角度
		FW_DONTCARE,		//文字の太さ
		FALSE,				//フォントがイタリックならTRUEを指定
		FALSE,				//下線を引くならTRUE
		FALSE,				//取り消し線を引くならTRUE
		SHIFTJIS_CHARSET,	//フォントの文字セット。このままでOK
		OUT_DEFAULT_PRECIS,	//出力精度の設定。このままでOK
		CLIP_DEFAULT_PRECIS,//クリッピング精度。このままでOK
		DRAFT_QUALITY,		//フォントの出力品質。このままでOK
		DEFAULT_PITCH,		//フォントのピッチとファミリを指定。このままでOK
		_T("Arial"));		// フォントのタイプフェイス名の指定。これは見たまんま

	//this->SetTimer(0,500,NULL);

	/*
	m_ToolTip.Create(this);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.SetMaxTipWidth(300);
	//m_ToolTip.AddTool(this,_T("test"));
	DWORD dwAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP,dwAutoPop*2);
	*/
	//EnableToolTips(TRUE);
	return 0;
}

//////////////////////////////////////////////////
//ウィンドウが破棄される前のメッセージ
//////////////////////////////////////////////////
void CVolumeWnd::OnDestroy()
{
	CWnd::OnDestroy();

	//フォントの削除
	m_VolumeTextFont.DeleteObject();

	//裏画面の削除
	DeleteBackSurface();
}
////////////////////////////////////////
//裏画面を作成する
////////////////////////////////////////
BOOL CVolumeWnd::CreateBackSurface(int nWidth,int nHeight)
{
	CDC* phBackDC = new CDC;
	CBitmap* phBackBitmap = new CBitmap;

	CDC* pdc = GetDC();

	phBackDC->CreateCompatibleDC(pdc);
	phBackBitmap->CreateCompatibleBitmap(pdc,nWidth,nHeight);
	phBackDC->SelectObject(phBackBitmap);

	if (m_phBackDC) {
		//TODO 既存のデバイスがあるので、Bitbltでコピーする
		DeleteBackSurface();
	}

	//メンバ変数化
	m_phBackDC     = phBackDC;
	m_phBackBitmap = phBackBitmap;

	ReleaseDC(pdc);
	return TRUE;
}
////////////////////////////////////////
//裏画面を削除する
////////////////////////////////////////
BOOL CVolumeWnd::DeleteBackSurface()
{
	//裏画面の削除
	if (m_phBackDC) {
		m_phBackDC->DeleteDC();
		delete m_phBackDC;
		m_phBackDC = NULL;
	}
	if (m_phBackBitmap) {
		m_phBackBitmap->DeleteObject();
		delete m_phBackBitmap;
		m_phBackBitmap = NULL;
	}
	return TRUE;
}


////////////////////////////////////////
//背景色の描画を抑制
////////////////////////////////////////
BOOL CVolumeWnd::OnEraseBkgnd(CDC* pDC)
{
	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}

//////////////////////////////////////////////////
//WM_PAINTが来たときのイベントハンドラ
//////////////////////////////////////////////////
void CVolumeWnd::OnPaint()
{
	CPaintDC dc(this);

	//クライアントのサイズを取得
	CRect rcClient;
	GetClientRect(&rcClient);

	//裏画面に描画を行う
	OnPaint2(*m_phBackDC);

	//裏画面(m_phBackDC)から表画面(dc)に画像を転送する
	dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),m_phBackDC,0,0,SRCCOPY);
}

//////////////////////////////////////////////////
//WM_PAINTが来たときのイベントハンドラ
//////////////////////////////////////////////////
void CVolumeWnd::OnPaint2(CDC &dc)
{
	CString strText;

	//全体を黒で塗りつぶし
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	dc.FillSolidRect(0,0,rcWindow.Width(),rcWindow.Height(),m_rgbBlack);

	//メモリ線の描画
	CPen cLine(PS_SOLID,1,m_rgbWhite);
	int nPrevBkMode = dc.SetBkMode(TRANSPARENT);
	CPen* pOldObject = dc.SelectObject(&cLine);
	for (int i=0;i<VOLUME_LINE_COUNT;i++) {
		dc.MoveTo(m_ndBLine[i],m_rcChannelBar[0].top    - 1);
		dc.LineTo(m_ndBLine[i],m_rcChannelBar[0].bottom + 1);
		dc.MoveTo(m_ndBLine[i],m_rcChannelBar[1].top    - 1);
		dc.LineTo(m_ndBLine[i],m_rcChannelBar[1].bottom + 1);
	}
	//目盛り文字の描画
	CFont* pOldFont = dc.SelectObject(&m_VolumeTextFont);
	dc.SetTextColor(m_rgbWhite);
	dc.SetBkMode(TRANSPARENT);
	int nTextTop = (m_rcChannelBar[0].bottom + m_rcChannelBar[1].top) / 2 - (VOLUME_FONT_SIZE/2);
	for (int i=0;i<VOLUME_LINE_COUNT;i++) {
		strText.Format(_T("%.f"),m_ddBLine[i]);
		//任意文字列の文字幅を取得する方法が分からなかったので、等幅フォントと仮定してlengthで割っている
		dc.TextOut(m_ndBLine[i] - (strText.GetLength()/2 * VOLUME_FONT_SIZE /2),nTextTop,strText);

	}
	//L文字とR文字の描画
	int nTextLeft = (m_rcChannelBar[0].left - (VOLUME_FONT_SIZE)) / 2;
	strText.Format(_T("L"));
	dc.TextOut(nTextLeft,(m_rcChannelBar[0].top + m_rcChannelBar[0].bottom - VOLUME_FONT_SIZE) / 2,strText);
	strText.Format(_T("R"));
	dc.TextOut(nTextLeft,(m_rcChannelBar[1].top + m_rcChannelBar[1].bottom - VOLUME_FONT_SIZE) / 2,strText);

	//バー(暗色)の描画
	for (int i=0;i<VOLUME_CHANNELS;i++) {
		for (int j=0;j<3;j++) {
			//dc.GradientFill(m_DarkTriVertex[i][j],2,&m_GradientRect,1,GRADIENT_FILL_RECT_H);
			dc.FillSolidRect(m_rcChannelColorBar[i][j],m_rgbDarkBar[j]);
		}
	}

	//デシベルデータの取得
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	m_ddB[0] = -90.0;
	m_ddB[1] = -90.0;
	if (pDSA->IsAudioVolumeEnabled())
	{
		int nChannels = pDSA->GetAudioVolumeChannels();
		if (nChannels >= 1)
		{
			double* pddB = new double[nChannels];
			if (pDSA->GetAudioVolumedB(pddB))
			{
				if (nChannels == 1) {
					m_ddB[0] = pddB[0];
					m_ddB[1] = pddB[0];
				} else {
					m_ddB[0] = pddB[0];
					m_ddB[1] = pddB[1];
				}
			}
			delete [] pddB;
		}

	}

	//バー(明色)の描画
	int ndB[VOLUME_CHANNELS];
	for (int i=0;i<VOLUME_CHANNELS;i++) {
		ndB[i] = GetdBPos(m_rcChannelBar[i],m_ddB[i]);
		for (int j=0;j<3;j++) 
		{
			if (ndB[i] < m_rcChannelColorBar[i][j].left)
				break;	//このグラデーションまで描画しなくて良い

			/*
			//描画領域の決定
			if (ndB[i] < m_rcChannelColorBar[i][j].right) {	//部分描画
				m_LightTriVertex[i][j][1].x     = ndB[i];
				double dPercent = 0.0;
				if (m_LightTriVertex[i][j][1].x - m_LightTriVertex[i][j][0].x != 0)
					dPercent = ((double)ndB[i]) / ((double)m_LightTriVertex[i][j][1].x - (double)m_LightTriVertex[i][j][0].x);
				else
					break;
//					dPercent = 0.0;
				m_LightTriVertex[i][j][1].Red   = ((COLOR16)(dPercent * (GetRValue(m_rgbLightBar[j+1]) - GetRValue(m_rgbLightBar[j])))) << 8;
				m_LightTriVertex[i][j][1].Green = ((COLOR16)(dPercent * (GetGValue(m_rgbLightBar[j+1]) - GetGValue(m_rgbLightBar[j])))) << 8;
				m_LightTriVertex[i][j][1].Blue  = ((COLOR16)(dPercent * (GetBValue(m_rgbLightBar[j+1]) - GetBValue(m_rgbLightBar[j])))) << 8;
			} else {										//全描画
				m_LightTriVertex[i][j][1].x     = m_rcChannelColorBar[i][j].right;
				m_LightTriVertex[i][j][1].Red   = ((COLOR16)GetRValue(m_rgbLightBar[j+1])) << 8;
				m_LightTriVertex[i][j][1].Green = ((COLOR16)GetGValue(m_rgbLightBar[j+1])) << 8;
				m_LightTriVertex[i][j][1].Blue  = ((COLOR16)GetBValue(m_rgbLightBar[j+1])) << 8;
			}
			*/

			if (ndB[i] < m_rcChannelColorBar[i][j].right) {	//部分描画
				CRect rcBar;
				rcBar = m_rcChannelColorBar[i][j];
				rcBar.right = ndB[i];
				dc.FillSolidRect(rcBar,m_rgbLightBar[j]);
			} else {										//全描画
				dc.FillSolidRect(m_rcChannelColorBar[i][j],m_rgbLightBar[j]);
			}
			//dc.GradientFill(m_LightTriVertex[i][j],2,&m_GradientRect,1,GRADIENT_FILL_RECT_H);
		}
	}

	//フォントを復元
	dc.SelectObject(&pOldFont);
	//ブラシを復元
	dc.SelectObject(&pOldObject);
	//BkModeを復元
	dc.SetBkMode(nPrevBkMode);
}

//////////////////////////////////////////////////
//デシベルデータが送られるときのメッセージハンドラ
//////////////////////////////////////////////////
LRESULT CVolumeWnd::OnSenddBData(WPARAM wParam,LPARAM lParam)
{
	InvalidateRect(NULL,FALSE);	//描画メッセージを発行
	return 0;
}

//////////////////////////////////////////////////
//ウィンドウサイズ変更に呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CVolumeWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	//サイズ変更後に各種バー領域のサイズの初期化
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	m_rcChannelBar[0] = GetBar(rcWindow,TRUE);
	m_rcChannelBar[1] = GetBar(rcWindow,FALSE);
	int n20dB = GetdBPos(m_rcChannelBar[0],-20.0);
	int n6dB  = GetdBPos(m_rcChannelBar[0],-6.0);
	for (int i=0;i<2;i++) {
		m_rcChannelColorBar[i][0].SetRect(
			m_rcChannelBar[i].left,m_rcChannelBar[i].top,
			n20dB,m_rcChannelBar[i].bottom);
		m_rcChannelColorBar[i][1].SetRect(
			n20dB,m_rcChannelBar[i].top,
			n6dB,m_rcChannelBar[i].bottom);
		m_rcChannelColorBar[i][2].SetRect(
			n6dB,m_rcChannelBar[i].top,
			m_rcChannelBar[i].right,m_rcChannelBar[i].bottom);
	}

	/*
	//グラデーション情報を更新
	for (int i=0;i<VOLUME_CHANNELS;i++) {
		for (int j=0;j<3;j++) {
			m_DarkTriVertex[i][j][0].x = m_rcChannelColorBar[i][j].left;
			m_DarkTriVertex[i][j][0].y = m_rcChannelColorBar[i][j].top;
			m_DarkTriVertex[i][j][1].x = m_rcChannelColorBar[i][j].right;
			m_DarkTriVertex[i][j][1].y = m_rcChannelColorBar[i][j].bottom;

			m_LightTriVertex[i][j][0].x = m_rcChannelColorBar[i][j].left;
			m_LightTriVertex[i][j][0].y = m_rcChannelColorBar[i][j].top;
			//m_LightTriVertex[i][j][1].x = m_rcChannelColorBar[i][j].right;	//この情報はDraw時に決定
			m_LightTriVertex[i][j][1].y = m_rcChannelColorBar[i][j].bottom;
		}
	}
	*/

	//メモリ線の座標を取得
	for (int i=0;i<VOLUME_LINE_COUNT;i++) {
		m_ndBLine[i] = GetdBPos(m_rcChannelBar[0],m_ddBLine[i]);
	}

	//表画面の同じ大きさのサーフェスを作成
	CreateBackSurface(rcWindow.Width(),rcWindow.Height());

}

/*
//ダミーたいまー
void CVolumeWnd::OnTimer(UINT_PTR nIDEvent)
{
	m_ddB[0] += 1.0;
	m_ddB[1] += 1.0;
	if (m_ddB[0] >= 0) {
		m_ddB[0] = -30.0;
		m_ddB[1] = -30.0;
	}
	InvalidateRect(NULL);	//描画メッセージを発行
	CWnd::OnTimer(nIDEvent);
}
*/

/*
////////////////////////////////////////
//ツールチップの表示
////////////////////////////////////////
BOOL CVolumeWnd::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN: 
		case WM_LBUTTONUP: 
		case WM_MOUSEMOVE: 
			m_ToolTip.RelayEvent(pMsg);
			break;
	}
	return CWnd::PreTranslateMessage(pMsg);
}
BOOL CVolumeWnd::OnToolTip( UINT ID_notused, NMHDR *pNMHDR, LRESULT *Result_notused )
{
	NMTTDISPINFO* pTTT = (NMTTDISPINFO*)pNMHDR;
	pTTT->lpszText = _T("test");
	pTTT->hinst = AfxGetResourceHandle();
	return FALSE;
}
BOOL CVolumeWnd::OnToolTipNotify(UINT id, NMHDR *pNMHDR,LRESULT *pResult)
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT nID =pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
            pTTT->lpszText = _T("test");
            pTTT->hinst = AfxGetResourceHandle();
            //pTTT->lpszText = MAKEINTRESOURCE(nID);
            //pTTT->hinst = AfxGetResourceHandle();
            return(TRUE);
        }
    }
    return(FALSE);
}
*/

//////////////////////////////////////////////////
//右クリックでのポップアップウィンドウを表示
//////////////////////////////////////////////////
void CVolumeWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	//ポップアップウィンドウを表示
	CMenu cMenu;
	cMenu.LoadMenu(IDR_VOLUME_POPUP);
	CMenu* pPopup = cMenu.GetSubMenu(0); assert(pPopup);
	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow();

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (pDSA->IsAudioVolumeEnabled())
		pPopup->CheckMenuItem(ID_POPUP_ENABLEVOLUME,MF_CHECKED|MF_BYCOMMAND);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,this);
	cMenu.DestroyMenu();

	CWnd::OnRButtonDown(nFlags, point);
}
//////////////////////////////////////////////////
//ボリュームの有効・無効
//////////////////////////////////////////////////
void CVolumeWnd::OnPopupEnableVolume()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	BOOL bEnable = !pDSA->IsAudioVolumeEnabled();

	//
	pDSA->AudioVolumeEnableCapture(bEnable);

	//設定ファイルにも書き出し
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	pSetting->m_bEnableAudioPreview = bEnable;

	InvalidateRect(NULL,FALSE);	//描画メッセージを発行
	return;
}