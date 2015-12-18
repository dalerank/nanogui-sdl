# NanoGUI-SDL

NanoGUI-SDL is a SDL port for https://github.com/wjakob/nanogui

## Example screenshot
![Screenshot](http://i.imgur.com/fe9nJBm.png "Screenshot")

## Description
NanoGUI-SDL builds on [SDL2](http://www.libsdl.org/) for cross-platform OpenGL context
creation and event handling, [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page) for
basic vector types, and [NanoVG](https://github.com/memononen/NanoVG) to draw
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
```

## "Simple mode"

Christian Schüller contributed a convenience class that makes it possible to
create AntTweakBar-style variable manipulators using just a few lines of code.
For instance, the source code below was used to create the following example
application.

![Screenshot](https://github.com/wjakob/nanogui/raw/master/resources/screenshot2.png "Screenshot")


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

## Compiling
Clone the repository and all dependencies (with `git clone --recursive`),
run CMake to generate Makefiles or CMake/Visual Studio project files, and
the rest should just work automatically.

On Debian/Ubuntu, make sure that you have installed the following packages
```bash
$ apt-get install cmake xorg-dev libglu1-mesa-dev libsdl2.0
```
### License

nanogui-sdl is provided under a BSD-style license that can be found in the
``LICENSE.txt`` file. By using, distributing, or contributing to this project,
you agree to the terms and conditions of this license.
