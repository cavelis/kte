
// MainFrm.h : CMainFrame クラスのインターフェイス
//

#pragma once


//各種バー
#include "MyStatusBar.h"
#include "MyToolBar.h"
#include "MySplitterWnd.h"

//子ウィンドウ
#include "PropertyFormView.h"
#include "PreviewWnd.h"
#include "VolumeWnd.h"
#include "MonitorFormView.h"

#include "MyTaskTray.h"

//
#include "DirectShowAccess.h"


//////////////////////////////////////////////////
//メインフレームウィンドウ
//	各ウィンドウの生成と管理
//	エンコードの開始と停止
//////////////////////////////////////////////////
class CMainFrame : public CFrameWnd
{
public:
	CMainFrame();			//コンストラクタ
	virtual ~CMainFrame();	//デストラクタ

protected: 
	//CObjectクラスから継承している場合は実行時型情報(RTTI)として、クラスの型を定義しなければならない
	DECLARE_DYNAMIC(CMainFrame)

public:
	//ウィンドウ生成時の構造体csを編集できる
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 実装
public:
	//各ウィンドウクラスを取得する
	inline CMyToolBar* GetToolBar(){return &m_wndToolBar;}
	inline CMyStatusBar* GetStatusBar(){return &m_wndStatusBar;}
	inline CPreviewWnd* GetPreviewWnd(){return &m_wndPreviewWnd;}
	inline CVolumeWnd*  GetVolumeWnd() {return &m_wndVolumeWnd;}
	inline CPropertyFormView* GetPropertyWnd() {
		CPropertyFormView* wnd = (CPropertyFormView*)m_wndLeftRightSplitter.GetColumnWnd(0);
		assert(wnd);
		return wnd;
	}
	inline CMonitorFormView* GetMonitorWnd() {
		CMonitorFormView* wnd = (CMonitorFormView*)m_wndTopButtomSplitter.GetRowWnd(1);
		assert(wnd);
		return wnd;
	}

	//エンコード開始と停止のボタンが押されているかどうか
	BOOL IsStartStopThread()const {return m_bStartStopThread;}
	//停止のためのアイドリングループ
	BOOL StopThreadMessageLoop();

	//タイトルバーの変更
	void ChangeTitleBar();
	//プロファイルの変更
	BOOL ChangeProfileNotify();
	//プロファイルの保存
	BOOL SaveProfileSetting();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	//////////////////////////////////////////////////
	//メンバ変数
	CMyToolBar m_wndToolBar;				//ツールバー
	CMyStatusBar m_wndStatusBar;			//ステータスバー
	CPreviewWnd m_wndPreviewWnd;			//プレビューウィンドウ(右上)
	CVolumeWnd	m_wndVolumeWnd;				//ボリュームウィンドウ(右上)

	//各種分割ウィンドウの情報
	CMySplitterWnd m_wndTopButtomSplitter;	//上下分割ウィンドウ
	CMySplitterWnd m_wndLeftRightSplitter;	//左右分割ウィンドウ
	CMySplitterWnd m_wndPreviewSplitter;	//プレビューの分割ウィンドウ
	BOOL m_bSplitterInit;			//分割ウィンドウの初期化がされているかどうか？
	int  m_nSplitterColumnWidth[2];	//左右ペインの各サイズ(設定値)
	int  m_nSplitterRowHeight[2];	//上下ペインの各サイズ(設定値)
	int  m_nSplitterPreviewSize[2];	//プレビューペインの各サイズ(設定値) ※この値は幅なのか高さなのかは分からない
	
	int m_nSplitterLineWidth;		//左右ペインの線の幅   (固定値)
	int m_nSplitterLineHegiht;		//上下ペインの線の高さ (固定値)

	//スレッド処理
	static UINT ThreadProcCalc(LPVOID pParam);	//スレッド関数
	CWinThread* m_StartStopThread;				//開始と終了スレッド
	BOOL        m_bStartStopThread;				//スレッドが実行中かどうか

	CMyTaskTray m_TaskTray;						//タスクトレイ
	//////////////////////////////////////////////////
	//メンバ変数(ここまで)

// 生成された、メッセージ割り当て関数
protected:

	//WM_CREATEのイベントハンドラ
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//WM_SETFOCUSのイベントハンドラ
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	//クラスがメッセージマップを宣言することを定義する
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	//分割ウィンドウの線幅を取得(サブルーチン)
	void CalcSplitterLine(int nClientWidth,int nClientHeight);

public:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg LRESULT OnGraphNotify(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTrayIcon(WPARAM wParam,LPARAM lParam);
	afx_msg void OnShowWindowFromTaskTray();

	afx_msg void OnUpdateEncode(CCmdUI *pCmdUI);
	afx_msg void OnUpdateProfile(CCmdUI *pCmdUI);

	void Showbar(const int nMenuID);
	afx_msg void OnShowToolbar(){Showbar(ID_SHOW_TOOLBAR);}
	afx_msg void OnShowStatusbar(){Showbar(ID_SHOW_STATUSBAR);}

	BOOL IsShowView(int nMenuID) {return (GetMenu()->GetMenuState(nMenuID,MF_BYCOMMAND) & MF_CHECKED);}

	void OnShowView(const int nMenuID,const BOOL bShow);
	afx_msg void OnShowProperty(){OnShowView(ID_SHOW_PROPERTY,!IsShowView(ID_SHOW_PROPERTY));}
	afx_msg void OnShowMonitor() {OnShowView(ID_SHOW_MONITOR, !IsShowView(ID_SHOW_MONITOR));}
	afx_msg void OnShowPreview() {OnShowView(ID_SHOW_PREVIEW, !IsShowView(ID_SHOW_PREVIEW));}
	afx_msg void OnShowVolume()  {OnShowView(ID_SHOW_VOLUME,  !IsShowView(ID_SHOW_VOLUME));}

	//エンコードの開始と停止
	afx_msg void OnStartEncode();	//エンコードの開始(ボタン)
	afx_msg void OnStopEncode();	//エンコードの停止(ボタン)
	afx_msg LRESULT OnNotifyStopEncode(WPARAM wParam,LPARAM lParam);
	BOOL StartEncode();			//エンコードの開始(関数)
	BOOL PreStopEncode();		//エンコード停止のプレ処理(WMメッセージ)
	BOOL PostStopEncode();		//エンコード停止のポスト処理(WMメッセージ)
};

//////////////////////////////////////////////////
//[EOF]
