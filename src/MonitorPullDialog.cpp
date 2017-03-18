// MonitorPullDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MonitorPullDialog.h"
#include "MainFrm.h"

// CMonitorPullDialog ダイアログ

IMPLEMENT_DYNAMIC(CMonitorPullDialog, CDialog)

CMonitorPullDialog::CMonitorPullDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitorPullDialog::IDD, pParent)
{
	m_nAllConnectCount = 0;
	m_ConnectCountList.clear();
	m_ConnectList.clear();
	m_AddConnectList.clear();
}

CMonitorPullDialog::~CMonitorPullDialog()
{
}

void CMonitorPullDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMonitorPullDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_MESSAGE(WM_NOTIFY_NETWORK_CLIENT_CONNECT,&CMonitorPullDialog::OnNetworkClientConnect)
	ON_MESSAGE(WM_NOTIFY_NETWORK_CLIENT_DISCONNECT,&CMonitorPullDialog::OnNetworkClientDisconnect)

END_MESSAGE_MAP()

////////////////////////////////////////
//ダイアログ初期化
////////////////////////////////////////
BOOL CMonitorPullDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	//ネットワークコールバックのウィンドウハンドルににこのウィドウを関連付けさせる
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	pDSA->SetNetworkCallbackHWnd(this->GetSafeHwnd());

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	CListCtrl* list = GetListCtrl();
	LVCOLUMN lvc;
	TCHAR       caption[][32] = {
		_T("ポート"),
		_T("IPアドレス"),
		_T("ホスト名"),
		_T("ポート先"),
		_T("接続開始時刻"),
		_T("接続経過時間")};
	const int   clmNum = sizeof caption /sizeof caption[0];
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	for (int i = 0; i < clmNum; i++)
	{
		lvc.iSubItem = i;
		lvc.pszText = caption[i];
		lvc.cx = pSetting->m_nMonitorPullColumnWidthList[i];
		list->InsertColumn(i,&lvc);
	}
	//列単位に選択
	list->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	//グリッド線を表示
	list->SetExtendedStyle((list->GetExtendedStyle()|LVS_EX_GRIDLINES));

	//ツールチップ情報はLVS_EX_INFOTIP

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

//////////////////////////////////////////////////
//ダイアログが終了するときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CMonitorPullDialog::OnDestroy()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (pDSA->IsEncode()) {
		Encode(FALSE);
	}

	//設定の保存(カラムの大きさ)
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	pSetting->m_nMonitorPullColumnWidthList.clear();
	CListCtrl* list = GetListCtrl();
	CHeaderCtrl* header = list->GetHeaderCtrl();
	assert(header);
	for (int i=0;i < header->GetItemCount();i++)
	{
		int nWidth = list->GetColumnWidth(i);
		pSetting->m_nMonitorPullColumnWidthList.push_back(nWidth);
	}


	CDialog::OnDestroy();
}
//////////////////////////////////////////////////
//ウィンドウサイズ変更
//////////////////////////////////////////////////
void CMonitorPullDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_PULL_LIST);
	if (!list) return;

	CRect rcClient;
	this->GetClientRect(&rcClient);
	CRect rcListWindow;
	list->GetWindowRect(&rcListWindow);
	this->ScreenToClient(&rcListWindow);
	CRect rcNewList;
	rcNewList.left = rcListWindow.left;
	rcNewList.top = rcListWindow.top;
	rcNewList.right = rcClient.right - rcListWindow.left;//  - rcListWindow.left;
	rcNewList.bottom = rcClient.bottom - rcListWindow.left;

	list->MoveWindow(&rcNewList,TRUE);
}
//////////////////////////////////////////////////
//WM_PAINTの描画ハンドラ
//////////////////////////////////////////////////
void CMonitorPullDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	//なぜか子ウィンドウのリストコントロールだけ再描画されないので、描画更新メッセージを与えておく
	CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_PULL_LIST);
	if (!list) return;
	list->InvalidateRect(NULL);
	list->UpdateWindow();
}


//////////////////////////////////////////////////
//エンコードの開始と終了
//////////////////////////////////////////////////
BOOL CMonitorPullDialog::Encode(BOOL bStart)
{
	const int TIMER_ID = 101;

	if (!bStart)
		KillTimer(TIMER_ID);

	//全接続情報を破棄
	m_nAllConnectCount = 0;		//接続人数
	m_ConnectCountList.clear();	//Pullごとの接続人数
	m_ConnectList.clear();		//接続情報
	m_AddConnectList.clear();	//追加された接続情報
	CListCtrl* list = GetListCtrl();
	list->DeleteAllItems();		//全てのアイテムを削除する

	if (bStart)
		SetTimer(TIMER_ID,1000,NULL);
	return TRUE;
}
//////////////////////////////////////////////////
//WM_TIMERのイベントハンドラ
//////////////////////////////////////////////////
void CMonitorPullDialog::OnTimer(UINT_PTR nIDEvent)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CListCtrl* list = GetListCtrl();

	if (!pDSA->IsEncode())
		return;


	//追加のアイテム情報について調べる
	for (int i=0;i<(int)m_AddConnectList.size();i++)
	{
		NetworkClientStruct ncs;
		DWORD dwSrcPort;
		if (!pDSA->GetPullSink(
			m_AddConnectList[i].nIndex,
			m_AddConnectList[i].dwIPAddress,
			m_AddConnectList[i].dwPort,
			ncs,
			dwSrcPort))
			continue;

		//構造体に格納しておく
		PullConnectStruct pcs;
		pcs.dwIPAddress = ncs.dwNetworkAddress;
		pcs.nIPAddress[0]  = ncs.nNetworkAddress[0];
		pcs.nIPAddress[1]  = ncs.nNetworkAddress[1];
		pcs.nIPAddress[2]  = ncs.nNetworkAddress[2];
		pcs.nIPAddress[3]  = ncs.nNetworkAddress[3];
		pcs.strIPAddress = ncs.strNetworkAddress.c_str();
		pcs.strHostName = ncs.strDNSName.c_str();
		pcs.nSrcPort =  dwSrcPort;
		pcs.nDestPort = ncs.dwPort;
		pcs.StartTime = CTime::GetCurrentTime();
		//pcs.ConnectTime = CTime::GetCurrentTime() - pcs.StartTime;
		
		//m_ConnectListの空き配列をチェックして挿入する
		BOOL bInsert = FALSE;
		int nInsertIndex = -1;	//配列の挿入場所
		for (int j=0;j<(int)m_ConnectList.size();j++)
		{
			if (m_ConnectList[j].dwIPAddress != 0)
				continue;
			if (m_ConnectList[j].nDestPort != 0)
				continue;
			if (m_ConnectList[j].nSrcPort != 0)
				continue;
			m_ConnectList[j] = pcs;
			nInsertIndex = j;
			bInsert = TRUE;
		}
		if (!bInsert) {
			m_ConnectList.push_back(pcs);	//新しく配列に追加する
			nInsertIndex = (int)m_ConnectList.size() - 1;
		}

		//リストビューに表示
		CString strNum;
		strNum.Format(_T("%d"),pcs.nSrcPort);
		int nNewIndex = list->InsertItem(list->GetItemCount(),strNum);
		strNum.Format(_T("%d.%d.%d.%d"),
			pcs.nIPAddress[0],
			pcs.nIPAddress[1],
			pcs.nIPAddress[2],
			pcs.nIPAddress[3]);
		list->SetItemText(nNewIndex,1,strNum);
		list->SetItemText(nNewIndex,2,pcs.strHostName);
		strNum.Format(_T("%d"),pcs.nDestPort);
		list->SetItemText(nNewIndex,3,strNum);
		strNum.Format(_T("%s"),pcs.StartTime.Format(_T("%H:%M:%S")));
		list->SetItemText(nNewIndex,4,strNum);
		list->SetItemData(nNewIndex,nInsertIndex);

		//接続クライアントの増加
		m_nAllConnectCount++;
		if (m_AddConnectList[i].nIndex < (int)m_ConnectCountList.size()) {
			m_ConnectCountList[m_AddConnectList[i].nIndex]++;
		} else {
			m_ConnectCountList.resize(m_AddConnectList[i].nIndex+1,0);
			m_ConnectCountList[m_AddConnectList[i].nIndex]++;
		}
		//接続人数テキストを更新する
		UpdateConnectionEdit();

		//イベントログに接続情報を送る
		//CMainFrame* frame = (CMainFrame*)::AfxGetMainWnd(); assert(frame);
		CString strAddLog;
		strAddLog.Format(_T("%d.%d.%d.%d(%s)が接続されました"),
			pcs.nIPAddress[0],
			pcs.nIPAddress[1],
			pcs.nIPAddress[2],
			pcs.nIPAddress[3],
			pcs.strHostName);
		CMonitorFormView* pWndMoniter = (CMonitorFormView*)GetParent();
		CMonitorEventLogDialog* pWndLog = pWndMoniter->GetEventLogTab();
		pWndLog->AddEventLog(strAddLog);
		//ブロードキャスト ポート: 8080 - 48 クライアント
		strAddLog.Format(_T("ブロードキャスト ポート:%d - %d クライアント"),
			pcs.nSrcPort,
			m_ConnectCountList[m_AddConnectList[i].nIndex]);
		pWndLog->AddEventLog(strAddLog);
	}

	if (m_AddConnectList.size() != 0)
		m_AddConnectList.clear();	//追記情報は処理済みのために全クリアする

	//経過時間を更新
	for (int i=0;i<(int)m_ConnectList.size();i++) {
		m_ConnectList[i].ConnectTime = CTime::GetCurrentTime() - m_ConnectList[i].StartTime;
	}

	//経過時間の更新をリストビューに反映
	/*
	CString strNum;
	int nArrayIndex;
	LONGLONG Second,Hour,Minute;
	for (int i=0;i<list->GetItemCount();i++)
	{
		nArrayIndex = (int)list->GetItemData(i);
		Second = m_ConnectList[nArrayIndex].ConnectTime.GetTotalSeconds();
		Hour = Second / (60*60);
		Minute = (Second / 60) % 60;
		Second = Second % 60;
		strNum.Format(_T("%I64d:%02I64d:%02I64d"),Hour,Minute,Second);
		list->SetItemText(i,5,strNum);
	}
	*/
	CString strNum;
	int nArrayIndex;
	for (int i=0;i<list->GetItemCount();i++)
	{
		nArrayIndex = (int)list->GetItemData(i);
		assert(nArrayIndex >=0 && nArrayIndex<(int)m_ConnectList.size());
		LONGLONG GetHour = m_ConnectList[nArrayIndex].ConnectTime.GetTotalHours();
		LONG GetMinite = m_ConnectList[nArrayIndex].ConnectTime.GetMinutes();
		LONG GetSecond = m_ConnectList[nArrayIndex].ConnectTime.GetSeconds();
		strNum.Format(_T("%I64d:%02d:%02d"),GetHour,GetMinite,GetSecond);
		list->SetItemText(i,5,strNum);
	}

	CDialog::OnTimer(nIDEvent);
}
////////////////////////////////////////
//ネットワークシンクから接続が来たときのイベントハンドラ
////////////////////////////////////////
LRESULT CMonitorPullDialog::OnNetworkClientConnect(WPARAM wParam,LPARAM lParam)
{
	//TRACE0("OnNetworkClientConnect\n");
	WM_CLIENT_PROPERTIES* pProp;
	pProp = (WM_CLIENT_PROPERTIES*)lParam;
	int nIndex;
	nIndex = (int)wParam;

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (!pDSA->IsVaildPullSink(nIndex))
		return 0;
	if (pProp->dwPort > USHRT_MAX)
		return 0;

	//追加アイテム情報に記載する
	AddConnectStruct acs;
	acs.dwIPAddress = pProp->dwIPAddress;
	acs.dwPort = pProp->dwPort;
	acs.nIndex = nIndex;
	m_AddConnectList.push_back(acs);

	return 0;
}
////////////////////////////////////////
//ネットワークシンクから切断が来たときのイベントハンドラ
////////////////////////////////////////
LRESULT CMonitorPullDialog::OnNetworkClientDisconnect(WPARAM wParam,LPARAM lParam)
{
	//TRACE0("OnNetworkClientDisconnect\n");
	WM_CLIENT_PROPERTIES* pProp;
	pProp = (WM_CLIENT_PROPERTIES*)lParam;
	int nIndex = (int)wParam;

	BOOL bDisconnect = FALSE;	//本当に切断されたかどうか
	int nSrcPort = -1;	//送信元ポートを記憶しておく

	CListCtrl* list = GetListCtrl();
	for (int i=0;i<(int)m_ConnectList.size();i++)
	{
		if (m_ConnectList[i].dwIPAddress != pProp->dwIPAddress)
			continue;
		if (m_ConnectList[i].nDestPort != pProp->dwPort)
			continue;

		//接続情報を送る
		CString strAddLog;
		strAddLog.Format(_T("%d.%d.%d.%d(%s)が切断されました"),
			m_ConnectList[i].nIPAddress[0],
			m_ConnectList[i].nIPAddress[1],
			m_ConnectList[i].nIPAddress[2],
			m_ConnectList[i].nIPAddress[3],
			m_ConnectList[i].strHostName);
		CMonitorFormView* pWndMoniter = (CMonitorFormView*)GetParent();
		CMonitorEventLogDialog* pWndLog = pWndMoniter->GetEventLogTab();
		pWndLog->AddEventLog(strAddLog);


		//リストビューからアイテムを削除する
		//int nArrayIndex;
		CString strNum,strNum2;
		for (int j=0;j<list->GetItemCount();j++)	//アイテム数のループ
		{
			strNum = list->GetItemText(j,0);	//ポート元
			if (_tstoi(strNum) != m_ConnectList[i].nSrcPort)
				continue;
			strNum = list->GetItemText(j,1);	//IPアドレス
			strNum2.Format(_T("%d.%d.%d.%d"),
				m_ConnectList[i].nIPAddress[0],
				m_ConnectList[i].nIPAddress[1],
				m_ConnectList[i].nIPAddress[2],
				m_ConnectList[i].nIPAddress[3]);
			if (strNum.Compare(strNum2) != 0)
				continue;
			strNum = list->GetItemText(j,2);	//ホスト名
			if (strNum.Compare(m_ConnectList[i].strHostName) != 0)
				continue;
			strNum = list->GetItemText(j,3);	//ポート先
			if (_tstoi(strNum) != m_ConnectList[i].nDestPort)
				continue;
			list->DeleteItem(j);
			break;
			//nArrayIndex = (int)list->GetItemData(i);
			//if (nArrayIndex == i) {
			//	list->DeleteItem(j);
			//	break;
			//}
		}
		nSrcPort = m_ConnectList[i].nSrcPort;
		//情報の消去
		m_ConnectList[i].nSrcPort = 0;
		m_ConnectList[i].strIPAddress = _T("");
		m_ConnectList[i].nIPAddress[0] = 0;
		m_ConnectList[i].nIPAddress[1] = 0;
		m_ConnectList[i].nIPAddress[2] = 0;
		m_ConnectList[i].nIPAddress[3] = 0;
		m_ConnectList[i].dwIPAddress = 0;
		m_ConnectList[i].strHostName = _T("");
		m_ConnectList[i].nDestPort = 0;
		m_ConnectList[i].nDestPort = 0;
		m_ConnectList[i].StartTime = 0;
		m_ConnectList[i].ConnectTime = 0;

		bDisconnect = TRUE;
		break;
	}

	if (bDisconnect)
	{
		//接続クライアントの減少
		m_nAllConnectCount--;
		if (nIndex < (int)m_ConnectCountList.size()) {
			m_ConnectCountList[nIndex]--;

			//イベントログを送る
			CMainFrame* frame = (CMainFrame*)::AfxGetMainWnd();
			assert(frame);
			CString strAddLog;
			//ブロードキャスト ポート: 8080 - 48 クライアント
			strAddLog.Format(_T("ブロードキャスト ポート:%d - %d クライアント"),
				nSrcPort,
				m_ConnectCountList[nIndex]);
			CMonitorFormView* pWndMoniter = (CMonitorFormView*)GetParent();
			CMonitorEventLogDialog* pWndLog = pWndMoniter->GetEventLogTab();
			pWndLog->AddEventLog(strAddLog);
		}
		//接続人数テキストを更新する
		UpdateConnectionEdit();
	}
	return 0;
}

////////////////////////////////////////
//総接続数表示の更新
////////////////////////////////////////
void CMonitorPullDialog::UpdateConnectionEdit()
{
	CString strNum;
	CWnd* pWnd = this->GetDlgItem(IDC_PULL_MAX_COUNT_STATIC);
	assert(pWnd);
	strNum.Format(_T("現在の総接続数 %d"),m_nAllConnectCount);
	if (m_ConnectCountList.size() >= 2)
	{
		CString strNum2;
		strNum += _T(" (");
		for (int i = 0; i <(int)m_ConnectCountList.size();i++)
		{
			strNum2.Format(_T("%d"),m_ConnectCountList[i]);
			strNum += strNum2;
			if (i != (int)m_ConnectCountList.size() - 1)
			{
				strNum += _T("+");
			}
		}
		strNum += _T(")");
	}
	pWnd->SetWindowText(strNum);

	return;
}
