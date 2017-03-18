#pragma once


// CMonitorPullDialog ダイアログ

class CMonitorPullDialog : public CDialog
{
	DECLARE_DYNAMIC(CMonitorPullDialog)

public:
	CMonitorPullDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CMonitorPullDialog();

// ダイアログ データ
	enum { IDD = IDD_MONITORPULLDIALOG };

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()

public:

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	LRESULT OnNetworkClientConnect(WPARAM wParam,LPARAM lParam);
	LRESULT OnNetworkClientDisconnect(WPARAM wParam,LPARAM lParam);


	BOOL Encode(BOOL bStart);

	afx_msg void OnPaint();

	int GetAllConnectCount()const {return m_nAllConnectCount;}

	int m_nAllConnectCount;			//総接続数
	vector<int> m_ConnectCountList;	//各ポートの接続数

	//リストビューの情報
	struct PullConnectStruct
	{
		int nSrcPort;			//こちら側のポート番号
		CString strIPAddress;	//IPアドレス(文字列)
		int nIPAddress[4];		//IPアドレス(数値)
		DWORD dwIPAddress;		//IPアドレス
		CString strHostName;	//ホスト名
		int nDestPort;			//あちら側のポート番号
		CTime StartTime;		//接続開始時間
		CTimeSpan ConnectTime;	//接続中の時間
		//BOOL bNewInsert;		//新しく挿入したデータのフラグ
	};
	vector<PullConnectStruct> m_ConnectList;	//リストビューの情報
	//追加する情報
	struct AddConnectStruct
	{
		DWORD dwIPAddress;
		DWORD dwPort;
		int nIndex;
	};
	vector<AddConnectStruct> m_AddConnectList;	//追加される情報

	//総接続数表示の更新
	void UpdateConnectionEdit();

	//タイマーのイベントハンドラ
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//リストコントロールの取得
	CListCtrl* GetListCtrl() {
		CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_PULL_LIST);
		assert(list);
		return list;
	}
};
