// MotionTriggeredAntiPeekAutoShoot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "iostream"
#include "Windows.h"

using namespace std;

int width = 10;
int height = 10;

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
	int tolerance = 15;
	int x, y, index;
	// int tooBright = 230;

	for (int i = 0; i < 2 * width; i++) {
		if (i < width) {
			x = i;
			y = 0;
		}
		else if (i >= width) {
			x = i - 10;
			y = height - 1;
		}
		index = y * width + x; // get 1d array index

		prevRed = (int)prev[index].rgbRed;
		prevGreen = (int)prev[index].rgbGreen;
		prevBlue = (int)prev[index].rgbBlue;
		currRed = (int)curr[index].rgbRed;
		currGreen = (int)curr[index].rgbGreen;
		currBlue = (int)curr[index].rgbBlue;

		if (abs(currRed - prevRed) > tolerance || abs(currGreen - prevGreen) > tolerance || abs(currBlue - prevBlue) > tolerance) {
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
	/*
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); // start left click
	for (int i = 0; i < 10; i++) {
		Sleep(20);
		mouse_event(MOUSEEVENTF_MOVE, 0, 10, 0, 0);
	}
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); // finish Left click
	*/

	INPUT _0_keyDown;
	_0_keyDown.type = INPUT_KEYBOARD;
	_0_keyDown.ki.wScan = MapVirtualKey(VK_NUMPAD0, MAPVK_VK_TO_VSC); // hardware scan code
	_0_keyDown.ki.time = 0;
	_0_keyDown.ki.wVk = VK_NUMPAD0; // virtual-key code
	_0_keyDown.ki.dwExtraInfo = 0;
	_0_keyDown.ki.dwFlags = 0; // 0 for key down
	INPUT _0_keyUp = _0_keyDown;
	_0_keyUp.ki.dwFlags = KEYEVENTF_KEYUP;
	
	SendInput(1, &_0_keyDown, sizeof(INPUT)); // begin burst
	Sleep(10); // delay to prevent displacement of first shot
	mouse_event(MOUSEEVENTF_MOVE, 0, 15, 0, 0); // additional dampening for first shot recoil
	for (int i = 0; i < 9; i++) { // recoil compensation
		Sleep(20);
		mouse_event(MOUSEEVENTF_MOVE, 0, 10, 0, 0);
	}
	SendInput(1, &_0_keyUp, sizeof(INPUT)); // end burst
}

void main() {
	POINT a, b;
	a.x = 1920 / 2 - width / 2;
	a.y = 1080 / 2 - height / 2;
	b.x = 1920 / 2 + width / 2;
	b.y = 1080 / 2 + height / 2;

	RGBQUAD* prev;
	RGBQUAD* curr;

	while (true) {
		// if ((GetKeyState(VK_CONTROL) & 0x100) != 0) { // while ctrl pressed
		if ((GetKeyState(VK_LBUTTON) & 0x100) != 0) { // while lmb pressed
			cout << "Engage motion trigger" << endl;
			prev = scan(a, b);
			curr = prev;
			// while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
			while (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && ((GetKeyState(VK_CAPITAL) & 0x100) == 0)) {
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
