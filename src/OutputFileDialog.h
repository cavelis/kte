#pragma once

#include "OutputBaseDialog.h"

// COutputFileDialog ダイアログ

class COutputFileDialog : public COutputBaseDialog
{
	DECLARE_DYNAMIC(COutputFileDialog)

public:
	COutputFileDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~COutputFileDialog();

// ダイアログ データ
	enum { IDD = IDD_OUTPUTFILEDIALOG };

	//出力名の取得(出力ファイル)
	BOOL GetOutputName(CString &strFileName);
	//出力名の設定(出力ファイル)
	BOOL SetOutputName(CString strFileName);
	//タブ名の取得
	BOOL GetTabName(CString &strTabName);
	//タブ名の設定
	BOOL SetTabName(CString strTabName);
	//有効チェックの取得
	BOOL GetEnableCheck();
	//有効チェックの設定
	BOOL SetEnableCheck(BOOL bCheck);
	//適用ボタンの表示と非表示
	BOOL ShowApplyButton(BOOL bShow);

	//出力設定名の取得と変更
	BOOL GetOutputFileName(CString &strFileName);
	BOOL GetOutputFolderName(CString &strFolderName);
	BOOL SetOutputFileName(CString strFileName);
	BOOL SetOutputFolderName(CString strFolderName);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK(){}
	virtual void OnCancel(){}

public:
	afx_msg void OnBnClickedOutputFileButton();
	afx_msg void OnBnClickedOutputFileEnableCheck();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedOutputApplyButton();

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	int m_nRegisterID;				//シンク登録ID(ファイルシンク)
	CString m_strRegisterFileName;	//シンク登録したファイル名
	CTime m_RegisterTime;			//シンク登録した時間
	CToolTipCtrl m_ToolTip;			//ダイアログに表示するツールチップ

	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	//出力ファイル名の設定
	BOOL MakeFilePath(CString strFolderPath,CString strFilePath,CTime &Time,CString &strMakePath);
	afx_msg void OnBnClickedOutputOpenButton();
};
