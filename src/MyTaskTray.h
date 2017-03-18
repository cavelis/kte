#pragma once

//トレイアイコン管理用IDを定義（1には特に意味はない）
#define TRAY_ID        1

//自前のメッセージ番号を定義（100には特に意味はない）
#define NOTIFY_TRAYICON (WM_APP + 100)

//タスクトレイの管理クラス
class CMyTaskTray
{
public:
	CMyTaskTray(void);
	virtual ~CMyTaskTray(void);

	BOOL Init(CWnd* pWnd);
	BOOL Exit();
	BOOL NotifyIcon();
	BOOL SetIcon(int nIconID);

	LRESULT OnTrayIcon(WPARAM wParam,LPARAM lParam);

private:
	NOTIFYICONDATA  m_nid;		//アイコン登録の構造体
	BOOL m_bid;					//アイコン登録がされているかどうか
	CWnd* m_pWnd;				//登録したウィンドウハンドル
};

