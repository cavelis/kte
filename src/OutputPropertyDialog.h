#pragma once

#include "SizeDialog.h"
#include "OutputBaseDialog.h"
#include "OutputPullDialog.h"
#include "OutputPushDialog.h"
#include "OutputFileDialog.h"

//自前のメッセージ番号を定義（100には特に意味はない）
#define NOTIFY_ENABLE_CHECK (WM_APP + 99)

// COutputPropertyDialog ダイアログ

class COutputPropertyDialog : public CSizeDialog
{
	DECLARE_DYNAMIC(COutputPropertyDialog)

public:
	COutputPropertyDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~COutputPropertyDialog();

// ダイアログ データ
	enum { IDD = IDD_OUTPUTPROPERTYDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK() {}
	virtual void OnCancel() {}

public:
	//プロファイルの変更通知
	BOOL ChangeProfileNotify();
	//プロファイルからのロード
	BOOL LoadProfileSetting();
	//プロファイルへのセーブ
	BOOL SaveProfileSetting();

	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);

	afx_msg void OnBnClickedOutputInsertButton();
	afx_msg void OnBnClickedOutputDeleteButton();

	enum OUTPUT_TYPE{
		OUTPUT_TYPE_PULL = 0,
		OUTPUT_TYPE_PUSH,
		OUTPUT_TYPE_FILE
	};

	afx_msg void OnInsertPull(){ this->OnInsertOutput(OUTPUT_TYPE_PULL,NULL,FALSE);}
	afx_msg void OnInsertPush(){ this->OnInsertOutput(OUTPUT_TYPE_PUSH,NULL,FALSE);}
	afx_msg void OnInsertFile(){ this->OnInsertOutput(OUTPUT_TYPE_FILE,NULL,FALSE);}
	HTREEITEM OnInsertOutput(OUTPUT_TYPE nOutputType,LPCTSTR lpszItemName,BOOL bCheck);

	virtual BOOL OnInitDialog();
private:
	//////////////////////////////////////////////////
	//メンバ変数
	//ツリービューのルートアイテムのハンドル
	HTREEITEM m_hPullTree;	//ブロードバンド
	HTREEITEM m_hPushTree;	//パブリッシュポイント
	HTREEITEM m_hFileTree;	//ファイルアーカイブ

	//下画面ダイアログ配列
	vector<COutputPullDialog*> m_pPullDialogList;
	vector<COutputPushDialog*> m_pPushDialogList;
	vector<COutputFileDialog*> m_pFileDialogList;

	COutputBaseDialog* m_pNowSelectDialog;		//現在表示中の下画面ダイアログのポインタ
	BOOL m_bFirstFlag;							//初回ダイアログ初期化フラグ
	CToolTipCtrl m_ToolTip;						//ダイアログに表示するツールチップ
	//メンバ変数ここまで
	//////////////////////////////////////////////////

public:
	afx_msg void OnTvnSelchangedOutputTree(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnTvnEndlabeleditOutputTree(NMHDR *pNMHDR, LRESULT *pResult);

	BOOL OnTreeCheckEvent(HTREEITEM hTreeItem,BOOL bCheck,BOOL bNotifyCall);
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg LRESULT OnChildEnableCheck(WPARAM wParam,LPARAM lParam);

	//ダイアログ配列を解放
	void DeleteAllDialog();

public:
	/*
	afx_msg void OnTvnBegindragOutputTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	*/
	afx_msg void OnTvnGetInfoTipOutputTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandingOutputTree(NMHDR *pNMHDR, LRESULT *pResult);

	//afx_msg void OnSize(UINT nType, int cx, int cy){CSizeDialog::OnSize(nType, cx, cy);}
	//afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar){CSizeDialog::OnVScroll(nSBCode, nPos, pScrollBar);}
};
