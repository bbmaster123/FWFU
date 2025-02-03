cd "C:\Users\[username]\AppData\Local\Packages\microsoft.windowscommunicationsapps_8wekyb3d8bbwe\Settings"
del settings.dat /F
copy settings-clean.dat settings.dat /Y
cd "C:\Users\[username]\AppData\Local\Packages\microsoft.windowscommunicationsapps_8wekyb3d8bbwe\LocalState\Migration\"
copy /y NUL handoff.txt
attrib +r handoff.txt
start outlookcal:

