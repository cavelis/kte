// MonitorEventLogDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MonitorEventLogDialog.h"


// CMonitorEventLogDialog ダイアログ

IMPLEMENT_DYNAMIC(CMonitorEventLogDialog, CDialog)

CMonitorEventLogDialog::CMonitorEventLogDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitorEventLogDialog::IDD, pParent)
{
	m_bEventLogFile = FALSE;
	m_strLogEvent = _T("");
}

CMonitorEventLogDialog::~CMonitorEventLogDialog()
{
}

void CMonitorEventLogDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMonitorEventLogDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_EVENT_LOG_CLEAR_BUTTON, &CMonitorEventLogDialog::OnBnClickedEventLogClearButton)
	ON_BN_CLICKED(IDC_EVENT_LOG_SAVE_BUTTON, &CMonitorEventLogDialog::OnBnClickedEventLogSaveButton)
	ON_BN_CLICKED(IDC_AUTO_SAVE_CHECK, &CMonitorEventLogDialog::OnBnClickedAutoSaveCheck)
	ON_BN_CLICKED(IDC_AUTO_SAVE_BUTTON, &CMonitorEventLogDialog::OnBnClickedAutoSaveButton)
END_MESSAGE_MAP()


////////////////////////////////////////
//ダイアログ初期化
////////////////////////////////////////
BOOL CMonitorEventLogDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	m_strLogEvent = _T("");

	CButton* button = (CButton*)this->GetDlgItem(IDC_AUTO_SAVE_CHECK);
	assert(button);
	if (pSetting->m_bEventLogAutoSave)
		button->SetCheck(BST_CHECKED);
	else
		button->SetCheck(BST_UNCHECKED);
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_AUTO_SAVE_EDIT);
	assert(edit);
	edit->SetWindowText(pSetting->m_strEventLogAutoSavePath);

	if (pSetting->m_bEventLogAutoSave && !pSetting->m_strEventLogAutoSavePath.IsEmpty()) {
		if (!OpenEventLogFile(pSetting->m_strEventLogAutoSavePath))
		{
			button->SetCheck(BST_UNCHECKED);
		}
	}

	CString strAddLog;
	strAddLog.Format(_T("%s(%d)を起動しました"),pSetting->m_strProgramName,pSetting->m_nVersion);
	AddEventLog(strAddLog);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

//////////////////////////////////////////////////
//ダイアログが終了するときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CMonitorEventLogDialog::OnDestroy()
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	CString strAddLog;
	strAddLog.Format(_T("%s(%d)を終了しました\n"),pSetting->m_strProgramName,pSetting->m_nVersion);
	AddEventLog(strAddLog);

	CButton* button = (CButton*)this->GetDlgItem(IDC_AUTO_SAVE_CHECK);
	assert(button);
	if (button->GetCheck() & BST_CHECKED) {
		pSetting->m_bEventLogAutoSave = TRUE;
	} else {
		pSetting->m_bEventLogAutoSave = FALSE;
	}
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_AUTO_SAVE_EDIT);
	assert(edit);
	edit->GetWindowText(pSetting->m_strEventLogAutoSavePath);

	CloseEventLogFile();

	CDialog::OnDestroy();
}

//////////////////////////////////////////////////
//ウィンドウサイズ変更
//////////////////////////////////////////////////
void CMonitorEventLogDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CEdit* edit = (CEdit*)GetDlgItem(IDC_EVENT_LOG_EDIT);
	if (!edit) return;

	CRect rcClient;
	this->GetClientRect(&rcClient);
	CRect rcEditWindow;
	edit->GetWindowRect(&rcEditWindow);
	this->ScreenToClient(&rcEditWindow);
	CRect rcNewList;
	rcNewList.left = rcEditWindow.left;
	rcNewList.top = rcEditWindow.top;
	rcNewList.right = rcClient.right - rcEditWindow.left;//  - rcListWindow.left;
	rcNewList.bottom = rcClient.bottom - rcEditWindow.left;

	edit->MoveWindow(&rcNewList,TRUE);

	// TODO: ここにメッセージ ハンドラ コードを追加します。
}
//////////////////////////////////////////////////
//エンコードの開始と終了
//////////////////////////////////////////////////
BOOL CMonitorEventLogDialog::Encode(BOOL bStart)
{
	if (bStart) {
		AddEventLog(_T("エンコードを開始しました"));
	} else {
		AddEventLog(_T("エンコードを終了しました"));
	}
	return TRUE;
}

//////////////////////////////////////////////////
//イベントログの追加
//////////////////////////////////////////////////
BOOL CMonitorEventLogDialog::AddEventLog(const CString &strEventLog)
{
	CTime time;
	time = CTime::GetCurrentTime();
	CString strAddString;
	strAddString.Format(_T("%s : %s"),time.Format(_T("%Y/%m/%d %H:%M:%S")),strEventLog);

	m_strLogEvent += strAddString;
	m_strLogEvent += _T("\r\n");
	CEdit* edit = (CEdit*)GetDlgItem(IDC_EVENT_LOG_EDIT);
	if (!edit) return FALSE;
	edit->SetWindowText(m_strLogEvent);

	//ログの追記書き込み
	if (m_bEventLogFile) {
		m_fEventLogFile.WriteString(strAddString);
		m_fEventLogFile.WriteString(_T("\n"));
		m_fEventLogFile.Flush();
	}

	return TRUE;
}
//////////////////////////////////////////////////
//イベントログのクリア
//////////////////////////////////////////////////
void CMonitorEventLogDialog::OnBnClickedEventLogClearButton()
{
	m_strLogEvent = _T("");
	CEdit* edit = (CEdit*)GetDlgItem(IDC_EVENT_LOG_EDIT);
	if (!edit) return;
	edit->SetWindowText(m_strLogEvent);
}
//////////////////////////////////////////////////
//ファイルに保存
//////////////////////////////////////////////////
void CMonitorEventLogDialog::OnBnClickedEventLogSaveButton()
{
   CFileDialog fd(FALSE,
		_T(""),
		_T(""),
		OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT ,
		_T("Text File (*.txt)|*.txt|")
		_T("Data File (*.dat)|*.dat|")
		_T("All files (*.*)|*.*|\0"),
		this);
	if(fd.DoModal() != IDOK)
		return;

	CStdioFile stdFile;
	if (stdFile.Open(fd.GetPathName(),CFile::modeWrite | CFile::modeCreate | CFile::typeBinary/*| CFile::typeText*/) == 0)
		return;
	stdFile.SeekToBegin();
	BYTE byBOM[2];
	byBOM[0] = 0xFF;
	byBOM[1] = 0xFE;
	stdFile.Write(byBOM,2);
	stdFile.WriteString(m_strLogEvent);
	stdFile.Close();
}
//////////////////////////////////////////////////
//自動保存にチェック
//////////////////////////////////////////////////
void CMonitorEventLogDialog::OnBnClickedAutoSaveCheck()
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_AUTO_SAVE_CHECK); assert(button);
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_AUTO_SAVE_EDIT); assert(edit);
	CString strAutoSavePath;
	edit->GetWindowText(strAutoSavePath);

	if (button->GetCheck() & BST_CHECKED) {
		//無効->有効へ
		if (strAutoSavePath.IsEmpty()) {
			button->SetCheck(BST_UNCHECKED);
			return;
		}
		if (!OpenEventLogFile(strAutoSavePath))
			button->SetCheck(BST_UNCHECKED);
	} else {
		//有効->無効へ
		CloseEventLogFile();
	}
}
//////////////////////////////////////////////////
//参照ボタン
//////////////////////////////////////////////////
void CMonitorEventLogDialog::OnBnClickedAutoSaveButton()
{
	//自動保存のパス
	CEdit* edit = (CEdit*)GetDlgItem(IDC_AUTO_SAVE_EDIT);
	assert(edit);
	CString strPath;
	edit->GetWindowText(strPath);

	CFileDialog fd(FALSE,
		_T(""),
		_T(""),
		OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT ,
		_T("Text File (*.txt)|*.txt|")
		_T("Data File (*.dat)|*.dat|")
		_T("All files (*.*)|*.*|\0"),
		this);
	//パスの分解
	TCHAR szDrive	[_MAX_DRIVE];
	TCHAR szDir		[_MAX_DIR];
	TCHAR szFileName[_MAX_FNAME];
	TCHAR szExt		[_MAX_EXT];
	_tsplitpath_s(strPath,szDrive,_MAX_DRIVE,szDir,_MAX_DIR,szFileName,_MAX_FNAME,szExt,_MAX_EXT);
	CString strFolder;
	strFolder.Format(_T("%s%s"),szDrive,szDir);
	//初期ディレクトリを設定
	fd.m_ofn.lpstrInitialDir = strFolder;

	if(fd.DoModal() != IDOK)
		return;

	edit->SetWindowText(fd.GetPathName());

	//チェックが有効でパスの変更をした場合
	CButton* button = (CButton*)this->GetDlgItem(IDC_AUTO_SAVE_CHECK); assert(button);
	if (button->GetCheck() & BST_CHECKED) {
		CloseEventLogFile();
		if (!OpenEventLogFile(fd.GetPathName()))
		{
			button->SetCheck(BST_UNCHECKED);
		}
	}
}

//////////////////////////////////////////////////
BOOL CMonitorEventLogDialog::OpenEventLogFile(CString &strFileName)
{
	if (strFileName.IsEmpty())
		return FALSE;
	if (m_bEventLogFile)	//既にファイルを開いている場合
		CloseEventLogFile();

	CFileFind find;
	if (find.FindFile(strFileName)) {
		if (m_fEventLogFile.Open(strFileName,CFile::modeWrite | CFile::modeNoTruncate | CFile::typeText | CFile::shareDenyWrite) == 0)
			return FALSE;
		m_fEventLogFile.SeekToEnd();
	} else {
		if (m_fEventLogFile.Open(strFileName,CFile::modeWrite | CFile::modeCreate | CFile::typeText | CFile::shareDenyWrite) == 0)
			return FALSE;
	}
	m_bEventLogFile = TRUE;
	return TRUE;
}

//////////////////////////////////////////////////
void CMonitorEventLogDialog::CloseEventLogFile()
{
	if (!m_bEventLogFile)
		return;
	m_fEventLogFile.Close();
	m_bEventLogFile = FALSE;
}
