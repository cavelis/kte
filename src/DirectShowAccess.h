#pragma once

#include "DirectShow.h"
#include "DirectShowEncoder.h"
#include "DirectShowASFWriter.h"
#include "DirectShowAudioVolume.h"

//プラグインの構造体
struct FilterPluginStruct
{
	GUID guidFilter;			//フィルタのGUID
	GUID guidCategory;			//フィルタのカテゴリGUID (DMOのみ)
	int nFilterID;				//フィルタに登録した際に得られるID (-1なら未作成)
	CString strFilterName;		//フィルタに登録した際の登録名
	BOOL bDMO;					//DMOデバイスかどうか	(DMOの場合はカテゴリGUIDが定義されている)
	BOOL bEnable;				//このフィルタが有効化どうか
};

//プルシンク構造体
struct PullSinkStruct
{
	int nID;				//インデックスID
	int nRegistID;			//登録ID
	DWORD dwPort;			//ポート番号
	DWORD dwMax;			//最大接続数
};
//プッシュシンク構造体
struct PushSinkStruct
{
	int nID;				//インデックスID
	int nRegistID;			//登録ID
	CString strServerName;	//サーバー名
	BOOL bAutoDelete;		//自動削除
};
//ファイルシンク構造体
struct FileSinkStruct
{
	int nID;				//インデックスID
	int nRegistID;			//登録ID
	CString strFileName;	//ファイル名
};

//WDMフィルタの列挙型
enum ENUM_WDM_FILTER
{
	WDM_FILTER_NONE = 0,
	WDM_FILTER_CROSS_BAR1,
	WDM_FILTER_CROSS_BAR2,
	WDM_FILTER_TV_AUDIO,
	WDM_FILTER_TV_TUNER
};

//////////////////////////////////////////////////
//MFC側からDirectShowに対するアクセサクラス
//////////////////////////////////////////////////
class CDirectShowAccess
{
public:
	CDirectShowAccess(void);			//コンストラクタ
	~CDirectShowAccess(void);			//デストラクタ
	BOOL Init();						//DirectShowを初期化
	void Exit();						//DirectShowを終了

	BOOL ChangeProfileNotify();			//プロファイルの変更通知
	BOOL SaveProfileSetting();			//プロファイルからのロード
	BOOL LoadProfileSetting();			//プロファイルへのセーブ

	//グラフファイルの保存
	HRESULT SaveGraphFile(WCHAR* wszPath)
		{return CDirectShowEtc::SaveGraphFile(m_pDirectShow->GetGraphBuilder(),wszPath);}

	//////////////////////////////////////////////////
	//フィルタの実行と停止
	BOOL RunFilter();					//フィルタの開始
	BOOL StopFilter();					//フィルタの停止
	BOOL IsRunFilter()const{			//フィルタの開始状態
		return m_bRunFilter;}
	BOOL m_bRunFilter;					//フィルタが実行しているかどうか

	//プレビューが可能か否か(ビデオのみチェック)
	inline BOOL IsCanPreview() const {
		return (m_nVideoIndex != -1 && m_nVideoSettingIndex != -1);
	}
	inline BOOL IsCanAudioPreview() const {return (m_nAudioIndex != -1);}

	//////////////////////////////////////////////////
	//ビデオキャプチャデバイスを選択します
	BOOL SelectVideoIndex(int nIndex);
	//ビデオキャプチャデバイスの設定を選択します
	BOOL SelectVideoSettingIndex(int nSettingIndex,AM_MEDIA_TYPE *pamt);
	//現在のキャプチャデバイスのビデオサイズを取得
	BOOL GetCurrentVideoSize(int &nWidth,int &nHeight);

	//////////////////////////////////////////////////
	//オーディオキャプチャデバイスを選択します
	BOOL SelectAudioIndex(int nIndex,BOOL bUseVideoPin);
	//オーディオキャプチャデバイスの設定を選択します
	BOOL SelectAudioSettingIndex(int nSettingIndex,AM_MEDIA_TYPE *pamt);
	//オーディオボリュームフィルタに接続する
	BOOL ConnectAudioVolume();

	int GetVideoIndex() const {return m_nVideoIndex;}
	int GetVideoSettingIndex() const {return m_nVideoSettingIndex;}

	int GetAudioIndex() const {return m_nAudioIndex;}
	int GetAudioSettingIndex() const {return m_nAudioSettingIndex;}
	int GetWDMFilterID(ENUM_WDM_FILTER e) const {
		switch (e) {
		case WDM_FILTER_NONE: return -1;
		case WDM_FILTER_CROSS_BAR1: return m_nCrossbar1ID;
		case WDM_FILTER_CROSS_BAR2: return m_nCrossbar2ID;
		case WDM_FILTER_TV_AUDIO:	 return m_nTVAudioID;
		case WDM_FILTER_TV_TUNER:   return m_nTVTunerID;
		default: return -1;
		}
	}
	BOOL IsVidoFindAudioPin() const {return m_bVideoFindAudioPin;}
	BOOL IsUseAudioVideoFilter() const {
		if (m_bUseAudioVideoFilter && m_nVideoFilterID == m_nAudioFilterID) return TRUE;
		return FALSE;}

	HRESULT RenderPreview();				//プレビューの接続の試行
	HRESULT RenderPreviewSubRoutine1(const int nLastFilterID,const wchar_t* lpszOutputPin);
	void    RenderPreviewSubRoutine2();

	BOOL CheckConnectFilter(BOOL bVideo);	//フィルタの接続状況を確認するサブルーチン


	//コールバックウィンドウハンドルの設定
	BOOL SetCallbackHWnd(HWND hWnd) {
		return (BOOL)m_pDirectShow->SetCallbackHWnd(hWnd);}
	//コールバックの通知
	LRESULT OnGraphNotify() {
		return m_pDirectShow->OnGraphNotify();}


	//////////////////////////////////////////////////
	//ビデオプレビューのアクセサ
	BOOL SetPreviewWindow(HWND hWnd);		//ビデオプレビューのウィンドウハンドルを設定する
	BOOL RemovePreviewWindow();				//ビデオプレビューのウィンドウハンドルを解除する
	BOOL ResizePreviewWindow(RECT rcClient);//ビデオプレビューのウィンドウサイズを変更する
	BOOL ShowPreviewWindow(BOOL bShow);		//ビデオプレビューの表示・非表示を切り替える
	
	//プロパティウィンドウを表示します
	BOOL ShowPropertySetting(BOOL bVideo,ENUM_WDM_FILTER eDevice,HWND hWnd);

	//////////////////////////////////////////////////
	//メンバ変数
	//ビデオキャプチャデバイスに関する設定
	vector<wstring> m_strVideoNameList;						//全てのビデオキャプチャデバイスの名前リスト
	vector<wstring> m_strVideoSettingList;					//現在のビデオキャプチャデバイスの設定リスト(設定略称)
	vector<AM_MEDIA_TYPE>			 m_VideoMediaTypeList;	//ビデオのメディアタイプリスト (対になっている)
	vector<VIDEO_STREAM_CONFIG_CAPS> m_VideoCapsList;		//ビデオのデバイス性能リスト   (対になっている)
	AM_MEDIA_TYPE m_VideoFirstMediaType;					//現在のビデオ接続メディアタイプ(最先端)
	AM_MEDIA_TYPE m_VideoLastMediaType;						//現在のビデオ接続メディアタイプ(最後尾)

	//オーディオキャプチャデバイスに関する設定
	vector<wstring> m_strAudioNameList;						//全てのオーディオキャプチャデバイスの名前
	vector<wstring> m_strAudioSettingList;					//現在のオーディオキャプチャデバイスの設定リスト(設定略称)
	vector<AM_MEDIA_TYPE> m_AudioMediaTypeList;				//オーディオのメディアタイプリスト	(対になっている)
	vector<AUDIO_STREAM_CONFIG_CAPS> m_AudioCapsList;		//オーディオのデバイス性能リスト	(対になっている)
	AM_MEDIA_TYPE m_AudioFirstMediaType;					//現在のオーディオ接続メディアタイプ(最先端)
	AM_MEDIA_TYPE m_AudioLastMediaType;						//現在のオーディオ接続メディアタイプ(最後尾)
	ALLOCATOR_PROPERTIES m_AudioSetAllocator;				//オーディオキャプチャのアロケーターバッファ(要求)
	ALLOCATOR_PROPERTIES m_AudioGetAllocator;				//オーディオキャプチャのアロケーターバッファ(実際の接続)
	
	//////////////////////////////////////////////////
	//プラグインに関する設定
	vector<FilterPluginStruct> m_VideoPluginList;		//ビデオプラグインの配列
	vector<FilterPluginStruct> m_AudioPluginList;		//オーディオプラグインの配列

	//フィルタ配列の接続
	BOOL ChainPluginList(BOOL bVideo,int nInputFilterID,const wchar_t* lpszInputPinName,int nOutputFilterID,const wchar_t* lpszOutputPinName);
	//プラグインの追加登録
	BOOL AddPlugin(BOOL bVideo,const GUID guid,const GUID guidCategory,LPCWSTR lpszPluginName);
	//プラグインの削除
	BOOL RemovePlugin(BOOL bVideo,int nIndex);
	//フィルタ配列の入れ替え(上へ・下へ)
	BOOL SwitchPlugin(BOOL bVideo,int nSrcIndex,int nDestIndex);
	//プラグインの設定ダイアログを表示
	BOOL ShowPluginDialog(HWND hWnd,BOOL bVideo,int nIndex);

	//プラグインここまで
	//////////////////////////////////////////////////

	//基準クロックからの基準タイムの取得
	BOOL GetClockTime(REFERENCE_TIME &rtTime) {assert(m_pDirectShow); return SUCCEEDED(m_pDirectShow->GetGraphClockTime(rtTime));}
	//基準クロックの分解能を取得
	BOOL GetClockResolution(REFERENCE_TIME &rcResolution) {assert(m_pDirectShow); return SUCCEEDED(m_pDirectShow->GetGraphClockResolution(rcResolution));}
	//ビデオキャプチャフィルタのフレーム数・ドロップ数などを取得
	BOOL GetVideoDropFrame(long* plNotDropped,long* plDropped,long* plAverageSize)
	{
		assert(m_pDirectShow);
		if (m_nVideoFilterID == -1)
			return FALSE;
		if (m_nVideoFilterOutputPinIndex == -1)
			return FALSE;
		HRESULT hr;
		hr = m_pDirectShow->GetDropFrame(
			m_pDirectShow->GetFilterByID(m_nVideoFilterID),m_nVideoFilterOutputPinIndex,
			plNotDropped,plDropped,plAverageSize);
		return SUCCEEDED(hr);
	}
	//デバイス性能(遅延時間・オフセット)などを取得
	BOOL GetPushSourceInfo(BOOL bVideo,REFERENCE_TIME* prtLatency,ULONG* pFlags,
		REFERENCE_TIME* prtOffset,REFERENCE_TIME* prtMaxOffset)
	{
		assert(m_pDirectShow);
		int nID = bVideo? m_nVideoFilterID : m_nAudioFilterID;
		if (nID == -1) return FALSE;
		int nPinIndex = bVideo ? m_nVideoFilterOutputPinIndex : m_nAudioFilterOutputPinIndex;
		if (nPinIndex == -1) return FALSE;
		HRESULT hr = m_pDirectShow->GetPushSourceInfo(
			m_pDirectShow->GetFilterByID(nID),
			nPinIndex,prtLatency,pFlags,prtOffset,prtMaxOffset);
		return SUCCEEDED(hr);
	}
	//ストリーム情報を取得
	BOOL GetStreamInfo(BOOL bVideo,AM_STREAM_INFO* pAsi)
	{
		assert(m_pDirectShow);
		int nID = bVideo? m_nVideoFilterID : m_nAudioFilterID;
		if (nID == -1) return FALSE;
		int nPinIndex = bVideo ? m_nVideoFilterOutputPinIndex : m_nAudioFilterOutputPinIndex;
		if (nPinIndex == -1) return FALSE;
		HRESULT hr = m_pDirectShow->GetStreamInfo(
			m_pDirectShow->GetFilterByID(nID),nPinIndex,pAsi);
		return SUCCEEDED(hr);
	}

	//
	void SetAudioVolumeHWnd(HWND hWnd) {
		//assert(m_pAudioVolumeFilter);
		if (m_pAudioVolumeFilter)
			m_pAudioVolumeFilter->SetHWnd(hWnd);
	}
	int GetAudioVolumeChannels() {
		//assert(m_pAudioVolumeFilter);
		if (m_pAudioVolumeFilter)
			return m_pAudioVolumeFilter->GetChannels();
		return -1;
	}
	BOOL GetAudioVolumedB(double *pddB) {
		//assert(m_pAudioVolumeFilter);
		if (m_pAudioVolumeFilter)
			return m_pAudioVolumeFilter->GetdB(pddB);
		return FALSE;
	}
	BOOL IsAudioVolumeEnabled() const {
		if (m_pAudioVolumeFilter)
			return m_pAudioVolumeFilter->IsCaptureEnabled();
		return FALSE;
	}
	void AudioVolumeEnableCapture(BOOL bEnable) {
		if (m_pAudioVolumeFilter)
			m_pAudioVolumeFilter->EnableCapture(bEnable);
		m_bAudioVolumeEnable = bEnable;
	}

private:

	CDirectShow* m_pDirectShow;	//DirectShowクラス

	//ビデオ周り
	int m_nVideoIndex;			//ビデオ選択のインデックス(デバイス名のインデックス)
	int m_nVideoSettingIndex;	//ビデオ選択設定のインデックス(設定略称のインデックス)

	int m_nVideoFilterID;			//ビデオフィルタのID
	int m_nVideoSmartTeeID;			//ビデオ分岐フィルタのID
	int m_nVideoColorSpaceID;		//ビデオ変換フィルタ(カラースペース)のID
	int m_nVideoAVIDecompressorID;	//ビデオ変換フィルタ(AVIデコンプレッサ)のID
	int m_nVideoOverlayMixerID;		//オーバーレイミキサのID
	int m_nCrossbar1ID;				//第1クロスバーのフィルタのID
	int m_nCrossbar2ID;				//第2クロスバーのフィルタのID
	int m_nTVAudioID;  				//TVオーディオのフィルタのID
	int m_nTVTunerID;  				//TVチューナーのフィルタのID

	int m_nVideoFilterOutputPinIndex;	//ビデオフィルタの出力ピンの番号
	BOOL m_bVideoFindAudioPin;		//ビデオデバイスにオーディオピンがあるかどうか
	BOOL m_bUseAudioVideoFilter;	//オーディオデバイスとビデオデバイスを共用している

	//オーディオ周り
	int m_nAudioIndex;			//オーディオ選択のインデックス
	int m_nAudioSettingIndex;	//オーディオ選択設定のインデックス(設定略称のインデックス)

	int m_nAudioFilterID;				//オーディオフィルタのID
	int m_nAudioSmartTeeID;				//オーディオ分岐フィルタのID
	int m_nAudioRenderID;				//オーディオレンダラのID (IReferenceClock用)
	int m_nAudioNullFilterID;			//オーディオフィルタのNullレンダラのID
	CDirectShowAudioVolume* m_pAudioVolumeFilter;	//オーディオボリュームのサンプルグラバのラッパーのポインタ

	int m_nAudioFilterOutputPinIndex;	//オーディオフィルタの出力ピンの番号
	BOOL m_bAudioVolumeEnable;			//オーディオボリュームが有効かどうか

	//ビデオプレビューの項目
	HWND m_hVideoWnd;			//ビデオプレビューのウィンドウハンドル
	BOOL m_bVideoShow;			//ビデオプレビューが表示されているかどうか？
	int m_nVideoVMR;			//ビデオミキシングレンダラの選択(0,1,7,9,10の選択)
	BOOL m_bVideoVMROverlay;	//プレビューのビデオミキシングレンダラのオーバーレイ確保の有無
	BOOL m_bVideoForceOff;		//ビデオプレビューを強制的に切る設定(再起動時で有効になる)



	//////////////////////////////////////////////////
	//エンコーダーに関する設定
	//////////////////////////////////////////////////
public:
	vector<CodecStruct> m_VideoCodecList;			//ビデオコーデック
	vector<CodecStruct> m_AudioCodecList;			//オーディオコーデック
	vector<AudioCodecFormatStruct> m_AudioFormatList;	//現在のオーディオコーデック設定リスト

	BOOL SelectVideoCodecIndex(int nIndex);
	BOOL SetVideoCodecSetting(const DWORD dwBitrate,const DWORD msBufferWindow,const double dFPS,const RECT rcSource,const RECT rcTarget);
	BOOL SetVideoCodecSetting2(const DWORD dwQuality,const int nMaxKeyFrameSpacing);
	BOOL SetVideoComplexity(const WORD wComplexity);

	BOOL SelectAudioCodecIndex(int nIndex);
	BOOL SelectAudioCodecFormatIndex(int nIndex);

	BOOL StartEncode();						//エンコードの開始
	BOOL StopEncode(BOOL bForceDisconnect);	//エンコードの停止

	//エンコード可能かどうか
	inline BOOL IsCanEncode() const {
		return IsCanPreview() && IsRunFilter() && IsCanAudioPreview() && !IsEncode() &&
			(m_nVideoCodecIndex != -1) && (m_nAudioCodecIndex != -1) && (m_nAudioCodecFormatIndex != -1);}
	//(m_PullSinkList.size() != 0 || m_PushSinkList.size() != 0 || m_FileSinkList.size() != 0)

	//エンコードしているかどうかを取得
	inline BOOL IsEncode() const { return m_bStartEncode;}

private:
	//ASFライタのパラメータを適用する(StartEncode()のサブルーチン)
	HRESULT SetAsfWriterParam();

	CDirectShowEncoder* m_pDirectShowEncoder;		//エンコーダークラス

	int m_bStartEncode;				//エンコードをしているかどうかの設定

	int m_nVideoCodecIndex;			//現在のビデオコーデックのインデックス
	int m_nAudioCodecIndex;			//現在のオーディオコーデックのインデックス
	int m_nAudioCodecFormatIndex;	//現在のオーディオコーデックのフォーマットインデックス



	//////////////////////////////////////////////////
	//ASFライタに関する設定
	//////////////////////////////////////////////////
private:
	CDirectShowASFWriter* m_pDirectShowASFWriter;		//ASFライタクラス
	CString m_strAsfTempName;							//ASFライタのテンポラリファイル(c:\temp\に出来るファイル名)
public:
	//プル出力を追加
	BOOL AddPullSink(DWORD dwPort,DWORD dwMax,int &nID);
	//プル出力を削除
	BOOL RemovePullSink(int nID);
	//プル出力の適用
	HRESULT ApplyPullSink(int nIndex);
	//プル出力を変更(最大接続人数の変更)
	BOOL ChangePullSink(int nID,DWORD dwMax);

	//プル配信の情報を取得
	BOOL GetPullSink(int nIndex,DWORD dwIPAddress,DWORD dwPort,NetworkClientStruct &Client,DWORD &dwSrcPort);
	//プル出力が正当なデータであるのかチェックする
	BOOL IsVaildPullSink(int nIndex);

	//プッシュ出力を追加
	BOOL AddPushSink(CString strServerName,BOOL bAutoDelete,int &nID);
	//プッシュ出力を削除
	BOOL RemovePushSink(int nID);
	//プッシュ出力の適用
	HRESULT ApplyPushSink(int nIndex);

	//ファイルに出力を追加
	BOOL AddFileSink(CString strFileName,int &nID);
	//ファイルに出力を削除
	BOOL RemoveFileSink(int nID);
	//ファイルに出力の適用
	HRESULT ApplyFileSink(int nIndex);

	vector<PullSinkStruct> m_PullSinkList;		//プルシンク
	vector<PushSinkStruct> m_PushSinkList;		//プッシュシンク
	vector<FileSinkStruct> m_FileSinkList;		//ファイルシンク

	//統計情報を取得
	BOOL GetStatistics(QWORD &cnsCurrentTime,WM_WRITER_STATISTICS &Stats,WM_WRITER_STATISTICS_EX &StatsEx);

	//ネットワークコールバック用のウィンドウハンドルを関連付け
	void SetNetworkCallbackHWnd(HWND hWnd){
		m_hNetworkCallbackWnd = hWnd;}
	HWND GetNetworkCallbackHWnd()const{
		return m_hNetworkCallbackWnd;}

	//メタデータを設定
	BOOL SetMetadata(CString &strTitle,CString &strAuthor,CString &strDescription,
		CString &strRating,CString &strCopyright);

private:
	//全てのシンク(プル・プッシュ・ファイル)を適用する
	BOOL ApplyAllSink();

	//属性タブ
	CString m_strTitle;			//題名
	CString m_strAuthor;		//作成者
	CString m_strDescription;	//説明
	CString m_strRating;		//規制
	CString m_strCopyright;		//著作権

	//プル出力の接続・切断のコールバックを受け取るウィンドウハンドル
	HWND m_hNetworkCallbackWnd;
};

//////////////////////////////////////////////////
//[EOF]