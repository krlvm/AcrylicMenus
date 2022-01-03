#pragma once

#include <Windows.h>
#include <Uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "uxtheme.lib")

#pragma region Pixel Color

inline int PixClr(int val)
{
    return val & 0xFFFFFF;
}

inline int PixR(BYTE* pPixel)
{
    return PixClr(pPixel[2]);
}
inline int PixG(BYTE* pPixel)
{
    return PixClr(pPixel[1]);
}
inline int PixB(BYTE* pPixel)
{
    return PixClr(pPixel[0]);
}
inline int PixA(BYTE* pPixel)
{
    return PixClr(pPixel[3]);
}

#pragma endregion

int RecolorizeBitmap(HBITMAP hbm)
{
    BITMAP bm;
    GetObject(hbm, sizeof(bm), &bm);

    if (!hbm || bm.bmBitsPixel != 32) {
        return FALSE;
    }

    BYTE* pBits = new BYTE[bm.bmWidth * bm.bmHeight * 4];
    GetBitmapBits(hbm, bm.bmWidth * bm.bmHeight * 4, pBits);

    for (int y = 0; y < bm.bmHeight; y++) {
        BYTE* pPixel = (BYTE*)pBits + bm.bmWidth * 4 * y;

        for (int x = 0; x < bm.bmWidth; x++) {

            int r = PixR(pPixel); // [2]
            int g = PixG(pPixel); // [1]
            int b = PixB(pPixel); // [0]
            int a = PixA(pPixel); // [3]

            if (r == 238 && g == 238 && b == 238)
            {
                pPixel[2] = 242;
                pPixel[1] = 242;
                pPixel[0] = 242;
            }
            else if (r == 255 && g == 255 && b == 255)
            {
                pPixel[2] = 217;
                pPixel[1] = 217;
                pPixel[0] = 217;
            }

            //pPixel[3] = 1;

            pPixel += 4;
        }
    }


    SetBitmapBits(hbm, bm.bmWidth * bm.bmHeight * 4, pBits);

    delete[] pBits;
    return TRUE;
}

int HelpTheme() {
    HBITMAP hBitmap;

    HTHEME hTheme = OpenThemeData(GetForegroundWindow(), L"ImmersiveStart::Menu");
    GetThemeBitmap(hTheme, 14, 0, TMT_DIBDATA, GBF_DIRECT, &hBitmap);
    CloseThemeData(hTheme);

    RecolorizeBitmap(hBitmap);

    return 0;
}