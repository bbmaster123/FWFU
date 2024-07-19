@Echo off
setlocal enabledelayedexpansion

:: Set the directory 
set "dir=C:\Program Files\DWMBlur\data\img"
cd /d "%dir%"

:: Count the number of files in the directory
set count=0
for /f "tokens=* delims=" %%f in ('dir /a-d /b /o:n "%dir%\*"') do (
    set /a count+=1
)

:: Generate a random number between 1 and the number of files
set /a "rand=%random% %% %count% + 1"

:: set image as reflection
set count=0
for /f "tokens=* delims=" %%f in ('dir /a-d /b /o:n "%dir%\*"') do (
    set /a count+=1
    if !count! equ %rand% (      
	copy "%%f" "C:\Program Files\DWMBlur\data\AeroPeek.png"
	cd ..
	cd ..
	/*dwmblurglass.exe -loaddll*/
        goto :eof
    )
)
:eof
