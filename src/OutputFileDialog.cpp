// OutputFileDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "OutputFileDialog.h"
#include "OutputPropertyDialog.h"

// COutputFileDialog ダイアログ

IMPLEMENT_DYNAMIC(COutputFileDialog, CDialog)

BEGIN_MESSAGE_MAP(COutputFileDialog, CDialog)
	ON_BN_CLICKED(IDC_OUTPUT_FILE_BUTTON, &COutputFileDialog::OnBnClickedOutputFileButton)
	ON_BN_CLICKED(IDC_OUTPUT_FILE_ENABLE_CHECK, &COutputFileDialog::OnBnClickedOutputFileEnableCheck)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OUTPUT_APPLY_BUTTON, &COutputFileDialog::OnBnClickedOutputApplyButton)
	ON_BN_CLICKED(IDC_OUTPUT_OPEN_BUTTON, &COutputFileDialog::OnBnClickedOutputOpenButton)
END_MESSAGE_MAP()

COutputFileDialog::COutputFileDialog(CWnd* pParent /*=NULL*/)
	: COutputBaseDialog(COutputFileDialog::IDD, pParent)
{
	m_nRegisterID = -1;
	m_strRegisterFileName = _T("");
	m_RegisterTime = 0;
}

COutputFileDialog::~COutputFileDialog()
{
}

void COutputFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// COutputFileDialog メッセージ ハンドラ

//////////////////////////////////////////////////
//出力名の取得
//////////////////////////////////////////////////
BOOL COutputFileDialog::GetOutputName(CString &strFileName)
{
	TRACE0("COutputFileDialog::GetOutputName");
	/*
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FILE_PATH_EDIT);
	if (edit == NULL)
		return FALSE;
	edit->GetWindowText(strFileName);
	*/
	return TRUE;
}
//////////////////////////////////////////////////
//出力名の設定
//////////////////////////////////////////////////
BOOL COutputFileDialog::SetOutputName(CString strFileName)
{
	TRACE0("COutputFileDialog::SetOutputName");
	/*
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FILE_PATH_EDIT);
	if (edit == NULL)
		return FALSE;
	edit->SetWindowText(strFileName);
	*/
	return TRUE;
}
//////////////////////////////////////////////////
//出力設定名の取得と変更
//////////////////////////////////////////////////
BOOL COutputFileDialog::GetOutputFileName(CString &strFileName)
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FILE_PATH_EDIT);
	if (edit == NULL) return FALSE;
	edit->GetWindowText(strFileName);
	return TRUE;
}
BOOL COutputFileDialog::GetOutputFolderName(CString &strFolderName)
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FOLDER_PATH_EDIT);
	if (edit == NULL) return FALSE;
	edit->GetWindowText(strFolderName);
	return TRUE;
}
BOOL COutputFileDialog::SetOutputFileName(CString strFileName)
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FILE_PATH_EDIT);
	if (edit == NULL) return FALSE;
	edit->SetWindowText(strFileName);
	return TRUE;
}
BOOL COutputFileDialog::SetOutputFolderName(CString strFolderName)
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FOLDER_PATH_EDIT);
	if (edit == NULL) return FALSE;
	edit->SetWindowText(strFolderName);
	return TRUE;
}

//////////////////////////////////////////////////
//タブ名の取得
//////////////////////////////////////////////////
BOOL COutputFileDialog::GetTabName(CString &strTabName)
{
	CWnd* wnd = (CWnd*)this->GetDlgItem(IDC_OUTPUT_FILE_GROUP);
	if (wnd == NULL)
		return FALSE;
	wnd->GetWindowText(strTabName);
	return TRUE;
}
//////////////////////////////////////////////////
//タブ名の設定
//////////////////////////////////////////////////
BOOL COutputFileDialog::SetTabName(CString strTabName)
{
	CWnd* wnd = (CWnd*)this->GetDlgItem(IDC_OUTPUT_FILE_GROUP);
	if (wnd == NULL)
		return FALSE;
	wnd->SetWindowText(strTabName);
	return TRUE;
}
//////////////////////////////////////////////////
//有効チェックの取得
//////////////////////////////////////////////////
BOOL COutputFileDialog::GetEnableCheck()
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_FILE_ENABLE_CHECK); 
	assert(button);
	if (button->GetCheck() & BST_CHECKED)
		return TRUE;
	return FALSE;
}
//////////////////////////////////////////////////
//有効チェックの設定
//////////////////////////////////////////////////
BOOL COutputFileDialog::SetEnableCheck(BOOL bCheck)
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_FILE_ENABLE_CHECK); 
	assert(button);
	if (bCheck)
		button->SetCheck(BST_CHECKED);
	else
		button->SetCheck(BST_UNCHECKED);
	return TRUE;
}
//////////////////////////////////////////////////
//有効チェックボックスのクリック
//////////////////////////////////////////////////
void COutputFileDialog::OnBnClickedOutputFileEnableCheck()
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_FILE_ENABLE_CHECK); 
	assert(button);
	//親ウィンドウに通知する
	CWnd* pWnd = this->GetParent();
	pWnd->SendMessage(NOTIFY_ENABLE_CHECK,(WPARAM)this,button->GetCheck());
}
//////////////////////////////////////////////////
//適用ボタンの表示と非表示
//////////////////////////////////////////////////
BOOL COutputFileDialog::ShowApplyButton(BOOL bShow)
{
	CWnd* button = (CWnd*)GetDlgItem(IDC_OUTPUT_APPLY_BUTTON);
	CWnd* staticlabel = (CWnd*)GetDlgItem(IDC_OUTPUT_APPLY_STATIC);
	assert(button);
	assert(staticlabel);
	button->ShowWindow(bShow ? SW_SHOW:SW_HIDE);
	staticlabel->ShowWindow(bShow ? SW_SHOW:SW_HIDE);
	return TRUE;
}
//////////////////////////////////////////////////
//参照ボタン
//////////////////////////////////////////////////
void COutputFileDialog::OnBnClickedOutputFileButton()
{
	LPMALLOC pMalloc;
	if( ::SHGetMalloc( &pMalloc ) != NOERROR )
		return;

	//ルートディレクトリを取得
	LPITEMIDLIST pRoot = NULL;
	if (::SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pRoot) != TRUE)
		::SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&pRoot);

	//初期ディレクトリを取得
	TCHAR SpecialPath[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_MYVIDEO,FALSE) != TRUE) {
		SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_DESKTOP,FALSE);
	}

	//フォルダを開くダイアログを表示
	BROWSEINFO bInfo;
	::ZeroMemory(&bInfo,sizeof(BROWSEINFO));
	TCHAR path[MAX_PATH];
	LPITEMIDLIST pIDList = NULL;
	//フォルダ選択ダイアログ　準備
	bInfo.hwndOwner             = this->GetSafeHwnd();		//ウインドウのハンドル
	bInfo.pidlRoot              = pRoot;						//デフォルトパス（NULL：デスクトップ)
	bInfo.pszDisplayName        = path;				//選択されたパス受け取り用バッファ
	bInfo.lpszTitle             = _T("出力フォルダの選択");		//ダイアログに表示するタイトル 
	bInfo.ulFlags               = BIF_RETURNONLYFSDIRS;
	bInfo.lpfn                  = BrowseCallbackProc;
	bInfo.lParam                = (LPARAM)SpecialPath;

	//フォルダ選択ダイアログ表示
	pIDList = ::SHBrowseForFolder(&bInfo);
	if (pIDList != NULL)
	{
		if(::SHGetPathFromIDList(pIDList, path)) {						//パス正常時
			CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FOLDER_PATH_EDIT);
			assert(edit);
			edit->SetWindowText(path);
		}
	}
	if( pRoot != NULL )
		pMalloc->Free( pRoot );
	if( pIDList != NULL )
		pMalloc->Free( pIDList );
	pMalloc->Release();
	return;
}
//////////////////////////////////////////////////
//開くボタン
//////////////////////////////////////////////////
void COutputFileDialog::OnBnClickedOutputOpenButton()
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_FOLDER_PATH_EDIT);
	CString strPath;
	edit->GetWindowText(strPath);
	if (strPath.IsEmpty())
		return;
	ShellExecute(HWND_DESKTOP,_T("open"),_T("explorer.exe"),strPath,NULL,SW_SHOW);
}

////////////////////////////////////////////////////
//フォルダを開くダイアログのコールバック
////////////////////////////////////////////////////
int CALLBACK COutputFileDialog::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  if (uMsg == BFFM_INITIALIZED)
    ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
  return 0;
}

//////////////////////////////////////////////////
//出力ファイル名の作成
//strFolderPath フォルダ名
//strFilePath ファイル名
//Time 作成する時間
//strMakePath 出力ファイル名
//////////////////////////////////////////////////
BOOL COutputFileDialog::MakeFilePath(CString strFolderPath,CString strFilePath,CTime &Time,CString &strMakePath)
{
	//フォルダ名が空ならデスクトップを指定する
	if (strFolderPath.IsEmpty()) {
		//初期ディレクトリを取得
		TCHAR SpecialPath[MAX_PATH];
		SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_DESKTOP,FALSE);
		strFolderPath = SpecialPath;
	}

	//フォルダパスの設定
	strMakePath = strFolderPath;
	if ((strFolderPath.Right(1)).Compare(_T("\\")) != 0)
		strMakePath += _T("\\");

	//FilePath文字列の置換処理
	LPCTSTR pPtr = strFilePath;
	CString strAppend;
	CString strFormat;
	while (*pPtr != _T('\0'))
	{
		if (*pPtr == _T('%'))	//変換文字
		{
			strFormat = _T("%");
			pPtr = _tcsinc(pPtr);
			if (*pPtr == _T('\0'))
				break;
			if (*pPtr == _T('#')) {
				strFormat.Append(pPtr,1);
				pPtr = _tcsinc(pPtr);
			}
			strFormat.Append(pPtr,1);
			switch (*pPtr)
			{
			case _T('Y'):	//10進数で表す4桁の西暦
			case _T('y'):	//10進数で表す西暦の下2桁(00～99)
			case _T('B'):	//月の正式名
			case _T('b'):	//月の省略形
			case _T('m'):	//10進数で表す月(01～12)
			case _T('A'):	//曜日の正式名
			case _T('a'):	//曜日の省略形
			case _T('d'):	//10進数で表す月の日付(01～31)
			case _T('H'):	//24時間表記の時間(00～23)
			case _T('I'):	//12時間表記の時間(00～12)
			case _T('j'):	//10進数で表す年初からの日付(000～366)
			case _T('M'):	//10進数で表す分(00～59)
			case _T('S'):	//10進数で表す秒(00～59)
			case _T('c'):	//ロケールに対応する日付と時刻の表現
			case _T('p'):	//現在のロケーるの午前/午後
			case _T('U'):	//10進数で表す週の通し番号。日曜日を週の最初の日とする(00～53)
			case _T('w'):	//10進数で表す曜日(0～6、日曜日が0)
			case _T('W'):	//10進数で表す週の通し番号。日曜日を週の最初の日とする(00～53)
			case _T('x'):	//現在のロケールの日付表現
			case _T('X'):	//現在のロケーるの時刻表現
			case _T('z'):	//レジストリの設定に応じて、タイムゾーンの名前または省略形を指定します。タイムゾーンが不明な場合は指定しません
			case _T('Z'):	//レジストリの設定に応じて、タイムゾーンの名前または省略形を指定します。タイムゾーンが不明な場合は指定しません
				strAppend = Time.Format(strFormat);
				break;
			case _T('%'):
				strAppend = _T('%');
			default:
				strAppend = _T('%');
				strAppend.Append(pPtr,1);
			}
			pPtr = _tcsinc(pPtr);
			strMakePath.Append(strAppend);
		} else {	//その他の文字
			strMakePath.Append(pPtr,1);
			pPtr = _tcsinc(pPtr);
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////
//エンコードの開始と停止
//////////////////////////////////////////////////
BOOL COutputFileDialog::Encode(BOOL bStart)
{
	ShowApplyButton(bStart);

	if (bStart)
	{
		if (!this->GetEnableCheck())
			return TRUE;
		CString strFileName;
		this->GetOutputFileName(strFileName);
		CString strFolderName;
		this->GetOutputFolderName(strFolderName);
		if (strFileName.IsEmpty() || strFileName.Compare(_T("")) == 0 || strFolderName.IsEmpty() || strFolderName.Compare(_T("")) == 0) {
			SetEnableCheck(FALSE);
			OnBnClickedOutputFileEnableCheck();
			return TRUE;
		}

		CString strMakePath;
		CTime time = CTime::GetCurrentTime();
		this->MakeFilePath(strFolderName,strFileName,time,strMakePath);
		if (strMakePath.IsEmpty() || strMakePath.Compare(_T("")) == 0) {
			SetEnableCheck(FALSE);
			OnBnClickedOutputFileEnableCheck();
			return TRUE;
		}

		//ここでファイル名(strMakePath)の重複をチェックする
		CDirectShowAccess* pDSA= ((CKTEApp*)AfxGetApp())->GetDSA();
		for (int i=0;i<(int)pDSA->m_FileSinkList.size();i++)
		{
			if (pDSA->m_FileSinkList[i].strFileName.Compare(strMakePath) == 0)
			{
				AfxMessageBox(_T("ファイル名が重複しています"),MB_OK|MB_ICONINFORMATION);
				SetEnableCheck(FALSE);
				OnBnClickedOutputFileEnableCheck();
				return TRUE;	//重複程度ではエラーにはらない
			}
		}
		m_nRegisterID = -1;
		m_strRegisterFileName = strMakePath;
		m_RegisterTime = time;
		return pDSA->AddFileSink(m_strRegisterFileName,m_nRegisterID);
	} else {
		if (m_nRegisterID != -1) {
			CDirectShowAccess* pDSA= ((CKTEApp*)AfxGetApp())->GetDSA();
			assert(pDSA);
			pDSA->RemoveFileSink(m_nRegisterID);
		}
		m_nRegisterID = -1;
		m_strRegisterFileName = _T("");
		m_RegisterTime = 0;
	}
	return TRUE;
}
//////////////////////////////////////////////////
//ツールチップの表示
//////////////////////////////////////////////////
BOOL COutputFileDialog::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN: 
		case WM_LBUTTONUP: 
		case WM_MOUSEMOVE: 
			m_ToolTip.RelayEvent(pMsg);
			break;
	}
	return COutputBaseDialog::PreTranslateMessage(pMsg);
}
//////////////////////////////////////////////////
//ダイアログ初期化処理
//////////////////////////////////////////////////
BOOL COutputFileDialog::OnInitDialog()
{
	COutputBaseDialog::OnInitDialog();

	m_ToolTip.Create(this);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_FILE_ENABLE_CHECK),IDC_OUTPUT_FILE_ENABLE_CHECK);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_FILE_PATH_EDIT),IDC_OUTPUT_FILE_PATH_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_FOLDER_PATH_EDIT),IDC_OUTPUT_FOLDER_PATH_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_FILE_BUTTON),IDC_OUTPUT_FILE_BUTTON);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_APPLY_BUTTON),IDC_OUTPUT_APPLY_BUTTON);
	m_ToolTip.SetMaxTipWidth(300);
	DWORD dwAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP,dwAutoPop*2);

	m_nRegisterID = -1;
	m_strRegisterFileName = _T("");
	m_RegisterTime = 0;


	//エンコード中だった場合
	CDirectShowAccess* pDSA= ((CKTEApp*)AfxGetApp())->GetDSA();
	if (pDSA->IsEncode())
	{
		ShowApplyButton(TRUE);
		SetEnableCheck(FALSE);
		OnBnClickedOutputFileEnableCheck();
	} else {
		ShowApplyButton(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

//////////////////////////////////////////////////
//ダイアログ終了処理
//////////////////////////////////////////////////
void COutputFileDialog::OnDestroy()
{
	//エンコード中場合は出力を削除する
	if (m_nRegisterID != -1)
	{
		Encode(FALSE);
	}

	COutputBaseDialog::OnDestroy();
}
//////////////////////////////////////////////////
//エンコード中の設定変更の適用ボタン
//////////////////////////////////////////////////
void COutputFileDialog::OnBnClickedOutputApplyButton()
{
	//有効ボタンのチェックの有無
	BOOL bEnableCheck = this->GetEnableCheck();

	//エンコード開始が無効の時
	if (m_nRegisterID == -1) {
		if (bEnableCheck) {
			Encode(TRUE);
		} else {
			return;
		}
	}
	else{	//エンコード開始が有効の時
		if (bEnableCheck) {
			//ファイル名の変更がある場合には、エンコードの再起動
			CString strFileName;
			CString strFolderName;
			CString strMakePath;
			GetDlgItem(IDC_OUTPUT_FILE_PATH_EDIT)->GetWindowText(strFileName);
			GetDlgItem(IDC_OUTPUT_FOLDER_PATH_EDIT)->GetWindowText(strFolderName);
			this->MakeFilePath(strFolderName,strFileName,this->m_RegisterTime,strMakePath);

			if (strMakePath.Compare(m_strRegisterFileName) != 0) {
				Encode(FALSE);
				Encode(TRUE);
			}
		} else {
			Encode(FALSE);	//エンコードの停止
			//適用ボタンだけは有効にしとく
			ShowApplyButton(TRUE);
		}
	}
}

/*
適用ボタンのシナリオ
エンコードの開始(無効->有効)
エンコードの無効(無効->無効) ※変化無し
エンコードの変更(有効->有効)
	ファイル名同じまま
	ファイル名が違う
エンコードの停止(有効->無効)
*/

