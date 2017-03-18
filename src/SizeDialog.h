#pragma once


// CSizeDialog ダイアログ

class CSizeDialog : public CDialog
{
	DECLARE_DYNAMIC(CSizeDialog)

public:
	CSizeDialog(int nID,CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CSizeDialog();

// ダイアログ データ
	//enum { IDD = IDD_SIZEDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()

	CSize m_ScrollSize;		//現在のダイアログの幅と高さ
	CPoint m_ScrollPos;		//現在のXY座標
	CRect m_ScorllRect;		//現在のダイアログの幅

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
