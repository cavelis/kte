// PluginDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "PluginDialog.h"
#include "MainFrm.h"

// CPluginDialog ダイアログ

IMPLEMENT_DYNAMIC(CPluginDialog, CDialog)

BEGIN_MESSAGE_MAP(CPluginDialog, CDialog)
	ON_BN_CLICKED(IDC_VIDEO_PLUGIN_INSERT_BUTTON, &CPluginDialog::OnBnClickedVideoPluginInsertButton)
	ON_BN_CLICKED(IDC_AUDIO_PLUGIN_INSERT_BUTTON, &CPluginDialog::OnBnClickedAudioPluginInsertButton)
	ON_BN_CLICKED(IDC_VIDEO_PLUGIN_DELETE_BUTTON, &CPluginDialog::OnBnClickedVideoPluginDeleteButton)
	ON_BN_CLICKED(IDC_AUDIO_PLUGIN_DELETE_BUTTON, &CPluginDialog::OnBnClickedAudioPluginDeleteButton)
	ON_BN_CLICKED(IDC_VIDEO_PLUGIN_PROPERTY_BUTTON, &CPluginDialog::OnBnClickedVideoPluginPropertyButton)
	ON_BN_CLICKED(IDC_AUDIO_PLUGIN_PROPERTY_BUTTON, &CPluginDialog::OnBnClickedAudioPluginPropertyButton)
	ON_BN_CLICKED(IDC_VIDEO_PLUGIN_UP_BUTTON, &CPluginDialog::OnBnClickedVideoPluginUpButton)
	ON_BN_CLICKED(IDC_VIDEO_PLUGIN_DOWN_BUTTON, &CPluginDialog::OnBnClickedVideoPluginDownButton)
	ON_BN_CLICKED(IDC_AUDIO_PLUGIN_UP_BUTTON, &CPluginDialog::OnBnClickedAudioPluginUpButton)
	ON_BN_CLICKED(IDC_AUDIO_PLUGIN_DOWN_BUTTON, &CPluginDialog::OnBnClickedAudioPluginDownButton)
	ON_BN_CLICKED(IDC_ENUM_PLUGIN_CHECK, &CPluginDialog::OnBnClickedEnumPluginCheck)
END_MESSAGE_MAP()

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CPluginDialog::CPluginDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPluginDialog::IDD, pParent)
{
	m_AllDShowFilterList.clear();
	m_VideoDShowFilterList.clear();
	m_AudioDShowFilterList.clear();
	m_VideoDMOFliterList.clear();
	m_AudioDMOFliterList.clear();
}
//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CPluginDialog::~CPluginDialog()
{
}
//////////////////////////////////////////////////
//DDX/DDV
//////////////////////////////////////////////////
void CPluginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
////////////////////////////////////////
//ダイアログの初期化
////////////////////////////////////////
BOOL CPluginDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ToolTip.Create(this);
	m_ToolTip.Activate(TRUE);
	//m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_MINI_TASKBAR_CHECK),IDC_MINI_TASKBAR_CHECK);
	m_ToolTip.SetMaxTipWidth(300);
	DWORD dwAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP,dwAutoPop*2);

	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();

	//挿入可能なプラグイン一覧の列挙
	EnumPluginList(pSetting->m_bEnumAllPlugin);
	CButton* check = (CButton*)GetDlgItem(IDC_ENUM_PLUGIN_CHECK);
	if (pSetting->m_bEnumAllPlugin)
		check->SetCheck(BST_CHECKED);

	//プラグイン表の初期化
	LVCOLUMN lvc;
	TCHAR caption[][32] = {
		_T("名前"),
		_T("有効")
	};
	const int clmNum = sizeof(caption) / sizeof(caption[0]);
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	CListCtrl* list = (CListCtrl*)this->GetDlgItem(IDC_VIDEO_PLUGIN_LIST); assert(list);
	list->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);	//列単位に選択
	list->SetExtendedStyle((list->GetExtendedStyle()|LVS_EX_GRIDLINES));						//グリッド線を表示
	for (int i = 0; i < clmNum; i++) {
		lvc.iSubItem = i;
		lvc.pszText = caption[i];
		if (i == 0) lvc.cx = 190;
		else        lvc.cx = 40;
		list->InsertColumn(i,&lvc);
	}
	list = (CListCtrl*)this->GetDlgItem(IDC_AUDIO_PLUGIN_LIST); assert(list);
	list->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);	//列単位に選択
	list->SetExtendedStyle((list->GetExtendedStyle()|LVS_EX_GRIDLINES));						//グリッド線を表示
	for (int i = 0; i < clmNum; i++) {
		lvc.iSubItem = i;
		lvc.pszText = caption[i];
		if (i == 0) lvc.cx = 190;
		else        lvc.cx = 40;
		list->InsertColumn(i,&lvc);
	}
	//プラグイン表の描画
	RedrawPlugin(TRUE);
	RedrawPlugin(FALSE);

	//エンコード中の場合は幾つかのボタンが無効になる
	if (pDSA->IsEncode())
	{
		CWnd* wnd;
		wnd = GetDlgItem(IDC_VIDEO_PLUGIN_INSERT_BUTTON);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_AUDIO_PLUGIN_INSERT_BUTTON);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_PLUGIN_DELETE_BUTTON);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_AUDIO_PLUGIN_DELETE_BUTTON);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_PLUGIN_UP_BUTTON);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_AUDIO_PLUGIN_UP_BUTTON);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_VIDEO_PLUGIN_DOWN_BUTTON);
		wnd->EnableWindow(FALSE);
		wnd = GetDlgItem(IDC_AUDIO_PLUGIN_DOWN_BUTTON);
		wnd->EnableWindow(FALSE);
	}

	m_bChange = FALSE;	//今のところ変更点なし
	return TRUE;
}

////////////////////////////////////////
//OKボタン
////////////////////////////////////////
void CPluginDialog::OnOK()
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	CButton* check = (CButton*)GetDlgItem(IDC_ENUM_PLUGIN_CHECK);
	if (check->GetCheck() & BST_CHECKED) {
		pSetting->m_bEnumAllPlugin = TRUE;
	} else {
		pSetting->m_bEnumAllPlugin = FALSE;
	}
	CDialog::OnOK();
}
////////////////////////////////////////
//キャンセルボタン
//とりあえずキャンセル不可にしておいた
////////////////////////////////////////
void CPluginDialog::OnCancel()
{
	CDialog::OnCancel();
}
////////////////////////////////////////
//ツールチップの表示
////////////////////////////////////////
BOOL CPluginDialog::PreTranslateMessage(MSG* pMsg)
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
//プラグインの追加ボタン
//////////////////////////////////////////////////
BOOL CPluginDialog::InsertPluginButton(BOOL bVideo)
{
	CComboBox* combo;
	if (bVideo)
		combo = (CComboBox*)this->GetDlgItem(IDC_VIDEO_PLUGIN_LIST_COMBO);
	else
		combo = (CComboBox*)this->GetDlgItem(IDC_AUDIO_PLUGIN_LIST_COMBO);
	assert(combo);

	CString strVideo;
	if (bVideo) {
		strVideo = _T("ビデオ");
	} else {
		strVideo = _T("オーディオ");
	}

	int nComboIndex = combo->GetCurSel();
	if (nComboIndex == CB_ERR)
	{
		CString strMessage;
		strMessage.Format(_T("挿入する%sプラグインが選択されていません"),strVideo);
		AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	FilterInfoStruct* pFIS = NULL;
	pFIS = (FilterInfoStruct*)combo->GetItemDataPtr(nComboIndex);
	if (pFIS == NULL)
		return FALSE;

	//プラグインの追加を試みる
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	BOOL bRet;
	GUID guidCategory = GUID_NULL;
	if (bVideo) {
		for (int i=0;i<(int)m_VideoDMOFliterList.size();i++)
		{
			if (pFIS == &(m_VideoDMOFliterList[i])) {
				guidCategory = DMOCATEGORY_VIDEO_EFFECT;
				break;
			}
		}
	} else {
		for (int i=0;i<(int)m_AudioDMOFliterList.size();i++)
		{
			if (pFIS == &(m_AudioDMOFliterList[i])) {
				guidCategory = DMOCATEGORY_AUDIO_EFFECT;
				break;
			}
		}
	}
	//フィルタの一時停止
	BOOL bRun = StopFilter();
	//プラグインの追加
	bRet = pDSA->AddPlugin(bVideo,pFIS->guid,guidCategory,pFIS->strName.c_str());
	if (!bRet)
	{
		CString strMessage;
		strMessage.Format(_T("%sプラグインに%sを挿入するのに失敗しました"),strVideo,pFIS->strName.c_str());
		AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	//フィルタの再開 (無効なフィルタの場合、このタイミングで失敗する)
	if (!StartFilter(bVideo,bRun,TRUE,FALSE))
	{
		CString strMessage;
		strMessage.Format(_T("%sプラグインに接続不可能なプラグイン%sが挿入されました"),strVideo,pFIS->strName.c_str());
		AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//ListCtrlの最後尾にプラグインを追加
	RedrawPlugin(bVideo);
	/*
	CListCtrl* pList = NULL;
	if (bVideo){
		pList = (CListCtrl*)GetDlgItem(IDC_VIDEO_PLUGIN_LIST); assert(pList);
	} else {
		pList = (CListCtrl*)GetDlgItem(IDC_AUDIO_PLUGIN_LIST); assert(pList);
	}
	if (!pList) return FALSE;
	pList->InsertItem(pList->GetItemCount(),pFIS->strName.c_str());
	*/

	///正常完了
	m_bChange = TRUE;	//変更あり
	return TRUE;
}

//////////////////////////////////////////////////
//プラグインの削除ボタン
//////////////////////////////////////////////////
BOOL CPluginDialog::DeletePluginButton(BOOL bVideo)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	//現在選択中のItemを取得
	int nIndex = -1;
	nIndex = GetSelectedListCtrlIndex(bVideo);
	if (nIndex == -1)
		return FALSE;

	//フィルタの一時停止
	BOOL bRun = StopFilter();
	//プラグインの削除
	BOOL bRet = pDSA->RemovePlugin(bVideo,nIndex);
	if (!bRet)
		return FALSE;
	//フィルタの再開
	if (!StartFilter(bVideo,bRun,FALSE,FALSE))
		return FALSE;

	//リストからプラグインを削除
	RedrawPlugin(bVideo);
	/*
	CListCtrl* pList = NULL;
	if (bVideo){
		pList = (CListCtrl*)GetDlgItem(IDC_VIDEO_PLUGIN_LIST); assert(pList);
	} else {
		pList = (CListCtrl*)GetDlgItem(IDC_AUDIO_PLUGIN_LIST); assert(pList);
	}
	if (!pList)
		return FALSE;
	pList->DeleteItem(nIndex);
	*/

	m_bChange = TRUE;	//変更あり
	return TRUE;
}
//////////////////////////////////////////////////
//上へ・下へボタン
//////////////////////////////////////////////////
BOOL CPluginDialog::UpDownButton(BOOL bVideo,BOOL bUp)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	CListCtrl* pList = NULL;
	if (bVideo){
		pList = (CListCtrl*)GetDlgItem(IDC_VIDEO_PLUGIN_LIST); assert(pList);
	} else {
		pList = (CListCtrl*)GetDlgItem(IDC_AUDIO_PLUGIN_LIST); assert(pList);
	}
	if (!pList)
		return FALSE;

	int nIndex = -1;
	int nDestIndex = -1;

	nIndex = GetSelectedListCtrlIndex(bVideo);
	if (nIndex == -1)
		return FALSE;
	if (nIndex == 0 && bUp == TRUE)								//最前列で上へしようとした
		return FALSE;
	if ((nIndex == pList->GetItemCount()-1) && bUp == FALSE)	//最後尾で下へしようとした
		return FALSE;

	if (bUp)
		nDestIndex = nIndex - 1;
	else
		nDestIndex = nIndex + 1;

	//フィルタの一時停止
	BOOL bRun = StopFilter();

	//プラグインの入れ替え
	BOOL bRet = pDSA->SwitchPlugin(bVideo,nIndex,nDestIndex);
	if (!bRet)
		return FALSE;

	//フィルタの再開
	if (!StartFilter(bVideo,bRun,FALSE,FALSE))
		return FALSE;

	//文字の入れ替え
	pList->SetItemState(nIndex,0, LVIS_SELECTED);
	pList->SetItemState(nDestIndex,LVIS_SELECTED,LVIS_SELECTED);
	RedrawPlugin(bVideo);
	/*
	CString strSrcText;
	CString strDestText;
	strSrcText = pList->GetItemText(nIndex,0);
	strDestText = pList->GetItemText(nDestIndex,0);
	pList->SetItemText(nIndex,0,strDestText);
	pList->SetItemText(nDestIndex,0,strSrcText);
	*/

	//変更あり
	m_bChange = TRUE;

	return TRUE;
}


//////////////////////////////////////////////////
//プロパティの表示ボタン (エンコード中はこの関数のみ有効。他は無効)
//////////////////////////////////////////////////
BOOL CPluginDialog::PropertyButton(BOOL bVideo)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	int nIndex = -1;	//選択中のアイテムインデックス
	nIndex = GetSelectedListCtrlIndex(bVideo);
	//プロパティの表示
	if (nIndex != -1)
		return pDSA->ShowPluginDialog(this->m_hWnd,bVideo,nIndex);
	return FALSE;
}

//////////////////////////////////////////////////
//プラグインの全列挙ボタン
//////////////////////////////////////////////////
void CPluginDialog::OnBnClickedEnumPluginCheck()
{
	CButton* check = (CButton*)GetDlgItem(IDC_ENUM_PLUGIN_CHECK);
	if (check->GetCheck() & BST_CHECKED) {
		EnumPluginList(TRUE);
	} else {
		EnumPluginList(FALSE);
	}
}

//////////////////////////////////////////////////
//プラグインの列挙を行うサブルーチン
//////////////////////////////////////////////////
BOOL CPluginDialog::EnumPluginList(BOOL bAllPlugin)
{
	if (m_AllDShowFilterList.size() != 0)
		m_AllDShowFilterList.clear();
	if (m_VideoDShowFilterList.size() != 0)
		m_VideoDShowFilterList.clear();
	if (m_AudioDShowFilterList.size() != 0)
		m_AudioDShowFilterList.clear();
	if (m_VideoDMOFliterList.size() != 0)
		m_VideoDMOFliterList.clear();
	if (m_AudioDMOFliterList.size() != 0)
		m_AudioDMOFliterList.clear();
	//ビデオ/オーディオのマッチフィルタの列挙
	CDirectShowEtc::EnumAllFilter(CLSID_LegacyAmFilterCategory,m_AllDShowFilterList);
	CDirectShowEtc::EnumMatchFilter(MEDIATYPE_Video,GUID_NULL,m_VideoDShowFilterList);
	CDirectShowEtc::EnumMatchFilter(MEDIATYPE_Audio,GUID_NULL,m_AudioDShowFilterList);
	CDirectShowEtc::EnumAllDMOFilter(DMOCATEGORY_VIDEO_EFFECT,m_VideoDMOFliterList);
	CDirectShowEtc::EnumAllDMOFilter(DMOCATEGORY_AUDIO_EFFECT,m_AudioDMOFliterList);

	//リスト除外のフィルタの列挙
	vector<FilterInfoStruct> VideoCompressorFilterList;	//リストから除去されるフィルタ
	vector<FilterInfoStruct> AudioCompressorFilterList;	//リストから除去されるフィルタ
	CDirectShowEtc::EnumAllFilter(CLSID_VideoCompressorCategory,VideoCompressorFilterList);
	CDirectShowEtc::EnumAllFilter(CLSID_AudioCompressorCategory,AudioCompressorFilterList);

	//コンボボックスを取得
	CComboBox* video = (CComboBox*)GetDlgItem(IDC_VIDEO_PLUGIN_LIST_COMBO); assert(video);
	CComboBox* audio = (CComboBox*)GetDlgItem(IDC_AUDIO_PLUGIN_LIST_COMBO); assert(audio);
	video->ResetContent();
	audio->ResetContent();
	if (!video->IsWindowEnabled()) video->EnableWindow(TRUE);
	if (!audio->IsWindowEnabled()) audio->EnableWindow(TRUE);

	int nInsertIndex;
	CString strInsertString;
	for (int i=0;i<(int)m_VideoDMOFliterList.size();i++)	//ビデオ(DMO)
	{
		strInsertString.Format(_T("(DMO) %s"),m_VideoDMOFliterList[i].strName.c_str());
		nInsertIndex = video->AddString(strInsertString);
		video->SetItemDataPtr(nInsertIndex,&m_VideoDMOFliterList[i]);	//ポインタを仕込む
	}
	for (int i=0;i<(int)m_AudioDMOFliterList.size();i++)	//オーディオ(DMO)
	{
		strInsertString.Format(_T("(DMO) %s"),m_AudioDMOFliterList[i].strName.c_str());
		nInsertIndex = audio->AddString(strInsertString);
		audio->SetItemDataPtr(nInsertIndex,&m_AudioDMOFliterList[i]);	//ポインタを仕込む
	}

	BOOL bFind;
	for (int i=0;i<(int)m_AllDShowFilterList.size();i++) {	//ビデオ(DirectShow)
		bFind = FALSE;
		for (int j=0;j<(int)VideoCompressorFilterList.size();j++) {
			if (VideoCompressorFilterList[j].guid == m_AllDShowFilterList[i].guid) {
				bFind = TRUE;
				break;
			}
		}
		if (bFind)		//Compressorは除外
			continue;
		bFind = FALSE;
		for (int j=0;j<(int)m_VideoDShowFilterList.size();j++) {
			if (m_AllDShowFilterList[i].guid == m_VideoDShowFilterList[j].guid) {
				bFind = TRUE;
				break;
			}
		}
		if (bFind || bAllPlugin) {
			nInsertIndex = video->AddString(m_AllDShowFilterList[i].strName.c_str());
			video->SetItemDataPtr(nInsertIndex,&m_AllDShowFilterList[i]);	//ポインタを仕込む
		}
	}
	for (int i=0;i<(int)m_AllDShowFilterList.size();i++) {	//オーディオ(DirectShow)
		bFind = FALSE;
		for (int j=0;j<(int)AudioCompressorFilterList.size();j++) {
			if (AudioCompressorFilterList[j].guid == m_AllDShowFilterList[i].guid) {
				bFind = TRUE;
				break;
			}
		}
		if (bFind)		//Compressorは除外
			continue;
		bFind = FALSE;
		for (int j=0;j<(int)m_AudioDShowFilterList.size();j++) {
			if (m_AllDShowFilterList[i].guid == m_AudioDShowFilterList[j].guid) {
				bFind = TRUE;
				break;
			}
		}
		if (bFind || bAllPlugin) {
			nInsertIndex = audio->AddString(m_AllDShowFilterList[i].strName.c_str());
			audio->SetItemDataPtr(nInsertIndex,&m_AllDShowFilterList[i]);	//ポインタを仕込む
		}
	}
	if (m_VideoDShowFilterList.size() == 0 && m_VideoDMOFliterList.size() == 0){
		video->AddString(_T("[なし]"));
		video->SetCurSel(0);
		video->EnableWindow(FALSE);
	} else {
		video->SetCurSel(0);
	}
	if (m_AudioDShowFilterList.size() == 0 && m_AudioDMOFliterList.size() == 0){
		audio->AddString(_T("[なし]"));
		audio->SetCurSel(0);
		audio->EnableWindow(FALSE);
	} else {
		audio->SetCurSel(0);
	}
	return TRUE;
}

//////////////////////////////////////////////////
//プラグインのリストを更新
//////////////////////////////////////////////////
BOOL CPluginDialog::RedrawPlugin(BOOL bVideo)
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	int nInsertIndex;
	CListCtrl* list = NULL;
	vector<FilterPluginStruct>* pPluginList = NULL;
	if (bVideo) {
		list = (CListCtrl*)this->GetDlgItem(IDC_VIDEO_PLUGIN_LIST); assert(list);
		pPluginList = &(pDSA->m_VideoPluginList);
	} else {
		list = (CListCtrl*)this->GetDlgItem(IDC_AUDIO_PLUGIN_LIST); assert(list);
		pPluginList = &(pDSA->m_AudioPluginList);
	}

	//現在選択中のアイテムを取得
	int nSelectedIndex = GetSelectedListCtrlIndex(bVideo);
	//現在の表を削除
	list->DeleteAllItems();
	for (int i=0;i<(int)pPluginList->size();i++)
	{
		nInsertIndex = list->InsertItem(list->GetItemCount(),
			pPluginList->at(i).strFilterName);
		list->SetItemText(nInsertIndex,1,pPluginList->at(i).bEnable?_T("有効"):_T("無効"));
	}

	//アイテムの選択
	if (list->GetItemCount() != 0)
	{
		if (nSelectedIndex != -1 && nSelectedIndex < list->GetItemCount())
			list->SetItemState(nSelectedIndex,LVIS_SELECTED,LVIS_SELECTED);
		else
			list->SetItemState(0,LVIS_SELECTED,LVIS_SELECTED);
	}
	return TRUE;
}

//////////////////////////////////////////////////
//選択されているListCtrlのインデックスを得るサブルーチン
//////////////////////////////////////////////////
int CPluginDialog::GetSelectedListCtrlIndex(BOOL bVideo)
{
	CListCtrl* pList = NULL;
	if (bVideo){
		pList = (CListCtrl*)GetDlgItem(IDC_VIDEO_PLUGIN_LIST); assert(pList);
	} else {
		pList = (CListCtrl*)GetDlgItem(IDC_AUDIO_PLUGIN_LIST); assert(pList);
	}
	if (!pList) return -1;
	if (pList->GetSelectedCount() != 1)
		return -1;
	for (int i=0;i<(int)pList->GetItemCount();i++)
	{
		if (pList->GetItemState(i,LVIS_SELECTED) == LVIS_SELECTED)
			return i;
	}
	return -1;
}

//////////////////////////////////////////////////
//フィルタ停止のサブルーチン
//////////////////////////////////////////////////
BOOL CPluginDialog::StopFilter()
{
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
	BOOL bRun = pDSA->IsRunFilter();
	if (bRun)
		pDSA->StopFilter();
	return bRun;
}
//////////////////////////////////////////////////
//フィルタ再開のサブルーチン
//BOOL bVideo ビデオフィルタかどうか
//BOOL bInsert 前の処理はフィルタ挿入したのかどうか
//BOOL bRestart 再起処理用
//////////////////////////////////////////////////
BOOL CPluginDialog::StartFilter(BOOL bVideo,BOOL bRun,BOOL bInsert,BOOL bRestart)
{
	BOOL bRet = FALSE;
	CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();

	//フィルタの再構築処理(プラグインの再配置がされる)
	if (bVideo)
	{
		//有効なメディアタイプが存在している場合
		if (pDSA->m_VideoFirstMediaType.formattype != GUID_NULL) {
			bRet = pDSA->SelectVideoSettingIndex(pDSA->GetVideoSettingIndex(),&(pDSA->m_VideoFirstMediaType));
		} else if (pDSA->GetVideoSettingIndex() != -1){
			bRet = pDSA->SelectVideoSettingIndex(pDSA->GetVideoSettingIndex(),NULL);
		} else {
			return FALSE;
		}
	}
	else {
		if (pDSA->m_AudioFirstMediaType.formattype != GUID_NULL) {
			bRet = pDSA->SelectAudioSettingIndex(pDSA->GetAudioSettingIndex(),&(pDSA->m_AudioFirstMediaType));
		} else if (pDSA->GetAudioSettingIndex() != -1) {
			bRet = pDSA->SelectAudioSettingIndex(pDSA->GetAudioSettingIndex(),NULL);
		} else {
			return FALSE;
		}
	}
	if (bRestart == TRUE)	//再起処理から戻る
		return TRUE;

	if (bRet == FALSE && bInsert == TRUE)
	{
		//フィルタ構築失敗の場合は最後に挿入したプラグインを削除する
		if (bVideo)
			pDSA->RemovePlugin(bVideo,(int)(pDSA->m_VideoPluginList.size()-1));
		else
			pDSA->RemovePlugin(bVideo,(int)(pDSA->m_AudioPluginList.size()-1));
		//再起処理 (bRestart == TRUE)
		StartFilter(bVideo,bRun,bInsert,TRUE);
	}

	//フィルタの再開
	if (pDSA->IsCanPreview())
	{
		if (bVideo)
		{
			((CMainFrame*)AfxGetMainWnd())->GetPreviewWnd()->SetPreviewWnd();
			((CMainFrame*)AfxGetMainWnd())->GetPreviewWnd()->SetPreviewSize(TRUE);
		}
		BOOL bRunFilter = pDSA->RunFilter();
		return (bRet && bRunFilter);
	}
	return bRet;
}

