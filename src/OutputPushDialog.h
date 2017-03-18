#pragma once

#include "OutputBaseDialog.h"

// COutputPushDialog ダイアログ

class COutputPushDialog : public COutputBaseDialog
{
	DECLARE_DYNAMIC(COutputPushDialog)

public:
	COutputPushDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~COutputPushDialog();
// ダイアログ データ
	enum { IDD = IDD_OUTPUTPUSHDIALOG };

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

	//自動削除の取得
	BOOL GetAutoDeleteCheck();
	//自動削除の設定
	BOOL SetAutoDeleteCheck(BOOL bCheck);

	//適用ボタンの表示・非表示
	BOOL ShowApplyButton(BOOL bShow);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK(){}
	virtual void OnCancel(){}

public:
	afx_msg void OnBnClickedOutputPushEnableCheck();
	afx_msg void OnBnClickedOutputPushAutodeleteCheck();

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);

	//サーバー名の文字列変換
	BOOL ConvertServerName(CString strOldName,CString &strNewName);

	CToolTipCtrl m_ToolTip;	//ダイアログに表示するツールチップ
	int m_nRegisterID;				//シンク登録ID(ネットワークシンク)
	CString m_strRegisterServerName;//シンク登録したサーバー名
	int m_bRegisterAutoDelete;		//シンク登録した自動削除設定

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOutputApplyButton();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedOutputPlayerButton();
};
