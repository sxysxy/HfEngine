[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Invoke-WebRequest -Uri 'https://github.com/sxysxy/HfEngine/releases/download/0.0/ruby260-HfEngine-dev.zip' -OutFile './ruby260-HfEngine-dev.zip'
Expand-Archive -Path "ruby260-HfEngine-dev.zip" -DestinationPath "./" -Force:$true

Invoke-WebRequest -Uri 'http://download.microsoft.com/download/F/1/7/F178BCE4-FA19-428F-BB60-F3DEE1130BFA/DXSDK_Feb10.exe' -OutFile 'DXSDK_Feb10.exe'
F178BCE4-FA19-428F-BB60-F3DEE1130BFA/DXSDK_Feb10.exe