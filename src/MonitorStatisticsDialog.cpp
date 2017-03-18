// MonitorStatisticsDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MonitorStatisticsDialog.h"
#include "MainFrm.h"

// CMonitorStatisticsDialog ダイアログ

IMPLEMENT_DYNAMIC(CMonitorStatisticsDialog, CDialog)

CMonitorStatisticsDialog::CMonitorStatisticsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitorStatisticsDialog::IDD, pParent)
{

}

CMonitorStatisticsDialog::~CMonitorStatisticsDialog()
{
}

void CMonitorStatisticsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMonitorStatisticsDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CMonitorStatisticsDialog メッセージ ハンドラ

////////////////////////////////////////
//ダイアログ初期化
////////////////////////////////////////
BOOL CMonitorStatisticsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ここに初期化を追加してください

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

//////////////////////////////////////////////////
//ダイアログが終了するときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CMonitorStatisticsDialog::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラ コードを追加します。
}

//////////////////////////////////////////////////
//ウィンドウサイズ変更
//////////////////////////////////////////////////
void CMonitorStatisticsDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: ここにメッセージ ハンドラ コードを追加します。
}
//////////////////////////////////////////////////
//エンコードの開始と終了
//////////////////////////////////////////////////
BOOL CMonitorStatisticsDialog::Encode(BOOL bStart)
{
	const int TIMER_ID = 102;
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	if (bStart)
	{
		//開始時間
		m_tStartTime = CTime::GetCurrentTime();
		CEdit* edit = (CEdit*)this->GetDlgItem(IDC_START_TIME_EDIT);
		assert(edit);
		edit->SetWindowText(m_tStartTime.Format(_T("%Y/%m/%d/%H:%M:%S")));
		//経過時間
		edit = (CEdit*)this->GetDlgItem(IDC_ELAPSE_TIME_EDIT);
		assert(edit);
		edit->SetWindowText(_T(""));
		//終了時間
		edit = (CEdit*)this->GetDlgItem(IDC_END_TIME_EDIT);
		assert(edit);
		edit->SetWindowText(_T(""));
		//統計タイマーの起動
		if (pSetting->m_nMonitorUpdateTimer != 0)
			SetTimer(TIMER_ID,pSetting->m_nMonitorUpdateTimer,NULL);
	} else {
		//統計タイマーの終了
		if (pSetting->m_nMonitorUpdateTimer != 0)
			KillTimer(TIMER_ID);
		//終了時の更新
		DrawStatics();

		//終了時間
		m_tEndTime = CTime::GetCurrentTime();
		CEdit* edit = (CEdit*)this->GetDlgItem(IDC_END_TIME_EDIT);
		assert(edit);
		edit->SetWindowText(m_tEndTime.Format(_T("%Y/%m/%d/%H:%M:%S")));
		//経過時間
		m_tElapseTime = m_tEndTime - m_tStartTime;
		edit = (CEdit*)this->GetDlgItem(IDC_ELAPSE_TIME_EDIT);
		assert(edit);
		CString strText;
		LONGLONG GetHour = m_tElapseTime.GetTotalHours();
		LONG GetMinite = m_tElapseTime.GetMinutes();
		LONG GetSecond = m_tElapseTime.GetSeconds();
		strText.Format(_T("%I64d:%02d:%02d"),GetHour,GetMinite,GetSecond);
		edit->SetWindowText(strText);
	}
	return TRUE;
}
//////////////////////////////////////////////////
//ウィンドウの表示・非表示に呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CMonitorStatisticsDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow) {
		CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
		if (pDSA->IsEncode()) {
			DrawStatics();
		}
	}
}

//////////////////////////////////////////////////
//WM_TIMERのイベントハンドラ
//////////////////////////////////////////////////
void CMonitorStatisticsDialog::OnTimer(UINT_PTR nIDEvent)
{
	//統計情報の表示
	if (IsWindowVisible())
		DrawStatics();

	CDialog::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////
//統計情報の表示
//////////////////////////////////////////////////
void CMonitorStatisticsDialog::DrawStatics()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	if (!pDSA->IsEncode())
		return;

	QWORD cnsCurrentTime;				//経過時間
	WM_WRITER_STATISTICS Stats;			//統計情報
	WM_WRITER_STATISTICS_EX StatsEx;	//統計情報EX
	if (pDSA->GetStatistics(cnsCurrentTime,Stats,StatsEx))
	{
		CString strText;
		//エンコード経過時間を計算
		CEdit* edit = (CEdit*)this->GetDlgItem(IDC_ELAPSE_TIME_EDIT); assert(edit);
		m_tElapseTime = CTime::GetCurrentTime() - m_tStartTime;
		LONGLONG GetHour = m_tElapseTime.GetTotalHours();
		LONG GetMinite = m_tElapseTime.GetMinutes();
		LONG GetSecond = m_tElapseTime.GetSeconds();
		strText.Format(_T("%I64d:%02d:%02d"),GetHour,GetMinite,GetSecond);
		edit->SetWindowText(strText);

		//サンプル数
		edit = (CEdit*)this->GetDlgItem(IDC_SAMPLE__EDIT); assert(edit);
		strText.Format(_T("%I64d %s"),Stats.qwSampleCount,ByteToString(Stats.qwByteCount));
		edit->SetWindowText(strText);
		//欠落したサンプル数
		edit = (CEdit*)this->GetDlgItem(IDC_SAMPLE_LOST_EDIT); assert(edit);
		strText.Format(_T("%I64d %s"),Stats.qwDroppedSampleCount,ByteToString(Stats.qwDroppedByteCount));
		edit->SetWindowText(strText);
		//欠落したサンプルの詳細
		edit = (CEdit*)this->GetDlgItem(IDC_SAMPLE_QUEUE_EDIT); assert(edit);
		strText.Format(_T("%d (%d)"),StatsEx.dwTotalSampleDropsInQueue,StatsEx.dwCurrentSampleDropRateInQueue);
		edit->SetWindowText(strText);
		edit = (CEdit*)this->GetDlgItem(IDC_SAMPLE_CODEC_EDIT); assert(edit);
		strText.Format(_T("%d (%d)"),StatsEx.dwTotalSampleDropsInCodec,StatsEx.dwCurrentSampleDropRateInCodec);
		edit->SetWindowText(strText);
		edit = (CEdit*)this->GetDlgItem(IDC_SAMPLE_MULTIPLEXER_EDIT); assert(edit);
		strText.Format(_T("%d (%d)"),StatsEx.dwTotalSampleDropsInMultiplexer,StatsEx.dwCurrentSampleDropRateInMultiplexer);
		edit->SetWindowText(strText);
		//ビットレートの詳細
		edit = (CEdit*)this->GetDlgItem(IDC_BITRATE_NOW_EDIT); assert(edit);
		strText.Format(_T("%.2f (Kbps)"),(((double)Stats.dwCurrentBitrate)/1000));
		edit->SetWindowText(strText);
		edit = (CEdit*)this->GetDlgItem(IDC_BITRATE_AVERAGE_EDIT); assert(edit);
		strText.Format(_T("%.2f (Kbps)"),(((double)Stats.dwAverageBitrate)/1000));
		edit->SetWindowText(strText);
		edit = (CEdit*)this->GetDlgItem(IDC_BITRATE_EXPECTED_EDIT); assert(edit);
		strText.Format(_T("%.2f (Kbps)"),(((double)Stats.dwExpectedBitrate)/1000));
		edit->SetWindowText(strText);
		edit = (CEdit*)this->GetDlgItem(IDC_BITRATE_OVERHEAD_EDIT); assert(edit);
		strText.Format(_T("%.2f (Kbps)"),(((double)StatsEx.dwBitratePlusOverhead)/1000));
		edit->SetWindowText(strText);
		//フレームレートの詳細
		edit = (CEdit*)this->GetDlgItem(IDC_FRAMERATE_NOW_EDIT); assert(edit);
		strText.Format(_T("%.2f (fps)"),(((double)Stats.dwCurrentSampleRate)/1000));
		edit->SetWindowText(strText);
		edit = (CEdit*)this->GetDlgItem(IDC_FRAMERATE_AVERAGE_EDIT); assert(edit);
		strText.Format(_T("%.2f (fps)"),(((double)Stats.dwAverageSampleRate)/1000));
		edit->SetWindowText(strText);
		edit = (CEdit*)this->GetDlgItem(IDC_FRAMERATE_EXPECTED_EDIT); assert(edit);
		strText.Format(_T("%.2f (fps)"),(((double)Stats.dwExpectedSampleRate)/1000));
		edit->SetWindowText(strText);

		//ステータスバーの更新
		CMainFrame* frame = (CMainFrame*)::AfxGetMainWnd(); assert(frame);
		CMyStatusBar* status = frame->GetStatusBar();
		//転送量の計算
		LONGLONG llTransformRate;		//最終的に計算した転送量
		DWORD dwTransformBitrate;
		if (StatsEx.dwBitratePlusOverhead != 0) {
			dwTransformBitrate = StatsEx.dwBitratePlusOverhead;
		} else {
			dwTransformBitrate = Stats.dwCurrentBitrate;
		}
		//接続人数を取得
		CMonitorFormView* monitor = frame->GetMonitorWnd();
		assert(monitor);
		CMonitorPullDialog* pull = monitor->GetPullTab();
		assert(pull);
		int nConnectCount = pull->GetAllConnectCount();
		llTransformRate = dwTransformBitrate * nConnectCount;	//転送量と人数を掛ける

		//Push配信量分も計算
		int nPushSinkCount = 0;
		for (int i=0;i<(int)pDSA->m_PushSinkList.size();i++)
		{
			if (pDSA->m_PushSinkList[i].nID != -1)
				continue;
			if (pDSA->m_PushSinkList[i].nRegistID != -1)
				continue;
			if (pDSA->m_PushSinkList[i].strServerName.IsEmpty())
				continue;
			nPushSinkCount++;
			nConnectCount++;
		}
		llTransformRate += (dwTransformBitrate * nPushSinkCount);

		if (llTransformRate >= (1000*1000)) {
			double dMbps = ((double)llTransformRate) / ((double)(1000*1000));
			strText.Format(_T("UP:%.2fMbps (%d)"),dMbps,nConnectCount);
		} else {
			double dKbps = ((double)llTransformRate) / ((double)(1000));
			strText.Format(_T("UP:%.2fKbps (%d)"),dKbps,nConnectCount);
		}
		status->SetTransform(strText);	//ステータスバーに通知
	}
}

//////////////////////////////////////////////////
//バイトサイズを文字列に変換するサブルーチン
//////////////////////////////////////////////////
CString CMonitorStatisticsDialog::ByteToString(QWORD qwByte)
{
	CString strText(_T(""));
	QWORD qwKillo;
	QWORD qwMega;
	QWORD qwGiga;
	qwKillo = qwByte / 1000;
	qwMega = qwKillo / 1000;
	qwGiga = qwMega / 1000;
	if (qwGiga != 0) {
		qwMega = qwMega % 1000;
		strText.Format(_T("(%I64d.%02I64dGB)"),qwGiga,qwMega/10);
	} else if (qwMega != 0) {
		qwKillo = qwKillo % 1000;
		strText.Format(_T("(%I64d.%02I64dMB)"),qwMega,qwKillo/10);
	} else if (qwKillo != 0) {
		qwByte = qwByte % 1000;
		strText.Format(_T("(%I64d.%02I64dKB)"),qwKillo,qwByte/10);
	} else {
		//qwByte = qwByte % 1000;
		strText.Format(_T("(%I64dB)"),qwByte);
	}
	return strText;
}

