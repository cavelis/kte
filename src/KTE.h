#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル
#include "DirectShowAccess.h"
#include "Setting.h"
#include "Profile.h"

//////////////////////////////////////////////////
//アプリケーションクラス
//////////////////////////////////////////////////
class CKTEApp : public CWinApp
{
public:
	//コンストラクタ
	CKTEApp();
	//デストラクタ
	virtual ~CKTEApp();

	//アプリケーションエントリポイント
	virtual BOOL InitInstance();
	//アプリケーション終了ポイント
	virtual int ExitInstance();

	//設定ファイルの取得
	inline CSetting* GetSetting() const {assert(m_pSetting); return m_pSetting;}
	//プロファイルの取得
	inline CProfile* GetProfile() const {assert(m_pProfile); return m_pProfile;}
	//DirectShowアクセスクラスの所得
	inline CDirectShowAccess* GetDSA() const {assert(m_pDSA);return m_pDSA;}

	//メニューからの呼び出し
	afx_msg void OnSettingDialog();				//環境設定ダイアログを表示
	afx_msg void OnPluginDialog();				//プラグインダイアログを表示
	afx_msg void OnPerformanceSettingDialog();	//プロセッサ設定ダイアログを表示
	afx_msg void OnSettingDefault();			//設定を初期状態に戻す
	afx_msg void OnAboutDialog();				//バージョン情報ダイアログを表示
	afx_msg void OnProfileNew();				//プロファイルを新規作成
	afx_msg void OnProfileOpen();				//プロファイルを開く
	afx_msg void OnProfileSave();				//プロファイルを保存
	afx_msg void OnProfileSaveas();				//プロファイルを別名保存

	//クラスがメッセージマップを宣言することを定義する
	DECLARE_MESSAGE_MAP()

private:
	//////////////////////////////////////////////////
	//メンバ変数(ここから)
	CSetting* m_pSetting;			//設定(INI)へのアクセス
	CProfile* m_pProfile;			//プロファイルへのアクセス
	CDirectShowAccess* m_pDSA;		//DirectShowへのアクセスクラス

	HANDLE m_hMutexApp;				//複数起動抑制用のミューテックス
	BOOL   m_bEnablePriority;		//CPU優先度の変更したかどうか
	//////////////////////////////////////////////////
	//メンバ変数(ここまで)

	//サブルーチン
	BOOL OSVersionCheck();				//OSのバージョンチェックを行う
	BOOL ArgCheck1();					//引数のチェックを行う
	BOOL ArgCheck2();					//引数のチェックを行う
	BOOL SetMutex(BOOL bEnable);		//複数起動の設定を行う
	BOOL SetTopMost(BOOL bEnable);		//常に手前に表示する
	BOOL SetAffinityMask(BOOL bInit);	//プロセスのアフィニティマスクを設定する (CProfile保存)
public:
	BOOL SetPriorityClass(BOOL bEnable,DWORD dwPriorityClass);	//プロセスの優先度を設定する (CProfile保存)

};

//////////////////////////////////////////////////
//theAppは唯一のグローバル変数(グローバルクラス)
//////////////////////////////////////////////////
extern CKTEApp theApp;

//////////////////////////////////////////////////
//[EOF]