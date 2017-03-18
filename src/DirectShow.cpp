#include "StdAfx.h"
#include "DirectShow.h"

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CDirectShow::CDirectShow(void)
{
	m_bComInit = false;
	m_pGraphBuilder = NULL;
	m_pCaptureGraphBuilder2 = NULL;
	m_pMediaControl = NULL;
	m_pMediaEventEx = NULL;
	m_pReferenceClock = NULL;

	m_pVideoWindow = NULL;

	m_pVideoRenderer = NULL;
	m_pAudioRenderer = NULL;

	m_dwRegister = 0x0;
	m_bRegister = false;

	m_FilterData.clear();
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CDirectShow::~CDirectShow(void)
{
	Exit();
}

//////////////////////////////////////////////////
//DirectShowの初期化
//////////////////////////////////////////////////
bool CDirectShow::Init(HINSTANCE hInst)
{
	HRESULT hr = NOERROR;
	if (!m_bComInit)
	{
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		//hr = CoInitialize(NULL);
		if (!CHECK_RESULT("Com InitializeEx.",hr)) {return false;}
		m_bComInit = true;
	}

	//デバッグライブラリを初期化
#ifdef _DEBUG
	DbgInitialise(hInst);
	DbgSetModuleLevel(LOG_TRACE,5);
	DbgSetModuleLevel(LOG_ERROR,5);
#endif
	//NOTE1("%S", __FUNCTION__ );

	//フィルタグラフマネージャ(m_pGraphBuilder)を作成する
	hr = CoCreateInstance(CLSID_FilterGraph,
		NULL,
		CLSCTX_INPROC,	//CLSCTX_INPROCCLSCTX_INPROC_SERVER
		IID_IGraphBuilder, (void **)&m_pGraphBuilder);
	if (!CHECK_RESULT("IGraphBuilder Create.",hr)) {return false;}

	//CaptureGraphBuilder2というキャプチャ用GraphBuilderを生成する
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,
		NULL, CLSCTX_INPROC, 
		IID_ICaptureGraphBuilder2, 
		(void **)&m_pCaptureGraphBuilder2);
	if (!CHECK_RESULT("Create ICaptureGraphBuilder2.",hr)) {return false;}

	// FilterGraphをセットする
	hr = m_pCaptureGraphBuilder2->SetFiltergraph(m_pGraphBuilder);
	if (!CHECK_RESULT("ICaptureGraphBuilder2 SetFiltergraph.",hr)) {return false;}

	// MediaControlインターフェース取得
	hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl,
		(void **)&m_pMediaControl);
	if (!CHECK_RESULT("IGraphBuilder Create IMediaControl.",hr)) {return false;}

#ifdef _DEBUG
	CDirectShowEtc::AddToRot(m_pGraphBuilder,m_dwRegister);
	m_bRegister = true;
#endif 
	return true;
}
//////////////////////////////////////////////////
//ビデオレンダラの初期化
//////////////////////////////////////////////////
bool CDirectShow::InitVideoRenderer(int nVMR,BOOL CanUseOverlay)
{
	assert(m_pGraphBuilder);
	HRESULT hr;
	if (m_pVideoRenderer)
	{
		hr = m_pGraphBuilder->RemoveFilter(m_pVideoRenderer);
		SAFE_RELEASE(m_pVideoRenderer);
	}

	switch (nVMR)
	{
	case 0:		//標準のビデオレンダラ
		hr = CoCreateInstance(CLSID_VideoRenderer,
			NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void **)&m_pVideoRenderer);
		if (!CHECK_RESULT("VideoRenderer(CLSID_VideoRenderer) Create.",hr)) {return false;}
		break;
	case 1:		//VMR7が使えるときは7を使い、そうでないなら標準のビデオレンダラ
		hr = CoCreateInstance(CLSID_VideoRendererDefault,
			NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void **)&m_pVideoRenderer);
		if (!CHECK_RESULT("VideoRenderer(CLSID_VideoRendererDefault) Create.",hr)) {return false;}
		break;
	case 7:		//VMR7を直接指定
		hr = CoCreateInstance(CLSID_VideoMixingRenderer,
			NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void **)&m_pVideoRenderer);
		if (!CHECK_RESULT("VideoRenderer(CLSID_VideoMixingRenderer) Create.",hr)) {return false;}
		break;
	case 9:		//VMR9を直接指定
		hr = CoCreateInstance(CLSID_VideoMixingRenderer9,
			NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void **)&m_pVideoRenderer);
		if (!CHECK_RESULT("VideoRenderer(CLSID_VideoMixingRenderer9) Create.",hr)) {return false;}
		break;
	/*
	case 10:	//Enhanced Video Renderer
		hr = CoCreateInstance(CLSID_EnhancedVideoRenderer,
			NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void **)&m_pVideoRenderer);
		if (!CHECK_RESULT("VideoRenderer(CLSID_EnhancedVideoRenderer) Create.",hr)) {return false;}
		break;
	*/
	}

	if (nVMR == 0) {
		CComPtr<IDirectDrawVideo> pDirectDrawVideo;
		hr = m_pVideoRenderer->QueryInterface(IID_IDirectDrawVideo,(void**)&pDirectDrawVideo);
		if (SUCCEEDED(hr)) {
			hr = pDirectDrawVideo->SetSwitches(AMDDS_DEFAULT);
		}
	}
	if (nVMR == 1 || nVMR == 7) {
		CComPtr<IVMRFilterConfig> pVMRFilterConfig;
		hr = m_pVideoRenderer->QueryInterface(IID_IVMRFilterConfig,(void**)&pVMRFilterConfig);
		if (SUCCEEDED(hr)) {
			hr = pVMRFilterConfig->SetNumberOfStreams(1);
			if (CanUseOverlay)	//オーバーレイを強制
				hr = pVMRFilterConfig->SetRenderingPrefs(RenderPrefs_ForceOverlays);
			else				//オフスクリーンを強制
				hr = pVMRFilterConfig->SetRenderingPrefs(RenderPrefs_ForceOffscreen);
		}
	}
	if (nVMR == 9) {
		CComPtr<IVMRFilterConfig9> pVMRFilterConfig9;
		hr = m_pVideoRenderer->QueryInterface(IID_IVMRFilterConfig9,(void**)&pVMRFilterConfig9);
		if (SUCCEEDED(hr)) {
			hr = pVMRFilterConfig9->SetNumberOfStreams(1);
		}
	}
	
	hr = m_pGraphBuilder->AddFilter(m_pVideoRenderer,L"Video Renderer");
	return true;
}
//////////////////////////////////////////////////
//ビデオレンダラのウィンドウハンドルを設定
//ウィンドウ描画オブジェクトが無いとエラーになる
//HWND 描画先のウィンドウハンドル
//////////////////////////////////////////////////
bool CDirectShow::SetVideoRendererHWnd(HWND hWnd)
{
	assert(m_pGraphBuilder);
	assert(m_pVideoRenderer);
	HRESULT hr = NOERROR;

	SAFE_RELEASE(m_pVideoWindow)
	//ビデオウィンドウインタフェースを取得(HWNDを設定)
	hr = m_pGraphBuilder->QueryInterface(IID_IVideoWindow,(void **)&m_pVideoWindow);
	if (!CHECK_RESULT("IVideoWindow Create.",hr)) {return false;}
	//Windowハンドル関連付け
	//TODO ピンが接続されていない場合はこの操作は無効になる
	hr = m_pVideoWindow->put_Owner((OAHWND)hWnd);
	if (!CHECK_RESULT("IVideoWindow put_Owner.",hr)) {return false;}
	//hr = m_pVideoWindow->put_WindowStyle(WS_CHILD);
	hr = m_pVideoWindow->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS);
	if (!CHECK_RESULT("IVideoWindow put_WindowStyle.",hr)) {return false;}
	/*
	RECT rect;
	GetClientRect(hWnd,&rect);
	hr = m_pVideoWindow->SetWindowPosition(
		0, 0,
		rect.right - rect.left, rect.bottom - rect.top);
	if (!CHECK_RESULT("IVideoWindow SetWindowPosition.",hr)) {return false;}
	*/
	hr = m_pVideoWindow->SetWindowForeground(OATRUE);
	if (!CHECK_RESULT("IVideoWindow SetWindowForeground.",hr)) {return false;}
	hr = m_pVideoWindow->put_Visible(OATRUE);
	if (!CHECK_RESULT("IVideoWindow put_Visible.",hr)) {return false;}

	hr = m_pVideoWindow->put_MessageDrain((OAHWND)hWnd);
	if (!CHECK_RESULT("IVideoWindow put_MessageDrain.",hr)) {return false;}
	return true;
}
//////////////////////////////////////////////////
//ビデオレンダラのウィンドウハンドルのサイズを変更する
//////////////////////////////////////////////////
bool CDirectShow::ResizeVideoRendererHWnd(RECT rcClient)
{
	assert(m_pGraphBuilder);
	assert(m_pVideoRenderer);
	assert(m_pVideoWindow);
	HRESULT hr = NOERROR;
	hr = m_pVideoWindow->SetWindowPosition(
		rcClient.left,
		rcClient.top,
		rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top);
	//フィルタ作成に失敗している場合に無効にしたい
	//if (!CHECK_RESULT("IVideoWindow SetWindowPosition.",hr)) {return false;}
	return SUCCEEDED(hr);
}
//////////////////////////////////////////////////
//ビデオレンダラの表示・非表示を切り替える(初期では表示モードになっている)
//////////////////////////////////////////////////
bool CDirectShow::ShowVideoRenderer(BOOL bShow)
{
	assert(m_pGraphBuilder);
	//assert(m_pVideoRenderer);
	assert(m_pVideoWindow);
	HRESULT hr = NOERROR;
	if (bShow)
		hr = m_pVideoWindow->put_Visible(OATRUE);
	else
		hr = m_pVideoWindow->put_Visible(OAFALSE);
	if (!CHECK_RESULT("IVideoWindow put_Visible.",hr)) {return false;}
	return true;
}
//////////////////////////////////////////////////
//ビデオレンダラの表示状態を取得する
//////////////////////////////////////////////////
bool CDirectShow::IsShowVideoRenderer()
{
	assert(m_pGraphBuilder);
	//assert(m_pVideoRenderer);
	if (!m_pVideoWindow)
		return true;
	HRESULT hr = NOERROR;
	LONG lVisible;
	hr = m_pVideoWindow->get_Visible(&lVisible);
	if (!CHECK_RESULT("IVideoWindow get_Visible.",hr)) {return false;}
	if (lVisible == OATRUE)
		return true;
	return false;
}
//////////////////////////////////////////////////
//ビデオレンダラからウィンドウハンドルを削除する
//////////////////////////////////////////////////
bool CDirectShow::ResetVideoRendererHWnd()
{
	assert(m_pGraphBuilder);
	//assert(m_pVideoRenderer);

	if (!m_pVideoWindow)
		return true;

	HRESULT hr = NOERROR;
	hr = m_pVideoWindow->put_Visible(OAFALSE);
	//TODO ここで連続エラー出てる
	//失敗しても気にしない仕様に変更
	//if (!CHECK_RESULT("IVideoWindow put_Visible.",hr)) {return false;}
	hr = m_pVideoWindow->put_Owner(NULL);
	//if (!CHECK_RESULT("IVideoWindow put_Owner.",hr)) {return false;}
	m_pVideoWindow->Release();
	m_pVideoWindow = NULL;
	return true;
}

//////////////////////////////////////////////////
//コールバック用のウィンドウハンドルを設定
//////////////////////////////////////////////////
bool CDirectShow::SetCallbackHWnd(HWND hWnd)
{
	assert(m_pGraphBuilder);
	HRESULT hr;
	if (m_pMediaEventEx) {
		m_pMediaEventEx->SetNotifyWindow(NULL, 0, 0);
		SAFE_RELEASE(m_pMediaEventEx)
	}
	hr = m_pGraphBuilder->QueryInterface(IID_IMediaEventEx,
		(LPVOID *)&m_pMediaEventEx);
	if (!CHECK_RESULT("IGraphBuilder Create IMediaEventEx.",hr)) {return false;}
	hr = m_pMediaEventEx->CancelDefaultHandling(EC_REPAINT);
	hr = m_pMediaEventEx->SetNotifyWindow((OAHWND)hWnd,WM_GRAPH_NOTIFY, 0);
	if (!CHECK_RESULT("IMediaEventEx SetNotifyWindow.",hr)) {return false;}
	return true;
}
//////////////////////////////////////////////////
//再生終了まで待つ
//////////////////////////////////////////////////
bool CDirectShow::WaitForCompletion(int nTimeOut)
{
	/*
	long eventCode;
 m_pMediaEventEx->WaitForCompletion(nTimeOut, &eventCode);
 switch (eventCode) {
 case 0:
	printf("timeout\n");
	break;
 case EC_COMPLETE:
	printf("complete\n");
	break;
 case EC_ERRORABORT:
	printf("errorabort\n");
	break;
 case EC_USERABORT:
	printf("userabort\n");
	break;
 }
 */
 return true;
}

//////////////////////////////////////////////////
//コールバック
//////////////////////////////////////////////////
LRESULT CDirectShow::OnGraphNotify()
{
	if (!m_pMediaEventEx)
		return NOERROR;
	long evCode;
	LONG_PTR param1,param2;
	// イベントを全て取得
	while (SUCCEEDED(m_pMediaEventEx->GetEvent(&evCode,(LONG_PTR*)&param1, (LONG_PTR*)&param2, 0)))
	{
		m_pMediaEventEx->FreeEventParams(evCode, param1, param2);
		switch (evCode) {
			case EC_COMPLETE:	//再生が正常に完了したことを知らせる。
				//MessageBox(NULL,_T("EC_COMPLETE"),NULL,MB_OK);
				break;
			case EC_USERABORT:	//ユーザーが再生を中断したことを知らせる。ユーザーがビデオ ウィンドウを閉じると、ビデオ レンダラがこのイベントを送信する。
				break;
			case EC_ERRORABORT:	//エラーのため再生が停止したことを知らせる。
				break;
			case EC_REPAINT:
//#ifdef _DEBUG
//				OutputDebugString(L"EC_REPAINT\n");
//#endif
				break;
		}
	}
	return NOERROR;
}

//////////////////////////////////////////////////
//サウンドレンダラの初期化
//////////////////////////////////////////////////
bool CDirectShow::InitAudioRenderer()
{
	assert(m_pGraphBuilder);
	SAFE_RELEASE(m_pAudioRenderer)
	HRESULT hr;
	hr = CoCreateInstance(CLSID_DSoundRender,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,
		(void **)&m_pAudioRenderer);
	if (!CHECK_RESULT("DSoundRenderer Create.",hr)) {return false;}
	hr = m_pGraphBuilder->AddFilter(m_pAudioRenderer,_T("DirectSound Renderer"));
	if (!CHECK_RESULT("IGraphBuilder AddFilter DirectSound Renderer.",hr)) {return false;}
	return true;
}

//////////////////////////////////////////////////
//ファイルからのレンダリングを行う
//////////////////////////////////////////////////
bool CDirectShow::RenderFile(BSTR strFilename)
{
	assert(m_pGraphBuilder);
	assert(m_pMediaControl);
	HRESULT hr = m_pMediaControl->RenderFile(strFilename);
	if (!CHECK_RESULT("IMediaControl RenderFile.",hr)) {return false;}
	return true;
}

//////////////////////////////////////////////////
//DirectShowの破棄
//////////////////////////////////////////////////
void CDirectShow::Exit()
{
	if (m_pMediaControl) {
		if (this->GetState() == State_Running) {
			this->Stop();
		}
	}

	//基準クロックを削除
	SAFE_RELEASE(m_pReferenceClock);

	//全てのフィルタデータを削除
	for (int i=0;i<(int)m_FilterData.size();i++)
	{
		if (m_pGraphBuilder && m_FilterData[i].pFilter)
			m_pGraphBuilder->RemoveFilter(m_FilterData[i].pFilter);
		SAFE_RELEASE(m_FilterData[i].pFilter)
		m_FilterData[i].strName.clear();
	}
	m_FilterData.clear();

	if (m_pVideoRenderer) {
		m_pGraphBuilder->RemoveFilter(m_pVideoRenderer);
		SAFE_RELEASE(m_pVideoRenderer)
	}
	if (m_pAudioRenderer) {
		m_pGraphBuilder->RemoveFilter(m_pAudioRenderer);
		SAFE_RELEASE(m_pAudioRenderer)
	}

	if (m_pVideoWindow){
		m_pVideoWindow->put_Visible(OAFALSE);
		m_pVideoWindow->put_Owner(NULL);
		SAFE_RELEASE(m_pVideoWindow)
	}
	if (m_pMediaEventEx){
		m_pMediaEventEx->SetNotifyWindow(NULL, 0, 0);
		SAFE_RELEASE(m_pMediaEventEx)
	}
	SAFE_RELEASE(m_pMediaControl);

	//未解放のフィルタを検索し解放する
	if (m_pGraphBuilder)
	{
		CComPtr<IEnumFilters> pEnum;
		m_pGraphBuilder->EnumFilters(&pEnum);
		pEnum->Reset();
		CComPtr<IBaseFilter> pFilter;
		ULONG nFetched1 = 0;
		while (pFilter.Release() , pEnum->Next(1,&(pFilter.p),&nFetched1) == S_OK) {
			if (!(pFilter.p)) continue;
			FILTER_INFO Info;
			pFilter->QueryFilterInfo(&Info);
			if (Info.pGraph) {
				SAFE_RELEASE(Info.pGraph);
			}
			m_pGraphBuilder->RemoveFilter(pFilter);
			pEnum->Reset();	//列挙子をリセット
			assert(_T("未解放のフィルタが検出されましたので、解放します") == 0);
		}		
	}
	
	if (m_bRegister)
	{
		CDirectShowEtc::RemoveFromRot(m_dwRegister);
		m_dwRegister = 0x0;
		m_bRegister = false;
	}

	SAFE_RELEASE(m_pCaptureGraphBuilder2)

	//デバッグ出力を停止する
#ifdef _DEBUG
	DbgTerminate();
#endif

	SAFE_RELEASE(m_pGraphBuilder)

	if (m_bComInit)
	{
		CoUninitialize();
		m_bComInit = false;
	}
}

//////////////////////////////////////////////////
//フィルタの状態を変更
//////////////////////////////////////////////////
HRESULT CDirectShow::SetState(OAFilterState ofs)
{
	assert(m_pGraphBuilder);
	assert(m_pMediaControl);
	HRESULT hr = NOERROR;
	switch (ofs)
	{
	case State_Running:	//再生
		hr = m_pMediaControl->Run();
		if (hr == S_FALSE)	//グラフは実行の準備をしているが、実行状態への移行が完了していないフィルタがある。
		{
			OAFilterState fs;
			hr = m_pMediaControl->GetState(500,&fs);
			switch (hr)
			{
			case S_OK:
				//OutputDebugString(_T("S_OK\r\n"));break;
			case VFW_S_STATE_INTERMEDIATE:
				//OutputDebugString(_T("VFW_S_STATE_INTERMEDIATE\r\n"));break;
			case VFW_S_CANT_CUE:
				//OutputDebugString(_T("VFW_S_CANT_CUE\r\n"));break;
			case E_FAIL:
				//OutputDebugString(_T("E_FAIL\r\n"));break;
			default:
				break;
			}
			if (hr == S_OK && fs == State_Running)
				return hr;
			if (!CHECK_RESULT("IMediaControl Run.",hr)) {return hr;}
		}
		break;
	case State_Paused:	//一時停止
		hr = m_pMediaControl->Pause();
		if (hr == S_FALSE)
		{
			OAFilterState fs;
			hr = m_pMediaControl->GetState(500,&fs);
			if (hr == S_OK && fs == State_Paused)
				return hr;
			if (!CHECK_RESULT("IMediaControl Pause.",hr)) {return hr;}
		}
		//if (!CHECK_RESULT("IMediaControl Pause.",hr)) {return false;}
		break;
	case State_Stopped:	//停止
		hr = m_pMediaControl->Stop();
		if (hr == S_FALSE)
		{
			OAFilterState fs;
			hr = m_pMediaControl->GetState(500,&fs);
			if (hr == S_OK && fs == State_Stopped)
				return hr;
			if (!CHECK_RESULT("IMediaControl Stop.",hr)) {return hr;}
		}
		//if (!CHECK_RESULT("IMediaControl Stop.",hr)) {return false;}
		break;
	}
	return hr;
}
//////////////////////////////////////////////////
//現在の再生状態の取得
//	State_Stopped
//	State_Paused
//	State_Running
//////////////////////////////////////////////////
OAFilterState CDirectShow::GetState()
{
	assert(m_pGraphBuilder);
	assert(m_pMediaControl);
	OAFilterState fs;
	m_pMediaControl->GetState(INFINITE,&fs);
	return fs;
}

/////////////////////////////////////////////////
//フィルタ接続を切断する
//IBaseFilter* pFilter 切断するフィルタ
//BOOL bDisconnectInputPins		入力ピンを切断するかどうか
//BOOL bDisconnectOutputPins	出力ピンを切断するかどうか
/////////////////////////////////////////////////
HRESULT CDirectShow::DisconnectFilter(IBaseFilter* pFilter,BOOL bDisconnectInputPins,BOOL bDisconnectOutputPins)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	HRESULT hr = NOERROR;
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return hr;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return hr;}
	//ピンの列挙
	CComPtr<IPin> pPin;
	while (pPin.Release(), pEnumPins->Next(1, &pPin, &nFetched) == S_OK)
	{
		if (!pPin) continue;

		PIN_DIRECTION PinDir;
		hr = pPin->QueryDirection(&PinDir);
		if (PinDir == PINDIR_INPUT && !bDisconnectInputPins) {
			continue;
		} else if (PinDir == PINDIR_OUTPUT && !bDisconnectOutputPins) {
			continue;
		}
		CComPtr<IPin> pConnectPin;
		hr = pPin->ConnectedTo(&pConnectPin);
		if (SUCCEEDED(hr) && (pConnectPin.p))
		{
			hr = m_pGraphBuilder->Disconnect(pConnectPin.p);
			hr = m_pGraphBuilder->Disconnect(pPin.p);
		}
	}
	return hr;
}
/////////////////////////////////////////////////
//フィルタ上の1つのピンを切断する
//この場合、接続の可否のかかわらず切断したのであればS_OKを返す
//IBaseFilter* pFilter 切断するフィルタ

/////////////////////////////////////////////////
HRESULT CDirectShow::DisconnectFilterPin(IBaseFilter* pFilter,int nPinIndex)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	HRESULT hr = NOERROR;
	
	CComPtr<IPin> pPin;
	pPin.Attach(CDirectShowEtc::GetPinByIndex(pFilter,nPinIndex));

	CComPtr<IPin> pConnectPin;
	hr = pPin->ConnectedTo(&(pConnectPin.p));
	if (SUCCEEDED(hr) && pConnectPin.p)
	{
		hr = m_pGraphBuilder->Disconnect(pConnectPin.p);	//接続先の切断
		hr = m_pGraphBuilder->Disconnect(pPin.p);			//接続元の切断
	}
	pPin.Release();
	pConnectPin.Release();
	return hr;
}
/////////////////////////////////////////////////
//フィルタの情報を取得する
//IBaseFilter* pFilter 情報を取得するフィルタ
//vector<PinInfoStruct>&PinInfoList ピン情報が入ったPinInfoStruct構造体の配列
//pInputPinCount 入力ピンの個数(NULLも可)
//pOutputPinCount 出力ピンの個数(NULLも可)
/////////////////////////////////////////////////
HRESULT CDirectShow::GetFilterInfo(IBaseFilter* pFilter,vector<PinInfoStruct>&PinInfoList,int* pInputPinCount,int* pOutputPinCount)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	HRESULT hr = NOERROR;
	if (pInputPinCount)
		(*pInputPinCount) = 0;
	if (pOutputPinCount)
		(*pOutputPinCount) = 0;
	PinInfoList.clear();
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return hr;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return hr;}
	IPin* pPin = NULL;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin) continue;

		PinInfoStruct pis;

		//ピンの方向を取得
		PIN_DIRECTION direction;
		hr = pPin->QueryDirection(&direction);
		if (!CHECK_RESULT("IPin QueryDirection.",hr)) {pPin->Release(); pPin = NULL; return hr;}
		pis.PinDir = direction;
		if (direction ==  PINDIR_INPUT){
			if (pInputPinCount)
				(*pInputPinCount)++;
		} else {
			if (pOutputPinCount)
				(*pOutputPinCount)++;
		}

		//ピンの名前の取得
		PIN_INFO info;
		hr = pPin->QueryPinInfo(&info);
		if (!CHECK_RESULT("IPin QueryPinInfo.",hr)) {pPin->Release(); pPin = NULL; return hr;}
		//pis.strPinName = reinterpret_cast<LPWSTR>(info.achName);
		pis.strPinName = info.achName;
		if (info.pFilter) {
			info.pFilter->Release();
			info.pFilter = NULL;
		}
		//ピンが接続されているかどうかチェック
		IPin* pConnectToPin = NULL;
		hr = pPin->ConnectedTo(&pConnectToPin);
		if (hr == VFW_E_NOT_CONNECTED) {	//接続されていない
			pis.bConnected = FALSE;
		} else if (hr == S_OK) {	//接続されている
			pis.bConnected = TRUE;
			pConnectToPin->Release();
			pConnectToPin = NULL;
		} else {	//その他のエラー
			if (!CHECK_RESULT("IPin ConnectedTo.",hr)) {pPin->Release(); pPin = NULL; return hr;}
		}
		//ピン情報を追記
		PinInfoList.push_back(pis);
		pPin->Release();
		pPin = NULL;
	}
	return S_OK;
}
/////////////////////////////////////////////////
//入力・出力ピンのピン番号を取得する
//(入力・出力ピンが１つしかない場合)
/////////////////////////////////////////////////
HRESULT CDirectShow::GetPinIndex(IBaseFilter* pFilter,int &nPinIndex,PIN_DIRECTION pin_dir)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	nPinIndex = -1;
	HRESULT hr = NOERROR;
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return hr;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return hr;}
	IPin* pPin = NULL;
	int nIndex = 0;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin) continue;
		//ピンの方向を取得
		PIN_DIRECTION direction;
		hr = pPin->QueryDirection(&direction);
		if (!CHECK_RESULT("IPin QueryDirection.",hr)) {pPin->Release(); pPin = NULL; return hr;}
		if (direction == pin_dir)
		{
			nPinIndex = nIndex;
			pPin->Release();
			break;
		}
		nIndex++;
		pPin->Release();
		pPin = NULL;
	}
	if (nPinIndex == -1)
		return E_FAIL;
	return hr;
}

/////////////////////////////////////////////////
//ピンのメディアタイプの列挙を行う
//以前にSetCaptureMediaType()が成功していた場合、1つだけのMediaTypeListが返る
//MediaTypeListは取得したらReleaseMediaTypeList()しておくこと
//IBaseFilter* pFilter	情報を取得するフィルタ
//int nPinIndex			ピンの番号
//vector<AM_MEDIA_TYPE> &MediaTypeList 列挙されたAM_MEDIA_TYPE構造体の配列(戻り値)
/////////////////////////////////////////////////
HRESULT CDirectShow::EnumMediaType(IBaseFilter* pFilter,int nPinIndex,vector<AM_MEDIA_TYPE> &MediaTypeList)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return hr;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return hr;}
	IPin* pPin = NULL;
	int nIndex = 0;
	BOOL bFind = FALSE;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin) continue;
		
		if (nIndex == nPinIndex)
		{
			CComPtr<IEnumMediaTypes> pEnumMediaType;
			hr = pPin->EnumMediaTypes(&pEnumMediaType);
			if (!CHECK_RESULT("IPin EnumMediaTypes.",hr)){return hr;}
			hr = pEnumMediaType->Reset();
			if (!CHECK_RESULT("IPin Reset.",hr)){return hr;}
			AM_MEDIA_TYPE *amt = NULL;
			while (pEnumMediaType->Next(1,&amt,NULL) == S_OK) {
				MediaTypeList.push_back(*amt);
			}
			bFind = TRUE;
		}
		pPin->Release();
		pPin = NULL;
		nIndex++;
		if (bFind)
			break;
	}
	return hr;
}

/////////////////////////////////////////////////
//現在接続されているピンのメディアタイプを取得
//ピンが接続されていない場合、VFW_E_NOT_CONNECTEDが返る
//AM_MEDIA_TYPEは取得したらReleaseMediaType()しておくこと
/////////////////////////////////////////////////
HRESULT CDirectShow::GetConnectionMediaType(IBaseFilter* pFilter,int nPinIndex,AM_MEDIA_TYPE &amt)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;

	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return hr;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return hr;}
	IPin* pPin = NULL;
	int nIndex = 0;
	BOOL bFind = FALSE;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin) continue;
		
		if (nIndex == nPinIndex)
		{
			hr = pPin->ConnectionMediaType(&amt);
			bFind = TRUE;
		}
		pPin->Release();
		pPin = NULL;
		nIndex++;
		if (bFind)
			break;
	}
	return hr;
}

/////////////////////////////////////////////////
//フィルタ同士を接続する(コンバート接続)
//IBaseFilter* pSrc		接続元のフィルタ
//IBaseFilter* pDest	接続先のフィルタ
//AM_MEDIA_TYPE* pAmt 接続する優先メディアタイプ(NULLも可,NULLの場合は優先メディアタイプを指定しない)
//LPCWSTR pSrcPinName	接続元のフィルタの出力ピン名(NULLも可)
//int* pID				インテリジェント接続において、新たに追加されたフィルタが存在する場合、フィルタIDを返す
/////////////////////////////////////////////////
HRESULT CDirectShow::ConnectFilterConvert(IBaseFilter* pSrc,IBaseFilter* pDest,AM_MEDIA_TYPE* pAmt,LPCWSTR pSrcPinName,int* pID)
{
	if (pID)
		*pID = -1;
	assert(m_pGraphBuilder);
	assert(m_pCaptureGraphBuilder2);
	assert(pSrc);
	HRESULT hr = NOERROR;

	if (pSrcPinName) {
		CComPtr<IPin> pOutPin;
		hr = GetPin(pSrc,PINDIR_OUTPUT,pOutPin,pAmt,pSrcPinName);
		if (!CHECK_RESULT("IBaseFilter GetPin PINDIR_OUTPUT.",hr)) {return hr;}
		assert(pOutPin);
		hr = m_pCaptureGraphBuilder2->RenderStream(NULL,((pAmt)?&(pAmt->majortype):NULL),pOutPin,NULL,pDest);
	} else {
		hr = m_pCaptureGraphBuilder2->RenderStream(NULL,((pAmt)?&(pAmt->majortype):NULL),pSrc,NULL,pDest);
	}

	if (FAILED(hr)) {	//インテリジェント接続に失敗している
		return hr;}
	if (!CHECK_RESULT("ICaptureGraphBuilder2 RenderStream.",hr)) {return hr;}

	//インテリジェント接続で追加されたフィルタグラフを検索する
	CComPtr<IPin> pOutPin;
	bool bOut;
	hr = GetPin(pSrc,PINDIR_OUTPUT,pOutPin,pAmt,pSrcPinName);
	CComPtr<IEnumFilters> pEnum;
	hr = m_pGraphBuilder->EnumFilters(&pEnum);
	hr = pEnum->Reset();
	IBaseFilter *pFilter = NULL;
	ULONG nFetched1 = 0;
	while (pEnum->Next(1,&pFilter,&nFetched1) == S_OK) {
		if (!pFilter) continue;
		bOut = false;
		//ピンを取得
		CComPtr<IEnumPins> pEnumPins;
		hr = pFilter->EnumPins(&pEnumPins);
		hr = pEnumPins->Reset();
		IPin* pPin = NULL;
		ULONG nFetched2 = 0;
		while (pEnumPins->Next(1,&pPin,&nFetched2) == S_OK)
		{
			if (!pPin) continue;
			IPin* pConnectedPin = NULL;
			hr = pPin->ConnectedTo(&pConnectedPin);	//接続先を取得
			if (pConnectedPin == pOutPin)
				bOut = true;
			if (pConnectedPin) {
				pConnectedPin->Release();
				pConnectedPin = NULL;
			}
			pPin->Release();
			pPin = NULL;
		}
		if (bOut && pFilter != pSrc && pFilter != pDest)
		{
			//FILTER_INFO Info;
			//hr = pFilter->QueryFilterInfo(&Info);
			//Info.pGraph->Release();
			//フィルタをフィルタリストに追加
			this->AddFilterList(pFilter,pID);
			return hr;	//管理フィルタに置くためにRelease()しない
		}

		pFilter->Release();
		pFilter = NULL;
	}

	return hr;
}
/////////////////////////////////////////////////
//フィルタ同士を接続する(ダイレクト接続)
//IBaseFilter* pSrc		接続元のフィルタ
//IBaseFilter* pDest	接続先のフィルタ
//AM_MEDIA_TYPE* pAmt 接続する優先メディアタイプ(NULLも可,NULLの場合は優先メディアタイプを指定しない)
//LPCWSTR pSrcPinName	接続元のフィルタの出力ピン名(NULLも可)
//LPCWSTR pDestPinName	接続先のフィルタの入力ピン名(NULLも可)
/////////////////////////////////////////////////
HRESULT CDirectShow::ConnectFilterDirect(IBaseFilter* pSrc,IBaseFilter* pDest,AM_MEDIA_TYPE* pAmt,LPCWSTR pSrcPinName,LPCWSTR pDestPinName)
{
	assert(m_pGraphBuilder);
	assert(pSrc);
	assert(pDest);
	HRESULT hr = NOERROR;

	/*
	//TODO こっちだと何故かVMRで繋がらない現象ありなので要調査
	IPin *pInputPin,*pOutputPin;
	pInputPin = CDirectShowEtc::GetPinByName(pSrc,PINDIR_OUTPUT,pSrcPinName,pAmt);
	assert(pInputPin);
	pOutputPin = CDirectShowEtc::GetPinByName(pDest,PINDIR_INPUT,pDestPinName,NULL);
	assert(pOutputPin);
	hr = m_pGraphBuilder->ConnectDirect(pOutputPin,pInputPin,pAmt);
	pInputPin->Release();
	pOutputPin->Release();
	*/

	CComPtr<IPin> pOutPin,pInPin;
	hr = GetPin(pSrc,PINDIR_OUTPUT,pOutPin,pAmt,pSrcPinName);
	if (!CHECK_RESULT("IBaseFilter GetPin PINDIR_OUTPUT.",hr)) {return hr;}
	assert(pOutPin.p);
	hr = GetPin(pDest,PINDIR_INPUT,pInPin ,NULL,pDestPinName);
	if (!CHECK_RESULT("IBaseFilter GetPin PINDIR_INPUT.",hr)) {return hr;}
	assert(pInPin.p);
	hr = m_pGraphBuilder->ConnectDirect(pOutPin,pInPin,pAmt);
	pOutPin.Release();
	pInPin.Release();

	return hr;	//接続できたかどうかはhrで判断する
}
/////////////////////////////////////////////////
//フィルタからピンを取得するサブモジュール
//IBaseFilter* pFilter			取得対象となるフィルタ
//PIN_DIRECTION pin_direction	ピンの方向(PINDIR_OUTPUTかPINDIR_INPUT)
//CComPtr<IPin> &pin			取得されたピンが返る(戻り値)
//AM_MEDIA_TYPE* pAmt			メディアタイプの条件
//LPCWSTR pPinName				検索されるピンの名前
/////////////////////////////////////////////////
HRESULT CDirectShow::GetPin(IBaseFilter* pFilter,PIN_DIRECTION pin_direction,CComPtr<IPin> &pin,AM_MEDIA_TYPE* pAmt,LPCWSTR pPinName)
{
	assert(pFilter);
	HRESULT hr = NOERROR;
	CComPtr<IEnumPins> pEnumPins;
	ULONG nFetched1 = 0;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return hr;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return hr;}

	//ピンの列挙
	while (pin.Release(),pEnumPins->Next(1, &pin.p, &nFetched1) == S_OK)
	{
		if (!pin) continue;

		//入出力Pinのどちらか取得
		PIN_DIRECTION this_direction;
		hr = pin->QueryDirection(&this_direction);
		if (!CHECK_RESULT("IPin QueryDirection.",hr)) {return hr;}
		if (this_direction != pin_direction)	//入出力pinが違う
			continue;

		//ピン名指定がある場合はピン名のチェックを行う
		if (pPinName != NULL) {
			PIN_INFO PinInfo;
			hr = pin->QueryPinInfo(&PinInfo);
			if (PinInfo.pFilter)
				PinInfo.pFilter->Release();
			if (_tcsnccmp(pPinName, PinInfo.achName, _tcslen(pPinName)) != 0)
				continue;
		}
		
		//メディアタイプが指定されていない場合はこのピンで決定する
		if (!pAmt) {
			return S_OK;
		}
		//メディアタイプの受け入れを確認する
		//http://msdn.microsoft.com/ja-jp/library/cc369447.aspx
		hr = pin->QueryAccept(pAmt);
		if (hr == S_OK) {
			return S_OK;
		} else {
			continue;
		}
	}
	//失敗の場合
	pin.Release();
	return E_FAIL;
}


/////////////////////////////////////////////////
//キャプチャデバイス名の列挙
//vector<wstring> strDeviceStrings	デバイス文字列の列挙
//bVideo							ビデオ or オーディオ
//bName								名前(フレンドリネーム) or ID(デバイスパス)
//////////////////////////////////////////////////
bool CDirectShow::EnumCaptureDevice(vector<wstring>& strDeviceStrings,bool bVideo,bool bName)
{
	HRESULT hr;
	strDeviceStrings.clear();

	CComPtr<ICreateDevEnum> pCreateDevEnum;
	CComPtr<IEnumMoniker> pEnumMoniker;
	IMoniker *pMoniker = NULL;
	ULONG nFetched = 0;

	//デバイスを列挙するためのCreateDevEnumを作成する
	hr = CoCreateInstance(CLSID_SystemDeviceEnum,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum,
		(PVOID *)&pCreateDevEnum);
	if (!CHECK_RESULT("ICreateDevEnum Create.",hr)) {return false;}

	//VideoInputDeviceを列挙するためのEnumMonikerを生成
	if (bVideo) {
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
			&pEnumMoniker, 0);
	} else {
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,
			&pEnumMoniker, 0);
	}
	if (!CHECK_RESULT("ICreateDevEnum CreateClassEnumerator.",hr)) {return false;}
	if (pEnumMoniker == NULL)
	{
		return true;			//列挙できるキャプチャデバイスがない
	}

	// EnumMonikerをResetする。Resetすると最初から数えなおす
	hr = pEnumMoniker->Reset();
	if (!CHECK_RESULT("IEnumMoniker Reset.",hr)) {return false;}
	while (pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK) {
		if (!pMoniker) continue;
		CComPtr<IPropertyBag> pPropertyBag;
		// IPropertyBagにbindする
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void **)&pPropertyBag);
		//if (!CHECK_RESULT("IMoniker BindToStorage.",hr)) {return false;}
		if (FAILED(hr)) {
			pMoniker->Release(); pMoniker = NULL;
			continue;
		}
		VARIANT var;
		VariantInit(&var);
		var.vt = VT_BSTR;
		if (bName) {
			// FriendlyNameを取得
			hr = pPropertyBag->Read(L"FriendlyName", &var, 0);
			//if (!CHECK_RESULT("IPropertyBag Read FriendlyName.",hr)) {return false;}
			//this->OutputDebugWindow(_T("Device FriendlyName:%s\n"),var.bstrVal);
		} else {
			// DevicePathを取得
			hr = pPropertyBag->Read(L"DevicePath", &var, 0);
			//if (!CHECK_RESULT("IPropertyBag Read DevicePath.",hr)) {return false;}
			//this->OutputDebugWindow(_T("Device FriendlyName:%s\n"),var.bstrVal);
		}
		if (FAILED(hr)) {
			VariantClear(&var);
			pMoniker->Release();
			pMoniker = NULL;
			continue;
		}
		//文字列を追記
		strDeviceStrings.push_back(var.bstrVal);

		VariantClear(&var);
		pMoniker->Release();
		pMoniker = NULL;
	}
	if (pMoniker)
		pMoniker->Release();
	return true;
}

//////////////////////////////////////////////////
//キャプチャデバイスの追加
//bVideo ビデオ or オーディオ
//bName	名前(フレンドリネーム) or ID(デバイスパス)
//////////////////////////////////////////////////
bool CDirectShow::AddCaptureDevice(wstring strDeviceName,bool bVideo,bool bName,int* pID)
{
	assert(m_pGraphBuilder);

	HRESULT hr = NOERROR;
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	CComPtr<IEnumMoniker> pEnumMoniker;
	IMoniker *pMoniker = NULL;
	ULONG nFetched = 0;
	//デバイスを列挙するためのCreateDevEnumを作成する
	hr = CoCreateInstance(CLSID_SystemDeviceEnum,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum,
		(PVOID *)&pCreateDevEnum);
	if (!CHECK_RESULT("ICreateDevEnum Create.",hr)) {return false;}

	//VideoInputDeviceを列挙するためのEnumMonikerを生成
	if (bVideo) {
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
			&pEnumMoniker, 0);
	} else {
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,
			&pEnumMoniker, 0);
	}
	if (!CHECK_RESULT("ICreateDevEnum CreateClassEnumerator.",hr)) {return false;}
	if (pEnumMoniker == NULL) {
		return false;			//列挙できるキャプチャデバイスがない
	}
	// EnumMonikerをResetする
	hr = pEnumMoniker->Reset();
	if (!CHECK_RESULT("IEnumMoniker Reset.",hr)) {return false;}
	bool bFindFlag = false;
	while (pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK)
	{
		if (!pMoniker) continue;

		CComPtr<IPropertyBag> pPropertyBag;
		// IPropertyBagにbindする
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void **)&pPropertyBag);
		//if (!CHECK_RESULT("IMoniker BindToStorage.",hr)) {return false;}
		if (FAILED(hr)) {
			pMoniker->Release(); pMoniker = NULL;
			continue;
		}
		VARIANT var;
		VariantInit(&var);
		var.vt = VT_BSTR;
		if (bName) {	// FriendlyNameを取得
			hr = pPropertyBag->Read(L"FriendlyName", &var, 0);
			//if (!CHECK_RESULT("IPropertyBag Read FriendlyName.",hr)) {return false;}
		} else {		// Device Path(ID)を取得
			hr = pPropertyBag->Read(L"DevicePath", &var, 0);
			//if (!CHECK_RESULT("IPropertyBag Read DevicePath.",hr)) {return false;}
		}
		if (FAILED(hr)) {
			VariantClear(&var);
			pMoniker->Release(); pMoniker = NULL;
			continue;
		}

		if ( _tcscmp(var.bstrVal,strDeviceName.c_str()) == 0)
			bFindFlag = true;
		else
			bFindFlag = false;
		VariantClear(&var);

		if (bFindFlag == true)
		{
			IBaseFilter *pCap = NULL;
			VARIANT name;
			VariantInit(&name);
			name.vt = VT_BSTR;
			hr = pPropertyBag->Read(L"FriendlyName", &name, 0);

			//キャプチャデバイスのバインド
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);
			if (!CHECK_RESULT("IMoniker BindToObject.",hr)) {return false;}

			//キャプチャフィルタの追加
			hr = m_pGraphBuilder->AddFilter(pCap, name.bstrVal);
			if (!CHECK_RESULT("IGraphBuilder AddFilter.",hr)) {return false;}

			//空いているvectorを探す
			bool bInsert = false;
			for (int i=0;i<(int)m_FilterData.size();i++) {
				if (!m_FilterData[i].pFilter) {
					//m_FilterData[i].guid = guid;
					m_FilterData[i].pFilter = pCap;
					m_FilterData[i].strName = name.bstrVal;
					*pID = i;
					bInsert = true;
					break;
				}
			}
			//空きがないのなら末尾に追加
			if (!bInsert) {
				FilterData Data;
				//Data.guid = guid;
				Data.pFilter = pCap;
				Data.strName = name.bstrVal;
				*pID = (int)m_FilterData.size();
				m_FilterData.push_back(Data);
			}
			VariantClear(&name);
		}

		pMoniker->Release();
		if (bFindFlag == TRUE)
			break;
	}
	return bFindFlag;
}
//////////////////////////////////////////////////
//キャプチャデバイスのメディアタイプを取得や設定を行う
//IBaseFilter* pFilter 列挙するフィルタ
//int nPinIndex ピンのインデックス
//BOOL bSet		設定するか否か
//AM_MEDIA_TYPE* pAmt 設定/取得するメディアタイプ (NULLを指定すると規定のフォーマットで初期化される)
//////////////////////////////////////////////////
HRESULT CDirectShow::GetSetCaptureMediaType(IBaseFilter* pFilter,int nPinIndex,BOOL bSet,AM_MEDIA_TYPE* pAmt)
{
	assert(pFilter);
	assert(nPinIndex >= 0);
	//assert(pAmt);
	HRESULT hr = NOERROR;

	CComPtr<IPin> pPin;
	pPin.Attach(CDirectShowEtc::GetPinByIndex(pFilter,nPinIndex));
	if (!pPin.p)
		return E_FAIL;

	CComPtr<IAMStreamConfig> pAMStreamConfig;
	hr = pPin->QueryInterface(IID_IAMStreamConfig,(void**)&pAMStreamConfig);
	if (FAILED(hr))
		return hr;
	if (bSet)
		hr = pAMStreamConfig->SetFormat(pAmt);
	else
		hr = pAMStreamConfig->GetFormat(&pAmt);
	if (FAILED(hr))
		return hr;
	return hr;
}


//////////////////////////////////////////////////
//ビデオ圧縮に関する情報を得る(テストコード)
//////////////////////////////////////////////////
HRESULT CDirectShow::GetVideoCompressInfo(IBaseFilter* pFilter,int nPinIndex)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return hr;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return hr;}
	IPin* pPin = NULL;
	int nIndex = 0;
	BOOL bFind = FALSE;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin) continue;
		
		if (nIndex == nPinIndex)
		{
			CComPtr<IAMVideoCompression> pAMVideoCompression;
			hr = pPin->QueryInterface(IID_IAMVideoCompression,(void**)&pAMVideoCompression);
			/*
			CComPtr<IEnumMediaTypes> pEnumMediaType;
			hr = pPin->EnumMediaTypes(&pEnumMediaType);
			if (!CHECK_RESULT("IPin EnumMediaTypes.",hr)){return hr;}
			hr = pEnumMediaType->Reset();
			if (!CHECK_RESULT("IPin Reset.",hr)){return hr;}
			AM_MEDIA_TYPE *amt = NULL;
			while (pEnumMediaType->Next(1,&amt,NULL) == S_OK) {
				MediaTypeList.push_back(*amt);
			}
			*/
			bFind = TRUE;
		}
		pPin->Release();
		pPin = NULL;
		nIndex++;
		if (bFind)
			break;
	}
	return hr;
}

//////////////////////////////////////////////////
//管理されるフィルタにフィルタの追加
//////////////////////////////////////////////////
HRESULT CDirectShow::AddFilterList(IBaseFilter* pFilter,int* pID)
{
	HRESULT hr = NOERROR;
	GUID guid;
	hr = pFilter->GetClassID(&guid);
	FILTER_INFO Info;
	hr = pFilter->QueryFilterInfo(&Info);
	if (Info.pGraph != NULL)
	{
		Info.pGraph->Release();
	}

	//空いているvectorを探す
	for (int i=0;i<(int)m_FilterData.size();i++) {
		if (!m_FilterData[i].pFilter) {
			m_FilterData[i].guid = guid;
			m_FilterData[i].pFilter = pFilter;
			m_FilterData[i].strName = Info.achName;
			if (pID)
				*pID = i;
			return S_OK;
		}
	}
	//空きがないのなら末尾に追加
	FilterData Data;
	Data.guid = guid;
	Data.pFilter = pFilter;
	Data.strName = Info.achName;
	if (pID)
		*pID = (int)m_FilterData.size();
	m_FilterData.push_back(Data);
	return S_OK;
}

//////////////////////////////////////////////////
//フィルタの追加
//////////////////////////////////////////////////
HRESULT CDirectShow::AddFilter(GUID guid,LPCWSTR strName,int* pID)
{
	assert(m_pGraphBuilder);
	HRESULT hr = NOERROR;
	IBaseFilter* pFilter = NULL;

	hr = CoCreateInstance(guid,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,
		(void **)&pFilter);
	if (SUCCEEDED(hr))
	{
		if (!CHECK_RESULT("AddCreateFilter.",hr)) {return hr;}
		hr = m_pGraphBuilder->AddFilter(pFilter, strName);
		if (!CHECK_RESULT("IGraph Builder AddFilter.",hr)) {return hr;}
		hr = AddFilterList(pFilter,pID);
		return hr;
	}
	
	//一部のフィルタはCoCreateInstance()では生成できないので、
	//System Device Enumeratorまたはフィルタマッパーを使って生成しなくてはならない
	//http://msdn.microsoft.com/ja-jp/library/cc371168.aspx
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum,(void**)&pCreateDevEnum);
	if (FAILED(hr))
		return hr;
	CComPtr<IEnumMoniker> pEnumMoniker;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_LegacyAmFilterCategory,&pEnumMoniker,0);
	if (hr != S_OK)
	{
		CHECK_RESULT("ICreateDevEnum CreateClassEnumerator.",hr);
		return hr;
	}
	CComPtr<IMoniker> pMoniker;
	ULONG cFetched;
	while (pEnumMoniker->Next(1,&(pMoniker.p),&cFetched) == S_OK)
	{
		CComPtr<IPropertyBag> pPropertyBag;
		GUID comp_guid = GUID_NULL;
		hr = pMoniker->BindToStorage(0,0,IID_IPropertyBag,(void**)&pPropertyBag);
		if (SUCCEEDED(hr))
		{
			VARIANT varCLSID;
			VariantInit(&varCLSID);
			hr = pPropertyBag->Read(L"CLSID",&varCLSID,0);
			if (SUCCEEDED(hr))
			{
				CDirectShowEtc::StringToGUID(varCLSID.bstrVal,comp_guid);
			}
			VariantClear(&varCLSID);
		}
		if (comp_guid == guid)	//GUIDが一致
		{
			//フィルタの追加
			hr = pMoniker->BindToObject(NULL,NULL,IID_IBaseFilter,(void**)&pFilter);
			if (!CHECK_RESULT("IMoniker BindToObject IID_IBaseFilter.",hr)) {return hr;}
			hr = m_pGraphBuilder->AddFilter(pFilter, strName);
			if (!CHECK_RESULT("IGraph Builder AddFilter.",hr)) {return hr;}
			hr = AddFilterList(pFilter,pID);
			return hr;
		}

		pMoniker.p->Release();
		pMoniker.p = NULL;
	}
	return hr;
}
//////////////////////////////////////////////////
//DMOフィルタの追加
//GUID guidCategory DMOフィルタのカテゴリ
//	guidCategory一覧
//		DMOのカテゴリ名					DirectShowの互換カテゴリ名
//		DMOCATEGORY_AUDIO_ENCODER		CLSID_AudioCompressorCategory
//		DMOCATEGORY_AUDIO_DECODER		CLSID_LegacyAmFilterCategory
//		DMOCATEGORY_AUDIO_EFFECT
//		DMOCATEGORY_VIDEO_ENCODER		CLSID_VideoCompressorCategory
//		DMOCATEGORY_VIDEO_DECODER		CLSID_LegacyAmFilterCategory
//		DMOCATEGORY_VIDEO_EFFECT
//		DMOCATEGORY_AUDIO_CAPTURE_EFFECT
//////////////////////////////////////////////////
HRESULT CDirectShow::AddDMOFilter(GUID guid,GUID guidCategory,LPCWSTR strName,int* pID)
{
	assert(m_pGraphBuilder);
	HRESULT hr = NOERROR;
	IBaseFilter* pFilter = NULL;

	hr = CoCreateInstance(CLSID_DMOWrapperFilter,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,
		(void **)&pFilter);
	if (!CHECK_RESULT("AddCreateFilter.",hr)) {return hr;}

	IDMOWrapperFilter* pDMOWrapperFilter;
	hr = pFilter->QueryInterface(
		IID_IDMOWrapperFilter,
		(void **)&pDMOWrapperFilter);
	if (!CHECK_RESULT("IBaseFilter QueryInterface IDMOWrapperFilter.",hr)) {return hr;}
	hr = pDMOWrapperFilter->Init(guid, guidCategory); 
	if (!CHECK_RESULT("IDMOWrapperFilter Init.",hr)) {return hr;}
	SAFE_RELEASE(pDMOWrapperFilter);

	hr = m_pGraphBuilder->AddFilter(pFilter, strName);
	if (!CHECK_RESULT("IGraph Builder AddFilter.",hr)) {return hr;}

	hr = AddFilterList(pFilter,pID);
	return hr;
}


//////////////////////////////////////////////////
//フィルタの削除
//////////////////////////////////////////////////
bool CDirectShow::RemoveFilter(IBaseFilter* pFilter)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	for (int i=0;i<(int)m_FilterData.size();i++) {
		if (m_FilterData[i].pFilter == pFilter) {
			HRESULT hr = m_pGraphBuilder->RemoveFilter(pFilter);
			if (!CHECK_RESULT("IGraphBuilder RemoveFilter.",hr)) {return false;}
			//m_FilterData[i].guid = {};
			SAFE_RELEASE(m_FilterData[i].pFilter);
			m_FilterData[i].pFilter = NULL;
			m_FilterData[i].strName.clear();
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////
//フィルタをIDから検索
//////////////////////////////////////////////////
IBaseFilter* CDirectShow::GetFilterByID(const int nID)
{
	assert(m_pGraphBuilder);
	assert(nID >= 0 && nID < (int)m_FilterData.size());
	assert(m_FilterData[nID].pFilter);
	return m_FilterData[nID].pFilter;
}
//////////////////////////////////////////////////
//フィルタを名前から検索
//////////////////////////////////////////////////
IBaseFilter* CDirectShow::GetFilterByName(LPCWSTR strName)
{
	assert(m_pGraphBuilder);
	for (int i=0;i<(int)m_FilterData.size();i++) {
		if (m_FilterData[i].strName.compare(strName) == 0)
			return m_FilterData[i].pFilter;
	}
	return NULL;
}
//////////////////////////////////////////////////
//キャプチャデバイス(ビデオ)のメディアタイプを列挙する
//IBaseFilter* pFilter 列挙するフィルタ
//int nPinIndex ピンのインデックス
//vector<AM_MEDIA_TYPE> &MediaTypeList メディアタイプの配列リスト
//vector<VIDEO_STREAM_CONFIG_CAPS> &VideoCapsList ビデオキャプチャタイプの配列リスト
//////////////////////////////////////////////////
HRESULT CDirectShow::EnumVideoCaptureMediaType(IBaseFilter* pFilter, int nPinIndex, vector<AM_MEDIA_TYPE> &MediaTypeList,vector<VIDEO_STREAM_CONFIG_CAPS> &VideoCapsList)
{
	assert(pFilter);
	assert(nPinIndex >= 0);
	CDirectShowEtc::ReleaseMediaTypeList(MediaTypeList);
	MediaTypeList.clear();
	HRESULT hr = NOERROR;

	//ピンを列挙
	CComPtr<IEnumPins> pEnumPins;
	IPin* pPin = NULL;
	ULONG nFetched = 0;
	hr = pFilter->EnumPins(&pEnumPins);
	hr = pEnumPins->Reset();
	int nIndex = 0;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) 
	{
		if (!pPin) continue;
		if (nIndex != nPinIndex) {
			nIndex++;
			pPin->Release();
			continue;
		}

		PIN_DIRECTION this_dir;
		hr = pPin->QueryDirection(&this_dir);
		if (this_dir != PINDIR_OUTPUT) {
			pPin->Release();
			return E_FAIL;	//出力ピンでない
		}

		CComPtr<IAMStreamConfig> pAMStreamConfig;
		hr = pPin->QueryInterface(IID_IAMStreamConfig,(void**)&pAMStreamConfig);
		int iCount = -1;	//列挙されるフォーマット数
		int iSize = -1;		//構成構造体のサイズ(バイト単位)
		hr = pAMStreamConfig->GetNumberOfCapabilities(&iCount,&iSize);
		if (iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
			continue;
		}
		for (int i=0;i<iCount;i++)
		{
			AM_MEDIA_TYPE* pAmt = NULL;
			VIDEO_STREAM_CONFIG_CAPS VideoStreamConfigCaps;
			hr = pAMStreamConfig->GetStreamCaps(i,&pAmt,(BYTE*)&VideoStreamConfigCaps);
			if (!pAmt) continue;
			MediaTypeList.push_back(*pAmt);
			VideoCapsList.push_back(VideoStreamConfigCaps);
		}
		pPin->Release();
		pPin = NULL;
		return hr;
	}
	return hr;
}
//////////////////////////////////////////////////
//キャプチャデバイス(オーディオ)のメディアタイプを列挙する
//IBaseFilter* pFilter 列挙するフィルタ
//int nPinIndex ピンのインデックス
//vector<AM_MEDIA_TYPE> &MediaTypeList メディアタイプの配列リスト
//vector<VIDEO_STREAM_CONFIG_CAPS> &VideoCapsList ビデオキャプチャタイプの配列リスト
//////////////////////////////////////////////////
HRESULT CDirectShow::EnumAudioCaptureMediaType(IBaseFilter* pFilter, int nPinIndex, vector<AM_MEDIA_TYPE> &MediaTypeList,vector<AUDIO_STREAM_CONFIG_CAPS> &AudioCapsList)
{
	assert(pFilter);
	assert(nPinIndex >= 0);
	CDirectShowEtc::ReleaseMediaTypeList(MediaTypeList);
	MediaTypeList.clear();
	HRESULT hr = NOERROR;

	//ピンを列挙
	CComPtr<IEnumPins> pEnumPins;
	IPin* pPin = NULL;
	ULONG nFetched = 0;
	hr = pFilter->EnumPins(&pEnumPins);
	hr = pEnumPins->Reset();
	int nIndex = 0;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) 
	{
		if (!pPin) continue;
		if (nIndex != nPinIndex) {
			nIndex++;
			pPin->Release();
			continue;
		}

		PIN_DIRECTION this_dir;
		hr = pPin->QueryDirection(&this_dir);
		if (this_dir != PINDIR_OUTPUT) {
			pPin->Release();
			return E_FAIL;	//出力ピンでない
		}

		CComPtr<IAMStreamConfig> pAMStreamConfig;
		hr = pPin->QueryInterface(IID_IAMStreamConfig,(void**)&pAMStreamConfig);
		if (FAILED(hr)) {
			pPin->Release();
			return E_FAIL;
		}
		int iCount = -1;	//列挙されるフォーマット数
		int iSize = -1;		//構成構造体のサイズ(バイト単位)
		hr = pAMStreamConfig->GetNumberOfCapabilities(&iCount,&iSize);
		if (iSize != sizeof(AUDIO_STREAM_CONFIG_CAPS)) {
			continue;
		}
		for (int i=0;i<iCount;i++)
		{
			AM_MEDIA_TYPE* pAmt = NULL;
			AUDIO_STREAM_CONFIG_CAPS AudioStreamConfigCaps;
			hr = pAMStreamConfig->GetStreamCaps(i,&pAmt,(BYTE*)&AudioStreamConfigCaps);
			if (!pAmt) continue;
			MediaTypeList.push_back(*pAmt);
			AudioCapsList.push_back(AudioStreamConfigCaps);
		}
		pPin->Release();
		pPin = NULL;
		return hr;
	}
	return hr;
}
//////////////////////////////////////////////////
//キャプチャデバイスの現在のメディアタイプを取得する
//////////////////////////////////////////////////
HRESULT CDirectShow::GetMediaType(IBaseFilter* pFilter,int nPinIndex,AM_MEDIA_TYPE &Amt)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;

	//ピンを列挙
	CComPtr<IEnumPins> pEnumPins;
	IPin* pPin = NULL;
	ULONG nFetched = 0;
	hr = pFilter->EnumPins(&pEnumPins);
	hr = pEnumPins->Reset();
	int nIndex = 0;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) 
	{
		if (!pPin) continue;
		if (nIndex != nPinIndex) {
			nIndex++;
			pPin->Release();
			continue;
		}
		PIN_DIRECTION this_dir;
		hr = pPin->QueryDirection(&this_dir);
		if (this_dir != PINDIR_OUTPUT) {
			pPin->Release();
			return E_FAIL;	//出力ピンでなかった
		}

		CComPtr<IAMStreamConfig> pAMStreamConfig;
		hr = pPin->QueryInterface(IID_IAMStreamConfig,(void**)&pAMStreamConfig);
		AM_MEDIA_TYPE* pAmt;
		hr = pAMStreamConfig->GetFormat(&pAmt);
		Amt = *pAmt;
		pPin->Release();
		pPin = NULL;
		CHECK_RESULT("IAMStreamConfig SetFormat.",hr);
		return hr;
	}
	return E_FAIL;
}

//////////////////////////////////////////////////
//キャプチャデバイスにメディアタイプの設定を行う
//IBaseFilter* pFilter 列挙するフィルタ
//int nPinIndex ピンのインデックス
//AM_MEDIA_TYPE* pAmt 設定するメディアタイプ (NULLを指定すると規定のフォーマットで初期化される)
//////////////////////////////////////////////////
HRESULT CDirectShow::SetMediaType(IBaseFilter* pFilter,int nPinIndex,AM_MEDIA_TYPE* pAmt)
{
	assert(m_pGraphBuilder);
	assert(pFilter);
	assert(nPinIndex >= 0);
	assert(pAmt);
	HRESULT hr = NOERROR;

	//ピンを列挙
	CComPtr<IEnumPins> pEnumPins;
	IPin* pPin = NULL;
	ULONG nFetched = 0;
	hr = pFilter->EnumPins(&pEnumPins);
	hr = pEnumPins->Reset();
	int nIndex = 0;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) 
	{
		if (!pPin) continue;
		if (nIndex != nPinIndex) {
			nIndex++;
			pPin->Release();
			continue;
		}
		PIN_DIRECTION this_dir;
		hr = pPin->QueryDirection(&this_dir);
		if (this_dir != PINDIR_OUTPUT) {
			pPin->Release();
			return E_FAIL;	//出力ピンでなかった
		}

		CComPtr<IAMStreamConfig> pAMStreamConfig;
		hr = pPin->QueryInterface(IID_IAMStreamConfig,(void**)&pAMStreamConfig);
		if (!CHECK_RESULT("IPin QueryInterface IAMStreamConfig.",hr)){pPin->Release(); return hr;}
		hr = pAMStreamConfig->SetFormat(pAmt);
		pPin->Release();
		pPin = NULL;
		CHECK_RESULT("IAMStreamConfig SetFormat.",hr);
		return hr;
	}
	return E_FAIL;
}

//////////////////////////////////////////////////
//プロパティページの表示
//////////////////////////////////////////////////
HRESULT CDirectShow::ShowPropertyDialog(IBaseFilter *pFilter,HWND hWnd)
{
	assert(m_pGraphBuilder);
	assert(pFilter);

	HRESULT hr;
	ISpecifyPropertyPages *pProp = NULL;
	hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	//フィルタがプロパティページを持っていない場合もあるので注意
	if (SUCCEEDED(hr))
	{
		// フィルタの名前と IUnknown ポインタを取得する。
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo); 
		IUnknown *pFilterUnk;
		pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);
		// ページを表示する。
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		pProp->Release();
		OleCreatePropertyFrame(
			hWnd,                   // 親ウィンドウ。
			0, 0,                   // 予約済み。
			FilterInfo.achName,     // ダイアログ ボックスのキャプション。
			1,                      // オブジェクト数 (フィルタのみ)。
			&pFilterUnk,            // オブジェクト ポインタの配列。
			caGUID.cElems,          // プロパティ ページ数。
			caGUID.pElems,          // プロパティ ページ CLSID の配列。
			0,                      // ロケール識別子。
			0, NULL                 // 予約済み。
		);
		// クリーン アップ。
		pFilterUnk->Release();
		FilterInfo.pGraph->Release(); 
		CoTaskMemFree(caGUID.pElems);
	}

	return hr;
}

//////////////////////////////////////////////////
//基準クロックの設定
//BOOL bUse	基準クロックを使用するか否か
//BOOL bDefaultSyncSource	bUse==TRUEのとき、デフォルトの基準クロックを使うかどうか？
//BOOL bMakeSystemClock	bUse==TRUE && bDefaultSyncSource==FALSE のとき、システム基準クロックを生成して使うかどうか
//IBaseFilter* pFilter	bUse==TRUE && bDefaultSyncSource==FALSE のとき、フィルタから基準クロックを取得して使うかどうか？
//////////////////////////////////////////////////
HRESULT CDirectShow::SetGraphClock(BOOL bUse,BOOL bDefaultSyncSource,BOOL bMakeSystemClock,IBaseFilter* pFilter)
{
	assert(m_pGraphBuilder);
	HRESULT hr = NO_ERROR;
	CComPtr<IMediaFilter> pMediaFilter;
	hr = m_pGraphBuilder->QueryInterface(IID_IMediaFilter,(void**)&pMediaFilter);
	if (!CHECK_RESULT("IGraphBuilder QueryInterface IMediaFilter.",hr)){return hr;}

	if (bUse)		//基準クロックを使用する(フィルタ同士が一つの基準クロックで統一される)
	{
		if (bDefaultSyncSource) {			//デフォルトの基準クロックを使用する(何も指定しない場合の規定の動作)
			hr = m_pGraphBuilder->SetDefaultSyncSource();
			if (m_pReferenceClock) {
				SAFE_RELEASE(m_pReferenceClock); }
			//デフォルトに決定された基準クロックを取得する
			hr = pMediaFilter->GetSyncSource(&m_pReferenceClock);
		} else {
			SAFE_RELEASE(m_pReferenceClock);
			if (bMakeSystemClock) {			//システムクロックを生成する(分解能1ms程度の低精度タイマーを使う)
				hr = CoCreateInstance(CLSID_SystemClock,
					NULL,CLSCTX_INPROC_SERVER,
					IID_IReferenceClock,(void**)&m_pReferenceClock); 
				if (FAILED(hr))
					return hr;
			} else {						//フィルタから基準クロックを指定する
				if (!pFilter)
					return E_FAIL;
				hr = pFilter->QueryInterface(IID_IReferenceClock,(void**)&m_pReferenceClock);
				if (FAILED(hr))
					return hr;
			}
			//指定の基準クロックを与える
			hr = pMediaFilter->SetSyncSource(m_pReferenceClock);
		}
	} else {
		//基準クロックを使用しない(各フィルタのクロック待ちが無くなるので最速の処理になる)
		hr = pMediaFilter->SetSyncSource(NULL);
		if (m_pReferenceClock) {
			SAFE_RELEASE(m_pReferenceClock);}
	}
	return hr;
}
//////////////////////////////////////////////////
//基準クロックから現在の基準タイムを取得する
//REFERENCE_TIME &rtTime	分解能(100ナノ秒単位)
//////////////////////////////////////////////////
HRESULT CDirectShow::GetGraphClockTime(REFERENCE_TIME &rtTime)
{
	if (!m_pReferenceClock)
		return E_FAIL;
	return m_pReferenceClock->GetTime(&rtTime);
}
//////////////////////////////////////////////////
//基準クロックの分解能を取得する
//REFERENCE_TIME &rtResolution	分解能(100ナノ秒単位)
//////////////////////////////////////////////////
HRESULT CDirectShow::GetGraphClockResolution(REFERENCE_TIME &rtResolution)
{
	if (!m_pReferenceClock)
		return E_FAIL;
	HRESULT hr = NOERROR;
	REFERENCE_TIME rtBeginTime;
	REFERENCE_TIME rtEndTime;
	hr = m_pReferenceClock->GetTime(&rtBeginTime);
	if (FAILED(hr))
		return E_FAIL;
	do {
		hr = m_pReferenceClock->GetTime(&rtEndTime);
	} while (SUCCEEDED(hr) && rtBeginTime == rtEndTime);
	rtResolution = rtEndTime - rtBeginTime;
	return hr;
}

//////////////////////////////////////////////////
//フィルタグラフに同期の可否を設定する。また、必要があれば最大遅延時間を設定する
//BOOL bUseStreamOffset	同期を有効にするかどうか
//REFERENCE_TIME* prtMaxGraphLatency 遅延時間を設定する(NULLの場合は設定は無し)
//////////////////////////////////////////////////
HRESULT CDirectShow::SyncStreamOffset(BOOL bUseStreamOffset,REFERENCE_TIME* prtMaxGraphLatency)
{
	assert(m_pGraphBuilder);
	if (!m_pGraphBuilder)
		return E_FAIL;
	HRESULT hr = NOERROR;
	CComPtr<IAMGraphStreams> pAMGraphStreams;
	hr = m_pGraphBuilder->QueryInterface(IID_IAMGraphStreams,(void**)&pAMGraphStreams);
	if (!CHECK_RESULT("IGraphBuilder QueryInterface IAMGraphStreams.",hr)){return hr;}
	hr = pAMGraphStreams->SyncUsingStreamOffset(bUseStreamOffset);
	if (!CHECK_RESULT("IAMGraphStreams SyncUsingStreamOffset.",hr)){return hr;}
	if (bUseStreamOffset && prtMaxGraphLatency)
	{
		hr = pAMGraphStreams->SetMaxGraphLatency(*prtMaxGraphLatency);
		if (!CHECK_RESULT("IAMGraphStreams SetMaxGraphLatency.",hr)){return hr;}
	}
	return hr;
}

//////////////////////////////////////////////////
//アロケーターバッファの設定
//BOOL bSet 値を設定する場合はTRUE, 値を取得する場合はFALSE
//オーディオピンの場合、デフォルトで500ミリ～1000ミリ秒の遅延
//通常は80ミリ秒のバッファが安全ではあるが、30～40ミリ秒のバッファでも十分な場合がある
//バッファが小さすぎると音質は劣化する(プチプチ)
//////////////////////////////////////////////////
HRESULT CDirectShow::SetGetAllocatorBuffer(IBaseFilter* pFilter,int nPinIndex,BOOL bSet,ALLOCATOR_PROPERTIES &AllocProp)
{
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;

	CComPtr<IPin> pPin;
	pPin.Attach(CDirectShowEtc::GetPinByIndex(pFilter,nPinIndex));
	if (!pPin.p) return E_FAIL;

	CComPtr<IAMBufferNegotiation> pAMBufferNegotation;
	hr = pPin->QueryInterface(IID_IAMBufferNegotiation,(void**)&pAMBufferNegotation);
	if (FAILED(hr)) {
		return E_FAIL;
	}
	if (bSet) {
		//アロケータの設定はピン接続前でしかできない
		hr = pAMBufferNegotation->SuggestAllocatorProperties(&AllocProp);
	} else {
		//アロケータのチェックはピン接続後でしかできない
		AllocProp.cBuffers  = -1;
		AllocProp.cbBuffer = -1;
		AllocProp.cbAlign = -1;
		AllocProp.cbPrefix = -1;
		hr = pAMBufferNegotation->GetAllocatorProperties(&AllocProp);
	}
	return hr;
}

//////////////////////////////////////////////////
//フィルタ開始以降のフレーム数・ドロップ数などを取得
//long* plNotDropped	ドロップされなかった総フレーム数
//long* plDropped		ドロップされた総フレーム数
//long* plAverageSize	フレームの平均サイズ(バイト単位)
//////////////////////////////////////////////////
HRESULT CDirectShow::GetDropFrame(IBaseFilter* pFilter,int nPinIndex,long* plNotDropped,long* plDropped,long* plAverageSize)
{
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;

	CComPtr<IPin> pPin;
	pPin.Attach(CDirectShowEtc::GetPinByIndex(pFilter,nPinIndex));
	if (!pPin.p) return E_FAIL;

	CComPtr<IAMDroppedFrames> pAMDroppedFrames;
	hr = pPin->QueryInterface(IID_IAMDroppedFrames,(void**)&pAMDroppedFrames);
	if (FAILED(hr))
		return E_FAIL;	//インタフェースが取得できない場合がほとんどである
	//if (!CHECK_RESULT("IPin QueryInterface IAMDroppedFrames.",hr)){return hr;}

	//ストリーミングの開始以降、フィルタが配信した総フレーム数を取得する
	if (plNotDropped) {
		hr = pAMDroppedFrames->GetNumNotDropped(plNotDropped);
		if (FAILED(hr))
			(*plNotDropped) = 0;
		//if (!CHECK_RESULT("IAMDroppedFrames GetNumNotDropped.",hr)){return hr;}
	}
	//ストリーミングの開始以降、フィルタがドロップさせた総フレーム数を取得する
	if (plDropped) {
		hr = pAMDroppedFrames->GetNumDropped(plDropped);
		if (FAILED(hr))
			(*plDropped) = 0;
		//if (!CHECK_RESULT("IAMDroppedFrames GetNumDropped.",hr)){return hr;}
	}
	//フィルタがキャプチャしたフレームの平均サイズを取得する。 
	if (plAverageSize) {
		hr = pAMDroppedFrames->GetAverageFrameSize(plAverageSize);
		if (FAILED(hr))
			(*plAverageSize) = 0;
		//if (!CHECK_RESULT("IAMDroppedFrames GetAverageFrameSize.",hr)){return hr;}
	}
	return hr;
}

//////////////////////////////////////////////////
//プッシュソースフィルタの性能と情報を得る
//REFERENCE_TIME *prtLatency	グラフの遅延を示す基準タイム得る
//ULONG* pFlags AM_PUSHSOURCE_FLAGS列挙型で定義される組み合わせ
//REFERENCE_TIME *prtOffset		現在のストリーム オフセットを示す基準タイム
//REFERENCE_TIME *prtMaxOffset	フィルタがサポートできる最大オフセットを示す基準タイム
//////////////////////////////////////////////////
HRESULT CDirectShow::GetPushSourceInfo(IBaseFilter* pFilter,int nPinIndex,REFERENCE_TIME *prtLatency,ULONG* pFlags,REFERENCE_TIME *prtOffset,REFERENCE_TIME *prtMaxOffset)
{
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;

	CComPtr<IPin> pPin;
	pPin.Attach(CDirectShowEtc::GetPinByIndex(pFilter,nPinIndex));
	if (!pPin.p) return E_FAIL;

	CComPtr<IAMPushSource> pAMPushSource;
	hr = pPin->QueryInterface(IID_IAMPushSource,(void**)&pAMPushSource);
	if (FAILED(hr))
		return E_FAIL;	//インタフェースが取得できない場合がある (例:AmaRec Audio Capture)
	//if (!CHECK_RESULT("IPin QueryInterface IAMPushSource.",hr)){return hr;}

	//このフィルタに関連付けられている、予期される遅延時間を取得する。 
	if (prtLatency) {
		hr = pAMPushSource->GetLatency(prtLatency);
		if (FAILED(hr))
			(*prtLatency) = 0;
		//if (!CHECK_RESULT("IAMPushSource GetLatency.",hr)){return hr;}
	}
	//フィルタの動作を表すフラグの組み合わせを取得する。
	if (pFlags) {
		hr = pAMPushSource->GetPushSourceFlags(pFlags);
		if (FAILED(hr))
			(*pFlags) = 0;
		//if (!CHECK_RESULT("IAMPushSource GetPushSourceFlags.",hr)){return hr;}
	}
	//タイム スタンプを生成するときにフィルタが使うオフセットを取得する。
	if (prtOffset) {
		hr = pAMPushSource->GetStreamOffset(prtOffset);
		if (FAILED(hr))
			(*prtOffset) = 0;
		//if (!CHECK_RESULT("IAMPushSource GetStreamOffset.",hr)){return hr;}
	}
	//フィルタがサポートできる最大ストリーム オフセットを取得する。
	if (prtMaxOffset) {
		hr = pAMPushSource->GetMaxStreamOffset(prtMaxOffset);
		if (FAILED(hr))
			(*prtMaxOffset) = 0;
		//if (!CHECK_RESULT("IAMPushSource GetMaxStreamOffset.",hr)){return hr;}
	}
	return hr;
}
//////////////////////////////////////////////////
//現在のストリーミング情報を取得
//AM_STREAM_INFO* pAsi	ストリーミング制御情報
//////////////////////////////////////////////////
HRESULT CDirectShow::GetStreamInfo(IBaseFilter* pFilter,int nPinIndex,AM_STREAM_INFO* pAsi)
{
	assert(pFilter);
	assert(nPinIndex >= 0);
	HRESULT hr = NOERROR;

	CComPtr<IPin> pPin;
	pPin.Attach(CDirectShowEtc::GetPinByIndex(pFilter,nPinIndex));
	if (!pPin.p) return E_FAIL;

	CComPtr<IAMStreamControl> pAMStreamControl;
	hr = pPin->QueryInterface(IID_IAMStreamControl,(void**)&pAMStreamControl);
	if (FAILED(hr))
		return E_FAIL;	//インタフェースが取得できない場合がある

	if (pAsi) {
		hr = pAMStreamControl->GetInfo(pAsi);
		return hr;
	}
	return E_FAIL;
}
//////////////////////////////////////////////////
//キャプチャフィルタの接続
//IBaseFilter* pFilter	追加元となるフィルタ
//BOOL bVideo			ビデオフィルタかどうか
//int* pCrossbar1ID		第1クロスバーのフィルタID
//int* pCrossbar2ID		第2クロスバーのフィルタID
//int* pTVAudioID		TVオーディオのフィルタID		
//int* pTVTunerID		TVチューナーのフィルタID
//////////////////////////////////////////////////
HRESULT CDirectShow::AddWDMFilter(IBaseFilter* pFilter,BOOL bVideo,int* pCrossbar1ID,int* pCrossbar2ID,int* pTVAudioID,int* pTVTunerID)
{
	assert(pFilter);
	assert(m_pGraphBuilder);
	assert(m_pCaptureGraphBuilder2);
	assert(pCrossbar1ID);
	assert(pCrossbar2ID);
	assert(pTVAudioID);
	assert(pTVTunerID);
	if (!pFilter) return E_FAIL;
	if (pCrossbar1ID) *pCrossbar1ID = -1;
	if (pCrossbar2ID) *pCrossbar2ID = -1;
	if (pTVAudioID) *pTVAudioID = -1;
	if (pTVTunerID) *pTVTunerID = -1;

	HRESULT hr = NOERROR;

	//一時接続処理用のNULLレンダラの作成
	CComPtr<IBaseFilter> pNullRenderer;
	int nNullRendererID = -1;
	GUID guidNullRenderer = {0xC1F400A4,0x3F08,0x11D3,{0x9F,0x0B,0x00,0x60,0x08,0x03,0x9E,0x37}};//CLSID_NullRenderer
	hr = CoCreateInstance(guidNullRenderer,
		NULL,CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,(void **)&pNullRenderer);
	if (SUCCEEDED(hr)) {
		hr = m_pGraphBuilder->AddFilter(pNullRenderer,_T("Null Renderer"));
		if (!CHECK_RESULT("IGraph Builder AddFilter.",hr)) {return hr;}
	}

	//フィルタの接続 (必要なフィルタがここで挿入される)
	hr = m_pCaptureGraphBuilder2->RenderStream(
		bVideo?(&PIN_CATEGORY_PREVIEW):(&PIN_CATEGORY_PREVIEW),/*&PIN_CATEGORY_CAPTURE,*/
		bVideo?(&MEDIATYPE_Video):(&MEDIATYPE_Audio),
		pFilter,
		NULL,
		pNullRenderer);

	//一時処理用のNULLレンダラの削除
	m_pGraphBuilder->RemoveFilter(pNullRenderer);

	//キャプチャピンしかない場合は後方に自動でSmartTeeが挿入されているので、削除する
	if (hr == VFW_S_NOPREVIEWPIN)
	{
		CComPtr<IBaseFilter> pInsertFilter;
		hr = m_pCaptureGraphBuilder2->FindInterface(
			&LOOK_DOWNSTREAM_ONLY,	//&LOOK_UPSTREAM_ONLY
			&MEDIATYPE_Video,		//出力ピンのメジャータイプ
			pFilter,
			IID_IBaseFilter,
			(void**)&pInsertFilter);
		if (SUCCEEDED(hr)) {
			hr = m_pGraphBuilder->RemoveFilter(pInsertFilter);
		}
	} else if (FAILED(hr)) {
		return hr;		//RenderStreamに失敗
	}

	//前方に自動追加されたフィルタ群を管理フィルタ下に置く

	//第1クロスバー
	CComPtr<IAMCrossbar> pAMCrossbar;
	hr = m_pCaptureGraphBuilder2->FindInterface(
		&LOOK_UPSTREAM_ONLY,NULL,pFilter,
		IID_IAMCrossbar,(void**)&pAMCrossbar);
	if (SUCCEEDED(hr)) {
		IBaseFilter* pCrossbarFilter = NULL;
		hr = pAMCrossbar->QueryInterface(IID_IBaseFilter,(void**)&pCrossbarFilter);
		if (SUCCEEDED(hr) && pCrossbarFilter) {
			if (pCrossbar1ID)
				hr = this->AddFilterList(pCrossbarFilter,pCrossbar1ID);

			//第2クロスバーを確認
			CComPtr<IAMCrossbar> pAMCrossbar2;
			hr = m_pCaptureGraphBuilder2->FindInterface(
				&LOOK_UPSTREAM_ONLY,NULL,pCrossbarFilter,
				IID_IAMCrossbar,(void**)&pAMCrossbar2);
			if (SUCCEEDED(hr))
			{
				IBaseFilter* pCrossbarFilter2 = NULL;
				hr = pAMCrossbar->QueryInterface(IID_IBaseFilter,(void**)&pCrossbarFilter2);
				if (SUCCEEDED(hr) && pCrossbarFilter2) {
					if (pCrossbar2ID)
						hr = this->AddFilterList(pCrossbarFilter2,pCrossbar2ID);
				}
			}

		}
	}

	//TVオーディオ
	CComPtr<IAMTVAudio> pAMTVAudio;
	hr = m_pCaptureGraphBuilder2->FindInterface(
		&LOOK_UPSTREAM_ONLY,NULL,pFilter,
		IID_IAMTVAudio,(void**)&pAMTVAudio);
	if (SUCCEEDED(hr)) {
		IBaseFilter* pTVAudioFilter = NULL;
		hr = pAMTVAudio->QueryInterface(IID_IBaseFilter,(void**)&pTVAudioFilter);
		if (SUCCEEDED(hr) && pTVAudioFilter) {
			if (pTVAudioID)
				hr = this->AddFilterList(pTVAudioFilter,pTVAudioID);
		}
	}

	//TVチューナー
	CComPtr<IAMTVTuner> pAMTVTuner;
	hr = m_pCaptureGraphBuilder2->FindInterface(
		&LOOK_UPSTREAM_ONLY,NULL,pFilter,
		IID_IAMTVTuner,(void**)&pAMTVTuner);
	if (SUCCEEDED(hr)) {
		IBaseFilter* pTVTunerFilter = NULL;
		hr = pAMTVTuner->QueryInterface(IID_IBaseFilter,(void**)&pTVTunerFilter);
		if (SUCCEEDED(hr) && pTVTunerFilter) {
			if (pTVTunerID)
				hr = this->AddFilterList(pTVTunerFilter,pTVTunerID);
		}
	}

	return S_OK;
}

