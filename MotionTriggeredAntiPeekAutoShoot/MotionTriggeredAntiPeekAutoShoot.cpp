// MotionTriggeredAntiPeekAutoShoot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "iostream"
#include "Windows.h"
#include "vector" 

using namespace std;

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
int width = 10;
int height = 10;
bool enabled = false;
int tolerance = 30;

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

void updateIgnore(vector<RGBQUAD>& ignore, RGBQUAD* curr) {
	//int ignoreSizeInitial = ignoreSize;
	int ignoreRed, ignoreGreen, ignoreBlue, currRed, currGreen, currBlue;
	if (ignore.size() == 0) {
		ignore.push_back(curr[0]);
		//for (int i = 0; i < width * height; i++) {
		//	ignore.push_back(curr[i]);
		//}
	}
	for (int i = 0; i < (width * height); i++) {
		currRed = (int)curr[i].rgbRed;
		currGreen = (int)curr[i].rgbGreen;
		currBlue = (int)curr[i].rgbBlue;
		//cout << currRed << " " << currGreen << " " << currBlue << endl;
		for (unsigned int j = 0; j < ignore.size(); j++) {
			ignoreRed = (int)ignore[j].rgbRed;
			ignoreGreen = (int)ignore[j].rgbGreen;
			ignoreBlue = (int)ignore[j].rgbBlue;
			if ((abs(currRed - ignoreRed) + abs(currGreen - ignoreGreen) + abs(currBlue - ignoreBlue) < 5) ) {
				break;
			} else if (j == (ignore.size() - 1)) {
				ignore.push_back(curr[i]);
				// cout << ignore.size() << endl;
			}
		}
	}
}

bool findDifference(vector<RGBQUAD>& prev, RGBQUAD* curr) {
	bool result = false;
	int prevRed, prevGreen, prevBlue, currRed, currGreen, currBlue;
	for (int i = 0; i < (width * height); i++) {
		currRed = (int)curr[i].rgbRed;
		currGreen = (int)curr[i].rgbGreen;
		currBlue = (int)curr[i].rgbBlue;
		for (unsigned int j = 0; j < prev.size(); j++) {
			prevRed = (int)prev[j].rgbRed;
			prevGreen = (int)prev[j].rgbGreen;
			prevBlue = (int)prev[j].rgbBlue;
			if ((abs(currRed - prevRed) + abs(currGreen - prevGreen) + abs(currBlue - prevBlue) < 30) ) {
				// && (abs(currRed - prevRed) < tolerance/3 && abs(currGreen - prevGreen) < tolerance/3 && abs(currBlue - prevBlue) < tolerance/3)
				break;
			} else if (j == (prev.size() - 1) && (abs(currRed - prevRed) + abs(currGreen - prevGreen) + abs(currBlue - prevBlue) > tolerance)) {
				result = true;
				cout << currRed << " " << currGreen << " " << currBlue << "    " << j ;
			}
		}
		if (result) { break; }
	}
	return result;
}

void shoot() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); // start left click
	Sleep(10);
	mouse_event(MOUSEEVENTF_MOVE, 0, 10, 0, 0); // First shot recoil additional dampening
	Sleep(90);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); // finish Left click
	Sleep(10);
}

void passiveRecoilCompensation() {
	while(1) {
		while (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && enabled) {
			Sleep(20);
			mouse_event(MOUSEEVENTF_MOVE, 0, 10, 0, 0);
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
	while(1) {
		if (((GetKeyState(0x41) & 0x100) != 0) && enabled) { // A key cuases left lean
			SendInput(1, &_VK_NUMPAD1_keyDown, sizeof(INPUT));
			while (((GetKeyState(0x41) & 0x100) != 0) && enabled) {
				Sleep(5);
			}
			SendInput(1, &_VK_NUMPAD1_keyUp, sizeof(INPUT));
		}
		if (((GetKeyState(0x44) & 0x100) != 0) && enabled) { // D key cuases left lean
			SendInput(1, &_VK_NUMPAD2_keyDown, sizeof(INPUT));
			while (((GetKeyState(0x44) & 0x100) != 0) && enabled) {
				Sleep(5);
			}
			SendInput(1, &_VK_NUMPAD2_keyUp, sizeof(INPUT));
		}
		Sleep(1);
	}
}
void trackEnabled() {
	while (1) {
		if ((GetKeyState(VK_CAPITAL) & 0x100) != 0) {
			enabled = !enabled;
			while ((GetKeyState(VK_CAPITAL) & 0x100) != 0) {
				Sleep(200);
			}
		}
	}
}

int main() {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveRecoilCompensation, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveLeaning, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) trackEnabled, 0, 0, 0);

	POINT preScanTopLeft, preScanBottomRight;
	preScanTopLeft.x = screenWidth / 2 - (width*10) / 2;
	preScanTopLeft.y = screenHeight / 2 - (height*10) / 2;
	preScanBottomRight.x = screenWidth / 2 + (width * 10) / 2;
	preScanBottomRight.y = screenHeight / 2 + (height * 10) / 2;
	POINT a, b;
	a.x = screenWidth / 2 - width / 2;
	a.y = screenHeight / 2 - height / 2;
	b.x = screenWidth / 2 + width / 2;
	b.y = screenHeight / 2 + height / 2;

	// RGBQUAD* prev;
	RGBQUAD* curr;
	int preScanCount = 20;

	while (1) {
		if ((GetKeyState(VK_CONTROL) & 0x100) != 0) { // while ctrl pressed
		// if ((GetKeyState(VK_LBUTTON) & 0x100) != 0) { // while lmb pressed
			cout << "Activate motion trigger" << endl;

			vector<RGBQUAD> ignore;
			/*
						RGBQUAD black;
			black.rgbRed = 0;
			black.rgbGreen = 0;
			black.rgbBlue = 0;
			RGBQUAD white;
			white.rgbRed = 255;
			white.rgbGreen = 255;
			white.rgbBlue = 255;
			ignore.push_back(black);
			ignore.push_back(white);
			*/

			for (int i = 0; i < 200; i++) {
				curr = scan(a, b);
				updateIgnore(ignore, curr);
				delete[] curr;
				//Sleep(2);
			}

			int ignoreRed, ignoreGreen, ignoreBlue;
			for (int k = 0; k < ignore.size(); k++) {
				ignoreRed = (int)ignore[k].rgbRed;
				ignoreGreen = (int)ignore[k].rgbGreen;
				ignoreBlue = (int)ignore[k].rgbBlue;
			}
			//cout << ignoreRed << " " << ignoreGreen << " " << ignoreBlue << endl;


			while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
			// while (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && ((GetKeyState(VK_CAPITAL) & 0x100) == 0)) {
				curr = scan(a, b);
				if (findDifference(ignore, curr)) {
					while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
						shoot();
					}
					delete[] curr;
					break;
				}
				delete[] curr;
			}
			ignore.clear();
			cout << "Release motion trigger" << endl;
		}
		Sleep(1);
	}
	return 0;
}
