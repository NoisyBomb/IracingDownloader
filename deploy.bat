@echo off
echo === iRacing Setup Manager Deploy ===

set QT_PATH=C:\Qt\6.8.3\msvc2022_64
set SCRIPT_DIR=%~dp0
set DIST_DIR=%SCRIPT_DIR%dist
set RELEASE_DIR=%SCRIPT_DIR%..\..\build\Desktop_Qt_6_8_3_MSVC2022_64bit-Release\release

echo Release folder: %RELEASE_DIR%
echo Output folder:  %DIST_DIR%
echo.

if not exist "%RELEASE_DIR%\IracingDownloader.exe" (
    echo ERROR: IracingDownloader.exe not found in:
    echo   %RELEASE_DIR%
    pause
    exit /b 1
)

if exist "%DIST_DIR%" rmdir /S /Q "%DIST_DIR%"
mkdir "%DIST_DIR%"

echo Copying exe...
copy "%RELEASE_DIR%\IracingDownloader.exe" "%DIST_DIR%\"

if exist "%SCRIPT_DIR%icon.ico" (
    echo Copying icon...
    copy "%SCRIPT_DIR%icon.ico" "%DIST_DIR%\"
)

if exist "%SCRIPT_DIR%pic" (
    echo Copying pic/ folder...
    xcopy /E /I /Y "%SCRIPT_DIR%pic" "%DIST_DIR%\pic\"
)

echo.
echo Running windeployqt...
"%QT_PATH%\bin\windeployqt.exe" --release --no-translations --no-system-d3d-compiler "%DIST_DIR%\IracingDownloader.exe"

echo.
echo ==============================
echo Done! dist folder: %DIST_DIR%
echo ==============================
pause
