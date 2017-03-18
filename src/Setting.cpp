#include "StdAfx.h"
#include "Setting.h"

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CSetting::CSetting(void)
{
	DefaultSetting();
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CSetting::~CSetting(void)
{
}

//////////////////////////////////////////////////
//設定項目に初期値に設定
//////////////////////////////////////////////////
void CSetting::DefaultSetting()
{
	//全般
	m_bEnableIniFile = TRUE;
	m_bSaveIniFile = TRUE;
	m_strProgramName = _T("KoToEncoder");
	//m_strProgramName = _T("KoToEncoder[TEST]");
	m_nVersion = 20170319;

	//初期プロファイルのパスを設定
	DefaultProfilePath();

	//画面設計
	m_rcMainWindow.SetRectEmpty();
	m_nMainSplitterWidth[0] = 0;
	m_nMainSplitterWidth[1] = 0;
	m_nMainSplitterHeight[0] = 0;
	m_nMainSplitterHeight[1] = 0;
	m_nMainSplitterPreview[0] = 0;
	m_nMainSplitterPreview[1] = 0;
	m_bShowToolBar = TRUE;
	m_bShowStatusBar = TRUE;
	m_bShowPropertyWindow = TRUE;
	m_bShowMonitorWindow = TRUE;
	m_bShowPreviewWindow = TRUE;
	m_bShowVolumeWindow = TRUE;
	m_nPropertyTabIndex = 0;
	m_nMonitorTabIndex = 0;
#ifdef _DEBUG
	m_bShowDebugWindow = TRUE;
#else
	m_bShowDebugWindow = FALSE;
#endif
	//環境設定
	m_bMiniTaskBar = FALSE;
	m_bCreateMutex = TRUE;
	m_bShowTopMost = FALSE;
	m_strVideoPlayerPath = _T("");
	m_strGetGlobalIPURL = _T("");
	m_bGetGlobalIPCheck = TRUE;
	m_strGlobalIPAddress = _T("");
	m_strGlobalHostName = _T("");
	m_bEnumAllPlugin = FALSE;

	//プレビューウィンドウの設定
	m_dPreviewChildTimes = 100;
	m_bFixByWindow = FALSE;
	m_bFixByClient = FALSE;
	m_bPreviewShow = TRUE;
	m_bPreviewForceOff = FALSE;
	m_nPreviewVMR = 0;
	m_bPreviewVMROverlay = FALSE;

	m_bEnableAudioPreview = TRUE;

	//モニターウィンドウ(統計タブ)の設定
	m_nMonitorUpdateTimer = 1000;
	//モニターウィンドウ(ブロードキャストタブ)の設定
	m_nMonitorPullColumnWidthList.clear();
	//イベントログの設定
	m_bEventLogAutoSave = FALSE;
	m_strEventLogAutoSavePath = _T("");
}

//////////////////////////////////////////////////
//設定データの読み込み
//////////////////////////////////////////////////
BOOL CSetting::LoadSetting()
{
	CIniFile Ini;
	if (!Ini.Open(SETTING_INI_FILE))
	{
		AfxMessageBox(_T("Iniファイルを開くのに失敗しました"),MB_OK|MB_ICONINFORMATION);
		Ini.Close();
		return FALSE;
	}

	//INIファイルの存在の確認
	m_bEnableIniFile = Ini.IsFind();

	//プロファイル
	m_strProfileShortPath = Ini.GetProfileString(_T("Profile"),_T("CurrentShortPath"),_T("Default.kte"));
	m_strProfileFullPath = Ini.GetProfileString(_T("Profile"),_T("CurrentLongPath"),_T(""));
	if (m_strProfileFullPath.IsEmpty() == TRUE || m_strProfileShortPath.IsEmpty() == TRUE) {
		DefaultProfilePath();	//初期プロファイルに戻す
	}

	//画面設計
	m_rcMainWindow.left = Ini.GetProfileInt(_T("MainWindow"),_T("Left"),0);
	m_rcMainWindow.top  = Ini.GetProfileInt(_T("MainWindow"),_T("Top"),0);
	m_rcMainWindow.right = Ini.GetProfileInt(_T("MainWindow"),_T("Right"),0);
	m_rcMainWindow.bottom = Ini.GetProfileInt(_T("MainWindow"),_T("Bottom"),0);

	m_nMainSplitterWidth[0] = Ini.GetProfileInt(_T("MainWindow"),_T("SplitterWidth0"),0);
	m_nMainSplitterWidth[1] = Ini.GetProfileInt(_T("MainWindow"),_T("SplitterWidth1"),0);
	m_nMainSplitterHeight[0] = Ini.GetProfileInt(_T("MainWindow"),_T("SplitterHeight0"),0);
	m_nMainSplitterHeight[1] = Ini.GetProfileInt(_T("MainWindow"),_T("SplitterHeight1"),0);
	m_nMainSplitterPreview[0] = Ini.GetProfileInt(_T("MainWindow"),_T("SplitterPreview0"),0);
	m_nMainSplitterPreview[1] = Ini.GetProfileInt(_T("MainWindow"),_T("SplitterPreview1"),0);

	m_bShowToolBar = Ini.GetProfileBOOL(_T("Show"),_T("ToolBar"),TRUE);
	m_bShowStatusBar = Ini.GetProfileBOOL(_T("Show"),_T("StatusBar"),TRUE);
	m_bShowPropertyWindow = Ini.GetProfileBOOL(_T("Show"),_T("PropertyWindow"),TRUE);
	m_bShowMonitorWindow = Ini.GetProfileBOOL(_T("Show"),_T("MonitorWindow"),TRUE);
	m_bShowPreviewWindow = Ini.GetProfileBOOL(_T("Show"),_T("PreviewWindow"),TRUE);
	m_bShowVolumeWindow = Ini.GetProfileBOOL(_T("Show"),_T("VolumeWindow"),TRUE);

	m_nPropertyTabIndex = Ini.GetProfileInt(_T("Show"),_T("PropertyTabIndex"),0);
	m_nMonitorTabIndex = Ini.GetProfileInt(_T("Show"),_T("MonitorTabIndex"),0);
#ifdef _DEBUG
	m_bShowDebugWindow = Ini.GetProfileBOOL(_T("Show"),_T("DebugWindow"),TRUE);
#else
	m_bShowDebugWindow = Ini.GetProfileBOOL(_T("Show"),_T("DebugWindow"),FALSE);
#endif

	//環境設定
	m_bMiniTaskBar = Ini.GetProfileBOOL(_T("Environment"),_T("MiniTaskBar"),FALSE);
	m_bCreateMutex = Ini.GetProfileBOOL(_T("Environment"),_T("CreateMutex"),TRUE);
	m_bShowTopMost = Ini.GetProfileBOOL(_T("Environment"),_T("ShowTopMost"),FALSE);
	m_strVideoPlayerPath = Ini.GetProfileString(_T("Environment"),_T("VideoPlayerPath"),_T(""));
	if (!m_bEnableIniFile)	//初期メディアプレイヤーのパスを設定する
	{
		TCHAR SpecialPath[MAX_PATH];
		SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_PROGRAM_FILES,FALSE);
		m_strVideoPlayerPath.Format(_T("%s\\Windows Media Player\\wmplayer.exe"),SpecialPath);
		CFileFind file;
		if (!file.FindFile(m_strVideoPlayerPath))
		{
			SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_PROGRAM_FILESX86,FALSE);
			m_strVideoPlayerPath.Format(_T("%s\\Windows Media Player\\wmplayer.exe"),SpecialPath);
			if (!file.FindFile(m_strVideoPlayerPath))
				m_strVideoPlayerPath = _T("");
		}
	}
	m_strGetGlobalIPURL = Ini.GetProfileString(_T("Environment"),_T("GlobalIPURL"),_T("http://kte.cavelis.net/cgi-bin/global.php"));
	m_bGetGlobalIPCheck = Ini.GetProfileBOOL(_T("Environment"),_T("GlobalIPCheck"),TRUE);
	m_strGlobalIPAddress = _T("");
	m_strGlobalHostName = _T("");
	m_bEnumAllPlugin = Ini.GetProfileBOOL(_T("Environment"),_T("EnumAllPlugin"),FALSE);

	//プレビューウィンドウの設定
	m_dPreviewChildTimes = Ini.GetProfileDouble(_T("Preview"),_T("ChildTimes"),100.0);
	m_bFixByWindow = Ini.GetProfileBOOL(_T("Preview"),_T("FixByWindow"),FALSE);
	m_bFixByClient = Ini.GetProfileBOOL(_T("Preview"),_T("FixByClient"),FALSE);
	if (m_bFixByWindow && m_bFixByClient) {
		m_dPreviewChildTimes = 100.0;
		m_bFixByWindow = FALSE;
		m_bFixByClient = FALSE;
	}
	m_bPreviewShow = Ini.GetProfileBOOL(_T("Preview"),_T("ChildShow"),TRUE);
	m_bPreviewForceOff = Ini.GetProfileBOOL(_T("Preview"),_T("ForceOff"),FALSE);
	m_nPreviewVMR = Ini.GetProfileInt(_T("Preview"),_T("VMR"),0);
	m_bPreviewVMROverlay = Ini.GetProfileBOOL(_T("Preview"),_T("Overlay"),FALSE);

	m_bEnableAudioPreview = Ini.GetProfileBOOL(_T("AudioPreview"),_T("Enable"),TRUE);

	//モニターウィンドウ(統計タブ)の設定
	m_nMonitorUpdateTimer = Ini.GetProfileInt(_T("MonitorWindow"),_T("UpdateTimer"),1000);
	//モニターウィンドウ(ブロードキャストタブ)の設定
	CString strEntry;
	const int nMonitorPullColumnCount = 6;
	for (int i=0;i<nMonitorPullColumnCount;i++)
	{
		strEntry.Format(_T("PullColumnWidth%d"),i+1);
		int nWidth = Ini.GetProfileInt(_T("MonitorWindow"),strEntry,100);
		m_nMonitorPullColumnWidthList.push_back(nWidth);
	}
	//イベントログの設定
	m_bEventLogAutoSave = Ini.GetProfileBOOL(_T("MonitorWindow"),_T("EventAutoSave"),FALSE);
	m_strEventLogAutoSavePath = Ini.GetProfileString(_T("MonitorWindow"),_T("EventAutoSavePath"),_T(""));

	//INI編集終了
	Ini.Close();
	return TRUE;
}

//////////////////////////////////////////////////
//設定データの書き込み
//////////////////////////////////////////////////
BOOL CSetting::SaveSetting()
{
	CIniFile Ini;
	if (!Ini.Open(SETTING_INI_FILE))
	{
		AfxMessageBox(_T("Iniファイルを開くのに失敗しました"),MB_OK|MB_ICONINFORMATION);
		Ini.Close();
		return FALSE;
	}

	//プロファイル
	Ini.WriteProfileString(_T("Profile"),_T("CurrentShortPath"),m_strProfileShortPath);
	Ini.WriteProfileString(_T("Profile"),_T("CurrentLongPath"),m_strProfileFullPath);

	//画面設計
	Ini.WriteProfileInt(_T("MainWindow"),_T("Left"),m_rcMainWindow.left);
	Ini.WriteProfileInt(_T("MainWindow"),_T("Top"),m_rcMainWindow.top);
	Ini.WriteProfileInt(_T("MainWindow"),_T("Right"),m_rcMainWindow.right);
	Ini.WriteProfileInt(_T("MainWindow"),_T("Bottom"),m_rcMainWindow.bottom);

	Ini.WriteProfileInt(_T("MainWindow"),_T("SplitterWidth0"),m_nMainSplitterWidth[0]);
	Ini.WriteProfileInt(_T("MainWindow"),_T("SplitterWidth1"),m_nMainSplitterWidth[1]);
	Ini.WriteProfileInt(_T("MainWindow"),_T("SplitterHeight0"),m_nMainSplitterHeight[0]);
	Ini.WriteProfileInt(_T("MainWindow"),_T("SplitterHeight1"),m_nMainSplitterHeight[1]);
	Ini.WriteProfileInt(_T("MainWindow"),_T("SplitterPreview0"),m_nMainSplitterPreview[0]);
	Ini.WriteProfileInt(_T("MainWindow"),_T("SplitterPreview1"),m_nMainSplitterPreview[1]);

	Ini.WriteProfileBOOL(_T("Show"),_T("ToolBar"),m_bShowToolBar);
	Ini.WriteProfileBOOL(_T("Show"),_T("StatusBar"),m_bShowStatusBar);
	Ini.WriteProfileBOOL(_T("Show"),_T("PropertyWindow"),m_bShowPropertyWindow);
	Ini.WriteProfileBOOL(_T("Show"),_T("MonitorWindow"),m_bShowMonitorWindow);
	Ini.WriteProfileBOOL(_T("Show"),_T("PreviewWindow"),m_bShowPreviewWindow);
	Ini.WriteProfileBOOL(_T("Show"),_T("VolumeWindow"),m_bShowVolumeWindow);
	Ini.WriteProfileInt(_T("Show"),_T("PropertyTabIndex"),m_nPropertyTabIndex);
	Ini.WriteProfileInt(_T("Show"),_T("MonitorTabIndex"),m_nMonitorTabIndex);
	Ini.WriteProfileBOOL(_T("Show"),_T("DebugWindow"),m_bShowDebugWindow);

	//環境設定
	Ini.WriteProfileBOOL(_T("Environment"),_T("MiniTaskBar"),m_bMiniTaskBar);
	Ini.WriteProfileBOOL(_T("Environment"),_T("CreateMutex"),m_bCreateMutex);
	Ini.WriteProfileBOOL(_T("Environment"),_T("ShowTopMost"),m_bShowTopMost);
	Ini.WriteProfileString(_T("Environment"),_T("VideoPlayerPath"),m_strVideoPlayerPath);
	Ini.WriteProfileString(_T("Environment"),_T("GlobalIPURL"),m_strGetGlobalIPURL);
	Ini.WriteProfileBOOL(_T("Environment"),_T("GlobalIPCheck"),m_bGetGlobalIPCheck);
	Ini.WriteProfileBOOL(_T("Environment"),_T("EnumAllPlugin"),m_bEnumAllPlugin);

	//プレビューウィンドウの設定
	Ini.WriteProfileDouble(_T("Preview"),_T("ChildTimes"),(float)m_dPreviewChildTimes);
	Ini.WriteProfileBOOL(_T("Preview"),_T("FixByWindow"),m_bFixByWindow);
	Ini.WriteProfileBOOL(_T("Preview"),_T("FixByClient"),m_bFixByClient);
	Ini.WriteProfileBOOL(_T("Preview"),_T("ChildShow"),m_bPreviewShow);
	Ini.WriteProfileBOOL(_T("Preview"),_T("ForceOff"),m_bPreviewForceOff);
	Ini.WriteProfileInt(_T("Preview"),_T("VMR"),m_nPreviewVMR);
	Ini.WriteProfileBOOL(_T("Preview"),_T("Overlay"),m_bPreviewVMROverlay);

	Ini.WriteProfileBOOL(_T("AudioPreview"),_T("Enable"),m_bEnableAudioPreview);

	//モニターウィンドウ(統計タブ)の設定
	Ini.WriteProfileInt(_T("MonitorWindow"),_T("UpdateTimer"),m_nMonitorUpdateTimer);
	//モニターウィンドウ(ブロードキャストタブ)の設定
	CString strEntry;
	for (int i=0;i<(int)m_nMonitorPullColumnWidthList.size();i++)
	{
		strEntry.Format(_T("PullColumnWidth%d"),i+1);
		Ini.WriteProfileInt(_T("MonitorWindow"),strEntry,m_nMonitorPullColumnWidthList[i]);
	}
	//イベントログの設定
	Ini.WriteProfileBOOL(_T("MonitorWindow"),_T("EventAutoSave"),m_bEventLogAutoSave);
	Ini.WriteProfileString(_T("MonitorWindow"),_T("EventAutoSavePath"),m_strEventLogAutoSavePath);

	//INI編集終了
	Ini.Close();
	return TRUE;
}

//////////////////////////////////////////////////
//プロファイルのパスを規定のパス("Default.kte")にする
//////////////////////////////////////////////////
void CSetting::DefaultProfilePath()
{
	m_strProfileShortPath = _T("Default.kte");
	TCHAR szNewPath	[_MAX_PATH];
	TCHAR szPath	[_MAX_PATH];
	TCHAR szDrive	[_MAX_DRIVE];
	TCHAR szDir		[_MAX_DIR];
	TCHAR szFileName[_MAX_FNAME];
	TCHAR szExt		[_MAX_EXT];
	TCHAR szShortPath [_MAX_PATH];

	::GetModuleFileName(NULL,szPath,_MAX_PATH);
	_tsplitpath_s(szPath,szDrive,_MAX_DRIVE,szDir,_MAX_DIR,szFileName,_MAX_FNAME,szExt,_MAX_EXT);
	_tcscpy_s(szShortPath, m_strProfileShortPath);
	_stprintf_s(szNewPath,_MAX_PATH,_T("%s%s%s"),szDrive,szDir, szShortPath);
	m_strProfileFullPath = szNewPath;
	return;
}

//////////////////////////////////////////////////
//プロファイルのパスを変更する
//LPCTSTR lpszFileName フルパス
//////////////////////////////////////////////////
BOOL CSetting::SetProfilePath(LPCTSTR lpszFileName)
{
	if (lpszFileName == NULL)
	{
		m_strProfileShortPath = _T("");
		m_strProfileFullPath = _T("");
		return FALSE;
	}
	TCHAR szDrive	[_MAX_DRIVE];
	TCHAR szDir		[_MAX_DIR];
	TCHAR szFileName[_MAX_FNAME];
	TCHAR szExt		[_MAX_EXT];
	_tsplitpath_s(lpszFileName,szDrive,_MAX_DRIVE,szDir,_MAX_DIR,szFileName,_MAX_FNAME,szExt,_MAX_EXT);
	m_strProfileShortPath.Format(_T("%s%s"),szFileName,szExt);
	m_strProfileFullPath = lpszFileName;
	return TRUE;
}
