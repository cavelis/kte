#pragma once

#include "SizeDialog.h"

// CMetadataPropertyDialog ダイアログ

class CMetadataPropertyDialog : public CSizeDialog
{
	DECLARE_DYNAMIC(CMetadataPropertyDialog)

public:
	CMetadataPropertyDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CMetadataPropertyDialog();

// ダイアログ データ
	enum { IDD = IDD_METADATAPROPERTYDIALOG };

	virtual void OnOK(){}
	virtual void OnCancel(){}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();

	//プロファイルの変更通知
	BOOL ChangeProfileNotify();
	//プロファイルからのロード
	BOOL LoadProfileSetting();
	//プロファイルへのセーブ
	BOOL SaveProfileSetting();

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);

	//afx_msg void OnSize(UINT nType, int cx, int cy){CSizeDialog::OnSize(nType, cx, cy);}
	//afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar){CSizeDialog::OnVScroll(nSBCode, nPos, pScrollBar);}
};
