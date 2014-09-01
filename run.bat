qmake -spec win32-msvc2013 -tp vc
"C:\Program Files\MSBuild\12.0\Bin\MSBuild.exe" sk.vcxproj /p:configuration=release

mkdir release\sk_portable
cp release\sk.exe release\sk_portable\sk.exe
windeployqt --release release\sk_portable\sk.exe

rem create an installer
"C:\Program Files\Inno Setup 5\compil32" /cc C:\Users\Shinichi\Code\sk\sk.iss

pause
