# KoToEncoder

配信に特化したWME互換クライアントです。  
ソフトの説明は、[README.txt](README.txt)を参照してください。

---

### ビルド方法


**(1) [Microsoft Windows SDK for Windows 7 and .NET Framework 4](https://www.microsoft.com/en-us/download/details.aspx?id=8279)のインストール  **


※必要なのは、[Windows Native Code Development]の[Samples]のみ。  
※インストールに失敗する場合は下記URLを参考に。  
  [Windows SDK の Windows 7 と、.NET Framework 4 をインストールするときにエラーが発生します。](https://support.microsoft.com/ja-jp/help/2934068/error-occurs-when-you-install-windows-sdk-for-windows-7-and-the-.net-framework-4)


**(2) [Visual Studio 2017 Community](https://www.visualstudio.com/)のインストール**


**(3) DirectShow基底クラスのビルド**  

(1)をインストールした際に入る、DirectShow基底クラスの開き、ビルドを行う。  
 *C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\baseclasses.sln*

※DLL依存させないように、静的リンクに変更してから、ビルドする。  
　[プロジェクト]⇒[プロパティ]⇒[C/C++]⇒「コード生成」⇒「ランタイム ライブラリ」  

 | d/r        | Platform    | ランタイム ライブラリ         |
 |:-----------|:------------|:------------------------------|
 | Debug      | Win32       | マルチスレッドデバッグ (/MTd) |
 | Release    | Win32       | マルチスレッド (/MT)          |
 | Debug      | x64         | マルチスレッドデバッグ (/MTd) |
 | Release    | x64         | マルチスレッド (/MT)          |  

※MBCSは使ってないので、こちらはビルド不要です。

**(4) KTEのビルド**  

.\KTE.slnを開き、全ビルドをする。実行ファイルが下記パスに出力される。  
32ビット版 .\Release\KTE.exe  
64ビット版 .\x64\Release\KTE.exe  
実行ファイル単体で起動できます。  
配付の際はREADME.txtを付けてください。  

---

Copyright (c) 2017 cavelis  
This software is released under the MIT License, see LICENSE text files.
