#include "StdAfx.h"
#include "DirectShowEtc.h"

static const GUID MEDIASUBTYPE_I420 = 
	{0x30323449,0x0000,0x0010,{0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};

/////////////////////////////////////////////////
//メディアタイプのフォーマットブロックを開放する
/////////////////////////////////////////////////
HRESULT CDirectShowEtc::ReleaseMediaType(AM_MEDIA_TYPE &amt)
{
    if (amt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)amt.pbFormat);
        amt.cbFormat = 0;
        amt.pbFormat = NULL;
    }
    if (amt.pUnk != NULL)
    {
		// pUnk は使用しない方がよいので不要だが、安全を期すため。
        amt.pUnk->Release();
        amt.pUnk = NULL;
    }
	ZeroMemory(&amt,sizeof(AM_MEDIA_TYPE));
	return S_OK;
}

/////////////////////////////////////////////////
//メディアタイプのフォーマットブロックを開放する
/////////////////////////////////////////////////
HRESULT CDirectShowEtc::ReleaseMediaTypeList(vector<AM_MEDIA_TYPE> &MediaTypeList)
{
	for (int i=0;i<(int)MediaTypeList.size();i++){
		ReleaseMediaType(MediaTypeList[i]);
	}
	MediaTypeList.clear();
	return S_OK;
}
//////////////////////////////////////////////////
//フィルタグラフをGraphEditファイル(.grf)に保存する
//////////////////////////////////////////////////
HRESULT CDirectShowEtc::SaveGraphFile(IGraphBuilder* pGraphBuilder,WCHAR *wszPath) 
{
#ifdef _DEBUG
	assert(pGraphBuilder);
	const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
	CComPtr<IStorage> pStorage;
	HRESULT hr;
	hr = StgCreateDocfile(
		wszPath,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, &pStorage);
	if (FAILED(hr)) {
		return hr;
	}
	CComPtr<IStream> pStream;
	hr = pStorage->CreateStream(
		wszStreamName,
		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStream);
	if (FAILED(hr))
	{
		return hr;
	}
	CComPtr<IPersistStream> pPersist;
	hr = pGraphBuilder->QueryInterface(IID_IPersistStream, (void**)&pPersist);
	hr = pPersist->Save(pStream, TRUE);
	if (SUCCEEDED(hr)) 
	{
		hr = pStorage->Commit(STGC_DEFAULT);
	}
	return hr;
#else
	return S_OK;
#endif
}

//////////////////////////////////////////////////
//フィルタグラフをオブジェクトテーブル(ROT)に登録する
//////////////////////////////////////////////////
HRESULT CDirectShowEtc::AddToRot(IGraphBuilder* pGraphBuilder,DWORD &dwRegister)
{
#ifdef _DEBUG
	assert(pGraphBuilder);
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }
    WCHAR wsz[256];
	_stprintf_s(wsz,256, L"FilterGraph %08x pid %08x", (DWORD_PTR)pGraphBuilder, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pGraphBuilder,
            pMoniker, &dwRegister);
        pMoniker->Release();
    }
    pROT->Release();
	//TODO m_bRegister = true;
    return hr;
#else
	return S_OK;
#endif
}

//////////////////////////////////////////////////
//フィルタグラフをオブジェクトテーブル(ROT)から削除する
//////////////////////////////////////////////////
void CDirectShowEtc::RemoveFromRot(DWORD &dwRegister)
{
	IRunningObjectTable *pROT;
	if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
		pROT->Revoke(dwRegister);
		pROT->Release();
	}
	dwRegister = 0x0;
}


//////////////////////////////////////////////////
//DirectShowの戻り値をチェックするサブモジュール
//////////////////////////////////////////////////
bool CDirectShowEtc::CheckResult(TCHAR* string,HRESULT hr,const char* pFile,int nLine)
{
	if (FAILED(hr)) {
		TCHAR szErr[MAX_ERROR_TEXT_LEN];
		AMGetErrorText(hr,szErr,MAX_ERROR_TEXT_LEN);
#ifdef	__AFXWIN_H__ 
		CString strErrorMessage;
		strErrorMessage.Format(_T("%s\r\nHRESULT=0x%08X\r\n%s"),string,hr,szErr);
		::AfxMessageBox(strErrorMessage,MB_OK|MB_ICONSTOP);
		AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);	//強制終了
#else
		//エラーメッセージを出して強制終了する
		CDirectShowEtc::OutputDebugWindow(_T("########## HRESULT ERROR DUMP START ##########\r\n"));
		CDirectShowEtc::OutputDebugWindow(_T("%s : HRESULT=%08X (%s) %s\r\n"),string,hr,(FAILED(hr)) ? TEXT("FAILED") : TEXT("SUCCEEDED"),szErr);
		CDirectShowEtc::OutputDebugWindow(_T("File : %s Line=%d\r\n"),pFile,nLine);
		CDirectShowEtc::OutputDebugWindow(_T("########## HRESULT ERROR DUMP END   ##########\r\n"));
		assert(FALSE == TRUE);
#endif
	}
	return SUCCEEDED(hr);	//成功かどうかを返す
}

//////////////////////////////////////////////////
//デバッグウィンドウに文字列を表示(printf代わり)
//////////////////////////////////////////////////
void CDirectShowEtc::OutputDebugWindow(LPCTSTR pString, ...)
{
	va_list arglist;
	va_start(arglist,pString);
	TCHAR ch_debug[1024];
	 _vstprintf_s(ch_debug,1024,pString,arglist);
	OutputDebugString(ch_debug);
	va_end(arglist);
}
//////////////////////////////////////////////////
//IPinインタフェースの取得
//IBaseFilter* pFilter ピンを取得するフィルタ(in)
//int nPinIndex ピン番号(in)
//戻り値 IPinインタフェース (後にリリースを必ず行うこと)
//////////////////////////////////////////////////
IPin* CDirectShowEtc::GetPinByIndex(IBaseFilter* pFilter,int nPinIndex)
{
	assert(pFilter);
	if (!pFilter) return NULL;
	assert(nPinIndex >= 0);
	if (nPinIndex < 0) return NULL;

	HRESULT hr = NOERROR;
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return NULL;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return NULL;}
	IPin* pPin = NULL;
	int nIndex = 0;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin)
			continue;
		if (nIndex == nPinIndex) {
			return pPin;
		}
		nIndex++;
		SAFE_RELEASE(pPin);
	}
	return NULL;
}
//////////////////////////////////////////////////
//IPinインタフェースを取得
//IBaseFilter* pFilter	ピンを取得するフィルタ(in)
//PIN_DIRECTION PinDir	ピンの方向(in)
//LPCWSTR pPinName		ピンの名前(in) (NULLも可。NULLの場合は名前を指定しない)
//AM_MEDIA_TYPE* pAmt	メディアタイプ(in) (NULLも可。NULLの場合はメディアタイプの指定がない)
//戻り値 IPinインタフェース (後にリリースを必ず行うこと)
//////////////////////////////////////////////////
IPin* CDirectShowEtc::GetPinByName(IBaseFilter* pFilter,PIN_DIRECTION PinDir,LPCWSTR pPinName,const AM_MEDIA_TYPE* pAmt)
{
	assert(pFilter);
	if (!pFilter) return NULL;
	HRESULT hr = NOERROR;
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return NULL;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return NULL;}
	IPin* pPin = NULL;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin)
			continue;
		//ピンの方向の指定のチェック
		PIN_DIRECTION GetPinDir;
		hr = pPin->QueryDirection(&GetPinDir);
		if (FAILED(hr) || GetPinDir != PinDir) {
			SAFE_RELEASE(pPin);
			continue;
		}
		//ピンの名前の指定のチェック
		if (pPinName)
		{
			PIN_INFO PinInfo;
			ZeroMemory(&PinInfo,sizeof(PIN_INFO));
			hr = pPin->QueryPinInfo(&PinInfo);
			if (FAILED(hr) || _tcsnccmp(pPinName,PinInfo.achName,_tcslen(pPinName)) != 0)
			{
				if (PinInfo.pFilter) {
					SAFE_RELEASE(PinInfo.pFilter);
				}
				SAFE_RELEASE(pPin);
				continue;
			}
			if (PinInfo.pFilter) {
				SAFE_RELEASE(PinInfo.pFilter);
			}
		}
		//メディアタイプの指定のチェック
		if (pAmt)
		{
			hr = pPin->QueryAccept(pAmt);
			if (hr != S_OK) {
				SAFE_RELEASE(pPin);
				continue;
			}
		}
		//このピンで決定
		return pPin;
	}
	return NULL;
}

//////////////////////////////////////////////////
//フィルタの接続チェック
//IBaseFilter* pFilter チェックを確認するフィルタ
//int* pInputCount	入力ピンについてチェックをするか？ NULLならチェックしない
//					接続された入力ピンの数が返る
//int* pOutputCount	出力ピンについてチェックをするか？ NULLならチェックしない
//					接続された出力ピンの数が返る
//////////////////////////////////////////////////
BOOL CDirectShowEtc::CheckConnectionFilter(IBaseFilter* pFilter,int* pInputCount,int* pOutputCount)
{
	assert(pFilter);
	if (pInputCount)  *pInputCount = 0;
	if (pOutputCount) *pOutputCount = 0;
	BOOL bRet = TRUE;
	HRESULT hr = NOERROR;
	ULONG nFetched = 0;
	CComPtr<IEnumPins> pEnumPins;
	hr = pFilter->EnumPins(&pEnumPins);
	if (!CHECK_RESULT("IBaseFilter EnumPins.",hr)) {return FALSE;}
	hr = pEnumPins->Reset();
	if (!CHECK_RESULT("IEnumPins Reset.",hr)) {return FALSE;}
	IPin* pPin = NULL;
	while (pEnumPins->Next(1,&pPin,&nFetched) == S_OK) {
		if (!pPin) continue;
		//ピンの方向を取得
		PIN_DIRECTION direction;
		hr = pPin->QueryDirection(&direction);
		if (!CHECK_RESULT("IPin QueryDirection.",hr)) {pPin->Release(); pPin = NULL; return FALSE;}
		if (direction ==  PINDIR_INPUT && pInputCount == NULL){
			pPin->Release(); pPin = NULL;
			continue;
		} else if (direction == PINDIR_OUTPUT && pOutputCount == NULL) {
			pPin->Release(); pPin = NULL;
			continue;
		}
		//ピンが接続されているかどうかチェック
		IPin* pConnectToPin = NULL;
		hr = pPin->ConnectedTo(&pConnectToPin);
		if (hr == VFW_E_NOT_CONNECTED) {	//接続されていない
		} else if (hr == S_OK) {	//接続されている
			if (direction == PINDIR_INPUT)
				(*pInputCount)++;
			else
				(*pOutputCount)++;
			pConnectToPin->Release();
			pConnectToPin = NULL;
		}  else {	//その他のエラー
			if (!CHECK_RESULT("IPin ConnectedTo.",hr)) {pPin->Release(); pPin = NULL; return FALSE;}
		}
		pPin->Release();
		pPin = NULL;
	}
	return TRUE;
}

//////////////////////////////////////////////////
//GUIDから文字列を生成する
//szGUID 生成されるGUID文字列(out)
//guid (in)
//////////////////////////////////////////////////
BOOL CDirectShowEtc::GUIDToString(TCHAR* szGUID,int nSize,const GUID &guid)
{
	ZeroMemory(szGUID,sizeof(TCHAR)*nSize);
	wsprintf(szGUID,_T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return TRUE;
}
//////////////////////////////////////////////////
//文字列からGUIDを生成する
//書式は {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
//TODO ここのGUID生成を作りこむ
//////////////////////////////////////////////////
BOOL CDirectShowEtc::StringToGUID(LPTSTR szGUID,GUID &guid)
{
	//guid = GUID_NULL;
	guid.Data1 =    (unsigned long) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,1,8);
	guid.Data2 =    (unsigned short)CDirectShowEtc::MakeGUIDSubRoutine(szGUID,10,4);
	guid.Data3 =    (unsigned short)CDirectShowEtc::MakeGUIDSubRoutine(szGUID,15,4);
	guid.Data4[0] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,20,2);
	guid.Data4[1] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,22,2);
	guid.Data4[2] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,25,2);
	guid.Data4[3] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,27,2);
	guid.Data4[4] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,29,2);
	guid.Data4[5] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,31,2);
	guid.Data4[6] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,33,2);
	guid.Data4[7] = (unsigned char) CDirectShowEtc::MakeGUIDSubRoutine(szGUID,35,2);
	return TRUE;
}
//////////////////////////////////////////////////
//GUID作成の為のサブルーチン(private)
//////////////////////////////////////////////////
unsigned long CDirectShowEtc::MakeGUIDSubRoutine(LPTSTR szGUID,int nStart,int nLength)
{
	LPTSTR pPtr = szGUID;
	unsigned long data = 0;
	TCHAR* pData = new TCHAR[nLength + 1];
	ZeroMemory(pData,sizeof(nLength + 1));

	for (int i=0;i<nStart;i++)
		pPtr = _tcsinc(pPtr);
	for (int i=0;i<nLength;i++)
	{
		pData[i] = (*pPtr);
		pPtr = _tcsinc(pPtr);
	}
	data =  (unsigned long)_tcstoul(pData,NULL,16);

	delete [] pData;

	return data;
}

//////////////////////////////////////////////////
//GUIDからCLSID文字列を取得
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeStringToMajorTypeGUID(TCHAR* szGUID,int nSize,GUID &guid)
{
	BOOL bRet = FALSE;
	ZeroMemory(szGUID,sizeof(TCHAR)*nSize);
	if (guid == MEDIATYPE_NULL) {
		wsprintf(szGUID,_T("MEDIATYPE_NULL")); bRet = TRUE; }
	else if (guid == MEDIATYPE_Video) {
		wsprintf(szGUID,_T("MEDIATYPE_Video")); bRet = TRUE; }
	else if (guid == MEDIATYPE_Audio) {
		wsprintf(szGUID,_T("MEDIATYPE_Audio")); bRet = TRUE; }
	else if (guid == MEDIATYPE_AnalogAudio) {
		wsprintf(szGUID,_T("MEDIATYPE_AnalogAudio")); bRet = TRUE; }
	else if (guid == MEDIATYPE_AnalogVideo) {
		wsprintf(szGUID,_T("MEDIATYPE_AnalogVideo")); bRet = TRUE; }
	else if (guid == MEDIATYPE_AUXLine21Data) {
		wsprintf(szGUID,_T("MEDIATYPE_AUXLine21Data")); bRet = TRUE; }
	else if (guid == MEDIATYPE_File) {
		wsprintf(szGUID,_T("MEDIATYPE_File")); bRet = TRUE; }
	else if (guid == MEDIATYPE_Interleaved) {
		wsprintf(szGUID,_T("MEDIATYPE_Interleaved")); bRet = TRUE; }
	else if (guid == MEDIATYPE_LMRT) {
		wsprintf(szGUID,_T("MEDIATYPE_LMRT")); bRet = TRUE; }
	else if (guid == MEDIATYPE_Midi) {
		wsprintf(szGUID,_T("MEDIATYPE_Midi")); bRet = TRUE; }
	else if (guid == MEDIATYPE_MPEG2_PES) {
		wsprintf(szGUID,_T("MEDIATYPE_MPEG2_PES")); bRet = TRUE; }
	//else if (guid == MEDIATYPE_MPEG2_SECTION) {
	//	wsprintf(szGUID,_T("MEDIATYPE_MPEG2_SECTION")); bRet = TRUE; }
	else if (guid == MEDIATYPE_Stream) {
		wsprintf(szGUID,_T("MEDIATYPE_Stream")); bRet = TRUE; }
	else if (guid == MEDIATYPE_Text) {
		wsprintf(szGUID,_T("MEDIATYPE_Text")); bRet = TRUE; }
	else if (guid == MEDIATYPE_Timecode) {
		wsprintf(szGUID,_T("MEDIATYPE_Timecode")); bRet = TRUE; }
	else if (guid == MEDIATYPE_URL_STREAM) {
		wsprintf(szGUID,_T("MEDIATYPE_URL_STREAM")); bRet = TRUE; }
	return bRet;
}
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeStringToSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid)
{
	BOOL bRet = FALSE;
	ZeroMemory(szGUID,sizeof(TCHAR)*nSize);
	return bRet;
}
//////////////////////////////////////////////////
//ビデオサブタイプの文字列変換
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeStringToVideoSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid)
{
	BOOL bRet = FALSE;
	ZeroMemory(szGUID,sizeof(TCHAR)*nSize);
	if (guid == MEDIASUBTYPE_RGB1) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB1")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB4) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB4")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB8) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB8")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB555) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB555")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB565) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB565")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB24) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB24")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB32) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB32")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_ARGB32) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_ARGB32")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_AYUV) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_AYUV")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_UYVY) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_UYVY")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_Y411) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_Y411")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_Y41P) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_Y41P")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_Y211) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_Y211")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_YUY2) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_YUY2")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_YVYU) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_YVYU")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_YUYV) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_YUYV")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_IF09) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_IF09")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_IYUV) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_IYUV")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_I420) {							
		wsprintf(szGUID,_T("MEDIASUBTYPE_I420")); bRet = TRUE; }	//IYUVの別名
	else if (guid == MEDIASUBTYPE_YV12) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_YV12")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_YVU9) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_YVU9")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_AI44) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_AI44")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_IA44) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_IA44")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_dvsl) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_dvsl")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_dvsd) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_dvsd")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_dvhd) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_dvhd")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_dv25) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_dv25")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_dv50) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_dv50")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_dvh1) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_dvh1")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_DVCS) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_DVCS")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB32_D3D_DX7_RT) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB32_D3D_DX7_RT")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RGB16_D3D_DX7_RT) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RGB16_D3D_DX7_RT")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_ARGB32_D3D_DX7_RT) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_ARGB32_D3D_DX7_RT")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_ARGB4444_D3D_DX7_RT) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_ARGB4444_D3D_DX7_RT")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_ARGB1555_D3D_DX7_RT) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_ARGB1555_D3D_DX7_RT")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_WMV1) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMV1")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_WMV2) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMV2")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_WMV3) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMV3")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_WMVA) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMVA")); bRet = TRUE; }
	//else if (guid == XXXXXXX) {
	//	wsprintf(szGUID,_T("XXXXX")); bRet = TRUE; }
	return bRet;
}
//////////////////////////////////////////////////
//オーディオサブタイプの文字列変換
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeStringToAudioSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid)
{
	BOOL bRet = FALSE;
	ZeroMemory(szGUID,sizeof(TCHAR)*nSize);
	if (guid == MEDIASUBTYPE_PCM) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_PCM")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG1Packet) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG1Packet")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG1Payload) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG1Payload")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG2_AUDIO) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG2_AUDIO")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_DVD_LPCM_AUDIO) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_DVD_LPCM_AUDIO")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG2_AUDIO) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG2_AUDIO")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_DRM_Audio) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_DRM_Audio")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_IEEE_FLOAT) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_IEEE_FLOAT")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_DOLBY_AC3) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_DOLBY_AC3")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_DOLBY_AC3_SPDIF) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_DOLBY_AC3_SPDIF")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_RAW_SPORT) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_RAW_SPORT")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_SPDIF_TAG_241h) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_SPDIF_TAG_241h")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_WMAudioV2) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMAudioV2")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_WMAudioV9) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMAudioV9")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_WMAudio_Lossless) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMAudio_Lossless")); bRet = TRUE; }
	//else if (guid == WMMEDIASUBTYPE_WMSP1) {
	//	wsprintf(szGUID,_T("WMMEDIASUBTYPE_WMSP1")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_MP3) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_MP3")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_MP43) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_MP43")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_M4S2) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_M4S2")); bRet = TRUE; }
	else if (guid == WMMEDIASUBTYPE_M4S2) {
		wsprintf(szGUID,_T("WMMEDIASUBTYPE_M4S2")); bRet = TRUE; }

	return bRet;
}
//////////////////////////////////////////////////
//ストリームサブタイプの文字列変換
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeStringToStreamSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid)
{
	BOOL bRet = FALSE;
	ZeroMemory(szGUID,sizeof(TCHAR)*nSize);
	if (guid == MEDIASUBTYPE_AIFF) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_AIFF")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_Asf) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_Asf")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_Avi) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_Avi")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_AU) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_AU")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_DssAudio) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_DssAudio")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_DssVideo) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_DssVideo")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG1Audio) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG1Audio")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG1System) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG1System")); bRet = TRUE; }
	//else if (guid == MEDIASUBTYPE_MPEG1SystemStream) {
	//	wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG1SystemStream")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG1Video) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG1Video")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_MPEG1VideoCD) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_MPEG1VideoCD")); bRet = TRUE; }
	else if (guid == MEDIASUBTYPE_WAVE) {
		wsprintf(szGUID,_T("MEDIASUBTYPE_WAVE")); bRet = TRUE; }
	return bRet;
}

//////////////////////////////////////////////////
//formatypeGUIDの文字列変換
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeStringToFormatTypeGUID(TCHAR* szGUID,int nSize,GUID &guid)
{
	BOOL bRet = FALSE;
	ZeroMemory(szGUID,sizeof(TCHAR)*nSize);
	if (guid == FORMAT_None) {
		wsprintf(szGUID,_T("FORMAT_None")); bRet = TRUE; }
	else if (guid == FORMAT_DvInfo) {
		wsprintf(szGUID,_T("FORMAT_DvInfo")); bRet = TRUE; }
	else if (guid == FORMAT_MPEGVideo) {
		wsprintf(szGUID,_T("FORMAT_MPEGVideo")); bRet = TRUE; }
	else if (guid == FORMAT_MPEG2Video) {
		wsprintf(szGUID,_T("FORMAT_MPEG2Video")); bRet = TRUE; }
	else if (guid == FORMAT_VideoInfo) {
		wsprintf(szGUID,_T("FORMAT_VideoInfo")); bRet = TRUE; }
	else if (guid == FORMAT_VideoInfo2) {
		wsprintf(szGUID,_T("FORMAT_VideoInfo2")); bRet = TRUE; }
	else if (guid == FORMAT_WaveFormatEx) {
		wsprintf(szGUID,_T("FORMAT_WaveFormatEx")); bRet = TRUE; }
	else if (guid == GUID_NULL) {
		wsprintf(szGUID,_T("GUID_NULL")); bRet = TRUE; }
	return bRet;
}

//////////////////////////////////////////////////
//メディアタイプののファイル出力
//////////////////////////////////////////////////
BOOL CDirectShowEtc::DumpMediaType(AM_MEDIA_TYPE* pamt,LPCTSTR lpszFileName)
{
	assert(lpszFileName);
	FILE* fp;
	_tfopen_s(&fp,lpszFileName,_T("w, ccs=UTF-8"));
	DumpMediaType(pamt,fp);
	fclose(fp);
	return TRUE;
}

//////////////////////////////////////////////////
//メディアタイプののファイル出力
//////////////////////////////////////////////////
BOOL CDirectShowEtc::DumpMediaType(AM_MEDIA_TYPE* pamt,FILE* fp)
{
	assert(fp);
	WCHAR* szBuffer = NULL;
	TCHAR* szGUID = new TCHAR[256];
	if (MakeStringToMajorTypeGUID(szGUID,256,pamt->majortype) == FALSE)
		GUIDToString(szGUID,256,pamt->majortype);
	_ftprintf_s(fp,_T("majortype,%s\n"),szGUID);

	if (pamt->majortype == MEDIATYPE_Video) {
		if (MakeStringToVideoSubTypeGUID(szGUID,256,pamt->subtype) == FALSE)
			GUIDToString(szGUID,256,pamt->subtype);
	} else if (pamt->majortype == MEDIATYPE_Audio) {
		if (MakeStringToAudioSubTypeGUID(szGUID,256,pamt->subtype) == FALSE)
			GUIDToString(szGUID,256,pamt->subtype);
	} else if (pamt->majortype == MEDIATYPE_Stream) {
		if (MakeStringToStreamSubTypeGUID(szGUID,256,pamt->subtype) == FALSE)
			GUIDToString(szGUID,256,pamt->subtype);
	} else {
		GUIDToString(szGUID,256,pamt->subtype);
	}
	_ftprintf_s(fp,_T("subtype,%s\n"),szGUID);
	_ftprintf_s(fp,_T("bFixedSizeSamples,%s\n"),pamt->bFixedSizeSamples?_T("TRUE"):_T("FALSE"));
	_ftprintf_s(fp,_T("bTemporalCompression,%s\n"),pamt->bTemporalCompression?_T("TRUE"):_T("FALSE"));
	_ftprintf_s(fp,_T("lSampleSize,%d\n"),pamt->lSampleSize);

	if (MakeStringToFormatTypeGUID(szGUID,256,pamt->formattype) == FALSE)
		GUIDToString(szGUID,256,pamt->formattype);
	_ftprintf_s(fp,_T("formattype,%s\n"),szGUID);
	_ftprintf_s(fp,_T("pUnk,%d\n"),(int)pamt->pUnk);
	_ftprintf_s(fp,_T("cbFormat,%d\n"),pamt->cbFormat);
	if (pamt->formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pamt->pbFormat;
		_ftprintf_s(fp,_T("rcSource,{%d-%d-%d-%d}\n"),vih->rcSource.left,vih->rcSource.top,vih->rcSource.right,vih->rcSource.bottom);
		_ftprintf_s(fp,_T("rcTarget,{%d-%d-%d-%d}\n"),vih->rcTarget.left,vih->rcTarget.top,vih->rcTarget.right,vih->rcTarget.bottom);
		_ftprintf_s(fp,_T("dwBitRate,%d\n"),vih->dwBitRate);
		_ftprintf_s(fp,_T("dwBitErrorRate,%d\n"),vih->dwBitErrorRate);
		_ftprintf_s(fp,_T("AvgTimePerFrame,%I64d\n"),vih->AvgTimePerFrame);
		_ftprintf_s(fp,_T("bmiHeader.biSize,%d\n"),vih->bmiHeader.biSize);
		_ftprintf_s(fp,_T("bmiHeader.biWidth,%d\n"),vih->bmiHeader.biWidth);
		_ftprintf_s(fp,_T("bmiHeader.biHeight,%d\n"),vih->bmiHeader.biHeight);
		_ftprintf_s(fp,_T("bmiHeader.biPlanes,%d\n"),vih->bmiHeader.biPlanes);
		_ftprintf_s(fp,_T("bmiHeader.biBitCount,%d\n"),vih->bmiHeader.biBitCount);
		_ftprintf_s(fp,_T("bmiHeader.biCompression,%d\n"),vih->bmiHeader.biCompression);
		_ftprintf_s(fp,_T("bmiHeader.biSizeImage,%d\n"),vih->bmiHeader.biSizeImage);
		_ftprintf_s(fp,_T("bmiHeader.biXPelsPerMeter,%d\n"),vih->bmiHeader.biXPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biYPelsPerMeter,%d\n"),vih->bmiHeader.biYPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biClrUsed,%d\n"),vih->bmiHeader.biClrUsed);
		_ftprintf_s(fp,_T("bmiHeader.biClrImportant,%d\n"),vih->bmiHeader.biClrImportant);
	} else if (pamt->formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)pamt->pbFormat;
		_ftprintf_s(fp,_T("rcSource,{%d-%d-%d-%d}\n"),vih2->rcSource.left,vih2->rcSource.top,vih2->rcSource.right,vih2->rcSource.bottom);
		_ftprintf_s(fp,_T("rcTarget,{%d-%d-%d-%d}\n"),vih2->rcTarget.left,vih2->rcTarget.top,vih2->rcTarget.right,vih2->rcTarget.bottom);
		_ftprintf_s(fp,_T("dwBitRate,%d\n"),vih2->dwBitRate);
		_ftprintf_s(fp,_T("dwBitErrorRate,%d\n"),vih2->dwBitErrorRate);
		_ftprintf_s(fp,_T("AvgTimePerFrame,%I64d\n"),vih2->AvgTimePerFrame);
		_ftprintf_s(fp,_T("dwInterlaceFlags,%d\n"),vih2->dwInterlaceFlags);
		_ftprintf_s(fp,_T("dwCopyProtectFlags,%d\n"),vih2->dwCopyProtectFlags);
		_ftprintf_s(fp,_T("dwPictAspectRatioX,%d\n"),vih2->dwPictAspectRatioX);
		_ftprintf_s(fp,_T("dwPictAspectRatioY,%d\n"),vih2->dwPictAspectRatioY);
		_ftprintf_s(fp,_T("dwControlFlags,%d\n"),vih2->dwControlFlags);
		_ftprintf_s(fp,_T("dwReserved2,%d\n"),vih2->dwReserved2);
		_ftprintf_s(fp,_T("bmiHeader.biSize,%d\n"),vih2->bmiHeader.biSize);
		_ftprintf_s(fp,_T("bmiHeader.biWidth,%d\n"),vih2->bmiHeader.biWidth);
		_ftprintf_s(fp,_T("bmiHeader.biHeight,%d\n"),vih2->bmiHeader.biHeight);
		_ftprintf_s(fp,_T("bmiHeader.biPlanes,%d\n"),vih2->bmiHeader.biPlanes);
		_ftprintf_s(fp,_T("bmiHeader.biBitCount,%d\n"),vih2->bmiHeader.biBitCount);
		_ftprintf_s(fp,_T("bmiHeader.biCompression,%d\n"),vih2->bmiHeader.biCompression);
		_ftprintf_s(fp,_T("bmiHeader.biSizeImage,%d\n"),vih2->bmiHeader.biSizeImage);
		_ftprintf_s(fp,_T("bmiHeader.biXPelsPerMeter,%d\n"),vih2->bmiHeader.biXPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biYPelsPerMeter,%d\n"),vih2->bmiHeader.biYPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biClrUsed,%d\n"),vih2->bmiHeader.biClrUsed);
		_ftprintf_s(fp,_T("bmiHeader.biClrImportant,%d\n"),vih2->bmiHeader.biClrImportant);
	} else if (pamt->formattype == FORMAT_WaveFormatEx) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)pamt->pbFormat;
		_ftprintf_s(fp,_T("wFormatTag,%d\n"),wfe->wFormatTag);
		_ftprintf_s(fp,_T("nChannels,%d\n"),wfe->nChannels);
		_ftprintf_s(fp,_T("nSamplesPerSec,%d\n"),wfe->nSamplesPerSec);
		_ftprintf_s(fp,_T("nAvgBytesPerSec,%d\n"),wfe->nAvgBytesPerSec);
		_ftprintf_s(fp,_T("nBlockAlign,%d\n"),wfe->nBlockAlign);
		_ftprintf_s(fp,_T("wBitsPerSample,%d\n"),wfe->wBitsPerSample);
		_ftprintf_s(fp,_T("nBlockAlign,%d\n"),wfe->nBlockAlign);
		_ftprintf_s(fp,_T("cbSize,%d\n"),wfe->cbSize);
	} else if (pamt->formattype == FORMAT_DvInfo) {
		DVINFO* dvi = (DVINFO*)pamt->pbFormat;
		_ftprintf_s(fp,_T("dwDVAAuxSrc,%d\n"),dvi->dwDVAAuxSrc);
		_ftprintf_s(fp,_T("dwDVAAuxCtl,%d\n"),dvi->dwDVAAuxCtl);
		_ftprintf_s(fp,_T("dwDVAAuxSrc1,%d\n"),dvi->dwDVAAuxSrc1);
		_ftprintf_s(fp,_T("dwDVAAuxCtl1,%d\n"),dvi->dwDVAAuxCtl1);
		_ftprintf_s(fp,_T("dwDVVAuxSrc,%d\n"),dvi->dwDVVAuxSrc);
		_ftprintf_s(fp,_T("dwDVVAuxCtl,%d\n"),dvi->dwDVVAuxCtl);
		_ftprintf_s(fp,_T("dwDVReserved[0],%d\n"),dvi->dwDVReserved[0]);
		_ftprintf_s(fp,_T("dwDVReserved[1],%d\n"),dvi->dwDVReserved[1]);
	} else if (pamt->formattype == FORMAT_MPEGVideo) {
	 	MPEG1VIDEOINFO* mp1vi = (MPEG1VIDEOINFO*)pamt->pbFormat;
		VIDEOINFOHEADER* vih = &(mp1vi->hdr);
		_ftprintf_s(fp,_T("rcSource,{%d-%d-%d-%d}\n"),vih->rcSource.left,vih->rcSource.top,vih->rcSource.right,vih->rcSource.bottom);
		_ftprintf_s(fp,_T("rcTarget,{%d-%d-%d-%d}\n"),vih->rcTarget.left,vih->rcTarget.top,vih->rcTarget.right,vih->rcTarget.bottom);
		_ftprintf_s(fp,_T("dwBitRate,%d\n"),vih->dwBitRate);
		_ftprintf_s(fp,_T("dwBitErrorRate,%d\n"),vih->dwBitErrorRate);
		_ftprintf_s(fp,_T("AvgTimePerFrame,%I64d\n"),vih->AvgTimePerFrame);
		_ftprintf_s(fp,_T("bmiHeader.biSize,%d\n"),vih->bmiHeader.biSize);
		_ftprintf_s(fp,_T("bmiHeader.biWidth,%d\n"),vih->bmiHeader.biWidth);
		_ftprintf_s(fp,_T("bmiHeader.biHeight,%d\n"),vih->bmiHeader.biHeight);
		_ftprintf_s(fp,_T("bmiHeader.biPlanes,%d\n"),vih->bmiHeader.biPlanes);
		_ftprintf_s(fp,_T("bmiHeader.biBitCount,%d\n"),vih->bmiHeader.biBitCount);
		_ftprintf_s(fp,_T("bmiHeader.biCompression,%d\n"),vih->bmiHeader.biCompression);
		_ftprintf_s(fp,_T("bmiHeader.biSizeImage,%d\n"),vih->bmiHeader.biSizeImage);
		_ftprintf_s(fp,_T("bmiHeader.biXPelsPerMeter,%d\n"),vih->bmiHeader.biXPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biYPelsPerMeter,%d\n"),vih->bmiHeader.biYPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biClrUsed,%d\n"),vih->bmiHeader.biClrUsed);
		_ftprintf_s(fp,_T("bmiHeader.biClrImportant,%d\n"),vih->bmiHeader.biClrImportant);

		_ftprintf_s(fp,_T("dwStartTimeCode,%d\n"),mp1vi->dwStartTimeCode);
		_ftprintf_s(fp,_T("cbSequenceHeader,%d\n"),mp1vi->cbSequenceHeader);
		//bSequenceHeader[1]がcbSequenceHeaderだけ続く
	} else if (pamt->formattype == FORMAT_VideoInfo2) {
		MPEG2VIDEOINFO* mp2vi = (MPEG2VIDEOINFO*)pamt->pbFormat;
		VIDEOINFOHEADER2* vih2 = &(mp2vi->hdr);
		_ftprintf_s(fp,_T("rcSource,{%d-%d-%d-%d}\n"),vih2->rcSource.left,vih2->rcSource.top,vih2->rcSource.right,vih2->rcSource.bottom);
		_ftprintf_s(fp,_T("rcTarget,{%d-%d-%d-%d}\n"),vih2->rcTarget.left,vih2->rcTarget.top,vih2->rcTarget.right,vih2->rcTarget.bottom);
		_ftprintf_s(fp,_T("dwBitRate,%d\n"),vih2->dwBitRate);
		_ftprintf_s(fp,_T("dwBitErrorRate,%d\n"),vih2->dwBitErrorRate);
		_ftprintf_s(fp,_T("AvgTimePerFrame,%I64d\n"),vih2->AvgTimePerFrame);
		_ftprintf_s(fp,_T("dwInterlaceFlags,%d\n"),vih2->dwInterlaceFlags);
		_ftprintf_s(fp,_T("dwCopyProtectFlags,%d\n"),vih2->dwCopyProtectFlags);
		_ftprintf_s(fp,_T("dwPictAspectRatioX,%d\n"),vih2->dwPictAspectRatioX);
		_ftprintf_s(fp,_T("dwPictAspectRatioY,%d\n"),vih2->dwPictAspectRatioY);
		_ftprintf_s(fp,_T("dwControlFlags,%d\n"),vih2->dwControlFlags);
		_ftprintf_s(fp,_T("dwReserved2,%d\n"),vih2->dwReserved2);
		_ftprintf_s(fp,_T("bmiHeader.biSize,%d\n"),vih2->bmiHeader.biSize);
		_ftprintf_s(fp,_T("bmiHeader.biWidth,%d\n"),vih2->bmiHeader.biWidth);
		_ftprintf_s(fp,_T("bmiHeader.biHeight,%d\n"),vih2->bmiHeader.biHeight);
		_ftprintf_s(fp,_T("bmiHeader.biPlanes,%d\n"),vih2->bmiHeader.biPlanes);
		_ftprintf_s(fp,_T("bmiHeader.biBitCount,%d\n"),vih2->bmiHeader.biBitCount);
		_ftprintf_s(fp,_T("bmiHeader.biCompression,%d\n"),vih2->bmiHeader.biCompression);
		_ftprintf_s(fp,_T("bmiHeader.biSizeImage,%d\n"),vih2->bmiHeader.biSizeImage);
		_ftprintf_s(fp,_T("bmiHeader.biXPelsPerMeter,%d\n"),vih2->bmiHeader.biXPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biYPelsPerMeter,%d\n"),vih2->bmiHeader.biYPelsPerMeter);
		_ftprintf_s(fp,_T("bmiHeader.biClrUsed,%d\n"),vih2->bmiHeader.biClrUsed);
		_ftprintf_s(fp,_T("bmiHeader.biClrImportant,%d\n"),vih2->bmiHeader.biClrImportant);

		_ftprintf_s(fp,_T("dwStartTimeCode,%d\n"),mp2vi->dwStartTimeCode);
		_ftprintf_s(fp,_T("cbSequenceHeader,%d\n"),mp2vi->cbSequenceHeader);
		_ftprintf_s(fp,_T("dwProfile,%d\n"),mp2vi->dwProfile);
		_ftprintf_s(fp,_T("dwLevel,%d\n"),mp2vi->dwLevel);
		_ftprintf_s(fp,_T("dwFlags,%d\n"),mp2vi->dwFlags);
		//bSequenceHeader[1]がcbSequenceHeaderだけ続く
	} else { //NULL
	}
	delete [] szGUID;
	return TRUE;
}

//////////////////////////////////////////////////
//ビデオストリーム設定の文字列のファイル出力
//////////////////////////////////////////////////
BOOL CDirectShowEtc::DumpVideoStreamConfigCaps(VIDEO_STREAM_CONFIG_CAPS* pvscc,LPCTSTR lpszFileName)
{
	assert(lpszFileName);
	FILE* fp;
	_tfopen_s(&fp,lpszFileName,_T("w, ccs=UTF-8"));
	DumpVideoStreamConfigCaps(pvscc,fp);
	fclose(fp);
	return TRUE;
}
//////////////////////////////////////////////////
//ビデオストリーム設定の文字列のファイル出力
//////////////////////////////////////////////////
BOOL CDirectShowEtc::DumpVideoStreamConfigCaps(VIDEO_STREAM_CONFIG_CAPS* pvscc,FILE* fp)
{
	assert(fp);
	TCHAR* szGUID = new TCHAR[256];
	if (MakeStringToFormatTypeGUID(szGUID,256,pvscc->guid) == FALSE)
		GUIDToString(szGUID,256,pvscc->guid);
	_ftprintf_s(fp,_T("guid,%s\n"),szGUID);
	_ftprintf_s(fp,_T("VideoStandard,%d\n"),pvscc->VideoStandard);
	_ftprintf_s(fp,_T("InputSize,{%d-%d}\n"),pvscc->InputSize.cx,pvscc->InputSize.cy);
	_ftprintf_s(fp,_T("MinCroppingSize,{%d-%d}\n"),pvscc->MinCroppingSize.cx,pvscc->MinCroppingSize.cy);
	_ftprintf_s(fp,_T("MaxCroppingSize,{%d-%d}\n"),pvscc->MaxCroppingSize.cx,pvscc->MaxCroppingSize.cy);
	_ftprintf_s(fp,_T("CropGranularityXY,{%d-%d}\n"),pvscc->CropGranularityX,pvscc->CropGranularityY);
	_ftprintf_s(fp,_T("CropAlignXY,{%d-%d}\n"),pvscc->CropAlignX,pvscc->CropAlignY);
	_ftprintf_s(fp,_T("MinOutputSize,{%d-%d}\n"),pvscc->MinOutputSize.cx,pvscc->MinOutputSize.cy);
	_ftprintf_s(fp,_T("MaxOutputSize,{%d-%d}\n"),pvscc->MaxOutputSize.cx,pvscc->MaxOutputSize.cy);
	_ftprintf_s(fp,_T("OutputGranularityXY,{%d-%d}\n"),pvscc->OutputGranularityX,pvscc->OutputGranularityY);
	_ftprintf_s(fp,_T("StretchTapsXY,{%d-%d}\n"),pvscc->StretchTapsX,pvscc->StretchTapsY);
	_ftprintf_s(fp,_T("ShrinkTapsXY,{%d-%d}\n"),pvscc->ShrinkTapsX,pvscc->ShrinkTapsY);
	_ftprintf_s(fp,_T("Min-MaxFrameInterval,{%I64d-%I64d}\n"),pvscc->MinFrameInterval,pvscc->MaxFrameInterval);
	double dMaxFPS = 0;
	double dMinFPS = 0;
	if (pvscc->MinFrameInterval != 0)
		dMaxFPS = 10000000.0 / (double)pvscc->MinFrameInterval;
	if (pvscc->MaxFrameInterval != 0)
		dMinFPS = 10000000.0 / (double)pvscc->MaxFrameInterval;
	_ftprintf_s(fp,_T("Min-MaxFPS,{%f-%f}\n"),dMinFPS,dMaxFPS);
	_ftprintf_s(fp,_T("Min-MaxBitsPerSecond,{%d-%d}\n"),pvscc->MinBitsPerSecond,pvscc->MaxBitsPerSecond);
	delete [] szGUID;
	return TRUE;
}
//////////////////////////////////////////////////
BOOL CDirectShowEtc::DumpAudioStreamConfigCaps(AUDIO_STREAM_CONFIG_CAPS* pascc,LPCTSTR lpszFileName)
{
	assert(lpszFileName);
	FILE* fp;
	_tfopen_s(&fp,lpszFileName,_T("w, ccs=UTF-8"));
	DumpAudioStreamConfigCaps(pascc,fp);
	fclose(fp);
	return TRUE;
}

//////////////////////////////////////////////////
BOOL CDirectShowEtc::DumpAudioStreamConfigCaps(AUDIO_STREAM_CONFIG_CAPS* pascc,FILE* fp)
{
	assert(fp);
	TCHAR* szGUID = new TCHAR[256];
	if (MakeStringToMajorTypeGUID(szGUID,256,pascc->guid) == FALSE)
		GUIDToString(szGUID,256,pascc->guid);
	_ftprintf_s(fp,_T("guid,%s\n"),szGUID);

	_ftprintf_s(fp,_T("MinimumChannels,%d\n"),pascc->MinimumChannels);
	_ftprintf_s(fp,_T("MaximumChannels,%d\n"),pascc->MaximumChannels);
	_ftprintf_s(fp,_T("ChannelsGranularity,%d\n"),pascc->ChannelsGranularity);
	_ftprintf_s(fp,_T("MinimumBitsPerSample,%d\n"),pascc->MinimumBitsPerSample);
	_ftprintf_s(fp,_T("MaximumBitsPerSample,%d\n"),pascc->MaximumBitsPerSample);
	_ftprintf_s(fp,_T("BitsPerSampleGranularity,%d\n"),pascc->BitsPerSampleGranularity);
	_ftprintf_s(fp,_T("MinimumSampleFrequency,%d\n"),pascc->MinimumSampleFrequency);
	_ftprintf_s(fp,_T("MaximumSampleFrequency,%d\n"),pascc->MaximumSampleFrequency);
	_ftprintf_s(fp,_T("SampleFrequencyGranularity,%d\n"),pascc->SampleFrequencyGranularity);
	delete [] szGUID;
	return TRUE;
}

//////////////////////////////////////////////////
//DirectShowフィルタ一覧を列挙する
//CLSID &guidCategory	検索するフィルタカテゴリ
//		CLSID_LegacyAmFilterCategory	DirectShowフィルタのカテゴリ(in)
//vector<FilterInfoStruct> &FilterList	取得されたフィルタのリスト(out)
//////////////////////////////////////////////////
HRESULT CDirectShowEtc::EnumAllFilter(const GUID &guidCategory,vector<FilterInfoStruct> &FilterList)
{
	HRESULT hr = NOERROR;
	FilterList.clear();
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pCreateDevEnum);
	if (!CHECK_RESULT("CoCreateInstance ICreateDevEnum.",hr)){return hr;}
	
	CComPtr<IEnumMoniker> pEnumMoniker;
	hr = pCreateDevEnum->CreateClassEnumerator(guidCategory, &pEnumMoniker,
				CDEF_DEVMON_FILTER /*| CDEF_DEVMON_PNP_DEVICE | CDEF_DEVMON_DMO | CDEF_DEVMON_CMGR_DEVICE*/);
	if (hr != S_OK) {
		CHECK_RESULT("ICreateDevEnum CreateClassEnumerator.",hr);
		return E_FAIL;
	}
	CComPtr<IMoniker> pMoniker;
	ULONG cFetched;
	while (pEnumMoniker->Next(1,&(pMoniker.p),&cFetched) == S_OK)
	{
		CComPtr<IPropertyBag> pPropertyBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,(void**)&pPropertyBag);
		if (SUCCEEDED(hr))
		{
			FilterInfoStruct fis;
			fis.guid = GUID_NULL;
			fis.strName = _T("");
			//フィルタのフレンドリ名を取得
			VARIANT varName,varCLSID;
			VariantInit(&varName);
			VariantInit(&varCLSID);
			hr = pPropertyBag->Read(L"FriendlyName",&varName,0);
			if (SUCCEEDED(hr))
			{
				fis.strName = varName.bstrVal;
			}
			VariantClear(&varName);

			hr = pPropertyBag->Read(L"CLSID",&varCLSID,0);
			if (SUCCEEDED(hr))
			{
				CDirectShowEtc::StringToGUID(varCLSID.bstrVal,fis.guid);
			}
			VariantClear(&varCLSID);

			if (fis.guid != GUID_NULL && !fis.strName.empty())
				FilterList.push_back(fis);
		}
		pMoniker.p->Release();
		pMoniker.p = NULL;
	}
	return hr;
}

//////////////////////////////////////////////////
//DMOフィルタの列挙
//const GUID &guidCategory GUIDのカテゴリ (in)
//				DMOCATEGORY_AUDIO_EFFECT
//vector<FilterInfoStruct> &FilterList	取得されたフィルタのリスト(out)
//////////////////////////////////////////////////
HRESULT CDirectShowEtc::EnumAllDMOFilter(const GUID &guidCategory,vector<FilterInfoStruct> &FilterList)
{
	HRESULT hr = NOERROR;
	FilterList.clear();
	CComPtr<IEnumDMO> pEnumDMO;
	hr = DMOEnum(guidCategory,
		DMO_ENUMF_INCLUDE_KEYED,
		0,NULL,0,NULL,&pEnumDMO);
	if (SUCCEEDED(hr))
	{
		CLSID clsidDMO;
		WCHAR* wszName;
		while (pEnumDMO->Next(1,&clsidDMO,&wszName,NULL) == S_OK)
		{
			FilterInfoStruct fis;
			fis.guid = clsidDMO;
			fis.strName = wszName;
			CoTaskMemFree(wszName);
			if (fis.guid != GUID_NULL && !fis.strName.empty())
				FilterList.push_back(fis);
		}
	}
	return hr;
}
//////////////////////////////////////////////////
//マッチングに合致したDirectShowフィルタを列挙する
//const GUID &guidMediaType		//MEDIATYPE_Video or MEDIATYPE_Audio (in)
//const GUID &guidMediaSubType	//GUID_NULL (in)
//vector<FilterInfoStruct> &FilterList	取得されたフィルタのリスト (out)
//////////////////////////////////////////////////
HRESULT CDirectShowEtc::EnumMatchFilter(const GUID &guidMediaType,const GUID &guidMediaSubType,vector<FilterInfoStruct> &FilterList)
{
	HRESULT hr = NOERROR;
	FilterList.clear();

	CComPtr<IFilterMapper2> pFilterMapper2;
	hr = CoCreateInstance(CLSID_FilterMapper2,
		NULL, CLSCTX_INPROC, IID_IFilterMapper2, (void**)&pFilterMapper2);
	if (!CHECK_RESULT("CoCreateInstance IFilterMapper2.",hr)){return hr;}

	CComPtr<IEnumMoniker> pEnumMoniker;

	GUID arrayInTypes[2];
	arrayInTypes[0] = guidMediaType;
	arrayInTypes[1] = guidMediaSubType;

	//http://msdn.microsoft.com/ja-jp/library/cc371157.aspx
	//http://msdn.microsoft.com/ja-jp/library/cc356772.aspx
	hr = pFilterMapper2->EnumMatchingFilters(
		&pEnumMoniker,
		0,                  // 予約済み。
		TRUE,               // 完全一致を使う?
		MERIT_DO_NOT_USE,	// 最小メリット。
		TRUE,               // 最低 1 つの入力ピン?
		1,                  // 入力用メジャー タイプ/サブタイプの対の数。
		arrayInTypes,       // 入力用メジャー タイプ/サブタイプの対の配列。
		NULL,               // 入力メディア。
		NULL,               // 入力ピン カテゴリ。
		FALSE,              // レンダラでなければならない?
		TRUE,               // 最低 1 つの出力ピン?
		1,                  // 出力用メジャー タイプ/サブタイプの対の数。
		arrayInTypes,       // 出力用メジャー タイプ/サブタイプの対の配列。
		NULL,               // 出力メディア。
		NULL);              // 出力ピン カテゴリ。

	CComPtr<IMoniker> pMoniker;
	ULONG cFetched;
	while (pEnumMoniker->Next(1,&(pMoniker.p),&cFetched) == S_OK)
	{
		CComPtr<IPropertyBag> pPropertyBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,(void**)&pPropertyBag);
		if (SUCCEEDED(hr))
		{
			FilterInfoStruct fis;
			fis.guid = GUID_NULL;
			fis.strName = _T("");
			//フィルタのフレンドリ名を取得
			VARIANT varName,varCLSID;
			VariantInit(&varName);
			VariantInit(&varCLSID);
			hr = pPropertyBag->Read(L"FriendlyName",&varName,0);
			if (SUCCEEDED(hr))
			{
				fis.strName = varName.bstrVal;
			}
			VariantClear(&varName);

			hr = pPropertyBag->Read(L"CLSID",&varCLSID,0);
			if (SUCCEEDED(hr))
			{
				CDirectShowEtc::StringToGUID(varCLSID.bstrVal,fis.guid);
			}
			VariantClear(&varCLSID);

			if (fis.guid != GUID_NULL && !fis.strName.empty())
				FilterList.push_back(fis);
		}
		pMoniker.Release();
	}

	return hr;
}


/////////////////////////////////////////////////
//インストールされているフィルタの検索
//GUID guid フィルタ名
//BOOL bFind みつかったかどうか
/////////////////////////////////////////////////
HRESULT CDirectShowEtc::FindInstallFilter(wstring strFilterName,BOOL &bFind)
{
	HRESULT hr = NOERROR;
	bFind = FALSE;

	// System Device Enumerator を作成する。
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
		return hr;

	// ビデオ コンプレッサ カテゴリのクラス列挙子を取得する。
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_LegacyAmFilterCategory, &pEnumCat, 0);
	if (hr == S_OK) 
	{
		// モニカを列挙する。
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// フィルタのフレンドリ名を取得するには、次の処理を行う。
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					if ( _tcscmp(varName.bstrVal,strFilterName.c_str()) == 0) {
						bFind = TRUE;
					}
				}
				VariantClear(&varName);
				pPropBag->Release();
			}
			pMoniker->Release();
			if (bFind)
				break;
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return hr;
}

//////////////////////////////////////////////////
//AM_MEDIA_TYPEからキャプチャ設定の文字列を生成するサブルーチン
//vector<AM_MEDIA_TYPE> &MediaTypeList	メディアタイプの配列(in)
//vector<wstring> &strVideoSettingList	生成された文字列(out)
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeVideoMediaString(const vector<AM_MEDIA_TYPE> &MediaTypeList,vector<wstring> &strVideoSettingList)
{
	strVideoSettingList.clear();
	int nWidth,nHeight;		//幅・高さ
	double dFPS;			//FPS
	wstring strColorSpace;	//色空間(文字列)
	TCHAR szSetting[256];	//設定名
	DWORD biCompression;	//圧縮
	WORD biBitCount;		//ビット数
	for (int i=0;i<(int)MediaTypeList.size();i++)
	{
		if (MediaTypeList[i].formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER* pVideoInfoHeader;
			pVideoInfoHeader = (VIDEOINFOHEADER *)MediaTypeList[i].pbFormat;
			nWidth = pVideoInfoHeader->bmiHeader.biWidth;
			nHeight = pVideoInfoHeader->bmiHeader.biHeight;
			if (pVideoInfoHeader->AvgTimePerFrame != 0)
				dFPS = (double)((double)10000000.0 / (double)pVideoInfoHeader->AvgTimePerFrame);
			else
				dFPS = 0.0;
			biCompression = pVideoInfoHeader->bmiHeader.biCompression;
			biBitCount = pVideoInfoHeader->bmiHeader.biBitCount;
		}
		else if (MediaTypeList[i].formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2* pVideoInfoHeader2;
			pVideoInfoHeader2 = (VIDEOINFOHEADER2*)MediaTypeList[i].pbFormat;
			nWidth = pVideoInfoHeader2->bmiHeader.biWidth;
			nHeight = pVideoInfoHeader2->bmiHeader.biHeight;
			if (pVideoInfoHeader2->AvgTimePerFrame != 0)
				dFPS = (double)((double)10000000.0 / (double)pVideoInfoHeader2->AvgTimePerFrame);
			else
				dFPS = 0.0;
			biCompression = pVideoInfoHeader2->bmiHeader.biCompression;
			biBitCount = pVideoInfoHeader2->bmiHeader.biBitCount;
		} else {
			nWidth = 0;
			nHeight = 0;
			biCompression = 0;
			biBitCount = 0;
		}
		strColorSpace.clear();

		if (biCompression == 0)	//非圧縮フォーマット
		{
			//strColorSpace = _T("RGB");
			wostringstream os;
			os << _T("RGB");
			os << biBitCount;
			strColorSpace = os.str();
		} else {				//圧縮フォーマット(FOURCC)
			wchar_t wchData[5];
			DWORD data1 = MediaTypeList[i].subtype.Data1;
			wchData[0] = (wchar_t)(data1 & 0xFF); 
			wchData[1] = (wchar_t)((data1 >> 8) & 0xFF);
			wchData[2] = (wchar_t)((data1 >> 16) & 0xFF);
			wchData[3] = (wchar_t)((data1 >> 24) & 0xFF);
			wchData[4] = _T('\0');
			strColorSpace = wchData;
		}
		
		//幅x高さ (色空間 fps)
		wstring strConvert = _T("");
		if ((( (int)(dFPS*100)) % 100) == 0) {
			strConvert = _T("%dx%d (%s %.0ffps)");
		} else if ((((int)(dFPS*10))  % 10) == 0 ) {
			strConvert = _T("%dx%d (%s %.1ffps)");
		} else {
			strConvert = _T("%dx%d (%s %.2ffps)");
		}
		_stprintf_s(szSetting,256,strConvert.c_str(),nWidth,nHeight,strColorSpace.c_str(),dFPS);
		strVideoSettingList.push_back(szSetting);
	}
	return TRUE;
}
//////////////////////////////////////////////////
//AM_MEDIA_TYPEからオーディオキャプチャ設定の文字列を生成するサブルーチン
//vector<AM_MEDIA_TYPE> &MediaTypeList	メディアタイプの配列(in)
//vector<wstring> &strAudioSettingList	生成された文字列(out)
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeAudioMediaString(const vector<AM_MEDIA_TYPE> &MediaTypeList,vector<wstring> &strAudioSettingList)
{
	strAudioSettingList.clear();
	TCHAR szSetting[256];	//設定名
	DWORD nSamplesPerSec;	//周波数
	WORD wBitsPerSample;	//分解能
	WORD nChennels;			//チャネル数
	wstring strCh;
	for (int i=0;i<(int)MediaTypeList.size();i++)
	{
		if (MediaTypeList[i].formattype == FORMAT_WaveFormatEx)
		{
			WAVEFORMATEX* pWaveFormatEx;
			pWaveFormatEx = (WAVEFORMATEX *)MediaTypeList[i].pbFormat;
			nSamplesPerSec = pWaveFormatEx->nSamplesPerSec;
			wBitsPerSample = pWaveFormatEx->wBitsPerSample;
			nChennels = pWaveFormatEx->nChannels;
		} else {
			nSamplesPerSec = 0;
			wBitsPerSample = 0;
			nChennels = 0;
		}
		if (nChennels == 6){
			strCh = _T("5.1");
		} else if (nChennels == 8){
			strCh = _T("7.1");
		} else {
			wostringstream os;
			os << nChennels;
			strCh = os.str();
		}
		//周波数Hz 分解能bit チャンネル数ch
		_stprintf_s(szSetting,256,
			_T("%dHz %dbit %sch"),nSamplesPerSec,wBitsPerSample,strCh.c_str());
		strAudioSettingList.push_back(szSetting);
	}
	return TRUE;
}

//////////////////////////////////////////////////
//AM_MEDIA_TYPEからダミーのAUDIO_STREAM_CONFIG_CAPSリストを作るサブルーチン
//vector<AM_MEDIA_TYPE> &MediaTypeList	メディアタイプの配列(in)
//vector<AUDIO_STREAM_CONFIG_CAPS> &AudioStreamConfigCapsList	生成されたダミーのオーディオキャプチャデバイスの性能リスト(out)
//////////////////////////////////////////////////
BOOL CDirectShowEtc::MakeDummyAudioCapsList(const vector<AM_MEDIA_TYPE> &MediaTypeList,vector<AUDIO_STREAM_CONFIG_CAPS> &AudioStreamConfigCapsList)
{
	AudioStreamConfigCapsList.clear();
	for (int i=0;i<(int)MediaTypeList.size();i++) {
		AUDIO_STREAM_CONFIG_CAPS ascc;
		ZeroMemory(&ascc,sizeof(AUDIO_STREAM_CONFIG_CAPS));
		ascc.guid = MEDIATYPE_Audio;
		if (MediaTypeList[i].formattype == FORMAT_WaveFormatEx)
		{
			WAVEFORMATEX* pWaveFormatEx;
			pWaveFormatEx = (WAVEFORMATEX *)MediaTypeList[i].pbFormat;
			ascc.MinimumChannels = pWaveFormatEx->nChannels;
			ascc.MaximumChannels = pWaveFormatEx->nChannels;
			ascc.ChannelsGranularity = 0;
			ascc.MinimumBitsPerSample = pWaveFormatEx->wBitsPerSample;
			ascc.MaximumBitsPerSample = pWaveFormatEx->wBitsPerSample;
			ascc.BitsPerSampleGranularity = 0;
			ascc.MinimumSampleFrequency = pWaveFormatEx->nSamplesPerSec;
			ascc.MaximumSampleFrequency = pWaveFormatEx->nSamplesPerSec;
			ascc.SampleFrequencyGranularity = 0;
		}
		AudioStreamConfigCapsList.push_back(ascc);
	}
	return TRUE;
}


/*
//////////////////////////////////////////////////
//ffdshowとスマートティーを接続するサブルーチン
//////////////////////////////////////////////////
HRESULT CDirectShowAccess::ConnectFFDshowToSmartTee()
{
	assert(m_pDirectShow);
	assert(m_nVideoFFDShowID != -1);
	HRESULT hr = NOERROR;

	//出力色空間名の設定
	DWORD dwFFDShowOutBitCount = 24;				//ffdshowの出力色数
	DWORD dwFFDShowOutCompression = 0;				//ffdshowの色圧縮名
	GUID guidFFDShowOutsubtype = MEDIASUBTYPE_RGB24;//ffdshowのサブタイプ名

	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	if (!GetColorSpace(pProfile->m_strffdshowColorSpace,
		guidFFDShowOutsubtype,dwFFDShowOutBitCount,dwFFDShowOutCompression))
		return E_FAIL;

	//ffdshowの出力ピンインデックスを取得
	int nFFDOutputPinIndex = -1;
	hr = m_pDirectShow->GetOutputPinIndex(m_pDirectShow->GetFilterByID(m_nVideoFFDShowID),nFFDOutputPinIndex);
	if (nFFDOutputPinIndex == -1)	//出力ピンが無い
		return E_FAIL;
	//ffdshowのメディアタイプの取得
	vector<AM_MEDIA_TYPE> FFDShowMediaType;
	hr = m_pDirectShow->EnumMediaType(m_pDirectShow->GetFilterByID(m_nVideoFFDShowID),
		nFFDOutputPinIndex,FFDShowMediaType);
	//メディアタイプの検索
	int nFFDShowMediaTypeIndex = -1;
	for (int i=0;i<(int)FFDShowMediaType.size();i++) {
		if (FFDShowMediaType[i].formattype != FORMAT_VideoInfo)
			continue;
		if (FFDShowMediaType[i].subtype != guidFFDShowOutsubtype)
			continue;
		VIDEOINFOHEADER* pVideoInfoHeader = (VIDEOINFOHEADER *)FFDShowMediaType[i].pbFormat;
		if (pVideoInfoHeader->bmiHeader.biCompression != dwFFDShowOutCompression)
			continue;
		if (pVideoInfoHeader->bmiHeader.biBitCount != dwFFDShowOutBitCount)
			continue;
		nFFDShowMediaTypeIndex = i;
		break;
	}
	if (nFFDShowMediaTypeIndex == -1) {	//メディアタイプ検索失敗
		CDirectShowEtc::ReleaseMediaTypeList(FFDShowMediaType);
		return E_FAIL;
	}
	//フィルタの接続(ffdshowフィルタ->ビデオ分岐フィルタ)
	hr = m_pDirectShow->ConnectFilterDirect(
		m_pDirectShow->GetFilterByID(m_nVideoFFDShowID),
		m_pDirectShow->GetFilterByID(m_nVideoSmartTeeID),
		&FFDShowMediaType[nFFDShowMediaTypeIndex],NULL,NULL);
	CDirectShowEtc::ReleaseMediaTypeList(FFDShowMediaType);		//メディアタイプのリリースを忘れないこと
	return hr;
}
//////////////////////////////////////////////////
//色空間名(文字列)から色数と圧縮を決定するサブルーチン
//////////////////////////////////////////////////
BOOL CDirectShowAccess::GetColorSpace(CString &strColorSpace,GUID &subtype,DWORD &dwBitCount,DWORD &dwCompresion)
{
	if (strColorSpace.Compare(_T("RGB32")) == 0) {
		subtype = MEDIASUBTYPE_RGB32;
		dwBitCount = 32;
		dwCompresion = 0x00000000;
	} else if (strColorSpace.Compare(_T("RGB24")) == 0) {
		subtype = MEDIASUBTYPE_RGB24;
		dwBitCount = 24;
		dwCompresion = 0x00000000;
	} else if (strColorSpace.Compare(_T("RGB16")) == 0) {
		subtype = MEDIASUBTYPE_RGB555;
		dwBitCount = 16;
		dwCompresion = 0x00000000;
	} else if (strColorSpace.Compare(_T("RGB555")) == 0) {
		subtype = MEDIASUBTYPE_RGB555;
		dwBitCount = 16;
		dwCompresion = 0x00000000;
	} else if (strColorSpace.Compare(_T("RGB565")) == 0) {
		subtype = MEDIASUBTYPE_RGB565;
		dwBitCount = 16;
		dwCompresion = 0x00000000;
	} else if (strColorSpace.Compare(_T("IYUV")) == 0 || strColorSpace.Compare(_T("I420")) == 0) {
		subtype = MEDIASUBTYPE_IYUV;
		dwBitCount = 12;
		dwCompresion = 0x56555949;	//IYUV(420)
	} else if (strColorSpace.Compare(_T("YV12")) == 0) {
		subtype = MEDIASUBTYPE_YV12;
		dwBitCount = 12;
		dwCompresion = 0x32315659;	//YV12
	} else if (strColorSpace.Compare(_T("YUY2")) == 0) {
		subtype = MEDIASUBTYPE_YUY2;
		dwBitCount = 16;
		dwCompresion = 0x32595559;	//YUY2
	} else if (strColorSpace.Compare(_T("UYVY")) == 0) {
		subtype = MEDIASUBTYPE_UYVY;
		dwBitCount = 16;
		dwCompresion = 0x59565955;	//UYVY
	} else if (strColorSpace.Compare(_T("YVYU")) == 0) {
		subtype = MEDIASUBTYPE_YVYU;
		dwBitCount = 16;
		dwCompresion = 0x55595659;	//YVYU
	} else if (strColorSpace.Compare(_T("NV12")) == 0) {
		subtype = MEDIASUBTYPE_NV12;
		dwBitCount = 12;
		dwCompresion = 0x3231564e;	//NV12
	}
	return TRUE;
}
*/