::Script to deploy Mapper 
:: Parameters:
:: 1 shadow build 'release' or 'debug' folder containing Mapper.exe and DLLs
:: 2 output location
::
:: Note Must be run in MSCV environment!
REM Deploying Mapper from %1 to %2

RMDIR %2 /S /Q
echo on
IF exist %2\nul ( echo %2 exists ) ELSE ( mkdir %2 && echo %2 created)
IF NOT EXIST %2\Resources\nul (mkdir %2\Resources && echo %2\Resources created)
REM C:\Qt\6.4.1\msvc2019_64\bin\windeployqt --dir %2 %1\mapper.exe
%QTDIR%\bin\windeployqt --dir %2 %1\mapper.exe

erase %2\mapper.exe

copy %1\mapper.exe %2
copy %1\..\..\..\mapper_QT\mapper_app\README.txt %2
copy %1\..\..\..\mapper_Qt\mapper_app\Resources\tram-icon.ico %2
REM copy C:\Users\allen\Downloads\sqlite-dll-win64-x64-3390400\sqlite3.dll %2

REM copy Resources and sub-folders
xcopy %1\..\..\..\mapper_QT\mapper_app\Resources %2\Resources
rem package base files
C:/Qt/Tools/QtInstallerFramework/4.5/bin/archivegen.exe C:\Users\allen\Projects\Mapper\mapper_QT\installer\packages\com.vendor.product\data\mapper_base.7z C:\Users\allen\Projects\mapperDeploy\*

rem package db files
IF NOT EXIST %2\Resources\databases\nul (mkdir %2\Resources\databases && echo %2\Resources\databases created)
REM copy databases from %1\..\..\..\mapper_QT\mapper_app\Resources\databases\ to %2\Resources\databases
copy %1\..\..\..\mapper_QT\mapper_app\Resources\databases\*.sqlite3 %2\Resources\databases
C:/Qt/Tools/QtInstallerFramework/4.5/bin/archivegen.exe C:\Users\allen\Projects\Mapper\mapper_QT\installer\packages\com.vendor.databases\data\mapper_databases.7z C:\Users\allen\Projects\mapperDeploy\Resources\databases\*

cd %1\..\..\..\mapper_QT\