@echo off
pushd %~dp0\..\
call dependencies\premake\bin\premake5.exe vs2022
popd

IF "%1" == "" (
    pause
    exit /b 1
)

IF "%2" == "" (
    echo [Win32-GenerateSolution] Opening and building visual studio solution!
    start devenv %1 /Command "Build.BuildSolution"
    exit /b 1
)

echo [Win32-GenerateSolution] Only building visual studio solution!
start devenv %1 /Build %2