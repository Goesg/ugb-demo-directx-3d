@echo off
setlocal

set PROJETO=%~dp0Demo3D.vcxproj
set EXECUTAVEL=%~dp0build\x64\Debug\Demo3D.exe

echo === Compilando Demo3D (Debug x64) ===

REM Procurar MSBuild em todas as versões e edições do Visual Studio
set MSBUILD=
for %%v in (18 2022 2019 17) do (
    for %%e in (Community Professional Enterprise) do (
        if exist "C:\Program Files\Microsoft Visual Studio\%%v\%%e\MSBuild\Current\Bin\MSBuild.exe" (
            set "MSBUILD=C:\Program Files\Microsoft Visual Studio\%%v\%%e\MSBuild\Current\Bin\MSBuild.exe"
            goto :encontrado
        )
    )
)

echo ERRO: MSBuild nao encontrado. Abra este .bat pelo Developer PowerShell do Visual Studio.
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
