#pragma once


//プラグインデータ読み書きの際の構造体
struct IniPluginStruct {
	GUID guid;				//GUID
	GUID guidCategory;		//GUIDカテゴリ
	CString strFilterName;	//フィルタ名
	BOOL bEnable;			//有効・無効
};
struct IniOutputBasicStruct {
	int nIndex;				//インデックス
	BOOL bTabEnable;		//有効か無効かどうか
	CString strTabName;		//タブ名
};
//プル出力データ読み書きの際の構造体
struct IniOutputPullStruct {
	IniOutputBasicStruct Data;
	int nPort;				//ポート番号
	int nMax;				//最大接続数
};
//プッシュ出力データ読み書きの際の構造体
struct IniOutputPushStruct {
	IniOutputBasicStruct Data;
	CString strServerName;	//サーバー名
	BOOL bAutoDelete;		//自動削除
};
//ファイル出力データ読み書きの際の構造体
struct IniOutputFileStruct {
	IniOutputBasicStruct Data;
	CString strFileName;	//出力ファイル名
	CString strFolderName;	//出力フォルダ名
};

//////////////////////////////////////////////////
//プロファイルの読み書きを行うクラス
//プロファイルは複数設定保存可能な設定データのこと
//プログラム起動時にデフォルトのプロファイルが読み込まれ、終了時にプロファイルの保存を行う
//プロファイルはプログラムの途中でも読み書きが行われる
//////////////////////////////////////////////////
class CProfile
{
public:
	CProfile(void);				//コンストラクタ
	virtual ~CProfile(void);	//デストラクタ
	void DefaultProfile();		//初期値に戻す
	BOOL LoadProfile(LPCTSTR lpszProfileName,BOOL bFirstFlag);	//プロファイルの読み込み
	BOOL SaveProfile(LPCTSTR lpszProfileName);					//プロファイルの書き込み


	//////////////////////////////////////////////////
	//様々なプロファイル項目はここに書く
	//////////////////////////////////////////////////

	//入力タブの設定
	CString m_strVideoName;			//ビデオキャプチャ名
	CString m_strVideoSettingName;	//ビデオキャプチャの設定名
	int     m_nVideoSettingIndex;	//ビデオキャプチャの設定インデックス
	int     m_nVideoSettingWidth;	//ビデオキャプチャの幅
	int     m_nVideoSettingHeight;	//ビデオキャプチャの高さ
	double  m_dVideoSettingFPS;		//ビデオキャプチャのフレームレート

	CString m_strAudioName;			//オーディオキャプチャ名
	CString m_strAudioSettingName;	//オーディオキャプチャの設定名
	int		m_nAudioSettingIndex;	//オーディオキャプチャの設定インデックス
	BOOL    m_bAudioInputVideoPin;	//オーディオキャプチャをビデオピンからとるか否か
	DWORD   m_nSamplesPerSec;		//オーディオキャプチャの周波数
	WORD    m_wBitsPerSample;		//オーディオキャプチャの分解能
	WORD    m_nChannels;			//オーディオキャプチャのチャンネル
	BOOL m_bAudioBufferEnable;		//オーディオバッファ設定の可否
	int m_nAudioBufferTime;			//オーディオバッファの時間(ミリ秒)
	int m_nAudioBufferCount;		//オーディオバッファの個数

	BOOL m_bGraphClockEnable;		//基準クロックの設定の有無
	int m_nGraphClockSetting;		//基準クロックの選択(0～2)
	BOOL m_bGraphSyncEnable;		//基準クロックの同期の有無

	//プラグインの設定
	int m_nVideoPluginCount;					//ビデオプラグインの個数
	vector<IniPluginStruct> m_VideoPluginList;	//ビデオプラグインの配列
	int m_nAudioPluginCount;					//オーディオプラグインの個数
	vector<IniPluginStruct> m_AudioPluginList;	//オーディオプラグインの配列

	//圧縮タブの設定(ビデオエンコード)
	CString m_strVideoEncodeName;	//ビデオエンコーダの名前
	int m_nVideoEnocdeBitrate;		//ビデオビットレート値(表示上の数字 単位はKbps)
	double m_dVideoEncodeFramerate;	//ビデオフレームレート(単位はfps)
	double m_dVideoEncodeMaxFramerateSetting;	//ビデオエンコードの最大フレームレート設定(単位はfps)
	BOOL m_bVideoEncodeSize;		//ビデオエンコードのサイズ変更
	int m_nVideoEncodeWidth;		//ビデオエンコードの幅
	int m_nVideoEncodeHeight;		//ビデオエンコードの高さ
	int m_nVideoEnocdeQuality;		//ビデオエンコードの滑らかさ
	int m_nBufferWindow;			//バッファリング時間	(単位はms。表示上には出てこない)
	int m_nMaxKeyFrameSpacing;		//キーフレームの間隔	(単位はms。表示上には出てこない)
	BOOL m_bMaxKeyFrameSpacing;		//キーフレームの間隔を自動で決定するかどうか？
	int m_nVideoComplexity;			//ビデオの複雑さ
	BOOL m_bVideoComplexity;		//ビデオの複雑さを自動で決めるかどうか

	//圧縮タブの設定(オーディオエンコード)
	CString m_strAudioEncodeName;		//オーディオエンコーダーの名前
	//CString m_AudioEncodeDescription;	//オーディオフォーマットの説明
	int m_nAudioEncodeBitrate;			//ビットレート		(単位はbps)
	int m_nAudioEncodeChannels;			//チャンネル数      (通常は2)
	int m_nAudioEncodeSamplesPerSec;	//サンプリングレート(通常は44100)
	int m_nAudioEncodeBitsPerSample;	//ビット数          (通常は16)
	BOOL m_bAudioEncodeAVSynchronization;	//AV同期		(通常はTRUE)

	//出力タブの設定
	BOOL m_bOutputPullCheck;			//ブロードバンド出力のチェック
	BOOL m_bOutputPushCheck;			//パブリッシュポイント出力のチェック
	BOOL m_bOutputFileCheck;			//ファイル出力のチェック
	int m_nOutputPullCount;				//ブロードバンド出力の数
	int m_nOutputPushCount;				//パブリッシュポイント出力の数
	int m_nOutputFileCount;				//ファイル出力の数
	vector<IniOutputPullStruct> m_OutputPullList;
	vector<IniOutputPushStruct> m_OutputPushList;
	vector<IniOutputFileStruct> m_OutputFileList;

	//出力タブの詳細設定
	int  m_nOutputMaxPacketSize;		//最大パケットサイズ(通常はゼロ)
	int  m_nOutputMinPacketSize;		//最小パケットサイズ(通常はゼロ)
	BOOL m_bOutputInsertIndex;			//インデックスモード(シーク可能なビデオを出力する)
	BOOL m_bOutputInsertIndexEnd;		//エンコード終了時にインデックスを付与する
	int  m_nOutputSyncToleranceTime;	//ネットワークに出力する際のバッファサイズ(通常は3000ミリ秒)
	BOOL m_bOutputForceDisconnect;		//エンコード終了時にネットワーク接続を強制切断する

	//属性タブの設定
	CString m_strMetadataTitle;			//題名
	CString m_strMetadataAuthor;		//作成者
	CString m_strMetadataCopyright;		//著作権
	CString m_strMetadataRating;		//規制
	CString m_strMetadataDescription;	//説明

	//CPU設定
	BOOL   m_bProcessPriority;			//プロセス優先度が有効かどうか
	int    m_nProcessPriority;			//エンコードプロセスの優先度(0:アイドル～5:リアルタイム)
	BOOL   m_bProcessLimitEnable;		//プロセスの制限が有効かどうか
	int    m_bProcessAffinityPriority;	//プロセスの完全制限か優先制限かどうか(TRUEなら完全制限,FALSEなら優先制限)
	DWORD  m_dwProcessAffinityMask;		//プロセスのアフィニティマスク(完全制限の場合)
	int    m_nProcessIdealProcessor;	//プロセスの優先プロセッサ(優先制限の場合)

	//////////////////////////////////////////////////
	//プロファイルの変数一覧ここまで
};
