cd build
rem copy tdengine.sln ..\tdengine.sln
cd ..
del /s /q build
rd build
pause
rem rd /q build
rem md build
cd build
cmake ..
copy ..\tdengine.sln tdengine.sln
pause