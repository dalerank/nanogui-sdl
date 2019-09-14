# NanoGUI-SDL [![Build Status](https://travis-ci.org/dalerank/nanogui-sdl.svg?branch=master)](https://travis-ci.org/dalerank/nanogui-sdl)

NanoGUI-SDL is a SDL port for https://github.com/wjakob/nanogui

## Example screenshot
![Screenshot](https://github.com/dalerank/nanogui-sdl/blob/master/resources/screenshot1.png "Screenshot")

## Description
NanoGUI-SDL builds on [SDL2](http://www.libsdl.org/) for cross-platform context
creation and event handling, for
basic vector types, and [NanoVG/NanoVG-RT](https://github.com/memononen/NanoVG) to draw
2D primitives.

NanoGUI-SDL currently works on Mac OS X (Clang) Linux (GCC or Clang) and Windows
(Visual Studio ≥ 2015, Gcc ≥ 4.8); it requires a recent C++11 capable compiler. All
dependencies are jointly built using a CMake-based build system.

## Creating widgets
NanoGUI-SDL makes it easy to instantiate widgets, set layout constraints, and
register event callbacks using high-level C++11 code. For instance, the
following two lines from the included example application add a new button to
an existing window `window` and register an event callback.
```C++
auto& button = window.add<Button>("Plain button")
                     .withCallback([] { cout << "pushed!" << endl; });
```

The following lines from the example application create the coupled
slider and text box on the bottom of the second window (see the screenshot).
```C++
/* Create an empty panel with a horizontal layout */
auto& panel = window.add<Widget>()
                    .withLayout<BoxLayout>(BoxLayout::Horizontal, BoxLayout::Middle, 0, 20);

/* Add a slider and set defaults */
auto& slider = panel.add<Slider>()
                    .withValue(0.5f)
                    .withFixedWidth(80);

/* Add a textbox and set defaults */
auto& textBox = panel.add<TextBox>()
                     .withFixedSize(Vector2i(60, 25))
                     .withValue("50")
                     .withUnits("%");

/* Propagate slider changes to the text box */
slider.setCallback([textBox](float value) { textBox.setValue(std::to_string((int) (value * 100))); });


/* Create an empty panel with a horizontal layout */
window.widget()
        .boxlayout(BoxLayout::Horizontal, BoxLayout::Middle, 0, 20))
            /* Add a slider and set defaults */
            .slider(0.5f, [](Slider* obj, float value) {
                if (auto* textBox = obj->gfind<TextBox>("slider-textbox"))
                    textBox->setValue(std::to_string((int) (value * 100)) );
            }).withFixedWidth(80).and() 
            /* Add a textbox and set defaults */   
            .textbox("50", "%").withFixedSize(Vector2i(60, 25))    
   		        .withId("slider-textbox")

```

## "Simple mode"

Christian Schüller contributed a convenience class that makes it possible to
create AntTweakBar-style variable manipulators using just a few lines of code.
For instance, the source code below was used to create the following example
application.

```C++
/// dvar, bar, strvar, etc. are double/bool/string/.. variables

FormHelper *gui = new FormHelper(screen);
ref<Window> window = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
gui->addGroup("Basic types");
gui->addVariable("bool", bvar);
gui->addVariable("string", strvar);

gui->addGroup("Validating fields");
gui->addVariable("int", ivar);
gui->addVariable("float", fvar);
gui->addVariable("double", dvar);

gui->addGroup("Complex types");
gui->addVariable("Enumeration", enumval, enabled)
   ->setItems({"Item 1", "Item 2", "Item 3"});
gui->addVariable("Color", colval);

gui->addGroup("Other widgets");
gui->addButton("A button", [](){ std::cout << "Button pressed." << std::endl; });

screen->setVisible(true);
screen->performLayout();
window->center();
```

## Compiling on windows (MSVC/git bash)
```bash
$ git clone https://github.com/dalerank/nanogui-sdl
$ cd nanogui-sdl
$ export CURSRC=`pwd`
# this part if you have not installed sdl2 libraries
$ curl https://www.libsdl.org/release/SDL2-2.0.10-win32-x86.zip --output sdl2.zip
$ 7z e sdl2.zip -obuild/debug -y -r -spf
$ curl https://www.libsdl.org/release/SDL2-devel-2.0.10-VC.zip --output sdl2-devel.zip
$ 7z e sdl2-devel.zip -y -r -spf
$ curl https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5-win32-x86.zip --output sdl2img.zip
$ 7z e sdl2img.zip -obuild/debug -y -r -spf
$ curl https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-VC.zip --output sdl2img-devel.zip
$ 7z e sdl2img-devel.zip -y -r -spf
$ curl https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.15-win32-x86.zip --output sdl2ttf.zip
$ 7z e sdl2ttf.zip -obuild/debug -y -r -spf
$ curl https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-VC.zip --output sdl2ttf-devel.zip
$ 7z e sdl2ttf-devel.zip -y -r -spf        
# now build example
$ mkdir -p build
$ cd build
$ cmake --version 
# here setup default path to libraries you may change it for real sdl2 files placed
$ cmake -DSDL2_LIBRARY="$CURSRC/SDL2-2.0.10/lib/x86/sdl2.lib" -DSDL2_INCLUDE_DIR="$CURSRC/sdl2-2.0.10/include" -   DSDL2TTF_LIBRARY="$CURSRC/SDL2_ttf-2.0.15/lib/x86/sdl2_ttf.lib" -DSDL2TTF_INCLUDE_DIR="$CURSRC/SDL2_ttf-2.0.15/include" -DSDL2_IMAGE_LIBRARY="$CURSRC/SDL2_image-2.0.5/lib/x86/sdl2_image.lib" -DSDL2_IMAGE_INCLUDE_DIR="$CURSRC/SDL2_image-2.0.5/include" ..
$ cmake --build .
```

### start On Linux (Ubuntu)
```
#prerequirements:

$ sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

#setup and run example:

$ git clone https://github.com/dalerank/nanogui-sdl
$ cd nanogui-sdl
$ mkdir -p build
$ cd build/
$ cmake ..
$ cmake --build .
$ ./example1

```

### License

nanogui-sdl is provided under a BSD-style license that can be found in the
``LICENSE.txt`` file. By using, distributing, or contributing to this project,
you agree to the terms and conditions of this license.
