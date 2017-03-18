// EncodePropertyDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "EncodePropertyDialog.h"


// CEncodePropertyDialog ダイアログ

IMPLEMENT_DYNAMIC(CEncodePropertyDialog, CSizeDialog)


BEGIN_MESSAGE_MAP(CEncodePropertyDialog, CSizeDialog)
	ON_CBN_SELCHANGE(IDC_VIDEO_ENCODER_COMBO, &CEncodePropertyDialog::OnCbnSelchangeVideoEncoderCombo)
	ON_CBN_SELCHANGE(IDC_AUDIO_ENCODER_COMBO, &CEncodePropertyDialog::OnCbnSelchangeAudioEncoderCombo)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_AUDIO_ENCODER_BITRATE_COMBO, &CEncodePropertyDialog::OnCbnSelchangeAudioEncoderBitrateCombo)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SIZE_AUTO_RADIO, &CEncodePropertyDialog::OnBnClickedSizeAutoRadio)
	ON_BN_CLICKED(IDC_SIZE_MANUAL_RADIO, &CEncodePropertyDialog::OnBnClickedSizeAutoRadio)
	ON_EN_CHANGE(IDC_VIDEO_QUALITY_EDIT, &CEncodePropertyDialog::OnEnChangeVideoQualityEdit)
	ON_EN_CHANGE(IDC_VIDEO_FRAMERATE_EDIT, &CEncodePropertyDialog::OnEnChangeVideoFramerateEdit)
	//ON_WM_SIZE()
	//ON_WM_VSCROLL()
	ON_CBN_SELCHANGE(IDC_AUDIO_ENCODER_SAMPLESPERSEC_COMBO, &CEncodePropertyDialog::OnCbnSelchangeAudioEncoderSamplespersecCombo)
	ON_CBN_SELCHANGE(IDC_AUDIO_ENCODER_BITSPERSAMPLE_COMBO, &CEncodePropertyDialog::OnCbnSelchangeAudioEncoderBitspersampleCombo)
	ON_CBN_SELCHANGE(IDC_AUDIO_ENCODER_CHANNELS_COMBO, &CEncodePropertyDialog::OnCbnSelchangeAudioEncoderChannelsCombo)
	ON_CBN_SELCHANGE(IDC_AUDIO_ENCODER_AVSYNCHRONIZATION, &CEncodePropertyDialog::OnCbnSelchangeAudioEncoderAvsynchronization)
END_MESSAGE_MAP()


CEncodePropertyDialog::CEncodePropertyDialog(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CEncodePropertyDialog::IDD, pParent)
{
	m_bFirstFlag = FALSE; 
	//m_strAudioBitrate = _T("");
	m_dwBitrate = 0;
	m_nSamplesPerSec = 0;
	m_wBitsPerSample = 0;
	m_nChannels = 0;
	m_bAVSynchronization = FALSE;
}

CEncodePropertyDialog::~CEncodePropertyDialog()
{
}

void CEncodePropertyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}




// CEncodePropertyDialog メッセージ ハンドラ

////////////////////////////////////////
void CEncodePropertyDialog::OnOK()
{
	//CDialog::OnOK();
}

////////////////////////////////////////
void CEncodePropertyDialog::OnCancel()
{
	//CDialog::OnCancel();
}

////////////////////////////////////////
//ダイアログ初期化
////////////////////////////////////////
BOOL CEncodePropertyDialog::OnInitDialog()
{
	CSizeDialog::OnInitDialog();
	m_bFirstFlag = TRUE;	//初期化フラグ有効

	//GetWindowRect(m_rect);
	//m_nScrollPos = 0;


	m_ToolTip.Create(this);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_VIDEO_ENCODER_COMBO),IDC_VIDEO_ENCODER_COMBO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_VIDEO_BITRATE_EDIT),IDC_VIDEO_BITRATE_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT),IDC_VIDEO_FRAMERATE_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_SIZE_AUTO_RADIO),IDC_SIZE_AUTO_RADIO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_SIZE_MANUAL_RADIO),IDC_SIZE_MANUAL_RADIO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_VIDEO_WIDTH_EDIT),IDC_VIDEO_WIDTH_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_VIDEO_HEIGHT_EDIT),IDC_VIDEO_HEIGHT_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_VIDEO_QUALITY_EDIT),IDC_VIDEO_QUALITY_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_BUFFER_WINDOW_EDIT),IDC_BUFFER_WINDOW_EDIT);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_AUDIO_ENCODER_COMBO),IDC_AUDIO_ENCODER_COMBO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_AUDIO_ENCODER_BITRATE_COMBO),IDC_AUDIO_ENCODER_BITRATE_COMBO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_AUDIO_ENCODER_SAMPLESPERSEC_COMBO),IDC_AUDIO_ENCODER_SAMPLESPERSEC_COMBO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_AUDIO_ENCODER_BITSPERSAMPLE_COMBO),IDC_AUDIO_ENCODER_BITSPERSAMPLE_COMBO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_AUDIO_ENCODER_CHANNELS_COMBO),IDC_AUDIO_ENCODER_CHANNELS_COMBO);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_AUDIO_ENCODER_AVSYNCHRONIZATION),IDC_AUDIO_ENCODER_AVSYNCHRONIZATION);

	m_ToolTip.SetMaxTipWidth(300);
	DWORD dwAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP,dwAutoPop*2);

	CDirectShowAccess *pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	CString strNum;

	//ビデオエンコードコンボボックスの設定
	CComboBox* combo = (CComboBox*)this->GetDlgItem(IDC_VIDEO_ENCODER_COMBO); assert(combo);
	if (pDSA->m_VideoCodecList.size() != 0)
	{
		for (int i=0;i<(int)pDSA->m_VideoCodecList.size();i++) {			//エンコードアイテム名の追加
			combo->AddString(pDSA->m_VideoCodecList[i].strName.c_str());
		}
		if (!pProfile->m_strVideoEncodeName.IsEmpty())	//INIの設定有り
		{
			for (int i=0;i<(int)pDSA->m_VideoCodecList.size();i++)
			{
				if (pProfile->m_strVideoEncodeName.Compare(pDSA->m_VideoCodecList[i].strName.c_str()) == 0)
				{
					combo->SetCurSel(i);
					break;
				}
			}
		}
		if (combo->GetCurSel() == CB_ERR){
			combo->SetCurSel(0);	//0番目の選択肢をとる
		}
		//ビデオエンコーダーの設定を変更する
		OnCbnSelchangeVideoEncoderCombo();
	} else {	//エンコーダーの設定がない(とても稀)
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
		AfxMessageBox(_T("インストールされているビデオエンコーダーが見つかりませんでした"),MB_OK|MB_ICONINFORMATION);
	}

	//ビデオビットレートの設定(単位はKbps)
	CSpinButtonCtrl* spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_BITRATE_SPIN); assert(spin);
	spin->SetRange32(4,135000);	//入力設定の最大は135Mbps
	spin->SetPos32(pProfile->m_nVideoEnocdeBitrate);
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_BITRATE_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEnocdeBitrate);
	edit->SetWindowText(strNum);

	//フレームレートの設定
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SPIN); assert(spin);
	spin->SetRange(1,(int)pProfile->m_dVideoEncodeMaxFramerateSetting);
	spin->SetPos32((int)pProfile->m_dVideoEncodeFramerate);		//スピンボタンから設定する
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT); assert(edit);
	if (((int)(pProfile->m_dVideoEncodeFramerate*100)) % 10 != 0)
		strNum.Format(_T("%.2f"),pProfile->m_dVideoEncodeFramerate);
	else if (((int)(pProfile->m_dVideoEncodeFramerate*10)) % 10 != 0)
		strNum.Format(_T("%.1f"),pProfile->m_dVideoEncodeFramerate);
	else
		strNum.Format(_T("%.0f"),pProfile->m_dVideoEncodeFramerate);
	edit->SetWindowText(strNum);
	/*
	CSliderCtrl* slider = (CSliderCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SLIDER); assert(slider);
	slider->SetTic(10);
	slider->SetTicFreq(200);
	slider->SetPageSize(100);
	slider->SetRange(1,(int)(pIni->m_dVideoEncodeMaxFramerateSetting*100));
	slider->SetPos((int)(pIni->m_dVideoEncodeFramerate*100));
	*/

	//サイズの変更
	CButton* button1 = (CButton*)this->GetDlgItem(IDC_SIZE_AUTO_RADIO); assert(button1);
	CButton* button2 = (CButton*)this->GetDlgItem(IDC_SIZE_MANUAL_RADIO); assert(button2);
	if (pProfile->m_bVideoEncodeSize)	//自動無効(サイズ変更有り)
	{
		button1->SetCheck(BST_UNCHECKED);
		button2->SetCheck(BST_CHECKED);
	} else {	//自動無効(サイズ変更無し)
		button1->SetCheck(BST_CHECKED);
		button2->SetCheck(BST_UNCHECKED);
	}
	OnBnClickedSizeAutoRadio();

	//ビデオの幅と高さ
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_WIDTH_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEncodeWidth);
	edit->SetWindowText(strNum);
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_HEIGHT_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEncodeHeight);
	edit->SetWindowText(strNum);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_WIDTH_SPIN); assert(spin);
	spin->SetRange32(1,9999);
	spin->SetPos32(pProfile->m_nVideoEncodeWidth);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_HEIGHT_SPIN); assert(spin);
	spin->SetRange32(1,9999);
	spin->SetPos32(pProfile->m_nVideoEncodeHeight);

	//ビデオの滑らかさ
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_QUALITY_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEnocdeQuality);
	edit->SetWindowText(strNum);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_QUALITY_SPIN); assert(spin);
	spin->SetRange32(0,100);
	OnEnChangeVideoQualityEdit();	//スタティックテキストを変更
	/*
	slider = (CSliderCtrl*)this->GetDlgItem(IDC_VIDEO_QUALITY_SLIDER); assert(slider);
	slider->SetTic(10);
	slider->SetTicFreq(10);
	slider->SetPageSize(10);
	slider->SetRange(0,100);
	slider->SetPos(pIni->m_nVideoEnocdeQuality);
	*/

	//バッファサイズ
	edit = (CEdit*)this->GetDlgItem(IDC_BUFFER_WINDOW_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nBufferWindow/1000);
	edit->SetWindowText(strNum);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_BUFFER_WINDOW_SPIN); assert(spin);
	spin->SetRange32(1,30);
	spin->SetPos32((pProfile->m_nBufferWindow)/1000);

	//オーディオエンコードコンボボックスの設定
	combo = (CComboBox*)this->GetDlgItem(IDC_AUDIO_ENCODER_COMBO); assert(combo);
	if (pDSA->m_AudioCodecList.size() != 0)
	{
		for (int i=0;i<(int)pDSA->m_AudioCodecList.size();i++) {
			combo->AddString(pDSA->m_AudioCodecList[i].strName.c_str());
		}
		if (!pProfile->m_strAudioEncodeName.IsEmpty()) {
			for (int i=0;i<(int)pDSA->m_AudioCodecList.size();i++)
			{
				if (pProfile->m_strAudioEncodeName.Compare(pDSA->m_AudioCodecList[i].strName.c_str()) == 0)
				{
					combo->SetCurSel(i);
					break;
				}
			}
		}
		if (combo->GetCurSel() == CB_ERR){
			combo->SetCurSel(0);	//0番目の選択肢をとる
		}
		//オーディオエンコーダーの詳細の読み込み
		m_dwBitrate = pProfile->m_nAudioEncodeBitrate;
		m_nSamplesPerSec = pProfile->m_nAudioEncodeSamplesPerSec;
		m_wBitsPerSample = pProfile->m_nAudioEncodeBitsPerSample;
		m_nChannels = pProfile->m_nAudioEncodeChannels;
		m_bAVSynchronization = pProfile->m_bAudioEncodeAVSynchronization;

		//オーディオエンコーダー設定コンボボックスを変更
		OnCbnSelchangeAudioEncoderCombo();
	} else {	//エンコーダーの設定がない(とても稀)
		combo->AddString(_T("[なし]"));
		combo->SetCurSel(0);
		combo->EnableWindow(FALSE);
		AfxMessageBox(_T("インストールされているオーディオエンコーダーが見つかりませんでした"),MB_OK|MB_ICONINFORMATION);
	}

	m_bFirstFlag = FALSE;	//初期化フラグ終了
	return TRUE;
}
////////////////////////////////////////
//ダイアログが終了したときに呼ばれるイベントハンドラ
////////////////////////////////////////
void CEncodePropertyDialog::OnDestroy()
{
	SaveProfileSetting();
	CDialog::OnDestroy();
}
////////////////////////////////////////
//ビデオエンコーダーの変更
////////////////////////////////////////
void CEncodePropertyDialog::OnCbnSelchangeVideoEncoderCombo()
{
	CDirectShowAccess *pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CComboBox* combo = (CComboBox*)this->GetDlgItem(IDC_VIDEO_ENCODER_COMBO); assert(combo);
	int nIndex = combo->GetCurSel();
	if (!pDSA->SelectVideoCodecIndex(nIndex)) {
		AfxMessageBox(_T("ビデオエンコーダーの設定変更に失敗しました"));
		return;
	}
}

////////////////////////////////////////
//オーディオエンコーダーの変更
////////////////////////////////////////
void CEncodePropertyDialog::OnCbnSelchangeAudioEncoderCombo()
{
	//ビットレート設定の項目を編集する
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_COMBO); assert(combo);
	int nIndex = combo->GetCurSel();	//インデックス
	if (nIndex == CB_ERR)
		return;
	assert(nIndex >= 0 && nIndex < (int)pDSA->m_AudioCodecList.size());
	if (!pDSA->SelectAudioCodecIndex(nIndex))
	{
		AfxMessageBox(_T("オーディオコーデックの変更に失敗しました。"));
		return;
	}

	//フォーマットの一覧を取得
	vector<DWORD> nSamplesPerSecList;
	vector<WORD> wBitsPerSampleList;
	vector<WORD> nChannelsList;
	vector<BOOL> bAVSynchronizationList;
	for (int i=0;i<(int)pDSA->m_AudioFormatList.size();i++)
	{
		//周波数
		BOOL bFind = FALSE;
		for (int j=0;j<(int)nSamplesPerSecList.size();j++){
			if (nSamplesPerSecList[j] == pDSA->m_AudioFormatList[i].nSamplesPerSec)
				bFind = TRUE;
		}
		if (!bFind)
			nSamplesPerSecList.push_back(pDSA->m_AudioFormatList[i].nSamplesPerSec);
		//分解能
		bFind = FALSE;
		for (int j=0;j<(int)wBitsPerSampleList.size();j++){
			if (wBitsPerSampleList[j] == pDSA->m_AudioFormatList[i].wBitsPerSample)
				bFind = TRUE;
		}
		if (!bFind)
			wBitsPerSampleList.push_back(pDSA->m_AudioFormatList[i].wBitsPerSample);
		//チャンネル
		bFind = FALSE;
		for (int j=0;j<(int)nChannelsList.size();j++){
			if (nChannelsList[j] == pDSA->m_AudioFormatList[i].nChannels)
				bFind = TRUE;
		}
		if (!bFind)
			nChannelsList.push_back(pDSA->m_AudioFormatList[i].nChannels);
		//A/V同期
		bFind = FALSE;
		for (int j=0;j<(int)bAVSynchronizationList.size();j++){
			if (bAVSynchronizationList[j] == pDSA->m_AudioFormatList[i].bAVSynchronization)
				bFind = TRUE;
		}
		if (!bFind)
			bAVSynchronizationList.push_back(pDSA->m_AudioFormatList[i].bAVSynchronization);
	}

	//フォーマット欄を初期化＆追記
	CComboBox* combo_samples = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_SAMPLESPERSEC_COMBO); assert(combo_samples);
	CComboBox* combo_bits = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_BITSPERSAMPLE_COMBO); assert(combo_bits);
	CComboBox* combo_channels = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_CHANNELS_COMBO); assert(combo_channels);
	CComboBox* combo_av = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_AVSYNCHRONIZATION); assert(combo_av);
	combo_samples->ResetContent();
	combo_bits->ResetContent();
	combo_channels->ResetContent();
	combo_av->ResetContent();
	if (!combo_samples->IsWindowEnabled()) combo_samples->EnableWindow(TRUE);
	if (!combo_bits->IsWindowEnabled()) combo_bits->EnableWindow(TRUE);
	if (!combo_channels->IsWindowEnabled()) combo_channels->EnableWindow(TRUE);
	if (!combo_av->IsWindowEnabled()) combo_av->EnableWindow(TRUE);

	CString strNum;
	int nInsertIndex;
	if (nSamplesPerSecList.size() != 0)
	{
		for (int i=0;i<(int)nSamplesPerSecList.size();i++)
		{
			//strNum.Format(_T("%.3f"),((float)nSamplesPerSecList[i]/1000.0));
			strNum.Format(_T("%d"),nSamplesPerSecList[i]);
			nInsertIndex = combo_samples->AddString(strNum);
			combo_samples->SetItemData(nInsertIndex,(DWORD_PTR)nSamplesPerSecList[i]);
			if (m_nSamplesPerSec == nSamplesPerSecList[i])
				combo_samples->SetCurSel(nInsertIndex);
		}
		if (combo_samples->GetCurSel() == CB_ERR)
			combo_samples->SetCurSel(0);
	} else {
		combo_samples->AddString(_T("なし"));
		combo_samples->SetCurSel(0);
		combo_samples->EnableWindow(FALSE);
	}
	if (wBitsPerSampleList.size() != 0)
	{
		for (int i=0;i<(int)wBitsPerSampleList.size();i++)
		{
			strNum.Format(_T("%d"),wBitsPerSampleList[i]);
			nInsertIndex = combo_bits->AddString(strNum);
			combo_bits->SetItemData(nInsertIndex,(DWORD_PTR)wBitsPerSampleList[i]);
			if (m_wBitsPerSample == wBitsPerSampleList[i])
				combo_bits->SetCurSel(nInsertIndex);
		}
		if (combo_bits->GetCurSel() == CB_ERR)
			combo_bits->SetCurSel(0);
	} else {
		combo_bits->AddString(_T("なし"));
		combo_bits->SetCurSel(0);
		combo_bits->EnableWindow(FALSE);
	}
	if (nChannelsList.size() != 0)
	{
		for (int i=0;i<(int)nChannelsList.size();i++)
		{
			if (nChannelsList[i] == 6)
				strNum = _T("5.1");
			else if (nChannelsList[i] == 8)
				strNum = _T("7.1");
			else
				strNum.Format(_T("%d"),nChannelsList[i]);
			nInsertIndex = combo_channels->AddString(strNum);
			combo_channels->SetItemData(nInsertIndex,(DWORD_PTR)nChannelsList[i]);
			if (m_nChannels == nChannelsList[i])
				combo_channels->SetCurSel(nInsertIndex);
		}
		if (combo_channels->GetCurSel() == CB_ERR)
			combo_channels->SetCurSel(0);
	} else {
		combo_channels->AddString(_T("なし"));
		combo_channels->SetCurSel(0);
		combo_channels->EnableWindow(FALSE);
	}

	if (bAVSynchronizationList.size() != 0)
	{
		for (int i=0;i<(int)bAVSynchronizationList.size();i++)
		{
			nInsertIndex = combo_av->AddString(bAVSynchronizationList[i]?_T("有効"):_T("無効"));
			combo_av->SetItemData(nInsertIndex,(DWORD_PTR)bAVSynchronizationList[i]);
			if (m_bAVSynchronization == bAVSynchronizationList[i])
				combo_av->SetCurSel(nInsertIndex);
		}
		if (combo_av->GetCurSel() == CB_ERR)
			combo_av->SetCurSel(0);
	} else {
		combo_av->AddString(_T("なし"));
		combo_av->SetCurSel(0);
		combo_av->EnableWindow(FALSE);
	}

	//ビットレートの項目を編集
	AudioEncoderSettingChange();
}

//////////////////////////////////////////////////
//オーディオエンコーダーの設定を変更した際に呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CEncodePropertyDialog::AudioEncoderSettingChange()
{
	//現在のコンボボックスの値を取得
	DWORD nSamplesPerSec = 0;
	WORD wBitsPerSample = 0;
	WORD nChannels = 0;
	BOOL bAVSynchronization = TRUE;
	CComboBox* combo_samples = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_SAMPLESPERSEC_COMBO); assert(combo_samples);
	CComboBox* combo_bits = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_BITSPERSAMPLE_COMBO); assert(combo_bits);
	CComboBox* combo_channels = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_CHANNELS_COMBO); assert(combo_channels);
	CComboBox* combo_av = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_AVSYNCHRONIZATION); assert(combo_av);
	if (combo_samples->IsWindowEnabled() && combo_samples->GetCurSel() != CB_ERR)
		nSamplesPerSec = (DWORD)combo_samples->GetItemData(combo_samples->GetCurSel());
	if (combo_bits->IsWindowEnabled() && combo_bits->GetCurSel() != CB_ERR)
		wBitsPerSample = (WORD)combo_bits->GetItemData(combo_bits->GetCurSel());
	if (combo_channels->IsWindowEnabled() && combo_channels->GetCurSel() != CB_ERR)
		nChannels = (WORD)combo_channels->GetItemData(combo_channels->GetCurSel());
	if (combo_av->IsWindowEnabled() && combo_av->GetCurSel() != CB_ERR)
		bAVSynchronization = (BOOL)combo_av->GetItemData(combo_av->GetCurSel());

	//詳細設定と適合するビットレート一覧の列挙
	CComboBox* combo_bitrate = (CComboBox*)GetDlgItem(IDC_AUDIO_ENCODER_BITRATE_COMBO); assert(combo_bitrate);
	combo_bitrate->ResetContent();
	if (!combo_bitrate->IsWindowEnabled())
		combo_bitrate->EnableWindow(TRUE);
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	int nInsertIndex;
	CString strNum;
	int nDiff = INT_MAX;
	for (int i=0;i<(int)pDSA->m_AudioFormatList.size();i++)
	{
		if (pDSA->m_AudioFormatList[i].nSamplesPerSec != nSamplesPerSec)
			continue;
		if (pDSA->m_AudioFormatList[i].wBitsPerSample != wBitsPerSample)
			continue;
		if (pDSA->m_AudioFormatList[i].nChannels != nChannels)
			continue;
		if (pDSA->m_AudioFormatList[i].bAVSynchronization != bAVSynchronization)
			continue;
		//strNum.Format(_T("%0.2f"),((float)pDSA->m_AudioFormatList[i].dwBitrate/1000));
		int nBitrate = (int) (((float)pDSA->m_AudioFormatList[i].dwBitrate/((float)1000.0))+(0.5) );
		strNum.Format(_T("%d"),nBitrate);
		nInsertIndex = combo_bitrate->AddString(strNum);
		combo_bitrate->SetItemData(nInsertIndex,(DWORD_PTR)i);	//設定インデックスを仕込む

		if (nDiff > abs((int)(m_dwBitrate - pDSA->m_AudioFormatList[i].dwBitrate)) )
		{
			combo_bitrate->SetCurSel(nInsertIndex);
			nDiff = abs((int)(m_dwBitrate - pDSA->m_AudioFormatList[i].dwBitrate));

		}
	}

	if (combo_bitrate->GetCount() != 0)
	{
		if (combo_bitrate->GetCurSel() == CB_ERR)
			combo_bitrate->SetCurSel(0);

	} else {	//該当するビットレートが無かった場合
		combo_bitrate->AddString(_T("[なし]"));
		combo_bitrate->SetCurSel(0);
		combo_bitrate->SetItemData(0,(DWORD_PTR)-1);
		combo_bitrate->EnableWindow(FALSE);
	}
	//ビットレート変更の適用
	OnCbnSelchangeAudioEncoderBitrateCombo();
	return;
}

////////////////////////////////////////
//オーディオのビットレートの変更コンボボックスを変更した時に呼ばれるイベントハンドラ
//ここでオーディオコーデックのフォーマットの改変する
////////////////////////////////////////
void CEncodePropertyDialog::OnCbnSelchangeAudioEncoderBitrateCombo()
{
	CDirectShowAccess *pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CComboBox* combo = (CComboBox*)this->GetDlgItem(IDC_AUDIO_ENCODER_BITRATE_COMBO); assert(combo);
	int nIndexData = combo->GetCurSel();
	if (nIndexData == CB_ERR)
		return;
	int nItemData = (int)combo->GetItemData(nIndexData);

	/*
	assert(nItemData >= 0 && nItemData < (int)pDSA->m_AudioFormatList.size());
	if (nItemData < 0 || nItemData >= (int)pDSA->m_AudioFormatList.size())
		return;
	*/
	if (pDSA->SelectAudioCodecFormatIndex(nItemData))
	{
		//設定値の記憶
		m_dwBitrate = pDSA->m_AudioFormatList[nItemData].dwBitrate;
		m_nSamplesPerSec = pDSA->m_AudioFormatList[nItemData].nSamplesPerSec;
		m_wBitsPerSample = pDSA->m_AudioFormatList[nItemData].wBitsPerSample;
		m_nChannels = pDSA->m_AudioFormatList[nItemData].nChannels;
		m_bAVSynchronization = pDSA->m_AudioFormatList[nItemData].bAVSynchronization;
	} else {
		//AfxMessageBox(_T("オーディオコーデックのフォーマット変更に失敗しました。"));
	}
	return;
}



////////////////////////////////////////
//スライダーの移動
////////////////////////////////////////
void CEncodePropertyDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	/*
	CSliderCtrl* slider = (CSliderCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SLIDER); assert(slider);
	if (pScrollBar->GetSafeHwnd() == slider->GetSafeHwnd())
	{
		//エディットボックスを更新
		CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT);
		assert(edit);
		double dFPS = 0;
		dFPS = ((double)slider->GetPos()) / ((double)100);
		CString strNum;
		if (((int)(dFPS*100)) % 10 != 0)
			strNum.Format(_T("%.2f"),dFPS);
		else if (((int)(dFPS*10)) % 10 != 0)
			strNum.Format(_T("%.1f"),dFPS);
		else
			strNum.Format(_T("%.0f"),dFPS);
		edit->SetWindowText(strNum);
		//本当はここにSpinを変更するコードが必要
		return;
	}
	slider = (CSliderCtrl*)this->GetDlgItem(IDC_VIDEO_QUALITY_SLIDER);
	assert(slider);
	if (pScrollBar->GetSafeHwnd() == slider->GetSafeHwnd())
	{
		//エディットボックスを更新
		CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_QUALITY_EDIT);
		assert(edit);
		CString strNum;
		strNum.Format(_T("%d"),slider->GetPos());
		edit->SetWindowText(strNum);
		OnEnChangeVideoQualityEdit();
		//本当はここにSpinを変更するコードが必要
		return;
	}
	*/
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
////////////////////////////////////////
//フレームレート テキストボックスの編集
////////////////////////////////////////
void CEncodePropertyDialog::OnEnChangeVideoFramerateEdit()
{
	/*
	CSliderCtrl* slider = (CSliderCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SLIDER);
	if (!slider) return;
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT); assert(edit);
	CString strNum;
	edit->GetWindowText(strNum);
	//値が変更された旨をスライダーコントロールに通知する
	int nPos = (int)(_tstof(strNum) * 100);
	slider->SetPos(nPos);
	*/
	return;
}
////////////////////////////////////////
//ビデオサイズの変更ボタン
////////////////////////////////////////
void CEncodePropertyDialog::OnBnClickedSizeAutoRadio()
{
	CButton* button1 = (CButton*)this->GetDlgItem(IDC_SIZE_AUTO_RADIO); assert(button1);
	CEdit* edit1 = (CEdit*)this->GetDlgItem(IDC_VIDEO_WIDTH_EDIT); assert(edit1);
	CEdit* edit2 = (CEdit*)this->GetDlgItem(IDC_VIDEO_HEIGHT_EDIT); assert(edit2);
	CSpinButtonCtrl* spin1 = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_WIDTH_SPIN); assert(spin1);
	CSpinButtonCtrl* spin2 = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_HEIGHT_SPIN); assert(spin1);

	if (button1->GetCheck() & BST_CHECKED)
	{
		edit1->EnableWindow(FALSE);
		edit2->EnableWindow(FALSE);
		spin1->EnableWindow(FALSE);
		spin2->EnableWindow(FALSE);
	} else {
		edit1->EnableWindow(TRUE);
		edit2->EnableWindow(TRUE);
		spin1->EnableWindow(TRUE);
		spin2->EnableWindow(TRUE);
	}
}
////////////////////////////////////////
//ビデオの滑らかさテキストボックスの編集
////////////////////////////////////////
void CEncodePropertyDialog::OnEnChangeVideoQualityEdit()
{
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_QUALITY_EDIT); assert(edit);
	CString strNum;
	edit->GetWindowText(strNum);
	int nVideoEncodeQuality = _tstoi(strNum);
	if (nVideoEncodeQuality >= 80) {
		this->SetDlgItemText( IDC_VIDEO_QUALITY_STATIC, _T("(最も鮮明)"));
	} else if (nVideoEncodeQuality >= 60) {
		this->SetDlgItemText( IDC_VIDEO_QUALITY_STATIC, _T("(より鮮明)"));
	} else if (nVideoEncodeQuality >= 40) {
		this->SetDlgItemText( IDC_VIDEO_QUALITY_STATIC, _T("(分散)"));
	} else if (nVideoEncodeQuality >= 20) {
		this->SetDlgItemText( IDC_VIDEO_QUALITY_STATIC, _T("(より滑らか)"));
	} else {
		this->SetDlgItemText( IDC_VIDEO_QUALITY_STATIC, _T("(最も滑らか)"));
	}
}

////////////////////////////////////////
//ツールチップの表示
////////////////////////////////////////
BOOL CEncodePropertyDialog::PreTranslateMessage(MSG* pMsg)
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
////////////////////////////////////////
//プロファイルの変更通知
////////////////////////////////////////
BOOL CEncodePropertyDialog::ChangeProfileNotify()
{
	m_bFirstFlag = TRUE;	//初期化フラグ有効
	LoadProfileSetting();
	m_bFirstFlag = FALSE;	//初期化フラグ無効
	return TRUE;
}
////////////////////////////////////////
//プロファイルからのロード
////////////////////////////////////////
BOOL CEncodePropertyDialog::LoadProfileSetting()
{
	CString strNum;
	CDirectShowAccess *pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	//ビデオエンコードコンボボックスの設定
	CComboBox* combo = (CComboBox*)this->GetDlgItem(IDC_VIDEO_ENCODER_COMBO); assert(combo);
	if (!pProfile->m_strVideoEncodeName.IsEmpty())	//INIの設定有り
	{
		for (int i=0;i<(int)pDSA->m_VideoCodecList.size();i++)
		{
			if (pProfile->m_strVideoEncodeName.Compare(pDSA->m_VideoCodecList[i].strName.c_str()) == 0)
			{
				combo->SetCurSel(i);
				break;
			}
		}
		if (combo->GetCurSel() == CB_ERR){
			combo->SetCurSel(0);	//0番目の選択肢をとる
		}
	} else {						//INIの設定が無い
		combo->SetCurSel(0);		//0番目の選択肢をとる
	}
	//ビデオビットレートの設定(単位はKbps)
	CSpinButtonCtrl* spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_BITRATE_SPIN); assert(spin);
	spin->SetPos32(pProfile->m_nVideoEnocdeBitrate);
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_BITRATE_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEnocdeBitrate);
	edit->SetWindowText(strNum);

	//フレームレートの設定
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SPIN); assert(spin);
	spin->SetRange(1,(int)pProfile->m_dVideoEncodeMaxFramerateSetting);
	spin->SetPos32((int)pProfile->m_dVideoEncodeFramerate);		//スピンボタンから設定する
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT); assert(edit);
	if (((int)(pProfile->m_dVideoEncodeFramerate*100)) % 10 != 0)
		strNum.Format(_T("%.2f"),pProfile->m_dVideoEncodeFramerate);
	else if (((int)(pProfile->m_dVideoEncodeFramerate*10)) % 10 != 0)
		strNum.Format(_T("%.1f"),pProfile->m_dVideoEncodeFramerate);
	else
		strNum.Format(_T("%.0f"),pProfile->m_dVideoEncodeFramerate);
	edit->SetWindowText(strNum);
	/*
	CSliderCtrl* slider = (CSliderCtrl*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_SLIDER); assert(slider);
	slider->SetPos((int)(pIni->m_dVideoEncodeFramerate*100));
	*/

	//サイズの変更
	CButton* button1 = (CButton*)this->GetDlgItem(IDC_SIZE_AUTO_RADIO); assert(button1);
	CButton* button2 = (CButton*)this->GetDlgItem(IDC_SIZE_MANUAL_RADIO); assert(button2);
	if (pProfile->m_bVideoEncodeSize)	//自動無効(サイズ変更有り)
	{
		button1->SetCheck(BST_UNCHECKED);
		button2->SetCheck(BST_CHECKED);
	} else {	//自動無効(サイズ変更無し)
		button1->SetCheck(BST_CHECKED);
		button2->SetCheck(BST_UNCHECKED);
	}
	OnBnClickedSizeAutoRadio();

	//ビデオの幅と高さ
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_WIDTH_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEncodeWidth);
	edit->SetWindowText(strNum);
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_HEIGHT_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEncodeHeight);
	edit->SetWindowText(strNum);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_WIDTH_SPIN); assert(spin);
	spin->SetPos32(pProfile->m_nVideoEncodeWidth);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_HEIGHT_SPIN); assert(spin);
	spin->SetPos32(pProfile->m_nVideoEncodeHeight);

	//ビデオの滑らかさ
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_QUALITY_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nVideoEnocdeQuality);
	edit->SetWindowText(strNum);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_VIDEO_QUALITY_SPIN); assert(spin);
	OnEnChangeVideoQualityEdit();	//スタティックテキストを変更

	//バッファサイズ
	edit = (CEdit*)this->GetDlgItem(IDC_BUFFER_WINDOW_EDIT); assert(edit);
	strNum.Format(_T("%d"),pProfile->m_nBufferWindow/1000);
	edit->SetWindowText(strNum);
	spin = (CSpinButtonCtrl*)this->GetDlgItem(IDC_BUFFER_WINDOW_SPIN); assert(spin);
	spin->SetPos32((pProfile->m_nBufferWindow)/1000);

	//オーディオエンコードコンボボックスの設定
	combo = (CComboBox*)this->GetDlgItem(IDC_AUDIO_ENCODER_COMBO); assert(combo);
	if (!pProfile->m_strAudioEncodeName.IsEmpty())	//INIの設定有り
	{
		for (int i=0;i<(int)pDSA->m_AudioCodecList.size();i++)
		{
			if (pProfile->m_strAudioEncodeName.Compare(pDSA->m_AudioCodecList[i].strName.c_str()) == 0)
			{
				combo->SetCurSel(i);
				break;
			}
		}
		if (combo->GetCurSel() == CB_ERR){
			combo->SetCurSel(0);	//0番目の選択肢をとる
		}

		//オーディオエンコーダーの詳細の読み込み
		m_dwBitrate = pProfile->m_nAudioEncodeBitrate;
		m_nSamplesPerSec = pProfile->m_nAudioEncodeSamplesPerSec;
		m_wBitsPerSample = pProfile->m_nAudioEncodeBitsPerSample;
		m_nChannels = pProfile->m_nAudioEncodeChannels;
		m_bAVSynchronization = pProfile->m_bAudioEncodeAVSynchronization;

		//オーディオエンコーダー設定コンボボックスを変更
		OnCbnSelchangeAudioEncoderCombo();
	} else {						//INIの設定が無い
		combo->SetCurSel(0);		//0番目の選択肢をとる
	}
	return TRUE;
}
////////////////////////////////////////
//プロファイルへのセーブ
////////////////////////////////////////
BOOL CEncodePropertyDialog::SaveProfileSetting()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	//ビデオエンコーダー設定の保存
	CComboBox* combo = (CComboBox*)this->GetDlgItem(IDC_VIDEO_ENCODER_COMBO); assert(combo);
	combo->GetLBText(combo->GetCurSel(),pProfile->m_strVideoEncodeName);
	//ビットレート設定の保存
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_BITRATE_EDIT); assert(edit);
	CString strNum;
	edit->GetWindowText(strNum);
	pProfile->m_nVideoEnocdeBitrate = _tstoi(strNum);
	//ビデオのフレームレートの保存
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT); assert(edit);
	edit->GetWindowText(strNum);
	pProfile->m_dVideoEncodeFramerate = (double)_tstof(strNum);
	//ビデオエンコードのサイズ変更の保存
	CButton* button1 = (CButton*)this->GetDlgItem(IDC_SIZE_AUTO_RADIO); assert(button1);
	if (button1->GetCheck() & BST_CHECKED)
		pProfile->m_bVideoEncodeSize = FALSE;
	else 
		pProfile->m_bVideoEncodeSize = TRUE;
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_WIDTH_EDIT); assert(edit);
	edit->GetWindowText(strNum);
	pProfile->m_nVideoEncodeWidth = _tstoi(strNum);
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_HEIGHT_EDIT); assert(edit);
	edit->GetWindowText(strNum);
	pProfile->m_nVideoEncodeHeight = _tstoi(strNum);
	//ビデオエンコードの滑らかさの保存
	edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_QUALITY_EDIT); assert(edit);
	edit->GetWindowText(strNum);
	pProfile->m_nVideoEnocdeQuality = _tstoi(strNum);
	//バッファサイズの保存
	edit = (CEdit*)this->GetDlgItem(IDC_BUFFER_WINDOW_EDIT);
	assert(edit);
	edit->GetWindowText(strNum);
	pProfile->m_nBufferWindow = _tstoi(strNum)*1000;

	//オーディオエンコーダーの保存
	combo = (CComboBox*)this->GetDlgItem(IDC_AUDIO_ENCODER_COMBO); assert(combo);
	if (combo->IsWindowEnabled())
		combo->GetLBText(combo->GetCurSel(),pProfile->m_strAudioEncodeName);
	else
		pProfile->m_strAudioEncodeName = _T("");
	//オーディオエンコーダーの詳細の保存
	if (m_dwBitrate != 0 && m_dwBitrate != -1)
		pProfile->m_nAudioEncodeBitrate = m_dwBitrate;
	if (m_nSamplesPerSec != 0 && m_nSamplesPerSec != -1)
		pProfile->m_nAudioEncodeSamplesPerSec = m_nSamplesPerSec;
	if (m_wBitsPerSample != 0 && m_wBitsPerSample != -1)
		pProfile->m_nAudioEncodeBitsPerSample = m_wBitsPerSample;
	if (m_nChannels != 0 && m_nChannels != -1)
		pProfile->m_nAudioEncodeChannels = m_nChannels;
	pProfile->m_bAudioEncodeAVSynchronization = m_bAVSynchronization;

	return TRUE;
}

////////////////////////////////////////
//エンコードの開始と停止
////////////////////////////////////////
BOOL CEncodePropertyDialog::Encode(BOOL bStart)
{
	if (bStart)
	{
		CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
		CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
		//設定値一覧
		DWORD dwBitrate = 0;		//ビットレート
		DWORD msBufferWindow = 0;	//バッファ時間
		double dFPS = 0.0;			//FPS
		CRect rcSource(0,0,0,0);	//転送元のRect
		CRect rcTarget(0,0,0,0);	//転送先のRect
		DWORD dwQuality = 0;		//なめらかさ
		int nMaxKeyFrameSpacing = 0;//キーフレームの間隔

		//ビットレート
		CString strNum;
		CEdit* edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_BITRATE_EDIT);
		edit->GetWindowText(strNum);
		dwBitrate = _tstoi(strNum);
		//バッファウィンドウ
		edit = (CEdit*)this->GetDlgItem(IDC_BUFFER_WINDOW_EDIT);
		edit->GetWindowText(strNum);
		msBufferWindow = _tstoi(strNum) * 1000;
		if (msBufferWindow == -1 || msBufferWindow == 0)	//3000が標準
			msBufferWindow = 3000;
		//フレームレート
		edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT);
		edit->GetWindowText(strNum);
		dFPS = (double)_tstof(strNum);
		//RECT構造体を編集する
		//TODO ここのRECT構造のサイズが本来のサイズを変わってしまっている場合がある
		//例えば、SCFH DSFプロパティで画像サイズを編集していると、このサイズが正しくない
		int nSrcWidth = 0;
		int nSrcHeight = 0;
		pDSA->GetCurrentVideoSize(nSrcWidth,nSrcHeight);
		rcSource.right = nSrcWidth;		//ここの幅と高さの指定おかしい？
		rcSource.bottom = nSrcHeight;	//ここの幅と高さの指定がおかしい？
		assert(rcSource.right != 0 && rcSource.bottom != 0);
		CButton* button = (CButton*)this->GetDlgItem(IDC_SIZE_AUTO_RADIO);
		if (button->GetCheck() & BST_CHECKED)
			rcTarget = rcSource;
		else  {
			edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_WIDTH_EDIT);
			edit->GetWindowText(strNum);
			rcTarget.right= _tstoi(strNum);
			edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_HEIGHT_EDIT);
			edit->GetWindowText(strNum);
			rcTarget.bottom = _tstoi(strNum);
		}

		//滑らかさ
		edit = (CEdit*)this->GetDlgItem(IDC_VIDEO_QUALITY_EDIT);
		edit->GetWindowText(strNum);
		dwQuality = _tstoi(strNum);
		//キーフレームの間隔
		BOOL bMaxKeyFrameSpacing = pProfile->m_bMaxKeyFrameSpacing;
		if (bMaxKeyFrameSpacing || nMaxKeyFrameSpacing == -1 || nMaxKeyFrameSpacing == 0)
		{
			if (dwBitrate <= 300)
				nMaxKeyFrameSpacing = 8000;
			else if (dwBitrate <= 600)
				nMaxKeyFrameSpacing = 6000;
			else if (dwBitrate <= 2000)
				nMaxKeyFrameSpacing = 4000;
			else
				nMaxKeyFrameSpacing = 3000;
		}
		//ビデオのパフォーマンス
		WORD wComplexity = (WORD)pProfile->m_nVideoComplexity;
		BOOL bComplexity = pProfile->m_bVideoComplexity;
		if (bComplexity == FALSE)
		{
			CComboBox* combo = (CComboBox*)this->GetDlgItem(IDC_VIDEO_ENCODER_COMBO); assert(combo);
			int nCodecIndex = combo->GetCurSel();
			if (nCodecIndex >= 0 && nCodecIndex < (int)pDSA->m_VideoCodecList.size())
			{
				WORD wMaxComplexity = (WORD)pDSA->m_VideoCodecList[nCodecIndex].dwComplexityMax;
				if (wComplexity > wMaxComplexity)
				{
					CString strMessage;
					strMessage.Format(_T("コーデック(%s)のビデオパフォーマンスの範囲は0から%dまです。\nビデオパフォーマンス設定は無効になり、コーデックの推奨値を使います"),
						pDSA->m_VideoCodecList[nCodecIndex].strName.c_str(),wMaxComplexity);
					AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
					bComplexity = TRUE;
				}
			} else {
				AfxMessageBox(_T("コーデックの選択が不明です"),MB_OK|MB_ICONINFORMATION);
				bComplexity = TRUE;
			}
		}

		BOOL bRet;
		//設定を適用する
		bRet = pDSA->SetVideoCodecSetting(dwBitrate*1000,msBufferWindow,dFPS,rcSource,rcTarget);
		if (bRet == FALSE) return FALSE;
		bRet = pDSA->SetVideoCodecSetting2(dwQuality,nMaxKeyFrameSpacing);
		if (bRet == FALSE) return FALSE;
		if (!bComplexity) {
			bRet = pDSA->SetVideoComplexity(wComplexity);
			if (bRet == FALSE) return FALSE;
		}
	}

	//ダイアログの無効と有効
	((CWnd*)GetDlgItem(IDC_VIDEO_ENCODER_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_VIDEO_BITRATE_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_VIDEO_BITRATE_SPIN))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_VIDEO_FRAMERATE_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_VIDEO_FRAMERATE_SPIN))->EnableWindow(bStart ? FALSE : TRUE);
	//((CWnd*)GetDlgItem(IDC_VIDEO_FRAMERATE_SLIDER))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_SIZE_AUTO_RADIO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_SIZE_MANUAL_RADIO))->EnableWindow(bStart ? FALSE : TRUE);
	CButton* button = (CButton*)this->GetDlgItem(IDC_SIZE_MANUAL_RADIO);
	if (button->GetCheck() & BST_CHECKED) {
		((CWnd*)GetDlgItem(IDC_VIDEO_WIDTH_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
		((CWnd*)GetDlgItem(IDC_VIDEO_WIDTH_SPIN))->EnableWindow(bStart ? FALSE : TRUE);
		((CWnd*)GetDlgItem(IDC_VIDEO_HEIGHT_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
		((CWnd*)GetDlgItem(IDC_VIDEO_HEIGHT_SPIN))->EnableWindow(bStart ? FALSE : TRUE);
	}
	((CWnd*)GetDlgItem(IDC_VIDEO_QUALITY_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_VIDEO_QUALITY_SPIN))->EnableWindow(bStart ? FALSE : TRUE);
	//((CWnd*)GetDlgItem(IDC_VIDEO_QUALITY_SLIDER))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_BUFFER_WINDOW_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_BUFFER_WINDOW_SPIN))->EnableWindow(bStart ? FALSE : TRUE);

	((CWnd*)GetDlgItem(IDC_AUDIO_ENCODER_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_AUDIO_ENCODER_BITRATE_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_AUDIO_ENCODER_SAMPLESPERSEC_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_AUDIO_ENCODER_BITSPERSAMPLE_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_AUDIO_ENCODER_CHANNELS_COMBO))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_AUDIO_ENCODER_AVSYNCHRONIZATION))->EnableWindow(bStart ? FALSE : TRUE);

	return TRUE;
}