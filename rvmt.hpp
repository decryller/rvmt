#ifndef RVMT_HPP
#define RVMT_HPP

#include <vector>
#include <sstream>
#include <X11/Xlib.h>

enum BoxStyle_ {
    BoxStyle_Simple,
    BoxStyle_Bold,
    BoxStyle_DoubleLine,
    BoxStyle_Round
};

extern int BoxStyle_Current;

namespace RVMT {

    // All of these variables could perfectly be inside the cpp, but I'd rather put them in an additional
    // namespace to make debug easier.
    namespace internal {
        
        enum ItemType_ {
            ItemType_None       = 0,
            ItemType_Slider     = 1,
            ItemType_Button     = 2,
            ItemType_Checkbox   = 3,
            ItemType_InputText  = 4
        };

        struct drawableElement {
            int x = 0;
            int y = 0;
            wchar_t ch = 0;
        };

        struct keyPress {
            char key;
            const char* field;

            keyPress operator=(keyPress from) const {
                return {from.key, from.field};
            }
        };

        // I need to tidy this up.
        extern std::vector<drawableElement> drawList; // Vector of "drawableElement" struct.
        extern std::vector<std::wstring> canvas; // Map to which drawlist's content will go to
        extern std::wostringstream preScreen; // Temporal buffer used to print the canvas all at once.

        extern unsigned short rowCount; // Terminal's row count (Y Axis).
        extern unsigned short colCount; // Terminal's column count (X Axis).

        extern int NEWINPUT_MOUSEWINX; // Mouse X Position inside the window.
        extern int NEWINPUT_MOUSEWINY; // Mouse Y Position inside the window.

        extern int NEWINPUT_MOUSEROW; // Row the mouse is at.
        extern int NEWINPUT_MOUSECOL; // Column the mouse is at.

        extern bool PREVINPUT_MOUSE1HELD; // Past state of NEWINPUT_MOUSE1HELD.
        extern bool NEWINPUT_MOUSE1HELD; // Is Mouse button 1 being pressed?

        extern int NEWINPUT_TERMX; // Terminal's window X Position.
        extern int NEWINPUT_TERMY; // Terminal's window Y Position.

        extern unsigned int NEWINPUT_TERMWIDTH; // Terminal's window width.
        extern unsigned int NEWINPUT_TERMHEIGHT; // Terminal's window height.

        extern unsigned int PREVINPUT_TERMWIDTH; // Terminal's previous window width.
        extern unsigned int PREVINPUT_TERMHEIGHT; // Terminal's previous window height.

        extern int NEWINPUT_CELLWIDTH; // Terminal's Cell width. (Not perfectly accurate)

        extern Display* rootDisplay; // XOpenDisplay(NULL) | Cleared by Stop().
        extern Window rootWindow; // DefaultRootWindow(rootDisplay)

        extern Window termX11Win; // Terminal's X11 Window ID

        extern bool sameLineCalled;
        extern int sameLineX;
        extern int sameLineY;

        extern int sameLineXRevert;
        extern int sameLineYRevert;

        extern ItemType_ activeItemType;
        extern const char* activeItemID;

        extern int _NULLINT;
        extern unsigned int _NULLUINT;
        extern Window _NULLX11WINDOW;

        extern bool startCalled;
        extern bool stopCalled; // Used to notify threads to exit their main loops.

        extern std::vector<keyPress> KEYPRESSES;
    }
    
    extern std::vector<bool> renderRequests;

    extern int cursorX;
    extern int cursorY;

    // !=== Widgets ===!
    // === Checkbox
    // === Button
    // === Slider
    // === InputText

    // Print a checkbox.
    // When val is false, it will print falseText.
    // When val is true, it will print trueText.
    // Returns true if the text is clicked.
    bool Checkbox(const char* trueText, const char* falseText, bool* val);

    // Print a button.
    // Uses "BoxStyle_Current" for its style.
    // Returns true if the box is clicked.
    bool Button(const char* text, ...);

    // Print a draggable slider.
    // Returns true if it's clicked
    bool Slider(const char* sliderID, int length, float minVal, float maxVal, float* var);

    // Print a text input field.
    // Returns true if it's clicked
    bool InputText(const char* fieldID, char* val, unsigned int maxStrSize, int width);

    // !=== Drawing ===!
    // === Text
    // === DrawBox
    // === DrawHSeparator
    // === DrawVSeparator
    // === SameLine

    // Print text.
    void Text(const char* val, ...);

    // Draw a box.
    // Does not modify the cursor.
    // Gets style from BoxStyle_Current.
    void DrawBox(int x, int y, int width, int height);

    // Draw a horizontal separator.
    // Does not modify the cursor.
    // Gets style from BoxStyle_Current.
    void DrawHSeparator(int x, int y, int width);

    // Draw a vertical separator.
    // Does not modify the cursor.
    // Gets style from BoxStyle_Current.
    void DrawVSeparator(int x, int y, int height);

    // Move cursor to the previous element's right.
    void SameLine();
    
    // !=== Internal ===!
    // === Render
    // === Start
    // === Stop

    // Render drawlist's contents.
    void Render();

    // Start.
    // RVMT will treat the window that is focused when this function gets called as the main window.
    void Start();

    // Stop.
    void Stop();
}
#endif