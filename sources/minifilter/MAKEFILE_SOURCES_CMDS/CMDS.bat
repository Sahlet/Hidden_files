SET THIS_DIR=%cd%
SET DDK_PATH=C:\WinDDK\7600.16385.1


%SystemRoot%\system32\cmd.exe /c "cd /d %DDK_PATH%\bin\&&setenv.bat %DDK_PATH%\ chk x86 WXP &&cd /d %THIS_DIR%\&&build -ceZg"