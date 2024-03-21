#include <windows.h>

#include "deflib.c"
#include "types.h"
#include "win32.c"

#include "string.c"
#include "format.c"
#include "font.c"
#include "cursor.c"

#include "performance.c"

typedef enum Mode
{
    ModeNormal,
    ModeInsert
} Mode;

f32 appScale;
#define PX(val) ((val) *appScale)

u32 screenWidth;
u32 screenHeight;
MyBitmap canvas;
bool isRunning = 1;
bool isFullscreen = 0;
bool isSpecialSymbolsShown = 0;
bool isJustSwitchedModeToInsert = 0;
Mode mode = ModeNormal;
FontData consolasFont14;
FontData segoeUiFont14;

BITMAPINFO bitmapInfo;
Cursor cursor;
StringBuffer buffer;

inline void CopyBitmapRectTo(MyBitmap *sourceT, MyBitmap *destination, u32 offsetX, u32 offsetY)
{
    u32 *row = (u32 *)destination->pixels + destination->width * (destination->height - 1) + offsetX - offsetY * destination->width;
    u32 *source = (u32 *)sourceT->pixels + sourceT->width * (sourceT->height - 1);
    for (u32 y = 0; y < sourceT->height; y += 1)
    {
        u32 *pixel = row;
        u32 *sourcePixel = source;
        for (u32 x = 0; x < sourceT->width; x += 1)
        {
            if(*sourcePixel != 0xff000000 && y + offsetY < destination->height && x + offsetX < destination->width)
                *pixel = *sourcePixel;
            sourcePixel += 1;
            pixel += 1;
            
        }
        source -= sourceT->width;
        row -= destination->width;
    }
}

inline void PaintRect(MyBitmap *destination, u32 offsetX, u32 offsetY, u32 width, u32 height, u32 color)
{
    u32 *row = (u32 *)destination->pixels + destination->width * (destination->height - 1) + offsetX - offsetY * destination->width;
    for (u32 y = 0; y < height; y += 1)
    {
        u32 *pixel = row;
        for (u32 x = 0; x < width; x += 1)
        {
            if(y + offsetY < destination->height && x + offsetX < destination->width)
                *pixel = color;
            pixel += 1;
            
        }
        row -= destination->width;
    }
}



void InsertChartUnderCursor(StringBuffer* buffer, WPARAM ch)
{
    WPARAM code = ch == '\r' ? '\n' : ch;
    InsertCharAt(buffer, cursor.pos, code);
    cursor.pos++;
    // UpdateCursorPosition(buffer, cursor.cursorIndex + 1);

    // cursor.selectionStart = SELECTION_NONE;
}



LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message){
        case WM_QUIT:
        case WM_DESTROY:
            PostQuitMessage(0);
            isRunning = 0;
        break;

        case WM_PAINT:
            PAINTSTRUCT paint;
            BeginPaint(window, &paint);
            EndPaint(window, &paint);
        break;

        case WM_SIZE:
            HDC dc = GetDC(window);
            appScale = (float)GetDeviceCaps(dc, LOGPIXELSY) / (float)USER_DEFAULT_SCREEN_DPI;
            screenWidth = LOWORD(lParam);
            screenHeight = HIWORD(lParam);
            InitBitmapInfo(&bitmapInfo, screenWidth, screenHeight);
            canvas.width =  screenWidth;
            canvas.height = screenHeight;
            canvas.bytesPerPixel = 4;
            //TODO: Initialize Arena of screen size and assign proper width and height on resize
            if(canvas.pixels)
                VirtualFreeMemory(canvas.pixels);
            canvas.pixels = VirtualAllocateMemory(sizeof(u32) * screenWidth * screenHeight);
        break;

        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_F11:
                    isFullscreen = !isFullscreen;
                    SetFullscreen(window, isFullscreen);
                break;
                case VK_F2:
                    isSpecialSymbolsShown = !isSpecialSymbolsShown;
                break;
                case 'H':
                    if(mode == ModeNormal)
                        CursorMoveLeft(&cursor, &buffer);
                break;
                case 'L':
                    if(mode == ModeNormal)
                        CursorMoveRight(&cursor, &buffer);
                break;
                case 'J':
                    if(mode == ModeNormal)
                        CursorMoveDown(&cursor, &buffer);
                break;
                case 'K':
                    if(mode == ModeNormal)
                        CursorMoveUp(&cursor, &buffer);
                break;
                case VK_DELETE:
                    if(cursor.pos < buffer.size)
                        RemoveCharAt(&buffer, cursor.pos);
                case VK_BACK:
                    if(cursor.pos > 0)
                    {
                        RemoveCharAt(&buffer, cursor.pos - 1);
                        cursor.pos--;
                    }
                break;
                case 'I':
                    if(mode == ModeNormal)
                    {
                        mode = ModeInsert;
                        isJustSwitchedModeToInsert = 1;
                    }
                break;
                case VK_ESCAPE:
                    if(mode == ModeInsert)
                    {
                        mode = ModeNormal;
                    }
                break;
            }
        break; 
        case WM_CHAR:
            if(isJustSwitchedModeToInsert)
                isJustSwitchedModeToInsert = 0;
            else if(mode == ModeInsert)
            {
                u8 DEL_CODE = 127;
                if(wParam != DEL_CODE && (wParam >= ' ' || wParam == '\r'))
                    InsertChartUnderCursor(&buffer, wParam);
            }
        break;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

void ReportAt(i32 rowIndex, char* label, u32 val)
{
    char buff[40];
    u32 buffIndex = 0;
    while (*label)
    {
        buff[buffIndex++] = *label;
        label++;
    }
    buff[buffIndex++] = ':';
    buff[buffIndex++] = ' ';

    i32 symbols = FormatNumber(val, buff + buffIndex);
    i32 x = screenWidth - currentFont->charWidth * (symbols + buffIndex + 2);
    i32 y = screenHeight - currentFont->charHeight * (rowIndex + 1);
    buff[symbols + buffIndex] = 'u';
    buff[symbols + buffIndex + 1] = 's';
    buff[symbols + buffIndex + 2] = '\0';

    char *ch = buff;
    while (*ch)
    {
        MyBitmap *texture = &currentFont->textures[*ch];
        CopyBitmapRectTo(texture, &canvas, x, y);
        ch++;
        x += currentFont->charWidth;
    }
}

void WinMainCRTStartup()
{
    PreventWindowsDPIScaling();

    HWND window = OpenWindow(OnEvent, 0x222222);
    HDC dc = GetDC(window);
    Arena arena = CreateArena(Megabytes(44));

    InitFontSystem();
    InitFont(&consolasFont14, FontInfoClearType("Consolas", 14, 0xfff0f0f0, 0x00000000), &arena);
    InitFont(&segoeUiFont14, FontInfoClearType("Segoe UI", 13, 0xfff0f0f0, 0x00000000), &arena);

    // currentFont = &consolasFont14;
    currentFont = &consolasFont14;
    
    buffer = ReadFileIntoDoubledSizedBuffer("..\\progress.txt");
    MSG msg;
    InitPerf();

    while(isRunning)
    {
        while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        StartMetric(Memory);
        memset(canvas.pixels, 0, canvas.bytesPerPixel * canvas.width * canvas.height);
        u32 usMemory = EndMetric(Memory);

        StartMetric(Draw);
        f32 leftOffset = PX(6);
        u32 x = leftOffset;
        u32 y = 0;

        char *ch = buffer.content;
        u32 i = 0;

        //TODO: handles only monospaced fonts
        u32 charWidth = currentFont->charWidth;
        while (*ch)
        {
            i32 isVisible = *ch >= ' ' || *ch == '\n';

            char textureIndex = 0;
            
            if(isSpecialSymbolsShown)
                textureIndex = *ch == ' ' ? 1 : (*ch == '\n' ? 2 : *ch);
            else 
                textureIndex = *ch;


            MyBitmap *texture = &currentFont->textures[textureIndex];
            u32 charWidth = currentFont->isMonospaced ? currentFont->charWidth : texture->width;
            if(isVisible)
            {
                if(i == cursor.pos)
                {
                    u32 color = mode == ModeNormal ? 0xff7B2CBF : 0xffBF2C7B;
                    PaintRect(&canvas, x, y, charWidth, currentFont->charHeight, color);
                }

                i++;
            }

            if (isVisible)
            {
                CopyBitmapRectTo(texture, &canvas, x, y);
                x += charWidth + GetKerningValue(*ch, *(ch + 1)); 

            }

            if (*ch == '\n')
            {
                x = leftOffset;
                y += currentFont->charHeight;
            }
            ch++;
        }


        u32 usDraw = EndMetric(Draw);

        u32 usPerFrame = EndMetric(Overall);

        ReportAt(3, "Memory", usMemory);
        ReportAt(2, "Drawing", usDraw);
        ReportAt(1, "DiBits", GetMicrosecondsFor(DiBits));
        ReportAt(0, "Overall", usPerFrame);

        StartMetric(DiBits);
        StretchDIBits(dc, 0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        EndMetric(DiBits);

        
        
        EndFrame();

        //TODO: proper sleep timing, currently just burning the CPU, dont forget about timeBeginPeriod
        // Sleep(10);
    }

    ExitProcess(0);
}