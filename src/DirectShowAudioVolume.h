#pragma once

#include "DirectShow.h"



//通知用のWMメッセージ
#define WM_SENDDBDATA WM_APP + 108


//////////////////////////////////////////////////
//DirectShow WM ASF Writerに関するクラス
//////////////////////////////////////////////////
class CDirectShowAudioVolume
{
public:
	//コンストラクタ
	CDirectShowAudioVolume(void);
	//デストラクタ
	virtual ~CDirectShowAudioVolume(void);

	//サンプルグラバを作成する
	HRESULT Create(IGraphBuilder* pGraphBuilder);
	//サンプルグラバを削除する
	void Delete(IGraphBuilder* pGraphBuilder);
	//サンプルグラバフィルタの取得
	inline IBaseFilter* GetFilter(){assert(m_pFilter);return m_pFilter;}

	//通知用のウィンドウハンドルを設定
	void SetHWnd(HWND hWnd){m_hWnd = hWnd;}

	//キャプチャ有効・無効の切り替え
	void EnableCapture(BOOL bEnable) {m_bEnable = bEnable;}
	BOOL IsCaptureEnabled() const {return m_bEnable;}

	//接続メディアタイプの取得 (フィルタ接続後に必ず呼ぶ)
	HRESULT GetConnectionMediaType();

	//チャンネル数の取得
	int GetChannels() const {return m_nChannels;}
	//デシベル数の取得
	BOOL GetdB(double *pddB) {
		for (int i=0;i<m_nChannels;i++) {
			(*pddB) = m_pddB[i];
			pddB++;
		}
		return TRUE;
	}

private:
	HRESULT SetMediaType(AM_MEDIA_TYPE* pamt);
	HRESULT GetMediaType(AM_MEDIA_TYPE** ppamt);

	//デシベルデータを送信
	HRESULT SenddBData(int nChannels,double* pddB);

	//他に必要な関数は何がある？

private:

	//////////////////////////////////////////////////
	//音量メーターのサンプルグラバのコールバッククラス
	class CDirectShowAudioVolumeCB : public ISampleGrabberCB
	{
	public:
		//コンストラクタ
		CDirectShowAudioVolumeCB(CDirectShowAudioVolume* pParentClass) {
			m_pParentClass = pParentClass;
		}
		//参照カウントのみせかけ
		STDMETHODIMP_(ULONG) AddRef() { return 1; }
		STDMETHODIMP_(ULONG) Release() { return 2; }
		//インタフェースの提供
		STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
		{
			if (ppvObject == NULL) return E_POINTER;
			if (riid == __uuidof(IUnknown)) {
				*ppvObject = static_cast<IUnknown*>(this);
				return S_OK;
			} else if (riid == __uuidof(ISampleGrabberCB)) {
				*ppvObject = static_cast<ISampleGrabberCB*>(this);
				return S_OK;
			}
			return E_NOTIMPL;
		}
		//サンプルの通過(IMediaSample型)
		STDMETHODIMP SampleCB(double Time, IMediaSample *pSample);/* {return E_NOTIMPL;}*/
		//サンプルの通過(BYTE型) こっちは使わない
		STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen) {return E_NOTIMPL;}
	private:
		CDirectShowAudioVolume* m_pParentClass;	//親ポインタ
	};

	//////////////////////////////////////////////////
	//メンバ変数(ここから)
	IBaseFilter* m_pFilter;								//フィルタ
	ISampleGrabber* m_pSampleGrabber;					//サンプルグラバインタフェース
	CDirectShowAudioVolumeCB* m_pAudioVolumeCallback;	//コールバック
	AM_MEDIA_TYPE m_ConnectionMediaType;				//現在の接続メディアタイプ
	HWND m_hWnd;										//通知用のウィンドウハンドル
	double* m_pddB;										//音量データのポインタ
	int     m_nChannels;								//音量データのチャンネル数
	BOOL m_bEnable;										//キャプチャが有効かどうか
	//////////////////////////////////////////////////
	//メンバ変数(ここまで)
};
