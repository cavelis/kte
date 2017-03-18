#pragma once

#include "IniFile.h"

#define SETTING_INI_FILE _T("Setting.ini")

//////////////////////////////////////////////////
//設定データの読み書きを行うクラス
//プログラム起動時にファイル読み込みし、プログラム終了時にファイル保存を行うのみ
//こちらはプロファイルと違って1つしか存在しない(Setting.ini)
//////////////////////////////////////////////////

class CSetting
{
public:
	CSetting(void);				//コンストラクタ
	virtual ~CSetting(void);	//デストラクタ

	void DefaultSetting();						//設定項目に初期値に設定
	BOOL LoadSetting();							//設定データの読み込み
	BOOL SaveSetting();							//設定データの書き込み
	void DefaultProfilePath();					//プロファイルのパスを規定のパスにする
	BOOL SetProfilePath(LPCTSTR lpszFileName);	//プロファイルのパスを設定する

	//////////////////////////////////////////////////
	//様々な設定項目はここに書く
	//////////////////////////////////////////////////

	//全体設定
	BOOL m_bEnableIniFile;			//INIファイルの存在の有無
	BOOL m_bSaveIniFile;			//INIファイルを終了時にセーブするか否か
	CString m_strProgramName;		//プログラム名   ("KoToEncoder")
	int m_nVersion;					//バージョン情報 (20100125)

	//プロファイルの情報
	CString m_strProfileShortPath;	//ロード時に読み込むプロファイル名(ショートパス)
	CString m_strProfileFullPath;	//ロード時に読み込むプロファイル名(フルパス)

	//画面設計
	CRect m_rcMainWindow;			//メインウィドウの座標
	int m_nMainSplitterWidth[2];	//メインの左右分割状態
	int m_nMainSplitterHeight[2];	//メインの上下分割状態
	int m_nMainSplitterPreview[2];	//メインのプレビュー部分の分割状態

	BOOL m_bShowToolBar;			//ツールバーを表示するか否か
	BOOL m_bShowStatusBar;			//ステータスバーを表示するか否か
	BOOL m_bShowPropertyWindow;		//プロパティウィンドウを表示するか否か
	BOOL m_bShowMonitorWindow;		//モニターウィンドウを表示するか否か
	BOOL m_bShowPreviewWindow;		//プレビューウィンドウを表示するか否か
	BOOL m_bShowVolumeWindow;		//ボリュームウィンドウを表示するか否か

	int m_nPropertyTabIndex;		//プロパティタブの表示インデックス
	int m_nMonitorTabIndex;			//モニタータブの表示インデックス
	BOOL m_bShowDebugWindow;		//デバッグウィンドウを表示するか否か

	//環境設定
	BOOL m_bMiniTaskBar;			//最小化時にタスクバーに格納するか否か
	BOOL m_bCreateMutex;			//複数起動を拒否するかどうか
	BOOL m_bShowTopMost;			//常に最前面に表示するかどうか
	CString m_strVideoPlayerPath;	//外部プレイヤーのパス
	CString m_strGetGlobalIPURL;	//グローバルIPアドレスを取得するURL
	BOOL m_bGetGlobalIPCheck;		//グローバルIPアドレスを取得する際に確認をする
	CString m_strGlobalIPAddress;	//グローバルIPアドレス
	CString m_strGlobalHostName;	//グローバルホスト名
	BOOL m_bEnumAllPlugin;			//プラグインの全列挙

	//プレビューウィンドウの設定
	double m_dPreviewChildTimes;	//子プレビューウィンドウの倍率
	BOOL m_bFixByWindow;			//ウィンドウサイズに拡大縮小倍率を合わせる
	BOOL m_bFixByClient;			//拡大縮小倍率にウィンドウサイズを合わせる
	BOOL m_bPreviewShow;			//プレビューを表示するかどうか
	BOOL m_bPreviewForceOff;		//プレビューを強制的に切る設定
	int  m_nPreviewVMR;				//プレビューのビデオミキシングレンダラの選択(0,1,7,9,10の選択)
	BOOL m_bPreviewVMROverlay;		//プレビューのビデオミキシングレンダラのオーバーレイ確保の有無

	//ボリュームウィンドウの設定
	BOOL m_bEnableAudioPreview;		//音量メーターが有効かどうか

	//モニターウィンドウ(統計タブ)の設定
	int m_nMonitorUpdateTimer;		//統計タイマーの間隔(ミリ秒)

	//モニターウィンドウ(ブロードキャストタブ)の設定
	vector<int> m_nMonitorPullColumnWidthList;

	//モニターウィンドウ(イベントログ)の設定
	BOOL m_bEventLogAutoSave;			//ログの自動保存
	CString m_strEventLogAutoSavePath;	//ログの保存パス

	//////////////////////////////////////////////////
	//設定の変数一覧ここまで
};
