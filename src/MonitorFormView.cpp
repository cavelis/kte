// MonitorFormView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MonitorFormView.h"


// CMonitorFormView

IMPLEMENT_DYNCREATE(CMonitorFormView, CFormView)

CMonitorFormView::CMonitorFormView()
	: CFormView(CMonitorFormView::IDD)
{

}

CMonitorFormView::~CMonitorFormView()
{
}

void CMonitorFormView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMonitorFormView, CFormView)
	ON_MESSAGE( WM_INITDIALOG, &CMonitorFormView::OnInitDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_MONITOR_TAB, &CMonitorFormView::OnTcnSelchangeMonitorTab)
END_MESSAGE_MAP()


// CMonitorFormView 診断

#ifdef _DEBUG
void CMonitorFormView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CMonitorFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CMonitorFormView メッセージ ハンドラ

//////////////////////////////////////////////////
//ウィンドウスタイル変更
//////////////////////////////////////////////////
BOOL CMonitorFormView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
    // TODO:
    //CSize sizeTotal;
    //sizeTotal.cx = sizeTotal.cy = 100;
    //SetScrollSizes(MM_TEXT, sizeTotal);

	return CFormView::PreCreateWindow(cs);
}

//////////////////////////////////////////////////
//WM_INITDIALOGのイベントハンドラ
//この段階でHWNDとコントロールは初期化されている(使用可能)
//だがDDX/DDVは使用不可能なのでGetDlgItemから取得すること。
//////////////////////////////////////////////////
LRESULT CMonitorFormView::OnInitDialog(WPARAM wParam,LPARAM lParam)
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	//各種タブ(子ダイアログ)を生成する
	m_StatisticsTab.Create(CMonitorStatisticsDialog::IDD,this);
	m_PullTab.Create(CMonitorPullDialog::IDD,this);
	m_EventLogTab.Create(CMonitorEventLogDialog::IDD,this);
	if (pSetting->m_bShowDebugWindow) {
		m_GraphTab.Create(CMoniterGraphDialog::IDD,this);}

	//子ダイアログを移動する
	CRect rcDialog;
	m_StatisticsTab.GetClientRect(&rcDialog);
	rcDialog.OffsetRect(0,22);
	m_StatisticsTab.MoveWindow(&rcDialog,FALSE);
	m_PullTab.MoveWindow(&rcDialog,FALSE);
	m_EventLogTab.MoveWindow(&rcDialog,FALSE);
	if (pSetting->m_bShowDebugWindow) {
		m_GraphTab.MoveWindow(&rcDialog,FALSE); }

	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_MONITOR_TAB);
	assert(tab);
	tab->ModifyStyle(0,TCS_FIXEDWIDTH,0);

	tab->InsertItem(0,_T("統計"));
	tab->InsertItem(1,_T("ブロードキャスト"));
	tab->InsertItem(2,_T("イベントログ"));
	if (pSetting->m_bShowDebugWindow) {
		tab->InsertItem(3,_T("フィルターグラフ")); }

	CSize size;
	CRect rcRect;
	tab->GetItemRect(0,&rcRect);
	size.SetSize(100,rcRect.bottom - rcRect.top);
	CSize old = tab->SetItemSize(size);
	size.SetSize(100,old.cy);
	tab->SetItemSize(size);

	//タブを表示
	tab->SetCurSel(pSetting->m_nMonitorTabIndex);
	OnTcnSelchangeMonitorTab(NULL,NULL);
	return 0;
}
//////////////////////////////////////////////////
//ウィンドウサイズ変更
//////////////////////////////////////////////////
void CMonitorFormView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	//タブコントロールを最大化する
	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_MONITOR_TAB);
	if (!tab) return;
	CRect rcClient;
	this->GetClientRect(&rcClient);
	tab->MoveWindow(rcClient,TRUE);

	//内側のダイアログタブも最大化っぽく移動する
	if (!m_StatisticsTab.GetSafeHwnd() || !m_PullTab.GetSafeHwnd())
		return;
	rcClient.OffsetRect(0,22);
	rcClient.right -= 5;
	rcClient.bottom -= 25;
	m_StatisticsTab.MoveWindow(&rcClient,TRUE);
	m_PullTab.MoveWindow(&rcClient,TRUE);
	m_EventLogTab.MoveWindow(&rcClient,TRUE);
	if (pSetting->m_bShowDebugWindow) {
		m_GraphTab.MoveWindow(&rcClient,TRUE); }

	// TODO: ここにメッセージ ハンドラ コードを追加します。
}
//////////////////////////////////////////////////
//ダイアログが終了するときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CMonitorFormView::OnDestroy()
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_MONITOR_TAB);
	if (tab)
		pSetting->m_nMonitorTabIndex = tab->GetCurSel();

	//各種タブダイアログを閉じる
	if (m_StatisticsTab.GetSafeHwnd())
	{
		m_StatisticsTab.EndDialog(0);
		m_StatisticsTab.DestroyWindow();
	}
	if (m_PullTab.GetSafeHwnd())
	{
		m_PullTab.EndDialog(0);
		m_PullTab.DestroyWindow();
	}
	if (m_EventLogTab.GetSafeHwnd())
	{
		m_EventLogTab.EndDialog(0);
		m_EventLogTab.DestroyWindow();
	}
	if (m_GraphTab.GetSafeHwnd())
	{
		m_GraphTab.EndDialog(0);
		m_GraphTab.DestroyWindow();
	}

	CFormView::OnDestroy();

	// TODO: ここにメッセージ ハンドラ コードを追加します。
}

//////////////////////////////////////////////////
//エンコードの開始と終了
//////////////////////////////////////////////////
BOOL CMonitorFormView::Encode(BOOL bStart)
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	if (!m_StatisticsTab.Encode(bStart))
		return FALSE;
	if (!m_PullTab.Encode(bStart))
		return FALSE;
	if (!m_EventLogTab.Encode(bStart))
		return FALSE;
	if (pSetting->m_bShowDebugWindow) {
		if (!m_GraphTab.Encode(bStart))
			return FALSE;
	}
	return TRUE;
}
//////////////////////////////////////////////////
//タブの変更
//////////////////////////////////////////////////
void CMonitorFormView::OnTcnSelchangeMonitorTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_MONITOR_TAB);
	if (!tab) return;

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	int nSelectSel = tab->GetCurSel();
	m_StatisticsTab.ShowWindow(nSelectSel == 0 ? SW_SHOW : SW_HIDE);
	m_PullTab.ShowWindow(nSelectSel == 1 ? SW_SHOW : SW_HIDE);
	m_EventLogTab.ShowWindow(nSelectSel == 2  ? SW_SHOW : SW_HIDE);
	if (pSetting->m_bShowDebugWindow) {
		m_GraphTab.ShowWindow(nSelectSel == 3  ? SW_SHOW : SW_HIDE);
	}
	if (pResult) *pResult = 0;
}

void CMonitorFormView::OnDraw(CDC* /*pDC*/)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
}
