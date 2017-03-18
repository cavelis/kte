#pragma once

#include "OutputBaseDialog.h"

// COutputPullDialog ダイアログ

class COutputPullDialog : public COutputBaseDialog
{
	DECLARE_DYNAMIC(COutputPullDialog)

public:
	COutputPullDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~COutputPullDialog();

// ダイアログ データ
	enum { IDD = IDD_OUTPUTPULLDIALOG };

	//出力名の取得(サーバー名と公開ポイント)
	BOOL GetOutputName(CString &strFileName);
	//出力名の設定(サーバー名と公開ポイント)
	BOOL SetOutputName(CString strFileName);
	//タブ名の取得
	BOOL GetTabName(CString &strTabName);
	//タブ名の設定
	BOOL SetTabName(CString strTabName);
	//有効チェックの取得
	BOOL GetEnableCheck();
	//有効チェックの設定
	BOOL SetEnableCheck(BOOL bCheck);

	//ポート番号の取得
	BOOL GetPort(int &nPort);
	//ポート番号の設定
	BOOL SetPort(int nPort);
	//最大接続人数の取得
	BOOL GetMax(int &nMax);
	//最大接続人数の設定
	BOOL SetMax(int nMax);

	//適用ボタンの表示・非表示
	BOOL ShowApplyButton(BOOL bShow);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK(){}
	virtual void OnCancel(){}

public:
	afx_msg void OnBnClickedOutputPullEnableCheck();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeOutputPullMaxEdit();
	//afx_msg void OnBnClickedOutputPullApplyButton();

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);

private:
	BOOL m_bFirstFlag;	//初期化フラグ
	CToolTipCtrl m_ToolTip;	//ダイアログに表示するツールチップ

	int m_nRegisterID;				//シンク登録ID(ネットワークシンク)
	int m_nRegisterPort;			//シンク登録したポート番号
	int m_nRegisterMax;			//シンク登録した最大接続人数

	//グローバルIPアドレスを取得
	BOOL GetGlobalIPAddress(CString strURL,CString &strIPAddress,CString &strHostName);
	//クリップボードに文字列を設定する
	BOOL SetClipText(LPCTSTR text);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOutputApplyButton();
	afx_msg void OnDestroy();
	afx_msg void OnEnChangeOutputPullPortEdit();
	afx_msg void OnBnClickedOutputPlayerButton();
	afx_msg void OnBnClickedOutputIpaddrButton();

	afx_msg void OnIpPopup(){OnIPHostPopup(TRUE);}
	afx_msg void OnHostPopup(){OnIPHostPopup(FALSE);}

	void OnIPHostPopup(BOOL bIP);
};
