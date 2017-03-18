// OutputPushDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "OutputPushDialog.h"
#include "OutputPropertyDialog.h"

// COutputPushDialog ダイアログ

IMPLEMENT_DYNAMIC(COutputPushDialog, CDialog)

COutputPushDialog::COutputPushDialog(CWnd* pParent /*=NULL*/)
	: COutputBaseDialog(COutputPushDialog::IDD, pParent)
{
	m_nRegisterID = -1;
	m_strRegisterServerName = _T("");
	m_bRegisterAutoDelete = FALSE;
}

COutputPushDialog::~COutputPushDialog()
{
}

void COutputPushDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COutputPushDialog, CDialog)
	ON_BN_CLICKED(IDC_OUTPUT_PUSH_ENABLE_CHECK, &COutputPushDialog::OnBnClickedOutputPushEnableCheck)
	ON_BN_CLICKED(IDC_OUTPUT_PUSH_AUTODELETE_CHECK, &COutputPushDialog::OnBnClickedOutputPushAutodeleteCheck)
	ON_BN_CLICKED(IDC_OUTPUT_APPLY_BUTTON, &COutputPushDialog::OnBnClickedOutputApplyButton)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OUTPUT_PLAYER_BUTTON, &COutputPushDialog::OnBnClickedOutputPlayerButton)
END_MESSAGE_MAP()

//////////////////////////////////////////////////
//出力名の取得
//////////////////////////////////////////////////
BOOL COutputPushDialog::GetOutputName(CString &strFileName)
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_PUSH_PATH_EDIT);
	if (edit == NULL)
		return FALSE;
	edit->GetWindowText(strFileName);
	return TRUE;
}
//////////////////////////////////////////////////
//出力名の設定
//////////////////////////////////////////////////
BOOL COutputPushDialog::SetOutputName(CString strFileName)
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_OUTPUT_PUSH_PATH_EDIT);
	if (edit == NULL)
		return FALSE;
	edit->SetWindowText(strFileName);
	return TRUE;
}
//////////////////////////////////////////////////
//タブ名の取得
//////////////////////////////////////////////////
BOOL COutputPushDialog::GetTabName(CString &strTabName)
{
	CWnd* wnd = (CWnd*)this->GetDlgItem(IDC_OUTPUT_PUSH_GROUP);
	if (wnd == NULL)
		return FALSE;
	wnd->GetWindowText(strTabName);
	return TRUE;
}
//////////////////////////////////////////////////
//タブ名の設定
//////////////////////////////////////////////////
BOOL COutputPushDialog::SetTabName(CString strTabName)
{
	CWnd* wnd = (CWnd*)this->GetDlgItem(IDC_OUTPUT_PUSH_GROUP);
	if (wnd == NULL)
		return FALSE;
	wnd->SetWindowText(strTabName);
	return TRUE;
}
//////////////////////////////////////////////////
//有効チェックの取得
//////////////////////////////////////////////////
BOOL COutputPushDialog::GetEnableCheck()
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_PUSH_ENABLE_CHECK); 
	assert(button);
	if (button->GetCheck() & BST_CHECKED)
		return TRUE;
	return FALSE;
}
//////////////////////////////////////////////////
//有効チェックの設定
//////////////////////////////////////////////////
BOOL COutputPushDialog::SetEnableCheck(BOOL bCheck)
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_PUSH_ENABLE_CHECK); 
	assert(button);
	if (bCheck)
		button->SetCheck(BST_CHECKED);
	else
		button->SetCheck(BST_UNCHECKED);
	return TRUE;
}
//////////////////////////////////////////////////
//自動削除の取得
//////////////////////////////////////////////////
BOOL COutputPushDialog::GetAutoDeleteCheck()
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_PUSH_AUTODELETE_CHECK); 
	assert(button);
	if (button->GetCheck() & BST_CHECKED)
		return TRUE;
	return FALSE;
}
//////////////////////////////////////////////////
//自動削除の設定
//////////////////////////////////////////////////
BOOL COutputPushDialog::SetAutoDeleteCheck(BOOL bCheck)
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_PUSH_AUTODELETE_CHECK); 
	assert(button);
	if (bCheck)
		button->SetCheck(BST_CHECKED);
	else
		button->SetCheck(BST_UNCHECKED);
	return TRUE;
}
//////////////////////////////////////////////////
//適用ボタンの表示と非表示
//////////////////////////////////////////////////
BOOL COutputPushDialog::ShowApplyButton(BOOL bShow)
{
	CWnd* button = (CWnd*)GetDlgItem(IDC_OUTPUT_APPLY_BUTTON);
	assert(button);
	button->ShowWindow(bShow ? SW_SHOW:SW_HIDE);
	CWnd* staticlabel = (CWnd*)GetDlgItem(IDC_OUTPUT_APPLY_STATIC);
	assert(staticlabel);
	staticlabel->ShowWindow(bShow ? SW_SHOW:SW_HIDE);
	button = (CWnd*)GetDlgItem(IDC_OUTPUT_PLAYER_BUTTON);
	assert(button);
	button->ShowWindow(bShow ? SW_SHOW:SW_HIDE);
	return TRUE;
}
//////////////////////////////////////////////////
//有効ボタンのチェック
//////////////////////////////////////////////////
void COutputPushDialog::OnBnClickedOutputPushEnableCheck()
{
	CButton* button = (CButton*)this->GetDlgItem(IDC_OUTPUT_PUSH_ENABLE_CHECK); 
	assert(button);
	//親ウィンドウに通知する
	CWnd* pWnd = this->GetParent();
	pWnd->SendMessage(NOTIFY_ENABLE_CHECK,(WPARAM)this,button->GetCheck());
}
//////////////////////////////////////////////////
//自動的に削除のチェック
//////////////////////////////////////////////////
void COutputPushDialog::OnBnClickedOutputPushAutodeleteCheck()
{
}
//////////////////////////////////////////////////
//ツールチップの表示
//////////////////////////////////////////////////
BOOL COutputPushDialog::PreTranslateMessage(MSG* pMsg)
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
//ダイアログの初期化
//////////////////////////////////////////////////
BOOL COutputPushDialog::OnInitDialog()
{
	COutputBaseDialog::OnInitDialog();

	m_ToolTip.Create(this);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_PUSH_ENABLE_CHECK),IDC_OUTPUT_PUSH_ENABLE_CHECK);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_PUSH_PATH_EDIT),IDC_OUTPUT_PUSH_PATH_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_PUSH_AUTODELETE_CHECK),IDC_OUTPUT_PUSH_AUTODELETE_CHECK);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_APPLY_BUTTON),IDC_OUTPUT_APPLY_BUTTON);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_OUTPUT_PLAYER_BUTTON),IDC_OUTPUT_PLAYER_BUTTON);
	m_ToolTip.SetMaxTipWidth(300);
	DWORD dwAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP,dwAutoPop*2);

	m_nRegisterID = -1;
	m_strRegisterServerName = _T("");
	m_bRegisterAutoDelete = FALSE;

	//適用ボタン
	CDirectShowAccess* pDSA= ((CKTEApp*)AfxGetApp())->GetDSA();
	assert(pDSA);
	if (pDSA->IsEncode())
	{
		ShowApplyButton(TRUE);
		SetEnableCheck(FALSE);
		OnBnClickedOutputPushEnableCheck();
	} else {
		ShowApplyButton(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
//////////////////////////////////////////////////
//ダイアログの終了
//////////////////////////////////////////////////
void COutputPushDialog::OnDestroy()
{
	//エンコード中場合は出力を削除する
	if (m_nRegisterID != -1)
	{
		Encode(FALSE);
	}
	COutputBaseDialog::OnDestroy();
}

//////////////////////////////////////////////////
//サーバー名の文字列変換
//足りない部分を補完する
//http://ServerName:PortNumer/PublishingPoint
//////////////////////////////////////////////////
BOOL COutputPushDialog::ConvertServerName(CString strOldName,CString &strNewName)
{
	strOldName.Trim();	//念のための余白のトリム
	int nIndex = strOldName.Find(_T("http://"));
	if (nIndex == 0) {
		strOldName = strOldName.Right( strOldName.GetLength() - 7);
	}
	nIndex = strOldName.Find(_T("ttp://"));
	if (nIndex == 0) {
		strOldName = strOldName.Right( strOldName.GetLength() - 6);
	}
	nIndex = strOldName.Find(_T("mms://"));
	if (nIndex == 0) {
		strOldName = strOldName.Right( strOldName.GetLength() - 6);
	}
	CString strServerName = _T("");
	CString strPortName = _T("8080");
	CString strPublishName = _T("");
	int nIndexColon = strOldName.Find(_T(':'));
	int nIndexSlash = strOldName.Find(_T('/'));
	if (nIndexColon == -1 && nIndexSlash == -1) {
		strServerName = strOldName;
	} else {
		if (nIndexColon == -1) {
			strServerName = strOldName.Left(nIndexSlash);
			strPublishName = strOldName.Mid(nIndexSlash + 1);
		} else {
			strServerName = strOldName.Left(nIndexColon);
			if (nIndexSlash == -1)
				strPortName = strOldName.Mid(nIndexColon + 1);
			else {
				strPortName = strOldName.Mid(nIndexColon + 1,nIndexSlash-nIndexColon - 1);
				strPublishName = strOldName.Mid(nIndexSlash + 1);
			}
		}
	}
	if (strPortName.IsEmpty() || strPortName.Compare(_T("")) == 0)
		strPortName = _T("8080");
	strNewName.Format(_T("http://%s:%s/%s"),strServerName,strPortName,strPublishName);
	return TRUE;
}

//////////////////////////////////////////////////
//プレイヤーで開くボタン
//////////////////////////////////////////////////
void COutputPushDialog::OnBnClickedOutputPlayerButton()
{
	CString strServerName;
	this->GetOutputName(strServerName);
	if (!strServerName.IsEmpty())
	{
		CString strConvertServerName = _T("");
		ConvertServerName(m_strRegisterServerName,strConvertServerName);
		if (!strConvertServerName.IsEmpty())
		{
			CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
			if (!pSetting->m_strVideoPlayerPath.IsEmpty())
			{
				//コマンドラインでメディアプレイヤーを起動する
				::ShellExecute(NULL,NULL,pSetting->m_strVideoPlayerPath,strConvertServerName,NULL,SW_SHOWNORMAL);
			}
		}
	}
}

//////////////////////////////////////////////////
//エンコードの開始と停止
//////////////////////////////////////////////////
BOOL COutputPushDialog::Encode(BOOL bStart)
{
	ShowApplyButton(bStart);

	if (bStart)
	{
		if (!GetEnableCheck())
			return TRUE;
		CString strServerName = _T("");
		this->GetOutputName(strServerName);
		if (strServerName.Compare(_T("")) == 0 || strServerName.IsEmpty()) {
			SetEnableCheck(FALSE);
			OnBnClickedOutputPushEnableCheck();
			return TRUE;
		}

		CDirectShowAccess* pDSA= ((CKTEApp*)AfxGetApp())->GetDSA();
		CString strConvertServerName = _T("");
		ConvertServerName(strServerName,strConvertServerName);
		BOOL bAlreadyInsert = FALSE;
		for (int i = 0;i<(int)pDSA->m_PushSinkList.size();i++)
		{
			if (pDSA->m_PushSinkList[i].strServerName.Compare(strConvertServerName) == 0)
			{
				AfxMessageBox(_T("サーバー接続先が重複しています"),MB_OK|MB_ICONINFORMATION);
				SetEnableCheck(FALSE);
				OnBnClickedOutputPushEnableCheck();
				return TRUE;	//重複程度ではエラーにはらない
			}
		}
		m_nRegisterID = -1;
		m_strRegisterServerName = strServerName;
		//ConvertServerName(m_strRegisterServerName,strConvertServerName);	//サーバー名の生成
		m_bRegisterAutoDelete = this->GetAutoDeleteCheck();
		return pDSA->AddPushSink(strConvertServerName,m_bRegisterAutoDelete,m_nRegisterID);
	} else {
		if (m_nRegisterID != -1) {
			CDirectShowAccess* pDSA= ((CKTEApp*)AfxGetApp())->GetDSA();
			assert(pDSA);
			pDSA->RemovePushSink(m_nRegisterID);
		}
		m_nRegisterID = -1;
		m_strRegisterServerName = _T("");
		m_bRegisterAutoDelete = FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////
//適用ボタン
//////////////////////////////////////////////////
void COutputPushDialog::OnBnClickedOutputApplyButton()
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
			//サーバー名の変更がある場合には、エンコードの再起動
			CString strServerName;
			this->GetOutputName(strServerName);
			if (strServerName.Compare(m_strRegisterServerName) != 0) {
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
	サーバー名のみの変更	→建て直し
	自動削除のみの変更		→変更なし
	両方の変更				→建て直し
エンコードの停止(有効->無効)
*/
