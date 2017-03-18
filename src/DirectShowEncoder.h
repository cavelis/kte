#pragma once

#include "DirectShow.h"

//コーデック列挙に関する構造体
typedef enum {
  CODEC_FORMAT_CBR    = 0,
  CODEC_FORMAT_VBR    = 1
} CODEC_FORMAT_BITRATE;
typedef enum {
  CODEC_FORMAT_ONEPASS    = 0,
  CODEC_FORMAT_TWOPASS    = 1
} CODEC_FORMAT_NUMPASS;

struct CodecStruct{
	GUID guidType;				//取得時のmajorGUID名 (WMMEDIATYPE_Audio,WMMEDIATYPE_Video)
	wstring strName;			//コーデックの名前
	DWORD dwComplexityMax;		//コーデックの複雑さの最大値(0～この値まで)
	DWORD dwComplexityOffline;	//ローカルに保存する際の提唱最適値
	DWORD dwComplexityLive;		//ストリーミング配信する際の提唱最適値
	BOOL  bIsVBRSupported;		//VBRをサポートしているかどうか
	DWORD bIsTwoPassSupported;	//2パスエンコードをサポートしているかどうか
	DWORD dwFormatCounts[2][2];	//保持しているフォーマット数 dwFormatCounts[CODEC_FORMAT_BITRATE][CODEC_FORMAT_NUMPASS]
};
struct AudioCodecFormatStruct {
	wstring strDescription;		//フォーマットの説明文
	DWORD dwBitrate;			//ビットレート
	WORD  nChannels;			//チャネル数
	DWORD nSamplesPerSec;		//サンプリングレート(Hz) 8000,11025,22050,44100
	WORD  wBitsPerSample;		//ビット数(8,12,16)
	BOOL bAVSynchronization;	//A/V同期か否か
};

//////////////////////////////////////////////////
//エンコーダーに関するクラス
//////////////////////////////////////////////////
class CDirectShowEncoder
{
public:
	CDirectShowEncoder(void);
	virtual ~CDirectShowEncoder(void);
	
	//プロファイルの作成
	HRESULT Create();
	//プロファイルの削除
	void Delete();

	//プロファイルの適用
	HRESULT ApplyProfiles();
	//プロファイルの取得
	inline IWMProfile* GetProfiles() {assert(m_pWMProfile);return m_pWMProfile;}

	//プロファイルのファイルセーブ
	HRESULT SaveProfile(IWMProfile* pWMProfile,LPCWSTR pwszFileName);

	//プロファイルのロード
	HRESULT LoadProfile(LPCWSTR pwszFileName);

	HRESULT ListAllProfiles(WMT_VERSION dwVersion);					//OS定義のプロファイルの全列挙し表示
	HRESULT ListProfiles(IWMProfile* pWMProfile);					//プロファイルの列挙し表示
	HRESULT ListStreamConfig(IWMStreamConfig* pWMStreamConfig);		//ストリーミング設定の表示
	HRESULT ListAllCodec(const GUID guidType);						//コーデックの全列挙し表示

	//コーデックの全列挙しCodecStruct構造体に取得する
	HRESULT EnumCodec(const GUID guidType,vector<CodecStruct> &Codec);
	//オーディオコーデックフォーマットを列挙しAudioCodecFormatStruct構造体に取得する
	HRESULT EnumAudioCodecFormat(const DWORD dwCodecIndex,CODEC_FORMAT_BITRATE format_bitrate,CODEC_FORMAT_NUMPASS format_numpass,vector<AudioCodecFormatStruct> &AudioFormat);

	//オーディオコーデックの設定
	HRESULT SetAudioCodec(const DWORD dwCodecIndex,const DWORD dwFormatIndex,const CODEC_FORMAT_BITRATE format_bitrate,const CODEC_FORMAT_NUMPASS format_numpass){
		return SetCodec(true,dwCodecIndex,dwFormatIndex,format_bitrate,format_numpass);}
	//ビデオコーデックの設定
	HRESULT SetVideoCodec(DWORD dwCodecIndex,CODEC_FORMAT_BITRATE format_bitrate,CODEC_FORMAT_NUMPASS format_numpass){
		return SetCodec(false,dwCodecIndex,0,format_bitrate,format_numpass);}

	//独自のビデオコーデックを設定
	HRESULT SetOriginalCodec(const bool bAudio,AM_MEDIA_TYPE* pAmt);

	//ビデオコーデックの編集項目を設定
	HRESULT SetVideoSetting(const DWORD dwBitrate,const DWORD msBufferWindow,const REFERENCE_TIME AvgTimePerFrame,const RECT rcSource,const RECT rcTarget);
	HRESULT SetVideoSetting2(const DWORD dwQuality,const REFERENCE_TIME MaxKeyFrameSpacing);
	HRESULT SetVideoComplexity(const WORD wComplexity);

	//ビデオコーデック情報のファイルセーブ(デバッグ用)
	HRESULT SaveCurrentVideoCodec(wstring strFileName);

	//最大・最小パケットサイズの設定
	HRESULT SetPacketSize(const DWORD dwMinPacketSize, const DWORD dwMaxPacketSize);
	//合計ビットレートの取得
	HRESULT GetTotalBitrate(DWORD* pdwAudioBitrate,DWORD* pdwVideoBitrate,DWORD* pdwAddedBitrate,DWORD* pdwTotalBitrate);

private:
	//コーデックの選択をして設定を行うサブルーチン(Audio,Video)
	HRESULT SetCodec(const bool bAudio,const DWORD dwCodecIndex,const DWORD dwFormatIndex,CODEC_FORMAT_BITRATE format_bitrate,CODEC_FORMAT_NUMPASS format_numpass);

	IWMProfile* m_pWMProfile;				//プロファイル情報
	IWMStreamConfig* m_pVideoStreamConfig;	//ビデオストリーミングの情報
	IWMStreamConfig* m_pAudioStreamConfig;	//オーディオストリーミングの情報

	BOOL MakeStringToGUID(TCHAR* szGUID,int nSize,GUID &guid);
};

/*
//■VBRに関する設定
http://msdn.microsoft.com/en-us/library/dd756995%28VS.85%29.aspx

//■帯域幅占有
IWMBandwidthSharing* pWMBandwidthSharing = NULL;
hr = CreateNewBandwidthSharing();
よくわからないけど帯域幅を占有宣言することで、
優先的に帯域が割り当てられるのかもしれない

*/