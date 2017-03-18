#pragma once

#include "SizeDialog.h"

// CEncodePropertyDialog ダイアログ

class CEncodePropertyDialog : public CSizeDialog
{
	DECLARE_DYNAMIC(CEncodePropertyDialog)

public:
	CEncodePropertyDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CEncodePropertyDialog();

// ダイアログ データ
	enum { IDD = IDD_ENCODEPROPERTYDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()

	BOOL m_bFirstFlag;		//初回ダイアログ初期化フラグ
	CToolTipCtrl m_ToolTip;	//ダイアログに表示するツールチップ

	//オーディオエンコーダーの設定
	DWORD m_dwBitrate;				//ビットレート
	DWORD m_nSamplesPerSec;			//周波数
	WORD m_wBitsPerSample;			//分解能
	WORD m_nChannels;				//チャンネル
	BOOL m_bAVSynchronization;		//A/V同期
	
	void AudioEncoderSettingChange();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnCbnSelchangeVideoEncoderCombo();
	afx_msg void OnCbnSelchangeAudioEncoderCombo();
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeAudioEncoderBitrateCombo();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedSizeAutoRadio();
	afx_msg void OnEnChangeVideoQualityEdit();

	//プロファイルの変更通知
	BOOL ChangeProfileNotify();
	//プロファイルからのロード
	BOOL LoadProfileSetting();
	//プロファイルへのセーブ
	BOOL SaveProfileSetting();

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);

protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnEnChangeVideoFramerateEdit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
public:
	//afx_msg void OnSize(UINT nType, int cx, int cy){CSizeDialog::OnSize(nType, cx, cy);}
	//afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar){CSizeDialog::OnVScroll(nSBCode, nPos, pScrollBar);}

	afx_msg void OnCbnSelchangeAudioEncoderSamplespersecCombo(){AudioEncoderSettingChange();}
	afx_msg void OnCbnSelchangeAudioEncoderBitspersampleCombo(){AudioEncoderSettingChange();}
	afx_msg void OnCbnSelchangeAudioEncoderChannelsCombo(){AudioEncoderSettingChange();}
	afx_msg void OnCbnSelchangeAudioEncoderAvsynchronization(){AudioEncoderSettingChange();}
};
