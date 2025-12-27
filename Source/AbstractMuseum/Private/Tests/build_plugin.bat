@echo off
setlocal enabledelayedexpansion

rem Find script dir
set "SCRIPT_DIR=%~dp0"

rem Find plugin dir
set "PLUGIN_DIR=%SCRIPT_DIR%\..\..\..\..\"

rem Get drive letter
for %%D in ("%cd%") do set "DRIVE=%%~dD"

rem Create dir in drive root
set "OUTPUT_DIR=%DRIVE%\AbstractMuseumBuild"
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)
echo === Searching RunUAT.bat ...
for /f "delims=" %%i in ('where RunUAT.bat 2^>nul') do (
    set "UAT_PATH=%%i"
    goto :found
)

echo RunUAT.bat not found with PATH. Get default path F:\UE_5.5
set "UAT_PATH=F:\UE_5.5\Engine\Build\BatchFiles\RunUAT.bat"

:found
if not exist "!UAT_PATH!" (
    echo [ERROR] RunUAT.bat not found: "!UAT_PATH!"
    pause
    goto :eof
)

echo Use UAT: "!UAT_PATH!"

call "!UAT_PATH!" BuildPlugin -plugin="%PLUGIN_DIR%\AbstractMuseum.uplugin" -package="%OUTPUT_DIR%" -nocompileeditor -progress -verbose

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Plugin build failed.
    echo Error code: %errorlevel%
    echo.
    pause
    goto :eof
)

echo.
echo === Successful plugin build completed! Path: %OUTPUT_DIR%
echo === Directory with build will be removed after test close.
echo.

rem Directory with build will be removed after test close.
pause
rd /s /q "%OUTPUT_DIR%"
endlocal