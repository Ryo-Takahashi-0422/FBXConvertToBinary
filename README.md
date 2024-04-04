# FBXConvertToBinary

## 用途
FBXファイルをFBXSDKを用いて解析し、その結果をバイナリファイルに出力する。
結果はRenderingDemoRebuiltで利用する。

## 背景
RenderingDemoRebuiltでは起動時に毎回FBXファイルを解析して頂点データ(位置、インデクス、マテリアル情報、アニメーション情報など)を抽出していた。
この処理が長かったため(8秒前後)改善することとした。

## 使用ソフトウェア
- Visual Studio Community 2019
- Binary Editor BZ 1.9.8.7

## 効果
RenderingDemoRebuiltでバイナリファイルの読み込みのみにした結果、起動の際のFBX解析が不要になったことで
起動時間が体感9割早くなった。(8秒→1秒未満)
