*DSFilters*
=============================================================================
## 概要

DirectShow Filterライブラリ。
複数のDirectShowのフィルタを実装しています。
ビルド後のファイルは
- DSFilter.ax
  Releaseビルド/x86
- DSFilterx64.ax
  Releaseビルド/x64
- DSFilterd.ax
  Debugビルド/x86
- DSFilterdx64.ax
  Debugビルド/x64


## フィルター概要
- CBaseMuxFilter
  合成処理を行うベースフィルタ。
  2つの入力ピン(Master/Slave)と1つの出力ピンを持つ。
  継承して使う。
  - ReceiveSlave
    Slave側のピンからの入力を受け取る。
    具象クラスではこのデータをバッファリングする。

- CVideoResizer
  ビデオのサイズを固定サイズに変更するフィルタ。
  - MEDIASUBTYPE_RGB8
  - MEDIASUBTYPE_RGB565
  - MEDIASUBTYPE_RGB555
  - MEDIASUBTYPE_ARGB1555
  - MEDIASUBTYPE_ARGB4444
  - MEDIASUBTYPE_RGB24
  - MEDIASUBTYPE_RGB32
  - MEDIASUBTYPE_ARGB32
  - MEDIASUBTYPE_A2R10G10B10
  -	MEDIASUBTYPE_A2B10G10R10
  をサポート

- CVideoMux
  2つのビデオを横に繋げて一つのビデオにします。
  2つのビデオのサイズはCVideoResizerなどを使って揃えて下さい。


## その他
- CDbgWnd
  デバッグ用の情報を出力するウィンドウ

- CRingBuffer
  固定サイズのバッファをリングバッファとして作成・管理するクラス。

- CSourceStreamEx
  ソースフィルタのプッシュピンの拡張。
  クロックに合わせてデータを出力する。
  簡単なクオリティーコントロール処理も行う。

- CToggleBuffer
  固定サイズのバッファを2つ使い交互に利用するバッファを作成・管理するクラス。

- CMediaSampleMonitor
  変換フィルターとしての機能はなく、通過するメディアサンプルの情報を表示します。
  メディアサンプルの内容が見たいときに間に挟む形で繋ぎます。


## 作りかけ

- CAudioMux
  音声の合成を行うフィルター



-----------------------------------------------------------------------------
## ビルド環境について

 Windows Software Development Kit (SDK) for Windows Server 2008 and .NET Framework 3.5
 DirectXのSDKを利用するので上記のSDKをインストールする必要があります。
 また、BaseClassesを利用するので、
 %PROGRAMFILES%\Microsoft SDKs\Windows\v6.1\Samples\Multimedia\DirectShow\BaseClasses
 のサンプルをビルドしておいてください。

 ※ 私の環境ではビルドはVisual Studio 2008で行っています。

-----------------------------------------------------------------------------
## サンプルについて

 Visual Studio 2008 / C#でのサンプルプログラム

-----------------------------------------------------------------------------
## ライセンス

 MITライセンス

-----------------------------------------------------------------------------
## リリースノート


