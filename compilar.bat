@echo off
setlocal

set PROJETO=%~dp0Demo3D.vcxproj
set EXECUTAVEL=%~dp0build\x64\Debug\Demo3D.exe

echo === Compilando Demo3D (Debug x64) ===

REM Tentar localizar o MSBuild do Visual Studio 2022
set MSBUILD=
for %%e in (Community Professional Enterprise) do (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\%%e\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\%%e\MSBuild\Current\Bin\MSBuild.exe"
        goto :encontrado
    )
)

echo ERRO: MSBuild nao encontrado. Instale o Visual Studio 2022.
pause
exit /b 1

:encontrado
"%MSBUILD%" "%PROJETO%" /p:Configuration=Debug /p:Platform=x64 /v:minimal

if %ERRORLEVEL% neq 0 (
    echo.
    echo === COMPILACAO FALHOU ===
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo === Compilacao concluida. Iniciando aplicacao... ===
echo.
start "" "%EXECUTAVEL%"
