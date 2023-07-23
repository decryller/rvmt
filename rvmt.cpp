#include "rvmt.hpp"

#include <cmath>
#include <iostream>
#include <cstdarg>
#include <sstream>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>

bool strEquals(const char* first, const char* second) {
    for (int i = 0; i < 4096; i++) {
        if (first[i] == 0 && second[i] == 0)
            return true;

        if (first[i] != second[i])
            return false;
    }
    return false;
}

int strLength(const char* str) {
    if (str[0] == 0)
        return 0;

    for (int i = 0; i < 4096; i++) 
        if (str[i] == 0)
            return i;
    
    return 4096;
}

// Start extern variables.
    std::vector<RVMT::internal::drawableElement> RVMT::internal::drawList{0};
    std::vector<std::wstring> RVMT::internal::canvas{L""};
    std::wostringstream RVMT::internal::preScreen{L""};

    unsigned short RVMT::internal::rowCount = 0;
    unsigned short RVMT::internal::colCount = 0;

    int RVMT::internal::NEWINPUT_MOUSEWINX = false;
    int RVMT::internal::NEWINPUT_MOUSEWINY = false;
    int RVMT::internal::NEWINPUT_MOUSEROW = false;
    int RVMT::internal::NEWINPUT_MOUSECOL = false;
    bool RVMT::internal::PREVINPUT_MOUSE1HELD = false;
    bool RVMT::internal::NEWINPUT_MOUSE1HELD = false;

    int RVMT::internal::NEWINPUT_TERMX = false;
    int RVMT::internal::NEWINPUT_TERMY = false;
    unsigned int RVMT::internal::NEWINPUT_TERMWIDTH = false;
    unsigned int RVMT::internal::PREVINPUT_TERMWIDTH = false;
    unsigned int RVMT::internal::NEWINPUT_TERMHEIGHT = false;
    unsigned int RVMT::internal::PREVINPUT_TERMHEIGHT = false;

    int RVMT::internal::NEWINPUT_CELLWIDTH = false;

    bool RVMT::internal::sameLineCalled = false;
    int RVMT::internal::sameLineX = 0;
    int RVMT::internal::sameLineXRevert = 0;
    int RVMT::internal::sameLineY = 0;
    int RVMT::internal::sameLineYRevert = 0;

    Display* RVMT::internal::rootDisplay = nullptr;
    Window RVMT::internal::rootWindow = 0;
    Window RVMT::internal::termX11Win = 0;

    int RVMT::internal::_NULLINT = 0;
    unsigned int RVMT::internal::_NULLUINT = 0;
    Window RVMT::internal::_NULLX11WINDOW = 0;

    RVMT::internal::ItemType_ RVMT::internal::activeItemType = ItemType_None;
    const char* RVMT::internal::activeItemID = "none";

    bool RVMT::internal::startCalled = false;
    bool RVMT::internal::stopCalled = false;
    std::vector<bool> RVMT::renderRequests;

    std::vector<RVMT::internal::keyPress> RVMT::internal::KEYPRESSES;

    int RVMT::cursorX = 0;
    int RVMT::cursorY = 0;

    int BoxStyle_Current = BoxStyle_Round;


using namespace RVMT;
using namespace RVMT::internal;

void requestNewRender() {
    renderRequests.push_back(1);
};

void resetActiveItem() {
    activeItemType = ItemType_None;
    activeItemID = "none";
}

// !=== Widgets ===!
// === RVMT::Checkbox
// === RVMT::Button
// === RVMT::Slider
// === RVMT::Slider

bool RVMT::Checkbox(const char* trueText, const char* falseText, bool* val) {
    const int startX = cursorX;
    const int startY = cursorY;

    // Print text
    const char* ptr = *val ? &falseText[0] : &trueText[0];
    unsigned short textWidth = 0;
    for (unsigned short i = 0; i < 32767; i++) {
        if (ptr[i] == 0) {
            textWidth = i;
            break;
        }
        drawList.push_back({cursorX++, cursorY, ptr[i]});
    }

    // Handle cursor and SameLine.
    sameLineX = cursorX;
    sameLineY = cursorY;

    if (sameLineCalled) 
        cursorX = sameLineXRevert,
        cursorY = sameLineYRevert,
        sameLineCalled = false;
    
    else 
        cursorX = startX,
        cursorY++;
    

    // Handle return value
    if (!PREVINPUT_MOUSE1HELD && NEWINPUT_MOUSE1HELD &&
        NEWINPUT_MOUSECOL >= startX && NEWINPUT_MOUSECOL < startX + textWidth &&
        NEWINPUT_MOUSEROW == startY) {
        
        resetActiveItem();
        *val = !*val;
        activeItemType = ItemType_Checkbox;
        return true;
    }

    return false;
}

bool RVMT::Button(const char* str, ...) {

    const int startX = cursorX;
    const int startY = cursorY;

    char buffer[1024];
    va_list args;
    va_start(args, str);
    const auto textLength = vsnprintf(buffer, sizeof(buffer), str, args);
    va_end(args);

    DrawBox(cursorX, cursorY, textLength, 1);

    // Print text in the button's middle.
    cursorX++;
    cursorY++;

    Text(buffer);

    // Handle cursor and SameLine.
    sameLineY = cursorY - 2;
    sameLineX = textLength + 2;

    if (sameLineCalled) 
        cursorX = sameLineXRevert,
        cursorY = sameLineYRevert,
        sameLineCalled = false;
    else
        cursorX = startX,
        cursorY++;

    // Handle return value
    if (!PREVINPUT_MOUSE1HELD && NEWINPUT_MOUSE1HELD &&
        NEWINPUT_MOUSECOL >= startX && NEWINPUT_MOUSECOL <= startX + textLength + 1 &&
        NEWINPUT_MOUSEROW >= startY && NEWINPUT_MOUSEROW < startY + 3) {

        resetActiveItem();
        activeItemType = ItemType_Button;
        return true;
    }
    return false;
}

bool RVMT::Slider(const char* sliderID, int length, float minVal, float maxVal, float* var) {

    if (length < 1)
        length = 1;

    const int x = cursorX;
    const int y = cursorY;

    const int startXPX = (x + 1) * NEWINPUT_CELLWIDTH;
    const int endXPX = (x + length + 1) * NEWINPUT_CELLWIDTH;

    bool rvalue = false; // Idle.

    // Begin interaction
    if (activeItemType != ItemType_Slider &&
        NEWINPUT_MOUSE1HELD &&
        NEWINPUT_MOUSEWINX > startXPX && NEWINPUT_MOUSEWINX < endXPX &&
        NEWINPUT_MOUSEROW == y) {

        resetActiveItem();
        rvalue = true; // Clicked
        activeItemType = ItemType_Slider;
        activeItemID = sliderID;
    }

    // Continue interaction
    if (activeItemType == ItemType_Slider &&
        strEquals(activeItemID, sliderID)) {

        if (NEWINPUT_MOUSEWINX >= endXPX) // Clamp to maxval
            *var = maxVal;

        else if (NEWINPUT_MOUSEWINX <= startXPX) // Clamp to minval
            *var = minVal;

        else
            *var = minVal + (((maxVal - minVal) / (endXPX - startXPX)) * (NEWINPUT_MOUSEWINX - startXPX));
    }

    // Prepare slider output
    std::string sliderStr(length, '-');

    const float sliderTickVal = (maxVal - minVal) / length;
    for (int i = 0; sliderTickVal * (i + 1) <= *var - minVal; i++) 
        sliderStr[i] = '=';

    // Push slider content
    drawList.push_back({x, y, '['});

    for (int i = 1; i <= length; i++) 
        drawList.push_back({x+i, y, sliderStr[i-1]});

    drawList.push_back({x+length+1, y, ']'});

    // Handle SameLine and cursor.
    sameLineX = cursorX + length + 2;
    sameLineY = cursorY;

    if (sameLineCalled)
        cursorX = sameLineXRevert,
        cursorY = sameLineYRevert,
        sameLineCalled = false;
    else
        cursorY++;

    return rvalue;
}

bool RVMT::InputText(const char* fieldID, char* val, unsigned int maxStrSize, int width) {
    const int startX = cursorX;
    const int startY = cursorY;
    bool rvalue = false;

    if (!PREVINPUT_MOUSE1HELD && NEWINPUT_MOUSE1HELD &&
        NEWINPUT_MOUSECOL >= startX && NEWINPUT_MOUSECOL <= startX + width + 1 &&
        NEWINPUT_MOUSEROW >= startY && NEWINPUT_MOUSEROW < startY + 3) {

        resetActiveItem();
        rvalue = true; // clicked
        activeItemType = ItemType_InputText;
        activeItemID = fieldID;
    }

    const bool thisFieldIsActive = (
        activeItemType == ItemType_InputText &&
        strEquals(activeItemID, fieldID)
    );

    int inputLength = strLength(val);

    if (thisFieldIsActive) 
        for (auto& keypress : KEYPRESSES)
            if (strEquals(keypress.field, activeItemID)) {
                const char KEY = keypress.key;

                KEYPRESSES.erase(KEYPRESSES.begin());

                // Escape / BEL
                if (KEY == 27 || KEY == 7) {
                    activeItemID = "none";
                    activeItemType = ItemType_None;
                    break;
                }

                // Delete / Backspace
                if (KEY == 127 || KEY == 8) {
                    if (inputLength == 0) // Empty field
                        continue;

                    val[inputLength - 1] = 0;
                }

                else if (inputLength < maxStrSize) {
                    val[inputLength] = KEY;
                    val[++inputLength] = 0;
                }
            }
    
    DrawBox(cursorX, cursorY, width, 1);
    cursorX++;
    cursorY++;

    // Unfocused and no text written.
    if (!thisFieldIsActive &&
        inputLength == 0)
        Text("...");

    else if (inputLength > width)
        Text("%s", &val[inputLength - width]);

    else
        Text("%s", val);

    // Handle cursor and SameLine
    sameLineX = width + 2;
    sameLineY = cursorY - 2;

    if (sameLineCalled)
        cursorX = sameLineXRevert,
        cursorY = sameLineYRevert,
        sameLineCalled = false;
    else   
        cursorY++,
        cursorX--;

    return 0;
}

// !=== Drawing ===!
// === Text
// === DrawBox
// === SameLine

void RVMT::Text(const char* val, ...) {
    const int startX = cursorX;
    const int startY = cursorY;
    
    char buffer[1024];

    va_list args;
    va_start(args, val);
    vsnprintf(buffer, sizeof(buffer), val, args);
    va_end(args);

    unsigned int textLength = 0;

    for (int z = 0; z < sizeof(buffer); z++) {
        if (buffer[z] == 0) {
            textLength = z;
            break;
        }

        if (buffer[z] == 10) { // Newline
            cursorX = startX;
            cursorY++;
            continue;
        }

        drawList.push_back({cursorX++, cursorY, buffer[z]});
    }

    // Handle cursor and SameLine.
    sameLineX = textLength;
    sameLineY = cursorY;

    if (sameLineCalled) 
        cursorX = sameLineXRevert,
        cursorY = sameLineYRevert,
        sameLineCalled = false;
    else 
        cursorX = startX,
        cursorY++;
    
}

void RVMT::DrawBox(int x, int y, int width, int height) {
    // Note that this function does not change the cursor.
    // Rather doing this than doing "+1" every time these are called.
    height++;
    width++;

    // Set box style according to BoxStyle_Current
    short TLC, TRC, vBorders;
    short BLC, BRC;
    short hBorders;

    switch (BoxStyle_Current) {
        case BoxStyle_Simple:
            TLC = 9484, TRC = 9488, vBorders = 9474,
            BLC = 9492, BRC = 9496,
            hBorders = 9472;
            break;

        case BoxStyle_Bold:
            TLC = 9487, TRC = 9491, vBorders = 9475,
            BLC = 9495, BRC = 9499,
            hBorders = 9473;
            break;

        case BoxStyle_DoubleLine:
            TLC = 9556, TRC = 9559, vBorders = 9553,
            BLC = 9562, BRC = 9565,
            hBorders = 9552;
            break;

        case BoxStyle_Round:
            TLC = 9581, TRC = 9582, vBorders = 9474,
            BLC = 9584, BRC = 9583,
            hBorders = 9472;
            break;
    }

    // Push corners
    drawList.push_back({x, y, TLC});
    drawList.push_back({x + width, y, TRC});

    drawList.push_back({x, y + height, BLC});
    drawList.push_back({x + width, y + height, BRC});

    // Push borders
    for (unsigned short i = 1; i < width ; i++) // Top
        drawList.push_back({x + i, y, hBorders});

    for (unsigned short i = 1; i < width; i++) // Bottom
        drawList.push_back({x + i, y + height, hBorders});

    for (unsigned short i = 1; i < height; i++) // Left
        drawList.push_back({x, y + i, vBorders});

    for (unsigned short i = 1; i < height; i++) // Right
        drawList.push_back({x + width, y + i, vBorders});

}

void RVMT::SameLine() {
    sameLineCalled = true;
    sameLineXRevert = cursorX;
    sameLineYRevert = cursorY;

    cursorX = sameLineX;
    cursorY = sameLineY;
}

// !=== Internal ===!
// === Input threads
// === RVMT::Render
// === RVMT::Start
// === RVMT::Stop

struct termios _termios;
std::thread mouseInputThread;
std::thread kbInputsThread;

void mouseInputThreadFunc() {

    while (!stopCalled) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // In some cases, such as in i3-wm:
        // XGetInputFocus somehow does not return the same as XQueryPointer.
        // XGetInputFocus returns the "child"
        // XQueryPointer returns the "parent"

        Window currentWindow;
        XQueryPointer(rootDisplay, rootWindow, &_NULLX11WINDOW, &currentWindow,
            &_NULLINT, &_NULLINT, &_NULLINT, &_NULLINT, &_NULLUINT);

        if (currentWindow != termX11Win &&
            activeItemType != ItemType_Slider)
            continue;
            
        // === Get borders thickness.
        // There are two ways of getting it.
        // 1) Using the XGetGeometry function that literally retrieves the borders' size.
        unsigned int topBorder;
        unsigned int leftBorder;

        PREVINPUT_TERMWIDTH = NEWINPUT_TERMWIDTH;
        PREVINPUT_TERMHEIGHT = NEWINPUT_TERMHEIGHT;

        XGetGeometry(rootDisplay, termX11Win, &_NULLX11WINDOW, &NEWINPUT_TERMX, &NEWINPUT_TERMY, &NEWINPUT_TERMWIDTH, &NEWINPUT_TERMHEIGHT, &leftBorder, &topBorder);

        // 2) Getting the "inner window's" x and y. Used only if the first one doesn't work.
        if (topBorder == 0 && leftBorder == 0) {
            unsigned long innerWindow;
            XGetInputFocus(rootDisplay, &innerWindow, &_NULLINT);

            XWindowAttributes windowAttributes;
            XGetWindowAttributes(rootDisplay, innerWindow, &windowAttributes);

            if (windowAttributes.y >= 0)
                topBorder = windowAttributes.y;

            if (windowAttributes.x >= 0)
                leftBorder = windowAttributes.x;
        }

        NEWINPUT_TERMWIDTH -= (leftBorder*2);
        NEWINPUT_TERMHEIGHT -= topBorder;

        // === Get mouse values.
        unsigned int mouseMask;
        int mouseXPos;
        int mouseYPos;
        XQueryPointer(rootDisplay, rootWindow, &_NULLX11WINDOW, &_NULLX11WINDOW,
                    &_NULLINT, &_NULLINT, &mouseXPos, &mouseYPos, &mouseMask);

        NEWINPUT_MOUSEWINX = mouseXPos - NEWINPUT_TERMX - leftBorder;
        NEWINPUT_MOUSEWINY = mouseYPos - NEWINPUT_TERMY - topBorder;

        PREVINPUT_MOUSE1HELD = NEWINPUT_MOUSE1HELD;
        NEWINPUT_MOUSE1HELD = mouseMask & Button1Mask;

        // "> 0"'s here to prevent floating point exceptions.
        if (colCount > 0 && rowCount > 0) {
            NEWINPUT_CELLWIDTH = ((float)NEWINPUT_TERMWIDTH / (float)colCount);
        
            if (NEWINPUT_TERMWIDTH > 0)
                NEWINPUT_MOUSECOL = std::floor((float)NEWINPUT_MOUSEWINX / ((float)NEWINPUT_TERMWIDTH / (float)colCount));
            
            if (NEWINPUT_TERMHEIGHT > 0) 
                NEWINPUT_MOUSEROW = std::floor((float)NEWINPUT_MOUSEWINY / ((float)NEWINPUT_TERMHEIGHT / (float)rowCount));
        }

        if (NEWINPUT_MOUSE1HELD ||
            (PREVINPUT_MOUSE1HELD && !NEWINPUT_MOUSE1HELD) ||
            PREVINPUT_TERMWIDTH != NEWINPUT_TERMWIDTH ||
            PREVINPUT_TERMHEIGHT != NEWINPUT_TERMHEIGHT)
            renderRequests.push_back(1);
    }
}

void kbInputsThreadFunc() {
    while (!stopCalled) {
        const char LATEST_KEYPRESS = std::cin.get();

        if (activeItemType == ItemType_InputText) {
            KEYPRESSES.push_back({
                LATEST_KEYPRESS,
                activeItemID
            });
            renderRequests.push_back(1);
        }
    }
}

void RVMT::Render() {

    // Reset active item if idling.
    if (!NEWINPUT_MOUSE1HELD &&
        activeItemType != ItemType_None &&
        activeItemType != ItemType_InputText) {
        
        activeItemType = ItemType_None;
        activeItemID = "none";
        renderRequests.push_back(1);
    }

    struct winsize terminalSize;
    ioctl(1, TIOCGWINSZ, &terminalSize);

    rowCount = terminalSize.ws_row;
    colCount = terminalSize.ws_col;

    // Populate canvas.
    for (int y = 0; y < rowCount; y++)
        canvas.push_back(std::wstring(colCount, ' '));

    // Push drawlist's elements into the canvas.
    for (auto &elem : drawList) {
        if (elem.y >= canvas.size())
            continue;
        
        if (elem.x >= canvas[elem.y].length())
            continue;
        canvas[elem.y][elem.x] = elem.ch;
    }
    drawList.clear();

    // Store into prescreen to print everything at once.
    for (int i = 0; i < canvas.size(); i++)
        preScreen << canvas[i];
    canvas.clear();

    // Clear screen
    std::wcout << "\033[H\033[J";

    std::wcout << preScreen.str(); std::wcout.flush();
    preScreen.str(L"");

    cursorX = 0;
    cursorY = 0;
}

void RVMT::Start() {
    // Set locale to print unicodes correctly.
    std::locale::global(std::locale(""));

    // Clear non-initialized vars from any possible trash data
    drawList.clear();
    canvas.clear();
    preScreen.str(L"");

    rootDisplay = XOpenDisplay(NULL);
    rootWindow = DefaultRootWindow(rootDisplay);

    XQueryPointer(
        rootDisplay, rootWindow, &_NULLX11WINDOW, &termX11Win,
        &_NULLINT, &_NULLINT, &_NULLINT, &_NULLINT, &_NULLUINT);


    tcgetattr(0, &_termios);
    // Turn off canonical mode and input echoing for keyboard inputs.
    _termios.c_lflag = _termios.c_lflag ^ ICANON;
    _termios.c_lflag = _termios.c_lflag ^ ECHO;
    tcsetattr(0, 0, &_termios);

    // Start input threads.
    mouseInputThread = std::thread(&mouseInputThreadFunc);
    kbInputsThread = std::thread(&kbInputsThreadFunc);

    startCalled = true;
}

void RVMT::Stop() {
    stopCalled = true;

    // Wait for input threads to finish.
    std::wcout << "\nPress any key to exit...\n";
    mouseInputThread.join();
    kbInputsThread.join();

    // Restore terminal to the normal state.
    _termios.c_lflag = _termios.c_lflag ^ ICANON;
    _termios.c_lflag = _termios.c_lflag ^ ECHO;
    tcsetattr(0, 0, &_termios);
    
    XCloseDisplay(rootDisplay);
}