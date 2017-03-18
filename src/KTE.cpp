
// KTE.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "KTE.h"
#include "MainFrm.h"
#include "DirectShowAccess.h"
#include "AboutDialog.h"
#include "PluginDialog.h"
#include "SettingDialog.h"
#include "PerformanceSettingDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////
CKTEApp theApp;		// 唯一の CKTEApp オブジェクトです。
//////////////////////////////////////////////////

//メッセージマップ
BEGIN_MESSAGE_MAP(CKTEApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CKTEApp::OnAboutDialog)
	ON_COMMAND(ID_SETTING_DIALOG, &CKTEApp::OnSettingDialog)
	ON_COMMAND(ID_SETTING_DEFAULT, &CKTEApp::OnSettingDefault)
	ON_COMMAND(ID_PERFORMANCE_SETTING_DIALOG, &CKTEApp::OnPerformanceSettingDialog)
	ON_COMMAND(ID_PROFILE_NEW, &CKTEApp::OnProfileNew)
	ON_COMMAND(ID_PROFILE_OPEN, &CKTEApp::OnProfileOpen)
	ON_COMMAND(ID_PROFILE_SAVE, &CKTEApp::OnProfileSave)
	ON_COMMAND(ID_PROFILE_SAVEAS, &CKTEApp::OnProfileSaveas)
	ON_COMMAND(ID_PLUGIN_DIALOG, &CKTEApp::OnPluginDialog)
END_MESSAGE_MAP()

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CKTEApp::CKTEApp()
	:	m_pSetting(NULL),
		m_pProfile(NULL),
		m_pDSA(NULL),
		m_hMutexApp(NULL),
		m_bEnablePriority(FALSE)
{
	// TODO: 下のアプリケーション ID 文字列を一意の ID 文字列で置換します。推奨される
	// 文字列の形式は CompanyName.ProductName.SubProduct.VersionInformation です
	SetAppID(_T("KTE.AppID.NoVersion"));

	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CKTEApp::~CKTEApp()
{
}

//////////////////////////////////////////////////
//アプリケーションエントリポイント
//////////////////////////////////////////////////
BOOL CKTEApp::InitInstance()
{
	//n番目のメモリリークを検出
	//_CrtSetBreakAlloc(n);

	setlocale(LC_CTYPE, "jpn");

	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// OLE ライブラリを初期化します。
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// RichEdit コントロールを使用するには AfxInitRichEdit2() が必要です	
	// AfxInitRichEdit2();

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。
	//SetRegistryKey(_T("アプリケーション ウィザードで生成されたローカル アプリケーション"));

	//引数をチェックして、再起動時のプロセスが終了するまで待っている
	if (!ArgCheck1())
		return FALSE;

	//OSのバージョンチェック
	if (!OSVersionCheck())
		return FALSE;

	//設定の初期化と読み込み
	m_pSetting = new CSetting();
	assert(m_pSetting);
	if (!m_pSetting->LoadSetting())
		return FALSE;
	//引数をチェックして、(*.kte)が引数に含まれているかの確認
	ArgCheck2();

	//プロファイルの初期化と読み込み
	m_pProfile = new CProfile();
	assert(m_pProfile);
	if (!m_pProfile->LoadProfile(m_pSetting->m_strProfileFullPath,TRUE))
	{
		if (m_pSetting->m_strProfileShortPath.Compare(_T("Default.kte")) != 0)
		{
			//プロファイルが存在しない
			CString strMessage;
			strMessage.Format(_T("プロファイルが存在しませんでした。\n%s\n空のプロファイルを新規作成します。"),m_pSetting->m_strProfileFullPath);
			AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		}
		m_pProfile->DefaultProfile();
	}

	//複数起動抑制用のミューテックスを作成
	if (!SetMutex(m_pSetting->m_bCreateMutex))
		return FALSE;

	//CPU設定を適用する
	if (!SetAffinityMask(TRUE))
		return FALSE;

	//DirectShow初期化
	m_pDSA = new CDirectShowAccess;
	assert(m_pDSA);
	if (!m_pDSA->Init())
		return FALSE;
	
	// メイン ウィンドウを作成するとき、このコードは新しいフレーム ウィンドウ オブジェクトを作成し、
	// それをアプリケーションのメイン ウィンドウにセットします
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;

	// フレームをリソースからロードして作成します
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	//常に手前に表示の適用
	if (!SetTopMost(m_pSetting->m_bShowTopMost))
		return FALSE;
	
	// メイン ウィンドウが初期化されたので、表示と更新を行います。
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// 接尾辞が存在する場合にのみ DragAcceptFiles を呼び出してください。
	//  SDI アプリケーションでは、ProcessShellCommand の直後にこの呼び出しが発生しなければなりません。
	return TRUE;
}

//////////////////////////////////////////////////
//アプリケーション終了ポイント
//////////////////////////////////////////////////
int CKTEApp::ExitInstance()
{
	//DirectShow終了
	if (m_pDSA)
	{
		if (m_pDSA->IsEncode() == TRUE)
			m_pDSA->StopEncode(TRUE);
		if (m_pDSA->IsRunFilter())
			m_pDSA->StopFilter();
		m_pDSA->SaveProfileSetting();		//DirectShowAccessの保持しているデータをプロファイルのセーブ
		m_pDSA->Exit();
		delete m_pDSA;
		m_pDSA = NULL;
	}

	//複数起動ミューテックス開放
	SetMutex(FALSE);

	//プロファイルの保存と終了
	if (m_pProfile)
	{
		if (m_pSetting->m_strProfileFullPath.IsEmpty() == FALSE)
			m_pProfile->SaveProfile(m_pSetting->m_strProfileFullPath);
		delete m_pProfile;
		m_pProfile = NULL;
	}

	//設定の保存と終了
	if (m_pSetting)
	{
		if (m_pSetting->m_bSaveIniFile)	//セーブを行う
			m_pSetting->SaveSetting();
		else {							//非セーブ状態(設定の初期化)
			//設定もプロファイルも削除する
			CFileFind find;
			if (find.FindFile(SETTING_INI_FILE))
				DeleteFile(SETTING_INI_FILE);
			m_pSetting->DefaultProfilePath();
			if (find.FindFile(m_pSetting->m_strProfileFullPath))
				DeleteFile(m_pSetting->m_strProfileFullPath);
		}
		delete m_pSetting;
		m_pSetting = NULL;
	}

	//COMの解放に失敗している場合、ここで固まる
	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

//////////////////////////////////////////////////
//バージョン情報を表示するダイアログを表示
//////////////////////////////////////////////////
void CKTEApp::OnAboutDialog()
{
	CAboutDialog dlg;
	dlg.DoModal();
}
//////////////////////////////////////////////////
//プラグイン設定を表示するダイアログを表示
//////////////////////////////////////////////////
void CKTEApp::OnPluginDialog()
{
	CPluginDialog dlg;
	dlg.DoModal();
}
//////////////////////////////////////////////////
//環境設定を表示するダイアログを表示
//////////////////////////////////////////////////
void CKTEApp::OnSettingDialog()
{
	CSettingDialog dlg;
	INT_PTR nRet = dlg.DoModal();
	if (nRet != IDOK) {
		return;
	}
	//複数起動禁止用のミューテックスの適用
	SetMutex(m_pSetting->m_bCreateMutex);
	//常に手前に表示の適用
	SetTopMost(m_pSetting->m_bShowTopMost);
}

//////////////////////////////////////////////////
//パフォーマンス設定を表示するダイアログを表示
//////////////////////////////////////////////////
void CKTEApp::OnPerformanceSettingDialog()
{
	//ダイアログを表示
	CPerformanceSettingDialog dlg;
	INT_PTR nRet = dlg.DoModal();
	if (nRet != IDOK)
		return;

	//アフィニティマスクの適用
	SetAffinityMask(FALSE);

	//エンコード中であった場合、プロセスプライオリティを変更
	CDirectShowAccess* pDSA = GetDSA();
	if (pDSA->IsEncode() == TRUE)
		SetPriorityClass(m_pProfile->m_bProcessPriority,m_pProfile->m_nProcessPriority);
	return;
}

//////////////////////////////////////////////////
//環境設定をデフォルトに戻す
//////////////////////////////////////////////////
void CKTEApp::OnSettingDefault()
{
	int ret_code;
	ret_code = AfxMessageBox(
		_T("設定を全て初期状態に戻しますが、よろしいですか？\n(プログラムが再起動します)"),
		MB_YESNO | MB_DEFBUTTON2 | MB_ICONINFORMATION);
	if (ret_code == IDYES)
	{
		//自身の実行ファイルをプロセスで立ち上げる
		TCHAR strPath[MAX_PATH];
		::GetModuleFileName(AfxGetInstanceHandle(),strPath,sizeof(strPath)/sizeof(TCHAR));
		TCHAR strCurDir[MAX_PATH];
		::GetCurrentDirectory(sizeof(strCurDir)/sizeof(TCHAR),strCurDir);
		DWORD dwProcessID = ::GetCurrentProcessId();
		CString strCmdLine;
		strCmdLine.Format(_T("/waitterminate %u"),dwProcessID);
		//プログラムを実行(再起動だが、引数チェック(/waitterminate)で現在のプロセスが終了するまで待つ仕様になっている)
		::ShellExecute(NULL,NULL,strPath,strCmdLine,strCurDir,SW_SHOWNORMAL);

		//プログラム終了
		m_pSetting->m_bSaveIniFile = FALSE;
		m_pMainWnd->SendMessage(WM_CLOSE);
	}
}

//////////////////////////////////////////////////
//OSバージョンチェックを行う
//////////////////////////////////////////////////
BOOL CKTEApp::OSVersionCheck()
{
	if (IsWindowsXPOrGreater() == false) {
		AfxMessageBox(_T("OSの動作要件を満たしていないので終了します\n本プログラムはWindows XP以上のバージョンで動作します\n"));
		return FALSE;
	}
	return TRUE;
}
//////////////////////////////////////////////////
//引数のチェック(再起動の際の引数のチェック)
//////////////////////////////////////////////////
BOOL CKTEApp::ArgCheck1()
{
	//再起動の際のコマンド
	BOOL bWaitTerminate = FALSE;
	DWORD dwWaitProcessID = 0;
	for (int i = 1;i<__argc;i++)
	{
		if (!(__wargv[i][0] == _T('/') || __wargv[i][0] == _T('-'))) {
			continue;
		}

		if (_tcsicmp(&__wargv[i][1],_T("waitterminate")) == 0)
		{
			if (++i >= __argc)
				continue;
			dwWaitProcessID = (DWORD)_tstoi(__wargv[i]);
			if (dwWaitProcessID != 0) {
				bWaitTerminate = TRUE;
			}
		}
	}
	if (bWaitTerminate)
	{
		HANDLE hWaitProcess = ::OpenProcess(SYNCHRONIZE, FALSE, dwWaitProcessID);
		if (hWaitProcess != NULL) {
			if (::WaitForSingleObject(hWaitProcess, 10 * 1000) == WAIT_TIMEOUT)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}
//////////////////////////////////////////////////
//引数のチェック その2 関連付けされたプロファイルの読み込み
//////////////////////////////////////////////////
BOOL CKTEApp::ArgCheck2()
{
	BOOL bFilePath = FALSE;
	if (__argc == 1)	//通常起動
		return FALSE;
	CFileFind find;
	if (find.FindFile(__wargv[1]) == TRUE)
	{
		m_pSetting->SetProfilePath(__wargv[1]);
	}
	return TRUE;
}
//////////////////////////////////////////////////
//複数起動の設定を行う
//////////////////////////////////////////////////
BOOL CKTEApp::SetMutex(BOOL bEnable)
{
#ifndef _DEBUG
	if (bEnable) {
		if (m_hMutexApp)
			return TRUE;	//既にMutexが存在している
		m_hMutexApp = CreateMutex(NULL,TRUE,m_pSetting->m_strProgramName);
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CString str;
			str.LoadString(IDS_ALREADY_EXISTS);
			AfxMessageBox(str,MB_ICONSTOP|MB_OK);
			return FALSE;
		}
	} else {
		if (m_hMutexApp)
		{
			ReleaseMutex(m_hMutexApp);
			CloseHandle(m_hMutexApp);
			m_hMutexApp = NULL;
		}
	}
#endif
	return TRUE;
}
//////////////////////////////////////////////////
//常に手前に表示
//////////////////////////////////////////////////
BOOL CKTEApp::SetTopMost(BOOL bEnable)
{
	CWnd* wnd = AfxGetApp()->GetMainWnd();
	if (!wnd)
		return FALSE;
	if (bEnable)
		wnd->SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	else
		wnd->SetWindowPos(&CWnd::wndNoTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	return TRUE;
}
//////////////////////////////////////////////////
//プロセス優先度の設定
//これがpublicになっているのは、pDSAからのアクセスがあるから
//////////////////////////////////////////////////
BOOL CKTEApp::SetPriorityClass(BOOL bEnable,DWORD dwPriorityClass)
{
	if (bEnable)
	{
		DWORD dwSetPriority = NORMAL_PRIORITY_CLASS;
		switch (dwPriorityClass)
		{
		case 0: dwSetPriority = IDLE_PRIORITY_CLASS; break;
		case 1: dwSetPriority = BELOW_NORMAL_PRIORITY_CLASS; break;
		case 2: dwSetPriority = NORMAL_PRIORITY_CLASS; break;
		case 3: dwSetPriority = ABOVE_NORMAL_PRIORITY_CLASS; break;
		case 4: dwSetPriority = HIGH_PRIORITY_CLASS; break;
		case 5: dwSetPriority = HIGH_PRIORITY_CLASS; break;
		}
		if (dwSetPriority != ::GetPriorityClass(GetCurrentProcess())) {
			::SetPriorityClass(GetCurrentProcess(),dwSetPriority);
			m_bEnablePriority = TRUE;
		}
	} else
	{
		if (m_bEnablePriority) {
			::SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
		}
		m_bEnablePriority = FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////
//プロセスのアフィニティマスクを設定する
//BOOL bInit = 初回起動時の場合はTRUE,それ以外はFALSE
//////////////////////////////////////////////////
BOOL CKTEApp::SetAffinityMask(BOOL bInit)
{
	if (bInit == TRUE && m_pProfile->m_bProcessLimitEnable == FALSE)
		return TRUE;

	//各種システム情報の取得
	SYSTEM_INFO sys_info;
	ZeroMemory(&sys_info,sizeof(SYSTEM_INFO));
	GetSystemInfo(&sys_info);
	DWORD_PTR dwProcessAffinityMask;
	DWORD_PTR dwSystemAffinityMask;
	::GetProcessAffinityMask(GetCurrentProcess(),&dwProcessAffinityMask,&dwSystemAffinityMask);

	//プロセスの制限と優先プロセッサの指定
	if (m_pProfile->m_bProcessAffinityPriority == TRUE) {	//プロセスの完全制限モード
		if (bInit == FALSE) {		//プロセスの優先度は元に戻される
			SetThreadIdealProcessor(GetCurrentThread(),(DWORD)MAXIMUM_PROCESSORS);}
		if (dwSystemAffinityMask >= m_pProfile->m_dwProcessAffinityMask &&
			m_pProfile->m_dwProcessAffinityMask != 0 && m_pProfile->m_dwProcessAffinityMask != 0xFFFFFFFF) {
				SetProcessAffinityMask(GetCurrentProcess(),m_pProfile->m_dwProcessAffinityMask);}
	} else {	//優先プロセスの設定モード
		if (bInit == FALSE) {		//アフィニティマスクは元に戻される
			SetProcessAffinityMask(GetCurrentProcess(),dwSystemAffinityMask);}
		if (sys_info.dwNumberOfProcessors < (DWORD)m_pProfile->m_nProcessIdealProcessor) {
			SetThreadIdealProcessor(GetCurrentThread(),(DWORD)m_pProfile->m_nProcessIdealProcessor);
		}
	}
	return TRUE;
}
/////////////////////////////////////////////////
//プロファイルの新規作成
//////////////////////////////////////////////////
void CKTEApp::OnProfileNew()
{
	int nRet = AfxMessageBox(_T("空のプロファイルを新規作成します。\nよろしいですか？"),MB_YESNO);
	if (nRet == IDNO)
		return;
	//プロファイルの初期化
	m_pProfile->DefaultProfile();
	//パスも初期化
	m_pSetting->DefaultProfilePath();
	//この後にすべてのダイアログに通知を行う
	CMainFrame* pFrame = (CMainFrame*)::AfxGetMainWnd();
	pFrame->ChangeProfileNotify();
	//タイトルバーも変更
	pFrame->ChangeTitleBar();

	//CPU優先度についても変更
	SetAffinityMask(FALSE);
}

//////////////////////////////////////////////////
//プロファイルを開く
//////////////////////////////////////////////////
void CKTEApp::OnProfileOpen()
{
	//ファイルを開くダイアログ
	CFileDialog fd(TRUE,
		_T(""), _T(""),
		OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST  ,
		_T("KoToEncoder Profile File (*.kte)|*.kte|")
		_T("All files (*.*)|*.*|\0"),
		::AfxGetMainWnd());

	//初期ディレクトリの設定
	CString strDefaultFolder;
	if (m_pSetting->m_strProfileFullPath.IsEmpty() == FALSE)
	{
		//パスの分解
		TCHAR szDrive	[_MAX_DRIVE];
		TCHAR szDir		[_MAX_DIR];
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExt		[_MAX_EXT];
		_tsplitpath_s(m_pSetting->m_strProfileFullPath,szDrive,_MAX_DRIVE,szDir,_MAX_DIR,szFileName,_MAX_FNAME,szExt,_MAX_EXT);
		strDefaultFolder.Format(_T("%s%s"),szDrive,szDir);
		fd.m_ofn.lpstrInitialDir = strDefaultFolder;
	}
	//ファイルを開くダイアログを起動
	if(fd.DoModal() != IDOK)
		return;
	if (fd.GetPathName().IsEmpty() == TRUE)
		return;

	//プロファイルの読み込み
	if (!m_pProfile->LoadProfile(fd.GetPathName(),FALSE)) {
		CString strMessage;
		strMessage.Format(_T("プロファイルの読み込みに失敗しました\n%s\n空のプロファイルを読み込みます"),fd.GetPathName());
		AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		m_pProfile->DefaultProfile();		//空のプロファイル
	}
	//プロファイル名の設定
	m_pSetting->SetProfilePath(fd.GetPathName());

	//この後にすべてのダイアログに通知を行う
	CMainFrame* pFrame = (CMainFrame*)::AfxGetMainWnd();
	pFrame->ChangeTitleBar();				//タイトルバー変更
	pFrame->ChangeProfileNotify();			//プロファイル変更の通知

	//CPU優先度についても変更
	SetAffinityMask(FALSE);
}

//////////////////////////////////////////////////
//プロファイルを上書き保存
//////////////////////////////////////////////////
void CKTEApp::OnProfileSave()
{
	if (m_pSetting->m_strProfileFullPath.IsEmpty() == TRUE) {
		OnProfileSaveas();	//名前をつけて保存
	} else {
		//各ウィンドウ内の情報をプロファイルに反映
		CMainFrame* pFrame = (CMainFrame*)::AfxGetMainWnd();
		pFrame->SaveProfileSetting();
		//プロファイルの上書き保存
		m_pProfile->SaveProfile(m_pSetting->m_strProfileFullPath);
	}
	return;
}
//////////////////////////////////////////////////
//プロファイルを名前をつけて保存
//////////////////////////////////////////////////
void CKTEApp::OnProfileSaveas()
{
	CFileDialog fd(FALSE,
		_T(""), _T(""),
		OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ,
		_T("KoToEncoder Profile File (*.kte)|*.kte|")
		_T("All files (*.*)|*.*|\0"),
		::AfxGetMainWnd());

	//初期ディレクトリの設定
	CString strDefaultFolder;
	if (m_pSetting->m_strProfileFullPath.IsEmpty() == FALSE)
	{
		//パスの分解
		TCHAR szDrive	[_MAX_DRIVE];
		TCHAR szDir		[_MAX_DIR];
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExt		[_MAX_EXT];
		_tsplitpath_s(m_pSetting->m_strProfileFullPath,szDrive,_MAX_DRIVE,szDir,_MAX_DIR,szFileName,_MAX_FNAME,szExt,_MAX_EXT);
		strDefaultFolder.Format(_T("%s%s"),szDrive,szDir);
		fd.m_ofn.lpstrInitialDir = strDefaultFolder;
	}

	//ファイルダイアログを起動
	if(fd.DoModal() != IDOK)
		return;
	if (fd.GetPathName().IsEmpty() == TRUE)
		return;

	//各ウィンドウ情報をプロファイルに反映
	CMainFrame* pFrame = (CMainFrame*)::AfxGetMainWnd();
	pFrame->SaveProfileSetting();
	//プロファイルの保存
	m_pProfile->SaveProfile(fd.GetPathName());
	//デフォルトプロファイルも変更
	m_pSetting->SetProfilePath(fd.GetPathName());
	//タイトルバー変更
	pFrame->ChangeTitleBar();
}

//////////////////////////////////////////////////
//[EOF]