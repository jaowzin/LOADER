@echo off
setlocal enabledelayedexpansion
if "%ANDROID_NDK_HOME%"=="" (
  if not "%ANDROID_NDK_ROOT%"=="" set ANDROID_NDK_HOME=%ANDROID_NDK_ROOT%
)
if "%ANDROID_NDK_HOME%"=="" (
  echo Set ANDROID_NDK_HOME or ANDROID_NDK_ROOT
  exit /b 1
)
if "%API%"=="" set API=23
set ROOT=%~dp0\..
set OUT=%ROOT%\out\arm64-v8a
set SRC=%ROOT%\src\libshared_kuboom_bootstrap_v1.c
mkdir "%OUT%" 2>nul
set CC=%ANDROID_NDK_HOME%\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android%API%-clang.cmd
if not exist "%CC%" (
  set CC=%ANDROID_NDK_HOME%\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android%API%-clang.exe
)
if not exist "%CC%" (
  echo Compiler not found. Check ANDROID_NDK_HOME and API.
  exit /b 1
)
"%CC%" -shared -fPIC -O2 -Wall -Wextra -fvisibility=hidden "%SRC%" -llog -o "%OUT%\libshared.so"
echo Built: %OUT%\libshared.so
