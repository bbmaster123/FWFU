# How To: Vivetool GUI

## Vivetool GUI is a program which allows the user to enable or disable chosen Windows Feature IDs (Mach Code)

## Warning

#### You may introduce bugs or other strange behaviors into Windows by messing with these Feature IDs. Please understand what you are doing before you Continue.
Proceed at your own risk. I would recommend keeping a list somewhere of all the IDs you change, and to which state (enabled/disabled)
##

The list of available Feature IDs and the number associated with that feature changes from build to build. This means that when you update Windows, IDs may be added, removed, or have a new ID number assigned to them, and may reset your override.


> ###### 1. Download Vivetool GUI 1.7 and extract: [Vivetool GUI](https://github.com/PeterStrick/ViVeTool-GUI/releases/download/v1.6.999.3/v1.7_PRE_RELEASE_HOTFIX.zip)
> ###### 2. Run Vivetool_GUI.exe. Once loaded, select a Windows build number from the dropdown labeled "Select build..." Use winver to find your Windows build number.
> ![1](https://github.com/user-attachments/assets/209e271a-efff-4312-8b0c-12da6ef6cbcb)
> ###### 3. Once a build is selected, the list will populate. This takes around a minute.
> ###### 4. Once the list has been loaded, find the ID you would like to change, select, and click the perform actions button at the top.
>![2](https://github.com/user-attachments/assets/daf276b1-fa15-4a0e-adca-78d2dbf61423)
> ![3](https://github.com/user-attachments/assets/ee4f0870-9f26-4093-a61d-b469cefca1c1)
> ###### 5. Reboot for changes to take effect.



## Feature Scanner:
Built-in Windows builds are mostly insider builds. This is still somewhat helpful as many insider build feature IDs eventually trickle down into stable releases with their ID number unchanged. Many times when the ID number does change, the name of the feature wont. This can help save time hunting for a Feature ID, or to track down around when a feature was introduced, removed or changed.

In cases where you need the list of ID's for a stable build, you will need to build this list yourself. This can be done using the feature scanner in Vivetool GUI.


> ###### 1. Run Vivetool GUI, and click About & Settings at the top right.
> ###### 2. Click the settings tab, and then scan this build for Feature IDs
> ![image](https://github.com/user-attachments/assets/130af7e2-1822-4347-8daf-c0db6a436865)
> ###### 3. Put your path to symchk.exe in the first box. you can get this file from the windows sdk (Windows Kits)
> ###### 4. In the second box, enter a path to store the temporary files the scanner will use. This Can take up to 50GB of free space. The entire folder can be deleted after scanning is complete. If you do not enter a path, temp files will be stored in C:\Windows\SYMBOLS\
> ###### 5. Click continue and allow the program to do its thing. This can take a few hours and cannot be paused.
> ###### 6. Once scanning is complete, it will prompt you to save a list of all ID's found as a .txt
> ![image](https://github.com/user-attachments/assets/f0c8a8f0-bdd8-453a-81fd-de2ce38d9a88)
> ###### 7. Save your .txt file and exit back to the main screen in Vivetool GUI.
> ###### 8. Click the dropdown labeled "Select build..." at the top left. Select the very first option "Load Manually..."
> ###### 9. Select your text file. The list will populate with the Feature IDs that have been found in your build of Windows.
