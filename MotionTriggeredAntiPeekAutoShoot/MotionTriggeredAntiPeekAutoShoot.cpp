// MotionTriggeredAntiPeekAutoShoot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "iostream"
#include "Windows.h"

using namespace std;

int width = 3;
int height = 3;

RGBQUAD* scan(POINT a, POINT b) {
	// copy screen to bitmap
	HDC     hScreen = GetDC(NULL);
	HDC     hDC = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, abs(b.x - a.x), abs(b.y - a.y));
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet = BitBlt(hDC, 0, 0, abs(b.x - a.x), abs(b.y - a.y), hScreen, a.x, a.y, SRCCOPY); // BitBlt does the copying

	/*
	// save bitmap to clipboard
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hBitmap);
	CloseClipboard();
	*/

	// Array conversion:
	RGBQUAD* pixels = new RGBQUAD[width * height];

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = width;
	bmi.biHeight = -height;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

	GetDIBits(hDC, hBitmap, 0, height, pixels, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	// clean up
	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);
	DeleteObject(hBitmap);
	return pixels;
}

bool compare(RGBQUAD* prev, RGBQUAD* curr) {
	bool result = false;
	int prevRed, prevGreen, prevBlue, currRed, currGreen, currBlue;
	// int tooBright = 230;
	for (int i = 0; i < width * height; i++) {
		prevRed = (int)prev[i].rgbRed;
		prevGreen = (int)prev[i].rgbGreen;
		prevBlue = (int)prev[i].rgbBlue;
		currRed = (int)curr[i].rgbRed;
		currGreen = (int)curr[i].rgbGreen;
		currBlue = (int)curr[i].rgbBlue;

		if (abs(currRed - prevRed) > 25 || abs(currGreen - prevGreen) > 25 || abs(currBlue - prevBlue) > 25) {
			result = true;
		}
		/*
		if (prevRed < tooBright && prevGreen < tooBright && prevBlue < tooBright &&
			currRed > tooBright && currGreen > tooBright && currBlue > tooBright) {
			result = false;
		}
		*/
	}
	return result;
}

void shoot() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); // start left click
	Sleep(100);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); // finish Left click 
}

void Aim() {
	POINT a, b;
	a.x = 1920 / 2 - width / 2;
	a.y = 1080 / 2 - height / 2;
	b.x = 1920 / 2 + width / 2;
	b.y = 1080 / 2 + height / 2;

	RGBQUAD* prev;
	RGBQUAD* curr;

	while (true) {
		if ((GetKeyState(VK_CONTROL) & 0x100) != 0) { // while ctrl pressed
			cout << "Engage motion trigger" << endl;
			prev = scan(a, b);
			curr = prev;
			while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
				curr = scan(a, b);
				if (compare(prev, curr)){
					delete[] prev;
					delete[] curr;
					shoot();
					break;
				}
				delete[] prev;
				prev = curr;
			}
			cout << "Release motion trigger" << endl;
		}
		Sleep(1);
	}
}

int main() {
	Aim();
	return 0;
}
