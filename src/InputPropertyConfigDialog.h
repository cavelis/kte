#pragma once


// CInputPropertyConfigDialog ダイアログ

class CInputPropertyConfigDialog : public CDialog
{
	DECLARE_DYNAMIC(CInputPropertyConfigDialog)

public:
	CInputPropertyConfigDialog(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CInputPropertyConfigDialog();

// ダイアログ データ
	enum { IDD = IDD_INPUTPROPERTYCONFIGDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();

	//変更があったかどうか？
	BOOL m_bChange;
	BOOL m_bAudioBufferChange;

	CToolTipCtrl m_ToolTip;					//ダイアログに表示するツールチップ

public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedClockDisableRadio(){OnBnClickedClockEnableRadio(FALSE);}
	afx_msg void OnBnClickedClockEnableRadio(){OnBnClickedClockEnableRadio(TRUE);}
	void OnBnClickedClockEnableRadio(BOOL bEnable);

	afx_msg void OnBnClickedAudioBufferAutoRadio(){OnBnClickedAudioBufferRadio(FALSE);}
	afx_msg void OnBnClickedAudioBufferManualRadio(){OnBnClickedAudioBufferRadio(TRUE);}
	void OnBnClickedAudioBufferRadio(BOOL bEnable);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedInputSettingResetButton();
};
