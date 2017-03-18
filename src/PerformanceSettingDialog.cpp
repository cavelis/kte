// PerformanceSettingDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "PerformanceSettingDialog.h"


// CPerformanceSettingDialog ダイアログ

IMPLEMENT_DYNAMIC(CPerformanceSettingDialog, CDialog)

CPerformanceSettingDialog::CPerformanceSettingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPerformanceSettingDialog::IDD, pParent)
{
}

CPerformanceSettingDialog::~CPerformanceSettingDialog()
{
}

void CPerformanceSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPerformanceSettingDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CPerformanceSettingDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPerformanceSettingDialog::OnBnClickedCancel)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CPU_RADIO1, &CPerformanceSettingDialog::OnBnClickedCpuRadio1)
	ON_BN_CLICKED(IDC_CPU_RADIO2, &CPerformanceSettingDialog::OnBnClickedCpuRadio2)
	ON_BN_CLICKED(IDC_CPU_RADIO3, &CPerformanceSettingDialog::OnBnClickedCpuRadio3)
	ON_BN_CLICKED(IDC_PROCESS_DISABLE_RADIO, &CPerformanceSettingDialog::OnBnClickedProcessDisableRadio)
	ON_BN_CLICKED(IDC_PROCESS_ENABLE_RADIO, &CPerformanceSettingDialog::OnBnClickedProcessEnableRadio)
END_MESSAGE_MAP()


//////////////////////////////////////////////////
//ダイアログの初期化
//////////////////////////////////////////////////
BOOL CPerformanceSettingDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ToolTip.Create(this);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_CPU_PRIORITY_SLIDER),IDC_CPU_PRIORITY_SLIDER);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_CPU_RADIO1),IDC_CPU_RADIO1);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_CPU_RADIO2),IDC_CPU_RADIO2);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_PROCESSOR_EDIT),IDC_CPU_RADIO2);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_PROCESSOR_SPIN),IDC_CPU_RADIO2);
	m_ToolTip.AddTool((CWnd *)GetDlgItem(IDC_CPU_RADIO3),IDC_CPU_RADIO3);
	m_ToolTip.SetMaxTipWidth(300);
	DWORD dwAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP,dwAutoPop*2);

	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	CButton* button;

	//プロセス
	if (pProfile->m_bProcessPriority) {
		button = (CButton*)GetDlgItem(IDC_PROCESS_ENABLE_RADIO); assert(button);
		button->SetCheck(BST_CHECKED);
		ProcessEnableRadio(TRUE);
	} else {
		button = (CButton*)GetDlgItem(IDC_PROCESS_DISABLE_RADIO); assert(button);
		button->SetCheck(BST_CHECKED);
		ProcessEnableRadio(FALSE);
	}
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem(IDC_CPU_PRIORITY_SLIDER); assert(slider);
	slider->SetRange(0,5);
	if (pProfile->m_nProcessPriority >= 0 && (pProfile->m_nProcessPriority <= 5))
		slider->SetPos(pProfile->m_nProcessPriority);
	else 
		slider->SetPos(2);
	this->OnHScroll(0,slider->GetPos(),(CScrollBar*)slider);

	//CPUの個数を取得
	SYSTEM_INFO sys_info;
	ZeroMemory(&sys_info,sizeof(SYSTEM_INFO));
	GetSystemInfo(&sys_info);

	CSpinButtonCtrl* spin = (CSpinButtonCtrl*)GetDlgItem(IDC_PROCESSOR_SPIN); assert(spin);
	spin->SetRange32(0,sys_info.dwNumberOfProcessors - 1);
	spin->SetPos32(pProfile->m_nProcessIdealProcessor);
	CEdit* edit = (CEdit*)GetDlgItem(IDC_PROCESSOR_EDIT); assert(edit);
	CString strNum;
	strNum.Format(_T("%d"),pProfile->m_nProcessIdealProcessor);
	edit->SetWindowText(strNum);

	//CPU個数以外のコントロールを無効にする
	int nStartID = IDC_CPU_CHECK0 + sys_info.dwNumberOfProcessors;
	int nEndID = IDC_CPU_CHECK31;
	for (int i = nStartID;i<=nEndID;i++) {
		button = (CButton*)GetDlgItem(i); assert(button);
		button->EnableWindow(FALSE);
	}

	//アフィニティマスクの設定
	if (pProfile->m_dwProcessAffinityMask == 0 || pProfile->m_dwProcessAffinityMask == 0xFFFFFFFF) {
		for (int i = IDC_CPU_CHECK0;i<=IDC_CPU_CHECK31;i++) {	//すべてにチェック
			button = (CButton*)GetDlgItem(i); assert(button);
			if (button->IsWindowEnabled())
				button->SetCheck(BST_CHECKED);
		}
	} else {	//アフィニティマスクあり
		for (int i = IDC_CPU_CHECK0;i<=IDC_CPU_CHECK31;i++) {	//すべてにチェック
			button = (CButton*)GetDlgItem(i); assert(button);
			if (button->IsWindowEnabled())
			{
				DWORD dwMask;
				dwMask = 0x01 << (i - IDC_CPU_CHECK0);
				if (dwMask & pProfile->m_dwProcessAffinityMask) {
					button->SetCheck(BST_CHECKED);
				}
			}
		}
	}

	if (pProfile->m_bProcessLimitEnable)
	{
		if (pProfile->m_bProcessAffinityPriority) {
			CButton* button1 = (CButton*)GetDlgItem(IDC_CPU_RADIO3);
			button1->SetCheck(BST_CHECKED);
			OnCPURadioButtonClick(3);
		} else {
			CButton* button1 = (CButton*)GetDlgItem(IDC_CPU_RADIO2);
			button1->SetCheck(BST_CHECKED);
			OnCPURadioButtonClick(2);
		}
	} else {
		CButton* button1 = (CButton*)GetDlgItem(IDC_CPU_RADIO1);
		button1->SetCheck(BST_CHECKED);
		OnCPURadioButtonClick(1);
	}

	return TRUE;
}

//////////////////////////////////////////////////
//OKボタン
//////////////////////////////////////////////////
void CPerformanceSettingDialog::OnBnClickedOk()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();

	CButton* button = (CButton*)GetDlgItem(IDC_PROCESS_DISABLE_RADIO); assert(button);
	if (button->GetCheck() & BST_CHECKED)
		pProfile->m_bProcessPriority = FALSE;
	else
		pProfile->m_bProcessPriority = TRUE;
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem(IDC_CPU_PRIORITY_SLIDER); assert(slider);
	pProfile->m_nProcessPriority = slider->GetPos();

	CButton* button1 = (CButton*)GetDlgItem(IDC_CPU_RADIO1); assert(button1);
	CButton* button2 = (CButton*)GetDlgItem(IDC_CPU_RADIO2); assert(button2);

	if (button1->GetCheck() & BST_CHECKED) {
		pProfile->m_bProcessLimitEnable = FALSE;
	} else {
		pProfile->m_bProcessLimitEnable = TRUE;
	}
	if (button2->GetCheck() & BST_CHECKED) {
		pProfile->m_bProcessAffinityPriority = FALSE;
	} else {
		pProfile->m_bProcessAffinityPriority = TRUE;
	}

	CSpinButtonCtrl* spin = (CSpinButtonCtrl*)GetDlgItem(IDC_PROCESSOR_SPIN); assert(spin);
	pProfile->m_nProcessIdealProcessor = spin->GetPos32();

	//アフィニティマスクを作成
	int dwMask = 0;
	for (int i=IDC_CPU_CHECK0;i<=IDC_CPU_CHECK31;i++)
	{
		button = (CButton*)GetDlgItem(i); assert(button);
		if (button->GetCheck() & BST_CHECKED)
		{
			dwMask |= 0x01 << (i - IDC_CPU_CHECK0);
		}
	}
	if (dwMask == 0) {
		dwMask = 0xFFFFFFFF;
	} else {
		pProfile->m_dwProcessAffinityMask = dwMask;
	}
	OnOK();
}

//////////////////////////////////////////////////
//キャンセルボタン
//////////////////////////////////////////////////
void CPerformanceSettingDialog::OnBnClickedCancel()
{
	OnCancel();
}
//////////////////////////////////////////////////
//プロセス優先度の有効・無効
//////////////////////////////////////////////////
void CPerformanceSettingDialog::ProcessEnableRadio(BOOL bEnable)
{
	CWnd* wnd;
	wnd = GetDlgItem(IDC_CPU_PRIORITY_SLIDER);
	wnd->EnableWindow(bEnable);
	wnd = GetDlgItem(IDC_CPU_PRIORITY_STATIC);
	wnd->EnableWindow(bEnable);
}
//////////////////////////////////////////////////
//スクロールバー
//////////////////////////////////////////////////
void CPerformanceSettingDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* slider = (CSliderCtrl*)this->GetDlgItem(IDC_CPU_PRIORITY_SLIDER); assert(slider);
	if (pScrollBar->GetSafeHwnd() == slider->GetSafeHwnd())
	{
		CWnd* staics = (CWnd*)GetDlgItem(IDC_CPU_PRIORITY_STATIC); assert(staics);
		switch (slider->GetPos())
		{
		case 0:
			staics->SetWindowText(_T("(アイドル)")); break;
		case 1:
			staics->SetWindowText(_T("(通常以下)")); break;
		case 3:
			staics->SetWindowText(_T("(通常以上)")); break;
		case 4:
			staics->SetWindowText(_T("(高い)")); break;
		case 5:
			staics->SetWindowText(_T("(リアルタイム)")); break;
		case 2:
		default:
			staics->SetWindowText(_T("(通常)"));
		}
		return;
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

//////////////////////////////////////////////////
//プロセッサの制限のラジオボタン
//////////////////////////////////////////////////
void CPerformanceSettingDialog::OnCPURadioButtonClick(int nIndex)
{
	if (nIndex == 2) {		//優先的にCPUコアを配分
		GetDlgItem(IDC_PROCESSOR_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROCESSOR_SPIN)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_PROCESSOR_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROCESSOR_SPIN)->EnableWindow(FALSE);
	}

	if (nIndex == 3) {		//使用するCPUコアを制限
		//CPUの個数を取得
		SYSTEM_INFO sys_info;
		ZeroMemory(&sys_info,sizeof(SYSTEM_INFO));
		GetSystemInfo(&sys_info);
		for (int i = IDC_CPU_CHECK0;i < (int)IDC_CPU_CHECK0 + (int)sys_info.dwNumberOfProcessors;i++)
			GetDlgItem(i)->EnableWindow(TRUE);
	} else {
		for (int i = IDC_CPU_CHECK0; i<=IDC_CPU_CHECK31; i++)
			GetDlgItem(i)->EnableWindow(FALSE);
	}
	return;
}

////////////////////////////////////////
//ツールチップの表示
////////////////////////////////////////
BOOL CPerformanceSettingDialog::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN: 
		case WM_LBUTTONUP: 
		case WM_MOUSEMOVE: 
			m_ToolTip.RelayEvent(pMsg);
			break;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

