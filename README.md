# カスタム版 M5Stack用 MicroPython

## 概要

* 公式M5Stack MicroPythonをローカルPCで開発しやすいように構成を調整したもの
* [オリジナルのREADME](README.original.md)

## 特徴

* ESP-IDFのバージョンを更新しやすいようにsubmoduleで取得するように変更
    * v3.1ベースに更新済み
* パーティション構成をArduino core for ESP32の`default_16MB`と同じ構成に変更
* SD-Updaterによる書き込みに対応
    * 起動時に`Button A`を押しっぱなしにして `app0` への切り替えを行う。
* [M5Stack_LovyanLauncher](https://github.com/lovyan03/M5Stack_LovyanLauncher) で設定したWi-Fiクライアント接続設定をNVSから取得して使用するように変更
    * NVS読み取り用モジュール (`nvs`) を追加

## 課題

* ESP-IDFの更新時に LoBo版で変更されていたI2Cモジュールへの修正を破棄してIDFのものにもどしたため、ハードウェアI2Cが使えません。
    * 代わりにオリジナルのMicroPythonに存在したソフトウェアI2Cを有効化
    * 個人的にはハードウェアI2Cはいろいろバグがあり、ソフトウェアI2Cでいいのではないかと思っているので、いまのところ対応する予定はありません。
* MicroPythonのベースバージョンは1.9.4のまま
    * 1.10.0に上げたいところ
    * LoBo版はスレッド周りにかなり手が加えられていて更新が結構辛そう

## 使い方

### 必要なもの
* Flashが16MBのモデルのM5Stack
* SDカード
* MicroPythonビルド済みバイナリ
    * [このディレクトリ](MicroPython_BUILD/firmware/sdupdater/) の内容をダウンロードしておきます。
### 手順

1. [らびやんランチャー](https://github.com/lovyan03/M5Stack_LovyanLauncher) を書き込みます。
    * LovyanLauncher-vx.x.x for 16MB model を選んで16MBモデル用の構成で書き込むこと
2. らびやんランチャーでWi-Fi接続設定をする。
3. SDカード用のFTPサーバーを立ち上げます。
4. (3)で立ち上げたFTPサーバーに接続し、[MicroPython.bin](MicroPython_BUILD/firmware/sdupdater/MicroPython.bin) をコピーします。
5. SPIFFS用のFTPサーバーを立ち上げます。
6. (5)で立ち上げたFTPサーバーに接続し、[main.py](MicroPython_BUILD/firmware/sdupdater/main.py) をコピーします。
7. SDUpdaterでMicroPython.binを書き込みます。
8. 書き込みが終わったらMicroPythonが動く状態になっています。らびやんランチャーに戻すには、`Button A`を押しっぱなしにしながらリセットボタンを押します。画面が白く光ったら`Button A`から指を離します。

