#include "StdAfx.h"
#include "IniFile.h"

//////////////////////////////////////////////////
//コンストラクタ
//////////////////////////////////////////////////
CIniFile::CIniFile(void)
{
	m_pWinApp = NULL;
	m_bFind = FALSE;

	m_strFullPath = _T("");
	m_strRelativePath = _T("");
}
//////////////////////////////////////////////////
//デストラクタ
//////////////////////////////////////////////////
CIniFile::~CIniFile(void)
{
	if (m_pWinApp)
		Close();
}
//////////////////////////////////////////////////
//INIファイルを開く
//INIファイル名は絶対パスの場合と相対パスの場合がある
//////////////////////////////////////////////////
BOOL CIniFile::Open(LPCTSTR strProfileName,CWinApp* pWinApp)
{
	if (!strProfileName)
		return FALSE;

	if (!SetCurrentProfilePath(strProfileName))
		return FALSE;

	//INIファイル名の確認
	CFileFind find;
	if (find.FindFile(m_strFullPath))
		m_bFind = TRUE;
	else
		m_bFind = FALSE;

	if (pWinApp)
		m_pWinApp = pWinApp;
	else
		m_pWinApp = AfxGetApp();

	//規定のINIプロファイルに設定
	if (m_pWinApp->m_pszProfileName)
	{
		delete ((void*)m_pWinApp->m_pszProfileName);
		m_pWinApp->m_pszProfileName = NULL;
	}
	m_pWinApp->m_pszProfileName = _tcsdup(m_strFullPath);
	return TRUE;
}
//////////////////////////////////////////////////
//INIファイルを閉じる
//////////////////////////////////////////////////
void CIniFile::Close()
{
	if (m_pWinApp)
	{
		if (m_pWinApp->m_pszProfileName)
		{
			delete ((void*)m_pWinApp->m_pszProfileName);
			m_pWinApp->m_pszProfileName = NULL;
		}
	}
	m_pWinApp = NULL;
	m_bFind = FALSE;
	m_strFullPath = _T("");
	m_strRelativePath = _T("");
}

//////////////////////////////////////////////////
//現在のパスを取得する
//////////////////////////////////////////////////
BOOL CIniFile::SetCurrentProfilePath(LPCTSTR lpszFileName)
{
	if (lpszFileName == NULL)
	{
		m_strFullPath = _T("");
		m_strRelativePath = _T("");
		return FALSE;
	}

	TCHAR szNewPath	[_MAX_PATH];
	TCHAR szPath	[_MAX_PATH];
	TCHAR szDrive	[_MAX_DRIVE];
	TCHAR szDir		[_MAX_DIR];
	TCHAR szFileName[_MAX_FNAME];
	TCHAR szExt		[_MAX_EXT];
	//フルパスかどうかの判定
	CString strPath = lpszFileName;
	CString strDot = strPath.Mid(1,2);
	if (strDot.Compare(_T(":\\")) == 0)
	{
		//フルパス
		_tsplitpath_s(szNewPath,szDrive,_MAX_DRIVE,szDir,_MAX_DIR,szFileName,_MAX_FNAME,szExt,_MAX_EXT);
		_stprintf_s(szNewPath,_MAX_PATH,_T("%s%s"),szFileName,szExt);
		m_strFullPath = lpszFileName;
		m_strRelativePath = szNewPath;
	} else {
		//相対パス
		::GetModuleFileName(NULL,szPath,_MAX_PATH);
		_tsplitpath_s(szPath,szDrive,_MAX_DRIVE,szDir,_MAX_DIR,szFileName,_MAX_FNAME,szExt,_MAX_EXT);
		_stprintf_s(szNewPath,_MAX_PATH,_T("%s%s%s"),szDrive,szDir,lpszFileName);
		m_strFullPath = szNewPath;
		m_strRelativePath = lpszFileName;
	}
	return TRUE;
}



//////////////////////////////////////////////////
//プロファイルのセーブ/ロードで使う関数
//////////////////////////////////////////////////
BOOL CIniFile::WriteProfileInt(LPCTSTR lpszSection,LPCTSTR lpszEntry,int nValue)
{
	ASSERT(m_pWinApp);
	return m_pWinApp->WriteProfileInt(lpszSection,lpszEntry,nValue);
}
BOOL CIniFile::WriteProfileBOOL(LPCTSTR lpszSection,LPCTSTR lpszEntry,BOOL bValue)
{
	ASSERT(m_pWinApp);
	return m_pWinApp->WriteProfileString(lpszSection,lpszEntry,(bValue)? _T("TRUE"):_T("FALSE"));
}
BOOL CIniFile::WriteProfilebool(LPCTSTR lpszSection,LPCTSTR lpszEntry,bool bValue)
{
	ASSERT(m_pWinApp);
	return m_pWinApp->WriteProfileString(lpszSection,lpszEntry,(bValue)? _T("true"):_T("false"));
}
BOOL CIniFile::WriteProfileFloat(LPCTSTR lpszSection,LPCTSTR lpszEntry,float fValue)
{
	ASSERT(m_pWinApp);
	TCHAR strString[MAX_PATH];
	_stprintf_s(strString,MAX_PATH,_T("%f"),fValue);
	return m_pWinApp->WriteProfileString(lpszSection,lpszEntry,strString);
}
BOOL CIniFile::WriteProfileDouble(LPCTSTR lpszSection,LPCTSTR lpszEntry,float dValue)
{
	ASSERT(m_pWinApp);
	TCHAR strString[MAX_PATH];
	_stprintf_s(strString,MAX_PATH,_T("%f"),dValue);
	return m_pWinApp->WriteProfileString(lpszSection,lpszEntry,strString);
}
BOOL CIniFile::WriteProfileString(LPCTSTR lpszSection,LPCTSTR lpszEntry,LPCTSTR lpszValue)
{
	ASSERT(m_pWinApp);
	return m_pWinApp->WriteProfileString(lpszSection,lpszEntry,lpszValue);
}
BOOL CIniFile::WriteProfileGUID(LPCTSTR lpszSection,LPCTSTR lpszEntry,GUID guidValue)
{
	ASSERT(m_pWinApp);
	CString strValue;
	strValue.Format(_T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
			guidValue.Data1,
			guidValue.Data2,
			guidValue.Data3,
			guidValue.Data4[0],
			guidValue.Data4[1],
			guidValue.Data4[2],
			guidValue.Data4[3],
			guidValue.Data4[4],
			guidValue.Data4[5],
			guidValue.Data4[6],
			guidValue.Data4[7]);
	return m_pWinApp->WriteProfileString(lpszSection,lpszEntry,strValue);
}


//////////////////////////////////////////////////
int CIniFile::GetProfileInt(LPCTSTR lpszSection,LPCTSTR lpszEntry,int nDefault)
{
	ASSERT(m_pWinApp);
	return (int)m_pWinApp->GetProfileInt(lpszSection,lpszEntry,nDefault);
}
BOOL CIniFile::GetProfileBOOL(LPCTSTR lpszSection,LPCTSTR lpszEntry,BOOL bDefault)
{
	ASSERT(m_pWinApp);
	CString strName = m_pWinApp->GetProfileString(lpszSection,lpszEntry,NULL);
	if (strName.IsEmpty()) return bDefault;
	if (strName == _T("TRUE") || strName == _T("True") || strName == _T("true")) return TRUE;
	return FALSE;
}
bool CIniFile::GetProfilebool(LPCTSTR lpszSection,LPCTSTR lpszEntry,bool bDefault)
{
	return (GetProfileBOOL(lpszSection,lpszEntry,bDefault ? TRUE : FALSE) ? TRUE : FALSE);
}
CString CIniFile::GetProfileString(LPCTSTR lpszSection,LPCTSTR lpszEntry,LPCTSTR lpszDefault)
{
	ASSERT(m_pWinApp);
	return m_pWinApp->GetProfileString(lpszSection,lpszEntry,lpszDefault);
}
double CIniFile::GetProfileDouble(LPCTSTR lpszSection,LPCTSTR lpszEntry,double dDefault)
{
	ASSERT(m_pWinApp);
	CString strName = m_pWinApp->GetProfileString(lpszSection,lpszEntry,NULL);
	if (strName.IsEmpty()) return dDefault;
	double dRet =  _tstof((LPCTSTR)strName);
	return dRet;
}
float CIniFile::GetProfileFloat(LPCTSTR lpszSection,LPCTSTR lpszEntry,float fDefault)
{
	return (float)GetProfileDouble(lpszSection,lpszEntry,(double)fDefault);
}

BOOL CIniFile::GetProfileGUID(LPCTSTR lpszSection,LPCTSTR lpszEntry,GUID guidDefault,GUID &guidValue)
{
	ASSERT(m_pWinApp);
	CString strValue = m_pWinApp->GetProfileString(lpszSection,lpszEntry,NULL);
	if (strValue.IsEmpty() == TRUE) {
		guidValue = guidDefault;
	} else {
		CString strBuffer;

		//GUID文字列の分析
		strBuffer = strValue.Mid(1,8);
		guidValue.Data1 = (unsigned long)_tcstoul(strBuffer,NULL,16);

		strBuffer = strValue.Mid(10,4);
		guidValue.Data2 = (unsigned short)_tcstoul(strBuffer,NULL,16);

		strBuffer = strValue.Mid(15,4);
		guidValue.Data3 = (unsigned short)_tcstoul(strBuffer,NULL,16);

		strBuffer = strValue.Mid(20,2);
		guidValue.Data4[0] = (unsigned char)_tcstoul(strBuffer,NULL,16);
		strBuffer = strValue.Mid(22,2);
		guidValue.Data4[1] = (unsigned char)_tcstoul(strBuffer,NULL,16);

		strBuffer = strValue.Mid(25,2);
		guidValue.Data4[2] = (unsigned char)_tcstoul(strBuffer,NULL,16);
		strBuffer = strValue.Mid(27,2);
		guidValue.Data4[3] = (unsigned char)_tcstoul(strBuffer,NULL,16);
		strBuffer = strValue.Mid(29,2);
		guidValue.Data4[4] = (unsigned char)_tcstoul(strBuffer,NULL,16);
		strBuffer = strValue.Mid(31,2);
		guidValue.Data4[5] = (unsigned char)_tcstoul(strBuffer,NULL,16);
		strBuffer = strValue.Mid(33,2);
		guidValue.Data4[6] = (unsigned char)_tcstoul(strBuffer,NULL,16);
		strBuffer = strValue.Mid(35,2);
		guidValue.Data4[7] = (unsigned char)_tcstoul(strBuffer,NULL,16);
	}
	return TRUE;
}
