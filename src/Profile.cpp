#include "StdAfx.h"
#include "Profile.h"
#include "IniFile.h"

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CProfile::CProfile(void)
{
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CProfile::~CProfile(void)
{
}

//////////////////////////////////////////////////
//初期値を設定
//////////////////////////////////////////////////
void CProfile::DefaultProfile()
{
	//入力タブの設定
	m_strVideoName        = _T("SCFH DSF");
	m_strVideoSettingName = _T("");
	m_nVideoSettingIndex  = -1;
	m_nVideoSettingWidth = -1;
	m_nVideoSettingHeight = -1;
	m_dVideoSettingFPS = 0.0;

	m_strAudioName        = _T("");
	m_strAudioSettingName = _T("");
	m_nAudioSettingIndex = -1;
	m_bAudioInputVideoPin = FALSE;
	m_nSamplesPerSec = 44100;
	m_wBitsPerSample = 16;
	m_nChannels = 2;
	m_bAudioBufferEnable = TRUE;
	m_nAudioBufferTime = 50;
	m_nAudioBufferCount = -1;

	m_bGraphClockEnable = TRUE;
	m_nGraphClockSetting = 0;
	m_bGraphSyncEnable = FALSE;

	//プラグインの設定
	m_nVideoPluginCount = 0;
	m_VideoPluginList.clear();
	m_nAudioPluginCount = 0;
	m_AudioPluginList.clear();

	//圧縮タブの設定(ビデオエンコード)
	m_strVideoEncodeName = _T("Windows Media Video 9");
	m_nVideoEnocdeBitrate = 512;//前バージョンでは256
	m_dVideoEncodeFramerate = 30.0;
	m_dVideoEncodeMaxFramerateSetting = 30.0;
	m_bVideoEncodeSize = FALSE;
	m_nVideoEncodeWidth = 640;
	m_nVideoEncodeHeight = 480;
	m_nVideoEnocdeQuality = 75;	//前バージョンでは50
	m_nBufferWindow = 5000;		//前バージョンでは8000
	m_nMaxKeyFrameSpacing = 8000;
	m_bMaxKeyFrameSpacing = TRUE;
	m_nVideoComplexity = 0;
	m_bVideoComplexity = TRUE;

	//圧縮タブの設定(オーディオエンコード)
	m_strAudioEncodeName = _T("Windows Media Audio 9.2");
	//m_AudioEncodeDescription = _T("");
	m_nAudioEncodeBitrate = 64040;
	m_nAudioEncodeChannels = 2;
	m_nAudioEncodeBitsPerSample = 16;
	m_nAudioEncodeSamplesPerSec = 44100;
	m_bAudioEncodeAVSynchronization = TRUE;

	//初期データが存在しない場合は、デフォルトの初期の出力先一覧を生成する
	m_bOutputPullCheck = TRUE;
	m_bOutputPushCheck = FALSE;
	m_bOutputFileCheck = TRUE;
	m_nOutputPullCount = 1;
	m_nOutputPushCount = 1;
	m_nOutputFileCount = 1;
	//プル配信
	IniOutputPullStruct pull;
	pull.Data.nIndex = 0;
	pull.Data.bTabEnable = TRUE;
	pull.Data.strTabName = _T("プル1");
	pull.nPort = 8080;
	pull.nMax = 10;
	m_OutputPullList.push_back(pull);
	//プッシュ配信
	IniOutputPushStruct push;
	push.Data.nIndex = 0;
	push.Data.bTabEnable = FALSE;
	push.Data.strTabName = _T("プッシュ1");
	push.strServerName = _T("");
	push.bAutoDelete = FALSE;
	m_OutputPushList.push_back(push);
	//ファイル出力
	IniOutputFileStruct file;
	file.Data.nIndex = 0;
	file.Data.bTabEnable = TRUE;
	file.Data.strTabName = _T("ファイル1");
	TCHAR SpecialPath[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_MYVIDEO,FALSE) != TRUE)
		SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_DESKTOP,FALSE);
	file.strFileName = _T("Live %Y年%m月%d日%H時%M分%S秒.asf");
	file.strFolderName = SpecialPath;
	m_OutputFileList.push_back(file);

	//出力タブの詳細設定
	m_nOutputMaxPacketSize = 0;
	m_nOutputMinPacketSize = 0;
	m_bOutputInsertIndex = TRUE;
	m_bOutputInsertIndexEnd = TRUE;
	m_nOutputSyncToleranceTime = 3000;
	m_bOutputForceDisconnect = FALSE;

	//属性タブの設定
	m_strMetadataTitle = _T("");
	m_strMetadataAuthor = _T("");
	m_strMetadataCopyright = _T("");
	m_strMetadataRating = _T("");
	m_strMetadataDescription = _T("");

	//パフォーマンス設定
	m_bProcessPriority = FALSE;
	m_nProcessPriority = 2;
	m_bProcessLimitEnable = FALSE;
	m_bProcessAffinityPriority = TRUE;
	m_dwProcessAffinityMask = 0xFFFFFFFF;
	m_nProcessIdealProcessor = 0;
}

//////////////////////////////////////////////////
//プロファイルの読み込み
//////////////////////////////////////////////////
BOOL CProfile::LoadProfile(LPCTSTR lpszProfileName,BOOL bFirstFlag)
{
	CIniFile Ini;
	if (!Ini.Open(lpszProfileName))
	{
		//AfxMessageBox(_T("プロファイルを開くのに失敗しました"),MB_OK|MB_ICONINFORMATION);
		//DefaultProfile();
		Ini.Close();
		return FALSE;
	}

	//初回起動時にファイルが存在しなかった場合
	if (bFirstFlag == TRUE && Ini.IsFind() == FALSE)
	{
		//DefaultProfile();
		Ini.Close();
		return FALSE;
	}
	//入力タブの設定
	m_strVideoName = Ini.GetProfileString(_T("Input"),_T("VideoName"),_T("SCFH DSF"));
	m_strVideoSettingName = Ini.GetProfileString(_T("Input"),_T("VideoSettingName"),_T(""));
	m_nVideoSettingIndex = Ini.GetProfileInt(_T("Input"),_T("VideoSettingIndex"),-1);
	m_nVideoSettingWidth = Ini.GetProfileInt(_T("Input"),_T("VideoSettingWidth"),-1);
	m_nVideoSettingHeight = Ini.GetProfileInt(_T("Input"),_T("VideoSettingHeiht"),-1);
	m_dVideoSettingFPS = Ini.GetProfileDouble(_T("Input"),_T("VideoSettingFPS"),0.0);

	m_strAudioName = Ini.GetProfileString(_T("Input"),_T("AudioName"),_T(""));
	m_strAudioSettingName = Ini.GetProfileString(_T("Input"),_T("AudioSettingName"),_T(""));
	m_nAudioSettingIndex = Ini.GetProfileInt(_T("Input"),_T("AudioSettingIndex"),-1);
	m_bAudioInputVideoPin = Ini.GetProfileBOOL(_T("Input"),_T("AudioInputVideoPin"),FALSE);
	m_nSamplesPerSec = (DWORD)Ini.GetProfileInt(_T("Input"),_T("SamplesPerSec"),44100);
	m_wBitsPerSample = (WORD)Ini.GetProfileInt(_T("Input"),_T("BitsPerSample"),16);
	m_nChannels = (WORD)Ini.GetProfileInt(_T("Input"),_T("Channels"),2);
	m_bAudioBufferEnable = Ini.GetProfileBOOL(_T("Input"),_T("AudioBuffer"),TRUE);
	m_nAudioBufferTime = Ini.GetProfileInt(_T("Input"),_T("AudioBufferTime"),50);
	m_nAudioBufferCount = Ini.GetProfileInt(_T("Input"),_T("AudioBufferCount"),-1);

	m_bGraphClockEnable = Ini.GetProfileBOOL(_T("Input"),_T("GraphClockEnable"),TRUE);
	m_nGraphClockSetting = Ini.GetProfileInt(_T("Input"),_T("GraphClockSetting"),0);
	m_bGraphSyncEnable =  Ini.GetProfileBOOL(_T("Input"),_T("GraphSyncEnable"),FALSE);

	//プラグインの設定
	CString strKeyName;
	//ビデオプラグイン
	m_nVideoPluginCount = Ini.GetProfileInt(_T("VideoPlugin"),_T("Count"),0);
	m_VideoPluginList.clear();
	for (int i=0;i<(int)m_nVideoPluginCount;i++)
	{
		IniPluginStruct ips;
		strKeyName.Format(_T("%d_Name"),i);
		ips.strFilterName = Ini.GetProfileString(_T("VideoPlugin"),strKeyName,_T(""));
		strKeyName.Format(_T("%d_GUID"),i);
		Ini.GetProfileGUID(_T("VideoPlugin"),strKeyName,GUID_NULL,ips.guid);
		strKeyName.Format(_T("%d_GUID_Category"),i);
		Ini.GetProfileGUID(_T("VideoPlugin"),strKeyName,GUID_NULL,ips.guidCategory);
		strKeyName.Format(_T("%d_Enable"),i);
		ips.bEnable = Ini.GetProfileBOOL(_T("VideoPlugin"),strKeyName,TRUE);
		if (ips.guid != GUID_NULL && !ips.strFilterName.IsEmpty())
			m_VideoPluginList.push_back(ips);
	}
	//オーディオプラグイン
	m_nAudioPluginCount = Ini.GetProfileInt(_T("AudioPlugin"),_T("Count"),0);
	m_AudioPluginList.clear();
	for (int i=0;i<(int)m_nAudioPluginCount;i++)
	{
		IniPluginStruct ips;
		strKeyName.Format(_T("%d_Name"),i);
		ips.strFilterName = Ini.GetProfileString(_T("AudioPlugin"),strKeyName,_T(""));
		strKeyName.Format(_T("%d_GUID"),i);
		Ini.GetProfileGUID(_T("AudioPlugin"),strKeyName,GUID_NULL,ips.guid);
		strKeyName.Format(_T("%d_GUID_Category"),i);
		Ini.GetProfileGUID(_T("AudioPlugin"),strKeyName,GUID_NULL,ips.guidCategory);
		strKeyName.Format(_T("%d_Enable"),i);
		ips.bEnable = Ini.GetProfileBOOL(_T("AudioPlugin"),strKeyName,TRUE);
		if (ips.guid != GUID_NULL && !ips.strFilterName.IsEmpty())
			m_AudioPluginList.push_back(ips);
	}

	//圧縮タブの設定(ビデオエンコード)
	m_strVideoEncodeName = Ini.GetProfileString(_T("VideoEncode"),_T("EncoderName"),_T("Windows Media Video 9"));
	m_nVideoEnocdeBitrate = Ini.GetProfileInt(_T("VideoEncode"),_T("Bitrate"),512);
	m_dVideoEncodeFramerate = Ini.GetProfileDouble(_T("VideoEncode"),_T("Framerate"),30.0);
	m_dVideoEncodeMaxFramerateSetting = Ini.GetProfileDouble(_T("VideoEncode"),_T("MaxFramerateSetting"),30.0);
	m_bVideoEncodeSize = Ini.GetProfileBOOL(_T("VideoEncode"),_T("EncodeSize"),FALSE);
	m_nVideoEncodeWidth = Ini.GetProfileInt(_T("VideoEncode"),_T("EncodeWidth"),640);
	m_nVideoEncodeHeight = Ini.GetProfileInt(_T("VideoEncode"),_T("EncodeHeight"),480);
	m_nVideoEnocdeQuality = Ini.GetProfileInt(_T("VideoEncode"),_T("Quality"),75);
	m_nBufferWindow = Ini.GetProfileInt(_T("VideoEncode"),_T("BufferWindow"),5000);
	m_nMaxKeyFrameSpacing = Ini.GetProfileInt(_T("VideoEncode"),_T("MaxKeyFrameSpacing"),8000);
	m_bMaxKeyFrameSpacing = Ini.GetProfileBOOL(_T("VideoEncode"),_T("AutoMaxKeyFrameSpacing"),TRUE);
	m_nVideoComplexity = Ini.GetProfileInt(_T("VideoEncode"),_T("Complexity"),0);
	m_bVideoComplexity = Ini.GetProfileBOOL(_T("VideoEncode"),_T("AutoComplexity"),TRUE);

	//圧縮タブの設定(オーディオエンコード)
	m_strAudioEncodeName = Ini.GetProfileString(_T("AudioEncode"),_T("EncoderName"),_T("Windows Media Audio 9.2"));
	//m_AudioEncodeDescription = Ini.GetProfileString(_T("AudioEncode"),_T("EncoderDescription"),_T(""));
	m_nAudioEncodeBitrate = Ini.GetProfileInt(_T("AudioEncode"),_T("Bitrate"),64040);
	m_nAudioEncodeChannels = Ini.GetProfileInt(_T("AudioEncode"),_T("Channels"),2);
	m_nAudioEncodeBitsPerSample = Ini.GetProfileInt(_T("AudioEncode"),_T("BitsPerSample"),16);
	m_nAudioEncodeSamplesPerSec = Ini.GetProfileInt(_T("AudioEncode"),_T("SamplesPerSec"),44100);
	m_bAudioEncodeAVSynchronization = Ini.GetProfileBOOL(_T("AudioEncode"),_T("AVSynchronization"),TRUE);

	//出力タブの設定
	m_bOutputPullCheck = Ini.GetProfileBOOL(_T("Output"),_T("PullCheck"),TRUE);
	m_bOutputPushCheck = Ini.GetProfileBOOL(_T("Output"),_T("PushCheck"),TRUE);
	m_bOutputFileCheck = Ini.GetProfileBOOL(_T("Output"),_T("FileCheck"),TRUE);
	m_nOutputPullCount = Ini.GetProfileInt(_T("Output"),_T("PullCount"),0);
	m_nOutputPushCount = Ini.GetProfileInt(_T("Output"),_T("PushCount"),0);
	m_nOutputFileCount = Ini.GetProfileInt(_T("Output"),_T("FileCount"),0);
	CString strSection;
	m_OutputPullList.clear();
	for (int i=1;i<=m_nOutputPullCount;i++)
	{
		strSection.Format(_T("Pull%d"),i);
		IniOutputPullStruct iops;
		iops.Data.nIndex = i - 1;
		iops.Data.bTabEnable = Ini.GetProfileBOOL(strSection,_T("TabEnable"),TRUE);
		iops.Data.strTabName = Ini.GetProfileString(strSection,_T("TabName"),_T("プル"));
		iops.nPort = Ini.GetProfileInt(strSection,_T("Port"),8080);
		iops.nMax = Ini.GetProfileInt(strSection,_T("Max"),10);
 		m_OutputPullList.push_back(iops);
 	}
	m_OutputPushList.clear();
	for (int i=1;i<=m_nOutputPushCount;i++)
	{
		strSection.Format(_T("Push%d"),i);
		IniOutputPushStruct iops;
		iops.Data.nIndex = i - 1;
		iops.Data.bTabEnable = Ini.GetProfileBOOL(strSection,_T("TabEnable"),TRUE);
		iops.Data.strTabName = Ini.GetProfileString(strSection,_T("TabName"),_T("プッシュ"));
		iops.strServerName = Ini.GetProfileString(strSection,_T("ServerName"),_T(""));
		iops.bAutoDelete = Ini.GetProfileBOOL(strSection,_T("AutoDelete"),TRUE);
 		m_OutputPushList.push_back(iops);
 	}
	m_OutputFileList.clear();
	for (int i=1;i<=m_nOutputFileCount;i++)
	{
		strSection.Format(_T("File%d"),i);
		IniOutputFileStruct iofs;
		iofs.Data.nIndex = i - 1;
		iofs.Data.bTabEnable = Ini.GetProfileBOOL(strSection,_T("TabEnable"),TRUE);
		iofs.Data.strTabName = Ini.GetProfileString(strSection,_T("TabName"),_T("ファイル"));
		iofs.strFileName = Ini.GetProfileString(strSection,_T("FileName"),_T(""));
		iofs.strFolderName = Ini.GetProfileString(strSection,_T("FolderName"),_T(""));
 		m_OutputFileList.push_back(iofs);
 	}

	//初期データが存在しない場合は、デフォルトの初期の出力先一覧を生成する
	if (Ini.IsFind() == FALSE)
	{
		m_bOutputPullCheck = TRUE;
		m_bOutputPushCheck = FALSE;
		m_bOutputFileCheck = TRUE;
		m_nOutputPullCount = 1;
		m_nOutputPushCount = 1;
		m_nOutputFileCount = 1;
		IniOutputPullStruct pull;
		pull.Data.nIndex = 0;
		pull.Data.bTabEnable = TRUE;
		pull.Data.strTabName = _T("プル1");
		pull.nPort = 8080;
		pull.nMax = 10;
		m_OutputPullList.clear();
		m_OutputPullList.push_back(pull);
		IniOutputPushStruct push;
		push.Data.nIndex = 0;
		push.Data.bTabEnable = FALSE;
		push.Data.strTabName = _T("プッシュ1");
		push.strServerName = _T("");
		push.bAutoDelete = FALSE;
		m_OutputPushList.clear();
		m_OutputPushList.push_back(push);
		IniOutputFileStruct file;
		file.Data.nIndex = 0;
		file.Data.bTabEnable = TRUE;
		file.Data.strTabName = _T("ファイル1");
		TCHAR SpecialPath[MAX_PATH];
		if (SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_MYVIDEO,FALSE) != TRUE)
			SHGetSpecialFolderPath(NULL,SpecialPath,CSIDL_DESKTOP,FALSE);
		file.strFileName = _T("Live %Y年%m月%d日%H時%M分%S秒.asf");
		file.strFolderName = SpecialPath;
		m_OutputFileList.clear();
		m_OutputFileList.push_back(file);
	}

	//出力タブの詳細設定
	m_nOutputMaxPacketSize = Ini.GetProfileInt(_T("OutputOption"),_T("MaxPacketSize"),0);
	m_nOutputMinPacketSize = Ini.GetProfileInt(_T("OutputOption"),_T("MinPacketSize"),0);
	m_bOutputInsertIndex = Ini.GetProfileBOOL(_T("OutputOption"),_T("InsertIndex"),TRUE);
	m_bOutputInsertIndexEnd = Ini.GetProfileBOOL(_T("OutputOption"),_T("InsertIndexEnd"),TRUE);
	m_nOutputSyncToleranceTime = Ini.GetProfileInt(_T("OutputOption"),_T("SyncToleranceTime"),3000);
	m_bOutputForceDisconnect = Ini.GetProfileBOOL(_T("OutputOption"),_T("ForceDisconnect"),FALSE);

	//属性タブの設定
	m_strMetadataTitle = Ini.GetProfileString(_T("Metadata"),_T("Title"),_T(""));
	m_strMetadataAuthor = Ini.GetProfileString(_T("Metadata"),_T("Author"),_T(""));
	m_strMetadataCopyright = Ini.GetProfileString(_T("Metadata"),_T("Copyright"),_T(""));
	m_strMetadataRating = Ini.GetProfileString(_T("Metadata"),_T("Rating"),_T(""));
	m_strMetadataDescription = Ini.GetProfileString(_T("Metadata"),_T("Description"),_T(""));

	//CPU設定
	m_bProcessPriority = Ini.GetProfileBOOL(_T("CPU"),_T("EnableProcessPriority"),FALSE);
	m_nProcessPriority = Ini.GetProfileInt(_T("CPU"),_T("ProcessPriority"),2);
	m_bProcessLimitEnable = Ini.GetProfileBOOL(_T("CPU"),_T("ProcessLimitEnable"),FALSE);
	m_bProcessAffinityPriority = Ini.GetProfileBOOL(_T("CPU"),_T("ProcessAffinityPriority"),TRUE);
	m_dwProcessAffinityMask = Ini.GetProfileInt(_T("CPU"),_T("ProcessAffinityMask"),0xFFFFFFFF);
	m_nProcessIdealProcessor = Ini.GetProfileInt(_T("CPU"),_T("ProcessIdealProcessor"),0);

	//INI編集終了
	Ini.Close();
	return TRUE;
}

//////////////////////////////////////////////////
//プロファイルの書き込み
//////////////////////////////////////////////////
BOOL CProfile::SaveProfile(LPCTSTR lpszProfileName)
{
	CIniFile Ini;
	if (!Ini.Open(lpszProfileName))
	{
		AfxMessageBox(_T("プロファイルを開くのに失敗しました"),MB_OK|MB_ICONINFORMATION);
		Ini.Close();
		return FALSE;
	}

	//入力タブの設定
	Ini.WriteProfileString(_T("Input"),_T("VideoName"),m_strVideoName);
	Ini.WriteProfileString(_T("Input"),_T("VideoSettingName"),m_strVideoSettingName);
	Ini.WriteProfileInt(_T("Input"),_T("VideoSettingIndex"),m_nVideoSettingIndex);
	Ini.WriteProfileInt(_T("Input"),_T("VideoSettingWidth"),m_nVideoSettingWidth);
	Ini.WriteProfileInt(_T("Input"),_T("VideoSettingHeiht"),m_nVideoSettingHeight);
	Ini.WriteProfileDouble(_T("Input"),_T("VideoSettingFPS"),(float)m_dVideoSettingFPS);

	Ini.WriteProfileString(_T("Input"),_T("AudioName"),m_strAudioName);
	Ini.WriteProfileString(_T("Input"),_T("AudioSettingName"),m_strAudioSettingName);
	Ini.WriteProfileInt(_T("Input"),_T("AudioSettingIndex"),m_nAudioSettingIndex);
	Ini.WriteProfileBOOL(_T("Input"),_T("AudioInputVideoPin"),m_bAudioInputVideoPin);
	Ini.WriteProfileInt(_T("Input"),_T("SamplesPerSec"),(int)m_nSamplesPerSec);
	Ini.WriteProfileInt(_T("Input"),_T("BitsPerSample"),(int)m_wBitsPerSample);
	Ini.WriteProfileInt(_T("Input"),_T("Channels"),(int)m_nChannels);
	Ini.WriteProfileBOOL(_T("Input"),_T("AudioBuffer"),m_bAudioBufferEnable);
	Ini.WriteProfileInt(_T("Input"),_T("AudioBufferTime"),m_nAudioBufferTime);
	Ini.WriteProfileInt(_T("Input"),_T("AudioBufferCount"),m_nAudioBufferCount);

	Ini.WriteProfileBOOL(_T("Input"),_T("GraphClockEnable"),m_bGraphClockEnable);
	Ini.WriteProfileInt(_T("Input"),_T("GraphClockSetting"),m_nGraphClockSetting);
	Ini.WriteProfileBOOL(_T("Input"),_T("GraphSyncEnable"),m_bGraphSyncEnable);

	//プラグインの設定
	//ビデオプラグイン
	CString strKeyName;
	Ini.WriteProfileInt(_T("VideoPlugin"),_T("Count"),m_nVideoPluginCount);
	for (int i=0;i<(int)m_VideoPluginList.size();i++)
	{
		strKeyName.Format(_T("%d_Name"),i);
		Ini.WriteProfileString(_T("VideoPlugin"),strKeyName,m_VideoPluginList[i].strFilterName);
		strKeyName.Format(_T("%d_GUID"),i);
		Ini.WriteProfileGUID(_T("VideoPlugin"),strKeyName,m_VideoPluginList[i].guid);
		strKeyName.Format(_T("%d_GUID_Category"),i);
		Ini.WriteProfileGUID(_T("VideoPlugin"),strKeyName,m_VideoPluginList[i].guidCategory);
		strKeyName.Format(_T("%d_Enable"),i);
		Ini.WriteProfileBOOL(_T("VideoPlugin"),strKeyName,m_VideoPluginList[i].bEnable);
	}
	//オーディオプラグイン
	Ini.WriteProfileInt(_T("AudioPlugin"),_T("Count"),m_nAudioPluginCount);
	for (int i=0;i<(int)m_AudioPluginList.size();i++)
	{
		strKeyName.Format(_T("%d_Name"),i);
		Ini.WriteProfileString(_T("AudioPlugin"),strKeyName,m_AudioPluginList[i].strFilterName);
		strKeyName.Format(_T("%d_GUID"),i);
		Ini.WriteProfileGUID(_T("AudioPlugin"),strKeyName,m_AudioPluginList[i].guid);
		strKeyName.Format(_T("%d_GUID_Category"),i);
		Ini.WriteProfileGUID(_T("AudioPlugin"),strKeyName,m_AudioPluginList[i].guidCategory);
		strKeyName.Format(_T("%d_Enable"),i);
		Ini.WriteProfileBOOL(_T("AudioPlugin"),strKeyName,m_AudioPluginList[i].bEnable);
	}

	//圧縮タブの設定(ビデオエンコード)
	Ini.WriteProfileString(_T("VideoEncode"),_T("EncoderName"),m_strVideoEncodeName);
	Ini.WriteProfileInt(_T("VideoEncode"),_T("Bitrate"),m_nVideoEnocdeBitrate);
	Ini.WriteProfileDouble(_T("VideoEncode"),_T("Framerate"),(float)m_dVideoEncodeFramerate);
	Ini.WriteProfileDouble(_T("VideoEncode"),_T("MaxFramerateSetting"),(float)m_dVideoEncodeMaxFramerateSetting);
	Ini.WriteProfileBOOL(_T("VideoEncode"),_T("EncodeSize"),m_bVideoEncodeSize);
	Ini.WriteProfileInt(_T("VideoEncode"),_T("EncodeWidth"),m_nVideoEncodeWidth);
	Ini.WriteProfileInt(_T("VideoEncode"),_T("EncodeHeight"),m_nVideoEncodeHeight);
	Ini.WriteProfileInt(_T("VideoEncode"),_T("Quality"),m_nVideoEnocdeQuality);
	Ini.WriteProfileInt(_T("VideoEncode"),_T("BufferWindow"),m_nBufferWindow);
	Ini.WriteProfileInt(_T("VideoEncode"),_T("MaxKeyFrameSpacing"),m_nMaxKeyFrameSpacing);
	Ini.WriteProfileBOOL(_T("VideoEncode"),_T("AutoMaxKeyFrameSpacing"),m_bMaxKeyFrameSpacing);
	Ini.WriteProfileInt(_T("VideoEncode"),_T("Complexity"),m_nVideoComplexity);
	Ini.WriteProfileBOOL(_T("VideoEncode"),_T("AutoComplexity"),m_bVideoComplexity);

	//圧縮タブの設定(オーディオエンコード)
	Ini.WriteProfileString(_T("AudioEncode"),_T("EncoderName"),m_strAudioEncodeName);
	//Ini.WriteProfileString(_T("AudioEncode"),_T("EncoderDescription"),m_AudioEncodeDescription);
	Ini.WriteProfileInt(_T("AudioEncode"),_T("Bitrate"),m_nAudioEncodeBitrate);
	Ini.WriteProfileInt(_T("AudioEncode"),_T("Channels"),m_nAudioEncodeChannels);
	Ini.WriteProfileInt(_T("AudioEncode"),_T("BitsPerSample"),m_nAudioEncodeBitsPerSample);
	Ini.WriteProfileInt(_T("AudioEncode"),_T("SamplesPerSec"),m_nAudioEncodeSamplesPerSec);
	Ini.WriteProfileBOOL(_T("AudioEncode"),_T("AVSynchronization"),m_bAudioEncodeAVSynchronization);

	//出力タブの設定
	Ini.WriteProfileBOOL(_T("Output"),_T("PullCheck"),m_bOutputPullCheck);
	Ini.WriteProfileBOOL(_T("Output"),_T("PushCheck"),m_bOutputPushCheck);
	Ini.WriteProfileBOOL(_T("Output"),_T("FileCheck"),m_bOutputFileCheck);
	Ini.WriteProfileInt(_T("Output"),_T("PullCount"),m_nOutputPullCount);
	Ini.WriteProfileInt(_T("Output"),_T("PushCount"),m_nOutputPushCount);
	Ini.WriteProfileInt(_T("Output"),_T("FileCount"),m_nOutputFileCount);
	assert(m_nOutputPullCount == m_OutputPullList.size());
	assert(m_nOutputPushCount == m_OutputPushList.size());
	assert(m_nOutputFileCount == m_OutputFileList.size());
	CString strSection;
	for (int i=0;i<m_nOutputPullCount;i++)
	{
		strSection.Format(_T("Pull%d"),i+1);
		Ini.WriteProfileBOOL(strSection,_T("TabEnable"),m_OutputPullList[i].Data.bTabEnable);
		Ini.WriteProfileString(strSection,_T("TabName"),m_OutputPullList[i].Data.strTabName);
		Ini.WriteProfileInt(strSection,_T("Port"),m_OutputPullList[i].nPort);
		Ini.WriteProfileInt(strSection,_T("Max"),m_OutputPullList[i].nMax);
 	}
	for (int i=0;i<m_nOutputPushCount;i++)
	{
		strSection.Format(_T("Push%d"),i+1);
		Ini.WriteProfileBOOL(strSection,_T("TabEnable"),m_OutputPushList[i].Data.bTabEnable);
		Ini.WriteProfileString(strSection,_T("TabName"),m_OutputPushList[i].Data.strTabName);
		Ini.WriteProfileString(strSection,_T("ServerName"),m_OutputPushList[i].strServerName);
		Ini.WriteProfileBOOL(strSection,_T("AutoDelete"),m_OutputPushList[i].bAutoDelete);
 	}
	for (int i=0;i<m_nOutputFileCount;i++)
	{
		strSection.Format(_T("File%d"),i+1);
		Ini.WriteProfileBOOL(strSection,_T("TabEnable"),m_OutputFileList[i].Data.bTabEnable);
		Ini.WriteProfileString(strSection,_T("TabName"),m_OutputFileList[i].Data.strTabName);
		Ini.WriteProfileString(strSection,_T("FileName"),m_OutputFileList[i].strFileName);
		Ini.WriteProfileString(strSection,_T("FolderName"),m_OutputFileList[i].strFolderName);
	}

	//出力タブの詳細設定
	Ini.WriteProfileInt(_T("OutputOption"),_T("MaxPacketSize"),m_nOutputMaxPacketSize);
	Ini.WriteProfileInt(_T("OutputOption"),_T("MinPacketSize"),m_nOutputMinPacketSize);
	Ini.WriteProfileBOOL(_T("OutputOption"),_T("InsertIndex"),m_bOutputInsertIndex);
	Ini.WriteProfileBOOL(_T("OutputOption"),_T("InsertIndexEnd"),m_bOutputInsertIndexEnd);
	Ini.WriteProfileInt(_T("OutputOption"),_T("SyncToleranceTime"),m_nOutputSyncToleranceTime);
	Ini.WriteProfileBOOL(_T("OutputOption"),_T("ForceDisconnect"),m_bOutputForceDisconnect);

	//属性タブの設定
	Ini.WriteProfileString(_T("Metadata"),_T("Title"),m_strMetadataTitle);
	Ini.WriteProfileString(_T("Metadata"),_T("Author"),m_strMetadataAuthor);
	Ini.WriteProfileString(_T("Metadata"),_T("Copyright"),m_strMetadataCopyright);
	Ini.WriteProfileString(_T("Metadata"),_T("Rating"),m_strMetadataRating);
	Ini.WriteProfileString(_T("Metadata"),_T("Description"),m_strMetadataDescription);

	//CPU設定
	Ini.WriteProfileBOOL(_T("CPU"),_T("EnableProcessPriority"),m_bProcessPriority);
	Ini.WriteProfileInt(_T("CPU"),_T("ProcessPriority"),m_nProcessPriority);
	Ini.WriteProfileBOOL(_T("CPU"),_T("ProcessLimitEnable"),m_bProcessLimitEnable);
	Ini.WriteProfileBOOL(_T("CPU"),_T("ProcessAffinityPriority"),m_bProcessAffinityPriority);
	Ini.WriteProfileInt(_T("CPU"),_T("ProcessAffinityMask"),m_dwProcessAffinityMask);
	Ini.WriteProfileInt(_T("CPU"),_T("ProcessIdealProcessor"),m_nProcessIdealProcessor);

	//INI編集終了
	Ini.Close();
	return TRUE;
}