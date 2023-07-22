#include "rvmt.hpp"
#include <iostream>
#include <thread>

#define CHRONOELAPSEDMS(TIMEPOINT) (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - TIMEPOINT).count())

int main() {
    RVMT::Start();
    
    auto lastInputTime = std::chrono::system_clock::now();
    bool quit = false;

    while (!quit) {
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        RVMT::RegisterInput();

        if (RVMT::internal::NEWINPUT_MOUSE1HELD ||
            RVMT::internal::PREVINPUT_TERMWIDTH != RVMT::internal::NEWINPUT_TERMWIDTH ||
            RVMT::internal::PREVINPUT_TERMHEIGHT != RVMT::internal::NEWINPUT_TERMHEIGHT)
            lastInputTime = std::chrono::system_clock::now();
        
        if (CHRONOELAPSEDMS(lastInputTime) > 200) {
            continue;
        }

        const unsigned short rowCount = RVMT::internal::rowCount;
        const unsigned short colCount = RVMT::internal::colCount;

        RVMT::Text("RVMT. Revamped Terminal.\nExample arrangement.\n");

        RVMT::Text("An unwanted Y Offset might be experienced\ndue to some terminal decorations, such\nas the menu bar in xfce4-terminal\n");
        RVMT::Text("A draggable slider between 1.3 and 1.4");

        static float sampleSlider = 1.0;
        RVMT::Slider("theSlider", 15, 1.3, 1.4, &sampleSlider);
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

            default:
                RVMT::Text("unknown... (?)");
                break;
        }

        RVMT::Text("Active Item ID: %s", RVMT::internal::activeItemID);

        RVMT::cursorX = 46;
        RVMT::cursorY = 0;

        BoxStyle_Current = BoxStyle_Simple;
        RVMT::Button(" BoxStyle_Simple ");

        BoxStyle_Current = BoxStyle_Bold;
        RVMT::Button(" BoxStyle_Bold ");

        BoxStyle_Current = BoxStyle_DoubleLine;
        RVMT::Button(" BoxStyle_DoubleLine ");

        BoxStyle_Current = BoxStyle_Round;
        RVMT::Button(" BoxStyle_Round ");

        RVMT::cursorX = colCount - 8;
        RVMT::cursorY = rowCount - 3;
        if (RVMT::Button(" Quit "))
            quit = true;

        RVMT::Render();
    }

    std::wcout << "\nExited main loop.\n"; std::wcout.flush();
    return 0;
}