#pragma once
#include "afxwin.h"


// CSettingDialog ダイアログ

class CSettingDialog : public CDialog
{
	DECLARE_DYNAMIC(CSettingDialog)

public:
	CSettingDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CSettingDialog();

// ダイアログ データ
	enum { IDD = IDD_SETTINGDIALOG };

private:
	CToolTipCtrl m_ToolTip;								//ダイアログに表示するツールチップ

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedApply();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedVideoPlayerButton();

	afx_msg void OnBnClickedKeyframeAutoRadio(){CheckKeyFrameButton(FALSE);}
	afx_msg void OnBnClickedKeyframeManualRadio(){CheckKeyFrameButton(TRUE);}
	void CheckKeyFrameButton(BOOL bEnable);

	afx_msg void OnBnClickedComplexityAutoRadio()   {CheckComplexityButton(FALSE);}
	afx_msg void OnBnClickedComplexityManualRadio() {CheckComplexityButton(TRUE);}
	void CheckComplexityButton(BOOL bEnable);

	afx_msg void OnNMCustomdrawCpuPrioritySlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedKeyframeManualRadio2();
	afx_msg void OnBnClickedComplexityAutoRadio2();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnBnClickedForcePreviewOffCheck();
};
