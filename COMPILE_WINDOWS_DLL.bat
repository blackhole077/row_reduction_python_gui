:: Author: Evan Beachly and Jeevan Rajagopal
:: Purpose: Compile C code into a DLL format using MSVC without dealing with MSVC


::Echo off so we don't echo these commands. Need the @ so it doesn't echo itself
@echo off

:: Set up the terminal's environment so we can call cl (the compiler). We want 64-bit.
:: Example: call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
call "location\of\vcvarsall.bat" x64

::Get rid of whatever that printed out
cls
:: Just a convenience variable so that you don't have to keep track of where you have to rename the variable
set FILESTUB=row_reduction
:: In order to compile the code such that it can by used by Python (ctypes), we need the location of the pythonXX.lib file.
:: Depending on what version of Python is being used, the last two digits will be different. For example, Python 3.9 will be named python39.lib
set PYTHONLIB= location/of/pythonxx.lib
::Delete previous build, if one exists.
del %FILESTUB%.exe
del %FILESTUB%.txt
del %FILESTUB%.pdb
del %FILESTUB%.exp
del %FILESTUB%.lib
del %FILESTUB%.dll

:: Call the compiler on the main file
::-nologo disables printing name of compiler
::-GR- disables runtime type information
::-Gm- disable incremental build stuff
::-GF Enables the compiler to create a single copy of identical strings in the program image and in memory during execution. This is an optimization called string pooling that can create smaller programs.
::-EHa- disables exception handling
::-Oi enables intrinsics
::-DDEBUG=1 defines the DEBUG macro to be 1
::-W4 warning level 4
::-wd4189 suppress warning 4189, local variable is initialized but not referenced
::-wd4101 suppress warning 4101, unreferenced local variable
::-wd4100 suppress warning 4100, unreferenced formal parameter
::-WX treat warnings as errors
::-Z7 Don't generate an extra .pdb file
::-LD Creates a DLL
::-MT statically link to C runtime library instead of requiring C redistributable. This is MT by default when using the command line, but MD by default in the IDE which is bad.
::-Fm generate map file so we can see what functions are in the executable
::/link start linker options
::-opt:ref Try to not include functions that nobody will use.
::-subsystem:console,5.2 for windows XP compatibility (use 5.1 for x86)
:: user32.lib needed for many Windows functions

:: While I'm not entirely sure, I think you need to include a Python library (e.g., python39 -> Python 3.9.x) for this to work with Python.
cl -nologo -Gm- -GR- -EHa- -GF -Oi -DDEBUG=1 -W4 -Z7 -LD %FILESTUB%.c /link -opt:ref -subsystem:console,5.02 user32.lib %PYTHONLIB%
:: Dump the DLL file information (used for reference)
DUMPBIN.EXE /EXPORTS /OUT:%FILESTUB%.txt %FILESTUB%.dll

::Don't need the object file (nor the other files)
del %FILESTUB%.obj
del %FILESTUB%.pdb
del %FILESTUB%.exp
del %FILESTUB%.lib
::If compilation succeeded,
echo %ERRORLEVEL%
if %ERRORLEVEL% == 0 (
	::Keep the console up for a couple seconds so we can see warnings
	timeout 5
) else (
	::There were build errors. Pause here so we can see them
	pause
)
GOTO :EOF

:Run_VS
echo "Running Visual Studio on %~1..."
devenv %1 /NoSplash
GOTO :EOF

:Run_RemedyBG
echo "Running RemedyBG on %~1..."
if exist session.rdbg (
	::Run RemedyBG on the session so we can debug it
	start "" "location/of/remedybg.exe" "../../session.rdbg"
) else (
	::Run RemedyBG on the executable so we can debug it
	start "" "location/of/remedybg.exe" %1
)
GOTO :EOF