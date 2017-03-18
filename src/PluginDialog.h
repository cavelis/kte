#pragma once


// CPluginDialog ダイアログ

class CPluginDialog : public CDialog
{
	DECLARE_DYNAMIC(CPluginDialog)

public:
	CPluginDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CPluginDialog();

// ダイアログ データ
	enum { IDD = IDD_PLUGINDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
public:
	virtual BOOL OnInitDialog();

private:
	//////////////////////////////////////////////////
	//メンバ変数ここから
	CToolTipCtrl m_ToolTip;					//ダイアログに表示するツールチップ

	//挿入可能なプラグインのリスト
	vector<FilterInfoStruct> m_AllDShowFilterList;			//DirectShowフィルタカテゴリのプラグイン
	vector<FilterInfoStruct> m_VideoDShowFilterList;		//DirectShowビデオプラグイン
	vector<FilterInfoStruct> m_AudioDShowFilterList;		//DirectShowオーディオプラグイン
	vector<FilterInfoStruct> m_VideoDMOFliterList;			//DMOビデオプラグイン
	vector<FilterInfoStruct> m_AudioDMOFliterList;			//DMOオーディオプラグイン

	BOOL m_bChange;	//変更点があったかどうか

	//////////////////////////////////////////////////
	//メンバ変数ここまで

	//メンバ関数
	BOOL InsertPluginButton(BOOL bVideo);	//追加ボタン
	BOOL DeletePluginButton(BOOL bVideo);	//削除ボタン
	BOOL PropertyButton(BOOL bVideo);		//プロパティの表示ボタン
	BOOL UpDownButton(BOOL bVideo,BOOL bUp);//上へ・下へボタン

	BOOL EnumPluginList(BOOL bAllPlugin);	
	BOOL RedrawPlugin(BOOL bVideo);
	int GetSelectedListCtrlIndex(BOOL bVideo);
	BOOL StopFilter();
	BOOL StartFilter(BOOL bVideo,BOOL bRun,BOOL bInsert,BOOL bRestart);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedVideoPluginInsertButton(){InsertPluginButton(TRUE);}
	afx_msg void OnBnClickedAudioPluginInsertButton(){InsertPluginButton(FALSE);}
	afx_msg void OnBnClickedVideoPluginDeleteButton(){DeletePluginButton(TRUE);}
	afx_msg void OnBnClickedAudioPluginDeleteButton(){DeletePluginButton(FALSE);}
	afx_msg void OnBnClickedVideoPluginPropertyButton(){PropertyButton(TRUE);}
	afx_msg void OnBnClickedAudioPluginPropertyButton(){PropertyButton(FALSE);}
	afx_msg void OnBnClickedVideoPluginUpButton(){UpDownButton(TRUE,TRUE);}
	afx_msg void OnBnClickedAudioPluginUpButton(){UpDownButton(FALSE,TRUE);}
	afx_msg void OnBnClickedVideoPluginDownButton(){UpDownButton(TRUE,FALSE);}
	afx_msg void OnBnClickedAudioPluginDownButton(){UpDownButton(FALSE,FALSE);}
	afx_msg void OnBnClickedEnumPluginCheck();
};
