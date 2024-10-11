# How To: Use UWPspy in combination with Windhawk Styler mods.
*First disable your styler mod, as both programs can conflict when they are used at the same time, causing UWPspy to be unable to set styles directly, and/or explorer restarts.

*Styles set within UWPspy do not get saved

*Styles Are Case-Sensitive. Use Camel Case (Each Word Starts With A Capital Letter)

*To see a video demonstration of the software, please scroll to the bottom of this page
##
> 1. Download the latest version of [UWPspy](https://ramensoftware.com/downloads/uwpspy.zip). Extract both files, and run UWPspy.exe
>
> 2. Select the process you'd like to target, choose UWP, and click spy
>
> ![1](https://github.com/user-attachments/assets/f78896e4-2fa0-49a8-b63b-bd08cec2bb42)
>
> 3. You will see a tree of target elements. As you click through each entry, the element will be hilighted in red
>    
> ![2](https://github.com/user-attachments/assets/8b88a3be-b25a-46ff-83db-edb755b87aec)
>
> 5. To target a specific element you do not know the name of, make sure UWPspy is the active window, hover your cursor over the element to target, and press CTRL+D. This will make UWPspy jump to that element
>
> 6. Once you have found the element you'd like to style, take note of which styles are avaible to that target. Each target may or may not support a given style. For example, an icon might have the Glyph style, but a BackgroundElement will not.
>
> 7. Some (but not all) elements have Visual States. This can include common states (Active, Inactive, PointerOver, etc) and unique states (RunningIndicatorStates, ResizeEnabled, Dragging, etc). Common visual states are shared across serveral elements, while unique states may only be for one specific target or group of targets (Ex. MultiWindowActive). To see which states a target might have, click the visual states tab on the right.  
>
>![3](https://github.com/user-attachments/assets/c7772887-6936-49cd-bcb3-e295905db361)
>
> 8. Now you may set, alter, or remove styles on the right by double clicking the style's name, and entering a value/style into the box below, then click set. 
>
> *Remove doesn't just set the value back to default, it will remove the current value and replace it with nothing. If the style had a default value, you will need to either set that in UWPspy or restart explorer to reset it
>
> *Enable the Detailed Property List checkbox at the bottom to see all styles. Once you are done looking through the list of styles and testing values, we can transfer that into the styler mod. 
>
> 8. To target the same element in the styler mod with the same styles, we first need the name of the target. to get this, look at the top right, you'll notice Class: and Name:
> Use the class, followed by # and then the name (Class#Name)
>
>![4](https://github.com/user-attachments/assets/b97e7a3a-6cdf-4cd9-87ce-69b56dd9eb26)
>
> ex.
> Taskbar.TaskListButtonPanel#ExperienceToggleButtonRootPanel
>
> To style the target only when in a specific state (ex. PointerOver) add @VisualState to the target path (Class#Name@VisualStatesGroupName)
>
> ex.
> Taskbar.TaskListButtonPanel#ExperienceToggleButtonRootPanel@CommonStates
>
> To target a child element (an element within a parent element) use the > symbol (Class#Name@VisualStatesGroupName > ChildElement)
> ex.
> Taskbar.TaskListButtonPanel#ExperienceToggleButtonRootPanel@CommonStates > Border#BackroundElement
> 
> 9. Once you have your target, you can copy the styles as is. For example, if you had Background set to Red in UWPspy, set Background=Red in the styler mod. 

## FAQ
- Why am I getting an error when I click Set?
> Either the styler mod was active when you started using UWPspy, or that style cannot be set with UWPspy. If the first thing, disable your styler mod, restart explorer, and try again 
- How do you get the targets for elements that disappear when you click away? (Ex. Menus, toasts, SnapLayout, etc) 
> If you are unable to CTRL+D to jump to your target, the best way would be to enlarge UWPspy so it has lots of space to show the visual tree, then bring up the element. You will see the visual tree update, and the targets will appear until you click away. Screenshot the window, paste somewhere, and work from that. 
- I was playing around, but I messed things up. How do I reset?
>  restart explorer
- Does this work on Windows 10?
>  UWPspy works, styler mods depend on which app you are trying to style. Taskbar Styler does not work. Notification Center Styler works, and you can fork it and change the includes (@include app.exe) to work with more windows 10 UWP apps
- I'm trying to move my element, but its dissapearing behind an invisible line? I just want (element) on the (position)! (Ex. Start Menu on the right) 
> Elements can have a bounding box. This s predefined region where the element may appear, and not outside that space. If you try to move your element outside this defined region, it will being to get cut off until it dissapears. There is no way around this unless you create a 
> completely new mod. (Ex Taskbar on left mod)
- I'm having trouble/I don't get something. Can you help me?
> Of course! Just create a new issue in the appropriate styler guide repo. Please do try to make sure its something that answered doezens of times before. Read through some of the open and closed issues to see if maybe your question has been already answered or explained. I, like most 
> people, don't thouroughly enjoy repeating myself :)

# Video Demonstration 
##### (credits to m417z)
[![Demo video](https://github.com/m417z/UWPSpy/raw/main/screenshot-video.png)](https://youtu.be/Zxgk_BOVpfk)
*Click on the image to view the demo video on YouTube.*
