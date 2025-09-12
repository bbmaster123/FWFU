@echo
for /f "tokens=3 delims=: " %%a in ('netsh interface show interface "Ethernet" ^| findstr "Admin State"') do (
  @echo Checking state...
  if /i "%%a" == "enabled" (
    @echo Disabling...
    netsh interface set interface "Ethernet" admin=disabled
  ) else (
    @echo Enabling...
    netsh interface set interface "Ethernet" admin=enabled
  )
)