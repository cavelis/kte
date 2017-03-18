// PropertyFormView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "KTE.h"
#include "PropertyFormView.h"


// CPropertyFormView

IMPLEMENT_DYNCREATE(CPropertyFormView, CFormView)

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CPropertyFormView::CPropertyFormView()
	: CFormView(CPropertyFormView::IDD)
{
}

//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CPropertyFormView::~CPropertyFormView()
{
}

//////////////////////////////////////////////////
//DDX/DDVの初期化
//////////////////////////////////////////////////
void CPropertyFormView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropertyFormView, CFormView)
	ON_MESSAGE( WM_INITDIALOG, &CPropertyFormView::OnInitDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_PROPERTY_TAB, &CPropertyFormView::OnTcnSelchangePropertyTab)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CPropertyFormView 診断

#ifdef _DEBUG
void CPropertyFormView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropertyFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

//////////////////////////////////////////////////
//ウィンドウスタイル変更
//////////////////////////////////////////////////
BOOL CPropertyFormView::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL bResult = CFormView::PreCreateWindow(cs);

	//cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	//cs.dwExStyle |= WS_EX_WINDOWEDGE;
	//cs.style &= ~WS_BORDER;

	return bResult;
}

//////////////////////////////////////////////////
//WM_INITDIALOGのイベントハンドラ
//この段階でHWNDとコントロールは初期化されている(使用可能)
//だがDDX/DDVは使用不可能なのでGetDlgItemから取得すること。
//////////////////////////////////////////////////
LRESULT CPropertyFormView::OnInitDialog(WPARAM wParam,LPARAM lParam)
{
	//各種タブ(子ダイアログ)を作成する
	m_InputTab.Create(CInputPropertyDialog::IDD,this);
	m_OutputTab.Create(COutputPropertyDialog::IDD,this);
	m_EncodeTab.Create(CEncodePropertyDialog::IDD,this);
	m_MetadataTab.Create(CMetadataPropertyDialog::IDD,this);

	//子ダイアログを移動する
	CRect rcDialog;
	m_InputTab.GetClientRect(&rcDialog);
	rcDialog.OffsetRect(0,20);
	m_InputTab.MoveWindow(&rcDialog,FALSE);
	m_OutputTab.MoveWindow(&rcDialog,FALSE);
	m_EncodeTab.MoveWindow(&rcDialog,FALSE);
	m_MetadataTab.MoveWindow(&rcDialog,FALSE);

	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_PROPERTY_TAB);
	tab->ModifyStyle(0,TCS_FIXEDWIDTH,0);

	tab->InsertItem(0,_T("入力"));
	tab->InsertItem(1,_T("出力"));
	tab->InsertItem(2,_T("圧縮"));
	tab->InsertItem(3,_T("属性"));

	CSize size;
	CRect rcRect;
	tab->GetItemRect(0,&rcRect);
	size.SetSize(65,rcRect.bottom - rcRect.top);
	CSize old = tab->SetItemSize(size);
	size.SetSize(65,old.cy);
	tab->SetItemSize(size);

	//タブを表示
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	tab->SetCurSel(pSetting->m_nPropertyTabIndex);
	OnTcnSelchangePropertyTab(NULL,NULL);
	return 0;
}
//////////////////////////////////////////////////
//ダイアログが終了するときに呼ばれるイベントハンドラ
//////////////////////////////////////////////////
void CPropertyFormView::OnDestroy()
{
	CSetting* pSetting = ((CKTEApp*)AfxGetApp())->GetSetting();
	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_PROPERTY_TAB);
	if (tab)
		pSetting->m_nPropertyTabIndex = tab->GetCurSel();

	//各種タブダイアログを閉じる
	if (m_EncodeTab.GetSafeHwnd())
	{
		m_EncodeTab.EndDialog(0);
		m_EncodeTab.DestroyWindow();
	}
	if (m_OutputTab.GetSafeHwnd())
	{
		m_OutputTab.EndDialog(0);
		m_OutputTab.DestroyWindow();
	}
	if (m_InputTab.GetSafeHwnd())
	{
		m_InputTab.EndDialog(0);
		m_InputTab.DestroyWindow();
	}
	if (m_MetadataTab.GetSafeHwnd())
	{
		m_MetadataTab.EndDialog(0);
		m_MetadataTab.DestroyWindow();
	}

	CFormView::OnDestroy();

	// TODO: ここにメッセージ ハンドラ コードを追加します。
}

//////////////////////////////////////////////////
//ウィンドウサイズ変更
//////////////////////////////////////////////////
void CPropertyFormView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	//タブコントロールを最大化する
	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_PROPERTY_TAB);
	if (!tab) return;
	CRect rcClient;
	this->GetClientRect(&rcClient);
	tab->MoveWindow(rcClient,TRUE);

	//内側のダイアログタブも最大化っぽく移動する
	if (!m_InputTab.GetSafeHwnd() || !m_OutputTab.GetSafeHwnd() || !m_EncodeTab.GetSafeHwnd())
		return;
	rcClient.OffsetRect(0,22);
	rcClient.right -= 5;
	rcClient.bottom -= 25;
	m_InputTab.MoveWindow(&rcClient);
	m_OutputTab.MoveWindow(&rcClient);
	m_EncodeTab.MoveWindow(&rcClient);
	m_MetadataTab.MoveWindow(&rcClient);
}

//////////////////////////////////////////////////
//タブを変更した時に呼び出されるイベントハンドラ
//////////////////////////////////////////////////
void CPropertyFormView::OnTcnSelchangePropertyTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	CTabCtrl* tab = (CTabCtrl*)GetDlgItem(IDC_PROPERTY_TAB);
	if (!tab) return;

	int nSelectSel = tab->GetCurSel();
	m_InputTab.ShowWindow(nSelectSel == 0 ? SW_SHOW : SW_HIDE);
	m_OutputTab.ShowWindow(nSelectSel == 1 ? SW_SHOW : SW_HIDE);
	m_EncodeTab.ShowWindow(nSelectSel == 2 ? SW_SHOW : SW_HIDE);
	m_MetadataTab.ShowWindow(nSelectSel == 3 ? SW_SHOW : SW_HIDE);

	if (pResult) *pResult = 0;
}

//////////////////////////////////////////////////
//プロファイルの変更通知
//////////////////////////////////////////////////
BOOL CPropertyFormView::ChangeProfileNotify()
{
	m_InputTab.ChangeProfileNotify();
	m_OutputTab.ChangeProfileNotify();
	m_EncodeTab.ChangeProfileNotify();
	m_MetadataTab.ChangeProfileNotify();
	return TRUE;
}
//////////////////////////////////////////////////
//プロファイルのセーブ
//////////////////////////////////////////////////
BOOL CPropertyFormView::SaveProfileSetting()
{
	m_InputTab.SaveProfileSetting();
	m_OutputTab.SaveProfileSetting();
	m_EncodeTab.SaveProfileSetting();
	m_MetadataTab.SaveProfileSetting();
	return TRUE;
}

//////////////////////////////////////////////////
//エンコードの開始と終了
//////////////////////////////////////////////////
BOOL CPropertyFormView::Encode(BOOL bStart)
{
	if (!m_InputTab.Encode(bStart))
		return FALSE;
	if (!m_OutputTab.Encode(bStart))
		return FALSE;
	if (!m_EncodeTab.Encode(bStart))
		return FALSE;
	if (!m_MetadataTab.Encode(bStart))
		return FALSE;
	return TRUE;
}
