#pragma once

#define CHECK_RESULT(str,hr) CDirectShowEtc::CheckResult(_T(str),hr,__FILE__,__LINE__)

//フィルタの情報構造体
struct FilterInfoStruct
{
	GUID guid;			//フィルタのGUID
	wstring strName;	//フィルタの名前
};

class CDirectShowEtc
{
public:
	//////////////////////////////////////////////////
	//メディアタイプ関連のサブルーチン
	//メディアタイプのフォーマットブロックを開放する
	static HRESULT ReleaseMediaType(AM_MEDIA_TYPE &amt);
	static HRESULT ReleaseMediaTypeList(vector<AM_MEDIA_TYPE> &MediaTypeList);

	//////////////////////////////////////////////////
	//デバッグ関連のサブルーチン
	//GRFファイルのセーブ
	static HRESULT SaveGraphFile(IGraphBuilder* pGraphBuilder,WCHAR *wszPath);
	//ROTテーブルに追記
	static HRESULT AddToRot(IGraphBuilder* pGraphBuilder,DWORD &dwRegister);
	static void RemoveFromRot(DWORD &dwRegister);
	//DirectShowの戻り値をチェックするサブモジュール
	static bool CheckResult(TCHAR* string,HRESULT hr,const char* pFile,int nLine);
	//デバッグウィンドウに出力
	static void OutputDebugWindow(LPCTSTR pString, ...);

	//////////////////////////////////////////////////
	//ピン関連のサブルーチン
	static IPin* GetPinByIndex(IBaseFilter* pFilter,int nPinIndex);
	static IPin* GetPinByName(IBaseFilter* pFilter,PIN_DIRECTION PinDir,LPCWSTR pPinName,const AM_MEDIA_TYPE* pAmt);
	//フィルタの接続チェック
	static BOOL CheckConnectionFilter(IBaseFilter* pFilter,int* pInputCount,int* pOutputCount);

	//////////////////////////////////////////////////
	//GUIDの文字列変換のサブルーチン
	static BOOL GUIDToString(TCHAR* szGUID,int nSize,const GUID &guid);
	static BOOL StringToGUID(LPTSTR szGUID,GUID &guid);
	static unsigned long MakeGUIDSubRoutine(LPTSTR szGUID,int nStart,int nLength);
	//メージャータイプの文字列変換
	static BOOL MakeStringToMajorTypeGUID(TCHAR* szGUID,int nSize,GUID &guid);
	static BOOL MakeStringToSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid);
	//ビデオサブタイプの文字列変換
	static BOOL MakeStringToVideoSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid);
	//オーディオサブタイプの文字列変換
	static BOOL MakeStringToAudioSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid);
	//ストリームサブタイプの文字列変換
	static BOOL MakeStringToStreamSubTypeGUID(TCHAR* szGUID,int nSize,GUID &guid);

	//フォーマットタイプの文字列変換
	static BOOL MakeStringToFormatTypeGUID(TCHAR* szGUID,int nSize,GUID &guid);

	//ダンプ出力
	static BOOL DumpMediaType(AM_MEDIA_TYPE* pamt,LPCTSTR lpszFileName);
	static BOOL DumpMediaType(AM_MEDIA_TYPE* pamt,FILE* fp);
	static BOOL DumpVideoStreamConfigCaps(VIDEO_STREAM_CONFIG_CAPS* pvscc,LPCTSTR lpszFileName);
	static BOOL DumpVideoStreamConfigCaps(VIDEO_STREAM_CONFIG_CAPS* pvscc,FILE* fp);
	static BOOL DumpAudioStreamConfigCaps(AUDIO_STREAM_CONFIG_CAPS* pascc,LPCTSTR lpszFileName);
	static BOOL DumpAudioStreamConfigCaps(AUDIO_STREAM_CONFIG_CAPS* pascc,FILE* fp);

	//すべてのフィルタの列挙
	static HRESULT EnumAllFilter(const GUID &guidCategory,vector<FilterInfoStruct> &FilterList);
	static HRESULT EnumAllDMOFilter(const GUID &guidCategory,vector<FilterInfoStruct> &FilterList);
	static HRESULT EnumMatchFilter(const GUID &guidMediaType,const GUID &guidMediaSubType,vector<FilterInfoStruct> &FilterList);

	//AM_MEDIA_TYPEからキャプチャ設定の文字列を生成するサブルーチン
	static BOOL MakeVideoMediaString(const vector<AM_MEDIA_TYPE> &MediaTypeList,vector<wstring> &strVideoSettingList);
	static BOOL MakeAudioMediaString(const vector<AM_MEDIA_TYPE> &MediaTypeList,vector<wstring> &strAudioSettingList);
	//AM_MEDIA_TYPEからダミーのAUDIO_STREAM_CONFIG_CAPSリストを作るサブルーチン
	static BOOL MakeDummyAudioCapsList(const vector<AM_MEDIA_TYPE> &MediaTypeList,vector<AUDIO_STREAM_CONFIG_CAPS> &AudioStreamConfigCapsList);

	//インストールされているフィルタの検索(フレンドリ名での検索)
	static HRESULT FindInstallFilter(wstring strFilterName,BOOL &bFind);
};
