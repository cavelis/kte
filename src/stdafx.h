
// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Windows ヘッダーから使用されていない部分を除外します。
#endif

//OSのバージョンによってヘッダの設定を変えれる
#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 一部の CString コンストラクターは明示的です。

// 一般的で無視しても安全な MFC の警告メッセージの一部の非表示を解除します。
#define _AFX_ALL_WARNINGS

#include <locale.h>


#include <afxwin.h>         // MFC のコアおよび標準コンポーネント
#include <afxext.h>         // MFC の拡張部分

#include <afxinet.h>

#include <afxmt.h>			// MFC (クリティカルセクション)

#include <afxdisp.h>        // MFC オートメーション クラス



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC の Internet Explorer 4 コモン コントロール サポート
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC の Windows コモン コントロール サポート
#endif // _AFX_NO_AFXCMN_SUPPORT


#include <afxsock.h>            // MFC のソケット拡張機能


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#define _WIN32_DCOM 
#pragma warning(disable : 4995)

//#include <stdio.h>
//#include <tchar.h>

//#include <windows.h>		//ウィンドウズの標準ヘッダ
//#include <atlbase.h>		//ATLのヘッダ

//GetVersionExの代替
#include <VersionHelpers.h>

//標準
#include <assert.h>
#include <tchar.h>

//STL
#include <vector>
#include <list>
#include <sstream>
//#include <algorithm>
//#include <functional>
using namespace std;

//DirectShowヘッダ
#include <dshow.h>
#include <Dvdmedia.h>

//Direct3Dヘッダ
#include <D3d9.h>
#include <Vmr9.h>

// for DMO, WMV, WMA
#include <dmodshow.h>
#include <dmoreg.h>
#include <wmcodecdsp.h>
#pragma comment(lib, "Dmoguids.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")


#pragma comment(lib,"Msdmo.lib")


//Windows Media Format SDK 11
#include <wmsdk.h>		//Windows Media Format 本体
#include <dshowasf.h>	//WMFとDirectShowの連携
#include <wmsysprf.h>	//エンコード設定に関するデフォルトのGUID

//ライブラリファイル
#pragma comment(lib,"strmiids.lib")	// クラス識別子 (CLSID) とインターフェイス識別子 (IID) エクスポートする。すべての DirecShow アプリケーションにはこのライブラリが必要。
#pragma comment(lib,"quartz.lib")	// AMGetErrorText 関数をエクスポートする。この関数を呼び出さない場合、このライブラリは必要ない。
#pragma comment(lib,"Wmvcore.lib")

#pragma comment(lib,"Wmvcore.lib")

#pragma comment(lib, "winmm.lib")		//ウィンドウズマルチメディアライブラリ
//#pragma comment(lib, "comctl32.lib")
//#pragma comment(lib, "avrt.lib")


//DirectShow基底クラスライブラリ
#include <wxdebug.h>	//DirectShowデバッグライブラリ
//#include <streams.h>
#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#pragma comment(lib, "strmbasd.lib")
//#pragma comment(lib, "msvcrtd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "strmbase.lib")
//#pragma comment(lib, "msvcrt.lib")
#endif

//GUIDのヘルパクラス
#include <rpc.h>
#pragma comment(lib ,"rpcrt4.lib")


//パフォーマンスデータヘルパー(PDH)
#include <pdh.h>
#pragma comment (lib, "pdh.lib")

//USBの接続を検出
#include <Dbt.h>

#ifdef _DEBUG
#ifdef DEBUG
#define DEBUG
#endif
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != NULL)        \
   {                    \
      x->Release();     \
      x = NULL;         \
   }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
   if(x != NULL)             \
   {                         \
      delete[] x;            \
      x = NULL;              \
   }
#endif


//#include <qedit.h>	//DirectShow SampleGrabberのヘッダ (要DirectX SDK)


#include <Objbase.h>