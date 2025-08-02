# RS Telecom

Lo-fi plugin with options of various telecommunications codecs including Mu-Law and A-Law 8-bit, and GSM 06.10. More codecs and glitching effects coming soon.

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

# Building from Source

## Build Dependencies:
- JUCE (https://juce.com/download/)
- Either:
  - Projucer (https://docs.juce.com/master/tutorial_new_projucer_project.html) (for creating Xcode/Visual Studio projects or Linux Makefiles in order to build)
  - CMake

## Compiling with the Projucer
Download the [JUCE repo](https://github.com/juce-framework/JUCE) onto your computer. You will need to set up the Projucer to know where this folder is. Open the .jucer file in the Projucer, generate the Linux Makefile, Visual Studio project, or Xcode project, and then you can compile the plugins using those respective tools. Alternatively, see below for CMake instructions.

## Compiling with CMake
- Clone the [JUCE repo](https://github.com/juce-framework/JUCE) into the working folder (`git clone https://github.com/juce-framework/JUCE.git`)
- Run the following for a CLI build. Note that you may need to run these with `sudo`:
```sh
# sets up a default build:
cmake -S . -B build
# alternatively, you can explicitly set up a debug or release build:
cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Debug
# or
cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Release

# you can add the flag -D COPY_PLUGIN_AFTER_BUILD=TRUE to copy the built files to the default location on macOS

# after running one of the above three options, run
cmake --build build
```

You can also use CMake to generate Xcode/Visual Studio projects.
- Run the following to generate an Xcode project:
```sh
cmake -B Builds -G Xcode
```
- Run the following to generate a Visual Studio project:
```sh
cmake -B Builds -G "Visual Studio 17 2022"
```

### Debugging
`launch.json` sets up the ability to launch an app of your choice (e.g., REAPER, JUCE's AudioPluginHost, etc.) as part of a debugging session. You can configure which app in your editor; e.g., for Zed, see [the debugger documentation](https://zed.dev/docs/debugger#configuration).

You can also use [Pluginval](https://github.com/Tracktion/pluginval). After installing Pluginval and adding the binary to your PATH, run `pluginval --strictness-level 5 <path-to-plugin>`. Strictness level 5 is the baseline, and it can go up to 10 for more rigorous testing.

Pluginval uses aufx under the hood for AU plugins on macOS. You can run auval directly with `auval -strict -v aufx Rstc Rspi` if this plugin is installed on the computer.
