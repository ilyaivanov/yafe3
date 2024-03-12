#include <windows.h>

#include "deflib.c"
#include "types.h"
#include "win32.c"

#include "format.c"
#include "font.c"


// u32 bgColor

f32 appScale;
#define PX(val) ((val) *appScale)
u32 screenWidth;
u32 screenHeight;
MyBitmap canvas;
i32 isRunning = 1;
i32 isFullscreen = 0;
FontData font;
BITMAPINFO bitmapInfo;
u32 selectedChar;
u32 lastFrames[20];
u32 currentFrame;

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
                case VK_LEFT:
                    if(selectedChar >= 0)
                        selectedChar--;
                break;
                case VK_RIGHT:
                    selectedChar++;
                break;
            }
        break; 
    }
    return DefWindowProc(window, message, wParam, lParam);
}

void WinMainCRTStartup()
{
    PreventWindowsDPIScaling();

    HWND window = OpenWindow(OnEvent, 0x222222);
    HDC dc = GetDC(window);
    Arena arena = CreateArena(Megabytes(44));

    InitFontSystem();
    InitFont(&font, FontInfoClearType("Consolas", 14, 0xfff0f0f0, 0x00000000), &arena);

    //TODO: remove fucking windows new line symbols
    FileContent file = ReadMyFileImp("..\\progress.txt");

    MSG msg;
    LARGE_INTEGER frequency = {0};
    LARGE_INTEGER start = {0};
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    while(isRunning)
    {
        while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        memset(canvas.pixels, 0, canvas.bytesPerPixel * canvas.width * canvas.height);

        f32 leftOffset = PX(6);
        u32 x = leftOffset;
        u32 y = 0;

        char *ch = file.content;
        u32 i = 0;

        //TODO: handles only monospaced fonts
        u32 charWidth = font.textures[*ch].width;
        u32 charHeight = font.textures[*ch].height;
        while (*ch)
        {
            i32 isVisible = *ch >= ' ' || *ch == '\n';

            MyBitmap *texture = &font.textures[*ch];
            if(isVisible)
            {
                if(i == selectedChar)
                    PaintRect(&canvas, x, y, charWidth, charHeight, 0xff7B2CBF);

                i++;
            }

            if (*ch >= ' ')
                CopyBitmapRectTo(texture, &canvas, x, y);


            if(isVisible)
                x += charWidth;


            if (*ch == '\n')
            {
                x = leftOffset;
                y += font.textures[' '].height;
            }
            ch++;
        }




        LARGE_INTEGER end = {0};
        QueryPerformanceCounter(&end);

        u32 usPerFrame = (u32)((f32)((end.QuadPart - start.QuadPart) * 1000 * 1000) / (f32)frequency.QuadPart);
        lastFrames[currentFrame] = usPerFrame;
        currentFrame = (currentFrame + 1) % ArrayLength(lastFrames);
        i32 hasAnyZeroFrame = 0;
        {
            u32 averageFrame = 0;
            for(int i = 0; i < ArrayLength(lastFrames); i++)
            {
                if(!lastFrames[i])
                    hasAnyZeroFrame = 1;
                averageFrame += lastFrames[i];
            }

            if(!hasAnyZeroFrame)
            {
                averageFrame /= ArrayLength(lastFrames);

                char buff[30];
                i32 symbols = FormatNumber(averageFrame, buff);
                i32 x = screenWidth - charWidth * (symbols + 2);
                i32 y = screenHeight - charHeight;
                buff[symbols] = 'u';
                buff[symbols + 1] = 's';
                buff[symbols + 2] = '\0';

                char* ch = buff;
                while(*ch)
                {
                    MyBitmap *texture = &font.textures[*ch];
                    CopyBitmapRectTo(texture, &canvas, x, y);
                    ch++;
                    x += charWidth;
                }

                OutputDebugStringA(buff);
                OutputDebugStringA("\n");
            }
        }

        StretchDIBits(dc, 0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        start = end;


        //TODO: frame timing
        //TODO: proper timing, currently just burning the CPU, dont forget about timeBeginPeriod
        // Sleep(10);
    }

    ExitProcess(0);
}