
// MainFrm.cpp : CMainFrame クラスの実装
//

#include "stdafx.h"
#include "KTE.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define WM_STOP_ENCODE (WM_APP + 102)

//////////////////////////////////////////////////
//CObjectクラスから継承している場合は実行時型情報(RTTI)として、クラスの型を定義しなければならない
//////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

//メッセージマップ開始
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()

	ON_COMMAND(ID_SHOW_TOOLBAR, &CMainFrame::OnShowToolbar)
	ON_COMMAND(ID_SHOW_STATUSBAR, &CMainFrame::OnShowStatusbar)
	ON_COMMAND(ID_SHOW_PROPERTY, &CMainFrame::OnShowProperty)
	ON_COMMAND(ID_SHOW_MONITOR, &CMainFrame::OnShowMonitor)
	ON_COMMAND(ID_SHOW_PREVIEW, &CMainFrame::OnShowPreview)
	ON_COMMAND(ID_SHOW_VOLUME, &CMainFrame::OnShowVolume)

	ON_MESSAGE(WM_GRAPH_NOTIFY,&CMainFrame::OnGraphNotify)
	ON_MESSAGE(NOTIFY_TRAYICON,&CMainFrame::OnTrayIcon)
	ON_COMMAND(ID_SHOW_WINDOW, &CMainFrame::OnShowWindowFromTaskTray)

	ON_MESSAGE(WM_STOP_ENCODE,&CMainFrame::OnNotifyStopEncode)
	ON_COMMAND(ID_PLAY_ENCODE, &CMainFrame::OnStartEncode)
	ON_COMMAND(ID_STOP_ENCODE, &CMainFrame::OnStopEncode)

	ON_UPDATE_COMMAND_UI_RANGE(ID_PLAY_ENCODE, ID_STOP_ENCODE, &CMainFrame::OnUpdateEncode )

	ON_UPDATE_COMMAND_UI(ID_PROFILE_NEW, &CMainFrame::OnUpdateProfile)
	ON_UPDATE_COMMAND_UI(ID_PROFILE_OPEN, &CMainFrame::OnUpdateProfile)
END_MESSAGE_MAP()
//メッセージマップ終了

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CMainFrame::CMainFrame()
{
	m_nSplitterColumnWidth[0] = 0;
	m_nSplitterColumnWidth[1] = 0;
	m_nSplitterRowHeight[0] = 0;
	m_nSplitterRowHeight[1] = 0;
	m_nSplitterPreviewSize[0] = 0;
	m_nSplitterPreviewSize[1] = 0;
	//m_nSplitterPreviewPos = 0;

	m_bSplitterInit = FALSE;

	m_StartStopThread = NULL;
	m_bStartStopThread = FALSE;
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CMainFrame::~CMainFrame()
{
}

//////////////////////////////////////////////////
//ウィンドウ生成時のスタイルを編集する
//////////////////////////////////////////////////
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	//初期標準のウィンドウサイズ
	cs.cx = 750;
	cs.cy = 580;

	//WS_OVERLAPPEDはオーバラップ ウィンドウは、通常キャプションと境界を持ちます。
	cs.style = WS_OVERLAPPED | WS_CAPTION | 
		FWS_ADDTOTITLE | 
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX
		| WS_SYSMENU | WS_THICKFRAME;

	cs.style &= ~WS_BORDER;
	cs.dwExStyle |= WS_EX_CLIENTEDGE;

	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

//////////////////////////////////////////////////
//WM_CREATEのイベントハンドラ
//////////////////////////////////////////////////
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//ウィンドウサイズの変更
	RECT rcWindow = pSetting->m_rcMainWindow;
	if ((rcWindow.right - rcWindow.left != 0) && (rcWindow.bottom - rcWindow.top != 0))
	{
		WINDOWPLACEMENT wnd;
		ZeroMemory(&wnd,sizeof(WINDOWPLACEMENT));
		wnd.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(&wnd);
		wnd.length = sizeof(WINDOWPLACEMENT);
		wnd.flags = 0;
		wnd.showCmd = SW_SHOW;
		wnd.rcNormalPosition = pSetting->m_rcMainWindow;
		SetWindowPlacement(&wnd);
	}

	//ステータスバーの作成
	if (!m_wndStatusBar.Create(this)) {
		TRACE0("ステータス バーの作成に失敗しました。\n");
		return -1;
	}
	//ツールバーの作成
	if (!m_wndToolBar.Create(this)) {
		TRACE0("ツール バーの作成に失敗しました。\n");
		return -1;
	}

	//タイトルバーの変更
	ChangeTitleBar();

	//ウィンドウアイコンの変更
	SetIcon(AfxGetApp()->LoadIcon(IDI_BLUE_OFF_ICON),TRUE);

	//タスクバーアイコンを作成する(表示はまだしない)
	m_TaskTray.Init(this);

	//画面表示設定の復元
	if (!pSetting->m_bShowStatusBar)
		OnShowStatusbar();
	if (!pSetting->m_bShowToolBar)
		OnShowToolbar();
	if (!pSetting->m_bShowPropertyWindow)
		OnShowProperty();
	if (!pSetting->m_bShowMonitorWindow)
		OnShowMonitor();
	if (!pSetting->m_bShowPreviewWindow)
		OnShowPreview();
	if (!pSetting->m_bShowVolumeWindow)
		OnShowVolume();

	//フィルタグラフからのGraphNotifyメッセージ通知の設定
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	pDSA->SetCallbackHWnd(this->GetSafeHwnd());
	return 0;
}

//////////////////////////////////////////////////
//ウィンドウ終了前の処理
//////////////////////////////////////////////////
void CMainFrame::OnClose()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
#ifndef _DEBUG
	if (pDSA->IsEncode())
	{
		if (AfxMessageBox(_T("エンコード中ですが終了します。\nよろしいですか？"),MB_YESNO) == IDNO)
		{
			return;
		}
	}
#endif

	//エンコード停止中/エンコード再生中 であった場合には、停止するまで待機する
	if (IsStartStopThread() == TRUE) {
		StopThreadMessageLoop();
	}

	//エンコード中であればエンコード停止して終了する
	if (pDSA->IsEncode() == TRUE)
	{
		this->PreStopEncode();
		pDSA->StopEncode(TRUE);
		this->PostStopEncode();
	}

	CFrameWnd::OnClose();
}
//////////////////////////////////////////////////
//ウィンドウ破棄時に呼ばれる
//////////////////////////////////////////////////
void CMainFrame::OnDestroy()
{
	//TRACE0("CMainFrame::OnDestroy()\r\n");

	//ウィンドウ位置の保存行う
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	WINDOWPLACEMENT wpl;
	GetWindowPlacement(&wpl);
	pSetting->m_rcMainWindow = wpl.rcNormalPosition;

	//現在の分割ウィンドウの状態を取得してみる
	int dummy;
	m_wndLeftRightSplitter.GetColumnInfo(0,pSetting->m_nMainSplitterWidth[0],dummy);
	m_wndLeftRightSplitter.GetColumnInfo(1,pSetting->m_nMainSplitterWidth[1],dummy);
	m_wndTopButtomSplitter.GetRowInfo(0,pSetting->m_nMainSplitterHeight[0],dummy);
	m_wndTopButtomSplitter.GetRowInfo(1,pSetting->m_nMainSplitterHeight[1],dummy);
	m_wndPreviewSplitter.GetRowInfo(0,pSetting->m_nMainSplitterPreview[0],dummy);
	m_wndPreviewSplitter.GetRowInfo(1,pSetting->m_nMainSplitterPreview[1],dummy);

	//画面表示・非表示の取得
	pSetting->m_bShowToolBar        = IsShowView(ID_SHOW_TOOLBAR);
	pSetting->m_bShowStatusBar      = IsShowView(ID_SHOW_STATUSBAR);
	pSetting->m_bShowPropertyWindow = IsShowView(ID_SHOW_PROPERTY);
	pSetting->m_bShowMonitorWindow  = IsShowView(ID_SHOW_MONITOR);
	pSetting->m_bShowPreviewWindow  = IsShowView(ID_SHOW_PREVIEW);
	pSetting->m_bShowVolumeWindow   = IsShowView(ID_SHOW_VOLUME);

	//タスクトレイアイコンの削除
	m_TaskTray.Exit();

	CFrameWnd::OnDestroy();
}

//////////////////////////////////////////////////
// CMainFrame 診断
//////////////////////////////////////////////////
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG
//////////////////////////////////////////////////



//////////////////////////////////////////////////
//タイトルバーの変更
//////////////////////////////////////////////////
void  CMainFrame::ChangeTitleBar()
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	//ウィンドウタイトルは KoToEncoder - ショートプロファイル名.kte
	CString strTitle;
	if (pSetting->m_strProfileShortPath.IsEmpty() == FALSE) {
		strTitle.Format(_T("%s - %s"),pSetting->m_strProgramName,pSetting->m_strProfileShortPath);
	} else {
		if (pSetting->m_strProfileFullPath.IsEmpty() == FALSE) {
			strTitle.Format(_T("%s - %s"),pSetting->m_strProgramName,pSetting->m_strProfileFullPath);
		} else {
			strTitle.Format(_T("%s - 空のプロファイル"),pSetting->m_strProgramName);
		}
	}
	CWnd::SetWindowText((LPCTSTR)strTitle);
	return;
}
//////////////////////////////////////////////////
//プロファイルの変更通知
//////////////////////////////////////////////////
BOOL CMainFrame::ChangeProfileNotify()
{
	//DSAに通達
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (!pDSA->ChangeProfileNotify())
		return FALSE;
	//プロパティウィドウに通知
	if (!GetPropertyWnd()->ChangeProfileNotify())
		return FALSE;
	//プレビューウィンドウに通知
	if (!GetPreviewWnd()->ChangeProfileNotify())
		return FALSE;
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルの保存
//////////////////////////////////////////////////
BOOL CMainFrame::SaveProfileSetting()
{
	//DSAに通達
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (!pDSA->SaveProfileSetting())
		return FALSE;
	//プロパティウィンドウに通達
	if (!GetPropertyWnd()->SaveProfileSetting())
		return FALSE;
	//プレビューウィンドウ自体にプロファイル保存の設定がない
	//if (!GetPreviewWnd()->SaveProfileSetting())
	//	return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////
//グラフからの通知メッセージ
//////////////////////////////////////////////////
LRESULT CMainFrame::OnGraphNotify(WPARAM wParam,LPARAM lParam)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	return pDSA->OnGraphNotify();
}

//////////////////////////////////////////////////
//タスクアイコンのメッセージ通知イベントハンドラ
//////////////////////////////////////////////////
LRESULT CMainFrame::OnTrayIcon(WPARAM wParam,LPARAM lParam)
{
	return m_TaskTray.OnTrayIcon(wParam,lParam);
}
//////////////////////////////////////////////////
//タスクトレイアイコンの右クリックメニュー(ウィンドウの表示)
//////////////////////////////////////////////////
void CMainFrame::OnShowWindowFromTaskTray()
{
	SendMessage(NOTIFY_TRAYICON,TRAY_ID,WM_LBUTTONDBLCLK);
}

//////////////////////////////////////////////////
//WM_SETFOCUSのイベントハンドラ
//////////////////////////////////////////////////
void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// ビュー ウィンドウにフォーカスを与えます
	if (m_wndPreviewWnd.m_hWnd != NULL)
		m_wndPreviewWnd.SetFocus();
}

//////////////////////////////////////////////////
//分割ビューを作成する
//////////////////////////////////////////////////
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	//分割ウィンドウの作成
	if(!m_wndTopButtomSplitter.CreateStatic(this,2,1,WS_CHILD | WS_VISIBLE)) {
		TRACE0("分割ウィンドウの作成に失敗しました。\n");
		return FALSE;
	}
	if(!m_wndLeftRightSplitter.CreateStatic(&m_wndTopButtomSplitter,1, 2, WS_CHILD | WS_VISIBLE,
		m_wndTopButtomSplitter.IdFromRowCol(0,0))) {
		TRACE0("分割ウィンドウの作成に失敗しました。\n");
		return FALSE;
	}
	if (!m_wndPreviewSplitter.CreateStatic(&m_wndLeftRightSplitter, 2, 1, WS_CHILD | WS_VISIBLE,
		m_wndLeftRightSplitter.IdFromRowCol(0,1))) {
		TRACE0("分割ウィンドウの作成に失敗しました。\n");
		return FALSE;
	}

	//上左側のビュー(プロパティページ)
	if (!m_wndLeftRightSplitter.CreateView(
		0, 0,
		RUNTIME_CLASS(CPropertyFormView),
		CSize(0,0),
		pContext)) {
		return FALSE;
	}

	//上右側のビュー(ボリュームビューページ)
	if (!m_wndVolumeWnd.CreateEx(NULL,NULL,_T(""),
		AFX_WS_DEFAULT_VIEW,
		CRect(0,0,0,0),&m_wndPreviewSplitter,m_wndPreviewSplitter.IdFromRowCol(0, 0))) {
		return FALSE;
	}
	//上右側のビュー(プレビューページ)
	if (!m_wndPreviewWnd.CreateEx(NULL,NULL,_T(""),
		AFX_WS_DEFAULT_VIEW,
		CRect(0,0,0,0),&m_wndPreviewSplitter,m_wndPreviewSplitter.IdFromRowCol(1, 0))) {
		return FALSE;
	}

	//下側のビュー(モニターページ)
	if(!m_wndTopButtomSplitter.CreateView(
		1, 0,
		RUNTIME_CLASS(CMonitorFormView),
		CSize(0,0),
		pContext)) {
		return FALSE;
	}

	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

//////////////////////////////////////////////////
//WM_SIZEのイベントハンドラ
//////////////////////////////////////////////////
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	//他のウィンドウが出来てない場合はすぐに戻る
	if (m_wndToolBar.GetSafeHwnd() == NULL || m_wndStatusBar.GetSafeHwnd() == NULL ||
		m_wndLeftRightSplitter.GetSafeHwnd() == NULL || m_wndTopButtomSplitter.GetSafeHwnd() == NULL || m_wndPreviewSplitter.GetSafeHwnd() == NULL)
	{
		CFrameWnd::OnSize(nType, cx, cy);
		return;
	}

	//ウィンドウ最小化時の処理
	if (nType == SIZE_MINIMIZED)
	{
		CFrameWnd::OnSize(nType, cx, cy);
		CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
		if (pSetting->m_bMiniTaskBar)
		{
			//タスクバーアイコンを登録する
			m_TaskTray.NotifyIcon();
			return;
		}
	}

	//ウィンドウのクライアント領域の取得
	CRect rcClient;
	GetClientRect(&rcClient);
	//ツールバーとステータスバーの高さを引く
	CRect rcToolBar(0,0,0,0);
	if (m_wndToolBar.IsVisible())
		m_wndToolBar.GetWindowRect(&rcToolBar);
	CRect rcStatusBar(0,0,0,0);
	if (m_wndStatusBar.IsVisible())
		m_wndStatusBar.GetWindowRect(&rcStatusBar);

	rcClient.bottom = rcClient.bottom - rcToolBar.Height() - rcStatusBar.Height();
	if (rcClient.bottom < 0)
		rcClient.bottom = 0;
	if (rcClient.right < 0)
		rcClient.right = 0;
	//この時点でクライアント領域のサイズがrcClient.bottomとrcClient.rightに入っている
	if (rcClient.right == 0 || rcClient.bottom == 0) {
		CFrameWnd::OnSize(nType, cx, cy);
		return;
	}

	//分割ウィンドウの線の幅を取得
	if (m_bSplitterInit == FALSE)
		CalcSplitterLine(rcClient.right,rcClient.bottom);

	//初回起動の場合のペイン領域を設定
	if (!m_bSplitterInit)
	{
		CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
		if (pSetting->m_nMainSplitterWidth[0] == 0 || pSetting->m_nMainSplitterWidth[1] == 0 ||
			pSetting->m_nMainSplitterHeight[0] == 0 || pSetting->m_nMainSplitterHeight[1] == 0 ||
			pSetting->m_nMainSplitterPreview[0] == 0 || pSetting->m_nMainSplitterPreview[1] == 0)
		{
			//プロパティウィンドウの最大幅
			const int nColumnMaxWidth = 280;
			if (rcClient.right >= nColumnMaxWidth + m_nSplitterLineWidth) {	//左右に余白有り
				m_nSplitterColumnWidth[0] = nColumnMaxWidth;
				m_nSplitterColumnWidth[1] = rcClient.right - nColumnMaxWidth - m_nSplitterLineWidth;
			} else {	//左右に余白無し
				m_nSplitterColumnWidth[0] = rcClient.right - m_nSplitterLineWidth;
				m_nSplitterColumnWidth[1] = 0;
			}
			//モニターウィンドウの初期固定値(iniから読む値)
			const int nRowDefaultHeight = 170;
			if (rcClient.bottom >= nRowDefaultHeight + m_nSplitterLineHegiht) {	//上下に余白有り
				m_nSplitterRowHeight[0] = rcClient.bottom - nRowDefaultHeight - m_nSplitterLineHegiht;
				m_nSplitterRowHeight[1] = nRowDefaultHeight;
			} else {	//上下に余白無し
				m_nSplitterRowHeight[0] = 0;
				m_nSplitterRowHeight[1] = rcClient.bottom - m_nSplitterLineHegiht;
			}
			//プレビューウィンドウの初期固定値(iniから読む値)
			const int nPreviewDefaultHeight = 40;
			if (m_nSplitterColumnWidth[1] >= nPreviewDefaultHeight + m_nSplitterLineHegiht) {	//上下に余白有り
				m_nSplitterPreviewSize[0] = nPreviewDefaultHeight;
				m_nSplitterPreviewSize[1] = m_nSplitterColumnWidth[1] - nPreviewDefaultHeight - m_nSplitterLineHegiht;
			} else {	//上下に余白無し
				m_nSplitterPreviewSize[0] = m_nSplitterColumnWidth[1] - m_nSplitterLineHegiht;
				m_nSplitterPreviewSize[1] = 0;
			}
		} else {
			//有効な値が入っている
			m_nSplitterColumnWidth[0] = pSetting->m_nMainSplitterWidth[0];
			m_nSplitterColumnWidth[1] = pSetting->m_nMainSplitterWidth[1];
			m_nSplitterRowHeight[0] = pSetting->m_nMainSplitterHeight[0];
			m_nSplitterRowHeight[1] = pSetting->m_nMainSplitterHeight[1];
			m_nSplitterPreviewSize[0] = pSetting->m_nMainSplitterPreview[0];
			m_nSplitterPreviewSize[1] = pSetting->m_nMainSplitterPreview[1];
		}
		m_bSplitterInit = TRUE;
	}
	else
	{
		//現在の分割ウィンドウの状態を取得してみる
		int dummy;
		int nSplitterWidth[2];
		int nSplitterHeight[2];
		int nPreviewPos[2];
		m_wndLeftRightSplitter.GetColumnInfo(0,nSplitterWidth[0],dummy);
		m_wndLeftRightSplitter.GetColumnInfo(1,nSplitterWidth[1],dummy);
		m_wndTopButtomSplitter.GetRowInfo(0,nSplitterHeight[0],dummy);
		m_wndTopButtomSplitter.GetRowInfo(1,nSplitterHeight[1],dummy);
		m_wndPreviewSplitter.GetRowInfo(0,nPreviewPos[0],dummy);
		m_wndPreviewSplitter.GetRowInfo(1,nPreviewPos[1],dummy);

		if (nSplitterWidth[0] + m_nSplitterLineWidth <= rcClient.right) {
			m_nSplitterColumnWidth[0] = nSplitterWidth[0];
			m_nSplitterColumnWidth[1] = rcClient.right - nSplitterWidth[0] - m_nSplitterLineWidth;
		} else {
			m_nSplitterColumnWidth[0] = rcClient.right - m_nSplitterLineWidth;
			m_nSplitterColumnWidth[1] = 0;
		}
		if (nSplitterHeight[1] + m_nSplitterLineHegiht <= rcClient.bottom) {
			m_nSplitterRowHeight[0] = rcClient.bottom - nSplitterHeight[1] - m_nSplitterLineHegiht;
			m_nSplitterRowHeight[1] = nSplitterHeight[1];
		} else {
			m_nSplitterRowHeight[0] = 0;
			m_nSplitterRowHeight[1] = rcClient.bottom - m_nSplitterLineHegiht;
		}
		if (nPreviewPos[0] + m_nSplitterLineHegiht <= m_nSplitterColumnWidth[1]) {
			m_nSplitterPreviewSize[0] = nPreviewPos[0];
			m_nSplitterPreviewSize[1] = m_nSplitterColumnWidth[1] - nPreviewPos[0] - m_nSplitterLineHegiht;
		} else {
			m_nSplitterPreviewSize[0] = m_nSplitterColumnWidth[1] - m_nSplitterLineHegiht;
			m_nSplitterPreviewSize[1] = 0;
		}
	}
	//ペインを設定する
	assert(m_nSplitterColumnWidth[0] >= 0);
	assert(m_nSplitterColumnWidth[1] >= 0);
	assert(m_nSplitterRowHeight[0] >= 0);
	assert(m_nSplitterRowHeight[1] >= 0);
	assert(m_nSplitterPreviewSize[0] >= 0);
	assert(m_nSplitterPreviewSize[1] >= 0);

	m_wndLeftRightSplitter.SetColumnInfo(0,m_nSplitterColumnWidth[0],0);
	m_wndLeftRightSplitter.SetColumnInfo(1,m_nSplitterColumnWidth[1],0);
	m_wndTopButtomSplitter.SetRowInfo(0,m_nSplitterRowHeight[0],0);
	m_wndTopButtomSplitter.SetRowInfo(1,m_nSplitterRowHeight[1],0);
	m_wndPreviewSplitter.SetRowInfo(0,m_nSplitterPreviewSize[0],0);
	m_wndPreviewSplitter.SetRowInfo(1,m_nSplitterPreviewSize[1],0);

	//分割ウィンドウに対して調節した後に正しく再表示する
	//ペイン内部のウィンドウは自動的に分割ウィンドウ内の大きさに
	//拡大されて表示されるので問題ない
	m_wndLeftRightSplitter.RecalcLayout();
	m_wndTopButtomSplitter.RecalcLayout();
	m_wndPreviewSplitter.RecalcLayout();

	CFrameWnd::OnSize(nType, cx, cy);

	return;
}
//////////////////////////////////////////////////
//分割ウィンドウの線の幅を取得する(サブルーチン)
//m_nSplitterLineWidth 線の幅
//m_nSplitterLineHegiht 線の高さ
//////////////////////////////////////////////////
void CMainFrame::CalcSplitterLine(int nClientWidth,int nClientHeight)
{
	m_wndPreviewSplitter.SetRowInfo(0,0,0);
	m_wndPreviewSplitter.SetRowInfo(1,0,0);
	m_wndLeftRightSplitter.SetColumnInfo(0,0,0);
	m_wndLeftRightSplitter.SetColumnInfo(1,0,0);
	m_wndTopButtomSplitter.SetRowInfo(0,0,0);
	m_wndTopButtomSplitter.SetRowInfo(1,0,0);

	m_wndPreviewSplitter.RecalcLayout();
	m_wndLeftRightSplitter.RecalcLayout();
	m_wndTopButtomSplitter.RecalcLayout();
	int dummy;
	int nSplitterWidth;
	int nSplitterHeight;
	m_wndLeftRightSplitter.GetColumnInfo(1,nSplitterWidth,dummy);
	m_nSplitterLineWidth = nClientWidth - nSplitterWidth;
	m_wndTopButtomSplitter.GetRowInfo(1,nSplitterHeight,dummy);
	m_nSplitterLineHegiht = nClientHeight - nSplitterHeight;
	m_wndTopButtomSplitter.GetRowInfo(0,nSplitterHeight,dummy);
	//CDirectShowEtc::OutputDebugWindow(_T("m_nSplitterLineWidth  = %d\n"),m_nSplitterLineWidth);
	//CDirectShowEtc::OutputDebugWindow(_T("m_nSplitterLineHegiht = %d\n"),m_nSplitterLineHegiht);
	assert(m_nSplitterLineWidth >= 0);
	assert(m_nSplitterLineHegiht >= 0);
	return;
}
//////////////////////////////////////////////////
//ステータスバー/ツールバーの表示と非表示の切り替え
//////////////////////////////////////////////////
void CMainFrame::Showbar(const int nMenuID)
{
	UINT uState = this->GetMenu()->GetMenuState(nMenuID,MF_BYCOMMAND);
	if (uState & MF_CHECKED) {	//表示状態から非表示状態へ
		if (nMenuID == ID_SHOW_TOOLBAR)
			m_wndToolBar.ShowWindow(SW_HIDE);
		else if (nMenuID == ID_SHOW_STATUSBAR)
			m_wndStatusBar.ShowWindow(SW_HIDE);
		this->GetMenu()->CheckMenuItem(nMenuID,MF_BYCOMMAND|MF_UNCHECKED);
	} else {					//非表示状態から表示状態へ
		if (nMenuID == ID_SHOW_TOOLBAR)
			m_wndToolBar.ShowWindow(SW_SHOW);
		else if (nMenuID == ID_SHOW_STATUSBAR)
			m_wndStatusBar.ShowWindow(SW_SHOW);
		this->GetMenu()->CheckMenuItem(nMenuID,MF_BYCOMMAND|MF_CHECKED);
	}
	this->RecalcLayout();	//ウィンドウレイアウトの再計算を行う
}

//////////////////////////////////////////////////
//各種ビューの表示・非表示の切り替え
//////////////////////////////////////////////////
void CMainFrame::OnShowView(const int nMenuID,const BOOL bShow)
{
	if (bShow) {
		m_wndToolBar.PressButton(nMenuID,TRUE);
		GetMenu()->CheckMenuItem(nMenuID,MF_BYCOMMAND|MF_CHECKED);
		switch (nMenuID)
		{
		case ID_SHOW_MONITOR:
			m_wndTopButtomSplitter.ShowRow();
			break;
		case ID_SHOW_PROPERTY:
			m_wndLeftRightSplitter.ShowColumn();
			break;
		case ID_SHOW_PREVIEW:
			if (m_wndLeftRightSplitter.IsHideColumnIndex() == 1)
				m_wndLeftRightSplitter.ShowColumn();
			m_wndPreviewSplitter.ShowRow();
			if (IsShowView(ID_SHOW_VOLUME) == FALSE)
				m_wndPreviewSplitter.HideRow(0);
			break;
		case ID_SHOW_VOLUME:
			if (m_wndLeftRightSplitter.IsHideColumnIndex() == 1)
				m_wndLeftRightSplitter.ShowColumn();
			m_wndPreviewSplitter.ShowRow();
			if (IsShowView(ID_SHOW_PREVIEW) == FALSE)
				m_wndPreviewSplitter.HideRow(1);
			break;
		}
	} else {	//非表示へ
		m_wndToolBar.PressButton(nMenuID,FALSE);
		GetMenu()->CheckMenuItem(nMenuID,MF_BYCOMMAND|MF_UNCHECKED);
		switch (nMenuID)
		{
		case ID_SHOW_MONITOR:
			m_wndTopButtomSplitter.HideRow(1);
			break;
		case ID_SHOW_PROPERTY:
			if (m_wndLeftRightSplitter.IsShowColumn() == FALSE)
			{
				m_wndLeftRightSplitter.ShowColumn();	//右全体を表示
				OnShowView(ID_SHOW_VOLUME,TRUE);
				OnShowView(ID_SHOW_PREVIEW,TRUE);
				m_wndLeftRightSplitter.HideColumn(0);	//その後に左を隠す
			} else {
				m_wndLeftRightSplitter.HideColumn(0);
			}
			break;
		case ID_SHOW_VOLUME:
			if (IsShowView(ID_SHOW_PREVIEW) == FALSE) {
				if (IsShowView(ID_SHOW_PROPERTY) == FALSE) {
					OnShowView(ID_SHOW_PROPERTY,TRUE);
				}
				m_wndLeftRightSplitter.HideColumn(1);	//右全体を隠す
			} else {
				m_wndPreviewSplitter.HideRow(0);
			}
			break;
		case ID_SHOW_PREVIEW:
			if (IsShowView(ID_SHOW_VOLUME) == FALSE) {
				if (IsShowView(ID_SHOW_PROPERTY) == FALSE) {
					OnShowView(ID_SHOW_PROPERTY,TRUE);
				}
				m_wndLeftRightSplitter.HideColumn(1);	//右全体を隠す
			} else {
				m_wndPreviewSplitter.HideRow(1);
			}
			break;
		}
	}
	RecalcLayout();	//ウィンドウレイアウトの再計算を行う
	return;
}

//////////////////////////////////////////////////
//再生と停止の為のワーカスレッド
//別スレッドとして実行される唯一の関数
//////////////////////////////////////////////////
UINT CMainFrame::ThreadProcCalc(LPVOID pParam)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CMainFrame* frame = (CMainFrame*)pParam;
	frame->m_bStartStopThread = TRUE;	//スレッド開始

	BOOL bStart;
	if (pDSA->IsEncode())
		bStart = FALSE;
	else
		bStart = TRUE;

	//TRACE0("Call CMainFrame::ThreadProcCalc\r\n");
	if (bStart)	//エンコード停止から開始へ
	{
		if (!pDSA->StartEncode()) {
			AfxMessageBox(_T("エンコードの開始に失敗しました。"),MB_OK|MB_ICONINFORMATION);
			pDSA->StopEncode(TRUE);
			::SendMessage(frame->GetSafeHwnd(),WM_STOP_ENCODE,0,0);
		}
	} else		//エンコード開始から停止へ
	{
		if (!pDSA->StopEncode(FALSE)) {
			AfxMessageBox(_T("エンコードの停止に失敗しました。"),MB_OK|MB_ICONINFORMATION);
		}
		::SendMessage(frame->GetSafeHwnd(),WM_STOP_ENCODE,0,0);
	}

	frame->m_bStartStopThread = FALSE;	//スレッド停止
	AfxEndThread(0,TRUE);	//スレッドインスタンスを削除する
	return 0;
}
//////////////////////////////////////////////////
//停止のためのアイドリングループ
//////////////////////////////////////////////////
BOOL CMainFrame::StopThreadMessageLoop()
{
	if (IsStartStopThread() == FALSE)
		return FALSE;
	//m_StartStopThread->PostThreadMessage(WM_QUIT,0,0);
	while ( IsStartStopThread() == TRUE )
	{ 
		MSG msg;
		while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) 
		{ 
			if (IsStartStopThread() == FALSE)
				break;
		} 
		LONG lIdle = 0;
		while ( AfxGetApp()->OnIdle(lIdle++ ) )
			;  
	}
	return TRUE;
}

//////////////////////////////////////////////////
//エンコードの開始(ボタン)
//////////////////////////////////////////////////
void CMainFrame::OnStartEncode()
{
	//エンコードの開始
	BOOL bRet = StartEncode();
	if (bRet == FALSE)	//失敗していた場合は停止
	{
		PreStopEncode();
		CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
		pDSA->StopEncode(TRUE);
		PostStopEncode();
	}
}
//////////////////////////////////////////////////
//エンコードの開始(関数)
//ウィンドウメッセージより呼び出される
//////////////////////////////////////////////////
BOOL CMainFrame::StartEncode()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (pDSA->IsEncode() || !pDSA->IsCanPreview())
		return FALSE;	//エンコード準備ができていない

	//各種ウィンドウにエンコードの開始を通知する
	//この作業は、主にコモンコントロールのDisableを行う
	if (!m_wndToolBar.Encode(TRUE))
		return FALSE;
	if (!m_wndStatusBar.Encode(TRUE))
		return FALSE;

	//プロパティウィンドウにエンコード開始を通知する
	CPropertyFormView* pPropertyWnd = this->GetPropertyWnd();
	if (!pPropertyWnd->Encode(TRUE))
		return FALSE;
	//プレビューウィンドウにエンコード開始を通知する
	CPreviewWnd* pPreviewWnd = this->GetPreviewWnd();
	if (!pPreviewWnd->Encode(TRUE))
		return FALSE;
	//モニターウィンドウにエンコード開始を通知する
	CMonitorFormView* pMonitorWnd = this->GetMonitorWnd();
	if (!pMonitorWnd->Encode(TRUE))
		return FALSE;

	//アイコンの変更
	m_TaskTray.SetIcon(IDI_RED_ON_ICON);

	//スレッドを生成する
	if (m_StartStopThread)	//すでにスレッドが存在する
	{
		//スレッド停止まで待機
		StopThreadMessageLoop();
	}
	m_StartStopThread = ::AfxBeginThread(CMainFrame::ThreadProcCalc,(LPVOID)this);
	if (m_StartStopThread == NULL)	//スレッド生成失敗の時
		return FALSE;

	return TRUE;
}
//////////////////////////////////////////////////
//エンコードの終了(ボタン)
//////////////////////////////////////////////////
void CMainFrame::OnStopEncode()
{
	if (!PreStopEncode()) {
		AfxMessageBox(_T("エンコード停止の開始に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return;
	}
	//スレッドを生成する
	if (m_StartStopThread) {
		//スレッド停止まで待機
		StopThreadMessageLoop();
	}
	m_StartStopThread = ::AfxBeginThread(CMainFrame::ThreadProcCalc,(LPVOID)this);
	if (m_StartStopThread == NULL)
		return;
	return;
}
//////////////////////////////////////////////////
//ウィンドウメッセージ処理で来たエンコードの停止
//////////////////////////////////////////////////
LRESULT CMainFrame::OnNotifyStopEncode(WPARAM wParam,LPARAM lParam)
{
	if (!PostStopEncode())
	{
		AfxMessageBox(_T("エンコード停止の終了に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return 0;
	}
	return 0;
}
//////////////////////////////////////////////////
//エンコード停止のプレ処理
//このメッセージはウィンドウメッセージ内で処理する
//////////////////////////////////////////////////
BOOL CMainFrame::PreStopEncode()
{
	//各種ウィンドウにエンコードのプレ終了を通知する
	if (!m_wndStatusBar.PreEncode(FALSE))
		return FALSE;
	//タイマーを使用しているウィンドウについては、停止命令をここで行う
	//モニターウィンドウにエンコード終了を通知する
	CMonitorFormView* pMonitorWnd = this->GetMonitorWnd();
	if (!pMonitorWnd->Encode(FALSE))
		return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////
//エンコード停止のポスト処理
//このメッセージはウィンドウメッセージ内で処理する
//////////////////////////////////////////////////
BOOL CMainFrame::PostStopEncode()
{
	//各種ウィンドウにエンコードの終了を通知する
	if (!m_wndToolBar.Encode(FALSE))
		return FALSE;
	if (!m_wndStatusBar.Encode(FALSE))
		return FALSE;

	//プロパティウィンドウにエンコード終了を通知する
	CPropertyFormView* pPropertyWnd = this->GetPropertyWnd();
	if (!pPropertyWnd->Encode(FALSE))
		return FALSE;
	//プレビューウィンドウにエンコード終了を通知する
	CPreviewWnd* pPreviewWnd = this->GetPreviewWnd();
	if (!pPreviewWnd->Encode(FALSE))
		return FALSE;
	/*
	//モニターウィンドウにエンコード終了を通知する
	CMonitorFormView* pMonitorWnd = this->GetMonitorWnd();
	if (!pMonitorWnd->Encode(FALSE))
		return FALSE;
	*/
	//アイコンの変更
	m_TaskTray.SetIcon(IDI_BLUE_OFF_ICON);

	return TRUE;
}

//////////////////////////////////////////////////
//ON_UPDATE_COMMAND_UIは、メニューやツールバーに対して有効・無効または、チェックマークをつけたりするのに利用する
//この関数は、メニューやボタンが押されたときに反応するのではなく、(ON_COMMANDではなく、)
//メニューやツールバー上でカーソルが動いたりとか、メニューの更新が必要な時に呼ばれるイベントハンドラである。
//つまり、この関数を定義しない限り、有効・無効のチェック操作が出来ない
//////////////////////////////////////////////////
void CMainFrame::OnUpdateEncode(CCmdUI *pCmdUI)
{
	//問題はツールバーにマウスが乗っただけでも反応してしまっているところ
	if (pCmdUI->m_nID == ID_PLAY_ENCODE)
	{
		//TRACE0("ID_PLAY_ENCODE()\n");
		CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
		BOOL bEnable = TRUE;
		if (!pDSA->IsCanEncode())
			bEnable = FALSE;
		if (IsStartStopThread())
			bEnable = FALSE;
		pCmdUI->Enable(bEnable);
	} else if (pCmdUI->m_nID == ID_STOP_ENCODE)
	{
		//TRACE0("ID_STOP_ENCODE()\n");
		CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
		BOOL bEnable = TRUE;
		if (!pDSA->IsEncode())
			bEnable = FALSE;
		if (IsStartStopThread())
			bEnable = FALSE;
		pCmdUI->Enable(bEnable);
	}
}
//////////////////////////////////////////////////
//メニュー項目(新規作成と開く)の可否
//////////////////////////////////////////////////
void CMainFrame::OnUpdateProfile(CCmdUI *pCmdUI)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	BOOL bEnable = TRUE;
	if (pDSA->IsEncode())
		bEnable = FALSE;
	if (IsStartStopThread())
		bEnable = FALSE;
	pCmdUI->Enable(bEnable);
}
