#pragma once


// CMonitorStatisticsDialog ダイアログ

class CMonitorStatisticsDialog : public CDialog
{
	DECLARE_DYNAMIC(CMonitorStatisticsDialog)

public:
	CMonitorStatisticsDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CMonitorStatisticsDialog();

// ダイアログ データ
	enum { IDD = IDD_MONITORSTATISTICSDIALOG };

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);
	//統計情報の表示
	void DrawStatics();

private:
	CTime m_tStartTime;			//開始時間
	CTime m_tEndTime;			//終了時間
	CTimeSpan m_tElapseTime;	//経過時間

	//バイトを文字列に変換するサブルーチン
	CString ByteToString(QWORD qwByte);
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
