::Script to deploy Mapper 
:: Parameters:
:: 1 shadow build 'release' or 'debug' folder containing Mapper.exe and DLLs
:: 2 output location
:: 3 location of sqlite3.dll (optional)
::
:: Note Must be run in MSCV environment!
REM Deploying Mapper from %1 to %2


echo on
IF exist %2\nul ( echo %2 exists ) ELSE ( mkdir %2 && echo %2 created)
IF NOT EXIST %2\Resources\nul (mkdir %2\Resources && %2\Resources created)
windeployqt --dir %2 %1\mapper.exe
copy %1\mapper.exe %2
copy %1\Console.dll %2
copy %1\functions.dll %2
copy %1\sqlfun.dll %2
copy C:\Users\allen\Downloads\sqlite-dll-win64-x64-3390400\sqlite3.dll %2 

copy %1\..\..\..\mapper_QT\mapper_app\api_keys.txt %2\Resources\
IF NOT EXIST %2\Resources\databases\nul (mkdir %2\Resources\databases && %2\Resources\databases created)
REM copy databases from %1\..\..\..\mapper_QT\mapper_app\Resources\databases\ to %2\Resources\databases
copy %1\..\..\..\mapper_QT\mapper_app\Resources\databases\*.sqlite3 %2\Resources\databases

REM copy wiki pages
IF NOT EXIST %2\wiki\nul (mkdir %2\wiki && %2\wiki created)
copy %1\..\..\..\mapper_QT\wiki\* %2\wiki
IF NOT EXIST %2\wiki\images\nul (mkdir %2\wiki\images && %2\wiki\images created)
copy %1\..\..\..\mapper_QT\wiki\images\* %2\wiki\images
cd %1\..\..\..\mapper_QT\
