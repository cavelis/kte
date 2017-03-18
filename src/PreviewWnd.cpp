// PreviewWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "PreviewWnd.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CPreviewWnd, CWnd)

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CPreviewWnd::CPreviewWnd()
	:	m_dTimes(100),
		m_nTimesID(ID_POPUP_100)
{
	m_bFirstFlag = FALSE;
	m_ClientSize.SetSize(0,0);
	m_ChildSize.SetSize(0,0);
	m_ChildPos.SetPoint(0,0);
	m_bChildShow = TRUE;
	m_bFixByClient = FALSE;
	m_bFixByWindow = FALSE;

	m_nScrollBarWidth = 0;
	m_nScrollBarHeight = 0;
	m_bShowHScroll = FALSE;
	m_bShowVScroll = FALSE;
	m_bRecalcWindowCall = FALSE;

	//m_hMouseHook = NULL;

	m_rgbBack = 0;
	m_rgbLine = 0;
}
//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CPreviewWnd::~CPreviewWnd()
{
}

//メッセージマップ開始
BEGIN_MESSAGE_MAP(CPreviewWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETFOCUS()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND_RANGE(ID_POPUP_400, ID_POPUP_25, &CPreviewWnd::OnViewPopup)
	ON_COMMAND(ID_POPUP_FIXWINDOWSIZE, &CPreviewWnd::OnPopupFixwindowsize)
	ON_COMMAND(ID_POPUP_FIXPREVIEWSIZE, &CPreviewWnd::OnPopupFixpreviewsize)
	ON_COMMAND(ID_POPUP_SHOWPREVIEW, &CPreviewWnd::OnPopupShowpreview)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()
//メッセージマップ終了

//////////////////////////////////////////////////
//ウィンドウ生成時のオプション変更
//////////////////////////////////////////////////
BOOL CPreviewWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	cs.style &= ~WS_BORDER;
	//cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.dwExStyle |= WS_EX_WINDOWEDGE;

	//子ウィンドウ領域の描画を除外
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (pSetting->m_bPreviewForceOff == FALSE)
		cs.style |= WS_CLIPCHILDREN;


	cs.style &= ~CS_VREDRAW;
	cs.style &= ~CS_HREDRAW;

	//スクロールバーを有する
	cs.style |= WS_VSCROLL;
	cs.style |= WS_HSCROLL;

	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

//	UINT nClassStyle = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
	UINT nClassStyle = CS_DBLCLKS;
	//cs.lpszClass = AfxRegisterWndClass(nClassStyle, 
	//	::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);
	cs.lpszClass = AfxRegisterWndClass(nClassStyle, 
		::LoadCursor(NULL, IDC_ARROW),(HBRUSH)GetStockObject(NULL_BRUSH), NULL);

	return TRUE;
}
//////////////////////////////////////////////////
//ウィンドウが生成されたとき(WM_CREATE)に呼ばれるイベントハンドら
//////////////////////////////////////////////////
int CPreviewWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bFirstFlag = FALSE;

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	//スクロールバーの高さと幅を取得(設定固定値とする)
	m_nScrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);
	m_nScrollBarHeight = GetSystemMetrics(SM_CYHSCROLL);

	//スクロールバーの初期設定
	SCROLLINFO ScrollInfo;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;	//SIF_ALL
	ScrollInfo.nMin = 0;				//最小のスクロール位置
	ScrollInfo.nMax = 0;				//最大のスクロール位置
	ScrollInfo.nPage = ScrollInfo.nMax/10;//ページサイズ(PageUp/PageDownの移動量)
	ScrollInfo.nPos = 0;				//現在の位置
	//ScrollInfo.nTrackPos = 50;		//ドラッグ中のつまみの位置(Setの場合は必要なし)
	this->SetScrollInfo(SB_HORZ,&ScrollInfo,FALSE);
	this->SetScrollInfo(SB_VERT,&ScrollInfo,FALSE);
	m_bShowHScroll = FALSE;
	m_bShowVScroll = FALSE;

	//ビュー用の子ウィンドウを生成する
	DWORD dwStyle = WS_CHILD | WS_VISIBLE; // | WS_BORDER;
	if (!m_wndChild.Create(NULL,_T(""),dwStyle,
		CRect(0,0,0,0),this,AFX_IDW_PANE_FIRST,NULL))
	{
		TRACE0("ビュー ウィンドウを作成できませんでした。\n");
		return -1;
	}
	m_ChildPos.SetPoint(0,0);
	m_ClientSize.SetSize(0,0);

	/*
	//マウスイベントのフックを行う
	m_hMouseHook = NULL;
	m_hMouseHook = SetWindowsHookEx(
		WH_MOUSE,
		(HOOKPROC)CPreviewWnd::MyMouseHookProc,
		AfxGetInstanceHandle(),
		AfxGetApp()->m_nThreadID);
	assert(m_hMouseHook);
	*/

	//倍率のサイズの決定
	this->m_dTimes = pSetting->m_dPreviewChildTimes;
	this->m_bFixByClient = pSetting->m_bFixByClient;
	this->m_bFixByWindow = pSetting->m_bFixByWindow;
	if (!m_bFixByWindow) {	//任意倍率でない場合
		switch ((int)m_dTimes) {
		case 400:
			m_nTimesID = ID_POPUP_400; break;
		case 300:
			m_nTimesID = ID_POPUP_300; break;
		case 200:
			m_nTimesID = ID_POPUP_200; break;
		case 150:
			m_nTimesID = ID_POPUP_150; break;
		case 100:
			m_nTimesID = ID_POPUP_100; break;
		case 75:
			m_nTimesID = ID_POPUP_75; break;
		case 66:
			m_nTimesID = ID_POPUP_66; break;
		case 50:
			m_nTimesID = ID_POPUP_50; break;
		case 33:
			m_nTimesID = ID_POPUP_33; break;
		case 25:
			m_nTimesID = ID_POPUP_25; break;
		default:
			m_nTimesID = 0;
		}
	}
	//このタイミングだと未だウィンドウが出来ていない
	//SetStatusTimes(this->m_dTimes);

	//色の決定
	m_rgbBack = RGB(0,0,0);
	if (pSetting->m_bPreviewForceOff == TRUE)
		m_rgbLine = RGB(255,128,0);
	else
		m_rgbLine = RGB(128,128,255);

	//プレビュー開始
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (pDSA->IsCanPreview())
	{
		SetPreviewWnd();		//プレビューウィンドウの割り当てを行う
		SetPreviewSize(FALSE);

		//プレビューの開始(フィルタグラフの実行)
		//CMainFrame* frame = (CMainFrame*)::AfxGetMainWnd();
		//CPropertyFormView* propertywnd =  frame->GetPropertyWnd();
		//CInputPropertyDialog* pInputTab = propertywnd->GetInputTab();
		//assert(pInputTab);
		//assert(pInputTab->GetSafeHwnd());

		BOOL bRet = pDSA->RunFilter();
		//pInputTab->OnCbnSelchangeVideoResolutionCombo();	//解像度変更がプレビューの開始トリガとなる
	}

	//表示・非表示を切り替える
	this->m_bChildShow = pSetting->m_bPreviewShow;
	if (!this->m_bChildShow) {
		m_wndChild.ShowWindow(SW_HIDE);
		pDSA->ShowPreviewWindow(FALSE);		//非表示モードにする
	}

	m_bFirstFlag = TRUE;
	return 0;
}

/*
//////////////////////////////////////////////////
//グローバルマウスフックイベント
//////////////////////////////////////////////////
LRESULT CALLBACK CPreviewWnd::MyMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	HHOOK hHook = NULL;
	CKTEApp* app = (CKTEApp*)AfxGetApp();
	assert(app);
	CMainFrame* frame = (CMainFrame*)app->GetMainWnd();
	assert(frame);
	CPreviewWnd* preview = frame->GetPreviewWnd();
	assert(preview);
	hHook = preview->GetHook();

	if (nCode < 0)
		return CallNextHookEx(hHook, nCode, wParam, lParam);

   switch (wParam)
   {
	case WM_LBUTTONDOWN:
		{
			//TRACE0("WM_LBUTTONDOWN\n");
			MOUSEHOOKSTRUCTEX *mmsg;
			mmsg = (MOUSEHOOKSTRUCTEX *)lParam;

			CPoint pt;
			pt = mmsg->pt;
			::ClientToScreen(mmsg->hwnd,&pt);

			CRect rcClient;
			preview->GetChildWnd()->GetWindowRect(&rcClient);
			if (rcClient.PtInRect(pt))
			{
				TRACE0("ClientWM_LBUTTONDOWN\n");
				//TRACE3("nCode=%d,wParam=%d,lParam=%d\n",nCode,wParam,lParam);
				return 1;
			}
			if (mmsg->hwnd == preview->GetSafeHwnd() ||
				mmsg->hwnd == preview->GetChildWnd()->GetSafeHwnd())
			{
				TRACE3("nCode=%d,wParam=%d,lParam=%d\n",nCode,wParam,lParam);
				preview->SendMessage(WM_LBUTTONDOWN,0,MAKELPARAM(mmsg->pt.x,mmsg->pt.y));
				return 1;
			}
			CRect rcClient;
			preview->GetWindowRect(&rcClient);
			CPoint pt;
			pt.x = lParam & 0xFFFF;
			pt.y = (lParam >> 16) & 0xFFFF;
			if (rcClient.PtInRect(pt))
			{
				TRACE3("nCode=%d,wParam=%d,lParam=%d\n",nCode,wParam,lParam);
				preview->SendMessage(WM_LBUTTONDOWN,wParam,lParam);
				return 1;
			}
		}
		break;
	//case WM_MOUSEWHELL:
		//TRACE3("nCode=%d,wParam=%d,lParam=%d\n",nCode,wParam,lParam);
	//	break;
   }


	//if(nCode != HC_ACTION)
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
*/

//////////////////////////////////////////////////
//ウィンドウが破棄される前のメッセージ
//////////////////////////////////////////////////
void CPreviewWnd::OnDestroy()
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	pSetting->m_dPreviewChildTimes = this->m_dTimes;
	pSetting->m_bFixByClient = this->m_bFixByClient;
	pSetting->m_bFixByWindow = this->m_bFixByWindow;
	pSetting->m_bPreviewShow = this->m_bChildShow;

	/*
	//マウスフックを終了する
	if (m_hMouseHook)
		UnhookWindowsHookEx(m_hMouseHook);
	*/

	//TRACE0("CPreviewWnd::OnDestroy()");
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	/*
	//エンコード中ならエンコードを停止する		←この処理は親ウィンドウの時点で処理済み
	if (pDSA->IsEncode()) {
		CMainFrame* frame = (CMainFrame*)::AfxGetMainWnd();
		frame->OnStopEncode();
	}
	*/
	//ウィンドウハンドルを取り除く
	if (pDSA->IsRunFilter()) {
		pDSA->StopFilter();
	}
	//ウィンドウハンドルを削除する
	pDSA->RemovePreviewWindow();

	//TODO このタイミングでリリースする
	//pDSA->Exit();

	CWnd::OnDestroy();
}

//////////////////////////////////////////////////
//WM_PAINTが来たときのイベントハンドラ
//////////////////////////////////////////////////
void CPreviewWnd::OnPaint() 
{
	CPaintDC dc(this);

	//全体を黒で塗りつぶし
	CRect rcWindow;
	this->GetWindowRect(&rcWindow);
	dc.FillSolidRect(0,0,
		rcWindow.right-rcWindow.left,
		rcWindow.bottom-rcWindow.top,
		m_rgbBack);

	//枠線の表示
	CRect rcChild;
	m_wndChild.GetWindowRect(&rcChild);
	rcChild.MoveToXY(rcChild.left - rcWindow.left,rcChild.top - rcWindow.top);
	rcChild.left--;
	rcChild.top--;
	rcChild.right++;
	rcChild.bottom++;
	CBrush line;
    CBrush *pOldBrush;
	line.CreateSolidBrush(m_rgbLine);
	pOldBrush = dc.SelectObject(&line);
	dc.FrameRect(&rcChild,&line);	//四角を描画

	//非表示時の灰色塗りつぶし
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (m_bChildShow == FALSE || pSetting->m_bPreviewForceOff == TRUE) {
		rcChild.left++;
		rcChild.top++;
		rcChild.right--;
		rcChild.bottom--;
		CBrush FillAreaBrush;
		FillAreaBrush.CreateSolidBrush(RGB(128,128,128));
		CBrush* Old = dc.SelectObject(&FillAreaBrush);
		dc.FillRect(&rcChild,&FillAreaBrush);
		dc.SelectObject(Old);
		FillAreaBrush.DeleteObject();
	}
	dc.SelectObject(pOldBrush);
	line.DeleteObject();
}

//////////////////////////////////////////////////
//プレビューのウィンドウを割り当てる
//ビデオサイズが変更されたときにInputPropertyDialogから呼ばれる
//////////////////////////////////////////////////
void CPreviewWnd::SetPreviewWnd()
{
	//プレビュー用のウィンドウハンドルを割り当てる
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (m_wndChild.GetSafeHwnd() != NULL)
	{
		if (pSetting->m_bPreviewForceOff == FALSE)
		{
			pDSA->SetPreviewWindow(m_wndChild.GetSafeHwnd());
		}
	}
	return;
}
//////////////////////////////////////////////////
//プレビューのサイズを変更
//ビデオサイズが変更されたときにInputPropertyDialogから呼ばれる
//あと拡大縮小倍率が呼ばれたときにも呼ばれる
//////////////////////////////////////////////////
void CPreviewWnd::SetPreviewSize(BOOL bSizeUpdateFlag)
{
	//未だ子ウィンドウが作成されていない場合はFALSEを返す
	if (m_wndChild.GetSafeHwnd() == NULL)
		return;

	//TRACE0("CPreviewWnd::SetPreviewSize()\n");
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	int nSrcWidth = 0;
	int nSrcHeight = 0;
	pDSA->GetCurrentVideoSize(nSrcWidth,nSrcHeight);

	assert(m_wndChild.GetSafeHwnd() != NULL);
	CSize size;
	size.cx = (int)(((double)nSrcWidth * this->m_dTimes) / 100);
	size.cy = (int)(((double)nSrcHeight * this->m_dTimes) / 100);
	//m_wndChild.MoveWindow(0,0,size.cx,size.cy,FALSE);

	m_ChildSize = size;

	//ウィンドウサイズ変更
	RecalcWindowArrangement(FALSE);

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (pSetting->m_bPreviewForceOff == FALSE)
	{
		CRect rcClient;
		rcClient.SetRect(0,0,size.cx,size.cy );
		BOOL bRet = pDSA->ResizePreviewWindow(rcClient);
		//TODO ここが問題？
		//assert(bRet);
	}

	if (bSizeUpdateFlag && this->m_bFixByClient)
		FixWindowSize();

	SetStatusTimes(this->m_dTimes);
	
	//ウィンドウの更新
	InvalidateRect(NULL,FALSE);
	return;
}

//////////////////////////////////////////////////
//WM_LBUTTONDOWNが来たときのイベントハンドラ
//////////////////////////////////////////////////
void CPreviewWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	//TRACE0("CPreviewWnd::OnLButtonDown\n");
	SetFocus();
	CWnd::OnLButtonDown(nFlags, point);
}
//////////////////////////////////////////////////
//マウスホイールのイベントハンドラ
//////////////////////////////////////////////////
BOOL CPreviewWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_bFixByWindow == TRUE)	//「プレビューの倍率をプレイヤーの大きさに合わせる」ときにはホイールできない
		return TRUE;

	UINT nID = 0;
	switch (m_nTimesID)
	{
	case ID_POPUP_400:
		nID = zDelta > 0 ? 0 : ID_POPUP_300;            break;
	case ID_POPUP_300:
		nID = zDelta > 0 ? ID_POPUP_400 : ID_POPUP_200; break;
	case ID_POPUP_200:
		nID = zDelta > 0 ? ID_POPUP_300 : ID_POPUP_150; break;
	case ID_POPUP_150:
		nID = zDelta > 0 ? ID_POPUP_200 : ID_POPUP_100; break;
	case ID_POPUP_100:
		nID = zDelta > 0 ? ID_POPUP_150 : ID_POPUP_75;  break;
	case ID_POPUP_75:
		nID = zDelta > 0 ? ID_POPUP_100 : ID_POPUP_66;  break;
	case ID_POPUP_66:
		nID = zDelta > 0 ? ID_POPUP_75 : ID_POPUP_50;   break;
	case ID_POPUP_50:
		nID = zDelta > 0 ? ID_POPUP_66 : ID_POPUP_33;   break;
	case ID_POPUP_33:
		nID = zDelta > 0 ? ID_POPUP_50 : ID_POPUP_25;  break;
	case ID_POPUP_25:
		nID = zDelta > 0 ? ID_POPUP_33 : 0; break;
	default:
		{
			//任意サイズからサイズ指定に移行した場合
			double dTimes = this->m_dTimes;
			if (dTimes >= 400) {
				nID = zDelta > 0 ? 0 : ID_POPUP_400;            break;
			} else if (dTimes > 300) {
				nID = zDelta > 0 ? ID_POPUP_400 : ID_POPUP_300;            break;
			} else if (dTimes > 200) {
				nID = zDelta > 0 ? ID_POPUP_300 : ID_POPUP_200;            break;
			} else if (dTimes > 150) {
				nID = zDelta > 0 ? ID_POPUP_200 : ID_POPUP_150;            break;
			} else if (dTimes > 100) {
				nID = zDelta > 0 ? ID_POPUP_150 : ID_POPUP_100;            break;
			} else if (dTimes > 75) {
				nID = zDelta > 0 ? ID_POPUP_100 : ID_POPUP_75;            break;
			} else if (dTimes > 66) {
				nID = zDelta > 0 ? ID_POPUP_75 : ID_POPUP_66;            break;
			} else if (dTimes > 50) {
				nID = zDelta > 0 ? ID_POPUP_66 : ID_POPUP_50;            break;
			} else if (dTimes > 33) {
				nID = zDelta > 0 ? ID_POPUP_50 : ID_POPUP_33;            break;
			} else if (dTimes > 25) {
				nID = zDelta > 0 ? ID_POPUP_33 : ID_POPUP_25;            break;
			} else {
				nID = zDelta > 0 ? ID_POPUP_25 : 0; break;
			}
		}
	}
	if (nID != 0)
		OnViewPopup(nID);
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

//////////////////////////////////////////////////
//水平スクロールバー
//////////////////////////////////////////////////
void CPreviewWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO info;
	this->GetScrollInfo(SB_HORZ,&info);
	int nMax = info.nMax - info.nPage + 1;
	int pos = info.nPos;
	switch (nSBCode)
	{
	case SB_LEFT:		//左端へスクロール
		pos = info.nMin; break;
	case SB_RIGHT:		//右端へスクロール
		pos = info.nMax; break;
	case SB_LINELEFT:	//左へスクロール
		pos -= 1; break;
	case SB_LINERIGHT:	//右へスクロール
		pos += 1; break;
	case SB_PAGELEFT:	//１ページ左へスクロール
		pos -= info.nPage; break;
	case SB_PAGERIGHT:	//１ページ右へスクロール
		pos += info.nPage; break;
	case SB_THUMBPOSITION:	//絶対位置へスクロール(nPos有り)
	case SB_THUMBTRACK:		//スクロール ボックスを指定位置へドラッグ(nPos有り)
		pos = nPos; break;
	case SB_ENDSCROLL:	//スクロール終了
		break;
	}
	if (pos < 0) pos = 0;
	if (pos > nMax) pos = nMax;
	this->SetScrollPos(SB_HORZ,pos,TRUE);
	MoveChildWindow();
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}
//////////////////////////////////////////////////
//垂直スクロールバー
//////////////////////////////////////////////////
void CPreviewWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO info;
	this->GetScrollInfo(SB_VERT,&info);
	int nMax = info.nMax - info.nPage + 1;
	int pos = info.nPos;
	switch (nSBCode)
	{
	case SB_TOP:		//一番上までスクロール
		pos = info.nMin; break;
	case SB_BOTTOM:		//一番下までスクロール
		pos = info.nMax; break;
	case SB_LINEDOWN:	//一行下へスクロール
		pos += 1; break;
	case SB_LINEUP:		//一行上へスクロール
		pos -= 1; break;
	case SB_PAGEDOWN:	//１ページ下へスクロール
		pos += info.nPage; break;
	case SB_PAGEUP:		//１ページ上へスクロール
		pos -= info.nPage; break;
	case SB_THUMBPOSITION:	//絶対位置へスクロール(nPos有り)
	case SB_THUMBTRACK:		//スクロール ボックスを指定位置へドラッグ(nPos有り)
		pos = nPos; break;
	case SB_ENDSCROLL:	//スクロール終了
		break;
	}
	if (pos < 0) pos = 0;
	if (pos > nMax) pos = nMax;
	this->SetScrollPos(SB_VERT,pos,TRUE);
	MoveChildWindow();
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

//////////////////////////////////////////////////
//スクロールした際のモジュール
//////////////////////////////////////////////////
void CPreviewWnd::MoveChildWindow()
{
	CPoint pt(0,0);
	if (this->m_bShowHScroll)
		pt.x = -this->GetScrollPos(SB_HORZ);
	else
		pt.x = m_ChildPos.x;
	if (this->m_bShowVScroll)
		pt.y = -this->GetScrollPos(SB_VERT);
	else
		pt.y = m_ChildPos.y;
	m_wndChild.MoveWindow(m_ChildPos.x,m_ChildPos.y,m_ChildSize.cx,m_ChildSize.cy,TRUE);
	m_ChildPos = pt;
	return;
}

//////////////////////////////////////////////////
//ウィンドウサイズ変更時に呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CPreviewWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	RecalcWindowArrangement(TRUE);
}
//////////////////////////////////////////////////
//ウィンドウ配置の計算
//BOOL bOnSize サイズ変更から呼ばれたかどうか
//////////////////////////////////////////////////
BOOL CPreviewWnd::RecalcWindowArrangement(BOOL bOnSize)
{
	//ウィンドウ配置の計算を開始
	if (m_bRecalcWindowCall)
		return TRUE;
	m_bRecalcWindowCall	= TRUE;

	//クライアントのサイズはスクロールバーを含めた大きさを指定
	CRect rcClient;
	GetClientRect(&rcClient);
	if (m_bShowHScroll)
		rcClient.bottom += m_nScrollBarHeight;
	if (m_bShowVScroll)
		rcClient.right += m_nScrollBarWidth;

	//プレビューウィンドウの倍率を変更する
	FixPreviewTimes(rcClient.right,rcClient.bottom);

	BOOL bUseHScroll;		//水平スクロールバーは必要かどうか
	BOOL bUseVScroll;		//垂直スクロールバーは必要かどうか
	CSize diff;
	diff.cx = rcClient.right - m_ChildSize.cx;
	diff.cy = rcClient.bottom - m_ChildSize.cy;
	if (diff.cx < 0)	//必ず水平スクロールバーが必要
	{
		bUseHScroll = TRUE;
		if (diff.cy < 0)	//必ず垂直スクロールバーが必要
		{
			bUseVScroll = TRUE;
		} else if (diff.cy < m_nScrollBarWidth)	//場合によっては垂直スクロールバーが必要
		{
			bUseVScroll = TRUE;
		} else				//垂直スクロールバーは不要
		{
			bUseVScroll = FALSE;
		}
	} else if (diff.cx < m_nScrollBarWidth)		//場合によっては水平スクロールバーが必要
	{
		if (diff.cy < 0)	//必ず垂直スクロールバーが必要
		{
			bUseHScroll = TRUE;
			bUseVScroll = TRUE;
		} else if (diff.cy < m_nScrollBarWidth)	//場合によっては垂直スクロールバーが必要
		{
			bUseHScroll = FALSE;
			bUseVScroll = FALSE;
		} else				//垂直スクロールバーは不要
		{
			bUseHScroll = FALSE;
			bUseVScroll = FALSE;
		}
	}
	else	//水平スクロールバーは不要
	{
		bUseHScroll = FALSE;
		if (diff.cy < 0)	//必ず垂直スクロールバーが必要
		{
			bUseVScroll = TRUE;
		} else if (diff.cy < m_nScrollBarWidth)	//場合によっては垂直スクロールバーが必要
		{
			bUseVScroll = FALSE;
		}
		else				//垂直スクロールバーは不要
		{
			bUseVScroll = FALSE;
		}
	}
	//スクロールバーが必要な場合はクライアントサイズが変更になる
	if (bUseHScroll)
		rcClient.bottom -= m_nScrollBarHeight;
	if (bUseVScroll)
		rcClient.right -= m_nScrollBarWidth;
	//このタイミングでスクロールバーの有無を含めたクライアント領域のサイズが決定される

	//子ウィンドウの座標(クライアント座標)
	CPoint pos(0,0);
	//スクロールバー構造体の初期化
	SCROLLINFO ScrollInfo;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;	//SIF_ALL
	ScrollInfo.nMin = 0;				//最小のスクロール位置
	if (bUseHScroll)
	{
		ScrollInfo.nMax = m_ChildSize.cx;	//最大のスクロール位置
		ScrollInfo.nPage = rcClient.right;	//ページサイズ(PageUp/PageDownの移動量)
		if (m_bShowHScroll)	//以前もスクロールバーがあった場合
		{
			ScrollInfo.nPos = - m_ChildPos.x;
			if (ScrollInfo.nPos + (int)ScrollInfo.nPage > ScrollInfo.nMax)
				ScrollInfo.nPos = ScrollInfo.nMax - ScrollInfo.nPage;
		} else {
			ScrollInfo.nPos = 0;//(m_ChildSize.cx - rcClient.right) / 2;				//現在の位置
		}
		this->SetScrollInfo(SB_HORZ,&ScrollInfo,FALSE);
		m_bShowHScroll = TRUE;
		pos.x = - ScrollInfo.nPos;
	} else {
		ScrollInfo.nMax = 0;				//最大のスクロール位置
		ScrollInfo.nPage = 0;				//ページサイズ(PageUp/PageDownの移動量)
		ScrollInfo.nPos = 0;				//現在の位置
		this->SetScrollInfo(SB_HORZ,&ScrollInfo,FALSE);
		m_bShowHScroll = FALSE;
		pos.x = (rcClient.right - m_ChildSize.cx) / 2;	//中央に移動
	}

	if (bUseVScroll)
	{
		ScrollInfo.nMax = m_ChildSize.cy;	//最大のスクロール位置
		ScrollInfo.nPage = rcClient.bottom;	//ページサイズ(PageUp/PageDownの移動量)
		if (m_bShowVScroll)
		{
			ScrollInfo.nPos = - m_ChildPos.y;
			if (ScrollInfo.nPos + (int)ScrollInfo.nPage > ScrollInfo.nMax)
				ScrollInfo.nPos = ScrollInfo.nMax - ScrollInfo.nPage;
		} else {
			ScrollInfo.nPos = 0;//(m_ChildSize.cy - rcClient.bottom) / 2;				//現在の位置
		}
		this->SetScrollInfo(SB_VERT,&ScrollInfo,FALSE);
		m_bShowVScroll = TRUE;
		pos.y = - ScrollInfo.nPos;
	} else {
		ScrollInfo.nMax = 0;				//最大のスクロール位置
		ScrollInfo.nPage = 0;				//ページサイズ(PageUp/PageDownの移動量)
		ScrollInfo.nPos = 0;				//現在の位置
		this->SetScrollInfo(SB_VERT,&ScrollInfo,FALSE);
		m_bShowVScroll = FALSE;
		pos.y = (rcClient.bottom - m_ChildSize.cy) / 2;	//中央に移動
	}

	m_wndChild.MoveWindow(pos.x,pos.y,m_ChildSize.cx,m_ChildSize.cy,FALSE);
	m_ChildPos = pos;	//現在の子プレビューウィンドウの位置を記憶
	m_ClientSize.SetSize(rcClient.right,rcClient.bottom);

	//画面全体の更新通知を行う
	//TODO ここをコメントアウトする
	//InvalidateRect(NULL, FALSE);

	m_bRecalcWindowCall	= FALSE;
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルの変更通知
//////////////////////////////////////////////////
BOOL CPreviewWnd::ChangeProfileNotify()
{
	m_bFirstFlag = FALSE;

	//プレビュー開始
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (pDSA->IsCanPreview())
	{
		SetPreviewWnd();		//プレビューウィンドウの割り当てを行う
		SetPreviewSize(FALSE);

		//プレビューの開始(フィルタグラフの実行)
		//CMainFrame* frame = (CMainFrame*)::AfxGetMainWnd();
		//CPropertyFormView* propertywnd =  frame->GetPropertyWnd();
		//CInputPropertyDialog* pInputTab = propertywnd->GetInputTab();
		//assert(pInputTab);
		//assert(pInputTab->GetSafeHwnd());

		BOOL bRet = pDSA->RunFilter();
		//pInputTab->OnCbnSelchangeVideoResolutionCombo();	//解像度変更がプレビューの開始トリガとなる
	}
	m_bFirstFlag = TRUE;
	return TRUE;
}

//////////////////////////////////////////////////
//エンコードの開始と停止
//////////////////////////////////////////////////
BOOL CPreviewWnd::Encode(BOOL bStart)
{
	return TRUE;
}
//////////////////////////////////////////////////
//フォーカスの設定
//////////////////////////////////////////////////
void CPreviewWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
}
//////////////////////////////////////////////////
//右クリックでの拡大縮小メニューを表示
//////////////////////////////////////////////////
void CPreviewWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	CMenu cMenu;
	cMenu.LoadMenu(IDR_VIEW_POPUP);
	CMenu* pPopup = cMenu.GetSubMenu(0);
	assert(pPopup);
	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow();

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (pSetting->m_bPreviewForceOff == TRUE)
		pPopup->EnableMenuItem(ID_POPUP_SHOWPREVIEW,MF_GRAYED | MF_BYCOMMAND);

	if (m_bChildShow)
		pPopup->CheckMenuItem(ID_POPUP_SHOWPREVIEW,MF_CHECKED|MF_BYCOMMAND);
	if (m_bFixByClient)
		pPopup->CheckMenuItem(ID_POPUP_FIXWINDOWSIZE,MF_CHECKED|MF_BYCOMMAND);

	if (m_bFixByWindow)
	{
		pPopup->CheckMenuItem(ID_POPUP_FIXPREVIEWSIZE,MF_CHECKED|MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_400,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_300,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_200,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_150,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_100,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_75,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_66,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_50,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_33,MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_POPUP_25,MF_GRAYED | MF_BYCOMMAND);
	} else {
		if (m_nTimesID != 0)
			pPopup->CheckMenuItem(m_nTimesID,MF_CHECKED|MF_BYCOMMAND);
	}

	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,this);
	cMenu.DestroyMenu();

	CWnd::OnRButtonDown(nFlags, point);
}
//////////////////////////////////////////////////
//プレビューの表示の非表示を切り替える(ポップアップメニュー)
//////////////////////////////////////////////////
void CPreviewWnd::OnPopupShowpreview()
{
	if (m_wndChild.GetSafeHwnd() == NULL)
		return;
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (m_bChildShow) {
		m_wndChild.ShowWindow(SW_HIDE);
		m_bChildShow = FALSE;
		pDSA->ShowPreviewWindow(FALSE);
	} else {
		m_wndChild.ShowWindow(SW_SHOW);
		m_bChildShow = TRUE;
		pDSA->ShowPreviewWindow(TRUE);
	}
}
//////////////////////////////////////////////////
//拡大縮小の右クリックメニューを選択する
//////////////////////////////////////////////////
void CPreviewWnd::OnViewPopup(UINT nID)
{
	switch (nID)
	{
	case ID_POPUP_400:
		m_dTimes = 400;break;
	case ID_POPUP_300:
		m_dTimes = 300;break;
	case ID_POPUP_200:
		m_dTimes = 200;break;
	case ID_POPUP_150:
		m_dTimes = 150;break;
	case ID_POPUP_100:
		m_dTimes = 100;break;
	case ID_POPUP_75:
		m_dTimes = 75;break;
	case ID_POPUP_66:
		m_dTimes = 66;break;
	case ID_POPUP_50:
		m_dTimes = 50;break;
	case ID_POPUP_33:
		m_dTimes = 33;break;
	case ID_POPUP_25:
		m_dTimes = 25;break;
	};
	m_nTimesID = nID;
	SetPreviewSize(TRUE);
	return;
}
//////////////////////////////////////////////////
//現在のプレビューウィンドウが綺麗に収まるように
//親ウィンドウサイズを変更する
//////////////////////////////////////////////////
void CPreviewWnd::OnPopupFixwindowsize()
{
	if (m_bFixByClient)
		m_bFixByClient = FALSE;
	else
	{
		m_bFixByClient = TRUE;
		m_bFixByWindow = FALSE;	//相反属性
		SetPreviewSize(TRUE);
	}
}
//////////////////////////////////////////////////
//ウィンドウサイズに収まるようにプレビューの大きさを変更する
//////////////////////////////////////////////////
void CPreviewWnd::OnPopupFixpreviewsize()
{
	if (m_bFixByWindow)
		m_bFixByWindow = FALSE;
	else
	{
		m_bFixByWindow = TRUE;
		m_bFixByClient = FALSE;	//相反属性
		m_nTimesID = 0;
		RecalcWindowArrangement(TRUE);
		//FixPreviewTimes(m_ClientSize.cx,m_ClientSize.cy);
	}
}


//////////////////////////////////////////////////
//MainFrameをプレビューサイズに合わせる
//////////////////////////////////////////////////
void CPreviewWnd::FixWindowSize()
{
	CWnd* pParent = ::AfxGetMainWnd();
	if (!pParent)
		return;
	if (!pParent->GetSafeHwnd())
		return;
	if (!this->m_bFirstFlag)	//初回起動時にはこの設定は適用しない
		return;
	//assert(pParent);
	CRect rcParentWindowRect;
	pParent->GetWindowRect(&rcParentWindowRect);

	CSize diff(0,0);	//増分させるクライアント領域のサイズ
	if (m_ChildPos.x < 0)
		diff.cx = m_ChildSize.cx - m_ClientSize.cx;
	else
		diff.cx  = m_ClientSize.cx - m_ChildSize.cx;
	if (this->m_bShowVScroll)
		diff.cx += this->m_nScrollBarWidth;

	if (m_ChildPos.y < 0)
		diff.cy = m_ChildSize.cy - m_ClientSize.cy;
	else
		diff.cy  = m_ClientSize.cy - m_ChildSize.cy;
	if (this->m_bShowHScroll)
		diff.cy += this->m_nScrollBarHeight;

	rcParentWindowRect.right = rcParentWindowRect.right - diff.cx;
	rcParentWindowRect.bottom = rcParentWindowRect.bottom - diff.cy;
	pParent->MoveWindow(&rcParentWindowRect,TRUE);
	//pParent->InvalidateRect(NULL,TRUE);
	//pParent->UpdateWindow();
}

//////////////////////////////////////////////////
//プレビューの拡大倍率を現在のプレイヤーサイズとぴったりになるように調整する
//////////////////////////////////////////////////
void CPreviewWnd::FixPreviewTimes(int nClientWidth ,int nClientHeight)
{
	if (m_bFixByWindow)
	{
		CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
		int nWidth = 0;
		int nHeight = 0;
		pDSA->GetCurrentVideoSize(nWidth,nHeight);
		if (nWidth == 0 || nHeight == 0) {
			//AfxMessageBox(_T("ビデオの解像度が0です"),MB_OK|MB_ICONINFORMATION);
			return;
		}
		//元の解像度
		double dWidthTimes = ((double)nClientWidth / (double)nWidth);
		double dHeightTimes = ((double)nClientHeight / (double)nHeight);
		double dTimes = dWidthTimes > dHeightTimes ? dHeightTimes : dWidthTimes;
		if (dTimes == 0.0) {
			//AfxMessageBox(_T("ビデオの倍率が0です"),MB_OK|MB_ICONINFORMATION);
			return;
		}

		//この倍率値を変更する
		CSize size;
		size.cx = (int) ((double)nWidth * dTimes);
		size.cy = (int) ((double)nHeight * dTimes);
		if (size.cx == 0 || size.cy == 0) {
			//AfxMessageBox(_T("プレビューのサイズが0です"),MB_OK|MB_ICONINFORMATION);
			return;
		}

		this->m_dTimes = dTimes;
		m_ChildSize.cx = size.cx;
		m_ChildSize.cy = size.cy;
		m_wndChild.MoveWindow(0,0,m_ChildSize.cx,m_ChildSize.cy,FALSE);
		CRect rcWindow;
		rcWindow.SetRect(0,0,m_ChildSize.cx,m_ChildSize.cy);
		pDSA->ResizePreviewWindow(rcWindow);

		this->m_dTimes = this->m_dTimes * 100;
		SetStatusTimes(this->m_dTimes);
		InvalidateRect(NULL,FALSE);
	}
}

//////////////////////////////////////////////////
//ステータスバーに拡大倍率を書き込む
//////////////////////////////////////////////////
void CPreviewWnd::SetStatusTimes(double dTimes)
{
	CMainFrame* frame = (CMainFrame*)::AfxGetMainWnd();
	if (!frame) return;
	CMyStatusBar* status = frame->GetStatusBar();
	if (!status) return;
	status->SetTimes(dTimes);
	return;
}


BOOL CPreviewWnd::OnEraseBkgnd(CDC* pDC)
{
	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}
