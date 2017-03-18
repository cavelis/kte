#include "StdAfx.h"

#include "DirectShowAccess.h"
#include "KTE.h"
#include "MainFrm.h"

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CDirectShowAccess::CDirectShowAccess(void)
{
	m_pDirectShow = NULL;
	m_bRunFilter = FALSE;

	m_nVideoIndex = -1;
	m_nVideoSettingIndex = -1;

	m_nVideoFilterID = -1;
	m_nVideoSmartTeeID = -1;
	m_nVideoColorSpaceID = -1;
	m_nVideoAVIDecompressorID = -1;
	m_nVideoOverlayMixerID = -1;

	m_nVideoFilterOutputPinIndex = -1;

	m_bVideoFindAudioPin = FALSE;
	m_bUseAudioVideoFilter = FALSE;

	m_nCrossbar1ID = -1;
	m_nCrossbar2ID = -1;
	m_nTVAudioID = -1;
	m_nTVTunerID = -1;

	m_hVideoWnd = NULL;
	m_bVideoShow = FALSE;
	m_bVideoForceOff = FALSE;
	m_nVideoVMR = 1;
	m_bVideoVMROverlay = TRUE;

	m_strVideoNameList.clear();
	m_strVideoSettingList.clear();
	m_VideoMediaTypeList.clear();
	m_VideoCapsList.clear();
	ZeroMemory(&m_VideoFirstMediaType,sizeof(AM_MEDIA_TYPE));
	ZeroMemory(&m_VideoLastMediaType,sizeof(AM_MEDIA_TYPE));

	m_nAudioIndex = -1;
	m_nAudioSettingIndex = -1;
	m_nAudioFilterID = -1;
	m_nAudioSmartTeeID = -1;
	//m_nAudioNullFilterID = -1;
	m_nAudioRenderID = -1;
	m_nAudioFilterOutputPinIndex = -1;

	m_nAudioNullFilterID = -1;
	m_pAudioVolumeFilter = NULL;
	m_bAudioVolumeEnable = FALSE;

	m_strAudioNameList.clear();
	m_strAudioSettingList.clear();
	m_AudioMediaTypeList.clear();
	m_AudioCapsList.clear();
	ZeroMemory(&m_AudioFirstMediaType,sizeof(AM_MEDIA_TYPE));
	ZeroMemory(&m_AudioLastMediaType,sizeof(AM_MEDIA_TYPE));
	ZeroMemory(&m_AudioSetAllocator,sizeof(ALLOCATOR_PROPERTIES));
	ZeroMemory(&m_AudioGetAllocator,sizeof(ALLOCATOR_PROPERTIES));

	m_VideoPluginList.clear();
	m_AudioPluginList.clear();

	//エンコーダー関連
	m_pDirectShowEncoder = NULL;
	m_VideoCodecList.clear();
	m_AudioCodecList.clear();
	m_AudioFormatList.clear();

	m_bStartEncode = FALSE;

	m_nVideoCodecIndex = -1;
	m_nAudioCodecIndex = -1;
	m_nAudioCodecFormatIndex = -1;

	//ASFライタ関連
	m_pDirectShowASFWriter = NULL;
	m_strAsfTempName = _T("");

	m_PullSinkList.clear();
	m_PushSinkList.clear();
	m_FileSinkList.clear();

	m_strTitle.Empty();
	m_strAuthor.Empty();
	m_strDescription.Empty();
	m_strRating.Empty();
	m_strCopyright.Empty();

	m_hNetworkCallbackWnd = NULL;
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CDirectShowAccess::~CDirectShowAccess(void)
{
	//Exit();
}

//////////////////////////////////////////////////
//DirectShowを初期化します
//////////////////////////////////////////////////
BOOL CDirectShowAccess::Init()
{
	m_pDirectShow = new CDirectShow();
	bool bret = m_pDirectShow->Init(AfxGetInstanceHandle());
	if (!bret) return FALSE;

	//ビデオキャプチャデバイス名の列挙
	m_strVideoNameList.clear();
	if (!m_pDirectShow->EnumVideoCaptureDeviceName(m_strVideoNameList))
		return FALSE;
	//オーディオキャプチャデバイス名の列挙
	m_strAudioNameList.clear();
	if (!m_pDirectShow->EnumAudioCaptureDeviceName(m_strAudioNameList))
		return FALSE;

	//エンコーダーアクセッサの初期化
	m_pDirectShowEncoder = new CDirectShowEncoder();
	HRESULT hr = m_pDirectShowEncoder->Create();
	if (FAILED(hr))
		return FALSE;

	//ビデオエンコーダの取得
	m_VideoCodecList.clear();
	hr = m_pDirectShowEncoder->EnumCodec(WMMEDIATYPE_Video,m_VideoCodecList);
	if (FAILED(hr))
		return FALSE;
	m_nVideoCodecIndex = -1;

	//オーディオエンコーダの取得
	m_AudioCodecList.clear();
	hr = m_pDirectShowEncoder->EnumCodec(WMMEDIATYPE_Audio,m_AudioCodecList);
	if (FAILED(hr))
		return FALSE;
	m_nAudioCodecIndex = -1;
	m_nAudioCodecFormatIndex = -1;

	//各種設定を読み込み
	CSetting* pSetting   = ((CKTEApp*)AfxGetApp())->GetSetting();
	m_bVideoForceOff     = pSetting->m_bPreviewForceOff;		//強制的にプレビューを切るかどうか？
	m_nVideoVMR          = pSetting->m_nPreviewVMR;			//ビデオミキシングレンダラの設定
	m_bVideoVMROverlay   = pSetting->m_bPreviewVMROverlay;	//オーバーレイ
	m_bAudioVolumeEnable = pSetting->m_bEnableAudioPreview;	//音量メーターの有無

	//プロファイルのロード(初回)
	LoadProfileSetting();
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルの変更通知
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ChangeProfileNotify()
{
	assert(m_pDirectShow);
	if (IsRunFilter())
		StopFilter();
	//プラグインの削除
	for (int i=0;i<(int)m_VideoPluginList.size();i++) {
		if (m_VideoPluginList[i].nFilterID != -1)
		{
			m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_VideoPluginList[i].nFilterID));
		}
		m_VideoPluginList[i].nFilterID = -1;
	}
	m_VideoPluginList.clear();
	for (int i=0;i<(int)m_AudioPluginList.size();i++) {
		if (m_AudioPluginList[i].nFilterID != -1)
		{
			m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_AudioPluginList[i].nFilterID));
		}
		m_AudioPluginList[i].nFilterID = -1;
	}
	m_AudioPluginList.clear();

	//プロファイルのロード
	LoadProfileSetting();
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルからのロード
//TODO ロードのやり方が変？
//////////////////////////////////////////////////
BOOL CDirectShowAccess::LoadProfileSetting()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	//ビデオプラグイン
	m_VideoPluginList.clear();
	for (int i=0;i<(int)pProfile->m_VideoPluginList.size();i++)
	{
		if (pProfile->m_VideoPluginList[i].guid == GUID_NULL)
			continue;
		FilterPluginStruct fps;
		fps.guidFilter = pProfile->m_VideoPluginList[i].guid;
		fps.guidCategory = pProfile->m_VideoPluginList[i].guidCategory;
		if (fps.guidCategory == GUID_NULL)
			fps.bDMO = FALSE;
		else
			fps.bDMO = TRUE;
		fps.nFilterID = -1;	//この時点ではフィルタ非生成
		fps.strFilterName = pProfile->m_VideoPluginList[i].strFilterName;
		fps.bEnable = pProfile->m_VideoPluginList[i].bEnable;
		//fps.nInputPinIndex = -1;
		//fps.nOutputPinIndex = -1;
		//fps.strGUIDName = pProfile->m_VideoPluginList[i].strFilterName;
		m_VideoPluginList.push_back(fps);
	}
	//オーディオプラグイン
	m_AudioPluginList.clear();
	for (int i=0;i<(int)pProfile->m_AudioPluginList.size();i++)
	{
		if (pProfile->m_AudioPluginList[i].guid == GUID_NULL)
			continue;
		FilterPluginStruct fps;
		fps.guidFilter = pProfile->m_AudioPluginList[i].guid;
		fps.guidCategory = pProfile->m_AudioPluginList[i].guidCategory;
		if (fps.guidCategory == GUID_NULL)
			fps.bDMO = FALSE;
		else
			fps.bDMO = TRUE;
		fps.nFilterID = -1;
		fps.strFilterName = pProfile->m_AudioPluginList[i].strFilterName;
		fps.bEnable = pProfile->m_AudioPluginList[i].bEnable;
		//fps.nInputPinIndex = -1;
		//fps.nOutputPinIndex = -1;
		//fps.strGUIDName = pProfile->m_AudioPluginList[i].strFilterName;
		m_AudioPluginList.push_back(fps);
	}

	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルへのセーブ
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SaveProfileSetting()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	//ビデオプラグインの情報
	pProfile->m_VideoPluginList.clear();
	for (int i=0;i<(int)m_VideoPluginList.size();i++)
	{
		IniPluginStruct ips;
		ips.guid = m_VideoPluginList[i].guidFilter;
		ips.guidCategory = m_VideoPluginList[i].guidCategory;
		ips.strFilterName = m_VideoPluginList[i].strFilterName;
		ips.bEnable = m_VideoPluginList[i].bEnable;
		if (ips.guid != GUID_NULL && ips.strFilterName.Compare(_T("")) != 0)
			pProfile->m_VideoPluginList.push_back(ips);
	}
	pProfile->m_nVideoPluginCount = (int)pProfile->m_VideoPluginList.size();
	//オーディオプラグインの情報
	pProfile->m_AudioPluginList.clear();
	for (int i=0;i<(int)m_AudioPluginList.size();i++)
	{
		IniPluginStruct ips;
		ips.guid = m_AudioPluginList[i].guidFilter;
		ips.guidCategory = m_AudioPluginList[i].guidCategory;
		ips.strFilterName = m_AudioPluginList[i].strFilterName;
		ips.bEnable = m_AudioPluginList[i].bEnable;
		if (ips.guid != GUID_NULL && ips.strFilterName.Compare(_T("")) != 0)
			pProfile->m_AudioPluginList.push_back(ips);
	}
	pProfile->m_nAudioPluginCount = (int)pProfile->m_AudioPluginList.size();
	return TRUE;
}

//////////////////////////////////////////////////
//DirectShowを終了します
//////////////////////////////////////////////////
void CDirectShowAccess::Exit()
{
	if (m_bStartEncode) {
		StopEncode(TRUE);
	}
	m_bStartEncode = FALSE;

	m_PullSinkList.clear();
	m_PushSinkList.clear();
	m_FileSinkList.clear();
	if (m_pDirectShowASFWriter) {
		m_pDirectShowASFWriter->Delete(m_pDirectShow->GetGraphBuilder());
		delete m_pDirectShowASFWriter;
		m_pDirectShowASFWriter = NULL;
	}

	m_VideoCodecList.clear();
	m_AudioCodecList.clear();
	m_AudioFormatList.clear();
	m_nVideoCodecIndex = -1;
	m_nAudioCodecIndex = -1;
	m_nAudioCodecFormatIndex = -1;
	if (m_pDirectShowEncoder) {
		m_pDirectShowEncoder->Delete();
		delete m_pDirectShowEncoder;
		m_pDirectShowEncoder = NULL;
	}

	m_strVideoNameList.clear();
	m_strVideoSettingList.clear();
	//m_VideoSettingList.clear();
	if (m_VideoMediaTypeList.size() != 0) {
		CDirectShowEtc::ReleaseMediaTypeList(m_VideoMediaTypeList);
		m_VideoMediaTypeList.clear();
	}
	if (m_VideoCapsList.size() != 0)
		m_VideoCapsList.clear();
	CDirectShowEtc::ReleaseMediaType(m_VideoFirstMediaType);
	CDirectShowEtc::ReleaseMediaType(m_VideoLastMediaType);
	
	m_nVideoIndex = -1;
	m_nVideoSettingIndex = -1;
	m_nVideoFilterID = -1;
	m_nVideoSmartTeeID = -1;
	m_nVideoColorSpaceID = -1;
	m_nVideoAVIDecompressorID = -1;
	m_nVideoOverlayMixerID = -1;
	m_nVideoFilterOutputPinIndex = -1;

	m_nCrossbar1ID = -1;
	m_nCrossbar2ID = -1;
	m_nTVAudioID = -1;
	m_nTVTunerID = -1;

	m_strAudioNameList.clear();
	m_strAudioSettingList.clear();
	if (m_AudioMediaTypeList.size() != 0) {
		CDirectShowEtc::ReleaseMediaTypeList(m_AudioMediaTypeList);
		m_AudioMediaTypeList.clear();
	}
	if (m_AudioCapsList.size() != 0)
		m_AudioCapsList.clear();
	CDirectShowEtc::ReleaseMediaType(m_AudioFirstMediaType);
	CDirectShowEtc::ReleaseMediaType(m_AudioLastMediaType);
	//ZeroMemory(&m_AudioSetAllocator,sizeof(ALLOCATOR_PROPERTIES));
	//ZeroMemory(&m_AudioGetAllocator,sizeof(ALLOCATOR_PROPERTIES));

	m_nAudioIndex = -1;
	m_nAudioSettingIndex = -1;
	m_nAudioSmartTeeID = -1;
	m_nAudioFilterOutputPinIndex = -1;
	//m_nAudioRenderID = -1;
	m_bVideoFindAudioPin = FALSE;
	m_bUseAudioVideoFilter = FALSE;

	m_nAudioNullFilterID = -1;
	if (m_pAudioVolumeFilter) {
		m_pAudioVolumeFilter->Delete(m_pDirectShow->GetGraphBuilder());
		delete m_pAudioVolumeFilter;
		m_pAudioVolumeFilter = NULL;
	}
	m_bAudioVolumeEnable = FALSE;

	//プラグインの削除
	for (int i=0;i<(int)m_VideoPluginList.size();i++) {
		if (m_VideoPluginList[i].nFilterID != -1)
		{
			m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_VideoPluginList[i].nFilterID));
		}
		m_VideoPluginList[i].nFilterID = -1;
	}
	m_VideoPluginList.clear();
	for (int i=0;i<(int)m_AudioPluginList.size();i++) {
		if (m_AudioPluginList[i].nFilterID != -1)
		{
			m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_AudioPluginList[i].nFilterID));
		}
		m_AudioPluginList[i].nFilterID = -1;
	}
	m_AudioPluginList.clear();

	//プラグインの削除
	for (int i=0;i<(int)m_VideoPluginList.size();i++) {
		if (m_VideoPluginList[i].nFilterID != -1)
		{
			m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_VideoPluginList[i].nFilterID));
		}
		m_VideoPluginList[i].nFilterID = -1;
	}
	m_VideoPluginList.clear();
	for (int i=0;i<(int)m_AudioPluginList.size();i++) {
		if (m_AudioPluginList[i].nFilterID != -1)
		{
			m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_AudioPluginList[i].nFilterID));
		}
		m_AudioPluginList[i].nFilterID = -1;
	}
	m_AudioPluginList.clear();

	m_hVideoWnd = NULL;
	m_bVideoShow = FALSE;
	m_nVideoVMR = 1;
	m_bVideoVMROverlay = FALSE;

	if (m_pDirectShow) {
		//m_pDirectShow->RemoveFromRot();
		m_pDirectShow->Exit();
		delete m_pDirectShow;
		m_pDirectShow = NULL;
	}
	m_bRunFilter = FALSE;
}
//////////////////////////////////////////////////
//フィルタグラフの実行
//////////////////////////////////////////////////
BOOL CDirectShowAccess::RunFilter()
{
	if (!IsCanPreview()) return TRUE;
	assert(m_pDirectShow);

	HRESULT hr;
	
	//TRACE0("StartPreview\n");
	SaveGraphFile(_T("RunFilter.grf"));

	/*
	//基準クロックの設定を行う
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	BOOL bUse = pProfile->m_bGraphClockEnable;
	BOOL bDefaultSyncSource = FALSE;
	BOOL bMakeSystemClock = FALSE;
	IBaseFilter* pFilter = NULL;
	switch (pProfile->m_nGraphClockSetting)
	{
	case 0:	//自動選択
		bDefaultSyncSource = TRUE;
		break;
	case 1:	//システムクロック
		bMakeSystemClock = TRUE;
		break;
	case 2:	//オーディオレンダラ(DirectSound)
		{
			if (bUse == FALSE)
				break;
			if (m_nAudioRenderID == -1) {	//オーディオレンダラの新規作成
				hr = m_pDirectShow->AddFilter(CLSID_DSoundRender,_T("DirectSound Renderer"),&m_nAudioRenderID);
				if (FAILED(hr)) {
					AfxMessageBox(_T("DirectSoundオーディオレンダラの生成に失敗しました"),MB_OK|MB_ICONINFORMATION);
					return FALSE;
				}
			}
			pFilter = m_pDirectShow->GetFilterByID(m_nAudioRenderID);
			assert(pFilter);
		}
		break;
	}
	{
		//基準クロックの設定に関して、不要な場合に於いて、オーディオレンダラの削除を行う
		if (m_nAudioRenderID != -1 && pFilter == NULL) {
			hr = m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nAudioRenderID));
			m_nAudioRenderID = -1;
		}
	}
	//基準クロックの設定
	hr = m_pDirectShow->SetGraphClock(bUse,bDefaultSyncSource,bMakeSystemClock,pFilter);
	if (FAILED(hr)) {
		AfxMessageBox(_T("基準クロックの設定に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	//同期の有無の設定を行う
	hr = m_pDirectShow->SyncStreamOffset(pProfile->m_bGraphSyncEnable,NULL);
	if (FAILED(hr)) {
		AfxMessageBox(_T("同期の設定に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	*/
	//AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);	//強制終了

	//フィルタの実行
	//hr = m_pDirectShow->Pause();
	hr = m_pDirectShow->Run();
	if (FAILED(hr)) {
		CString strErrorMessage;
		TCHAR szErr[MAX_ERROR_TEXT_LEN];
		AMGetErrorText(hr,szErr,MAX_ERROR_TEXT_LEN);
		strErrorMessage.Format(_T("%s\r\nHRESULT=0x%08X\r\n%s"),_T("StartPreview Run."),hr,szErr);
		::AfxMessageBox(strErrorMessage,MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	/*
	if (hr == 0x800705AA) {
		AfxMessageBox(_T("キャプチャデバイスの確保に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	} else if (hr == 0x8007048F) {
		AfxMessageBox(_T("デバイスが接続されていません"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	} else if (hr == 0x8007001F) {
		AfxMessageBox(_T("システムに接続されたデバイスが機能していません"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	} else if (FAILED(hr)){	//例外エラー
	}
	*/
	m_bRunFilter = TRUE;
	return TRUE;
}
//////////////////////////////////////////////////
//フィルタグラフの停止
//////////////////////////////////////////////////
BOOL CDirectShowAccess::StopFilter()
{
	assert(m_pDirectShow);
	//HRESULT hr = m_pDirectShow->Pause();
	HRESULT hr = m_pDirectShow->Stop();
	m_bRunFilter = FALSE;
	return (BOOL)SUCCEEDED(hr);
}

//////////////////////////////////////////////////
//オーディオを選択する
//int nIndex オーディオキャプチャデバイス配列のインデックス
//BOOL bUseVideoPin	接続にビデオピンを使う場合(共有モードへ移行)
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SelectAudioIndex(int nIndex,BOOL bUseVideoPin)
{
	assert(m_pDirectShow);
	assert(nIndex >= 0);
	assert(nIndex < (int)m_strAudioNameList.size());
	HRESULT hr;

	//SaveGraphFile(_T("SelectAudioIndex(start).grf"));

	//現在のオーディオキャプチャデバイスを削除する
	if (IsUseAudioVideoFilter()) {	//現在は共有モードである
		//切断のみでフィルタを削除しない
		hr = m_pDirectShow->DisconnectFilterPin(
			m_pDirectShow->GetFilterByID(m_nAudioFilterID),
			m_nAudioFilterOutputPinIndex);
	} else {						//現在は通常モードである
		//フィルタの切断と削除
		if (m_nAudioFilterID != -1) {
			if (m_nAudioFilterID != m_nVideoFilterID) {
				//hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nAudioFilterID),TRUE,TRUE);
				m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nAudioFilterID));
				m_nAudioFilterID = -1;
			}
		}
	}

	//設定の初期化
	m_nAudioIndex = -1;
	m_nAudioSettingIndex = -1;
	m_nAudioFilterID = -1;
	m_nAudioFilterOutputPinIndex = -1;

	//オーディオキャプチャデバイスを新規作成
	int nAudioFilterID = -1;
	if (bUseVideoPin)
	{
		//共有モードに移行
		if (m_nVideoFilterID == -1) {
			AfxMessageBox(_T("オーディオピンを有するビデオデバイスが存在しませんでした"),MB_OK|MB_ICONINFORMATION);
			return FALSE;
		}
		//ビデオとオーディオでフィルタIDの共有が行われてフィルタの新規作成は無し
		nAudioFilterID = m_nVideoFilterID;
	} else {		
		//通常モードに移行 (オーディオキャプチャデバイスを新規作成)
		if (!m_pDirectShow->AddAudioCaptureDeviceByName(m_strAudioNameList[nIndex],&nAudioFilterID)) {
			AfxMessageBox(_T("オーディオキャプチャデバイスの追加に失敗しました"),MB_OK|MB_ICONINFORMATION);
			return FALSE;
		}
	}

	//出力ピン番号をここで取得しておく
	int nOutputPinIndex = -1;
	vector<PinInfoStruct> PinInfoList;
	hr = m_pDirectShow->GetFilterInfo(
		m_pDirectShow->GetFilterByID(nAudioFilterID),
		PinInfoList,NULL,NULL);
	for (int i=0;i<(int)PinInfoList.size();i++) {
		if (PinInfoList[i].PinDir != PINDIR_OUTPUT)
			continue;
		//if (PinInfoList[i].bConnected == TRUE)
		//	continue;
		vector<AM_MEDIA_TYPE> MediaTypeList;
		m_pDirectShow->EnumMediaType(
			m_pDirectShow->GetFilterByID(nAudioFilterID),
			i,MediaTypeList);
		if (MediaTypeList.size() == 0)
			continue;
		if (MediaTypeList[0].majortype == MEDIATYPE_Audio)
		{
			if (nOutputPinIndex == -1)
				nOutputPinIndex = i;
		}
		CDirectShowEtc::ReleaseMediaTypeList(MediaTypeList);
	}
	if (nOutputPinIndex == -1) //適合するオーディオ出力ピンが無かった
		return FALSE;

	//TODO ここは直す必要があるかもしれない
	/*
	int nID[4];
	hr = m_pDirectShow->AddWDMFilter(
		m_pDirectShow->GetFilterByID(nAudioFilterID),FALSE,
		&nID[0],&nID[1],&nID[2],&nID[3]);
		*/

	//オーディオキャプチャのデバイス性能を取得
	if (m_AudioMediaTypeList.size() != 0) {
		CDirectShowEtc::ReleaseMediaTypeList(m_AudioMediaTypeList);
		m_AudioMediaTypeList.clear();
	}
	m_AudioCapsList.clear();
	hr = m_pDirectShow->EnumAudioCaptureMediaType(
		m_pDirectShow->GetFilterByID(nAudioFilterID),
		nOutputPinIndex,m_AudioMediaTypeList,m_AudioCapsList);

	if (FAILED(hr) && bUseVideoPin)	//共有モードの場合、キャプチャデバイスでないのでデバイス性能が取得できない
	{
		//メディアタイプのみをを取得
		m_pDirectShow->EnumMediaType(
			m_pDirectShow->GetFilterByID(nAudioFilterID),
			nOutputPinIndex,m_AudioMediaTypeList);
		//ダミーのオーディオ生成リストを生成しておく
		CDirectShowEtc::MakeDummyAudioCapsList(m_AudioMediaTypeList,m_AudioCapsList);
	}

	//設定項目リストを取得し、オーディオ情報文字列を生成する
	if (m_strAudioSettingList.size() != 0)
		m_strAudioSettingList.clear();
	CDirectShowEtc::MakeAudioMediaString(m_AudioMediaTypeList,m_strAudioSettingList);

	m_nAudioIndex = nIndex;							//選択したオーディオ番号
	m_nAudioSettingIndex = -1;						
	m_nAudioFilterOutputPinIndex = nOutputPinIndex;	//選択したオーディオの出力ピンインデックス
	m_nAudioFilterID = nAudioFilterID;				//オーディオフィルタのID
	m_bUseAudioVideoFilter = bUseVideoPin;			//共有モードの有効・無効
	//SaveGraphFile(_T("SelectAudioIndex(end).grf"));
	return TRUE;
}
//////////////////////////////////////////////////
//オーディオキャプチャ設定を適用する
//エンコード開始の場合のフィルタ接続方法
//TODO 初回起動時に2回呼ばれている
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SelectAudioSettingIndex(int nSettingIndex,AM_MEDIA_TYPE *pamt)
{
	assert(m_pDirectShow);
	assert(nSettingIndex >= 0);
	assert(nSettingIndex < (int)m_strAudioSettingList.size());
	assert(m_nAudioFilterOutputPinIndex != -1);
	HRESULT hr = NOERROR;

	//フィルタの切断
	if (m_nAudioFilterID != -1) {
		//Audio出力ピンのみを切断する
		hr = m_pDirectShow->DisconnectFilterPin(
			m_pDirectShow->GetFilterByID(m_nAudioFilterID),m_nAudioFilterOutputPinIndex);
	}
	/*
	//オーディオプラグインを全削除する
	for (int i=0;i<(int)m_AudioPluginList.size();i++)
	{
		if (m_AudioPluginList[i].nFilterID != -1)
		{
			m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_AudioPluginList[i].nFilterID));
			m_AudioPluginList[i].nFilterID = -1;
		}
	}*/
	/*
	//オーディオ分岐フィルタを削除する
	if (m_nAudioSmartTeeID != -1)
	{
		//hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),TRUE,TRUE);
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID));
		m_nAudioSmartTeeID = -1;
	}*/
	/*
	//オーディオ用のNULLレンダラを削除する
	if (m_nAudioNullFilterID != -1) {
		hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nAudioNullFilterID),TRUE,TRUE);
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nAudioNullFilterID));
		m_nAudioNullFilterID = -1;
	}*/

	//設定の適用
	if (pamt)		//メディアタイプ指定(pamt)
	{
		hr = m_pDirectShow->GetSetCaptureMediaType(
			m_pDirectShow->GetFilterByID(m_nAudioFilterID),
			m_nAudioFilterOutputPinIndex,TRUE,pamt);
	} else {		//メディアタイプ指定なし(インデックスによる接続)
		hr = m_pDirectShow->GetSetCaptureMediaType(
			m_pDirectShow->GetFilterByID(m_nAudioFilterID),
			m_nAudioFilterOutputPinIndex,TRUE,&m_AudioMediaTypeList[nSettingIndex]);
	}

	//オーディオアロケーターの設定
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	if (pProfile->m_bAudioBufferEnable && pProfile->m_nAudioBufferTime != 0 && pProfile->m_nAudioBufferCount != 0)
	{
		m_AudioSetAllocator.cBuffers = pProfile->m_nAudioBufferCount;
		WAVEFORMATEX* WaveFormatEx;
		if (pamt) {
			WaveFormatEx = (WAVEFORMATEX*)pamt->pbFormat;
		} else {
			WaveFormatEx = (WAVEFORMATEX*)m_AudioMediaTypeList[nSettingIndex].pbFormat;
		}
		assert(WaveFormatEx);
		m_AudioSetAllocator.cbBuffer = WaveFormatEx->nAvgBytesPerSec *  pProfile->m_nAudioBufferTime / 1000;
		m_AudioSetAllocator.cbAlign  = -1;
		m_AudioSetAllocator.cbPrefix = -1;
	} else {
		m_AudioSetAllocator.cBuffers = -1;
		m_AudioSetAllocator.cbBuffer = -1;
		m_AudioSetAllocator.cbAlign  = -1;
		m_AudioSetAllocator.cbPrefix = -1;
	}
	hr = m_pDirectShow->SetGetAllocatorBuffer(
		m_pDirectShow->GetFilterByID(m_nAudioFilterID),
		m_nAudioFilterOutputPinIndex,TRUE,m_AudioSetAllocator);
	//if (FAILED(hr))
	//	return FALSE;

	//オーディオ分岐フィルタを新規作成
	//int nAudioSmartTeeID = -1;
	if (m_nAudioSmartTeeID == -1)
	{
		hr = m_pDirectShow->AddFilter(CLSID_InfTee,_T("Audio Infinite Tee Filter"),&m_nAudioSmartTeeID);
		//hr = m_pDirectShow->AddFilter(CLSID_SmartTee,_T("Audio Smart Tee Filter"),&m_nAudioSmartTeeID);
		assert(m_nAudioSmartTeeID != -1);
		if (FAILED(hr)) {
			AfxMessageBox(_T("オーディオ分岐フィルタの作成に失敗しました"),MB_OK|MB_ICONINFORMATION);
			return FALSE;
		}
	} else {
		//ピンの切断
		hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),TRUE,TRUE);
	}
	//hr = m_pDirectShow->AddFilter(CLSID_InfTee,_T("Audio Infinite Pin Tee Filter"),&nAudioSmartTeeID);

	//ピン名を指定して接続を行う
	vector<PinInfoStruct> PinInfoList;
	hr = m_pDirectShow->GetFilterInfo(
		m_pDirectShow->GetFilterByID(m_nAudioFilterID),
		PinInfoList,NULL,NULL);
	//フィルタの接続(オーディオキャプチャ→プラグインリスト→オーディオ分岐フィルタ)
	if (!ChainPluginList(FALSE,m_nAudioFilterID,
		PinInfoList[m_nAudioFilterOutputPinIndex].strPinName.c_str(),
		m_nAudioSmartTeeID,NULL)) {
		return FALSE;
	}

	//フィルタの再構築(ここまで)
	if (!CheckConnectFilter(FALSE)) {
		AfxMessageBox(_T("オーディオフィルタの接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	//SaveGraphFile(_T("connect_audio_conplite.grf"));

	//現在の接続メディアタイプを取得する
	CDirectShowEtc::ReleaseMediaType(m_AudioFirstMediaType);
	CDirectShowEtc::ReleaseMediaType(m_AudioLastMediaType);
	ZeroMemory(&m_AudioFirstMediaType,sizeof(AM_MEDIA_TYPE));
	ZeroMemory(&m_AudioLastMediaType,sizeof(AM_MEDIA_TYPE));
	hr = m_pDirectShow->GetConnectionMediaType(
		m_pDirectShow->GetFilterByID(m_nAudioFilterID),
		m_nAudioFilterOutputPinIndex,m_AudioFirstMediaType);
	hr = m_pDirectShow->GetConnectionMediaType(
		m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),
		0,m_AudioLastMediaType);
	//CDirectShowEtc::DumpMediaType(&m_CurrentAudioMediaType,_T("CurrentAudioMediaType.dat"));

	//現在のアロケーターサイズを取得する
	hr = m_pDirectShow->SetGetAllocatorBuffer(
		m_pDirectShow->GetFilterByID(m_nAudioFilterID),
		m_nAudioFilterOutputPinIndex,FALSE,m_AudioGetAllocator);
	/*
	//アロケーターの比較
	BOOL bSuccess = TRUE;
	if (SUCCEEDED(hr))
	{
		//バッファ
		if (pProfile->m_bAudioBufferEnable && pProfile->m_nAudioBufferTime != 0 && pProfile->m_nAudioBufferCount != 0)
		{
			//要求サイズと同じサイズ分だけバッファが確保できたかどうか
			if (m_AudioSetAllocator.cbBuffer != m_AudioGetAllocator.cbBuffer || 
				(m_AudioSetAllocator.cBuffers != -1 && m_AudioSetAllocator.cBuffers != m_AudioGetAllocator.cBuffers)) {
					//バッファが確保できていない
					if (m_AudioGetAllocator.cbBuffer == 0 || m_AudioGetAllocator.cBuffers == 0)
						bSuccess = FALSE;
			}
		}
	} else {
		bSuccess = FALSE;
	}
	if (!bSuccess)
		AfxMessageBox(_T("オーディオバッファの確保に失敗しました\nバッファサイズの確認をしてください"),MB_OK|MB_ICONINFORMATION);
	*/

	//オーディオ音量フィルタに接続する
	ConnectAudioVolume();

	//m_nAudioSmartTeeID = nAudioSmartTeeID;		//オーディオ分岐フィルタのID
	//m_nAudioNullFilterID = nAudioNullFilterID;	//NULLフィルタのID
	m_nAudioSettingIndex = nSettingIndex;		//選択したオーディオ設定番号
	//SaveGraphFile(_T("SelectAudioSettingIndex().grf"));
	return TRUE;
}

//////////////////////////////////////////////////
//オーディオ音量フィルタに接続・切断する
//BOOL bConnect 接続するかどうか (FALSEなら切断)
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ConnectAudioVolume()
{
	HRESULT hr = NOERROR;

	//オーディオ用のNULLレンダラを新規作成
	if (m_nAudioNullFilterID == -1)
	{
		GUID guidNullRenderer = {0xC1F400A4,0x3F08,0x11D3,{0x9F,0x0B,0x00,0x60,0x08,0x03,0x9E,0x37}};//CLSID_NullRenderer
		hr = m_pDirectShow->AddFilter(guidNullRenderer,_T("Audio Volume Null Renderer"),&m_nAudioNullFilterID);
		if (FAILED(hr)) {
			AfxMessageBox(_T("オーディオボリュームNULLレンダラの追加に失敗しました"),MB_OK|MB_ICONINFORMATION);
			return FALSE;
		}
	} else {
		hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nAudioNullFilterID),TRUE,TRUE);
	}
	if (!m_pAudioVolumeFilter) {
		m_pAudioVolumeFilter = new CDirectShowAudioVolume();
		hr = m_pAudioVolumeFilter->Create(m_pDirectShow->GetGraphBuilder());
		if (FAILED(hr)) {
			AfxMessageBox(_T("オーディオボリュームサンプルグラバの追加に失敗しました"),MB_OK|MB_ICONINFORMATION);
			return FALSE;
		}
	} else {
		hr = m_pDirectShow->DisconnectFilter(m_pAudioVolumeFilter->GetFilter(),TRUE,TRUE);
	}

	LPCWSTR lpszOutputPin  = NULL;
	vector<PinInfoStruct> PinInfo;
	m_pDirectShow->GetFilterInfo(m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),PinInfo,NULL,NULL);
	for (int i=0;i<(int)PinInfo.size();i++) {
		if (PinInfo[i].PinDir != PINDIR_OUTPUT)
			continue;
		if (PinInfo[i].bConnected == TRUE)
			continue;
		lpszOutputPin = PinInfo[i].strPinName.c_str();
		break;
	}
	hr =m_pDirectShow->ConnectFilterDirect(
		m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),
		m_pAudioVolumeFilter->GetFilter(),
		NULL,lpszOutputPin,NULL);

	if (FAILED(hr)) {
		AfxMessageBox(_T("オーディオ分岐フィルタとオーディオボリュームサンプルグラバの接続に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	hr =m_pDirectShow->ConnectFilterDirect(
		m_pAudioVolumeFilter->GetFilter(),
		m_pDirectShow->GetFilterByID(m_nAudioNullFilterID),
		NULL,NULL,NULL);
	if (FAILED(hr)) {
		AfxMessageBox(_T("オーディオボリュームサンプルグラバとオーディオボリュームNULLレンダラの接続に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	//接続後にメディアタイプの確認をする
	hr = m_pAudioVolumeFilter->GetConnectionMediaType();
	
	//有効・無効の切り替え
	m_pAudioVolumeFilter->EnableCapture(m_bAudioVolumeEnable);
	return TRUE;
}

//////////////////////////////////////////////////
//ビデオを選択して設定項目を取得する
//ビデオの切り替え
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SelectVideoIndex(int nIndex)
{
	HRESULT hr = NOERROR;
	assert(m_pDirectShow);
	assert(nIndex >= 0);
	assert(nIndex < (int)m_strVideoNameList.size());

	//現在選択中のビデオのビデオキャプチャデバイスを削除する
	if (m_nVideoFilterID != -1) {
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nVideoFilterID));
		m_nVideoFilterID = -1;
		m_nVideoFilterOutputPinIndex = -1;
	}
	m_nVideoIndex = -1;
	m_nVideoSettingIndex = -1;
	m_bVideoFindAudioPin = FALSE;
	m_bUseAudioVideoFilter = FALSE;
	//ビデオキャプチャに付随したWDMフィルターの削除
	if (m_nCrossbar1ID != -1)
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nCrossbar1ID));
	m_nCrossbar1ID = -1;
	if (m_nCrossbar2ID != -1)
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nCrossbar2ID));
	m_nCrossbar2ID = -1;
	if (m_nTVAudioID != -1)
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nTVAudioID));
	m_nTVAudioID = -1;
	if (m_nTVTunerID != -1)
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nTVTunerID));
	m_nTVTunerID = -1;

	//ビデオキャプチャデバイスに追加
	int nVideoFilterID = -1;
	if (!m_pDirectShow->AddVideoCaptureDeviceByName(m_strVideoNameList[nIndex],&nVideoFilterID))
		return FALSE;

	//WDMドライバフィルタの作成と追加
	int nCrossbar1ID = -1;
	int nCrossbar2ID = -1;
	int nTVAudioID = -1;
	int nTVTunerID = -1;
	hr = m_pDirectShow->AddWDMFilter(
		m_pDirectShow->GetFilterByID(nVideoFilterID),TRUE,
		&nCrossbar1ID,&nCrossbar2ID,&nTVAudioID,&nTVTunerID);

	//ビデオキャプチャデバイスの出力ピンを取得する
	//同時にオーディオピンがあるか否かも調べる
	vector<PinInfoStruct> PinInfoList;
	int nOutputPinIndex = -1;
	BOOL bVideoFindAudioPin = FALSE;
	hr = m_pDirectShow->GetFilterInfo(m_pDirectShow->GetFilterByID(nVideoFilterID),PinInfoList,NULL,NULL);
	for (int i=0;i<(int)PinInfoList.size();i++) {
		if (PinInfoList[i].PinDir != PINDIR_OUTPUT)
			continue;
		//if (PinInfoList[i].bConnected == TRUE)
		//	continue;
		vector<AM_MEDIA_TYPE> MediaTypeList;
		m_pDirectShow->EnumMediaType(
			m_pDirectShow->GetFilterByID(nVideoFilterID),
			i,MediaTypeList);
		if (MediaTypeList.size() == 0)
			continue;
		if (MediaTypeList[0].majortype == MEDIATYPE_Video)
		{
			if (nOutputPinIndex == -1)
				nOutputPinIndex = i;
			CDirectShowEtc::ReleaseMediaTypeList(MediaTypeList);
			continue;
		} else if (MediaTypeList[0].majortype == MEDIATYPE_Audio) {
			bVideoFindAudioPin = TRUE;	//ビデオデバイスにオーディオピンが存在する
		}
		CDirectShowEtc::ReleaseMediaTypeList(MediaTypeList);
	}
	if (nOutputPinIndex == -1)	//適合するビデオ出力ピンが無かった
		return FALSE;

	//ビデオの設定の初期化と設定配列の取得
	if (m_VideoMediaTypeList.size() != 0) {
		CDirectShowEtc::ReleaseMediaTypeList(m_VideoMediaTypeList);
		m_VideoMediaTypeList.clear();
	}
	if (m_VideoCapsList.size() != 0)
		m_VideoCapsList.clear();
	hr = m_pDirectShow->EnumVideoCaptureMediaType(m_pDirectShow->GetFilterByID(nVideoFilterID),
		nOutputPinIndex,m_VideoMediaTypeList,m_VideoCapsList);

	//設定項目リストを取得し、ビデオ設定情報文字列を生成する
	if (m_strVideoSettingList.size() != 0)
		m_strVideoSettingList.clear();
	CDirectShowEtc::MakeVideoMediaString(m_VideoMediaTypeList,m_strVideoSettingList);

	//設定をメンバ変数に書き出し
	m_nVideoIndex = nIndex;					//選択したビデオ番号
	m_nVideoSettingIndex = -1;
	m_nVideoFilterID = nVideoFilterID;		//ビデオフィルタのID
	m_nVideoFilterOutputPinIndex = nOutputPinIndex;	//選択したビデオの出力ピンインデックス
	m_bVideoFindAudioPin = bVideoFindAudioPin;	//オーディオピンの有無
	m_nCrossbar1ID = nCrossbar1ID;				//第1クロスバーのフィルタのID
	m_nCrossbar2ID = nCrossbar2ID;				//第2クロスバーのフィルタのID
	m_nTVAudioID   = nTVAudioID;				//TVオーディオのフィルタのID
	m_nTVTunerID   = nTVTunerID;				//TVチューナーのフィルタのID

	//SaveGraphFile(_T("SelectVideoIndex().grf"));
	return TRUE;
}
//////////////////////////////////////////////////
//ビデオキャプチャ設定を適用する
//エンコード開始の場合のフィルタ接続方法
//TODO 呼び出し2回
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SelectVideoSettingIndex(int nSettingIndex,AM_MEDIA_TYPE *pamt)
{
	assert(m_pDirectShow);
	assert(nSettingIndex >= 0);
	assert(nSettingIndex < (int)m_strVideoSettingList.size());
	assert(m_nVideoFilterID != -1);
	assert(m_nVideoFilterOutputPinIndex != -1);

	HRESULT hr = NOERROR;

	//SaveGraphFile(_T("SelectVideoSettingIndex(0).grf"));

	//フィルタの切断
	//if (m_pDirectShow->GetVideoRenderer() != NULL)
	//	m_pDirectShow->DisconnectFilter(m_pDirectShow->GetVideoRenderer(),TRUE,TRUE);
	//if (m_nVideoConvertID != -1)
	//	m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nVideoConvertID),TRUE,TRUE);
	//if (m_nVideoSmartTeeID != -1)
	//	m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),TRUE,TRUE);
	if (m_nVideoFilterID != -1){	//この切断は下流のみ 
		//自身のビデオピンのみを切断する
		if (m_nVideoFilterOutputPinIndex != -1) {
			m_pDirectShow->DisconnectFilterPin(m_pDirectShow->GetFilterByID(m_nVideoFilterID),
				m_nVideoFilterOutputPinIndex);
		}
	}

	//ビデオ分岐フィルタの削除
	/*
	if (m_nVideoSmartTeeID != -1) {
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID));
		m_nVideoSmartTeeID = -1;
	}*/

	//設定の適用方法はAM_MEDIA_TYPEを使う
	if (pamt) {	//メディアタイプ指定あり(pamt)
		hr = m_pDirectShow->GetSetCaptureMediaType(
			m_pDirectShow->GetFilterByID(m_nVideoFilterID),
			m_nVideoFilterOutputPinIndex,TRUE,pamt);
	} else {	//メディアタイプ指定なし(インデックスによる接続)
		hr = m_pDirectShow->GetSetCaptureMediaType(
			m_pDirectShow->GetFilterByID(m_nVideoFilterID),
			m_nVideoFilterOutputPinIndex,TRUE,&m_VideoMediaTypeList[nSettingIndex]);
	}

	//ピン情報を取得
	vector<PinInfoStruct> PinInfoList;
	hr = m_pDirectShow->GetFilterInfo(
		m_pDirectShow->GetFilterByID(m_nVideoFilterID),
		PinInfoList,NULL,NULL);

	//ビデオ分岐フィルタの作成 (ここは毎回再生成するように変更した)
	if (m_nVideoSmartTeeID != -1) {
		hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),TRUE,TRUE);
		hr = m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID));
		m_nVideoSmartTeeID = -1;
	}
	hr = m_pDirectShow->AddFilter(CLSID_InfTee,_T("Video Infinite Tee Filter"),&m_nVideoSmartTeeID);
	//hr = m_pDirectShow->AddFilter(CLSID_SmartTee,_T("Video Smart Tee Filter"),&m_nVideoSmartTeeID);
	assert(m_nVideoSmartTeeID != -1);
	if (FAILED(hr))
	{
		AfxMessageBox(_T("ビデオ分岐フィルタの作成に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//この段階ではフィルタはずたずた
	//SaveGraphFile(_T("SelectVideoSettingIndex(5).grf"));

	//フィルタの接続(ビデオキャプチャ→プラグインリスト→ビデオ分岐フィルタ)
	if (!ChainPluginList(TRUE,m_nVideoFilterID,
		PinInfoList[m_nVideoFilterOutputPinIndex].strPinName.c_str(),
						m_nVideoSmartTeeID,NULL))
	{
		AfxMessageBox(_T("ビデオフィルタの接続に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	//この段階では失敗している(無効なフィルタグラフ)
	//SaveGraphFile(_T("SelectVideoSettingIndex(6).grf"));

	//現在のビデオメディアタイプを取得する (ビデオキャプチャの接続)
	hr = CDirectShowEtc::ReleaseMediaType(m_VideoFirstMediaType);
	hr = CDirectShowEtc::ReleaseMediaType(m_VideoLastMediaType);
	ZeroMemory(&m_VideoFirstMediaType,sizeof(AM_MEDIA_TYPE));
	ZeroMemory(&m_VideoLastMediaType,sizeof(AM_MEDIA_TYPE));
	hr = m_pDirectShow->GetConnectionMediaType(
		m_pDirectShow->GetFilterByID(m_nVideoFilterID),
		m_nVideoFilterOutputPinIndex,m_VideoFirstMediaType);
	hr = m_pDirectShow->GetConnectionMediaType(
		m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),
		0,m_VideoLastMediaType);
	//接続メディアタイプのDump
	//CDirectShowEtc::DumpMediaType(&m_VideoFirstMediaType,_T("DumpVideoFirst.dat"));
	//CDirectShowEtc::DumpMediaType(&m_VideoLastMediaType,_T("DumpVideoLast.dat"));

	//ビデオフィルタの接続
	hr = RenderPreview();
	if (FAILED(hr)) {
		AfxMessageBox(_T("ビデオフィルタの接続に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	//ビデオハンドラの設定
	if (m_hVideoWnd != NULL)
		SetPreviewWindow(m_hVideoWnd);

	//フィルタの接続(ここまで)
	if (!CheckConnectFilter(TRUE))
	{
		AfxMessageBox(_T("ビデオフィルタの接続チェックに失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//SaveGraphFile(_T("SelectVideoIndexSetting(end).grf"));

	m_nVideoSettingIndex = nSettingIndex;			//選択したビデオ設定番号
	return TRUE;
}



//////////////////////////////////////////////////
//プレビューの接続の試行
//////////////////////////////////////////////////
HRESULT CDirectShowAccess::RenderPreview()
{
	if (m_bVideoForceOff == TRUE)	//強制非表示モードの場合
		return S_OK;
	HRESULT hr  = NOERROR;

	//ビデオレンダラが存在しない場合は作成
	if (m_pDirectShow->GetVideoRenderer() == NULL) {
		if (!m_pDirectShow->InitVideoRenderer(m_nVideoVMR,m_bVideoVMROverlay))
			return E_FAIL;
	} else {
		hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetVideoRenderer(),TRUE,TRUE);
	}

	int nLastFilterID = m_nVideoSmartTeeID;		//最後尾のフィルタID
	const wchar_t* lpszOutputPin;

	//空きの有限分岐フィルタ出力ピンを検索
	vector<PinInfoStruct> PinInfo;
	m_pDirectShow->GetFilterInfo(
		m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),
		PinInfo,NULL,NULL);
	for (int i=0;i<(int)PinInfo.size();i++) {
		if (PinInfo[i].PinDir != PINDIR_OUTPUT)
			continue;
		if (PinInfo[i].bConnected == TRUE)
			continue;
		lpszOutputPin = PinInfo[i].strPinName.c_str();
		break;
	}

	//直接接続を試行
	hr = RenderPreviewSubRoutine1(nLastFilterID,lpszOutputPin);
	if (SUCCEEDED(hr)) {
		RenderPreviewSubRoutine2();
		return hr;
	}

	//接続情報の取得
	BOOL bIsVideoInfo2;		//VideoInfoHeader2かどうか
	BOOL bRGB;				//RGB系かどうか (FALSEならYUV系)
	if (m_VideoLastMediaType.formattype == FORMAT_VideoInfo) {
		bIsVideoInfo2 = FALSE;
		VIDEOINFOHEADER* pVideoInfoHeader = (VIDEOINFOHEADER *)m_VideoLastMediaType.pbFormat;
		if (pVideoInfoHeader->bmiHeader.biCompression == 0)
			bRGB = TRUE;
		else
			bRGB = FALSE;
	} else if (m_VideoLastMediaType.formattype == FORMAT_VideoInfo2){
		bIsVideoInfo2 = TRUE;
		VIDEOINFOHEADER2* pVideoInfoHeader2 = (VIDEOINFOHEADER2*)m_VideoLastMediaType.pbFormat;
		if (pVideoInfoHeader2->bmiHeader.biCompression == 0)
			bRGB = TRUE;
		else
			bRGB = FALSE;
	}

	//VideoInfoHeader2
	if (bIsVideoInfo2 == TRUE)
	{
		//オーバーレイミキサを挟む
		if (m_nVideoOverlayMixerID == -1) {
			hr = m_pDirectShow->AddFilter(CLSID_OverlayMixer,_T("Overlay Mixer"),&m_nVideoOverlayMixerID);
			assert(m_nVideoOverlayMixerID != -1);
		} else {
			hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nVideoOverlayMixerID),TRUE,TRUE);
		}
		hr = m_pDirectShow->ConnectFilterDirect(
			m_pDirectShow->GetFilterByID(nLastFilterID),
			m_pDirectShow->GetFilterByID(m_nVideoOverlayMixerID),
			NULL,lpszOutputPin,NULL);
		if (SUCCEEDED(hr)) {
			nLastFilterID = m_nVideoOverlayMixerID;
			lpszOutputPin = NULL;
			//直接接続を試行
			hr = RenderPreviewSubRoutine1(nLastFilterID,lpszOutputPin);
			if (SUCCEEDED(hr)) {
				RenderPreviewSubRoutine2();
				return S_OK;
			}
		}
	}

	//YUVソースの場合
	if (bRGB == FALSE)
	{
		//AVI Decompressorを挟む
		if (m_nVideoAVIDecompressorID == -1) {
			hr = m_pDirectShow->AddFilter(CLSID_AVIDec,_T("AVI Decompressor"),&m_nVideoAVIDecompressorID);
			assert(m_nVideoAVIDecompressorID != -1);
		} else {
			hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nVideoAVIDecompressorID),TRUE,TRUE);
		}
		hr = m_pDirectShow->ConnectFilterDirect(
			m_pDirectShow->GetFilterByID(nLastFilterID),
			m_pDirectShow->GetFilterByID(m_nVideoAVIDecompressorID),
			NULL,lpszOutputPin,NULL);
		if (SUCCEEDED(hr)) {
			nLastFilterID = m_nVideoAVIDecompressorID;
			lpszOutputPin = NULL;
			//直接接続を試行
			hr = RenderPreviewSubRoutine1(nLastFilterID,lpszOutputPin);
			if (SUCCEEDED(hr)) {
				RenderPreviewSubRoutine2();
				return S_OK;
			}
		}
	}

	//色空間変換が必要な場合、カラースペースコンバーターを挟む
	if (m_nVideoColorSpaceID == -1) {
		hr = m_pDirectShow->AddFilter(CLSID_Colour,_T("Color Space Converter"),&m_nVideoColorSpaceID);
		assert(m_nVideoColorSpaceID != -1);
	} else {
		hr = m_pDirectShow->DisconnectFilter(m_pDirectShow->GetFilterByID(m_nVideoColorSpaceID),TRUE,TRUE);
	}
	hr = m_pDirectShow->ConnectFilterDirect(
		m_pDirectShow->GetFilterByID(nLastFilterID),
		m_pDirectShow->GetFilterByID(m_nVideoColorSpaceID),
		NULL,lpszOutputPin,NULL);
	if (SUCCEEDED(hr)) {
		nLastFilterID = m_nVideoColorSpaceID;
		lpszOutputPin = NULL;
		//直接接続を試行
		hr = RenderPreviewSubRoutine1(nLastFilterID,lpszOutputPin);
		if (SUCCEEDED(hr)) {
			RenderPreviewSubRoutine2();
			return S_OK;
		}
	}
	RenderPreviewSubRoutine2();
	return E_FAIL;
}
//////////////////////////////////////////////////
//プレビューの接続の試行で使うサブルーチン (ビデオとの接続を試みる)
HRESULT CDirectShowAccess::RenderPreviewSubRoutine1(const int nLastFilterID,const wchar_t* lpszOutputPin)
{
	return m_pDirectShow->ConnectFilterDirect(
		m_pDirectShow->GetFilterByID(nLastFilterID),
		m_pDirectShow->GetVideoRenderer(),
		NULL,lpszOutputPin,NULL);
}
//////////////////////////////////////////////////
//プレビューの接続の試行で使うサブルーチン (未使用のフィルタの削除をする)
void CDirectShowAccess::RenderPreviewSubRoutine2()
{
	int nInputPinCount = 0;
	int nOutputPinCount = 0;
	HRESULT hr = NOERROR;
	if (m_nVideoOverlayMixerID != -1) {
		CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nVideoOverlayMixerID),&nInputPinCount,&nOutputPinCount);
		if (nInputPinCount == 0 && nOutputPinCount == 0) {
			hr = m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nVideoOverlayMixerID));
			m_nVideoOverlayMixerID = -1;
		}
	}
	nInputPinCount = 0;
	nOutputPinCount = 0;
	if (m_nVideoAVIDecompressorID != -1) {
		CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nVideoAVIDecompressorID),&nInputPinCount,&nOutputPinCount);
		if (nInputPinCount == 0 && nOutputPinCount == 0) {
			hr = m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nVideoAVIDecompressorID));
			m_nVideoAVIDecompressorID = -1;
		}
	}
	nInputPinCount = 0;
	nOutputPinCount = 0;
	if (m_nVideoColorSpaceID != -1) {
		CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nVideoColorSpaceID),&nInputPinCount,&nOutputPinCount);
		if (nInputPinCount == 0 && nOutputPinCount == 0) {
			hr = m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(m_nVideoColorSpaceID));
			m_nVideoColorSpaceID = -1;
		}
	}
}

//////////////////////////////////////////////////
//フィルタの接続状況を確認するサブルーチン
//BOOL bVideo ビデオフィルタをチェックするか、オーディオフィルタをチェックするか
//////////////////////////////////////////////////
BOOL CDirectShowAccess::CheckConnectFilter(BOOL bVideo)
{
	assert(m_pDirectShow);
	int nInputCount = 0;
	int nOutputCount = 0;
	if (bVideo)
	{
		if (m_nVideoFilterID != -1) {
			CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nVideoFilterID),NULL,&nOutputCount);
			if (nOutputCount == 0) {
				AfxMessageBox(_T("ビデオキャプチャデバイスの出力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
				return FALSE;}
		}
		if (m_nVideoSmartTeeID != -1) {
			CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),&nInputCount,&nOutputCount);
			if (nInputCount == 0) {
				AfxMessageBox(_T("ビデオ分岐フィルタの入力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
				return FALSE;}
			if (m_bVideoForceOff == FALSE) {
				if (nOutputCount == 0) {
					AfxMessageBox(_T("ビデオ分岐フィルタの出力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
					this->SaveGraphFile(_T("testes.grf"));
					return FALSE;}
			}
		}
		/*
		//TODO このチェックも作っておく
		if (m_nVideoConvertID != -1) {
			CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nVideoConvertID),&nInputCount,&nOutputCount);
			if (nInputCount == 0) {
				AfxMessageBox(_T("ビデオ変換フィルタの入力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
				return FALSE;}
			if (nOutputCount == 0) {
				AfxMessageBox(_T("ビデオ変換フィルタの出力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
				return FALSE;}
		}
		*/
		if (m_bVideoForceOff == FALSE) {
			CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetVideoRenderer(),&nInputCount,NULL);
			if (nOutputCount == 0) {
				AfxMessageBox(_T("ビデオ変換フィルタの出力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
				return FALSE;}
		}
	}
	else	//オーディオキャプチャのチェック
	{
		if (m_nAudioFilterID != -1) {
			CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nAudioFilterID),NULL,&nOutputCount);
			if (nOutputCount == 0) {
				AfxMessageBox(_T("オーディオキャプチャデバイスの出力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
				return FALSE;}
		}
		if (m_nAudioSmartTeeID != -1) {
			CDirectShowEtc::CheckConnectionFilter(m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),&nInputCount,NULL);
			if (nInputCount == 0) {
				AfxMessageBox(_T("オーディオ分岐フィルタの入力接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
				return FALSE;}
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////
//現在のビデオサイズを取得
//int &nWidth	ビデオの幅
//int &nHeight	ビデオの高さ
//////////////////////////////////////////////////
BOOL CDirectShowAccess::GetCurrentVideoSize(int &nWidth,int &nHeight)
{
	nWidth = 0;
	nHeight = 0;
	AM_MEDIA_TYPE* pAmt = NULL;
	if (m_VideoLastMediaType.formattype == FORMAT_VideoInfo || 
		m_VideoLastMediaType.formattype == FORMAT_VideoInfo2)
	{
		pAmt = &(m_VideoLastMediaType);
	} else {
		pAmt = &(m_VideoFirstMediaType);
	}
	if (pAmt == NULL)
		return FALSE;
	if (pAmt->formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER *pVideoInfoHeader = (VIDEOINFOHEADER *)pAmt->pbFormat;
		nWidth = pVideoInfoHeader->bmiHeader.biWidth;
		nHeight = pVideoInfoHeader->bmiHeader.biHeight;
	} else if (pAmt->formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2 *pVideoInfoHeader2 = (VIDEOINFOHEADER2 *)pAmt->pbFormat;
		nWidth = pVideoInfoHeader2->bmiHeader.biWidth;
		nHeight = pVideoInfoHeader2->bmiHeader.biHeight;
	}
	return TRUE;
}
//////////////////////////////////////////////////
//ビデオプレビューのウィンドウハンドルを設定する
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SetPreviewWindow(HWND hWnd)
{
	bool bret = false;
	assert(m_pDirectShow);
	assert(hWnd);
	if (m_bVideoForceOff == TRUE)
		return TRUE;
	bret = m_pDirectShow->SetVideoRendererHWnd(hWnd);
	m_hVideoWnd = hWnd;
	return (BOOL) bret;
}
//////////////////////////////////////////////////
//ビデオプレビューのウィンドウサイズを変更する
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ResizePreviewWindow(RECT rcClient)
{
	bool bret = false;
	assert(m_pDirectShow);
	if (m_bVideoForceOff == TRUE)
		return TRUE;
	if (!m_pDirectShow->GetVideoRenderer() || m_hVideoWnd == NULL)
		return FALSE;
	bret = m_pDirectShow->ResizeVideoRendererHWnd(rcClient);
	return (BOOL) bret;
}
//////////////////////////////////////////////////
//ビデオプレビューの表示・非表示を切り替える
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ShowPreviewWindow(BOOL bShow)
{
	bool bret = false;
	assert(m_pDirectShow);
	if (m_bVideoForceOff == TRUE)
		return TRUE;
	if (!m_pDirectShow->GetVideoRenderer() || m_hVideoWnd == NULL) {
		m_bVideoShow = FALSE;
		return FALSE;}
	bret = m_pDirectShow->ShowVideoRenderer(bShow);	//ビデオレンダラの表示・非表示
	m_bVideoShow = bShow;
	return (BOOL) bret;
}
//////////////////////////////////////////////////
//ビデオプレビューのウィンドウハンドルを解除する
//////////////////////////////////////////////////
BOOL CDirectShowAccess::RemovePreviewWindow()
{
	assert(m_pDirectShow);
	if (m_bVideoForceOff == TRUE)
		return TRUE;
	bool bret = false;
	if (m_hVideoWnd != NULL) {
		bret = m_pDirectShow->ResetVideoRendererHWnd();
	} else {
		bret = TRUE;
	}
	m_hVideoWnd = NULL;
	m_bVideoShow = FALSE;
	return (BOOL) bret;
}
//////////////////////////////////////////////////
//ビデオ/オーディオキャプチャデバイスのプロパティページを表示する
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ShowPropertySetting(BOOL bVideo,ENUM_WDM_FILTER eDevice,HWND hWnd)
{
	assert(m_pDirectShow);
	assert(hWnd);
	int nShowPropertyID = -1;
	if (eDevice != WDM_FILTER_NONE)
	{
		switch (eDevice)
		{
		case WDM_FILTER_CROSS_BAR1:
			nShowPropertyID = m_nCrossbar1ID; break;
		case WDM_FILTER_CROSS_BAR2:
			nShowPropertyID = m_nCrossbar2ID; break;
		case WDM_FILTER_TV_AUDIO:
			nShowPropertyID = m_nTVAudioID; break;
		case WDM_FILTER_TV_TUNER:
			nShowPropertyID = m_nTVTunerID; break;
		}
	} else {
		if (bVideo) {
			if (m_nVideoIndex != -1)
				nShowPropertyID = m_nVideoFilterID;
		} else {
			if (m_nAudioIndex != -1)
				nShowPropertyID = m_nAudioFilterID;
		}
	}

	if (nShowPropertyID != -1)
	{
		HRESULT hr = m_pDirectShow->ShowPropertyDialog(
			m_pDirectShow->GetFilterByID(nShowPropertyID),hWnd);
		return SUCCEEDED(hr);
	}
	return FALSE;
}

//////////////////////////////////////////////////
//フィルタ配列の接続
//BOOL bVideo ビデオフィルタかどうか FALSEならaudioフィルタ
//int nInputFilterID		最初入力のフィルタID
//LPCTSTR lpszInputPinName	最初入力ピンの名前
//int nOutputFilterID		最終出力のフィルタID
//LPCTSTR lpszOutputPinName 最終出力ピンの名前
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ChainPluginList(BOOL bVideo,int nInputFilterID,const wchar_t* lpszInputPinName,int nOutputFilterID,const wchar_t* lpszOutputPinName)
{
	assert(m_pDirectShow);
	assert(nInputFilterID != -1);
	assert(nOutputFilterID != -1);

	int nInputID = nInputFilterID;
	int nOutputID = -1;
	const wchar_t* lpszInputPin = lpszInputPinName;
	const wchar_t* lpszOutputPin = NULL;
	int nSize = 0;
	if (bVideo)
		nSize = (int)m_VideoPluginList.size();
	else
		nSize = (int)m_AudioPluginList.size();

	FilterPluginStruct *pDestFilter = NULL;	//接続先のフィルタ
	HRESULT hr = NOERROR;

	for (int i=0;i<=nSize;i++)
	{
		//入力・出力のフィルターIDを取得
		if (nSize == i)	//最終接続
		{
			pDestFilter = NULL;
			nOutputID = nOutputFilterID;
			lpszOutputPin = lpszOutputPinName;
		} else {		//プラグイン接続
			if (bVideo)
				pDestFilter = &(m_VideoPluginList[i]);
			else
				pDestFilter = &(m_AudioPluginList[i]);
		}

		if (pDestFilter)	//プラグイン接続の場合
		{
			if (pDestFilter->bEnable == FALSE) {			//無効なプラグインの場合
				if (pDestFilter->nFilterID != -1) {				//フィルタが存在しているのなら削除しておく
					m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(pDestFilter->nFilterID));
					pDestFilter->nFilterID = -1;
				}
				continue;
			}
			/*
			if (pDestFilter->nFilterID == -1) {	//フィルタが存在しないので、フィルタの生成を行う
				HRESULT hr;
				if (pDestFilter->bDMO)
					hr = m_pDirectShow->AddDMOFilter(pDestFilter->guidFilter,pDestFilter->guidCategory,pDestFilter->strFilterName,&(pDestFilter->nFilterID));
				else
					hr = m_pDirectShow->AddFilter(pDestFilter->guidFilter,pDestFilter->strFilterName,&(pDestFilter->nFilterID));
				if (FAILED(hr))
				{
					CString strMessage;
					strMessage.Format(_T("%d番目の%sプラグイン\n%sの新規作成に失敗しました。\nこのプラグインは無効になります"),i+1,bVideo?_T("ビデオ"):_T("オーディオ"),pDestFilter->strFilterName);
					AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
					pDestFilter->nFilterID = -1;
					pDestFilter->bEnable = FALSE;
					continue;
				}
			} else {	//フィルタは存在しているので切断だけ行う
				m_pDirectShow->DisconnectFilter(
					m_pDirectShow->GetFilterByID(pDestFilter->nFilterID),TRUE,TRUE);
			}*/
			if (pDestFilter->nFilterID != -1)
			{
				m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(pDestFilter->nFilterID));
				pDestFilter->nFilterID = -1;
			}
			HRESULT hr;
			if (pDestFilter->bDMO)
				hr = m_pDirectShow->AddDMOFilter(pDestFilter->guidFilter,pDestFilter->guidCategory,pDestFilter->strFilterName,&(pDestFilter->nFilterID));
			else
				hr = m_pDirectShow->AddFilter(pDestFilter->guidFilter,pDestFilter->strFilterName,&(pDestFilter->nFilterID));
			if (FAILED(hr))
			{
				CString strMessage;
				strMessage.Format(_T("%d番目の%sプラグイン\n%sの新規作成に失敗しました。\nこのプラグインは無効になります"),i+1,bVideo?_T("ビデオ"):_T("オーディオ"),pDestFilter->strFilterName);
				AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
				pDestFilter->nFilterID = -1;
				pDestFilter->bEnable = FALSE;
				continue;
			}
			nOutputID = pDestFilter->nFilterID;
			lpszOutputPin = NULL;
		}

		//プラグインの連結を行う
		assert(nInputID != -1);
		assert(nOutputID != -1);

		//接続には入出力ピン情報を用いて、すべてのピンに対して行列で接続を試みる
		vector<PinInfoStruct> InputPinInfo,OutputPinInfo;
		m_pDirectShow->GetFilterInfo(m_pDirectShow->GetFilterByID(nInputID),InputPinInfo,NULL,NULL);
		m_pDirectShow->GetFilterInfo(m_pDirectShow->GetFilterByID(nOutputID),OutputPinInfo,NULL,NULL);

		/*
		HRESULT CDirectShowEtc::ConnectFilterMatrix(
			IBaseFilter* pSrcFilter,
			IBaseFilter* pDestFiter,
			LPCWSTR lpszInputPin,
			LPCWSTR lpszOutputPin);
			*/

		if (lpszInputPin) {			//入力ピン指定
			if (lpszOutputPin) {
				//出力ピン指定
				hr = m_pDirectShow->ConnectFilterDirect(
					m_pDirectShow->GetFilterByID(nInputID),
					m_pDirectShow->GetFilterByID(nOutputID),
					NULL,lpszInputPin,lpszOutputPin);
			} else {
				//出力ピン不指定
				for (int j=0;j<(int)OutputPinInfo.size();j++) {
					if (OutputPinInfo[j].PinDir != PINDIR_INPUT)
						continue;
					hr = m_pDirectShow->ConnectFilterDirect(
						m_pDirectShow->GetFilterByID(nInputID),
						m_pDirectShow->GetFilterByID(nOutputID),
						NULL,lpszInputPin,OutputPinInfo[j].strPinName.c_str());
					if (SUCCEEDED(hr))
						break;
				}
			}
		} else {					//入力ピン不指定
			for (int i=0;i<(int)InputPinInfo.size();i++) {
				if (InputPinInfo[i].PinDir != PINDIR_OUTPUT)
					continue;

				if (lpszOutputPin) {	//出力ピン指定
					hr = m_pDirectShow->ConnectFilterDirect(
						m_pDirectShow->GetFilterByID(nInputID),
						m_pDirectShow->GetFilterByID(nOutputID),
						NULL,InputPinInfo[i].strPinName.c_str(),lpszOutputPin);
					if (SUCCEEDED(hr))
						break;
				} else {				//出力ピン不指定
					for (int j=0;j<(int)OutputPinInfo.size();j++) {
						if (OutputPinInfo[j].PinDir != PINDIR_INPUT)
							continue;
						hr = m_pDirectShow->ConnectFilterDirect(
							m_pDirectShow->GetFilterByID(nInputID),
							m_pDirectShow->GetFilterByID(nOutputID),
							NULL,InputPinInfo[i].strPinName.c_str(),OutputPinInfo[j].strPinName.c_str());
						if (SUCCEEDED(hr))
							break;
					}
				}
				if (SUCCEEDED(hr))
					break;
			}
		}

		if (FAILED(hr))
		{
			SaveGraphFile(_T("PluginConnectionError.grf"));
			CString strMessage;
			strMessage.Format(_T("%d番目の%sプラグイン\n%sの接続に失敗しました。\nこのプラグインは無効になります"),i+1,bVideo?_T("ビデオ"):_T("オーディオ"),
				pDestFilter?pDestFilter->strFilterName:_T("NULL"));
			AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);

			//プラグインが無効になる
			if (pDestFilter) {	//通常接続(何か->プラグイン)
				if (pDestFilter->nFilterID != -1) {
					m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(pDestFilter->nFilterID));
					pDestFilter->nFilterID = -1;
				}
				pDestFilter->bEnable = FALSE;
			} else {			//最終接続の場合(プラグイン->スマートティー)
				//前のフィルタを削除
				FilterPluginStruct* pSrcFilter = NULL;
				if (i >= 1) {
					if (bVideo)
						pSrcFilter = &(m_VideoPluginList[i-1]);
					else
						pSrcFilter = &(m_AudioPluginList[i-1]);
				}
				if (pSrcFilter == NULL) {
					return FALSE;
				}
				if (pSrcFilter->nFilterID != -1) {
					m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(pSrcFilter->nFilterID));
					pSrcFilter->nFilterID = -1;
				}
				pSrcFilter->bEnable = FALSE;
				//ここで必要になるのはもうさらに1つ前のID
				if (i >= 2) {
					if (bVideo)
						nInputID = m_VideoPluginList[i-2].nFilterID;
					else
						nInputID = m_AudioPluginList[i-2].nFilterID;
					lpszInputPin = NULL;
				} else {
					nInputID = nInputFilterID;
					lpszInputPin = lpszInputPinName;
				}
				i--;	//一つ処理が戻る
			}
			continue;
		}

		//次のフィルタに移行するために出力を入力にする
		nInputID = nOutputID;
		lpszInputPin = NULL;
	}
	return TRUE;
}
//////////////////////////////////////////////////
//プラグインの追加登録(最後尾にプラグインフィルタが追加される)
//BOOL bVideo				ビデオフィルタかどうか
//const GUID guid			プラグインのGUID名
//const GUID guidCategory	guidのカテゴリ(DMOの場合は値が入り、それ以外の場合はGUID_NULLを指定)
//LPCWSTR lpszPluginName	プラグインの名前
//////////////////////////////////////////////////
BOOL CDirectShowAccess::AddPlugin(BOOL bVideo,const GUID guid,const GUID guidCategory,LPCWSTR lpszPluginName)
{
	assert(m_pDirectShow);
	if (guid == GUID_NULL)
		return FALSE;

	FilterPluginStruct fps;
	fps.guidFilter = guid;
	if (guidCategory == GUID_NULL)
		fps.bDMO = FALSE;
	else
		fps.bDMO = TRUE;
	fps.guidCategory = guidCategory;
	fps.nFilterID = -1;
	fps.strFilterName = lpszPluginName;
	fps.bEnable = TRUE;	//追加したときはフィルタは有効にしておく

	//ここでフィルタの追加を試みる
	HRESULT hr;
	if (fps.bDMO == TRUE)
	{
		hr = m_pDirectShow->AddDMOFilter(fps.guidFilter,fps.guidCategory,fps.strFilterName,&(fps.nFilterID));
	} else {
		hr = m_pDirectShow->AddFilter(fps.guidFilter,fps.strFilterName,&(fps.nFilterID));
	}
	if (FAILED(hr))
		return FALSE;	//生成できないエラー

	//フィルタの情報を確認
	vector<PinInfoStruct> pis;
	int nInputPinCount,nOutputPinCount;
	m_pDirectShow->GetFilterInfo(
		m_pDirectShow->GetFilterByID(fps.nFilterID),
		pis,&nInputPinCount,&nOutputPinCount);
	//入力ピンと出力ピンは最低1つずつ必要です
	if (nInputPinCount == 0 || nOutputPinCount == 0)
	{
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(fps.nFilterID));
		return FALSE;	//フィルタ自体の問題
	}
	if (bVideo)
		m_VideoPluginList.push_back(fps);
	else
		m_AudioPluginList.push_back(fps);
	return TRUE;
}
//////////////////////////////////////////////////
//プラグインの削除 (削除した後にvectorの更新が行われる)
//BOOL bVideo	ビデオプラグインかどうか
//int nIndex	プラグイン配列のインデックス
//////////////////////////////////////////////////
BOOL CDirectShowAccess::RemovePlugin(BOOL bVideo,int nIndex)
{
	assert(m_pDirectShow);
	assert(nIndex >= 0);
	if (bVideo) {
		if (nIndex >= (int)m_VideoPluginList.size())
			return FALSE;
	} else {
		if (nIndex >= (int)m_AudioPluginList.size())
			return FALSE;
	}
	FilterPluginStruct* pFilter = NULL;
	if (bVideo)
		pFilter = &(m_VideoPluginList[nIndex]);
	else
		pFilter = &(m_AudioPluginList[nIndex]);
	if (!pFilter)
		return FALSE;

	//フィルタの削除
	if (pFilter->nFilterID != -1) {
		m_pDirectShow->RemoveFilter(m_pDirectShow->GetFilterByID(pFilter->nFilterID));
		pFilter->nFilterID = -1;
	}

	//vectorの更新(新しい配列に確保される)
	vector<FilterPluginStruct> NewPluginList;
	if (bVideo) {
		for (int i=0;i<(int)m_VideoPluginList.size();i++)
		{
			if (i != nIndex)
				NewPluginList.push_back(m_VideoPluginList[i]);
		}
		m_VideoPluginList.clear();
		m_VideoPluginList = NewPluginList;
	} else {
		for (int i=0;i<(int)m_AudioPluginList.size();i++)
		{
			if (i != nIndex)
				NewPluginList.push_back(m_AudioPluginList[i]);
		}
		m_AudioPluginList.clear();
		m_AudioPluginList = NewPluginList;
	}
	return TRUE;
}
//////////////////////////////////////////////////
//フィルタ配列の入れ替え(上へ・下へ)
//BOOL bVideo		ビデオフィルタかどうか
//int nSrcIndex		入れ替え元
//int nDestIndex	入れ替え先
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SwitchPlugin(BOOL bVideo,int nSrcIndex,int nDestIndex)
{
	assert(m_pDirectShow);
	assert(nSrcIndex >= 0);
	assert(nDestIndex >= 0);
	if (bVideo) {
		if (nSrcIndex >= (int)m_VideoPluginList.size() || 
			nDestIndex >= (int)m_VideoPluginList.size())
			return FALSE;
	} else {
		if (nSrcIndex >= (int)m_AudioPluginList.size() ||
			nDestIndex >= (int)m_AudioPluginList.size())
			return FALSE;
	}

	FilterPluginStruct* pSrcFilter = NULL;
	FilterPluginStruct* pDestFilter = NULL;
	if (bVideo)
	{
		pSrcFilter = &(m_VideoPluginList[nSrcIndex]);
		pDestFilter = &(m_VideoPluginList[nDestIndex]);
	}
	else {
		pSrcFilter = &(m_AudioPluginList[nSrcIndex]);
		pDestFilter = &(m_AudioPluginList[nDestIndex]);
	}
	if (!pSrcFilter || !pDestFilter)
		return FALSE;

	FilterPluginStruct temp;
	temp = (*pSrcFilter);
	(*pSrcFilter) = (*pDestFilter);
	(*pDestFilter) = temp;

	return TRUE;
}

//////////////////////////////////////////////////
//プラグインの設定ダイアログを表示
//HWND hWnd		ダイアログを表示する親ウィドウハンドル
//BOOL bVideo	ビデオフィルタかどうか
//int nIndex	フィルタ番号
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ShowPluginDialog(HWND hWnd,BOOL bVideo,int nIndex)
{
	assert(m_pDirectShow);
	assert(nIndex >= 0);

	FilterPluginStruct* pPlugin = NULL;
	if (bVideo) {
		if (nIndex < (int)m_VideoPluginList.size())
			pPlugin = &(m_VideoPluginList[nIndex]);
	}
	else {
		if (nIndex < (int)m_AudioPluginList.size())
			pPlugin = &(m_AudioPluginList[nIndex]);
	}
	if (!pPlugin)
		return FALSE;
	if (pPlugin->nFilterID == -1)
		return FALSE;
	HRESULT hr = m_pDirectShow->ShowPropertyDialog(m_pDirectShow->GetFilterByID(pPlugin->nFilterID),hWnd);
	return SUCCEEDED(hr);
}






//////////////////////////////////////////////////
//コーデック関係
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//ビデオコーデックの選択
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SelectVideoCodecIndex(int nIndex)
{
	assert(m_pDirectShowEncoder);
	assert(nIndex >= 0 && nIndex < (int)m_VideoCodecList.size());

	CODEC_FORMAT_BITRATE bitrate = CODEC_FORMAT_CBR;		//ビットレートのタイプ(CODEC_FORMAT_CBR,CODEC_FORMAT_VBR)
	CODEC_FORMAT_NUMPASS numpass = CODEC_FORMAT_ONEPASS;	//1パス2パスエンコード(CODEC_FORMAT_ONEPASS,CODEC_FORMAT_TWOPASS)
	HRESULT hr = m_pDirectShowEncoder->SetVideoCodec(nIndex,bitrate,numpass);
	if (FAILED(hr))
		return FALSE;
	m_nVideoCodecIndex = nIndex;
	return TRUE;
}
//////////////////////////////////////////////////
//ビデオコーデックの設定その１
//dwBitrate			ビデオビットレート(bit per second)
//msBufferWindow	バッファリング時間(単位はms 標準で8秒ぐらい)
//double dFPS		フレームレート(単位はフレーム毎秒)
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SetVideoCodecSetting(const DWORD dwBitrate,const DWORD msBufferWindow,const double dFPS,const RECT rcSource,const RECT rcTarget)
{
	assert(m_pDirectShowEncoder);
	assert(m_nVideoCodecIndex != -1);
	REFERENCE_TIME AvgTimePerFrame = 0;
	AvgTimePerFrame = (REFERENCE_TIME) ((double)10000000 / (double)dFPS);

	HRESULT hr = m_pDirectShowEncoder->SetVideoSetting(dwBitrate,msBufferWindow,AvgTimePerFrame,rcSource,rcTarget);
	if (FAILED(hr))
		return FALSE;
	return TRUE;
}
//////////////////////////////////////////////////
//ビデオコーデックの設定その２
//dwQuality		滑らかさ～鮮明さ(0～100)
//nMaxKeyFrameSpacing キーフレームの間隔(単位はms)
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SetVideoCodecSetting2(DWORD dwQuality,int nMaxKeyFrameSpacing)
{
	assert(m_pDirectShowEncoder);
	assert(m_nVideoCodecIndex != -1);
	assert(dwQuality >= 0 && dwQuality <= 100);
	assert(nMaxKeyFrameSpacing > 0 && nMaxKeyFrameSpacing <= 30000);	//0～30秒まで
	REFERENCE_TIME MaxKeyFrameSpacing = nMaxKeyFrameSpacing * 10000;
	HRESULT hr = m_pDirectShowEncoder->SetVideoSetting2(dwQuality,MaxKeyFrameSpacing);
	if (FAILED(hr))
		return FALSE;
	return TRUE;
}
//////////////////////////////////////////////////
//ビデオコーデックの複雑さの設定(ビデオパフォーマンス)
//WORD wComplexity 0～5の範囲まで
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SetVideoComplexity(const WORD wComplexity)
{
	assert(m_pDirectShowEncoder);
	assert(m_nVideoCodecIndex != -1);
	assert(wComplexity >= 0 && wComplexity <= 5);
	HRESULT hr = m_pDirectShowEncoder->SetVideoComplexity(wComplexity);
	if (FAILED(hr))
		return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////
//オーディオコーデックの選択
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SelectAudioCodecIndex(int nIndex)
{
	assert(m_pDirectShowEncoder);
	assert(nIndex >= 0 && nIndex < (int)m_AudioCodecList.size());
	m_AudioFormatList.clear();

	CODEC_FORMAT_BITRATE bitrate = CODEC_FORMAT_CBR;		//ビットレートのタイプ(CODEC_FORMAT_CBR,CODEC_FORMAT_VBR)
	CODEC_FORMAT_NUMPASS numpass = CODEC_FORMAT_ONEPASS;	//1パス2パスエンコード(CODEC_FORMAT_ONEPASS,CODEC_FORMAT_TWOPASS)
	HRESULT hr = m_pDirectShowEncoder->EnumAudioCodecFormat(nIndex,bitrate,numpass,m_AudioFormatList);
	if (FAILED(hr))
		return FALSE;

	//TIPS この段階ではオーディオコーデックを選択しない
	m_nAudioCodecIndex = nIndex;
	m_nAudioCodecFormatIndex = -1;	//無効なフォーマット設定の代入
	return TRUE;
}
//////////////////////////////////////////////////
//オーディオコーデックフォーマットの選択
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SelectAudioCodecFormatIndex(int nIndex)
{
	assert(m_pDirectShowEncoder);
	assert(m_nAudioCodecIndex != -1);

	if (nIndex < 0 || nIndex >= (int)m_AudioFormatList.size())
	{
		m_nAudioCodecFormatIndex = -1;
		return FALSE;
	}
	//assert(nIndex >= 0 && nIndex < (int)m_AudioFormatList.size());

	//フォーマットの適用
	CODEC_FORMAT_BITRATE bitrate = CODEC_FORMAT_CBR;		//ビットレートのタイプ(CODEC_FORMAT_CBR,CODEC_FORMAT_VBR)
	CODEC_FORMAT_NUMPASS numpass = CODEC_FORMAT_ONEPASS;	//1パス2パスエンコード(CODEC_FORMAT_ONEPASS,CODEC_FORMAT_TWOPASS)
	HRESULT hr = m_pDirectShowEncoder->SetAudioCodec(m_nAudioCodecIndex,nIndex,bitrate,numpass);
	if (FAILED(hr))
		return FALSE;
	m_nAudioCodecFormatIndex = nIndex;
	return TRUE;
}



//////////////////////////////////////////////////
//エンコードの開始
//////////////////////////////////////////////////
BOOL CDirectShowAccess::StartEncode()
{
	if (m_bStartEncode)
		return FALSE;
	TRACE0("CDirectShowAccess::StartEncode()\n");

	assert(m_pDirectShow);
	assert(m_pDirectShowEncoder);
	assert(m_nVideoCodecIndex != -1);
	assert(m_nAudioCodecIndex != -1);
	assert(m_nAudioCodecFormatIndex != -1);

	HRESULT hr = NOERROR;

	//プレビュー不可の状態ならエンコード不可になる
	if (!IsCanPreview())
		return FALSE;

	//最大・最小パケットサイズの設定
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	if (pProfile->m_nOutputMaxPacketSize != 0 || pProfile->m_nOutputMinPacketSize != 0) {
		hr = m_pDirectShowEncoder->SetPacketSize(pProfile->m_nOutputMinPacketSize,pProfile->m_nOutputMaxPacketSize);
		if (FAILED(hr))
		{
			AfxMessageBox(_T("最大・最小パケットサイズの変更に失敗しました"),MB_OK|MB_ICONINFORMATION);
			return FALSE;
		}
	}

	//プロファイルの適用
	hr = m_pDirectShowEncoder->ApplyProfiles();
	if (FAILED(hr))	{
		AfxMessageBox(_T("エンコーダープロファイルの適用に失敗しました\r\n圧縮設定を見直してください"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

#ifdef DEBUG
	m_pDirectShowEncoder->SaveProfile(m_pDirectShowEncoder->GetProfiles(),_T("debug_profile.prx"));
#endif

	//フィルタの停止
	if (IsRunFilter())
		StopFilter();

	//テンポラリ出力パスの設定
	if (m_strAsfTempName.IsEmpty())
	{
		TCHAR strTempPath[MAX_PATH];
		GetTempPath(MAX_PATH,strTempPath);
		m_strAsfTempName = strTempPath;
		m_strAsfTempName += _T("kte_temp_asf.tmp");
	}

	//ASFライタフィルタが存在している場合は削除
	if (m_pDirectShowASFWriter) {
		m_pDirectShowASFWriter->Delete(m_pDirectShow->GetGraphBuilder());
		delete m_pDirectShowASFWriter;
		m_pDirectShowASFWriter = NULL;
	}
	//ASFライタフィルタの作成
	m_pDirectShowASFWriter = new CDirectShowASFWriter;
	hr = m_pDirectShowASFWriter->Create(m_pDirectShow->GetGraphBuilder(),m_strAsfTempName);
	if (FAILED(hr)) {
		AfxMessageBox(_T("ネットワーク出力の適用に失敗しました\r\n出力設定を見直してください"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//ASFライタにプロファイルの適用
	hr = m_pDirectShowASFWriter->ApplyProfiles(m_pDirectShowEncoder->GetProfiles());
	if (FAILED(hr)) {
		AfxMessageBox(_T("ネットワーク出力設定に圧縮情報の適用に失敗しました\r\n出力設定を見直してください"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//ASFライタの種々のパラメータの設定 (StartEncode()のサブルーチン)
	hr = SetAsfWriterParam();
	if (FAILED(hr)) {
		AfxMessageBox(_T("ネットワーク出力のパラメータ設定の適用に失敗しました\r\n出力・属性設定を見直してください"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	LPCWSTR lpszOutputPin  = NULL;
	vector<PinInfoStruct> PinInfo;
	m_pDirectShow->GetFilterInfo(
		m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),
		PinInfo,NULL,NULL);
	for (int i=0;i<(int)PinInfo.size();i++) {
		if (PinInfo[i].PinDir != PINDIR_OUTPUT)
			continue;
		if (PinInfo[i].bConnected == TRUE)
			continue;
		lpszOutputPin = PinInfo[i].strPinName.c_str();
		break;
	}
	//オーディオフィルタの接続
	hr = m_pDirectShow->ConnectFilterDirect(
		m_pDirectShow->GetFilterByID(m_nAudioSmartTeeID),
		m_pDirectShowASFWriter->GetFilter(),
		NULL/*&m_AudioLastMediaType*/,lpszOutputPin,L"Audio Input 01");
	if (FAILED(hr)){
		AfxMessageBox(_T("ネットワークライタへのオーディオフィルタ接続に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//ビデオフィルタの接続
	//空きの有限分岐フィルタ出力ピンを検索
	lpszOutputPin  = NULL;
	PinInfo.clear();
	m_pDirectShow->GetFilterInfo(
		m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),
		PinInfo,NULL,NULL);
	for (int i=0;i<(int)PinInfo.size();i++) {
		if (PinInfo[i].PinDir != PINDIR_OUTPUT)
			continue;
		if (PinInfo[i].bConnected == TRUE)
			continue;
		lpszOutputPin = PinInfo[i].strPinName.c_str();
		break;
	}
	hr = m_pDirectShow->ConnectFilterDirect(
		m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),
		m_pDirectShowASFWriter->GetFilter(),
		NULL/*&m_VideoLastMediaType*/,lpszOutputPin,L"Video Input 01");
	if (FAILED(hr)){
		AfxMessageBox(_T("ネットワークライタへのビデオフィルタ接続に失敗しました"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//入力接続のチェック
	int nInputCount = -1;
	CDirectShowEtc::CheckConnectionFilter(m_pDirectShowASFWriter->GetFilter(),&nInputCount,NULL);
	if (nInputCount != 2) {
		AfxMessageBox(_T("ネットワークライタへの接続に失敗しています"),MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	//グラフファイルのセーブ
	SaveGraphFile(_T("encode_start.grf"));

	//出力リストの追加(ここで失敗する可能性はある)
	if (!ApplyAllSink())
		return FALSE;
	
	//TODO テストコード
	//m_pDirectShowASFWriter->SetFixedFrameRate(FALSE);

	//プロセスプライオリティの変更
	CKTEApp* pApp = ((CKTEApp*)AfxGetApp());
	pApp->SetPriorityClass(pProfile->m_bProcessPriority,pProfile->m_nProcessPriority);

	//フィルタの再開
	if (!RunFilter())
	{
		AfxMessageBox(_T("エンコードの開始に失敗しました"));
		return FALSE;
	}
	//エンコード中のフラグが立つ
	m_bStartEncode = TRUE;
	return TRUE;
}

//////////////////////////////////////////////////
//ASFライタにパラメータの適用
//StartEncode()のサブルーチン
//////////////////////////////////////////////////
HRESULT CDirectShowAccess::SetAsfWriterParam()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	HRESULT hr = NOERROR;
	//バッファ時間の設定
	if (pProfile->m_nOutputSyncToleranceTime != 0)
	{
		hr = m_pDirectShowASFWriter->SetAsfSyncTolerance(pProfile->m_nOutputSyncToleranceTime);
		if (FAILED(hr))return hr;
	}
	//インデックスモード有効
	hr = m_pDirectShowASFWriter->SetAsfIndexMode(pProfile->m_bOutputInsertIndex);
	if (FAILED(hr))return hr;
	//終了時にインデックスを生成するか否か？
	if (pProfile->m_bOutputInsertIndex)
	{
		hr = m_pDirectShowASFWriter->SetAsfParam(AM_CONFIGASFWRITER_PARAM_AUTOINDEX,pProfile->m_bOutputInsertIndexEnd);
		if (FAILED(hr))return hr;
	}
	//2pass以上の場合はTRUEにする
	hr = m_pDirectShowASFWriter->SetAsfParam(AM_CONFIGASFWRITER_PARAM_MULTIPASS,FALSE);
	if (FAILED(hr)) return hr;
	//非圧縮モードを有効にするかどうか？
	hr = m_pDirectShowASFWriter->SetAsfParam(AM_CONFIGASFWRITER_PARAM_DONTCOMPRESS,FALSE);
	if (FAILED(hr)) return hr;

	//メタデータの設定
	m_pDirectShowASFWriter->SetTitle(m_strTitle);
	m_pDirectShowASFWriter->SetAuthor(m_strAuthor);
	m_pDirectShowASFWriter->SetDescription(m_strDescription);
	m_pDirectShowASFWriter->SetRating(m_strRating);
	m_pDirectShowASFWriter->SetCopyright(m_strCopyright);

	//コールバック用のウィンドウハンドルの設定
	assert(m_hNetworkCallbackWnd);
	if (m_hNetworkCallbackWnd)
		m_pDirectShowASFWriter->SetNetworkCallbackHWnd(m_hNetworkCallbackWnd);
	else
		return E_FAIL;
	return hr;
}
//////////////////////////////////////////////////
//全てのシンク(プル・プッシュ・ファイル)を適用する
//StartEncode()のサブルーチン
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ApplyAllSink()
{
	HRESULT hr = NOERROR;
	for (int i=0;i<(int)m_PullSinkList.size();i++) {
		if (m_PullSinkList[i].nID != -1 && m_PullSinkList[i].dwPort != 0) {
			hr = ApplyPullSink(i);
			if (FAILED(hr)) return FALSE;
		}
	}
	for (int i=0;i<(int)m_PushSinkList.size();i++) {
		if (m_PushSinkList[i].nID != -1 && !m_PushSinkList[i].strServerName.IsEmpty()) {
			hr = ApplyPushSink(i);
			if (FAILED(hr)) return FALSE;
		}
	}
	for (int i=0;i<(int)m_FileSinkList.size();i++) {
		if (m_FileSinkList[i].nID != -1 && !m_FileSinkList[i].strFileName.IsEmpty()) {
			hr = ApplyFileSink(i);
			if (FAILED(hr)) return FALSE;
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////
//エンコードの停止
//////////////////////////////////////////////////
BOOL CDirectShowAccess::StopEncode(BOOL bForceDisconnect)
{
	//if (!m_bStartEncode)	//エンコードが開始していない
	//	return FALSE;
	TRACE0("CDirectShowAccess::StopEncode()\n");

	assert(m_pDirectShow);

	//ネットワークの強制切断を実行
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	if (bForceDisconnect == TRUE || pProfile->m_bOutputForceDisconnect)
	{
		if (m_pDirectShowASFWriter)
			m_pDirectShowASFWriter->DeleteAllSink();
	}

	//フィルタの停止
	if (IsRunFilter())
		StopFilter();

	//ASFライタの削除
	//m_pDirectShow->SaveGraphFile(_T("encode_end_start.grf"));
	if (m_pDirectShowASFWriter)
	{
		m_pDirectShowASFWriter->Delete(m_pDirectShow->GetGraphBuilder());
		delete m_pDirectShowASFWriter;
		m_pDirectShowASFWriter = NULL;
	}
	//m_pDirectShow->SaveGraphFile(_T("encode_end_end.grf"));

	//テンポラリファイルの削除
	if (m_strAsfTempName.IsEmpty() != FALSE)
		DeleteFile(m_strAsfTempName);

	//プロセスプライオリティを元に戻す
	CKTEApp* pApp = (CKTEApp*)AfxGetApp();
	pApp->SetPriorityClass(FALSE,0);
	
	//プレビューの再開
	if (IsCanPreview())
		RunFilter();

	m_bStartEncode = FALSE;		//エンコード中のフラグが消える
	return TRUE;
}

//////////////////////////////////////////////////
//プル出力を追加
//////////////////////////////////////////////////
BOOL CDirectShowAccess::AddPullSink(DWORD dwPort,DWORD dwMax,int &nID)
{
	assert(dwPort > 0 && dwPort <= 655535);
	assert(dwMax > 0 && dwMax <= 50);
	BOOL bAdd = FALSE;
	for (int i = 0;i<(int)m_PullSinkList.size();i++) {
		if (m_PullSinkList[i].nID == -1) {
			m_PullSinkList[i].nID = i;
			m_PullSinkList[i].nRegistID = -1;
			m_PullSinkList[i].dwPort = dwPort;
			m_PullSinkList[i].dwMax = dwMax;
			nID = m_PullSinkList[i].nID;
			bAdd = TRUE;
			break;
		}
	}
	if (bAdd == FALSE) {
		PullSinkStruct pss;
		pss.nID = (int)m_PullSinkList.size();
		pss.nRegistID = -1;
		pss.dwPort = dwPort;
		pss.dwMax = dwMax;
		nID = pss.nID;
		m_PullSinkList.push_back(pss);
	}
	//エンコード中の場合は即時追加する
	if (m_pDirectShowASFWriter && IsEncode()) {
		HRESULT hr;
		hr = ApplyPullSink(nID);
		return SUCCEEDED(hr);
	}
	return TRUE;
}
//////////////////////////////////////////////////
//プル出力を削除
//////////////////////////////////////////////////
BOOL CDirectShowAccess::RemovePullSink(int nID)
{
	assert(nID >= 0);
	//エンコード中の場合はシンクを削除する
	if (m_pDirectShowASFWriter && IsEncode()) {
		int nRegistID = -1;
		for (int i=0;i<(int)m_PullSinkList.size();i++)
		{
			if (m_PullSinkList[i].nID == nID) {
				nRegistID = m_PullSinkList[i].nRegistID;
				break;
			}
		}
		if (nRegistID != -1) {
			HRESULT hr = NOERROR;
			hr = m_pDirectShowASFWriter->RemoveNetworkSink(nRegistID);
			assert(SUCCEEDED(hr));
		}
	}
	//リストから削除
	for (int i=0;i<(int)m_PullSinkList.size();i++) {
		if (m_PullSinkList[i].nID == nID) {
			m_PullSinkList[i].nID = -1;
			m_PullSinkList[i].nRegistID = -1;
			m_PullSinkList[i].dwPort = 0;
			m_PullSinkList[i].dwMax = 0;
			return TRUE;
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////
//プル出力の適用
//////////////////////////////////////////////////
HRESULT CDirectShowAccess::ApplyPullSink(int nIndex)
{
	assert(m_pDirectShowASFWriter);
	assert(nIndex >= 0 && nIndex <(int)m_PullSinkList.size());
	if (m_PullSinkList[nIndex].nID == -1)
		return E_FAIL;

	HRESULT hr = NOERROR;
	m_PullSinkList[nIndex].nRegistID = -1;
	//ネットワークシンクの追加
	hr = m_pDirectShowASFWriter->AddNetworkSink(
		&(m_PullSinkList[nIndex].dwPort),
		m_PullSinkList[nIndex].dwMax,
		&(m_PullSinkList[nIndex].nRegistID));
	if (FAILED(hr))
	{
		CString strMessage;
		strMessage.Format(_T("ブロードバンド出力の登録に失敗しました。\nポートが重複している可能性があります。\nポート番号:%d"),
			m_PullSinkList[nIndex].dwPort);
		AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		RemovePullSink(m_PullSinkList[nIndex].nID);
	}
	return hr;
}
//////////////////////////////////////////////////
//プル出力を変更(最大接続人数の変更)
//////////////////////////////////////////////////
BOOL CDirectShowAccess::ChangePullSink(int nID,DWORD dwMax)
{
	assert(nID >= 0);
	if (m_pDirectShowASFWriter && IsEncode()) {
		int nRegistID = -1;
		for (int i=0;i<(int)m_PullSinkList.size();i++)
		{
			if (m_PullSinkList[i].nID == nID) {
				nRegistID = m_PullSinkList[i].nRegistID;
				break;
			}
		}
		if (nRegistID != -1) {
			HRESULT hr = NOERROR;
			hr = m_pDirectShowASFWriter->ChangeNetworkSink(nRegistID,dwMax);
			assert(SUCCEEDED(hr));
			return SUCCEEDED(hr);
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////
//プル出力の情報を取得
//////////////////////////////////////////////////
BOOL CDirectShowAccess::GetPullSink(int nIndex,DWORD dwIPAddress,DWORD dwPort,NetworkClientStruct &Client,DWORD &dwSrcPort)
{
	assert(nIndex >= 0);
	if (m_pDirectShowASFWriter && IsEncode()) {

		for (int i=0;i<(int)m_PullSinkList.size();i++)
		{
			if (m_PullSinkList[i].nRegistID == nIndex)
			{
				HRESULT hr = NOERROR;
				dwSrcPort = m_PullSinkList[i].dwPort;
				hr = m_pDirectShowASFWriter->GetNetworkSink(nIndex,dwIPAddress,dwPort,Client);
				return SUCCEEDED(hr);
			}
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////
//プル出力が正当なデータであるのかチェックする
//これは無効なコネクションが来た場合のチェック機構である
//////////////////////////////////////////////////
BOOL CDirectShowAccess::IsVaildPullSink(int nIndex)
{
	assert(nIndex >= 0);
	if (m_pDirectShowASFWriter && IsEncode()) {
		for (int i=0;i<(int)m_PullSinkList.size();i++)
		{
			if (m_PullSinkList[i].nRegistID == nIndex)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////
//プッシュ出力を追加
//////////////////////////////////////////////////
BOOL CDirectShowAccess::AddPushSink(CString strServerName,BOOL bAutoDelete,int &nID)
{
	assert(strServerName.Compare(_T("")) != 0 && strServerName.IsEmpty() != TRUE);
	BOOL bAdd = FALSE;
	for (int i = 0;i<(int)m_PushSinkList.size();i++) {
		if (m_PushSinkList[i].nID == -1) {
			m_PushSinkList[i].nID = i;
			m_PushSinkList[i].nRegistID = -1;
			m_PushSinkList[i].strServerName = strServerName;
			m_PushSinkList[i].bAutoDelete = bAutoDelete;
			nID = m_PushSinkList[i].nID;
			bAdd = TRUE;
			break;
		}
	}
	if (bAdd == FALSE) {
		PushSinkStruct pss;
		pss.nID = (int)m_PushSinkList.size();
		pss.nRegistID = -1;
		pss.strServerName = strServerName;
		pss.bAutoDelete = bAutoDelete;
		nID = pss.nID;
		m_PushSinkList.push_back(pss);
	}
	//エンコード中の場合は即時追加する
	if (m_pDirectShowASFWriter && IsEncode()) {
		HRESULT hr;
		hr = ApplyPushSink(nID);
		return SUCCEEDED(hr);
	}
	return TRUE;
}
//////////////////////////////////////////////////
//プッシュ出力を削除
//////////////////////////////////////////////////
BOOL CDirectShowAccess::RemovePushSink(int nID)
{
	assert(nID >= 0);
	//エンコード中の場合はシンクを削除する
	if (m_pDirectShowASFWriter && IsEncode()) {
		int nRegistID = -1;
		for (int i=0;i<(int)m_PushSinkList.size();i++)
		{
			if (m_PushSinkList[i].nID == nID) {
				nRegistID = m_PushSinkList[i].nRegistID;
				break;
			}
		}
		if (nRegistID != -1) {
			HRESULT hr = NOERROR;
			hr = m_pDirectShowASFWriter->RemovePushSink(nRegistID);
			assert(SUCCEEDED(hr));
		}
	}
	//リストから削除
	for (int i=0;i<(int)m_PushSinkList.size();i++) {
		if (m_PushSinkList[i].nID == nID) {
			m_PushSinkList[i].nID = -1;
			m_PushSinkList[i].nRegistID = -1;
			m_PushSinkList[i].strServerName.Empty();
			m_PushSinkList[i].bAutoDelete = FALSE;
			return TRUE;
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////
//プッシュ出力の適用
//////////////////////////////////////////////////
HRESULT CDirectShowAccess::ApplyPushSink(int nIndex)
{
	assert(m_pDirectShowASFWriter);
	assert(nIndex >= 0 && nIndex <(int) m_PushSinkList.size());
	if (m_PushSinkList[nIndex].nID == -1)
		return E_FAIL;
	if (m_PushSinkList[nIndex].strServerName.IsEmpty())
		return E_FAIL;

	HRESULT hr = NOERROR;
	m_PushSinkList[nIndex].nRegistID = -1;
	hr = m_pDirectShowASFWriter->AddPushSink(
		m_PushSinkList[nIndex].strServerName,
		NULL,
		m_PushSinkList[nIndex].bAutoDelete,
		&(m_PushSinkList[nIndex].nRegistID));
	if (FAILED(hr))
	{
		CString strMessage;
		strMessage.Format(_T("パブリッシュポイント出力の登録に失敗しました。\n無効なサーバー名又はサーバーへ接続できない状態にあります。\n%s"),
			m_PushSinkList[nIndex].strServerName);
		AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		RemovePushSink(m_PushSinkList[nIndex].nID);
	}
	return hr;
}

//////////////////////////////////////////////////
//ファイルに出力を追加
//////////////////////////////////////////////////
BOOL CDirectShowAccess::AddFileSink(CString strFileName,int &nID)
{
	assert(strFileName.Compare(_T("")) != 0 && strFileName.IsEmpty() != TRUE);
	BOOL bAdd = FALSE;
	for (int i = 0;i<(int)m_FileSinkList.size();i++) {
		if (m_FileSinkList[i].nID == -1) {
			m_FileSinkList[i].nID = i;
			m_FileSinkList[i].nRegistID = -1;
			m_FileSinkList[i].strFileName = strFileName;
			nID = m_FileSinkList[i].nID;
			bAdd = TRUE;
			break;
		}
	}
	if (bAdd == FALSE) {
		FileSinkStruct fss;
		fss.nID = (int)m_FileSinkList.size();
		fss.nRegistID = -1;
		fss.strFileName = strFileName;
		nID = fss.nID;
		m_FileSinkList.push_back(fss);
	}

	//エンコード中の場合は即時追加する
	if (m_pDirectShowASFWriter && IsEncode()) {
		HRESULT hr;
		hr = ApplyFileSink(nID);
		return SUCCEEDED(hr);
	}
	return TRUE;
}
//////////////////////////////////////////////////
//ファイルに出力を削除
//////////////////////////////////////////////////
BOOL CDirectShowAccess::RemoveFileSink(int nID)
{
	assert(nID >= 0);
	//エンコード中の場合はシンクを削除する
	if (m_pDirectShowASFWriter && IsEncode()) {
		int nRegistID = -1;
		for (int i=0;i<(int)m_FileSinkList.size();i++)
		{
			if (m_FileSinkList[i].nID == nID) {
				nRegistID = m_FileSinkList[i].nRegistID;
				break;
			}
		}
		if (nRegistID != -1) {
			HRESULT hr = NOERROR;
			hr = m_pDirectShowASFWriter->RemoveFileSink(nRegistID);
			assert(SUCCEEDED(hr));
		}
	}
	//リストから削除(無効な情報にしておく)
	for (int i=0;i<(int)m_FileSinkList.size();i++) {
		if (m_FileSinkList[i].nID == nID) {
			m_FileSinkList[i].nID = -1;
			m_FileSinkList[i].nRegistID = -1;
			m_FileSinkList[i].strFileName.Empty();
			return TRUE;
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////
//ファイルに出力の適用
//////////////////////////////////////////////////
HRESULT CDirectShowAccess::ApplyFileSink(int nIndex)
{
	assert(m_pDirectShowASFWriter);
	assert(nIndex >= 0 && nIndex <(int) m_FileSinkList.size());
	if (m_FileSinkList[nIndex].nID == -1)
		return E_FAIL;
	if (m_FileSinkList[nIndex].strFileName.IsEmpty())
		return E_FAIL;

	HRESULT hr = NOERROR;
	m_FileSinkList[nIndex].nRegistID = -1;
	hr = m_pDirectShowASFWriter->AddFileSink(m_FileSinkList[nIndex].strFileName,&(m_FileSinkList[nIndex].nRegistID));
	if (FAILED(hr))
	{
		CString strMessage;
		strMessage.Format(_T("ファイル出力の登録に失敗しました。パスの確認をしてください。\n%s"),m_FileSinkList[nIndex].strFileName);
		AfxMessageBox(strMessage,MB_OK|MB_ICONINFORMATION);
		RemoveFileSink(m_FileSinkList[nIndex].nID);
	}
	return hr;
}

//////////////////////////////////////////////////
//統計情報を取得
//////////////////////////////////////////////////
BOOL CDirectShowAccess::GetStatistics(QWORD &cnsCurrentTime,WM_WRITER_STATISTICS &Stats,WM_WRITER_STATISTICS_EX &StatsEx)
{
	assert(m_pDirectShowASFWriter);
	HRESULT hr = m_pDirectShowASFWriter->GetStatistics(cnsCurrentTime,Stats,StatsEx);
	return SUCCEEDED(hr);
}

//////////////////////////////////////////////////
//メタデータを設定
//////////////////////////////////////////////////
BOOL CDirectShowAccess::SetMetadata(CString &strTitle,CString &strAuthor,CString &strDescription,CString &strRating,CString &strCopyright)
{
	//このタイミングでは値の格納のみしかできない(設定はStartEncode()で行う)
	m_strTitle = strTitle;
	m_strAuthor = strAuthor;
	m_strDescription = strDescription;
	m_strRating = strRating;
	m_strCopyright = strCopyright;
	return TRUE;
}



