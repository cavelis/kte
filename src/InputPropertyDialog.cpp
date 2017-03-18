// InputPropertyDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "InputPropertyDialog.h"
#include "InputPropertyConfigDialog.h"
#include "MainFrm.h"

// CInputPropertyDialog ダイアログ

IMPLEMENT_DYNAMIC(CInputPropertyDialog, CSizeDialog)


BEGIN_MESSAGE_MAP(CInputPropertyDialog, CSizeDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_VSCROLL()
	ON_CBN_SELCHANGE(IDC_VIDEO_SETTING_COMBO, &CInputPropertyDialog::OnCbnSelchangeVideoSettingCombo)
	ON_CBN_SELCHANGE(IDC_VIDEO_RESOLUTION_COMBO, &CInputPropertyDialog::OnCbnSelchangeVideoResolutionCombo)
	ON_CBN_SELCHANGE(IDC_AUDIO_SETTING_COMBO, &CInputPropertyDialog::OnCbnSelchangeAudioSettingCombo)
	ON_BN_CLICKED(IDC_VIDEO_SETTING_BUTTON, &CInputPropertyDialog::OnBnClickedVideoSettingButton)
	ON_BN_CLICKED(IDC_AUDIO_SETTING_BUTTON, &CInputPropertyDialog::OnBnClickedAudioSettingButton)
	ON_BN_CLICKED(IDC_VIDEO_RESOLUTION_BUTTON, &CInputPropertyDialog::OnBnClickedVideoResolutionButton)
	//ON_MESSAGE(WM_DEVICECHANGE, &CInputPropertyDialog::OnDeviceChange)
	//ON_WM_SIZE()
	//ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_AUDIO_RESOLUTION_BUTTON, &CInputPropertyDialog::OnBnClickedAudioResolutionButton)
	ON_CBN_SELCHANGE(IDC_AUDIO_RESOLUTION_COMBO, &CInputPropertyDialog::OnCbnSelchangeAudioResolutionCombo)
	ON_BN_CLICKED(IDC_INPUT_CONFIG_BUTTON, &CInputPropertyDialog::OnBnClickedInputConfigButton)
	ON_COMMAND(ID_WDM_DEVICE, &CInputPropertyDialog::OnWdmDevice)
	ON_COMMAND(ID_WDM_CROSSBAR1, &CInputPropertyDialog::OnWdmCrossbar1)
	ON_COMMAND(ID_WDM_CROSSBAR2, &CInputPropertyDialog::OnWdmCrossbar2)
	ON_COMMAND(ID_WDM_TVAUDIO, &CInputPropertyDialog::OnWdmTvaudio)
	ON_COMMAND(ID_WDM_TVTUNER, &CInputPropertyDialog::OnWdmTvtuner)
END_MESSAGE_MAP()


CInputPropertyDialog::CInputPropertyDialog(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CInputPropertyDialog::IDD, pParent)
{
	m_bFirstFlag = TRUE;
}

CInputPropertyDialog::~CInputPropertyDialog()
{
}

void CInputPropertyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



//////////////////////////////////////////////////
//初期化処理
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::OnInitDialog()
{
	CSizeDialog::OnInitDialog();
	m_bFirstFlag = TRUE;	//初期設定開始

	m_ToolTip.Create(this);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.AddTool((CComboBox *)GetDlgItem(IDC_VIDEO_SETTING_COMBO),IDC_VIDEO_SETTING_COMBO);
	m_ToolTip.AddTool((CButton *)GetDlgItem(IDC_VIDEO_SETTING_BUTTON),IDC_VIDEO_SETTING_BUTTON);
	m_ToolTip.AddTool((CComboBox *)GetDlgItem(IDC_VIDEO_RESOLUTION_COMBO),IDC_VIDEO_RESOLUTION_COMBO);
	m_ToolTip.AddTool((CComboBox *)GetDlgItem(IDC_VIDEO_RESOLUTION_BUTTON),IDC_VIDEO_RESOLUTION_BUTTON);
	m_ToolTip.AddTool((CComboBox *)GetDlgItem(IDC_AUDIO_SETTING_COMBO),IDC_AUDIO_SETTING_COMBO);
	m_ToolTip.AddTool((CButton *)GetDlgItem(IDC_AUDIO_SETTING_BUTTON),IDC_AUDIO_SETTING_BUTTON);
	m_ToolTip.AddTool((CComboBox *)GetDlgItem(IDC_AUDIO_RESOLUTION_COMBO),IDC_AUDIO_RESOLUTION_COMBO);
	m_ToolTip.AddTool((CComboBox *)GetDlgItem(IDC_AUDIO_RESOLUTION_BUTTON),IDC_AUDIO_RESOLUTION_BUTTON);
	m_ToolTip.AddTool((CComboBox *)GetDlgItem(IDC_INPUT_CONFIG_BUTTON),IDC_INPUT_CONFIG_BUTTON);

	m_ToolTip.SetMaxTipWidth(300);
	DWORD dwAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP,dwAutoPop*2);

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	CComboBox* combo;
	int nInsertIndex;
	//ビデオキャプチャデバイスのコンボボックスを作成
	combo = (CComboBox*)GetDlgItem(IDC_VIDEO_SETTING_COMBO); assert(combo);
	if (pDSA->m_strVideoNameList.size() != 0)
	{
		for (int i=0;i<(int)pDSA->m_strVideoNameList.size();i++)
		{
			combo->AddString(pDSA->m_strVideoNameList[i].c_str());
		}
	} else
	{
		VideoDeviceFailed(0);
		CEdit* edit = (CEdit*)GetDlgItem(IDC_INPUT_ERROR_EDIT); assert(edit);
		edit->SetWindowText(_T("ビデオキャプチャデバイスが検出されませんでした\r\nエンコードは開始できません\r\n"));
	}

	//オーディオキャプチャデバイスのコンボボックスを作成
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_SETTING_COMBO); assert(combo);
	if (pDSA->m_strAudioNameList.size() != 0)
	{
		for (int i=0;i<(int)pDSA->m_strAudioNameList.size();i++)
		{
			nInsertIndex = combo->AddString(pDSA->m_strAudioNameList[i].c_str());
			combo->SetItemData(nInsertIndex,(DWORD_PTR)i);
		}
	} else
	{
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
		CWnd* wnd = (CWnd*)GetDlgItem(IDC_AUDIO_SETTING_BUTTON); assert(wnd);
		wnd->EnableWindow(FALSE);
		//エラー表示
		CEdit* edit = (CEdit*)GetDlgItem(IDC_INPUT_ERROR_EDIT); assert(edit);
		CString strErr;
		edit->GetWindowText(strErr);
		strErr += _T("オーディオキャプチャデバイスが検出されませんでした\r\nエンコードは開始できません"); 
		edit->SetWindowText(strErr);
	}

	//プロファイルのロード
	LoadProfileSetting();

	m_bFirstFlag = FALSE;	//初期設定フラグの終了

	return TRUE;
}


//////////////////////////////////////////////////
//ダイアログ終了時
//////////////////////////////////////////////////
void CInputPropertyDialog::OnDestroy()
{
	//プロファイルのセーブ
	SaveProfileSetting();

	CDialog::OnDestroy();
}
//////////////////////////////////////////////////
//ツールチップの表示
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN: 
		case WM_LBUTTONUP: 
		case WM_MOUSEMOVE: 
			m_ToolTip.RelayEvent(pMsg);
			break;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
//////////////////////////////////////////////////
//警告コントロールについてのブラシ色変更
//////////////////////////////////////////////////
HBRUSH CInputPropertyDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (nCtlColor == CTLCOLOR_STATIC) {
		if (pWnd->GetDlgCtrlID() == IDC_INPUT_ERROR_EDIT)
			pDC->SetTextColor(RGB(255,0,0));
	}
	return hbr;
}
//////////////////////////////////////////////////
//ビデオキャプチャデバイス(コンボボックス)を
//選択したときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CInputPropertyDialog::OnCbnSelchangeVideoSettingCombo()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_VIDEO_SETTING_COMBO);
	CComboBox* combo_resolution = (CComboBox*)GetDlgItem(IDC_VIDEO_RESOLUTION_COMBO);
	if (!combo || !combo_resolution)
		return;
	int nIndex = combo->GetCurSel();
	if (nIndex == CB_ERR) return;

	//フィルタの一時停止
	BOOL bPreview = pDSA->IsRunFilter();
	if (m_bFirstFlag == FALSE && bPreview)
		pDSA->StopFilter();

	//ビデオを変更する
	if (!(pDSA->SelectVideoIndex(nIndex)))
	{
		AfxMessageBox(_T("ビデオキャプチャデバイスの確保に失敗しました"));
		VideoDeviceFailed(1);
		return;
	}

	//構成ボタンが有効になる
	CWnd* wnd = GetDlgItem(IDC_VIDEO_SETTING_BUTTON); assert(wnd);
	if (wnd->IsWindowEnabled() == FALSE)
		wnd->EnableWindow(TRUE);

	//解像度一覧を取得する
	vector<wstring> video_resolution = ((CKTEApp*)AfxGetApp())->GetDSA()->m_strVideoSettingList;
	if (video_resolution.size() == 0) {
		//解像度一覧設定が全く無い場合
		VideoDeviceFailed(2);
		return;
	}

	//ビデオキャプチャデバイスの設定
	if (combo_resolution->IsWindowEnabled() == FALSE)
		combo_resolution->EnableWindow(TRUE);
	combo_resolution->ResetContent();
	int nInsertIndex;
	for (int i=0;i<(int)video_resolution.size();i++)
	{
		//※FORMAT_VideoInfo2のタイプについては記載しない
		if (pDSA->m_VideoMediaTypeList[i].formattype == FORMAT_VideoInfo)
		{
			nInsertIndex = combo_resolution->AddString(video_resolution[i].c_str());
			combo_resolution->SetItemData(nInsertIndex,i);	//インデックスを埋め込み
		}
	}
	//初回起動時のみのデフォルト値を設定する
	if (m_bFirstFlag)
	{
		if (pProfile->m_nVideoSettingIndex >= 0 && pProfile->m_nVideoSettingIndex < (int)video_resolution.size())
		{
			if (pProfile->m_strVideoSettingName.Compare(video_resolution[pProfile->m_nVideoSettingIndex].c_str()) == 0)
				combo_resolution->SetCurSel(pProfile->m_nVideoSettingIndex);	//n番目のビデオを選択
		}
	}
	if (combo_resolution->GetCurSel() == CB_ERR)
		combo_resolution->SetCurSel(0);	//0番目のビデオ解像度を選択

	//ビデオ解像度コンボボックスを選択したモジュールを呼び出す
	OnSelectVideoSettingIndex(NULL);
	//オーディオピンの有無を確認し、オーディオキャプチャリストに追加する
	ProcVideoAudioInputPin();
	return;
}
//////////////////////////////////////////////////
//ビデオキャプチャデバイス(コンボボックス)の解像度設定を
//選択したときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CInputPropertyDialog::OnCbnSelchangeVideoResolutionCombo()
{
	OnSelectVideoSettingIndex(NULL);
}
//////////////////////////////////////////////////
//ビデオキャプチャデバイス(コンボボックス)の解像度設定を
//選択したときに呼ばれるイベントハンドラの中身
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::OnSelectVideoSettingIndex(AM_MEDIA_TYPE* pamt)
{
	CComboBox*  combo = (CComboBox*)GetDlgItem(IDC_VIDEO_SETTING_COMBO);
	if (!combo) return FALSE;
	if (combo->IsWindowEnabled() == FALSE) return FALSE;
	if (combo->GetCurSel() == CB_ERR) return FALSE;
	CComboBox* combo_resolution = (CComboBox*)GetDlgItem(IDC_VIDEO_RESOLUTION_COMBO);
	if (!combo_resolution) return FALSE;
	if (combo_resolution->IsWindowEnabled() == FALSE) return FALSE;
	if (combo_resolution->GetCurSel() == CB_ERR) return FALSE;

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	//フィルタの一時停止
	BOOL bPreview = pDSA->IsRunFilter();
	if (bPreview)
		pDSA->StopFilter();

	//この解像度変更がフィルタ再構築の方法
	int nIndex = (int)combo_resolution->GetItemData(combo_resolution->GetCurSel());

	if (!pDSA->SelectVideoSettingIndex(nIndex,pamt))
	{
		AfxMessageBox(_T("ビデオ解像度の変更に失敗しました"));
		VideoDeviceFailed(3);
		return FALSE;
	}

	//デバイスのキャパシティゾーンを設定(幅と高さとフレームレート)
	VideoDeviceCapacity(nIndex);

	//////////////////////////////////////////////////
	//プレビューの再開
	//カメラが占有されている場合はタイミングで失敗する
	if (m_bFirstFlag == FALSE && !pDSA->IsRunFilter() && pDSA->IsCanPreview())
	{
		//このタイミングで解像度が変更された旨をプレビューウィンドウに通知する
		((CMainFrame*)AfxGetMainWnd())->GetPreviewWnd()->SetPreviewWnd();
		((CMainFrame*)AfxGetMainWnd())->GetPreviewWnd()->SetPreviewSize(m_bFirstFlag?FALSE:TRUE);
		BOOL bRet = pDSA->RunFilter();
		if (bRet == FALSE)
		{
			VideoDeviceFailed(1);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////
//ビデオデバイスのキャパシティゾーンを設定(ビデオの幅・高さ・フレームレート・適用ボタン)
//int nIndex 設定状況を取得するAM_MEDIA_TYPEのインデックス
//TODO 64ビットカメラ環境にてこの処理に問題あり(何かパラメータが取得できない問題)
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::VideoDeviceCapacity(int nIndex)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (nIndex < 0 || nIndex >= (int)pDSA->m_VideoCapsList.size()) return FALSE;

	//現在のビデオ設定のキャパシティ最大・最小値を取得
	VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)pDSA->m_VideoFirstMediaType.pbFormat;					//現在の設定値
	VIDEO_STREAM_CONFIG_CAPS* pvscc = (VIDEO_STREAM_CONFIG_CAPS*)&(pDSA->m_VideoCapsList[nIndex]);	//キャプチャビデオデバイスの性能

	if (pvih == NULL) {
		pvih = (VIDEOINFOHEADER*)pDSA->m_VideoMediaTypeList[nIndex].pbFormat;			//適用した設定値
	}

	//ビデオの幅
	CSpinButtonCtrl* spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_WIDTH_SPIN); assert(spin);
	spin->SetRange32(pvscc->MinOutputSize.cx,pvscc->MaxOutputSize.cx);
	spin->SetPos32(pvih->bmiHeader.biWidth);
	if (pvscc->MinOutputSize.cx != pvscc->MaxOutputSize.cx) {
		spin->EnableWindow(TRUE);
		GetDlgItem(IDC_VIDEO_WIDTH_EDIT)->EnableWindow(TRUE);
	} else {
		spin->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_WIDTH_EDIT)->EnableWindow(FALSE);
	}

	//ビデオの高さ
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_HEIGHT_SPIN); assert(spin);
	spin->SetRange32(pvscc->MinOutputSize.cy,pvscc->MaxOutputSize.cy);
	spin->SetPos32(pvih->bmiHeader.biHeight);
	if (pvscc->MinOutputSize.cy != pvscc->MaxOutputSize.cy) {
		spin->EnableWindow(TRUE);
		GetDlgItem(IDC_VIDEO_HEIGHT_EDIT)->EnableWindow(TRUE);
	} else {
		spin->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_HEIGHT_EDIT)->EnableWindow(FALSE);
	}
	//ビデオのフレームレート
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SPIN); assert(spin);
	double dCurrentFPS,dMinFPS,dMaxFPS;
	if (pvih->AvgTimePerFrame != 0)
		dCurrentFPS = (double)((double)10000000.0/(double)pvih->AvgTimePerFrame);
	else
		dCurrentFPS = 0.0;
	if (pvscc->MaxFrameInterval != 0)
		dMinFPS = (double)((double)10000000.0/(double)pvscc->MaxFrameInterval);
	else
		dMinFPS = 0.0;
	if (pvscc->MinFrameInterval != 0)
		dMaxFPS = (double)((double)10000000.0/(double)pvscc->MinFrameInterval);
	else
		dMaxFPS = 0.0;
	//spin->SetRange32((int)(dMinFPS*100),(int)(dMaxFPS*100));
	//spin->SetPos32((int)(dCurrentFPS*100));
	spin->SetRange32((int)(dMinFPS),(int)(dMaxFPS));
	spin->SetPos32((int)dCurrentFPS);
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT); assert(edit);
	CString strNum;
	if (((int)(dCurrentFPS*100)) % 10 != 0)
		strNum.Format(_T("%.2f"),dCurrentFPS);
	else if (((int)(dCurrentFPS*10)) % 10 != 0)
		strNum.Format(_T("%.1f"),dCurrentFPS);
	else
		strNum.Format(_T("%.0f"),dCurrentFPS);
	edit->SetWindowText(strNum);

	if (dMinFPS != dMaxFPS) {
		spin->EnableWindow(TRUE);
		edit->EnableWindow(TRUE);
	} else {
		spin->EnableWindow(FALSE);
		edit->EnableWindow(FALSE);
	}

	//適用ボタンの有効・無効
	if (!GetDlgItem(IDC_VIDEO_WIDTH_EDIT)->IsWindowEnabled() && 
		!GetDlgItem(IDC_VIDEO_HEIGHT_EDIT)->IsWindowEnabled() &&
		!GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT)->IsWindowEnabled())
		GetDlgItem(IDC_VIDEO_RESOLUTION_BUTTON)->EnableWindow(FALSE);
	else
		GetDlgItem(IDC_VIDEO_RESOLUTION_BUTTON)->EnableWindow(TRUE);

	//ツールチップの配置
	strNum.Format(_T("幅の最大値:%d\r\n幅の最小値:%d\r\n幅の増分:%d"),
		pvscc->MaxOutputSize.cx,pvscc->MinOutputSize.cx,pvscc->OutputGranularityX);
	m_ToolTip.DelTool(GetDlgItem(IDC_VIDEO_WIDTH_EDIT));
	m_ToolTip.AddTool(GetDlgItem(IDC_VIDEO_WIDTH_EDIT),strNum);
	strNum.Format(_T("高さの最大値:%d\r\n高さの最小値:%d\r\n高さの増分:%d"),
		pvscc->MaxOutputSize.cy,pvscc->MinOutputSize.cy,pvscc->OutputGranularityY);
	m_ToolTip.DelTool(GetDlgItem(IDC_VIDEO_HEIGHT_EDIT));
	m_ToolTip.AddTool(GetDlgItem(IDC_VIDEO_HEIGHT_EDIT),strNum);
	strNum.Format(_T("フレームレートの最大値:%.2f\r\nフレームレートの最小値:%.2f"),dMaxFPS,dMinFPS);
	m_ToolTip.DelTool(GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT));
	m_ToolTip.AddTool(GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT),strNum);

	return TRUE;
}

//////////////////////////////////////////////////
//ビデオ解像度の適用ボタン
//////////////////////////////////////////////////
void CInputPropertyDialog::OnBnClickedVideoResolutionButton()
{
	//適用ボタンが有効かどうかチェックする
	CComboBox*  combo = (CComboBox*)GetDlgItem(IDC_VIDEO_SETTING_COMBO);
	if (!combo) return;
	if (combo->IsWindowEnabled() == FALSE) return;
	if (combo->GetCurSel() == CB_ERR) return;
	CComboBox* combo_resolution = (CComboBox*)GetDlgItem(IDC_VIDEO_RESOLUTION_COMBO);
	if (!combo_resolution) return;
	if (combo_resolution->IsWindowEnabled() == FALSE) return;
	if (combo_resolution->GetCurSel() == CB_ERR) return;

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	//設定の取得
	int nWidth , nHeight;
	double dFPS;
	GetVideoCurrentInput(nWidth,nHeight,dFPS);

	//フィルタの一時停止
	BOOL bPreview = pDSA->IsRunFilter();
	if (bPreview == TRUE)
		pDSA->StopFilter();

	//メディアタイプの改変
	AM_MEDIA_TYPE amt;
	ZeroMemory(&amt,sizeof(AM_MEDIA_TYPE));
	amt = pDSA->m_VideoFirstMediaType;
	VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)amt.pbFormat;
	if (pvih)
	{
		pvih->bmiHeader.biWidth = nWidth;
		pvih->bmiHeader.biHeight = nHeight;
		if (dFPS != 0.0)
			pvih->AvgTimePerFrame = (REFERENCE_TIME)(((double)10000000)/dFPS);
		//サンプルサイズを変更
		ULONG ISampleSize = (nWidth*nHeight*pvih->bmiHeader.biBitCount)/8;
		amt.lSampleSize = ISampleSize;
		pvih->bmiHeader.biSizeImage = ISampleSize;

		//設定の適用
		if (!pDSA->SelectVideoSettingIndex(pDSA->GetVideoSettingIndex(),&amt)) {
			AfxMessageBox(_T("ビデオ設定の適用に失敗しました"),MB_OK|MB_ICONINFORMATION);
			return;
		}
	}

	//ビデオ設定値の取得
	int nIndex = (int)combo_resolution->GetItemData(combo_resolution->GetCurSel());
	VideoDeviceCapacity(nIndex);

	//プレビューの再開
	if (m_bFirstFlag == FALSE && pDSA->IsRunFilter() == FALSE && pDSA->IsCanPreview() == TRUE)
	{
		//このタイミングで解像度が変更された旨をプレビューウィンドウに通知する
		((CMainFrame*)AfxGetMainWnd())->GetPreviewWnd()->SetPreviewWnd();
		((CMainFrame*)AfxGetMainWnd())->GetPreviewWnd()->SetPreviewSize(m_bFirstFlag?FALSE:TRUE);
		BOOL bRet = pDSA->RunFilter();
	}
	return;
}



//////////////////////////////////////////////////
//オーディオコンボボックスを選択した際に呼ばれるイベントハンドら
//////////////////////////////////////////////////
void CInputPropertyDialog::OnCbnSelchangeAudioSettingCombo()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_SETTING_COMBO); assert(combo);
	if (!combo) return;
	if (combo->IsWindowEnabled() == FALSE) return;
	if (combo->GetCurSel() == CB_ERR) return;

	int nIndex = (int)combo->GetItemData(combo->GetCurSel());

	if (nIndex == SHARE_VIDEO_AUDIO) {
		if (!pDSA->IsVidoFindAudioPin()) {
			AfxMessageBox(_T("ビデオキャプチャデバイス上にオーディオが見つかりませんでした"));
			//TODO 構成ボタン周りも押せなくするか、
			//前に選んでいた状態に戻すかする
			return;
		}
	}

	//フィルタの一時停止
	BOOL bPreview = pDSA->IsRunFilter();
	if (bPreview)
		pDSA->StopFilter();

	//
	if (nIndex >= 0 && nIndex<(int)pDSA->m_strAudioNameList.size()){
		//普通のオーディオキャプチャカードが選択された場合
		if (!pDSA->SelectAudioIndex(nIndex,FALSE))
		{
			AfxMessageBox(_T("オーディオキャプチャデバイスの確保に失敗しました"));
		}
	} else if (nIndex == SHARE_VIDEO_AUDIO){
		//ビデオフィルターが有しているオーディオキャプチャカードを使用する場合
		//ここを直した
		int nAudioIndex = pDSA->GetAudioIndex();
		if (nAudioIndex == -1)
			nAudioIndex = 0;
		if (!pDSA->SelectAudioIndex(nAudioIndex,TRUE)){
			AfxMessageBox(_T("ビデオキャプチャデバイス上のオーディオキャプチャデバイスの確保に失敗しました"));
		}
	}

	//詳細コンボボックスの編集
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_RESOLUTION_COMBO); assert(combo);
	combo->ResetContent();
	if (!combo->IsWindowEnabled()) combo->EnableWindow(TRUE);
	int nInsertIndex;
	for (int i=0;i<(int)pDSA->m_strAudioSettingList.size();i++)
	{
		nInsertIndex = combo->AddString(pDSA->m_strAudioSettingList[i].c_str());
		combo->SetItemData(nInsertIndex,(DWORD_PTR)i);
	}
	if (combo->GetCount() == 0)
	{
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
	}

	//初回起動時のみのデフォルト値を設定する
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	if (m_bFirstFlag)
	{
		if (pProfile->m_nAudioSettingIndex >= 0 && pProfile->m_nAudioSettingIndex < (int)pDSA->m_strAudioSettingList.size())
		{
			if (pProfile->m_strAudioSettingName.Compare(pDSA->m_strAudioSettingList[pProfile->m_nAudioSettingIndex].c_str()) == 0)
				combo->SetCurSel(pProfile->m_nAudioSettingIndex);	//n番目の設定を選択
		}
	}
	if (combo->GetCurSel() == CB_ERR)
		combo->SetCurSel(0);	//0番目の設定を選択

	//オーディオコンボボックスを変更
	OnCbnSelchangeAudioResolutionCombo();

}
//////////////////////////////////////////////////
//オーディオの詳細設定コンボボックスを変更した際に呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CInputPropertyDialog::OnCbnSelchangeAudioResolutionCombo()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_RESOLUTION_COMBO); assert(combo);
	if (!combo)
		return;
	if (combo->IsWindowEnabled() == FALSE)
		return;
	if (combo->GetCurSel() == CB_ERR)
		return;

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	
	//フィルタの一時停止
	BOOL bPreview = pDSA->IsRunFilter();
	if (bPreview)
		pDSA->StopFilter();

	//オーディオ設定の変更を行う
	int nIndex = (int)combo->GetItemData(combo->GetCurSel());
	if (!pDSA->SelectAudioSettingIndex(nIndex,NULL))
	{
		AfxMessageBox(_T("オーディオ設定の変更に失敗しました"));
		return;
	}
	//オーディオデバイスのキャパシティゾーンを設定
	AudioDeviceCapacity(nIndex);

	//プレビューの再開
	if (!m_bFirstFlag && !pDSA->IsRunFilter() && pDSA->IsCanPreview())
	{
		pDSA->RunFilter();
	}
	return;
}

//////////////////////////////////////////////////
//オーディオのキャパシティゾーンを設定(周波数・分解能・チャンネル・適用ボタン)
//int nIndex 設定状況を取得するAM_MEDIA_TYPEのインデックス
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::AudioDeviceCapacity(int nIndex)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	if (nIndex < 0 || nIndex >= (int)pDSA->m_AudioCapsList.size()) return FALSE;

	WAVEFORMATEX* pWaveFormatEx = (WAVEFORMATEX*)pDSA->m_AudioFirstMediaType.pbFormat;
	AUDIO_STREAM_CONFIG_CAPS* pascc = &(pDSA->m_AudioCapsList[nIndex]);

	int nInsertIndex;
	CString strNum;
	//周波数
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_FREQ_COMBO); assert(combo); 
	combo->ResetContent();
	if (!combo->IsWindowEnabled())
		combo->EnableWindow(TRUE);

	ULONG i = pascc->MinimumSampleFrequency;
	do {
		strNum.Format(_T("%d"),i);
		nInsertIndex = combo->AddString(strNum);
		combo->SetItemData(nInsertIndex,(DWORD_PTR)i);
		if (i == pWaveFormatEx->nSamplesPerSec)
			combo->SetCurSel(nInsertIndex);
		i += pascc->SampleFrequencyGranularity;
	} while (i <= pascc->MaximumSampleFrequency && pascc->SampleFrequencyGranularity != 0);
	if (combo->GetCount() == 0)
	{
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
	}
	if (combo->GetCurSel() == CB_ERR)
		combo->SetCurSel(0);
	if (pascc->MinimumSampleFrequency == pascc->MaximumSampleFrequency)	//設定が変更できない
		combo->EnableWindow(FALSE);

	//分解能
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_BITS_COMBO); assert(combo); 
	combo->ResetContent();
	if (!combo->IsWindowEnabled())
		combo->EnableWindow(TRUE);
	i = pascc->MinimumBitsPerSample;
	do {
		strNum.Format(_T("%d"),i);
		nInsertIndex = combo->AddString(strNum);
		combo->SetItemData(nInsertIndex,(DWORD_PTR)i);
		if (i == pWaveFormatEx->wBitsPerSample)
			combo->SetCurSel(nInsertIndex);
		i += pascc->BitsPerSampleGranularity;
	} while (i <= pascc->MaximumBitsPerSample && pascc->BitsPerSampleGranularity != 0);
	if (combo->GetCount() == 0)
	{
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
	}
	if (combo->GetCurSel() == CB_ERR)
		combo->SetCurSel(0);
	if (pascc->MinimumBitsPerSample == pascc->MaximumBitsPerSample)	//設定が変更できない
		combo->EnableWindow(FALSE);

	//チャンネル
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_CHANNELS_COMBO); assert(combo); 
	combo->ResetContent();
	if (!combo->IsWindowEnabled())
		combo->EnableWindow(TRUE);
	i = pascc->MinimumChannels;
	do {
		strNum.Format(_T("%d"),i);
		nInsertIndex = combo->AddString(strNum);
		combo->SetItemData(nInsertIndex,(DWORD_PTR)i);
		if (i == pWaveFormatEx->nChannels)
			combo->SetCurSel(nInsertIndex);
		i += pascc->ChannelsGranularity;
	} while (i <= pascc->MaximumChannels && pascc->ChannelsGranularity != 0);
	if (combo->GetCount() == 0)
	{
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
	}
	if (combo->GetCurSel() == CB_ERR)
		combo->SetCurSel(0);
	if (pascc->MinimumChannels == pascc->MaximumChannels)	//設定が変更できない
		combo->EnableWindow(FALSE);

	//適用ボタンの有効・無効
	if (!GetDlgItem(IDC_AUDIO_FREQ_COMBO)->IsWindowEnabled() && 
		!GetDlgItem(IDC_AUDIO_BITS_COMBO)->IsWindowEnabled() &&
		!GetDlgItem(IDC_AUDIO_CHANNELS_COMBO)->IsWindowEnabled())
		GetDlgItem(IDC_AUDIO_RESOLUTION_BUTTON)->EnableWindow(FALSE);
	else
		GetDlgItem(IDC_AUDIO_RESOLUTION_BUTTON)->EnableWindow(TRUE);

	//ツールチップの配置
	strNum.Format(_T("周波数の最大値:%d\r\n周波数の最小値:%d\r\n周波数の増分:%d"),
		pascc->MaximumSampleFrequency,pascc->MinimumSampleFrequency,pascc->SampleFrequencyGranularity);
	m_ToolTip.DelTool(GetDlgItem(IDC_AUDIO_FREQ_COMBO));
	m_ToolTip.AddTool(GetDlgItem(IDC_AUDIO_FREQ_COMBO),strNum);
	strNum.Format(_T("分解能の最大値:%d\r\n分解能の最小値:%d\r\n分解能の増分:%d"),
		pascc->MaximumBitsPerSample,pascc->MinimumBitsPerSample,pascc->BitsPerSampleGranularity);
	m_ToolTip.DelTool(GetDlgItem(IDC_AUDIO_BITS_COMBO));
	m_ToolTip.AddTool(GetDlgItem(IDC_AUDIO_BITS_COMBO),strNum);
	strNum.Format(_T("チャンネルの最大値:%d\r\nチャンネルの最小値:%d\r\nチャンネルの増分:%d"),
		pascc->MaximumChannels,pascc->MinimumChannels,pascc->ChannelsGranularity);
	m_ToolTip.DelTool(GetDlgItem(IDC_AUDIO_CHANNELS_COMBO));
	m_ToolTip.AddTool(GetDlgItem(IDC_AUDIO_CHANNELS_COMBO),strNum);	

	return TRUE;
}

//////////////////////////////////////////////////
//オーディオコンボボックスの適用ボタン
//////////////////////////////////////////////////
void CInputPropertyDialog::OnBnClickedAudioResolutionButton()
{
	//適用ボタンが有効かどうかチェックする
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_SETTING_COMBO); assert(combo);
	if (!combo)return;
	if (!combo->IsWindowEnabled()) return;
	if (combo->GetCurSel() == CB_ERR) return;
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_RESOLUTION_COMBO); assert(combo);
	if (!combo)return;
	if (!combo->IsWindowEnabled()) return;
	if (combo->GetCurSel() == CB_ERR) return;

	//設定の取得
	DWORD nSamplesPerSec;
	WORD wBitsPerSample;
	WORD nChannels;
	GetAudioCurrentInput(nSamplesPerSec,wBitsPerSample,nChannels);

	//フィルタの一時停止
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	BOOL bPreview = pDSA->IsRunFilter();
	if (bPreview == TRUE)
		pDSA->StopFilter();

	//メディアタイプを改変
	AM_MEDIA_TYPE amt;
	ZeroMemory(&amt,sizeof(AM_MEDIA_TYPE));
	amt = pDSA->m_AudioFirstMediaType;
	WAVEFORMATEX* pWaveFormatEx = (WAVEFORMATEX*)amt.pbFormat;
	pWaveFormatEx->nSamplesPerSec = nSamplesPerSec;
	pWaveFormatEx->wBitsPerSample = wBitsPerSample;
	pWaveFormatEx->nChannels = nChannels;
	pWaveFormatEx->nBlockAlign = (nChannels * wBitsPerSample) / 8;
	pWaveFormatEx->nAvgBytesPerSec = nSamplesPerSec * pWaveFormatEx->nBlockAlign;

	//設定の適用
	if (!pDSA->SelectAudioSettingIndex(pDSA->GetAudioSettingIndex(),&amt)) {
		AfxMessageBox(_T("オーディオ設定の適用に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return;
	}

	//オーディオ設定値の取得
	//int nIndex = (int)combo->GetItemData(combo->GetCurSel());
	//AudioDeviceCapacity(nIndex);

	//プレビューの再開
	if (!m_bFirstFlag && !pDSA->IsRunFilter() && pDSA->IsCanPreview())
	{
		pDSA->RunFilter();
	}
	return;
}


//////////////////////////////////////////////////
//ビデオ構成ボタン (ダイアログを表示する)
//////////////////////////////////////////////////
void CInputPropertyDialog::OnBnClickedVideoSettingButton()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	if (pDSA->GetWDMFilterID(WDM_FILTER_CROSS_BAR1) == -1 && 
		pDSA->GetWDMFilterID(WDM_FILTER_CROSS_BAR2) == -1 &&
		pDSA->GetWDMFilterID(WDM_FILTER_TV_AUDIO) == -1 &&
		pDSA->GetWDMFilterID(WDM_FILTER_TV_TUNER) == -1) {
			pDSA->ShowPropertySetting(TRUE,WDM_FILTER_NONE,this->GetSafeHwnd());
			return;
	}

	//WDMデバイスフィルターが存在するので、別の方法(メニュー表示型)で表示
	CMenu cMenu;
	cMenu.LoadMenu(IDR_WDM_POPUP);
	CMenu* pPopup = cMenu.GetSubMenu(0);
	assert(pPopup);
	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow();

	if (pDSA->GetWDMFilterID(WDM_FILTER_CROSS_BAR1) == -1)
		pPopup->EnableMenuItem(ID_WDM_CROSSBAR1,MF_GRAYED | MF_BYCOMMAND);
	if (pDSA->GetWDMFilterID(WDM_FILTER_CROSS_BAR2) == -1)
		pPopup->EnableMenuItem(ID_WDM_CROSSBAR2,MF_GRAYED | MF_BYCOMMAND);
	if (pDSA->GetWDMFilterID(WDM_FILTER_TV_AUDIO) == -1)
		pPopup->EnableMenuItem(ID_WDM_TVAUDIO,MF_GRAYED | MF_BYCOMMAND);
	if (pDSA->GetWDMFilterID(WDM_FILTER_TV_TUNER) == -1)
		pPopup->EnableMenuItem(ID_WDM_TVTUNER,MF_GRAYED | MF_BYCOMMAND);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,this);
	cMenu.DestroyMenu();
}
//////////////////////////////////////////////////
//ビデオ構成メニュー (ダイアログを表示する)
//////////////////////////////////////////////////
void CInputPropertyDialog::OnWDMVideoSettingMenu(ENUM_WDM_FILTER e)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	pDSA->ShowPropertySetting(TRUE,e,this->GetSafeHwnd());
}

//////////////////////////////////////////////////
//オーディオ構成ボタン (ダイアログを表示する)
//////////////////////////////////////////////////
void CInputPropertyDialog::OnBnClickedAudioSettingButton()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	pDSA->ShowPropertySetting(FALSE,WDM_FILTER_NONE,this->GetSafeHwnd());
}

//////////////////////////////////////////////////
//プロファイルの変更通知
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::ChangeProfileNotify()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	//フィルタの一時停止
	pDSA->StopFilter();

	m_bFirstFlag = TRUE;	//初期設定開始
	LoadProfileSetting();
	m_bFirstFlag = FALSE;	//初期設定終了

	//フィルタの再開(これは、PreviewWindowが行う処理である)
	//pDSA->RunFilter();
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルからのロード
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::LoadProfileSetting()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	CComboBox* combo;

	//ビデオキャプチャデバイス
	combo = (CComboBox*)GetDlgItem(IDC_VIDEO_SETTING_COMBO); assert(combo);
	if (combo->IsWindowEnabled())
	{
		if (!pProfile->m_strVideoName.IsEmpty())
		{
			for (int i=0;i<(int)pDSA->m_strVideoNameList.size();i++)
			{
				if (pProfile->m_strVideoName.Compare(pDSA->m_strVideoNameList[i].c_str()) == 0)
				{
					combo->SetCurSel(i);
					break;
				}
			}
		}
		if (combo->GetCurSel() == CB_ERR)
			combo->SetCurSel(0);

		//ビデオコンボボックスを選択モジュールを呼び出す
		OnCbnSelchangeVideoSettingCombo();

		//前回のビデオ幅・高さ・フレームレートの値を復元
		if (!pProfile->m_strVideoName.IsEmpty() && pProfile->m_nVideoSettingIndex != -1 && 
			pProfile->m_nVideoSettingWidth != 0 && pProfile->m_nVideoSettingHeight != 0 && pProfile->m_dVideoSettingFPS != 0.0)
		{
			SetVideoCurrentInput(pProfile->m_nVideoSettingWidth,
				pProfile->m_nVideoSettingHeight,pProfile->m_dVideoSettingFPS);
			//ビデオ適用コンボボックスを選択モジュールを呼び出す
			OnBnClickedVideoResolutionButton();
		}
	}

	//オーディオキャプチャデバイス
	//TODO 共有モードの際にここにバグがあるが、とりあえず後回しで他の実装を優先
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_SETTING_COMBO); assert(combo);
	if (combo->IsWindowEnabled())
	{
		if (!pProfile->m_strAudioName.IsEmpty())
		{
			for (int i=0;i<(int)pDSA->m_strAudioNameList.size();i++)
			{
				if (pProfile->m_strAudioName.Compare(pDSA->m_strAudioNameList[i].c_str()) == 0)
				{
					combo->SetCurSel(i);
					break;
				}
			}
		}
		//共有モード
		if (pProfile->m_bAudioInputVideoPin) {
			combo->SetCurSel(combo->GetCount() -1);	//最後尾を選択
		}
		if (combo->GetCurSel() == CB_ERR)
			combo->SetCurSel(0);

		//オーディオコンボボックスを選択モジュールを呼び出す
		OnCbnSelchangeAudioSettingCombo();

		//前回の値を復元(周波数・分解能・チャンネル)
		if (!pProfile->m_strAudioName.IsEmpty() && pProfile->m_nAudioSettingIndex != -1 && 
			pProfile->m_nSamplesPerSec > 0 && pProfile->m_wBitsPerSample > 0 && pProfile->m_nChannels > 0)
		{
			SetAudioCurrentInput(pProfile->m_nSamplesPerSec,
				pProfile->m_wBitsPerSample,pProfile->m_nChannels);
			//オーディオ適用コンボボックスを選択モジュールを呼び出す
			OnBnClickedAudioResolutionButton();
		}
	}
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルへのセーブ
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::SaveProfileSetting()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	CComboBox *combo1,*combo2;

	//ビデオキャプチャデバイス
	combo1 = (CComboBox*)GetDlgItem(IDC_VIDEO_SETTING_COMBO);
	combo2 = (CComboBox*)GetDlgItem(IDC_VIDEO_RESOLUTION_COMBO);
	if (combo1->IsWindowEnabled() && combo2->IsWindowEnabled() && combo1->GetCurSel() != CB_ERR && combo2->GetCurSel() != CB_ERR) {
		combo1->GetLBText(combo1->GetCurSel(),pProfile->m_strVideoName);
		combo2->GetLBText(combo2->GetCurSel(),pProfile->m_strVideoSettingName);
		pProfile->m_nVideoSettingIndex = combo2->GetCurSel();
	} else {
		pProfile->m_strVideoName = _T("SCFH DSF");
		pProfile->m_strVideoSettingName = _T("");
		pProfile->m_nVideoSettingIndex = -1;
	}
	//ビデオ解像度(幅・高さ・フレームレート)
	int nWidth , nHeight;
	double dFPS;
	GetVideoCurrentInput(nWidth,nHeight,dFPS);
	if (nWidth != 0 && nHeight != 0 && dFPS != 0.0) {
		pProfile->m_nVideoSettingWidth = nWidth;
		pProfile->m_nVideoSettingHeight = nHeight;
		pProfile->m_dVideoSettingFPS = dFPS;
	} else {
		pProfile->m_nVideoSettingWidth = -1;
		pProfile->m_nVideoSettingHeight = -1;
		pProfile->m_dVideoSettingFPS = 0.0;
	}

	//オーディオキャプチャデバイス
	combo1 = (CComboBox*)GetDlgItem(IDC_AUDIO_SETTING_COMBO);
	combo2 = (CComboBox*)GetDlgItem(IDC_AUDIO_RESOLUTION_COMBO);
	if (combo1->IsWindowEnabled() && combo2->IsWindowEnabled()) {
		combo1->GetLBText(combo1->GetCurSel(),pProfile->m_strAudioName);
		combo2->GetLBText(combo2->GetCurSel(),pProfile->m_strAudioSettingName);
		pProfile->m_nAudioSettingIndex = combo2->GetCurSel();
		if (combo1->GetItemData((combo1->GetCurSel())) == SHARE_VIDEO_AUDIO)
			pProfile->m_bAudioInputVideoPin = TRUE;
		else 
			pProfile->m_bAudioInputVideoPin = FALSE;
	} else {
		pProfile->m_strAudioName = _T("");
		pProfile->m_strAudioSettingName = _T("");
		pProfile->m_nAudioSettingIndex = -1;
		pProfile->m_bAudioInputVideoPin = FALSE;
	}
	//オーディオ設定(周波数・分解能・チャンネル)
	DWORD nSamplesPerSec;
	WORD wBitsPerSample,nChannels;
	GetAudioCurrentInput(nSamplesPerSec,wBitsPerSample,nChannels);
	if (nSamplesPerSec != 0 && wBitsPerSample != 0 && nChannels != 0)
	{
		pProfile->m_nSamplesPerSec = nSamplesPerSec;
		pProfile->m_wBitsPerSample = wBitsPerSample;
		pProfile->m_nChannels = nChannels;
	} else {
		pProfile->m_nSamplesPerSec = 44100;
		pProfile->m_wBitsPerSample = 16;
		pProfile->m_nChannels = 2;
	}
	return TRUE;
}
//////////////////////////////////////////////////
//エンコードの開始と停止
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::Encode(BOOL bStart)
{
	//ビデオ解像度周りのOn/Off
	((CWnd*)GetDlgItem(IDC_VIDEO_SETTING_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_VIDEO_RESOLUTION_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	if (bStart)
	{
		GetDlgItem(IDC_VIDEO_WIDTH_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_WIDTH_SPIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_HEIGHT_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_HEIGHT_SPIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_FRAMERATE_SPIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIDEO_RESOLUTION_BUTTON)->EnableWindow(FALSE);
	} else
	{
		CSpinButtonCtrl* spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_WIDTH_SPIN); assert(spin);
		int nMin,nMax;
		spin->GetRange32(nMin,nMax);
		if (nMin != nMax) {
			GetDlgItem(IDC_VIDEO_WIDTH_EDIT)->EnableWindow(TRUE);
			spin->EnableWindow(TRUE);
		}
		spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_HEIGHT_SPIN); assert(spin);
		spin->GetRange32(nMin,nMax);
		if (nMin != nMax) {
			GetDlgItem(IDC_VIDEO_HEIGHT_EDIT)->EnableWindow(TRUE);
			spin->EnableWindow(TRUE);
		}
		spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SPIN); assert(spin);
		spin->GetRange32(nMin,nMax);
		if (nMin != nMax) {
			GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT)->EnableWindow(TRUE);
			spin->EnableWindow(TRUE);
		}
		if (GetDlgItem(IDC_VIDEO_WIDTH_EDIT)->IsWindowEnabled() ||
			GetDlgItem(IDC_VIDEO_HEIGHT_EDIT)->IsWindowEnabled() ||
			GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT)->IsWindowEnabled())
			GetDlgItem(IDC_VIDEO_RESOLUTION_BUTTON)->EnableWindow(TRUE);
	}

	//オーディオ周りのOn/Off
	((CWnd*)GetDlgItem(IDC_AUDIO_SETTING_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_AUDIO_RESOLUTION_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	if (bStart)
	{
		GetDlgItem(IDC_AUDIO_FREQ_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUDIO_BITS_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUDIO_CHANNELS_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUDIO_RESOLUTION_BUTTON)->EnableWindow(FALSE);
	} else {
		//設定がひとつしかない場合はOFFのままでいい
		CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_FREQ_COMBO); assert(combo);
		if (combo->GetCount() > 1)
			combo->EnableWindow(TRUE);
		combo = (CComboBox*)GetDlgItem(IDC_AUDIO_BITS_COMBO); assert(combo);
		if (combo->GetCount() > 1)
			combo->EnableWindow(TRUE);
		combo = (CComboBox*)GetDlgItem(IDC_AUDIO_CHANNELS_COMBO); assert(combo);
		if (combo->GetCount() > 1)
			combo->EnableWindow(TRUE);

		if (GetDlgItem(IDC_AUDIO_FREQ_COMBO)->IsWindowEnabled() ||
			GetDlgItem(IDC_AUDIO_BITS_COMBO)->IsWindowEnabled() ||
			GetDlgItem(IDC_AUDIO_CHANNELS_COMBO)->IsWindowEnabled())
			GetDlgItem(IDC_AUDIO_RESOLUTION_BUTTON)->EnableWindow(TRUE);
	}


	((CWnd*)GetDlgItem(IDC_INPUT_CONFIG_BUTTON))->EnableWindow(bStart ? FALSE : TRUE);

	return TRUE;
}


//////////////////////////////////////////////////
//ビデオデバイスの操作に失敗した場合のサブルーチン
//int nLevel	0:ビデオデバイスの検出に失敗(一つも見つからなかった)
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::VideoDeviceFailed(int nLevel)
{
	CComboBox* combo;
	CWnd* wnd;
	if (nLevel <= 0) {
		combo = (CComboBox*)GetDlgItem(IDC_VIDEO_SETTING_COMBO); assert(combo);
		combo->ResetContent();
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
	}
	if (nLevel <= 1) {
		wnd = GetDlgItem(IDC_VIDEO_SETTING_BUTTON); assert(wnd);
		wnd->EnableWindow(FALSE);
	}
	if (nLevel <= 2) {
		combo = (CComboBox*)GetDlgItem(IDC_VIDEO_RESOLUTION_COMBO); assert(combo);
		combo->ResetContent();
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
	}
	if (nLevel <= 3) {
		wnd = GetDlgItem(IDC_VIDEO_WIDTH_EDIT); assert(wnd);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_WIDTH_SPIN); assert(wnd);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_HEIGHT_EDIT); assert(wnd);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_HEIGHT_SPIN); assert(wnd);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT); assert(wnd);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_FRAMERATE_SPIN); assert(wnd);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_RESOLUTION_BUTTON); assert(wnd);
		wnd->EnableWindow(FALSE);
	}
	return TRUE;
}

//////////////////////////////////////////////////
//現在のビデオ入力値(幅・高さ・フレームレート)を得るサブルーチン
//////////////////////////////////////////////////
void CInputPropertyDialog::GetVideoCurrentInput(int &nWidth,int &nHeight,double &dFPS)
{
	//ビデオ解像度(幅・高さ・フレームレート)
	nWidth = 0;
	nHeight = 0;
	dFPS = 0.0;
	CString strNum;
	CEdit* edit = (CEdit*)GetDlgItem(IDC_VIDEO_WIDTH_EDIT);
	if (!edit) return;
	edit->GetWindowText(strNum);
	nWidth = _tstoi(strNum);
	edit = (CEdit*)GetDlgItem(IDC_VIDEO_HEIGHT_EDIT);
	if (!edit) return;
	edit->GetWindowText(strNum);
	nHeight = _tstoi(strNum);
	edit = (CEdit*)GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT);
	if (!edit) return;
	edit->GetWindowText(strNum);
	dFPS = _tstof(strNum);
	return;
}

//////////////////////////////////////////////////
//ビデオ入力値を設定 (値を表示するだけで適用はしない)
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::SetVideoCurrentInput(int nWidth, int nHeight, double dFPS)
{
	CString strNum;
	CEdit* edit = (CEdit*)GetDlgItem(IDC_VIDEO_WIDTH_EDIT);
	if (!edit) return FALSE;
	strNum.Format(_T("%d"),nWidth);
	edit->SetWindowText(strNum);
	edit = (CEdit*)GetDlgItem(IDC_VIDEO_HEIGHT_EDIT);
	if (!edit) return FALSE;
	strNum.Format(_T("%d"),nHeight);
	edit->SetWindowText(strNum);
	edit = (CEdit*)GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT);
	if (!edit) return FALSE;
	strNum.Format(_T("%fd"),dFPS);
	edit->SetWindowText(strNum);
	return TRUE;
}

//////////////////////////////////////////////////
//オーディオ入力値を取得
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::GetAudioCurrentInput(DWORD &nSamplesPerSec,WORD &wBitsPerSample,WORD &nChannels)
{
	nSamplesPerSec = 0;
	wBitsPerSample = 0;
	nChannels = 0;
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_FREQ_COMBO); assert(combo);
	if (combo->GetCurSel() != CB_ERR)
		nSamplesPerSec = (DWORD)combo->GetItemData(combo->GetCurSel());
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_BITS_COMBO); assert(combo);
	if (combo->GetCurSel() != CB_ERR)
		wBitsPerSample = (WORD)combo->GetItemData(combo->GetCurSel());
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_CHANNELS_COMBO); assert(combo);
	if (combo->GetCurSel() != CB_ERR)
		nChannels = (WORD)combo->GetItemData(combo->GetCurSel());
	return TRUE;
}

//////////////////////////////////////////////////
//オーディオ入力値を設定 (値を表示するだけで適用はしない)
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::SetAudioCurrentInput(DWORD nSamplesPerSec,WORD wBitsPerSample,WORD nChannels)
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_FREQ_COMBO);
	if (!combo) return FALSE;
	for (int i=0;i<combo->GetCount();i++) {
		if ((DWORD)combo->GetItemData(i) == nSamplesPerSec) {
			combo->SetCurSel(i);
			break;
		}
	}
	if (combo->GetCurSel() == CB_ERR)
		combo->SetCurSel(0);
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_BITS_COMBO);
	if (!combo) return FALSE;
	for (int i=0;i<combo->GetCount();i++) {
		if ((WORD)combo->GetItemData(i) == wBitsPerSample) {
			combo->SetCurSel(i);
			break;
		}
	}
	if (combo->GetCurSel() == CB_ERR)
		combo->SetCurSel(0);
	combo = (CComboBox*)GetDlgItem(IDC_AUDIO_CHANNELS_COMBO);
	if (!combo) return FALSE;
	for (int i=0;i<combo->GetCount();i++) {
		if ((WORD)combo->GetItemData(i) == nChannels) {
			combo->SetCurSel(i);
			break;
		}
	}
	if (combo->GetCurSel() == CB_ERR)
		combo->SetCurSel(0);
	return TRUE;
}

//////////////////////////////////////////////////
//入力の詳細設定ボタン (入力の詳細設定ダイアログを表示)
//////////////////////////////////////////////////
void CInputPropertyDialog::OnBnClickedInputConfigButton()
{
	CInputPropertyConfigDialog dlg;
	dlg.DoModal();
}

//////////////////////////////////////////////////
//ビデオフィルターのオーディオピンの入力の有無に関するサブルーチン
//////////////////////////////////////////////////
void CInputPropertyDialog::ProcVideoAudioInputPin()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_SETTING_COMBO); assert(combo);

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	//ビデオピンの有無
	BOOL bFindAudioPin = pDSA->IsVidoFindAudioPin();

	//現在のカーソル位置インデックスを取得
	int nNowComboIndex = 0;
	if (combo->GetCurSel() != CB_ERR)
		nNowComboIndex = (int)combo->GetItemData(combo->GetCurSel());

	if (nNowComboIndex == SHARE_VIDEO_AUDIO) {	//オーディオ設定入力が共有モードになっている
		if (bFindAudioPin) {
			//現在のオーディオキャプチャで、共有モードにてフィルタ再構築を行う
			OnCbnSelchangeAudioSettingCombo();
		} else {
			//非共有モードになる(選択オーディオデバイスはとりあえず0番目)
			combo->DeleteString(combo->GetCount() -1);
			if (combo->GetCurSel() == CB_ERR)
				combo->SetCurSel(0);
			//0番目のオーディオデバイスを選択する
			OnCbnSelchangeAudioSettingCombo();
		}
	} else {		//オーディオ設定入力が共有モードになっている
		BOOL bInsertComboIndex = FALSE;
		for (int i=0;i<combo->GetCount();i++) {
			if ( (int)combo->GetItemData(i) == SHARE_VIDEO_AUDIO )
				bInsertComboIndex = TRUE;
		}
		//コンボボックスの中を編集するのみに留める
		if (bFindAudioPin) {
			if (!bInsertComboIndex) {
				int nInsertIndex = combo->AddString(_T("ビデオキャプチャデバイスのオーディオ"));
				combo->SetItemData(nInsertIndex,(DWORD_PTR)SHARE_VIDEO_AUDIO);
			}
		} else {
			if (bInsertComboIndex) {
				combo->DeleteString(combo->GetCount() -1);
			}
		}
	}
	return;
}


//////////////////////////////////////////////////
//デバイス通知のコールバックメッセージ
//TODO ここを後で書く
//////////////////////////////////////////////////
BOOL CInputPropertyDialog::DeviceChange()
{
	/*
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	BOOL bRet = TRUE;		//戻り値が正常であったかどうか
	BOOL bChange = FALSE;	//変更があったかどうかの有無
	vector<wstring> strNameList;
	bRet = pDSA->ReScanDevice(TRUE,bChange,strNameList);
	if (bChange) {
		TRACE0("ビデオデバイスが検出されました\n");
		for (int i=0;i<(int)strNameList.size();i++) {
			TRACE2("Video[%d]:%s\n",i,strNameList[i].c_str());
		}
	}
	bRet = pDSA->ReScanDevice(FALSE,bChange,strNameList);
	if (bChange) {
		TRACE0("オーディオデバイスが検出されました\n");
		for (int i=0;i<(int)strNameList.size();i++) {
			TRACE2("Audio[%d]:%s\n",i,strNameList[i].c_str());
		}
	}
	*/
	/*
	デバイスの再検出
	検出前のデバイス一覧と比較して差があるかどうか調べる
	差があった場合(①と②に分かれる)
	①デバイスが追加された場合
		そのまま追記でコンボボックスリストを書き換える
		コンボボックスのインデックスも書き換わるために注意が必要
	②デバイスが削除された場合
		場合によっては今使ってるデバイスが削除される場合がある(この場合はエラーを出そう)
	あと、エンコード中の場合には、さらにデバイスの検出は後回しにする仕組みにしよう
	*/
	return TRUE;
}


