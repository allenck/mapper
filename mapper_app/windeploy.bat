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
windeployqt --dir %2 %1\mapper.exe

erase %2\mapper.exe

copy %1\mapper.exe %2
copy %1\Console.dll %2
copy %1\functions.dll %2
copy %1\sqlfun.dll %2
copy C:\Users\allen\Downloads\sqlite-dll-win64-x64-3390400\sqlite3.dll %2 

copy %1\..\..\..\mapper_QT\mapper_app\overlays.xml %2
copy %1\..\..\..\mapper_Qt\READMe.md %2
copy %1\..\..\..\mapper_QT\mapper_app\README.txt %2
copy %1\..\..\..\mapper_Qt\mapper_app\Resources\tram-icon.ico %2
REM copy html file
IF NOT EXIST %2\html\nul (mkdir %2\html && echo %2\html created)
xcopy %1\..\..\..\mapper_QT\mapper_app\html\* %2\html

REM copy wiki pagesiic
IF NOT EXIST %2\wiki\nul (mkdir %2\wiki && echo %2\wiki created)
copy %1\..\..\..\mapper_QT\wiki\* %2\wiki
IF NOT EXIST %2\wiki\images\nul (mkdir %2\wiki\images && echo %2\wiki\images created)
copy %1\..\..\..\mapper_QT\wiki\images\* %2\wiki\images

rem package base files
C:/Qt/Tools/QtInstallerFramework/4.5/bin/archivegen.exe C:\Users\allen\Projects\Mapper\mapper_QT\installer\packages\com.vendor.product\data\mapper_base.7z C:\Users\allen\Projects\mapperDeploy\*

rem package db files
IF NOT EXIST %2\Resources\databases\nul (mkdir %2\Resources\databases && echo %2\Resources\databases created)
REM copy databases from %1\..\..\..\mapper_QT\mapper_app\Resources\databases\ to %2\Resources\databases
copy %1\..\..\..\mapper_QT\mapper_app\Resources\databases\*.sqlite3 %2\Resources\databases
C:/Qt/Tools/QtInstallerFramework/4.5/bin/archivegen.exe C:\Users\allen\Projects\Mapper\mapper_QT\installer\packages\com.vendor.databases\data\mapper_databases.7z C:\Users\allen\Projects\mapperDeploy\Resources\databases\*

cd %1\..\..\..\mapper_QT\
