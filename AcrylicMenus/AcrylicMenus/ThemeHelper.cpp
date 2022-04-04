#include "ThemeHelper.h"
#include <iostream>

int RecolorizeBitmap(HBITMAP hbm)
{
    BITMAP bm;
    GetObject(hbm, sizeof(bm), &bm);

    if (!hbm || bm.bmBitsPixel != 32)
    {
        return FALSE;
    }

    BYTE* pBits = new BYTE[bm.bmWidth * bm.bmHeight * 4];
    GetBitmapBits(hbm, bm.bmWidth * bm.bmHeight * 4, pBits);

    for (int y = 0; y < bm.bmHeight; y++) {
        BYTE* pPixel = (BYTE*)pBits + bm.bmWidth * 4 * y;

        for (int x = 0; x < bm.bmWidth; x++) {

            int r = pPixel[2] & 0xFFFFFF;
            //int g = pPixel[1] & 0xFFFFFF;
            //int b = pPixel[0] & 0xFFFFFF;
            //int a = pPixel[3] & 0xFFFFFF;

            //std::cout << r << " " << g << " " << b << " / " << a << std::endl;

            pPixel[0] = 0;
            pPixel[1] = 0;
            pPixel[2] = 0;
            
            if (r == 238 || r == 43) {
                pPixel[3] = 0;
            }
            else if (r == 255 || r == 65) {
                pPixel[3] = 48;
            }

            pPixel += 4;
        }
    }


    SetBitmapBits(hbm, bm.bmWidth * bm.bmHeight * 4, pBits);

    delete[] pBits;
    return TRUE;
}

int ModifyThemeData(LPCWSTR pszClassList) {
    HBITMAP hBitmap;

    HTHEME hTheme = OpenThemeData(GetForegroundWindow(), pszClassList);
    GetThemeBitmap(hTheme, 9, 0, TMT_DIBDATA, GBF_DIRECT, &hBitmap);
    GetThemeBitmap(hTheme, 14, 0, TMT_DIBDATA, GBF_DIRECT, &hBitmap);
    CloseThemeData(hTheme);

    return RecolorizeBitmap(hBitmap);
}

int UpdateThemeData() {
    ModifyThemeData(L"ImmersiveStart::Menu");
    ModifyThemeData(L"ImmersiveStartDark::Menu");

    return TRUE;
}