@echo off

rem system="Visual Studio 15 2017 Win64"
set system="Visual Studio 15 2017"
rem config="Debug;Release"
set config="Debug;MinSizeRel"

if "%1" == "get" exit /b 0

pushd %~dp0

for /f "tokens=2 delims=( " %%i in ('findstr /c:"project(" CMakeLists.txt') do (
  set project=%%i
)

if not exist build (
  mkdir build
)
pushd build

if not exist msvc (
  mkdir msvc
)
pushd msvc

cmake -G %system% -DCMAKE_CONFIGURATION_TYPES=%config% -DCMAKE_INSTALL_PREFIX:PATH=../.. ../..

if %errorlevel% == 0 (
  start %project%.sln
) else (
  pause
)

popd
popd
popd
