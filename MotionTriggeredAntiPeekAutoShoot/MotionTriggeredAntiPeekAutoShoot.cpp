// MotionTriggeredAntiPeekAutoShoot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "iostream"
#include "Windows.h"
#include "vector" 

using namespace std;

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
int width = 6;
int height = 6;
bool enabled = true;

RGBQUAD* scan(POINT a, POINT b) {
	// copy screen to bitmap
	HDC     hScreen = GetDC(NULL);
	HDC     hDC = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, abs(b.x - a.x), abs(b.y - a.y));
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet = BitBlt(hDC, 0, 0, abs(b.x - a.x), abs(b.y - a.y), hScreen, a.x, a.y, SRCCOPY); // BitBlt performs copying

	// Array conversion:
	RGBQUAD* pixels = new RGBQUAD[width * height];

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = width;
	bmi.biHeight = -height;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0; // 3 * ScreenX * ScreenY;

	GetDIBits(hDC, hBitmap, 0, height, pixels, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	// clean up
	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);
	DeleteObject(hBitmap);
	return pixels;
}

bool compareFrames(RGBQUAD* prev, RGBQUAD* curr) {
	int prevRed, prevGreen, prevBlue, currRed, currGreen, currBlue, absDifference;
	for (int i = 0; i < width; i++) {
		currRed = (int)curr[i].rgbRed;
		currGreen = (int)curr[i].rgbGreen;
		currBlue = (int)curr[i].rgbBlue;
		for (int j = 0; j < width * height; j++) {
			prevRed = (int)prev[j].rgbRed;
			prevGreen = (int)prev[j].rgbGreen;
			prevBlue = (int)prev[j].rgbBlue;
			absDifference = abs(currRed - prevRed) + abs(currGreen - prevGreen) + abs(currBlue - prevBlue);
			if (absDifference < 30) {
				break;
			} else if (j == width * height - 1) {
				return true;
			}
		}
	}
	return false;
}

void shoot() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); // start left click
	Sleep(200);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); // finish Left click
}

void passiveRecoilCompensation() { //
	bool primaryEnabled = true;
	INPUT _VK_NUMPAD0_keyDown;
	_VK_NUMPAD0_keyDown.type = INPUT_KEYBOARD;
	_VK_NUMPAD0_keyDown.ki.wScan = MapVirtualKey(VK_NUMPAD0, MAPVK_VK_TO_VSC); // hardware scan code
	_VK_NUMPAD0_keyDown.ki.time = 0;
	_VK_NUMPAD0_keyDown.ki.wVk = VK_NUMPAD0; // virtual-key code
	_VK_NUMPAD0_keyDown.ki.dwExtraInfo = 0;
	_VK_NUMPAD0_keyDown.ki.dwFlags = 0; // 0 for key down
	INPUT _VK_NUMPAD0_keyUp = _VK_NUMPAD0_keyDown;
	_VK_NUMPAD0_keyUp.ki.dwFlags = KEYEVENTF_KEYUP;
	while (1) {
		while (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && ((GetKeyState(VK_RBUTTON) & 0x100) != 0) && enabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			for (int i = 0; i < 4; i++) {
				Sleep(10);
				mouse_event(MOUSEEVENTF_MOVE, 0, 25, 0, 0);
			}
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
			Sleep(40);
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			while ((GetKeyState(VK_LBUTTON) & 0x100) != 0) {
				Sleep(20);
				mouse_event(MOUSEEVENTF_MOVE, 0, 45, 0, 0);
			}
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
		}
		if ((GetKeyState(VK_LBUTTON) & 0x100) != 0 && enabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			while ((GetKeyState(VK_LBUTTON) & 0x100) != 0) {
				Sleep(20);
			}
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
		}
		Sleep(1);
	}
}

void passiveLeaning() {
	INPUT _VK_NUMPAD1_keyDown;
	_VK_NUMPAD1_keyDown.type = INPUT_KEYBOARD;
	_VK_NUMPAD1_keyDown.ki.wScan = MapVirtualKey(VK_NUMPAD1, MAPVK_VK_TO_VSC); // hardware scan code
	_VK_NUMPAD1_keyDown.ki.time = 0;
	_VK_NUMPAD1_keyDown.ki.wVk = VK_NUMPAD1; // virtual-key code
	_VK_NUMPAD1_keyDown.ki.dwExtraInfo = 0;
	_VK_NUMPAD1_keyDown.ki.dwFlags = 0; // 0 for key down
	INPUT _VK_NUMPAD1_keyUp = _VK_NUMPAD1_keyDown;
	_VK_NUMPAD1_keyUp.ki.dwFlags = KEYEVENTF_KEYUP;

	INPUT _VK_NUMPAD2_keyDown = _VK_NUMPAD1_keyDown;
	_VK_NUMPAD2_keyDown.ki.wScan = MapVirtualKey(VK_NUMPAD2, MAPVK_VK_TO_VSC); // hardware scan code
	_VK_NUMPAD2_keyDown.ki.wVk = VK_NUMPAD2; // virtual-key code
	INPUT _VK_NUMPAD2_keyUp = _VK_NUMPAD2_keyDown;
	_VK_NUMPAD2_keyUp.ki.dwFlags = KEYEVENTF_KEYUP;

	INPUT _VK_NUMPAD4_keyDown = _VK_NUMPAD1_keyDown;
	_VK_NUMPAD4_keyDown.ki.wScan = MapVirtualKey(VK_NUMPAD4, MAPVK_VK_TO_VSC); // hardware scan code
	_VK_NUMPAD4_keyDown.ki.wVk = VK_NUMPAD4; // virtual-key code
	INPUT _VK_NUMPAD4_keyUp = _VK_NUMPAD4_keyDown;
	_VK_NUMPAD4_keyUp.ki.dwFlags = KEYEVENTF_KEYUP;

	INPUT _VK_NUMPAD5_keyDown = _VK_NUMPAD1_keyDown;
	_VK_NUMPAD5_keyDown.ki.wScan = MapVirtualKey(VK_NUMPAD5, MAPVK_VK_TO_VSC); // hardware scan code
	_VK_NUMPAD5_keyDown.ki.wVk = VK_NUMPAD5; // virtual-key code
	INPUT _VK_NUMPAD5_keyUp = _VK_NUMPAD5_keyDown;
	_VK_NUMPAD5_keyUp.ki.dwFlags = KEYEVENTF_KEYUP;

	INPUT _C_keyDown = _VK_NUMPAD1_keyDown;
	_C_keyDown.ki.wScan = MapVirtualKey(0x43, MAPVK_VK_TO_VSC); // hardware scan code
	_C_keyDown.ki.wVk = 0x43; // virtual-key code
	INPUT _C_keyUp = _C_keyDown;
	_C_keyUp.ki.dwFlags = KEYEVENTF_KEYUP;

	while (1) {
		if (((GetKeyState(VK_SHIFT) & 0x100) != 0) && ((GetKeyState(0x41) & 0x100) != 0) && enabled) { // A key cuases left lean
			SendInput(1, &_VK_NUMPAD1_keyDown, sizeof(INPUT));
			//SendInput(1, &_VK_NUMPAD4_keyDown, sizeof(INPUT));
			Sleep(20);
			SendInput(1, &_VK_NUMPAD1_keyUp, sizeof(INPUT));
			while (((GetKeyState(0x41) & 0x100) != 0)) {
				Sleep(20);
			}
			//SendInput(1, &_VK_NUMPAD4_keyUp, sizeof(INPUT));
//
			//SendInput(1, &_VK_NUMPAD1_keyDown, sizeof(INPUT));
			//SendInput(1, &_VK_NUMPAD5_keyDown, sizeof(INPUT));
			//Sleep(250);
			//SendInput(1, &_VK_NUMPAD1_keyUp, sizeof(INPUT));
			//SendInput(1, &_VK_NUMPAD5_keyUp, sizeof(INPUT));
		}
		if (((GetKeyState(VK_SHIFT) & 0x100) != 0) && ((GetKeyState(0x44) & 0x100) != 0) && enabled) { // D key cuases left lean
			SendInput(1, &_VK_NUMPAD2_keyDown, sizeof(INPUT));
			//SendInput(1, &_VK_NUMPAD5_keyDown, sizeof(INPUT));
			Sleep(20);
			SendInput(1, &_VK_NUMPAD2_keyUp, sizeof(INPUT));
			while (((GetKeyState(0x44) & 0x100) != 0)) {
				Sleep(20);
			}
			//SendInput(1, &_VK_NUMPAD5_keyUp, sizeof(INPUT));
//
			//SendInput(1, &_VK_NUMPAD2_keyDown, sizeof(INPUT));
			//SendInput(1, &_VK_NUMPAD4_keyDown, sizeof(INPUT));
			//Sleep(250);
			//SendInput(1, &_VK_NUMPAD2_keyUp, sizeof(INPUT));
			//SendInput(1, &_VK_NUMPAD4_keyUp, sizeof(INPUT));
		}
		Sleep(1);
	}
}

void trackEnabled() {
	while (1) {
		while ((GetKeyState(VK_F1) & 0x100) != 0) {
			enabled = !enabled;
			Sleep(200);
		}
		Sleep(2);
	}
}

int main() {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveRecoilCompensation, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveLeaning, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) trackEnabled, 0, 0, 0);

	POINT a, b;
	a.x = screenWidth / 2 - width / 2;
	a.y = screenHeight / 2 - height / 2;
	b.x = screenWidth / 2 + width / 2;
	b.y = screenHeight / 2 + height / 2;

	RGBQUAD* prev;
	RGBQUAD* curr;
	int preScanCount = 20;

	while (1) {
		if ((GetKeyState(VK_CONTROL) & 0x100) != 0) { // while ctrl pressed
			cout << "Activate motion trigger" << endl;
			prev = scan(a, b);
			while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
				curr = scan(a, b);
				if (compareFrames(prev, curr)) {
					shoot();
					while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
						Sleep(20);
					}
					delete[] curr;
					break;
				}
				delete[] prev;
				prev = curr;
			}
			cout << "Release motion trigger" << endl << endl;
		}
		Sleep(2);
	}
	return 0;
}
