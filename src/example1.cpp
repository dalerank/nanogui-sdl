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
#include <include/window.h>
#include <include/layout.h>
#include <include/label.h>
#include <include/checkbox.h>
#include <include/button.h>
#include <include/toolbutton.h>
#include <include/popupbutton.h>
#include <include/combobox.h>
#include <include/progressbar.h>
#include <include/entypo.h>
#include <include/messagedialog.h>
#include <include/textbox.h>
#include <include/slider.h>
#include <include/imagepanel.h>
#include <include/imageview.h>
#include <include/vscrollpanel.h>
#include <include/colorwheel.h>
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

using namespace nanogui;

class TestWindow : public nanogui::Screen
{
public:
    TestWindow( SDL_Window* pwindow, int rwidth, int rheight )
      : nanogui::Screen( pwindow, Eigen::Vector2i(rwidth, rheight), "SDL_gui Test")
      {
        using namespace nanogui;

        auto& nwindow = add<Window>("Button demo");
        nwindow.setPosition(Vector2i(15, 15));
        nwindow.setLayout(new GroupLayout());

        nwindow.add<Label>("Push buttons", "sans-bold");
        nwindow.add<Button>("Plain button").withCallback([] { cout << "pushed!" << endl; } );

        nwindow.add<Button>("Styled", ENTYPO_ICON_ROCKET).withCallback([] { cout << "pushed!" << endl; })
                                                         .withBackgroundColor( Color(0, 0, 255, 25) );

        nwindow.add<Label>("Toggle buttons", "sans-bold");
        nwindow.add<Button>("Toggle me").withFlags(Button::ToggleButton)
                                        .withChangeCallback([](bool state) { cout << "Toggle button state: " << state << endl; });

        Window* window = &nwindow;

        new Label(window, "Radio buttons", "sans-bold");
        auto button = new Button(window, "Radio button 1");
        button->setFlags(Button::RadioButton);
        button = new Button(window, "Radio button 2");
        button->setFlags(Button::RadioButton);

        new Label(window, "A tool palette", "sans-bold");
        Widget *tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));

        button = new ToolButton(tools, ENTYPO_ICON_CLOUD);
        button = new ToolButton(tools, ENTYPO_ICON_FF);
        button = new ToolButton(tools, ENTYPO_ICON_COMPASS);
        button = new ToolButton(tools, ENTYPO_ICON_INSTALL);

        new Label(window, "Popup buttons", "sans-bold");
        PopupButton *popupBtn = new PopupButton(window, "Popup", ENTYPO_ICON_EXPORT);
        Popup *popup = popupBtn->popup();
        popup->setLayout(new GroupLayout());
        new Label(popup, "Arbitrary widgets can be placed here");
        new CheckBox(popup, "A check box");
        popupBtn = new PopupButton(popup, "Recursive popup", ENTYPO_ICON_FLASH);
        popup = popupBtn->popup();
        popup->setLayout(new GroupLayout());
        new CheckBox(popup, "Another check box");

        window = new Window(this, "Basic widgets");
        window->setPosition(Vector2i(200, 15));
        window->setLayout(new GroupLayout());

        new Label(window, "Message dialog", "sans-bold");
        tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        button = new Button(tools, "Info");
        button->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Information, "Title", "This is an information message");
            dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
        });
        button = new Button(tools, "Warn");
        button->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a warning message");
            dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
        });
        button = new Button(tools, "Ask");
        button->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a question message", "Yes", "No", true);
            dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
        });

        std::vector<std::pair<int, std::string>>
            icons = loadImageDirectory(mNVGContext, "icons");

        new Label(window, "Image panel & scroll panel", "sans-bold");
        PopupButton *imagePanelBtn = new PopupButton(window, "Image Panel");
        imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
        popup = imagePanelBtn->popup();
        VScrollPanel *vscroll = new VScrollPanel(popup);
        ImagePanel *imgPanel = new ImagePanel(vscroll);
        imgPanel->setImages(icons);
        popup->setFixedSize(Vector2i(245, 150));

        auto img_window = new Window(this, "Selected image");
        img_window->setPosition(Vector2i(675, 15));
        img_window->setLayout(new GroupLayout());

        auto img = new ImageView(img_window);
        img->setPolicy(ImageView::SizePolicy::Expand);
        img->setFixedSize(Vector2i(300, 300));
        img->setImage(icons[0].first);
        imgPanel->setCallback([&, img, imgPanel, imagePanelBtn](int i) {
            img->setImage(imgPanel->images()[i].first); cout << "Selected item " << i << endl;
        });
        auto img_cb = new CheckBox(img_window, "Expand",
            [img](bool state) { if (state) img->setPolicy(ImageView::SizePolicy::Expand);
                                else       img->setPolicy(ImageView::SizePolicy::Fixed); });
        img_cb->setChecked(true);

        new Label(window, "File dialog", "sans-bold");
        tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        button = new Button(tools, "Open");
        button->setCallback([&] {
            cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, false) << endl;
        });
        button = new Button(tools, "Save");
        button->setCallback([&] {
            cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, true) << endl;
        });

        new Label(window, "Combo box", "sans-bold");
        new ComboBox(window, { "Combo box item 1", "Combo box item 2", "Combo box item 3"});
        new Label(window, "Check box", "sans-bold");
        CheckBox *cb = new CheckBox(window, "Flag 1",
            [](bool state) { cout << "Check box 1 state: " << state << endl; }
        );
        cb->setChecked(true);
        cb = new CheckBox(window, "Flag 2",
            [](bool state) { cout << "Check box 2 state: " << state << endl; }
        );
        new Label(window, "Progress bar", "sans-bold");
        mProgress = new ProgressBar(window);

        new Label(window, "Slider and text box", "sans-bold");

        Widget *panel = new Widget(window);
        panel->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 20));

        Slider *slider = new Slider(panel);
        slider->setValue(0.5f);
        slider->setFixedWidth(80);

        TextBox *textBox = new TextBox(panel);
        textBox->setFixedSize(Vector2i(60, 25));
        textBox->setValue("50");
        textBox->setUnits("%");
        slider->setCallback([textBox](float value) {
            textBox->setValue(std::to_string((int) (value * 100)));
        });
        slider->setFinalCallback([&](float value) {
            cout << "Final slider value: " << (int) (value * 100) << endl;
        });
        textBox->setFixedSize(Vector2i(60,25));
        textBox->setFontSize(20);
        textBox->setAlignment(TextBox::Alignment::Right);

        window = new Window(this,"Misc. widgets");
        window->setPosition(Vector2i(425,15));
        window->setLayout(new GroupLayout());
        new Label(window,"Color wheel","sans-bold");
        new ColorWheel(window);
        new Label(window, "Function graph", "sans-bold");
        Graph *graph = new Graph(window, "Some function");
        graph->setHeader("E = 2.35e-3");
        graph->setFooter("Iteration 89");
        VectorXf &func = graph->values();
        func.resize(100);
        for (int i = 0; i < 100; ++i)
            func[i] = 0.5f * (0.5f * std::sin(i / 10.f) +
                              0.5f * std::cos(i / 23.f) + 1);

        window = new Window(this, "Grid of small widgets");
        window->setPosition(Vector2i(425, 288));
        GridLayout *layout =
            new GridLayout(Orientation::Horizontal, 2,
                           Alignment::Middle, 15, 5);
        layout->setColAlignment(
            { Alignment::Maximum, Alignment::Fill });
        layout->setSpacing(0, 10);
        window->setLayout(layout);

        {
            new Label(window, "Floating point :", "sans-bold");
            textBox = new TextBox(window);
            textBox->setEditable(true);
            textBox->setFixedSize(Vector2i(100, 20));
            textBox->setValue("50");
            textBox->setUnits("GiB");
            textBox->setDefaultValue("0.0");
            textBox->setFontSize(16);
            textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
        }

        {
            new Label(window, "Positive integer :", "sans-bold");
            textBox = new TextBox(window);
            textBox->setEditable(true);
            textBox->setFixedSize(Vector2i(100, 20));
            textBox->setValue("50");
            textBox->setUnits("Mhz");
            textBox->setDefaultValue("0.0");
            textBox->setFontSize(16);
            textBox->setFormat("[1-9][0-9]*");
        }

        {
            new Label(window, "Checkbox :", "sans-bold");

            cb = new CheckBox(window, "Check me");
            cb->setFontSize(16);
            cb->setChecked(true);
        }

        new Label(window, "Combo box :", "sans-bold");
        ComboBox *cobo =
            new ComboBox(window, { "Item 1", "Item 2", "Item 3" });
        cobo->setFontSize(16);
        cobo->setFixedSize(Vector2i(100,20));

        new Label(window, "Color button :", "sans-bold");
        popupBtn = new PopupButton(window, "", 0);
        popupBtn->setBackgroundColor(Color(255, 120, 0, 255));
        popupBtn->setFontSize(16);
        popupBtn->setFixedSize(Vector2i(100, 20));
        popup = popupBtn->popup();
        popup->setLayout(new GroupLayout());

        ColorWheel *colorwheel = new ColorWheel(popup);
        colorwheel->setColor(popupBtn->backgroundColor());

        Button *colorBtn = new Button(popup, "Pick");
        colorBtn->setFixedSize(Vector2i(100, 25));
        Color c = colorwheel->color();
        colorBtn->setBackgroundColor(c);

        colorwheel->setCallback([colorBtn](const Color &value) {
            colorBtn->setBackgroundColor(value);
        });

        colorBtn->setChangeCallback([colorBtn, popupBtn](bool pushed) {
            if (pushed) {
                popupBtn->setBackgroundColor(colorBtn->backgroundColor());
                popupBtn->setPushed(false);
            }
        });

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
        mProgress->setValue( 0.5f );

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
