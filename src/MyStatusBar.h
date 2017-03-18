#pragma once
#include "afxext.h"

//////////////////////////////////////////////////
//自作ステータスバー
//////////////////////////////////////////////////
class CMyStatusBar : public CStatusBar
{
public:
	//コンストラクタ
	CMyStatusBar(void);
	//デストラクタ
	virtual ~CMyStatusBar(void);

	//ステータスバーを作成
	virtual BOOL Create(CFrameWnd* pFrameWnd);

	//ステータスバーに文字列を書き込む
	BOOL SetString(LPCTSTR lpszText);
	//ステータスバーに転送量を書き込む
	BOOL SetTransform(LPCTSTR lpszText);

	//ステータスバーの再生/停止状態を更新する
	BOOL SetPlayStatus(BOOL bPlay);

	//ステータスバーに拡大縮小倍率を書き込む
	BOOL SetTimes(double dTimes);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL DestroyWindow();

public:
	//エンコードの開始と終了(プレ処理)
	BOOL PreEncode(BOOL bStart);
	//エンコードの開始と終了
	BOOL Encode(BOOL bStart);

private:
	//メンバ変数
	PDH_HQUERY m_hQuery;		//高精度パフォーマンスカウンタのハンドル
	HCOUNTER   m_hCounter;		//高精度パフォーマンスカウンタ
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};
