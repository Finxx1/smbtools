@echo off

set Output=aminfo.exe
set Cflags=/W4 /Z7 /D_CRT_SECURE_NO_WARNINGS
set Files=main.c

set Target=%1
if "%1"=="" (
set Target=build
)

if "%Target%"=="build" (

cl %Files% %Cflags% %Ldflags% /Fe:%Output%

)
