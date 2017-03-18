#pragma once


// CMonitorEventLogDialog ダイアログ

class CMonitorEventLogDialog : public CDialog
{
	DECLARE_DYNAMIC(CMonitorEventLogDialog)

public:
	CMonitorEventLogDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CMonitorEventLogDialog();

// ダイアログ データ
	enum { IDD = IDD_MONITOREVENTLOGDIALOG };

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	//エンコードの開始と終了
	BOOL Encode(BOOL bStart);

	//イベントログの追加
	BOOL AddEventLog(const CString &strEventLog);

	//ログイベントのバッファ
	CString m_strLogEvent;

	CStdioFile m_fEventLogFile;		//自動保存のファイルクラス
	BOOL m_bEventLogFile;			//自動保存のファイルを開いているかどうか

	BOOL OpenEventLogFile(CString &strFileName);
	void CloseEventLogFile();

	afx_msg void OnBnClickedEventLogClearButton();
	afx_msg void OnBnClickedEventLogSaveButton();
	afx_msg void OnBnClickedAutoSaveCheck();
	afx_msg void OnBnClickedAutoSaveButton();
};
