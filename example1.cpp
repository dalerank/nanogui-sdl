/*
    sdlgui/example1.cpp -- C++ version of an example application that shows 
    how to use the various widget classes. 

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/screen.h>
#include <sdlgui/window.h>
#include <sdlgui/layout.h>
#include <sdlgui/label.h>
#include <sdlgui/checkbox.h>
#include <sdlgui/button.h>
#include <sdlgui/toolbutton.h>
#include <sdlgui/popupbutton.h>
#include <sdlgui/combobox.h>
#include <sdlgui/dropdownbox.h>
#include <sdlgui/progressbar.h>
#include <sdlgui/entypo.h>
#include <sdlgui/messagedialog.h>
#include <sdlgui/textbox.h>
#include <sdlgui/slider.h>
#include <sdlgui/imagepanel.h>
#include <sdlgui/imageview.h>
#include <sdlgui/vscrollpanel.h>
#include <sdlgui/colorwheel.h>
#include <sdlgui/graph.h>
#include <sdlgui/tabwidget.h>
#include <sdlgui/switchbox.h>
#include <sdlgui/formhelper.h>
#include <memory>

#if defined(_WIN32)
#include <windows.h>
#endif
#include <iostream>

#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#if defined(_WIN32)
#include <SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif

using std::cout;
using std::cerr;
using std::endl;

#undef main

using namespace sdlgui;

class TestWindow : public Screen
{
public:
    TestWindow( SDL_Window* pwindow, int rwidth, int rheight )
      : Screen( pwindow, Vector2i(rwidth, rheight), "SDL_gui Test")
      {
        {
          auto& nwindow = window("Button demo", Vector2i{15, 15})
                            .withLayout<GroupLayout>();

          nwindow.label("Push buttons", "sans-bold")._and()
                 .button("Plain button", [] { cout << "pushed!" << endl; })
                    .withTooltip("This is plain button tips");

          nwindow.button("Styled", ENTYPO_ICON_ROCKET, [] { cout << "pushed!" << endl; })
                   .withBackgroundColor( Color(0, 0, 255, 25) );

          nwindow.label("Toggle buttons", "sans-bold")._and()
                 .button("Toggle me", [](bool state) { cout << "Toggle button state: " << state << endl; })
                    .withFlags(Button::ToggleButton);

          nwindow.label("Radio buttons", "sans-bold")._and()
                 .button("Radio button 1")
                    .withFlags(Button::RadioButton);

          nwindow.button("Radio button 2")
                    .withFlags(Button::RadioButton)._and()
                 .label("A tool palette", "sans-bold");

          auto& tools = nwindow.widget().boxlayout(Orientation::Horizontal, Alignment::Middle, 0, 6);

          tools.toolbutton(ENTYPO_ICON_CLOUD)._and()
               .toolbutton(ENTYPO_ICON_FF)._and()
               .toolbutton(ENTYPO_ICON_COMPASS)._and()
               .toolbutton(ENTYPO_ICON_INSTALL);  

          nwindow.label("Popup buttons", "sans-bold")._and()
                 .popupbutton("Popup", ENTYPO_ICON_EXPORT)
                    .popup()
                      .withLayout<GroupLayout>()
                         .label("Arbitrary widgets can be placed here")._and()
                         .checkbox("A check box")._and()
                      .popupbutton("Recursive popup", ENTYPO_ICON_FLASH).popup()
                         .withLayout<GroupLayout>()
                           .checkbox("Another check box");
        }

        ListImages images = loadImageDirectory(SDL_GetRenderer(pwindow), "icons");

        {
          auto& pwindow = window("Basic widgets", Vector2i{ 200, 15 }).withLayout<GroupLayout>();
                          
          pwindow.label("Message dialog", "sans-bold")._and()
                 .widget()
                    .boxlayout(Orientation::Horizontal,Alignment::Middle, 0, 6)
                      .button("Info", [&] {
                            msgdialog(MessageDialog::Type::Information, "Title", "This is an information message",
                                      [](int result) { cout << "Dialog result: " << result << endl; }); })._and()
                      .button("Warn", [&] {
                            msgdialog(MessageDialog::Type::Warning, "Title", "This is a warning message",
                                      [](int result) { cout << "Dialog result: " << result << endl; }); })._and()
                      .button("Ask", [&] {
                            msgdialog(MessageDialog::Type::Warning, "Title", "This is a question message", 
                                      "Yes", "No", true, [](int result) { cout << "Dialog result: " << result << endl; }); });

          pwindow.label("Image panel & scroll panel", "sans-bold");
          auto& imagePanelBtn = pwindow.popupbutton("Image Panel", ENTYPO_ICON_FOLDER);

          // Load all of the images by creating a GLTexture object and saving the pixel data.
          mCurrentImage = 0;
          for (auto& icon : images) mImagesData.emplace_back(icon.tex);

          auto& img_window = window("Selected image", Vector2i(675, 15));
          img_window.withLayout<GroupLayout>();
          
          auto imageView = img_window.add<ImageView>(mImagesData[0]);

          imagePanelBtn.popup(Vector2i(245, 150))
                         .vscrollpanel()
                           .imgpanel(images)
                             .setCallback([this, imageView](int i)
                             {
                               if (i >= mImagesData.size())
                                 return;
                               imageView->bindImage(mImagesData[i]);
                               mCurrentImage = i;
                               cout << "Selected item " << i << '\n';
                             });


          // Change the active textures.
          
          imageView->setGridThreshold(20);
          imageView->setPixelInfoThreshold(20);
          imageView->setPixelInfoCallback(
              [this, imageView](const Vector2i& index) -> std::pair<std::string, Color>
              {
                void *pixels;
                int pitch, w, h;
                SDL_QueryTexture(mImagesData[mCurrentImage], nullptr, nullptr, &w, &h);

                SDL_LockTexture(mImagesData[mCurrentImage], nullptr, &pixels, &pitch);
                Uint32 *imageData = (Uint32*)pixels;

                std::string stringData;
                uint16_t channelSum = 0;
                for (int i = 0; i != 4; ++i) 
                {
                    uint8_t *data = (uint8_t*)imageData;
                    auto& channelData = data[4*index.y*w + 4*index.x + i];
                    channelSum += channelData;
                    stringData += (std::to_string(static_cast<int>(channelData)) + "\n");
                }
                float intensity = static_cast<float>(255 - (channelSum / 4)) / 255.0f;
                float colorScale = intensity > 0.5f ? (intensity + 1) / 2 : intensity / 2;
                Color textColor = Color(colorScale, 1.0f);
                SDL_UnlockTexture(mImagesData[mCurrentImage]);
                return { stringData, textColor };
              }
          );

          pwindow.label("File dialog", "sans-bold")._and()
                 .widget()
                       .boxlayout(Orientation::Horizontal, Alignment::Middle, 0, 6 )
                         .button("Open", [&] {
                                cout << "File dialog result: " << file_dialog(
                                { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, false) << endl;
                              })._and()
                         .button("Save", [&] {
                                cout << "File dialog result: " << file_dialog(
                                { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, true) << endl;
                              });

          pwindow.label("Combo box", "sans-bold")._and()
                 .dropdownbox(std::vector<std::string>{ "Dropdown item 1", "Dropdown item 2", "Dropdown item 3" })._and()
                 .combobox(std::vector<std::string>{ "Combo box item 1", "Combo box item 2", "Combo box item 3"})._and()
                 .label("Check box", "sans-bold")._and()
                 .checkbox("Flag 1", [](bool state) { cout << "Check box 1 state: " << state << endl; })
                     .withChecked(true)._and()
                 .checkbox("Flag 2", [](bool state) { cout << "Check box 2 state: " << state << endl; })._and()
                 .label("Progress bar", "sans-bold")._and()
                 .progressbar().withId("progressbar")._and()
                 .label("Slider and text box", "sans-bold")._and()
                 .widget().withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Middle, 0, 20)
                    .slider(0.5f, [](Slider* obj, float value) {
                      if (auto* textBox = obj->gfind<TextBox>("slider-textbox"))
                        textBox->setValue(std::to_string((int)(value * 100)));
                    }, [](float value) { cout << "Final slider value: " << (int)(value * 100) << endl; })
                        .withFixedWidth(80)._and()
                 .textbox("50", "%").withAlignment(TextBox::Alignment::Right)
                    .withId("slider-textbox")
                    .withFixedSize(Vector2i(60, 25))
                    .withFontSize(20);

          pwindow.label("A switch boxes", "sans-bold");
          Widget *swbx = new Widget(&pwindow);
          swbx->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 2));

          auto* swbh = new SwitchBox(swbx, SwitchBox::Alignment::Horizontal, "");
          swbh->setFixedSize(Vector2i(84, 32));
          new SwitchBox(swbx, SwitchBox::Alignment::Vertical, "");
        }

        {
          auto& window = wdg<Window>("Misc. widgets");
          window.setPosition(425,15);
          window.setFixedSize({ 400, 300 });
          window.setLayout(new GroupLayout());
          TabWidget* tabWidget = window.add<TabWidget>();

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
          std::vector<float> &func = graph->values();
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
                  std::string tabName = "Dynamic " + std::to_string(counter);
                  Widget* layerDyn = tabWidget->createTab(index, tabName);
                  layerDyn->setLayout(new GroupLayout());
                  layerDyn->add<Label>("Function graph widget", "sans-bold");
                  Graph *graphDyn = layerDyn->add<Graph>("Dynamic function");

                  graphDyn->setHeader("E = 2.35e-3");
                  graphDyn->setFooter("Iteration " + std::to_string(index*counter));
                  std::vector<float> &funcDyn = graphDyn->values();
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
          auto& panel = window.wdg<Widget>();
          panel.add<Label>("Jump to tab: ");
          panel.setLayout(new BoxLayout(Orientation::Horizontal,
                                         Alignment::Middle, 0, 6));

          auto ib = panel.add<IntBox<int>>();
          ib->setEditable(true);

          auto b = panel.add<Button>("", ENTYPO_ICON_FORWARD);
          b->setFixedSize(Vector2i(22, 22));
          ib->setFixedHeight(22);
          b->setCallback([tabWidget, ib] {
              int value = ib->value();
              if (value >= 0 && value < tabWidget->tabCount()) {
                  tabWidget->setActiveTab(value);
                  tabWidget->ensureTabVisible(value);
              }
          });
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
          auto& popup = popupBtn.popup().withLayout<GroupLayout>();

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
        performLayout(mSDL_Renderer);
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

    virtual void draw(SDL_Renderer* renderer)
    {
      if (auto pbar = gfind<ProgressBar>("progressbar"))
      {
        pbar->setValue(pbar->value() + 0.001f);
        if (pbar->value() >= 1.f)
          pbar->setValue(0.f);
      }

      Screen::draw(renderer);
    }

    virtual void drawContents()
    {
    }
private:
    std::vector<SDL_Texture*> mImagesData;
    int mCurrentImage;
};


class Fps
{
public:
  explicit Fps(int tickInterval = 30)
      : m_tickInterval(tickInterval)
      , m_nextTime(SDL_GetTicks() + tickInterval)
  {
  }

  void next()
  {
    SDL_Delay(getTicksToNextFrame());

    m_nextTime += m_tickInterval;
  }

private:
  const int m_tickInterval;
  Uint32 m_nextTime;

  Uint32 getTicksToNextFrame() const
  {
    Uint32 now = SDL_GetTicks();

    return (m_nextTime <= now) ? 0 : m_nextTime - now;
  }
};


int main(int /* argc */, char ** /* argv */)
{
    char rendername[256] = {0};
    SDL_RendererInfo info;

    SDL_Init(SDL_INIT_VIDEO);   // Initialize SDL2

    SDL_Window *window;        // Declare a pointer to an SDL_Window

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
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN  | SDL_WINDOW_ALLOW_HIGHDPI        //    Uint32 flags: window options, see docs
    );

    // Check that the window was successfully made
    if(window==NULL){
      // In the event that the window could not be made...
      std::cout << "Could not create window: " << SDL_GetError() << '\n';
      SDL_Quit();
      return 1;
    }

    auto context = SDL_GL_CreateContext(window);

    for (int it = 0; it < SDL_GetNumRenderDrivers(); it++) {
        SDL_GetRenderDriverInfo(it, &info);
        strcat(rendername, info.name);
        strcat(rendername, " ");
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TestWindow *screen = new TestWindow(window, winWidth, winHeight);

    Fps fps;

    bool quit = false;
    try
    {
        SDL_Event e;
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

            // Render the rect to the screen
            SDL_RenderPresent(renderer);

            fps.next();
        }
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
