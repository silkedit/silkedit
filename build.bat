if "%3"=="edge" (
  "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %1 & cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_EDGE=ON -G "NMake Makefiles" .. & jom %2
) else (
  "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %1 & cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" .. & jom %2
)
