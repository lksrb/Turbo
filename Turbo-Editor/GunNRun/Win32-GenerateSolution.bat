@echo off
call premake5.exe %1
exit /b 1

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