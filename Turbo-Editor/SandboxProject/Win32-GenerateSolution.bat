@echo off
premake5 vs2022

IF %1 == "" (
pause
exit /b 1
)

start devenv %1 /Build %2
start devenv %1