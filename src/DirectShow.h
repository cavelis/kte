#pragma once

//#pragma comment(lib,"libcpmt.lib")

#include "qedit.h"
#include "DirectShowEtc.h"

#define	WM_GRAPH_NOTIFY		(WM_APP + 505)

//フィルタのピン情報の構造体
struct PinInfoStruct
{
	PIN_DIRECTION PinDir;	//ピンの方向(PINDIR_INPUT or PINDIR_OUTPUT)
	wstring strPinName;		//ピンの名前
	BOOL bConnected;		//接続されているかどうか
};

//////////////////////////////////////////////////
//DirectShowの制御クラス
//////////////////////////////////////////////////
class CDirectShow
{
public:
	//コンストラクタ
	CDirectShow(void);
	//デストラクタ
	virtual ~CDirectShow(void);

	//DirectShowの初期化
	bool Init(HINSTANCE hInst);
	//DirectShowの終了
	void Exit();

	//ビデオレンダラの初期化
	bool InitVideoRenderer(int nVMR,BOOL CanUseOverlay);

	//ビデオレンダラにウィンドウハンドルを関連付けする
	bool SetVideoRendererHWnd(HWND hWnd);
	//ビデオレンダラのウィンドウサイズを変更する
	bool ResizeVideoRendererHWnd(RECT rcClient);
	//ビデオレンダラからウィンドウハンドルを削除する
	bool ResetVideoRendererHWnd();
	//ビデオレンダラの表示・非表示を切り替える
	bool ShowVideoRenderer(BOOL bShow);
	bool IsShowVideoRenderer();

	//コールバック先のウィンドウハンドルを設定する
	bool SetCallbackHWnd(HWND hWnd);
	//コールバックから呼び出される関数
	LRESULT OnGraphNotify();

	//ビデオが再生終了するまで待つ
	bool WaitForCompletion(int nTimeOut);

	//オーディオレンダラの初期化
	bool InitAudioRenderer();

	//グラフビルダの取得
	inline IGraphBuilder* GetGraphBuilder() {assert(m_pGraphBuilder);return m_pGraphBuilder;}
	//ビデオレンダラの取得
	inline IBaseFilter* GetVideoRenderer() { /*assert(m_pVideoRenderer)*/;return m_pVideoRenderer; }
	//オーディオレンダラの取得
	inline IBaseFilter* GetAudioRenderer() { assert(m_pAudioRenderer);return m_pAudioRenderer; }

	HRESULT SetState(OAFilterState ofs);				//フィルタグラフの状態を変更
	OAFilterState GetState();							//フィルタグラフの状態を取得
	HRESULT Run(){return SetState(State_Running);}		//フィルタグラフの再生
	HRESULT Stop(){return SetState(State_Stopped);}		//フィルタグラフの停止
	HRESULT Pause(){return SetState(State_Paused);}		//フィルタグラフの一時停止
		
	//ファイルからインテリジェント接続でレンダリング
	bool RenderFile(BSTR strFilename);

	//フィルタ同士を接続する(ダイレクト接続)
	HRESULT ConnectFilterDirect(IBaseFilter* pSrc,IBaseFilter* pDest,AM_MEDIA_TYPE* pAmt = NULL,LPCWSTR pSrcPinName = NULL,LPCWSTR pDestPinName = NULL);
	//フィルタ同時を接続する(コンバート接続)
	HRESULT ConnectFilterConvert(IBaseFilter* pSrc,IBaseFilter* pDest,AM_MEDIA_TYPE* pAmt = NULL,LPCWSTR pSrcPinName = NULL,int* pID = NULL);

	//フィルタの全てのピンを切断する
	HRESULT DisconnectFilter(IBaseFilter* pFilter,BOOL bDisconnectInputPins,BOOL bDisconnectOutputPins);
	//フィルタ上の1つのピンを切断する
	HRESULT DisconnectFilterPin(IBaseFilter* pFilter,int nPinIndex);

	//フィルタの情報を取得する(ピン数・ピンの名前・ピンが接続されているかどうか)
	HRESULT GetFilterInfo(IBaseFilter* pFilter,vector<PinInfoStruct>&PinInfoList,int* pInputPinCount,int* pOutputPinCount);
	//入力・出力ピンの番号を取得する(入力・出力ピンが１つしかない場合)
	HRESULT GetPinIndex(IBaseFilter* pFilter,int &nPinIndex,PIN_DIRECTION pin_dir);
	HRESULT GetOutputPinIndex(IBaseFilter* pFilter,int &nOutputPinIndex) { return GetPinIndex(pFilter,nOutputPinIndex,PINDIR_OUTPUT);}
	HRESULT GetInputPinIndex(IBaseFilter* pFilter,int &nInputPinIndex)   { return GetPinIndex(pFilter,nInputPinIndex,PINDIR_INPUT);}

	//ピンの優先メディアタイプを列挙する
	HRESULT EnumMediaType(IBaseFilter* pFilter,int nPinIndex,vector<AM_MEDIA_TYPE> &MediaTypeList);
	//既に接続されているピンのメディアタイプを取得する
	HRESULT GetConnectionMediaType(IBaseFilter* pFilter,int nPinIndex,AM_MEDIA_TYPE &amt);

	//ピンが出力メディアタイプ変更可能なら設定を行う
	//HRESULT SetMediaType(IBaseFilter* pFilter,int nPinIndex,AM_MEDIA_TYPE* pamt);

	//プロパティページの表示
	HRESULT ShowPropertyDialog(IBaseFilter *pFilter,HWND hWnd);

	//キャプチャデバイスの追加
	bool EnumCaptureDevice(vector<wstring>& strDeviceStrings,bool bVideo,bool bName);
	bool AddCaptureDevice(wstring strDeviceName,bool bVideo,bool bName,int* pID);

	//キャプチャデバイスの列挙(ビデオ/オーディオ , 名前/ID)
	bool EnumVideoCaptureDeviceName(vector<wstring>& strDeviceStrings) {return EnumCaptureDevice(strDeviceStrings,true,true);}
	bool EnumVideoCaptureDeviceID(vector<wstring>& strDeviceStrings)   {return EnumCaptureDevice(strDeviceStrings,true,false);}
	bool EnumAudioCaptureDeviceName(vector<wstring>& strDeviceStrings) {return EnumCaptureDevice(strDeviceStrings,false,true);}
	bool EnumAudioCaptureDeviceID(vector<wstring>& strDeviceStrings)   {return EnumCaptureDevice(strDeviceStrings,false,false);}
	//キャプチャデバイスの追加(ビデオ/オーディオ , 名前/ID)
	bool AddVideoCaptureDeviceByName(wstring strDeviceName,int* pID)   {return AddCaptureDevice(strDeviceName,true,true,pID);}
	bool AddVideoCaptureDeviceByID(wstring strDeviceName,int* pID)     {return AddCaptureDevice(strDeviceName,true,false,pID);}
	bool AddAudioCaptureDeviceByName(wstring strDeviceName,int* pID)   {return AddCaptureDevice(strDeviceName,false,true,pID);}
	bool AddAudioCaptureDeviceByID(wstring strDeviceName,int* pID)     {return AddCaptureDevice(strDeviceName,false,false,pID);}

	//キャプチャデバイス(ビデオ)のメディアタイプを列挙する
	HRESULT EnumVideoCaptureMediaType(IBaseFilter* pFilter, int nPinIndex, vector<AM_MEDIA_TYPE> &MediaTypeList,vector<VIDEO_STREAM_CONFIG_CAPS> &VideoCapsList);
	//キャプチャデバイス(オーディオ)のメディアタイプを列挙する
	HRESULT EnumAudioCaptureMediaType(IBaseFilter* pFilter, int nPinIndex, vector<AM_MEDIA_TYPE> &MediaTypeList,vector<AUDIO_STREAM_CONFIG_CAPS> &AudioCapsList);

	//キャプチャデバイスにメディアタイプの設定を行う
	HRESULT SetMediaType(IBaseFilter* pFilter,int nPinIndex,AM_MEDIA_TYPE* pAmt);
	//キャプチャデバイスのメディアタイプを取得する
	HRESULT GetMediaType(IBaseFilter* pFilter,int nPinIndex,AM_MEDIA_TYPE &Amt);

	HRESULT GetSetCaptureMediaType(IBaseFilter* pFilter,int nPinIndex,BOOL bSet,AM_MEDIA_TYPE* pAmt);

	//ビデオ圧縮に関する情報を得る
	HRESULT GetVideoCompressInfo(IBaseFilter* pFilter,int nPinIndex);

private:
	//ピンを取得するサブモジュール
	//HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION dir, CComPtr<IPin> &pin, GUID majorType,LPCWSTR pPinName);

	//ピンを取得するサブモジュール
	HRESULT GetPin(IBaseFilter* pFilter,PIN_DIRECTION pin_direction,CComPtr<IPin> &pin,AM_MEDIA_TYPE* pAmt,LPCWSTR pPinName);

	//メンバ変数
	bool m_bComInit;									//COMが初期化されているかどうか

	IGraphBuilder*         m_pGraphBuilder;				//フィルタグラフマネージャー
	ICaptureGraphBuilder2* m_pCaptureGraphBuilder2;		//キャプチャ用のグラフビルダ
	IMediaControl*         m_pMediaControl;				//ストリーミングを制御するメソッド
	IMediaEventEx*         m_pMediaEventEx;				//イベント通知用のインタフェース
	IVideoWindow*          m_pVideoWindow;				//ウィンドウハンドルインタフェース
	IReferenceClock*       m_pReferenceClock;			//基準クロック

	IBaseFilter*           m_pVideoRenderer;			//標準のビデオレンダラフィルター
	IBaseFilter*           m_pAudioRenderer;			//標準のオーディオレンダラフィルター(DirectSound)


	//////////////////////////////////////////////////
	//フィルタの型の構造体
	struct FilterData {
		IBaseFilter* pFilter;	//フィルタのポインタ
		GUID         guid;		//フィルタのGUID
		wstring      strName;	//フィルタ名
	};
	vector<FilterData> m_FilterData;					//フィルタリスト

public:
	//フィルタを管理フィルタに追加
	HRESULT AddFilterList(IBaseFilter* pFilter,int* pID);
	//フィルタの追加
	HRESULT AddFilter(GUID guid,LPCWSTR strName,int* pID);

	//DMOフィルタの追加
	HRESULT AddDMOFilter(GUID guid,GUID guidCategory,LPCWSTR strName,int* pID);

	//フィルタの除去
	bool RemoveFilter(IBaseFilter* pFilter);
	//
	IBaseFilter* GetFilterByID(const int nID);
	IBaseFilter* GetFilterByName(LPCWSTR strName);
	//////////////////////////////////////////////////

private:
	DWORD m_dwRegister;			//ROTのレジスタID
	bool m_bRegister;			//ROTにレジストしているかどうか

public:

	//グラフクロック(基準クロック)の設定
	HRESULT SetGraphClock(BOOL bUse,BOOL bDefaultSyncSource,BOOL bMakeSystemClock,IBaseFilter* pFilter);
	//基準クロックから現在の基準タイムを取得する
	HRESULT GetGraphClockTime(REFERENCE_TIME &rtTime);
	//基準クロックの分解能を取得する
	HRESULT GetGraphClockResolution(REFERENCE_TIME &rtResolution);
	//フィルタグラフに同期の可否を設定する。また、必要があれば最大遅延時間を設定する
	HRESULT SyncStreamOffset(BOOL bUseStreamOffset,REFERENCE_TIME* prtMaxGraphLatency);
	//アロケーターバッファの設定
	HRESULT SetGetAllocatorBuffer(IBaseFilter* pFilter,int nPinIndex,BOOL bSet,ALLOCATOR_PROPERTIES &AllocProp);

	//フィルタ開始以降のフレーム数・ドロップ数などを取得
	HRESULT GetDropFrame(IBaseFilter* pFilter,int nPinIndex,long* plNotDropped,long* plDropped,long* plAverageSize);
	//プッシュソースフィルタの性能と情報を得る
	HRESULT GetPushSourceInfo(IBaseFilter* pFilter,int nPinIndex,REFERENCE_TIME *prtLatency,ULONG* pFlags,REFERENCE_TIME *prtOffset,REFERENCE_TIME *prtMaxOffset);
	//現在のストリーミング情報を取得
	HRESULT GetStreamInfo(IBaseFilter* pFilter,int nPinIndex,AM_STREAM_INFO* pAsi);
	//WDMドライバソースフィルタから自動で必須フィルタの追加を行う
	HRESULT AddWDMFilter(IBaseFilter* pFilter,BOOL bVideo,int* pCrossbar1ID,int* pCrossbar2ID,int* pTVAudioID,int* pTVTunerID);
};

	//TODO オーディオキャプチャデバイスのボリューム変更に関する検証コード
	//HRESULT SetAudioCaptureConfig(IBaseFilter* pFilter);
	//http://msdn.microsoft.com/ja-jp/library/cc352249.aspx


//////////////////////////////////////////////////
//[EOF]
