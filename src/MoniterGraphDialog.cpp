// MoniterGraphDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MoniterGraphDialog.h"
#include "afxdialogex.h"


// MoniterGraphDialog ダイアログ

IMPLEMENT_DYNAMIC(CMoniterGraphDialog, CDialog)

BEGIN_MESSAGE_MAP(CMoniterGraphDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


////////////////////////////////////////
//コンストラクタ
////////////////////////////////////////
CMoniterGraphDialog::CMoniterGraphDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMoniterGraphDialog::IDD, pParent)
{

}

////////////////////////////////////////
//デストラクタ
////////////////////////////////////////
CMoniterGraphDialog::~CMoniterGraphDialog()
{
}

void CMoniterGraphDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// MoniterGraphDialog メッセージ ハンドラー

////////////////////////////////////////
//ダイアログ初期化
////////////////////////////////////////
BOOL CMoniterGraphDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (pSetting->m_nMonitorUpdateTimer != 0)
		SetTimer(TIMER_ID,pSetting->m_nMonitorUpdateTimer,NULL);

	return TRUE;
}
//////////////////////////////////////////////////
//ダイアログが終了するときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CMoniterGraphDialog::OnDestroy()
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (pSetting->m_nMonitorUpdateTimer != 0)
		KillTimer(TIMER_ID);
	//ここにコード
	CDialog::OnDestroy();
}
//////////////////////////////////////////////////
//ウィンドウサイズ変更
//////////////////////////////////////////////////
void CMoniterGraphDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}

//////////////////////////////////////////////////
//エンコードの開始と終了
//////////////////////////////////////////////////
BOOL CMoniterGraphDialog::Encode(BOOL bStart)
{
	return TRUE;
}

//////////////////////////////////////////////////
//WM_TIMERのイベントハンドラ
//////////////////////////////////////////////////
void CMoniterGraphDialog::OnTimer(UINT_PTR nIDEvent)
{
	CString strMessage;
	CString strFourCC;
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();


	//フィルタグラフの状況
	CWnd* wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_STATUS_EDIT);assert(wnd);
	strMessage.Format(_T("フィルタ状況:%s\r\nエンコード状況:%s\r\nビデオプレビュー:%s\r\nオーディオプレビュー:%s\r\nエンコード:%s\r\n"),
		pDSA->IsRunFilter()?_T("再生"):_T("停止"),
		pDSA->IsEncode()?_T("再生"):_T("停止"),
		pDSA->IsCanPreview()?_T("可"):_T("否"),
		pDSA->IsCanAudioPreview()?_T("可"):_T("否"),
		pDSA->IsCanEncode()?_T("可"):_T("否"));
	wnd->SetWindowText(strMessage);

	//ビデオフォーマット
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_VIDEO_INPUT_EDIT);assert(wnd);
	CreateMediaTypeString(&pDSA->m_VideoFirstMediaType,strMessage);
	wnd->SetWindowText(strMessage);
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_VIDEO_OUTPUT_EDIT);assert(wnd);
	CreateMediaTypeString(&pDSA->m_VideoLastMediaType,strMessage);
	wnd->SetWindowText(strMessage);

	//オーディオフォーマット
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_AUDIO_INPUT_EDIT);assert(wnd);
	CreateMediaTypeString(&pDSA->m_AudioFirstMediaType,strMessage);
	wnd->SetWindowText(strMessage);
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_AUDIO_OUTPUT_EDIT);assert(wnd);
	CreateMediaTypeString(&pDSA->m_AudioLastMediaType,strMessage);
	wnd->SetWindowText(strMessage);

	//オーディオバッファサイズ
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_AUDIO_BUFFER_EDIT);assert(wnd);
	strMessage.Format(_T("Set.cBuffers:%d\r\nSet.cbBuffer:%d\r\nSet.cbAlign:%d\r\nSet.cbPrefix:%d\r\nGet.cBuffers:%d\r\nGet.cbBuffer:%d\r\nGet.cbAlign:%d\r\nGet.cbPrefix:%d\r\n"),
		pDSA->m_AudioSetAllocator.cBuffers,
		pDSA->m_AudioSetAllocator.cbBuffer,
		pDSA->m_AudioSetAllocator.cbAlign,
		pDSA->m_AudioSetAllocator.cbPrefix,
		pDSA->m_AudioGetAllocator.cBuffers,
		pDSA->m_AudioGetAllocator.cbBuffer,
		pDSA->m_AudioGetAllocator.cbAlign,
		pDSA->m_AudioGetAllocator.cbPrefix);
	wnd->SetWindowText(strMessage);

	//基準クロックからの基準タイムの取得
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_REFTIME_EDIT);assert(wnd);
	REFERENCE_TIME rtTime = 0;
	REFERENCE_TIME rtResolution = 0;
	if (pDSA->GetClockTime(rtTime)) {
		if (pDSA->GetClockResolution(rtResolution)) {
			strMessage.Format(_T("Time%I64d(100ns)\r\n分解能:%I64d(マイクロs)"),rtTime,(rtResolution/(REFERENCE_TIME)100));
		}
	} else {
		strMessage.Format(_T("基準タイム:無し"));
	}
	wnd->SetWindowText(strMessage);

	//ビデオの総フレーム・ドロップフレームの取得
	//(ほとんどのビデオキャプチャデバイスはこのインタフェースをサポートしていない)
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_VIDEO_DROP_EDIT);assert(wnd);
	long lNotDropped = 0;
	long lDropped = 0;
	long lAverageSize = 0;
	if (pDSA->GetVideoDropFrame(&lNotDropped,&lDropped,&lAverageSize)) {
		strMessage.Format(_T("NotDropped:%d\r\nDropped:%d\r\nAverageSize:%d"),
			lNotDropped,lDropped,lAverageSize);
	} else {
		strMessage.Format(_T("Unknown"));
	}
	wnd->SetWindowText(strMessage);

	//遅延・オフセット
	REFERENCE_TIME rtLatency,rtOffset,rtMaxOffset;		//遅延時間・オフセット
	ULONG uFlags;                   //フラグ
	rtLatency = 0;
	rtOffset = 0;
	rtMaxOffset = 0;
	uFlags = 0;
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_VIDEO_PUSHINFO_EDIT);assert(wnd);
	if (pDSA->GetPushSourceInfo(TRUE,&rtLatency,&uFlags,&rtOffset,&rtMaxOffset)) {
		strMessage.Format(_T("Latency:%I64d\r\nOffset:%I64d\r\nMaxOffset:%I64d"),
			lNotDropped,lDropped,lAverageSize);
	} else {
		strMessage.Format(_T("Unknown"));
	}
	wnd->SetWindowText(strMessage);

	rtLatency = 0;
	rtOffset = 0;
	rtMaxOffset = 0;
	uFlags = 0;
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_AUDIO_PUSHINFO_EDIT);assert(wnd);
	if (pDSA->GetPushSourceInfo(FALSE,&rtLatency,&uFlags,&rtOffset,&rtMaxOffset)) {
		strMessage.Format(_T("Latency:%I64d\r\nOffset:%I64d\r\nMaxOffset:%I64d"),
			lNotDropped,lDropped,lAverageSize);
	} else {
		strMessage.Format(_T("Unknown"));
	}
	wnd->SetWindowText(strMessage);

	//ストリーム情報
	AM_STREAM_INFO asi;
	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_VIDEO_STREAM_EDIT);assert(wnd);
	if (pDSA->GetStreamInfo(TRUE,&asi)) {
		strMessage.Format(_T("tStart:%I64d\r\ntStop:%I64d\r\nCokkie:%d-%d\r\nFlags:%d"),
			asi.tStart,asi.tStop,
			asi.dwStartCookie,asi.dwStopCookie,
			asi.dwFlags);
	} else {
		strMessage.Format(_T("Unknown"));
	}
	wnd->SetWindowText(strMessage);

	wnd = (CWnd*)GetDlgItem(IDC_MONITOR_GRAPH_AUDIO_STREAM_EDIT);assert(wnd);
	if (pDSA->GetStreamInfo(FALSE,&asi)) {
		strMessage.Format(_T("tStart:%I64d\r\ntStop:%I64d\r\nCokkie:%d-%d\r\nFlags:%d"),
			asi.tStart,asi.tStop,
			asi.dwStartCookie,asi.dwStopCookie,
			asi.dwFlags);
	} else {
		strMessage.Format(_T("Unknown"));
	}
	wnd->SetWindowText(strMessage);


	CDialog::OnTimer(nIDEvent);
}
//////////////////////////////////////////////////
//メディアタイプから情報文字列を生成する
//////////////////////////////////////////////////
void CMoniterGraphDialog::CreateMediaTypeString(AM_MEDIA_TYPE* pAmt,CString &strMessage)
{
	strMessage = _T("");
	if (pAmt->formattype == FORMAT_VideoInfo) {
		CString strFourCC;
		VIDEOINFOHEADER* VideoInfoHeader = (VIDEOINFOHEADER*)pAmt->pbFormat;
		CreateFourCCString(VideoInfoHeader->bmiHeader.biBitCount,
			VideoInfoHeader->bmiHeader.biCompression,
			pAmt->subtype.Data1,strFourCC);
		strMessage.Format(_T("幅:%d 高さ:%d\r\n色空間:%s\r\n色数:%d\r\n圧縮:%s\r\nFPS:%f\r\n"),
			VideoInfoHeader->bmiHeader.biWidth,
			VideoInfoHeader->bmiHeader.biHeight,
			strFourCC,
			VideoInfoHeader->bmiHeader.biBitCount,
			VideoInfoHeader->bmiHeader.biCompression==BI_RGB?_T("非圧縮"):_T("圧縮"),
			(double) ((double)10000000.0 / (double)VideoInfoHeader->AvgTimePerFrame));
	} else if (pAmt->formattype == FORMAT_VideoInfo2) {
		CString strFourCC;
		VIDEOINFOHEADER2* VideoInfoHeader2 = (VIDEOINFOHEADER2*)pAmt->pbFormat;
		CreateFourCCString(VideoInfoHeader2->bmiHeader.biBitCount,
			VideoInfoHeader2->bmiHeader.biCompression,
			pAmt->subtype.Data1,strFourCC);
		strMessage.Format(_T("幅:%d 高さ:%d\r\n色空間:%s\r\n色数:%d\r\n圧縮:%s\r\nFPS:%f\r\n"),
			VideoInfoHeader2->bmiHeader.biWidth,
			VideoInfoHeader2->bmiHeader.biHeight,
			strFourCC,
			VideoInfoHeader2->bmiHeader.biBitCount,
			VideoInfoHeader2->bmiHeader.biCompression==BI_RGB?_T("非圧縮"):_T("圧縮"),
			(double) ((double)10000000.0 / (double)VideoInfoHeader2->AvgTimePerFrame));
	} else if (pAmt->formattype == FORMAT_WaveFormatEx){
		WAVEFORMATEX* WaveFormatEx = (WAVEFORMATEX*)pAmt->pbFormat;
		strMessage.Format(_T("周波数:%dHz\r\n分解能%dbit\r\nチャンネル%dch\r\n"),
			WaveFormatEx->nSamplesPerSec,
			WaveFormatEx->wBitsPerSample,
			WaveFormatEx->nChannels);
	} else {
		strMessage = _T("不明な\r\nフォーマット");
	}
}

//////////////////////////////////////////////////
//FOURCC文字列生成のサブルーチン
//////////////////////////////////////////////////
void CMoniterGraphDialog::CreateFourCCString(WORD biBitCount,DWORD biCompression,DWORD data1,CString &strFourcc)
{
	strFourcc = _T("");
	if (biCompression == 0)	//非圧縮フォーマット
	{
		strFourcc.Format(_T("RGB%d"),biBitCount);
	} else {				//圧縮フォーマット(FOURCC)
		wchar_t wchData[5];
		wchData[0] = (wchar_t)(data1 & 0xFF); 
		wchData[1] = (wchar_t)((data1 >> 8) & 0xFF);
		wchData[2] = (wchar_t)((data1 >> 16) & 0xFF);
		wchData[3] = (wchar_t)((data1 >> 24) & 0xFF);
		wchData[4] = _T('\0');
		strFourcc = wchData;
	}
	return;
}