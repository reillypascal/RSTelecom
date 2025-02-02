# RS Telecom

Lo-fi plugin with options of various telecommunications codecs including Mu-Law and A-Law 8-bit, and GSM 06.10. More codecs and glitching effects coming soon.

## Build Dependencies:
- JUCE (https://juce.com/download/)
- Projucer (https://docs.juce.com/master/tutorial_new_projucer_project.html) (for creating Xcode/Visual Studio projects or Linux Makefiles in order to build)

<!--## Windows:
- Compiled Windows files are available under "Releases". Unzip the files and place them in 
	- C:\Program Files\Common Files\VST3 (VST3)
	- C:\Program Files\Common Files\Avid\Audio\Plug-Ins (AAX) 
-->
## macOS:
- Compiled macOS files are available under "Releases".
- You will likely need to disable Gatekeeper for the plugins. To do this for AU, open Terminal.app and type...
```sh
spctl --add "/Library/Audio/Plug-Ins/Components/RSBrokenMedia.component"
```

...for VST3...
```sh
spctl --add "/Library/Audio/Plug-Ins/VST3/RSBrokenMedia.vst3"
```

<!--...or for AAX...
```sh
spctl --add "/Library/Application Support/Avid/Audio/Plug-Ins/RSBrokenMedia.aaxplugin"
```
-->
- You can also add the file path by typing...
```sh
spctl --add 
```

...(with a space at the end) and dragging the plugin file into the terminal, which will automatically add the file path.

## Linux/Windows:
Compiler targets are available for Linux and Windows. Set up JUCE on your computer, open the .jucer file in the Projucer, generate the Linux Makefile or Visual Studio project, and then you can compile the plugins.
