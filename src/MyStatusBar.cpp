#include "StdAfx.h"
#include "MyStatusBar.h"

#include "KTE.h"

static UINT indicators[] =
{
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_SEPARATOR,           // ステータス ライン インジケータ
};

BEGIN_MESSAGE_MAP(CMyStatusBar, CStatusBar)
	ON_WM_TIMER()
END_MESSAGE_MAP()

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CMyStatusBar::CMyStatusBar(void)
{
	m_hQuery = NULL;
	m_hCounter = NULL;
}
//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CMyStatusBar::~CMyStatusBar(void)
{
}

//////////////////////////////////////////////////
//ウィンドウのスタイルを変更する
//////////////////////////////////////////////////
BOOL CMyStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~WS_BORDER;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.dwExStyle |= WS_EX_WINDOWEDGE;
	return CStatusBar::PreCreateWindow(cs);
}
//////////////////////////////////////////////////
//ステータスバーを作成したときに呼ばれる
//////////////////////////////////////////////////
BOOL CMyStatusBar::Create(CFrameWnd* pFrameWnd)
{
	if (!CStatusBar::Create(pFrameWnd))
		return FALSE;      // 作成できない場合

	//インジケータを設定
	CStatusBar::SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	CStatusBar::SetPaneInfo(0,0,SBPS_STRETCH | SBPS_NOBORDERS,0);	//情報ウィンドウ
	CStatusBar::SetPaneInfo(1,0,SBPS_NORMAL,25);					//停止・再生
	CStatusBar::SetPaneInfo(2,0,SBPS_NORMAL,40);					//拡大倍率
	CStatusBar::SetPaneInfo(3,0,SBPS_NORMAL,120);					//転送量
	CStatusBar::SetPaneInfo(4,0,SBPS_NORMAL,70);					//CPU使用率

	SetPlayStatus(FALSE);
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	SetTimes(pSetting->m_dPreviewChildTimes);

	//CStatusBar::SetPaneText(2,_T("UPXXX.XXKbps"));
	//CStatusBar::SetPaneText(3,_T(""));

	//高精度パフォーマンスカウンタの初期化
	if (m_hQuery) PdhCloseQuery( m_hQuery );
	if ( PdhOpenQuery(NULL,0,&m_hQuery) == ERROR_SUCCESS )
	{
		PdhAddCounter( m_hQuery, _T("\\Processor(_Total)\\% Processor Time"), 0, &m_hCounter );
		PdhCollectQueryData( m_hQuery );

		//タイマーを起動
		CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
		if (pSetting->m_nMonitorUpdateTimer != 0)
			SetTimer(0,pSetting->m_nMonitorUpdateTimer, NULL);
	}
	return TRUE;
}

//////////////////////////////////////////////////
//ステータスバーに文字列を書き込む
//////////////////////////////////////////////////
BOOL CMyStatusBar::SetString(LPCTSTR lpszText)
{
	return CStatusBar::SetPaneText(0,lpszText);
}
//////////////////////////////////////////////////
//ステータスバーの再生/停止状態を更新する
//////////////////////////////////////////////////
BOOL CMyStatusBar::SetPlayStatus(BOOL bPlay)
{
	if (bPlay)
		return CStatusBar::SetPaneText(1,_T("再生"));
	return CStatusBar::SetPaneText(1,_T("停止"));
}
//////////////////////////////////////////////////
//ステータスバーに拡大縮小倍率を書き込む
//////////////////////////////////////////////////
BOOL CMyStatusBar::SetTimes(double dTimes)
{
	if (this->GetSafeHwnd() == NULL)
		return FALSE;
	CString strNum;
	strNum.Format(_T("%.0f%%"),dTimes);
	return CStatusBar::SetPaneText(2,strNum);
}

//////////////////////////////////////////////////
//ステータスバーに転送量を書き込む
//////////////////////////////////////////////////
BOOL CMyStatusBar::SetTransform(LPCTSTR lpszText)
{
	return CStatusBar::SetPaneText(3,lpszText);
}


//////////////////////////////////////////////////
//WM_TIMERによるステータスバーの更新
//////////////////////////////////////////////////
void CMyStatusBar::OnTimer(UINT_PTR nIDEvent)
{
	ASSERT(m_hQuery);
	ASSERT(m_hCounter);
	//CPU使用率を取得する
	PDH_FMT_COUNTERVALUE fntValue;
	fntValue.doubleValue = 0.0;
	PdhCollectQueryData( m_hQuery );
	PdhGetFormattedCounterValue( m_hCounter, PDH_FMT_DOUBLE, NULL, &fntValue );
	CString cstr;
	cstr.Format(_T("CPU:%02.0f%%"),fntValue.doubleValue);
	CStatusBar::SetPaneText(4,cstr);

	CStatusBar::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////
//ウィンドウ破棄のタイミングで呼ばれる
//////////////////////////////////////////////////
BOOL CMyStatusBar::DestroyWindow()
{
	//クエリを削除
	if (m_hQuery)
		PdhCloseQuery( m_hQuery );
	m_hQuery = NULL;
	m_hCounter = NULL;

	//タイマーを削除
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	if (pSetting->m_nMonitorUpdateTimer != 0)
		KillTimer(0);

	return CStatusBar::DestroyWindow();
}

//////////////////////////////////////////////////
//エンコードの開始と終了(プレ処理)
//////////////////////////////////////////////////
BOOL CMyStatusBar::PreEncode(BOOL bStart)
{
	if (bStart == FALSE)
		SetString(_T("エンコードを停止しています..."));
	return TRUE;
}

//////////////////////////////////////////////////
//エンコードの開始と終了
//////////////////////////////////////////////////
BOOL CMyStatusBar::Encode(BOOL bStart)
{
	SetPlayStatus(bStart);
	if (bStart)
		SetString(_T("エンコードを開始しました"));
	else
		SetString(_T("エンコードを終了しました"));
	SetTransform(_T(""));	//転送量のクリア
	return TRUE;
}
