# How To: Use UWPspy in combination with Windhawk Styler mods.
*First disable your Styler mod, as both programs can conflict when they are used at the same time, causing UWPspy to be unable to set styles directly, and/or explorer restarts.

*Styles set within UWPspy do not get saved

> 1. Download the latest version of [UWPspy](https://ramensoftware.com/downloads/uwpspy.zip). Extract both files, and run UWPspy.exe
>
> 2. Select the process you'd like to target, choose UWP, and click spy
> 
> ![1](https://github.com/user-attachments/assets/f78896e4-2fa0-49a8-b63b-bd08cec2bb42)
>
> 3. You will see a tree of target elements. As you click through each entry, the element will be hilighted in red
> ![2](https://github.com/user-attachments/assets/8b88a3be-b25a-46ff-83db-edb755b87aec)
>
> 4. To target a specific element you do not know the name of, make sure uwpspy is the active window, hover your cursor over the element to target, and press CTRL+D. This will make UWPspy jump to that element
>
> 5. Once you have found the element you'd like to style, take note of which styles are avaible to that target. Each target may or may not support a given style. For example, an icon might have the Glyph style, but a BackgroundElement will not.
>
> 6. Some (but not all) elements have Visual States. This can include common states Active, Inactive, PointerOver, etc) and unique states (RunningIndicatorStates, ResizeEnabled, Dragging, etc). Common Visual states are shared across serveral elements, while unique states may only be for one specific target. To see which states a target might have, click the visual states tab on the right.  
>
> 7. Now you may set, alter, or remove styles on the right by double clicking the style's name, and entering a value/style into the box below, then click set/remove. Enable the Detailed Property List checkbox to see all styles. Once you are done looking through the list of styles and testing values, we can transfer that into the styler mod. 
>
> 8. To target the same element in the styler mod with the same styles, we first need the name of the target. to get this, look at the top right, you'll notice Class: and Name:
> Use the class, followed by # and then the name (Class#Name)
> ex.
> Taskbar.TaskListButtonPanel#ExperienceToggleButtonRootPanel
>
> To style the target only when in a specific state (ex. PointerOver) add @VisualState to the target path (Class#Name@VisualStatesGroupName)
> ex.
> Taskbar.TaskListButtonPanel#ExperienceToggleButtonRootPanel@CommonStates

> 9. To target a child element (an element within another parent element) use the > symbol (Class#Name@VisualStatesGroupName > ChildElement)
> ex.
> Taskbar.TaskListButtonPanel#ExperienceToggleButtonRootPanel@CommonStates > Border#BackroundElement
> 10. Once you have your target, you can copy the styles as is. For example, if you had Background=Red, set Background=Red in the styler. 

# Video Demonstration 
##### (credits to m417z)
[![Demo video](https://github.com/m417z/UWPSpy/raw/main/screenshot-video.png)](https://youtu.be/Zxgk_BOVpfk)
*Click on the image to view the demo video on YouTube.*
