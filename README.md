# FBXConvertToBinary

##用途
FBXファイルをFBXSDKを用いて解析し、その結果をバイナリファイルに出力する。
結果はRenderingDemoRebuiltで利用する。

##効果
RenderingDemoRebuiltでバイナリファイルの読み込みのみにした結果、起動の際のFBX解析が不要になったことで
起動時間が体感9割早くなった。(8秒→1秒未満)
