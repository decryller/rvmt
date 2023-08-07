#include "rvmt.hpp"
#include <iostream>
#include <thread>

// How this example works.
// RVMT::Start() starts the input handling threads and allocate some X11 resources.
// In the main loop | while (!quit)
//    A condition waits until a new render is requested. | while (RVMT::renderRequests.size() == 0)
//    Once a new render is requested, widgets will get sent to the drawlist | Text/Button/Etc.
//    Drawlist contents will get printed to the terminal by RVMT::Render().
//    Repeat.
// RVMT::Stop() stops the input threads and releases X11 Resources.

int main() {
    RVMT::Start();
    bool quit = false;
    while (!quit) {

        while (RVMT::renderRequests.size() == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));

        // Consume render request.
        RVMT::renderRequests.erase(RVMT::renderRequests.begin());

        const unsigned short rowCount = RVMT::internal::rowCount;
        const unsigned short colCount = RVMT::internal::colCount;

        // === Widget showcase
        RVMT::Text("RVMT. Revamped Terminal.\nExample arrangement.\n");

        RVMT::Text("An unwanted Y Offset might be experienced\ndue to some terminal decorations, such\nas the menu bar in xfce4-terminal\n");
        RVMT::Text("A draggable slider between 1.3 and 1.4");

        static float sampleSlider = 1.0;
        RVMT::Slider("sample slider", 15, 1.3, 1.4, &sampleSlider);
        RVMT::SameLine();
        RVMT::Text(" Its value is %.4f", sampleSlider);

        static unsigned int clickCount = 0;
        if (RVMT::Button(" A button "))
            clickCount++;

        if (clickCount > 0) {
            RVMT::SameLine();
            if (clickCount == 1)
                RVMT::Text("\nthat has been clicked 1 time.");

            else if (clickCount > 1)
                RVMT::Text("\nthat has been clicked %i times.", clickCount);
        }

        static bool checkboxValue = false;
        RVMT::Text("Click this checkbox --> ");
        RVMT::SameLine();
        RVMT::Checkbox("[True]", "[False]", &checkboxValue);

        static char inputFieldCharArr[65]; // +1 for a null-terminated string
        RVMT::SetCursorY(NewCursorPos_ADD, 1);
        RVMT::Text("An input field using\nchar inputFieldCharArr[65]");
        RVMT::InputText("sample input text using a char array", inputFieldCharArr, 64, 24);

        static std::string inputFieldSTDString(64, 0);
        RVMT::SetCursorY(NewCursorPos_ADD, 1);
        RVMT::Text("An input field using\nstd::string inputFieldSTDString");
        RVMT::InputText("sample input text using std::string", &inputFieldSTDString[0], 64, 24);

        // === Box styles
        RVMT::SetCursorX(NewCursorPos_ABSOLUTE, 46);
        RVMT::SetCursorY(NewCursorPos_ABSOLUTE, 0);

        BoxStyle_Current = BoxStyle_Simple;
        RVMT::Button(" BoxStyle_Simple ");
        RVMT::DrawHSeparator(72, 1, 1);
        RVMT::DrawVSeparator(71, 0, 1);

        BoxStyle_Current = BoxStyle_Bold;
        RVMT::Button(" BoxStyle_Bold ");
        RVMT::DrawHSeparator(72, 4, 1);
        RVMT::DrawVSeparator(71, 3, 1);

        BoxStyle_Current = BoxStyle_DoubleLine;
        RVMT::Button(" BoxStyle_DoubleLine ");
        RVMT::DrawHSeparator(72, 7, 1);
        RVMT::DrawVSeparator(71, 6, 1);

        BoxStyle_Current = BoxStyle_Round;
        RVMT::Button(" BoxStyle_Round ");
        RVMT::DrawHSeparator(72, 10, 1);
        RVMT::DrawVSeparator(71, 9, 1);
        
        // SameLine testing
        static char BXChar[5]{0,0,0,0,0};
        static bool CXBool = true;

        RVMT::Text("SameLine testing");
        // Buttons
        RVMT::Button("A0");

        RVMT::SameLine();
        RVMT::Button("A1");
        
        RVMT::SameLine();
        RVMT::Button("A2");

        RVMT::SameLine();
        RVMT::Button("A3");

        RVMT::InputText("B1", BXChar, 4, 4);

        RVMT::SameLine();
        RVMT::InputText("B2", BXChar, 4, 4);

        RVMT::SameLine();
        RVMT::InputText("B3", BXChar, 4, 4);

        RVMT::SameLine();
        RVMT::InputText("B4", BXChar, 4, 4);

        RVMT::Checkbox("[CT]", "[CF]", &CXBool);
        
        RVMT::SameLine();
        RVMT::Checkbox("[CT]", "[CF]", &CXBool);

        RVMT::SameLine();
        RVMT::Checkbox("[CT]", "[CF]", &CXBool);

        RVMT::SameLine();
        RVMT::Checkbox("[CT]", "[CF]", &CXBool);

        RVMT::Text("D1");

        RVMT::SameLine();
        RVMT::Text("D2");

        RVMT::SameLine();
        RVMT::Text("D3");

        RVMT::SameLine();
        RVMT::Text("D4");

        static unsigned int frameCount;
        RVMT::Text("Frame count: %i", frameCount++);

        RVMT::Text("Active Item Type: ");
        RVMT::SameLine();

        // This is mainly debug info, so I'm not ashamed of this looking ugly.
        switch (RVMT::internal::activeItemType) {
            case RVMT::internal::ItemType_None:
                RVMT::Text("ItemType_None (%i)", RVMT::internal::ItemType_None);
                break;

            case RVMT::internal::ItemType_Slider:
                RVMT::Text("ItemType_Slider (%i)", RVMT::internal::ItemType_Slider);
                break;

            case RVMT::internal::ItemType_Button:
                RVMT::Text("ItemType_Button (%i)", RVMT::internal::ItemType_Button);
                break;

            case RVMT::internal::ItemType_Checkbox:
                RVMT::Text("ItemType_Checkbox (%i)", RVMT::internal::ItemType_Checkbox);
                break;

            case RVMT::internal::ItemType_InputText:
                RVMT::Text("ItemType_InputText (%i)", RVMT::internal::ItemType_InputText);
                break;

            default:
                RVMT::Text("unknown... (?)");
                break;
        }

        RVMT::Text("Active Item ID: %s", RVMT::internal::activeItemID);

        RVMT::SetCursorX(NewCursorPos_ABSOLUTE, colCount - 8);
        RVMT::SetCursorY(NewCursorPos_ABSOLUTE, rowCount - 3);

        if (RVMT::Button(" Quit "))
            quit = true;

        RVMT::Render();
    }

    std::wcout << "\nExited main loop."; std::wcout.flush();
    RVMT::Stop();

    return 0;
}