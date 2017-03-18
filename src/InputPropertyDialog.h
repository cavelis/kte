#pragma once

#include "SizeDialog.h"

// CInputPropertyDialog ダイアログ

class CInputPropertyDialog : public CSizeDialog
{
	DECLARE_DYNAMIC(CInputPropertyDialog)

public:
	CInputPropertyDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CInputPropertyDialog();

// ダイアログ データ
	enum { IDD = IDD_INPUTPROPERTYDIALOG };
	
	static const int SHARE_VIDEO_AUDIO = -2;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()

	//メンバ変数ここから
	BOOL m_bFirstFlag;			//初回ダイアログ初期化フラグ
	CToolTipCtrl m_ToolTip;		//ダイアログに表示するツールチップ
	//メンバ変数ここまで

	//ビデオデバイスの初期化に失敗
	BOOL VideoDeviceFailed(int nLevel);
	//ビデオデバイスの性能を表示
	BOOL VideoDeviceCapacity(int nIndex);
	//現在のビデオ入力値(幅・高さ・フレームレート)を得る
	void GetVideoCurrentInput(int &nWidth,int &nHeight,double &dFPS);
	//ビデオ入力値を設定(値を表示するだけで適用はしない)
	BOOL SetVideoCurrentInput(int nWidth, int nHeight, double dFPS);

	//オーディオデバイスの性能を表示
	BOOL AudioDeviceCapacity(int nIndex);
	//現在のオーディオ入力値を得る
	BOOL GetAudioCurrentInput(DWORD &nSamplesPerSec,WORD &wBitsPerSample,WORD &nChannels);
	//オーディオ入力値を設定(値を表示するだけで適用はしない)
	BOOL SetAudioCurrentInput(DWORD nSamplesPerSec,WORD wBitsPerSample,WORD nChannels);

	//ビデオキャプチャデバイスのオーディオピンの有無の処理
	void ProcVideoAudioInputPin();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeVideoSettingCombo();
	afx_msg void OnCbnSelchangeVideoResolutionCombo();
	BOOL OnSelectVideoSettingIndex(AM_MEDIA_TYPE* pamt);


	afx_msg void OnCbnSelchangeAudioSettingCombo();
	afx_msg void OnBnClickedVideoSettingButton();
	afx_msg void OnBnClickedAudioSettingButton();
	afx_msg void OnDestroy();

	BOOL ChangeProfileNotify();	//プロファイルの変更通知
	BOOL LoadProfileSetting();	//プロファイルからのロード
	BOOL SaveProfileSetting();	//プロファイルへのセーブ

	BOOL Encode(BOOL bStart);	//エンコードの開始と停止

	BOOL DeviceChange();	//デバイス通知のコールバックメッセージ

protected:
	virtual void OnOK(){}
	virtual void OnCancel(){}
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedVideoResolutionButton();

	//afx_msg void OnSize(UINT nType, int cx, int cy){CSizeDialog::OnSize(nType, cx, cy);}
	//afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar){CSizeDialog::OnVScroll(nSBCode, nPos, pScrollBar);}
	afx_msg void OnBnClickedAudioResolutionButton();
	afx_msg void OnCbnSelchangeAudioResolutionCombo();
	afx_msg void OnBnClickedInputConfigButton();

	afx_msg void OnWdmDevice(){OnWDMVideoSettingMenu(WDM_FILTER_NONE);}
	afx_msg void OnWdmCrossbar1(){OnWDMVideoSettingMenu(WDM_FILTER_CROSS_BAR1);}
	afx_msg void OnWdmCrossbar2(){OnWDMVideoSettingMenu(WDM_FILTER_CROSS_BAR2);}
	afx_msg void OnWdmTvaudio(){OnWDMVideoSettingMenu(WDM_FILTER_TV_AUDIO);}
	afx_msg void OnWdmTvtuner(){OnWDMVideoSettingMenu(WDM_FILTER_TV_TUNER);}

	void OnWDMVideoSettingMenu(ENUM_WDM_FILTER e);

};
