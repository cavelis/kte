#include "StdAfx.h"
#include "DirectShowAudioVolume.h"

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CDirectShowAudioVolume::CDirectShowAudioVolume(void)
	:   m_pFilter(NULL),
		m_pSampleGrabber(NULL),
		m_pAudioVolumeCallback(NULL),
		m_hWnd(NULL),
		m_pddB(NULL),
		m_nChannels(0),
		m_bEnable(FALSE)
{
	ZeroMemory(&m_ConnectionMediaType,sizeof(AM_MEDIA_TYPE));
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CDirectShowAudioVolume::~CDirectShowAudioVolume(void)
{
	assert(!m_pFilter);
	assert(!m_pSampleGrabber);
	assert(!m_pAudioVolumeCallback);
	CDirectShowEtc::ReleaseMediaType(m_ConnectionMediaType);
}

//////////////////////////////////////////////////
//サンプルグラバを作成する
//////////////////////////////////////////////////
HRESULT CDirectShowAudioVolume::Create(IGraphBuilder* pGraphBuilder)
{
	assert(pGraphBuilder);
	if (!pGraphBuilder) return E_POINTER;
	if (m_pFilter)
		Delete(pGraphBuilder);

	//フィルタの生成
	HRESULT hr = NOERROR;
	hr = CoCreateInstance(CLSID_SampleGrabber,
		NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void **)&m_pFilter);
	if (FAILED(hr)) return hr;
	hr = pGraphBuilder->AddFilter(m_pFilter,_T("Audio Volume SampleGrabber"));
	if (FAILED(hr)) return hr;

	//FilterからISampleGrabberインターフェースを取得
	if (m_pSampleGrabber) {
		SAFE_RELEASE(m_pSampleGrabber);}

	hr = m_pFilter->QueryInterface(IID_ISampleGrabber,(void **)&m_pSampleGrabber);
	if (FAILED(hr)) return hr;

	//SampleGrabberの接続メディアタイプを設定
	AM_MEDIA_TYPE amt;
	ZeroMemory(&amt, sizeof(amt));
	amt.majortype = MEDIATYPE_Audio;
	amt.subtype = MEDIASUBTYPE_PCM;
	amt.formattype = FORMAT_WaveFormatEx;
	hr = m_pSampleGrabber->SetMediaType(&amt);

	//フィルタ内を通るサンプルをバッファにコピーするかどうかを指定する
	hr = m_pSampleGrabber->SetBufferSamples(FALSE);
	//上記関数がサンプルのON/OFFを切り替える

	//ワンショットモードをOFFにする
	hr = m_pSampleGrabber->SetOneShot(FALSE);

	//コールバックインタフェースの設定
	m_pAudioVolumeCallback = new CDirectShowAudioVolume::CDirectShowAudioVolumeCB(this);
	hr = m_pSampleGrabber->SetCallback(m_pAudioVolumeCallback,0);
	//0ならSampleCBタイプで1ならBufferCBタイプ

	//接続メディアタイプの消去
	CDirectShowEtc::ReleaseMediaType(m_ConnectionMediaType);

	//音量の初期化
	m_nChannels = 0;
	if (m_pddB) {
		delete [] m_pddB;
		m_pddB = NULL;
	}

	m_hWnd = NULL;
	return hr;
}

//////////////////////////////////////////////////
//サンプルグラバを削除する
//////////////////////////////////////////////////
void CDirectShowAudioVolume::Delete(IGraphBuilder* pGraphBuilder)
{
	HRESULT hr = NOERROR;
	assert(pGraphBuilder);
	if (!pGraphBuilder)
		return;

	//インタフェースの解放
	if (m_pSampleGrabber) {
		SAFE_RELEASE(m_pSampleGrabber);
	}
	//フィルタグラフから削除
	if (m_pFilter) {
		hr = pGraphBuilder->RemoveFilter(m_pFilter);
		SAFE_RELEASE(m_pFilter);
	}
	//コールバックの削除は最後に
	if (m_pAudioVolumeCallback){
		delete m_pAudioVolumeCallback;
		m_pAudioVolumeCallback = NULL;
	}

	//音量情報の削除
	m_nChannels = 0;
	if (m_pddB) {
		delete [] m_pddB;
		m_pddB = NULL;
	}

	m_bEnable = FALSE;

	return;
}
//////////////////////////////////////////////////
//接続メディアタイプの取得(接続後に呼び出す)
//////////////////////////////////////////////////
HRESULT CDirectShowAudioVolume::GetConnectionMediaType()
{
	//接続メディアタイプの消去
	CDirectShowEtc::ReleaseMediaType(m_ConnectionMediaType);
	//接続メディアタイプの取得
	HRESULT hr = NOERROR;
	hr = m_pSampleGrabber->GetConnectedMediaType(&m_ConnectionMediaType);
	return hr;
}
//////////////////////////////////////////////////
//接続メディアタイプの変更通知 (private)
//////////////////////////////////////////////////
HRESULT CDirectShowAudioVolume::SetMediaType(AM_MEDIA_TYPE* pamt)
{
	CDirectShowEtc::ReleaseMediaType(m_ConnectionMediaType);
	m_ConnectionMediaType = (*pamt);
	return S_OK;
}
//////////////////////////////////////////////////
//接続メディアタイプの取得 (private)
//////////////////////////////////////////////////
HRESULT CDirectShowAudioVolume::GetMediaType(AM_MEDIA_TYPE** ppamt)
{
	(*ppamt) = &m_ConnectionMediaType;
	return S_OK;
}

//////////////////////////////////////////////////
//デシベルデータを登録し、親ウィンドウに通知する(WM_SENDDBDATAを送るだけ)
//////////////////////////////////////////////////
HRESULT CDirectShowAudioVolume::SenddBData(int nChannels,double* pddB)
{
	if (m_nChannels != nChannels) {
		assert(nChannels != 0);
		if (m_pddB) {
			delete [] m_pddB;
		}
		m_pddB = new double[nChannels];
		m_nChannels = nChannels;
	}
	for (int i=0;i<m_nChannels;i++) {
		m_pddB[i] = pddB[i];
	}

	if (!m_hWnd)
		return E_FAIL;
	PostMessage(m_hWnd,WM_SENDDBDATA,0,0);
	return S_OK;
}

//////////////////////////////////////////////////
//サンプルグラバを通過した際に呼ばれるコールバック
//////////////////////////////////////////////////
STDMETHODIMP CDirectShowAudioVolume::CDirectShowAudioVolumeCB::SampleCB(double Time, IMediaSample *pSample)
{
	HRESULT hr = NOERROR;

	//メディアタイプの変更通知の有無の確認
	AM_MEDIA_TYPE* pamt = NULL;
	hr = pSample->GetMediaType(&pamt);
	if (hr == S_OK) {
		//メディアタイプが変更されているので変更通知
		m_pParentClass->SetMediaType(pamt);
	}

	//キャプチャが無効だった場合の処理
	if (m_pParentClass->IsCaptureEnabled() == FALSE)
		return S_OK;

	//メディアタイプを取得
	hr = m_pParentClass->GetMediaType(&pamt);
	if (pamt == NULL)
		return S_OK;

	//メディアタイプから各種必要な情報を取り出す
	WAVEFORMATEX* pWaveFormatEx = NULL;
	if (pamt->formattype == FORMAT_WaveFormatEx) {
		pWaveFormatEx = (WAVEFORMATEX*)pamt->pbFormat;
	}
	if (!pWaveFormatEx)
		return S_OK;

	//サンプルの取得
	long lSampleSize = pSample->GetSize();	//サンプルのサイズ
	BYTE* pData = NULL;						//サンプルのデータ
	hr = pSample->GetPointer(&pData);
	if (FAILED(hr) || pData == NULL)
		return S_OK;

	long* plMin = new long[pWaveFormatEx->nChannels];		//最大音量
	long* plMax = new long[pWaveFormatEx->nChannels];		//最小音量
	double* pddB = new double[pWaveFormatEx->nChannels];	//デシベル変換値
	for (int i=0;i<pWaveFormatEx->nChannels;i++) {
		plMin[i] = LONG_MAX;
		plMax[i] = LONG_MIN;
		pddB[i] = -90,0;
	}
	BYTE* pNowPtr = pData;
	BYTE* pEndPtr = pData + lSampleSize;
	int nData;
	while (pNowPtr < pEndPtr)
	{
		for (int i=0;i<pWaveFormatEx->nChannels;i++){
			//bitごとに無音の値が異なるので注意
			switch (pWaveFormatEx->wBitsPerSample)
			{
			case 8:
				//8bitならば符号無し(0～255,無音は 128)
				nData = ((long) *((BYTE*)pNowPtr)) - 128;
				pNowPtr++;
				break;
			case 16:
				//16bitならば符号付き(-32768 ～ +32767, 無音は 0)
				nData = (long) *((signed short*)pNowPtr);
				pNowPtr++;
				pNowPtr++;
				break;
			case 24:
				//24bitの場合はリトルエイディアンで格納されている
				//無音は0なのかどうなのか分からないが、無音は8388608と仮定してみる
				nData = (long) (*((BYTE*)pNowPtr));
				pNowPtr++;
				nData |= (long) ((*((BYTE*)pNowPtr)) << 8);
				pNowPtr++;
				nData |= (long) ((*((BYTE*)pNowPtr)) << 16);
				pNowPtr++;
				nData -= 8388608;
			default:
				nData = 0;
				break;
			}
			plMin[i] = min(plMin[i],nData);
			plMax[i] = max(plMax[i],nData);
		}
	}

	//音量をデシベルに変換
	for (int i=0;i<pWaveFormatEx->nChannels;i++)
	{
		nData = max(labs(plMin[i]),labs(plMax[i]));
		switch (pWaveFormatEx->wBitsPerSample)
		{
		case 8:
			pddB[i] = 20 * log10((double)nData / (double)(SCHAR_MAX+1));
			break;
		case 16:
			pddB[i] = 20 * log10((double)nData / (double)(SHRT_MAX+1));
			break;
		case 24:
			pddB[i] = 20 * log10((double)nData / (double)(8388608));	//2^24/2 = 8388608
			break;
		}
	}

	//親クラスにデシベルのデータを送る
	m_pParentClass->SenddBData(pWaveFormatEx->nChannels,pddB);

	delete [] pddB;
	delete [] plMin;
	delete [] plMax;

	//ポインタの削除は不要
	//delete [] pData;

	return S_OK;
}

