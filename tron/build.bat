
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0"\VC\vcvarsall.bat

cl /EHsc carlos.cpp tron.cpp  /link  /SUBSYSTEM:CONSOLE

cl /EHsc flynn.cpp tron.cpp  /link  /SUBSYSTEM:CONSOLE


PAUSE
