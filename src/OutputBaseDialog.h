#pragma once


// COutputBaseDialog ダイアログ

class COutputBaseDialog : public CDialog
{
	//DECLARE_DYNAMIC(COutputBaseDialog)

public:
	//COutputBaseDialog(CWnd* pParent = NULL);   // 標準コンストラクタ

	explicit COutputBaseDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL)
		: CDialog(nIDTemplate,pParentWnd)
	{
	}
	virtual ~COutputBaseDialog(){
	}

	//ファイル名の取得
	virtual BOOL GetOutputName(CString &strFileName) = 0;
	//ファイル名の設定
	virtual BOOL SetOutputName(CString strFileName) = 0;
	//タブ名の取得
	virtual BOOL GetTabName(CString &strTabName) = 0;
	//タブ名の設定
	virtual BOOL SetTabName(CString strTabName) = 0;
	//有効チェックの取得
	virtual BOOL GetEnableCheck() = 0;
	//有効チェックの設定
	virtual BOOL SetEnableCheck(BOOL bCheck) = 0;

// ダイアログ データ
	//enum { IDD = IDD_OUTPUTBASEDIALOG };

	//エンコードの開始と停止
	virtual BOOL Encode(BOOL bStart) = 0;

protected:
	//virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//DECLARE_MESSAGE_MAP()
};
