// MetadataPropertyDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "MetadataPropertyDialog.h"


// CMetadataPropertyDialog ダイアログ

IMPLEMENT_DYNAMIC(CMetadataPropertyDialog, CSizeDialog)

BEGIN_MESSAGE_MAP(CMetadataPropertyDialog, CSizeDialog)
	ON_WM_DESTROY()
	//ON_WM_SIZE()
	//ON_WM_VSCROLL()
END_MESSAGE_MAP()


CMetadataPropertyDialog::CMetadataPropertyDialog(CWnd* pParent /*=NULL*/)
	: CSizeDialog(CMetadataPropertyDialog::IDD, pParent)
{

}

CMetadataPropertyDialog::~CMetadataPropertyDialog()
{
}

void CMetadataPropertyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}




// CMetadataPropertyDialog メッセージ ハンドラ

//////////////////////////////////////////////////
//ダイアログ初期化
//////////////////////////////////////////////////
BOOL CMetadataPropertyDialog::OnInitDialog()
{
	CSizeDialog::OnInitDialog();

	//プロファイルの読み込みと反映
	LoadProfileSetting();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
//////////////////////////////////////////////////
//ダイアログ終了時
//////////////////////////////////////////////////
void CMetadataPropertyDialog::OnDestroy()
{
	//プロファイルへの保存
	SaveProfileSetting();
	CDialog::OnDestroy();
}

//////////////////////////////////////////////////
//ツールチップの表示
//////////////////////////////////////////////////
BOOL CMetadataPropertyDialog::PreTranslateMessage(MSG* pMsg)
{
	return CDialog::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////
//プロファイルの変更通知
//////////////////////////////////////////////////
BOOL CMetadataPropertyDialog::ChangeProfileNotify()
{
	//プロファイルのロード
	LoadProfileSetting();
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルからのロード
//////////////////////////////////////////////////
BOOL CMetadataPropertyDialog::LoadProfileSetting()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	CEdit* edit = (CEdit*)GetDlgItem(IDC_METADATA_TITLE_EDIT); assert(edit);
	edit->SetWindowText(pProfile->m_strMetadataTitle);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_AUTHOR_EDIT); assert(edit);
	edit->SetWindowText(pProfile->m_strMetadataAuthor);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_COPYRIGHT_EDIT); assert(edit);
	edit->SetWindowText(pProfile->m_strMetadataCopyright);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_RATING_EDIT); assert(edit);
	edit->SetWindowText(pProfile->m_strMetadataRating);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_DESCRIPTION_EDIT); assert(edit);
	edit->SetWindowText(pProfile->m_strMetadataDescription);
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルへのセーブ
//////////////////////////////////////////////////
BOOL CMetadataPropertyDialog::SaveProfileSetting()
{
	CProfile* pProfile = ((CKTEApp*)AfxGetApp())->GetProfile();
	CEdit* edit = (CEdit*)GetDlgItem(IDC_METADATA_TITLE_EDIT); assert(edit);
	edit->GetWindowText(pProfile->m_strMetadataTitle);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_AUTHOR_EDIT); assert(edit);
	edit->GetWindowText(pProfile->m_strMetadataAuthor);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_COPYRIGHT_EDIT); assert(edit);
	edit->GetWindowText(pProfile->m_strMetadataCopyright);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_RATING_EDIT); assert(edit);
	edit->GetWindowText(pProfile->m_strMetadataRating);
	edit = (CEdit*)GetDlgItem(IDC_METADATA_DESCRIPTION_EDIT); assert(edit);
	edit->GetWindowText(pProfile->m_strMetadataDescription);
	return TRUE;
}

//////////////////////////////////////////////////
//エンコードの開始と停止
//////////////////////////////////////////////////
BOOL CMetadataPropertyDialog::Encode(BOOL bStart)
{
	if (bStart) {
		CString strMetadataTitle;
		CString strMetadataAuthor;
		CString strMetadataCopyright;
		CString strMetadataRating;
		CString strMetadataDescription;
		CDirectShowAccess* pDSA = ((CKTEApp*)AfxGetApp())->GetDSA();
		CEdit* edit = (CEdit*)GetDlgItem(IDC_METADATA_TITLE_EDIT); assert(edit);
		edit->GetWindowText(strMetadataTitle);
		edit = (CEdit*)GetDlgItem(IDC_METADATA_AUTHOR_EDIT); assert(edit);
		edit->GetWindowText(strMetadataAuthor);
		edit = (CEdit*)GetDlgItem(IDC_METADATA_COPYRIGHT_EDIT); assert(edit);
		edit->GetWindowText(strMetadataCopyright);
		edit = (CEdit*)GetDlgItem(IDC_METADATA_RATING_EDIT); assert(edit);
		edit->GetWindowText(strMetadataRating);
		edit = (CEdit*)GetDlgItem(IDC_METADATA_DESCRIPTION_EDIT); assert(edit);
		edit->GetWindowText(strMetadataDescription);

		pDSA->SetMetadata(strMetadataTitle,strMetadataAuthor,
			strMetadataDescription,strMetadataRating,strMetadataCopyright);
	}
	//コモンコントロールの無効
	((CWnd*)GetDlgItem(IDC_METADATA_TITLE_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_METADATA_AUTHOR_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_METADATA_COPYRIGHT_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_METADATA_RATING_EDIT))->EnableWindow(bStart ? FALSE : TRUE);
	((CWnd*)GetDlgItem(IDC_METADATA_DESCRIPTION_EDIT))->EnableWindow(bStart ? FALSE : TRUE);

	return TRUE;
}