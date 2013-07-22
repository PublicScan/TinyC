@echo off
set PATH=F:\MinGW\bin;%PATH%
call "%VS90COMNTOOLS%vsvars32.bat" > nul
rem call "%VS110COMNTOOLS%vsvars32.bat" > nul
make.exe %*
