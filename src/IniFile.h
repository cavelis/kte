#pragma once

//////////////////////////////////////////////////
//INIファイルを読み書きするクラス
//////////////////////////////////////////////////
class CIniFile
{
public:
	//コンストラクタ
	CIniFile(void);
	//デストラクタ
	virtual ~CIniFile(void);

	//INIファイルを開く
	BOOL Open(LPCTSTR strProfileName,CWinApp* pWinApp = NULL);
	//INIファイルを閉じる
	void Close();
	//ファイルの存在の確認
	BOOL IsFind() const {return m_bFind;}

	//INIファイルの読み込みと書き込み
	BOOL WriteProfileInt(LPCTSTR lpszSection,LPCTSTR lpszEntry,int nValue);
	BOOL WriteProfileBOOL(LPCTSTR lpszSection,LPCTSTR lpszEntry,BOOL bValue);
	BOOL WriteProfilebool(LPCTSTR lpszSection,LPCTSTR lpszEntry,bool bValue);
	BOOL WriteProfileFloat(LPCTSTR lpszSection,LPCTSTR lpszEntry,float fValue);
	BOOL WriteProfileDouble(LPCTSTR lpszSection,LPCTSTR lpszEntry,float dValue);
	BOOL WriteProfileString(LPCTSTR lpszSection,LPCTSTR lpszEntry,LPCTSTR lpszValue);
	BOOL WriteProfileGUID(LPCTSTR lpszSection,LPCTSTR lpszEntry,GUID guidValue);

	int GetProfileInt(LPCTSTR lpszSection,LPCTSTR lpszEntry,int nDefault);
	BOOL GetProfileBOOL(LPCTSTR lpszSection,LPCTSTR lpszEntry,BOOL bDefault);
	bool GetProfilebool(LPCTSTR lpszSection,LPCTSTR lpszEntry,bool bDefault);
	CString GetProfileString(LPCTSTR lpszSection,LPCTSTR lpszEntry,LPCTSTR lpszDefault = NULL);
	double GetProfileDouble(LPCTSTR lpszSection,LPCTSTR lpszEntry,double dDefault);
	float GetProfileFloat(LPCTSTR lpszSection,LPCTSTR lpszEntry,float fDefault);
	BOOL GetProfileGUID(LPCTSTR lpszSection,LPCTSTR lpszEntry,GUID guidDefault,GUID &guidValue);

private:
	CWinApp* m_pWinApp;	//WinAppクラスのポインタ
	BOOL m_bFind;		//ファイルの存在の有無
	CString m_strFullPath;		//INIの絶対パス
	CString m_strRelativePath;	//INIの相対パス

	//現在のパスを設定
	BOOL SetCurrentProfilePath(LPCTSTR lpszFileName);
};
