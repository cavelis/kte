#pragma once
#include "afxext.h"

//自作ツールバークラス
class CMyToolBar : public CToolBar
{
protected:
	//DECLARE_DYNAMIC(CToolBar)
	//クラスがメッセージマップを宣言することを定義する
	//DECLARE_MESSAGE_MAP()

public:
	//コンストラクタ
	CMyToolBar(void);
	//デストラクタ
	~CMyToolBar(void);

	//ツールバーの作成
	BOOL Create(CFrameWnd* pFrameWnd);
	//ボタンを押した状態にする
	BOOL PressButton(int nID,BOOL bPress);
	//エンコードの開始と終了
	BOOL Encode(BOOL bStart);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
