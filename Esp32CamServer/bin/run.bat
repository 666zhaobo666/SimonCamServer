:utf-8 zh
cd ..
chcp 65001
echo "在此下载jdk17 https://learn.microsoft.com/zh-cn/java/openjdk/download"
: 该代码经过微软jdk测试 https://learn.microsoft.com/zh-cn/java/openjdk/download
: 指定jdk17(jre17)java文件的全路径
set java="C:\Program Files\Microsoft\jdk-21.0.1.12-hotspot\bin\java"
 %java% -Ddb.path=db -Djdk.crypto.KeyAgreement.legacyKDF=true -jar .\lib\simonCAM-server-1.0.0.jar
pause