#pragma once

//////////////////////////////////////////////////
//ダイアログクラス
//////////////////////////////////////////////////
class CAboutDialog : public CDialog
{
	DECLARE_DYNAMIC(CAboutDialog)

public:
	//コンストラクタ
	CAboutDialog(CWnd* pParent = NULL);
	//デストラクタ
	virtual ~CAboutDialog();

	//ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};



