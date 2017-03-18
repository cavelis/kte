// AboutDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "AboutDialog.h"


// CAboutDialog ダイアログ

IMPLEMENT_DYNAMIC(CAboutDialog, CDialog)

//コンストラクタ
CAboutDialog::CAboutDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDialog::IDD, pParent)
{
}
//デストラクタ
CAboutDialog::~CAboutDialog()
{
}
//DDX/DDV
void CAboutDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//メッセージマップ開始
BEGIN_MESSAGE_MAP(CAboutDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CAboutDialog::OnBnClickedOk)
END_MESSAGE_MAP()
//メッセージマップ終了


void CAboutDialog::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	OnOK();
}

BOOL CAboutDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	CStatic* text = (CStatic*)this->GetDlgItem(IDC_VERSION_STATIC);
	assert(text);

	CString strVersion(_T(""));
	//text->GetWindowText(strVersion);
	strVersion.Format(_T("%s(KTE)  Version %d"),pSetting->m_strProgramName,pSetting->m_nVersion);
	text->SetWindowText(strVersion);

	HICON hIcon = ::LoadIcon (AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_BLUE_ON_ICON));
	((CStatic*)GetDlgItem(IDC_PICTURE_STATIC))->SetIcon(hIcon);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
