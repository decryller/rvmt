#include "rvmt.hpp"
#include <iostream>
#include <thread>

// How this example works.
//	RVMT::Start() sets some necessary terminal config up.
//	In the main loop | while (!quit)
//		Internal variables are prepared to render a new frame by a function | RVMT::BeginFrame()
//		Stuff is written | widgets and things.
//		Stuff is rendered | RVMT::Render.
//		A function stalls the loop until a new input is detected | RVMT::WaitForNewInput()
// RVMT::Stop() resets the terminal to its previous settings.

enum GUIPage {
		GUIPage_Button,
		GUIPage_Checkbox,
		GUIPage_Slider,
		GUIPage_InputText,
	};
    
GUIPage GUIPage_Current = GUIPage_Button;

int main() {
    RVMT::Start();
    RVMT::SetTerminalTitle("RVMT v2.0 Example");

    bool quit = false;
    while (!quit) {

        RVMT::BeginFrame();

        const unsigned short rowCount = RVMT::GetRowCount();
        const unsigned short colCount = RVMT::GetColCount();

        // === Bottom bar
        RVMT::SetCursorY(NewCursorPos_ABSOLUTE, rowCount - 3);
        RVMT::SetCursorX(NewCursorPos_ABSOLUTE, 0);
        if (RVMT::Button("Button"))
			GUIPage_Current = GUIPage_Button;

        RVMT::SameLine();
        if (RVMT::Button("Checkbox"))
			GUIPage_Current = GUIPage_Checkbox;

        RVMT::SameLine();
        if (RVMT::Button("Slider"))
			GUIPage_Current = GUIPage_Slider;

		RVMT::SameLine();
        if (RVMT::Button("InputText"))
			GUIPage_Current = GUIPage_InputText;

        RVMT::SameLine();
        RVMT::SetCursorX(NewCursorPos_ABSOLUTE, colCount - 8);
        if (RVMT::Button(" Quit "))
            quit = true;
        
        RVMT::SetCursorY(NewCursorPos_ABSOLUTE, 0);
        RVMT::SetCursorX(NewCursorPos_ABSOLUTE, 0);

        switch (GUIPage_Current) {
			case GUIPage_Button:
                static bool textOnlyCB = false;
                static unsigned int totalClicks = 0;

                RVMT::Text("RVMT::Button | When clicked, return true.\n");

                RVMT::Text("WidgetProp_Button_TextOnly ");
                RVMT::SameLine();
                RVMT::Checkbox("[Disable]", "[Enable]", &textOnlyCB);
                RVMT::Text("Draw the button as text only, no box.\n");


                if (textOnlyCB)
                    RVMT::PushPropertyForNextItem(WidgetProp_Button_TextOnly);

                if (RVMT::Button("The button"))
                    totalClicks++;

                RVMT::Text("%i clicks.", totalClicks);
                break;

            case GUIPage_Checkbox:
                static bool sampleCB = false;
                RVMT::Text("RVMT::Checkbox | When clicked, \"invert\" the assigned bool.\n");

                RVMT::Checkbox("[trueText]", "[falseText]", &sampleCB);
                break;

            case GUIPage_Slider:
                static float sampleFloatSlider = 3.0;
                RVMT::Text("RVMT::Slider | A draggable slider.\n");
                
                RVMT::Text("RVMT::Slider ");
				RVMT::SameLine();
				RVMT::Slider("sample float slider", 10, 3.0, 1.31, &sampleFloatSlider);

                RVMT::SameLine();
                RVMT::Text(" %.2f", sampleFloatSlider);
                break;
				
			case GUIPage_InputText:
				static char sampleInputChar[33];
                static bool censorOutputCB = false;
                static bool customCharsetCB = false;
                static bool customPlaceholderCB = false;

				RVMT::Text("RVMT::InputText | A field to write text into");

				RVMT::Text("WidgetProp_InputText_Censor ");
                RVMT::SameLine();
                RVMT::Checkbox("[Disable]", "[Enable]", &censorOutputCB);
				RVMT::Text("Censor typed characters, print an asterisk instead of the actual characters.\n");

				RVMT::Text("WidgetProp_InputText_Charset (Numbers only) ");
                RVMT::SameLine();
                RVMT::Checkbox("[Disable]", "[Enable]", &customCharsetCB);
				RVMT::Text("Provide a custom charset so only characters in the charset can be typed.\n");

				RVMT::Text("WidgetProp_InputText_Placeholder (hello) ");
                RVMT::SameLine();
                RVMT::Checkbox("[Disable]", "[Enable]", &customPlaceholderCB);
				RVMT::Text("Provide a placeholder that will be shown in the field when it's empty and not active.\n");

                if (censorOutputCB)
                    RVMT::PushPropertyForNextItem(WidgetProp_InputText_Censor);
                
				if (customCharsetCB)
                    RVMT::PushPropertyForNextItem(WidgetProp_InputText_Charset, "0123456789");
				
				if (customPlaceholderCB)
                    RVMT::PushPropertyForNextItem(WidgetProp_InputText_Placeholder, "hello");

				RVMT::InputText("sampleInputChar field", sampleInputChar, 32, 16);
			break;
		}

        RVMT::Render();
        RVMT::WaitForNewInput();
    }

    std::wcout << "\nExited main loop."; std::wcout.flush();
    RVMT::Stop();

    return 0;
}