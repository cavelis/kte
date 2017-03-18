#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "InputPropertyDialog.h"
#include "OutputPropertyDialog.h"
#include "EncodePropertyDialog.h"
#include "MetadataPropertyDialog.h"

// CPropertyFormView フォーム ビュー

class CPropertyFormView : public CFormView
{
	DECLARE_DYNCREATE(CPropertyFormView)

protected:

public:
	CPropertyFormView();           // 動的生成で使用される protected コンストラクタ
	virtual ~CPropertyFormView();
	enum { IDD = IDD_PROPERTYFORMVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	//タブとタブダイアログ
	CTabCtrl m_xcPropertyTab;			//タブコントロール
	CInputPropertyDialog m_InputTab;	//入力タブ
	COutputPropertyDialog m_OutputTab;	//出力タブ
	CEncodePropertyDialog m_EncodeTab;	//圧縮タブ
	CMetadataPropertyDialog m_MetadataTab;//メタデータタブ

	CInputPropertyDialog* GetInputTab() {return &m_InputTab;}
	COutputPropertyDialog* GetOutputTab() {return &m_OutputTab;}
	CEncodePropertyDialog* GetEncodeTab() {return &m_EncodeTab;}
	CMetadataPropertyDialog* GetMetadataTab() {return &m_MetadataTab;}

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	LRESULT OnInitDialog(WPARAM wParam,LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnTcnSelchangePropertyTab(NMHDR *pNMHDR, LRESULT *pResult);

	//プロファイルの変更通知
	BOOL ChangeProfileNotify();
	//プロファイルのセーブ
	BOOL SaveProfileSetting();
	//エンコードの開始と停止
	BOOL Encode(BOOL bStart);
};


