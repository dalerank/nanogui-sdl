/*
    src/example1.cpp -- C++ version of an example application that shows 
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <include/screen.h>
#include <include/layout.h>
#include <include/label.h>
#include <include/toolbutton.h>
#include <include/progressbar.h>
#include <include/textbox.h>
#include <include/imagepanel.h>
#include <include/imageview.h>
#include <include/vscrollpanel.h>
#include <include/graph.h>
#include <include/formhelper.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#include <iostream>
#include <SDL2/SDL.h>

using std::cout;
using std::cerr;
using std::endl;

#undef main


enum test_enum {
    Item1 = 0,
    Item2,
    Item3
};

using namespace nanogui;

bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

int main(int /* argc */, char ** /* argv */)
{
    SDL_Init(SDL_INIT_VIDEO);   // Initialize SDL2

    SDL_Window *window;        // Declare a pointer to an SDL_Window

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

    int winWidth = 1024;
    int winHeight = 768;

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
      "An SDL2 window",         //    const char* title
      SDL_WINDOWPOS_UNDEFINED,  //    int x: initial x position
      SDL_WINDOWPOS_UNDEFINED,  //    int y: initial y position
      winWidth,                      //    int w: width, in pixels
      winHeight,                      //    int h: height, in pixels
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN          //    Uint32 flags: window options, see docs
    );

    // Check that the window was successfully made
    if(window==NULL){
      // In the event that the window could not be made...
      std::cout << "Could not create window: " << SDL_GetError() << '\n';
      SDL_Quit();
      return 1;
    }

    auto context = SDL_GL_CreateContext(window);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );

    Screen *screen = new Screen( window, Vector2i(winWidth, winHeight), "NanoGUI test");

    bool enabled = true;
    FormHelper *gui = new FormHelper(screen);
    ref<Window> rwindow = gui->addWindow(Vector2i(10, 10), "Form helper example");
    gui->addGroup("Basic types");
    gui->addVariable("bool", bvar);
    gui->addVariable("string", strval);

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
    rwindow->center();


    bool quit = false;
    try
    {
        //nanogui::init();

        //Event handler
        SDL_Event e;
        //nanogui::ref<ExampleApplication> app = new ExampleApplication();
        while( !quit )
        {
            //Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                screen->onEvent( e );
            }

            SDL_SetRenderDrawColor(renderer, 0xd3, 0xd3, 0xd3, 0xff );
            SDL_RenderClear( renderer );

            screen->drawAll();

            SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff );
            SDL_Rect r{ 0, 0, 20, 30 };
            SDL_RenderFillRect( renderer, &r );

            //Update screen
            SDL_GL_SwapWindow(window);
        }

        //nanogui::shutdown();
    }
    catch (const std::runtime_error &e)
    {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
