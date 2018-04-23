SET SRC_PATH=src
SET MAKE_PARAMS_PATH=MAKEFILE_SOURCES_CMDS

SET FOLDER=_tmp

SET SYM_PATH=sym
SET SYS_PATH=sys

rmdir /S /Q %FOLDER%
mkdir %FOLDER%


copy %SRC_PATH%\* %FOLDER%
copy %MAKE_PARAMS_PATH%\* %FOLDER%

%SystemRoot%\system32\cmd.exe /c "cd %FOLDER% && CMDS.bat && cd .."

copy %FOLDER%\bin\i386\*.pdb %SYM_PATH%
copy %FOLDER%\bin\i386\*.sys %SYS_PATH%

copy %FOLDER%\bin\amd64\*.pdb %SYM_PATH%
copy %FOLDER%\bin\amd64\*.sys %SYS_PATH%

pause