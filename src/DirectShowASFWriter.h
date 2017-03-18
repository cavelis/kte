#pragma once

#include "DirectShow.h"

/*
手順
	ASFWriterの作成
	コーデックの列挙(VideoとAudio)
	コーデックの選択(VideoとAudio)
	ビデオコーデックの設定の編集(Videoのみ)
	ASFWriterフィルタへフィルタの接続
	プロファイルの適用	ApplyProfiles();
	Sinkの追加と削除
	フィルタグラフの実行
*/

#define WM_NOTIFY_NETWORK_CLIENT_CONNECT    (WM_APP + 500)
#define WM_NOTIFY_NETWORK_CLIENT_DISCONNECT (WM_APP + 501)

//////////////////////////////////////////////////
//ネットワーク構造体
//////////////////////////////////////////////////
struct NetworkClientStruct
{
	wstring strNetworkAddress;	//IPアドレス(文字列)
	int   nNetworkAddress[4];	//IPアドレス(数値)
	DWORD dwNetworkAddress;		//IPアドレス
	DWORD dwPort;				//クライアント側のポート番号
	wstring strDNSName;			//DNS名(文字列)
};

//////////////////////////////////////////////////
//コールバック用のインタフェース
//////////////////////////////////////////////////
class CDirectShowASFWriterNetworkCallback : public IWMStatusCallback
{
public:
	//コンストラクタ
	CDirectShowASFWriterNetworkCallback();
	//デストラクタ
	~CDirectShowASFWriterNetworkCallback();
	//コールバック
    virtual HRESULT STDMETHODCALLTYPE OnStatus(WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext);
	//参照回数
	ULONG STDMETHODCALLTYPE AddRef(void){return 1;}
	ULONG STDMETHODCALLTYPE Release(void){return 1;}
	//インタフェース取得
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject);
	void SetHWnd(HWND hWnd){m_hWnd = hWnd;}
	HWND GetHWnd()const{return m_hWnd;}
private:
	HWND m_hWnd;
};


//////////////////////////////////////////////////
//DirectShow WM ASF Writerに関するクラス
//////////////////////////////////////////////////
class CDirectShowASFWriter
{
public:
	//コンストラクタ
	CDirectShowASFWriter(void);
	//デストラクタ
	virtual ~CDirectShowASFWriter(void);

	//ASF Writerを作成する
	HRESULT Create(IGraphBuilder* pGraphBuilder,LPCTSTR lpszDefaultFileName);
	//ASF Writerを削除する
	void Delete(IGraphBuilder* pGraphBuilder);
	//全てのSinkを削除する
	HRESULT DeleteAllSink();

	//WM ASF Writerフィルタの取得
	inline IBaseFilter* GetFilter(){assert(m_pAsfWriter);return m_pAsfWriter;}

	//プロファイルの適用
	HRESULT ApplyProfiles(IWMProfile* pProfile);

	//現在のビデオ/オーディオピンの受け入れフォーマットを取得する
	HRESULT GetFormat(AM_MEDIA_TYPE &amt,BOOL bVideo = TRUE);

	//ASF Writerに関する諸設定を適用する
	HRESULT SetAsfSyncTolerance(const DWORD dwWindow);
	HRESULT SetAsfIndexMode(const BOOL bIndexFile);
	HRESULT SetAsfParam(const DWORD dwSettingParam,const DWORD dwParam);
	HRESULT SetFixedFrameRate(BOOL bFixedFrameRate);


	//ファイルに出力
	HRESULT AddFileSink(const WCHAR* pwszFileName,int* pnIndex);
	HRESULT RemoveFileSink(const int nIndex);

	//ブロードキャスト出力
	HRESULT AddNetworkSink(DWORD* pdwPortNum,const DWORD dwMaxClients,int* pnIndex);
	HRESULT RemoveNetworkSink(const int nIndex);
	HRESULT ChangeNetworkSink(const int nIndex,const DWORD dwMaxClients);

	//ブロードキャスト出力での接続された情報を取得
	HRESULT GetNetworkSink(const int nIndex,DWORD dwIPAddress,DWORD dwPort,NetworkClientStruct &Client);

	//プッシュ出力
	HRESULT AddPushSink(LPCWSTR pwszURL,LPCWSTR pwszTemplateURL,const BOOL fAutoDestroy,int* pnIndex);
	HRESULT RemovePushSink(const int nIndex);

	//統計情報を取得
	HRESULT GetStatistics(QWORD &cnsCurrentTime,WM_WRITER_STATISTICS &Stats,WM_WRITER_STATISTICS_EX &StatsEx);

	//メタデータを設定する
	void SetTitle(LPCTSTR strTitle) { m_strTitle = strTitle;}
	void SetAuthor(LPCTSTR strAuthor) { m_strAuthor = strAuthor;}
	void SetDescription(LPCTSTR strDescription) { m_strDescription = strDescription;}
	void SetRating(LPCTSTR strRating) { m_strRating = strRating;}
	void SetCopyright(LPCTSTR strCopyright) { m_strCopyright = strCopyright;}

	//コールバック用のウィンドウハンドルを関連付け
	void SetNetworkCallbackHWnd(HWND hWnd){
		assert(m_pNetworkCallback);
		m_pNetworkCallback->SetHWnd(hWnd);}
	HWND GetNetworkCallbackHWnd()const{
		assert(m_pNetworkCallback);
		return m_pNetworkCallback->GetHWnd();}

private:
	IBaseFilter* m_pAsfWriter;					//WM ASF Writer フィルタ
	IWMWriterAdvanced3 *m_pWMWriterAdvanced3;	//IWMWriterSinkへの問い合わせインタフェース
	IConfigAsfWriter2* m_pConfigAsfWriter2;		//ASFライタの設定


	BOOL m_bIsWriterAdvanced3;

	vector<IWMWriterNetworkSink*> m_NetworkSinkList;
	vector<IWMWriterPushSink*>    m_PushSinkList;
	vector<IWMWriterFileSink*>    m_FileSinkList;

	//メタデータ
	IWMHeaderInfo* m_pWMHeaderInfo;
	wstring m_strTitle;			//タイトル
	wstring m_strAuthor;		//著作者
	wstring m_strDescription;	//説明
	wstring m_strRating;		//規制
	wstring m_strCopyright;		//説明

	//メタデータの設定
	HRESULT SetAttribute(int nIndex);

	//ネットワークシンクのコールバック用のクラス
	CDirectShowASFWriterNetworkCallback* m_pNetworkCallback;
};

