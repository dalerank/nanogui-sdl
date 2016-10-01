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

#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/formhelper.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#include <iostream>

#ifdef NANOGUI_LINUX
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#else
    #include <SDL/SDL_opengl.h>
#endif

using std::cout;
using std::cerr;
using std::endl;

#undef main

using namespace nanogui;

class TestWindow : public nanogui::Screen
{
public:
    TestWindow( SDL_Window* pwindow, int rwidth, int rheight )
      : nanogui::Screen( pwindow, Eigen::Vector2i(rwidth, rheight), "SDL_gui Test")
      {
        mProgress = nullptr;
        using namespace nanogui;

        {
          auto& nwindow = wdg<Window>("Button demo");
          nwindow.setPosition(Vector2i(15, 15));
          nwindow.setLayout(new GroupLayout());

          nwindow.wdg<Label>("Push buttons", "sans-bold");
          nwindow.wdg<Button>("Plain button").withCallback([] { cout << "pushed!" << endl; } );

          nwindow.wdg<Button>("Styled", ENTYPO_ICON_ROCKET).withCallback([] { cout << "pushed!" << endl; })
                                                           .withBackgroundColor( Color(0, 0, 255, 25) );

          nwindow.wdg<Label>("Toggle buttons", "sans-bold");
          nwindow.wdg<Button>("Toggle me").withFlags(Button::ToggleButton)
                                          .withChangeCallback([](bool state) { cout << "Toggle button state: " << state << endl; });

          nwindow.wdg<Label>("Radio buttons", "sans-bold");
          nwindow.wdg<Button>("Radio button 1").withFlags(Button::RadioButton);

          nwindow.wdg<Button>("Radio button 2").withFlags(Button::RadioButton);
          nwindow.wdg<Label>("A tool palette", "sans-bold");

          auto& tools = nwindow.wdg<Widget>().withLayout<BoxLayout>(Orientation::Horizontal,
                                                                    Alignment::Middle, 0, 6);

          tools.add<ToolButton>(ENTYPO_ICON_CLOUD);
          tools.add<ToolButton>(ENTYPO_ICON_FF);
          tools.add<ToolButton>(ENTYPO_ICON_COMPASS);
          tools.add<ToolButton>(ENTYPO_ICON_INSTALL);

          nwindow.wdg<Label>("Popup buttons", "sans-bold");
          PopupButton& pButton = nwindow.wdg<PopupButton>("Popup", ENTYPO_ICON_EXPORT);
          pButton.popup()->withLayout<GroupLayout>();
          pButton.popup()->wdg<Label>("Arbitrary widgets can be placed here");
          pButton.popup()->wdg<CheckBox>("A check box");

          PopupButton& rPopupBtn = pButton.popup()->wdg<PopupButton>("Recursive popup", ENTYPO_ICON_FLASH);
          rPopupBtn.popup()->withLayout<GroupLayout>();
          rPopupBtn.popup()->add<CheckBox>("Another check box");
        }

        std::vector<std::pair<int, std::string>>
            icons = loadImageDirectory(mNVGContext, "icons");

        {
          auto& window = wdg<Window>("Basic widgets")
                              .withPosition(Vector2i(200, 15))
                              .withLayout<GroupLayout>();

          window.add<Label>("Message dialog", "sans-bold");
          auto& tools = window.wdg<Widget>()
                              .withLayout<BoxLayout>(Orientation::Horizontal,
                                                       Alignment::Middle, 0, 6);
          tools.wdg<Button>("Info")
                .withCallback([&] {
              wdg<MessageDialog>(MessageDialog::Type::Information, "Title", "This is an information message")
                    .withCallback([](int result) { cout << "Dialog result: " << result << endl; });
            } );
          tools.wdg<Button>("Warn")
                .withCallback([&]
              {
                wdg<MessageDialog>(MessageDialog::Type::Warning, "Title", "This is a warning message")
                      .withCallback([](int result) { cout << "Dialog result: " << result << endl; });
              }
          );
          tools.wdg<Button>("Ask")
                .withCallback([&] {
              wdg<MessageDialog>(MessageDialog::Type::Warning, "Title", "This is a question message", "Yes", "No", true)
                    .withCallback([](int result) { cout << "Dialog result: " << result << endl; });
            });

          window.add<Label>("Image panel & scroll panel", "sans-bold");
          PopupButton& imagePanelBtn = window.wdg<PopupButton>("Image Panel");

          imagePanelBtn.setIcon(ENTYPO_ICON_FOLDER);
          auto* popup = imagePanelBtn.popup();
          VScrollPanel& vscroll = popup->wdg<VScrollPanel>();
          ImagePanel& imgPanel = vscroll.wdg<ImagePanel>()
                                        .withImages(icons);

          popup->setFixedSize(Vector2i(245, 150));

          auto& img_window = wdg<Window>("Selected image");
          img_window.withPosition(Vector2i(675, 15))
                    .withLayout<GroupLayout>();

          auto& img = img_window.wdg<ImageView>()
                                .withPolicy(ImageView::SizePolicy::Expand)
                                .withImage(icons[0].first);
          img.setFixedSize(Vector2i(300, 300));
          imgPanel.setCallback([&img, &imgPanel, &imagePanelBtn](int i) {
              img.setImage(imgPanel.images()[i].first); cout << "Selected item " << i << endl;
          });

          auto& img_cb = img_window.wdg<CheckBox>( "Expand",
              [&img](bool state) { if (state) img.setPolicy(ImageView::SizePolicy::Expand);
                                   else       img.setPolicy(ImageView::SizePolicy::Fixed); });
          img_cb.setChecked(true);
          window.wdg<Label>("File dialog", "sans-bold");
          auto& tools2 = window.wdg<Widget>().withLayout<BoxLayout>( Orientation::Horizontal,
                                                                     Alignment::Middle, 0, 6 );
          tools2.wdg<Button>("Open")
                .withCallback([&] {
                    cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, false) << endl;
              });

          tools2.wdg<Button>("Save").withCallback([&] {
              cout << "File dialog result: " << file_dialog(
                      { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, true) << endl;
          });

          window.add<Label>("Combo box", "sans-bold");
          window.add<ComboBox>(std::vector<std::string>{ "Combo box item 1", "Combo box item 2", "Combo box item 3"});
          window.add<Label>("Check box", "sans-bold");

          auto& cb = window.wdg<CheckBox>("Flag 1",
              [](bool state) { cout << "Check box 1 state: " << state << endl; }
          );
          cb.setChecked(true);
          window.add<CheckBox>("Flag 2",
              [](bool state) { cout << "Check box 2 state: " << state << endl; }
          );
          window.wdg<Label>("Progress bar", "sans-bold");
          mProgress = &window.wdg<ProgressBar>();

          window.add<Label>("Slider and text box", "sans-bold");

          auto& panel = window.wdg<Widget>()
                              .withLayout<BoxLayout>(Orientation::Horizontal,
                                                     Alignment::Middle, 0, 20);

          auto& slider = panel.wdg<Slider>();
          slider.setValue(0.5f);
          slider.setFixedWidth(80);

          auto& textBox = panel.wdg<TextBox>();
          textBox.setFixedSize(Vector2i(60, 25));
          textBox.setValue("50");
          textBox.setUnits("%");
          slider.setCallback([&textBox](float value) {
              textBox.setValue(std::to_string((int) (value * 100)));
          });
          slider.setFinalCallback([](float value) {
              cout << "Final slider value: " << (int) (value * 100) << endl;
          });
          textBox.setFixedSize(Vector2i(60,25));
          textBox.setFontSize(20);
          textBox.setAlignment(TextBox::Alignment::Right);
        }

        {
          auto& window = wdg<Window>("Misc. widgets");
          window.setPosition(Vector2i(425,15));
          window.setLayout(new GroupLayout());
          /*TabWidget* tabWidget = window->add<TabWidget>();

                  Widget* layer = tabWidget->createTab("Color Wheel");
                  layer->setLayout(new GroupLayout());

                  // Use overloaded variadic add to fill the tab widget with Different tabs.
                  layer->add<Label>("Color wheel widget", "sans-bold");
                  layer->add<ColorWheel>();

                  layer = tabWidget->createTab("Function Graph");
                  layer->setLayout(new GroupLayout());

                  layer->add<Label>("Function graph widget", "sans-bold");

                  Graph *graph = layer->add<Graph>("Some Function");

                  graph->setHeader("E = 2.35e-3");
                  graph->setFooter("Iteration 89");
                  VectorXf &func = graph->values();
                  func.resize(100);
                  for (int i = 0; i < 100; ++i)
                      func[i] = 0.5f * (0.5f * std::sin(i / 10.f) +
                                        0.5f * std::cos(i / 23.f) + 1);

                  // Dummy tab used to represent the last tab button.
                  tabWidget->createTab("+");

                  // A simple counter.
                  int counter = 1;
                  tabWidget->setCallback([tabWidget, this, counter] (int index) mutable {
                      if (index == (tabWidget->tabCount()-1)) {
                          // When the "+" tab has been clicked, simply add a new tab.
                          string tabName = "Dynamic " + to_string(counter);
                          Widget* layerDyn = tabWidget->createTab(index, tabName);
                          layerDyn->setLayout(new GroupLayout());
                          layerDyn->add<Label>("Function graph widget", "sans-bold");
                          Graph *graphDyn = layerDyn->add<Graph>("Dynamic function");

                          graphDyn->setHeader("E = 2.35e-3");
                          graphDyn->setFooter("Iteration " + to_string(index*counter));
                          VectorXf &funcDyn = graphDyn->values();
                          funcDyn.resize(100);
                          for (int i = 0; i < 100; ++i)
                              funcDyn[i] = 0.5f *
                                  std::abs((0.5f * std::sin(i / 10.f + counter) +
                                            0.5f * std::cos(i / 23.f + 1 + counter)));
                          ++counter;
                          // We must invoke perform layout from the screen instance to keep everything in order.
                          // This is essential when creating tabs dynamically.
                          performLayout();
                          // Ensure that the newly added header is visible on screen
                          tabWidget->ensureTabVisible(index);

                      }
                  });
                  tabWidget->setActiveTab(0);

                  // A button to go back to the first tab and scroll the window.
                  panel = window->add<Widget>();
                  panel->add<Label>("Jump to tab: ");
                  panel->setLayout(new BoxLayout(Orientation::Horizontal,
                                                 Alignment::Middle, 0, 6));

                  auto ib = panel->add<IntBox<int>>();
                  ib->setEditable(true);

                  b = panel->add<Button>("", ENTYPO_ICON_FORWARD);
                  b->setFixedSize(Vector2i(22, 22));
                  ib->setFixedHeight(22);
                  b->setCallback([tabWidget, ib] {
                      int value = ib->value();
                      if (value >= 0 && value < tabWidget->tabCount()) {
                          tabWidget->setActiveTab(value);
                          tabWidget->ensureTabVisible(value);
                      }
                  });*/
        }

        {
          auto& window = wdg<Window>("Grid of small widgets");
          window.withPosition({425, 288});
          auto* layout = new GridLayout(Orientation::Horizontal, 2,
                                         Alignment::Middle, 15, 5);
          layout->setColAlignment({ Alignment::Maximum, Alignment::Fill });
          layout->setSpacing(0, 10);
          window.setLayout(layout);

          window.add<Label>("Floating point :", "sans-bold");
          auto& textBox = window.wdg<TextBox>();
          textBox.setEditable(true);
          textBox.setFixedSize(Vector2i(100, 20));
          textBox.setValue("50");
          textBox.setUnits("GiB");
          textBox.setDefaultValue("0.0");
          textBox.setFontSize(16);
          textBox.setFormat("[-]?[0-9]*\\.?[0-9]+");

          window.add<Label>("Positive integer :", "sans-bold");
          auto& textBox2 = window.wdg<TextBox>();
          textBox2.setEditable(true);
          textBox2.setFixedSize(Vector2i(100, 20));
          textBox2.setValue("50");
          textBox2.setUnits("Mhz");
          textBox2.setDefaultValue("0.0");
          textBox2.setFontSize(16);
          textBox2.setFormat("[1-9][0-9]*");

          window.add<Label>( "Checkbox :", "sans-bold");

          window.wdg<CheckBox>("Check me")
                .withChecked(true)
                .withFontSize(16);

          window.add<Label>("Combo box :", "sans-bold");
          window.wdg<ComboBox>()
                .withItems(std::vector<std::string>{ "Item 1", "Item 2", "Item 3" })
                .withFontSize(16)
                .withFixedSize(Vector2i(100,20));

          window.add<Label>("Color button :", "sans-bold");
          auto& popupBtn = window.wdg<PopupButton>("", 0);
          popupBtn.setBackgroundColor(Color(255, 120, 0, 255));
          popupBtn.setFontSize(16);
          popupBtn.setFixedSize(Vector2i(100, 20));
          auto& popup = popupBtn.popup()->withLayout<GroupLayout>();

          ColorWheel& colorwheel = popup.wdg<ColorWheel>();
          colorwheel.setColor(popupBtn.backgroundColor());

          Button& colorBtn = popup.wdg<Button>("Pick");
          colorBtn.setFixedSize(Vector2i(100, 25));
          Color c = colorwheel.color();
          colorBtn.setBackgroundColor(c);

          colorwheel.setCallback([&colorBtn](const Color &value) {
              colorBtn.setBackgroundColor(value);
          });

          colorBtn.setChangeCallback([&colorBtn, &popupBtn](bool pushed) {
              if (pushed) {
                  popupBtn.setBackgroundColor(colorBtn.backgroundColor());
                  popupBtn.setPushed(false);
              }
          });
        }
        performLayout(mNVGContext);
    }

    ~TestWindow() {
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers)
    {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;

        //if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        // {
        //    setVisible(false);
        //    return true;
        //}
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        // Animate the scrollbar
        if( mProgress )
        {
          mProgress->setValue( mProgress->value() + 0.001f );
          if( mProgress->value() >= 1.f )
            mProgress->setValue( 0.f );
        }

        // Draw the user interface
        Screen::draw(ctx);
    }

    virtual void drawContents()
    {
    }
private:
    nanogui::ProgressBar *mProgress;
};

int main(int /* argc */, char ** /* argv */)
{
    SDL_Init(SDL_INIT_VIDEO);   // Initialize SDL2

    SDL_Window *window;        // Declare a pointer to an SDL_Window

#ifdef NANOVG_GL2_IMPLEMENTATION
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
#elif defined(NANOVG_GL3_IMPLEMENTATION)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
#elif defined(NANOVG_GLES2_IMPLEMENTATION)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
#elif defined(NANOVG_GLES3_IMPLEMENTATION)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
#endif
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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


    //SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );

    TestWindow *screen = new TestWindow( window, winWidth, winHeight );

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
            
            glViewport(0, 0, winWidth, winHeight);

            /*SDL_SetRenderDrawColor(renderer, 0xd3, 0xd3, 0xd3, 0xff );
            SDL_RenderClear( renderer );*/
            glClearColor(0.9f, 0.9f, 0.9f, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            screen->drawAll();

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
