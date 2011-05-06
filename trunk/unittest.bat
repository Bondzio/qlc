@ECHO OFF

pushd .
REM Enttec wing test
cd plugins\ewinginput\src
..\test\test_ewing.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
popd

pushd .
REM Velleman test
cd plugins\vellemanout\src
SET OLDPATH=%PATH%
PATH=%PATH%;C:\k8062d
REM Surprise, surprise, windows doesn't know how to handle mock objects *sigh*
REM ..\test\vellemanout_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
SET PATH=%OLDPATH%
popd

pushd .
REM Engine test
cd engine\test
SET OLDPATH=%PATH%
PATH=%PATH%;..\src
test_engine.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
SET PATH=%OLDPATH%
popd

pushd .
REM UI test
cd ui\test
SET OLDPATH=%PATH%
PATH=%PATH%;..\..\..\engine\src;..\..\..\ui\src

cd aboutbox
aboutbox_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
cd ..

cd addfixture
addfixture_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
cd ..

cd vcframe
vcframe_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
cd ..

cd vclabel
vclabel_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
cd ..

cd vcproperties
vcproperties_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
cd ..

cd vcwidgetproperties
vcwidgetproperties_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
cd ..

cd vcxypadfixture
vcxypadfixture_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
cd ..

SET PATH=%OLDPATH%
popd