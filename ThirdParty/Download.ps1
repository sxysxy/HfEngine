[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 #

if(!(Test-Path -Path "SDL2-2.0.9")) {
    if(!(Test-Path -Path "SDL2-2.0.9.zip")) {
        Invoke-WebRequest -Uri 'https://github.com/sxysxy/HfEngine/releases/download/0.0/SDL2-2.0.9.zip' -OutFile "SDL2-2.0.9.zip"
    }
    Expand-Archive -Path "SDL2-2.0.9.zip" -DestinationPath "./" -Force:$true
}

if(!(Test-Path -Path "ruby260-lib")) {
    if(!(Test-Path -Path "ruby260-HfEngine-dev.zip")) {
        Invoke-WebRequest -Uri 'https://github.com/sxysxy/HfEngine/releases/download/0.0/ruby260-HfEngine-dev.zip' -OutFile './ruby260-HfEngine-dev.zip'
    }
    Expand-Archive -Path "ruby260-HfEngine-dev.zip" -DestinationPath "./" -Force:$true
}

if(!($env:DXSDK_DIR)) {
    if(!(Test-Path -Path "DXSDK_Feb10.exe")) {
        Invoke-WebRequest -Uri 'http://download.microsoft.com/download/F/1/7/F178BCE4-FA19-428F-BB60-F3DEE1130BFA/DXSDK_Feb10.exe' -OutFile 'DXSDK_Feb10.exe'
    }
    .\DXSDK_Feb10.exe
}