// MotionTriggeredAntiPeekAutoShoot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "iostream"
#include "Windows.h"
#include "vector" 

using namespace std;

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
int width = 200;
int height = 200;
bool enabled = true;
bool motionDetected = false;

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

POINT compareFrames(RGBQUAD* prev, RGBQUAD* curr) {
	POINT targetCoordinates{width/2, height/2};
	int prevRed, prevGreen, prevBlue, currRed, currGreen, currBlue, x, y, prevX, prevY, index, indexPrev;
	double radius = 1, angle = 2 * 3.141592654 * 3 / 4;;
	const int sampleCount = 32;

	for (int i = 0; i < (sampleCount * (width - 3)); i++) {
		x = (int)(radius * cos(angle) + width / 2);
		y = (int)(radius * sin(angle) + height / 2);
		if (!(prevX == x && prevY == y)) {
			if (i % sampleCount == 0) { // if ring is complete
				radius++; // increment radius
			}
			angle += 2 * 3.141592654 / sampleCount; // increment angle each iteration
			if (x < 3 || x > width - 4 || y < 3 || y > height - 4) { // boundary check
				break;
			}
			index = y * width + x; // get 1d array index
			currRed = (int)curr[index].rgbRed;
			currGreen = (int)curr[index].rgbGreen;
			currBlue = (int)curr[index].rgbBlue;
			for (int j = x - 2; j < x + 2; j++) {
				for (int k = y - 2; k < y + 2; k++) {
					indexPrev = k * width + j;
					prevRed = (int)prev[indexPrev].rgbRed;
					prevGreen = (int)prev[indexPrev].rgbGreen;
					prevBlue = (int)prev[indexPrev].rgbBlue;
					int absDifference = abs(currRed - prevRed) + abs(currGreen - prevGreen) + abs(currBlue - prevBlue);
					if (absDifference < 30) {
						break;
					} else if (j == x + 2 && k == y + 2) {
						motionDetected = true;
						targetCoordinates.x = index % width;
						targetCoordinates.y = index / width;
					}
				}
				if (motionDetected) {break;}
			}
		}
		prevX = x;
		prevY = y;
		if (motionDetected) { break; }
	}
	return targetCoordinates;
}

void shoot(POINT targetCoordinates) {
	double calibrationFactor = 1;
	mouse_event(MOUSEEVENTF_MOVE, (int) calibrationFactor * (targetCoordinates.x - (width / 2)), (int) calibrationFactor * (targetCoordinates.y - (height / 2)), 0, 0);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); // start left click
	Sleep(50);
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
	while(1) {
/*
		if (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && enabled) {
			Sleep(10);
			mouse_event(MOUSEEVENTF_MOVE, 0, 3, 0, 0);
		}

		// machine pistol
		if (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && enabled && primaryEnabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			Sleep(9);
			mouse_event(MOUSEEVENTF_MOVE, 0, 16, 0, 0);
			for (int i = 0; i < 9; i++) {
				Sleep(19);
				mouse_event(MOUSEEVENTF_MOVE, 0, 8, 0, 0);
			}
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
			Sleep(5);
		}
		if (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && enabled && !primaryEnabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			for (int i = 0; i < 10; i++) {
				Sleep(11);
				mouse_event(MOUSEEVENTF_MOVE, 0, 4, 0, 0);
			}
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
			Sleep(5);
		}
*/
		if ((GetKeyState(0x31) & 0x100) != 0) {
			primaryEnabled = true;
			Sleep(200);
		}
		if ((GetKeyState(0x32) & 0x100) != 0) {
			primaryEnabled = false;
			Sleep(200);
		}

		while (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && ((GetKeyState(VK_RBUTTON) & 0x100) != 0) && enabled && primaryEnabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			for (int i = 0; i < 15; i++) {
				Sleep(15);
				mouse_event(MOUSEEVENTF_MOVE, 0, 8, 0, 0);
			}
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
			Sleep(6);
		}
		while (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && ((GetKeyState(VK_RBUTTON) & 0x100) != 0) && enabled && !primaryEnabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			Sleep(6);
			mouse_event(MOUSEEVENTF_MOVE, 0, 15, 0, 0);
			Sleep(98);
			mouse_event(MOUSEEVENTF_MOVE, 0, 15, 0, 0);
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
			Sleep(6);
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

	while(1) {
		if (((GetKeyState(0x41) & 0x100) != 0) && ((GetKeyState(VK_RBUTTON) & 0x100) == 0) && enabled) { // A key cuases left lean
			SendInput(1, &_VK_NUMPAD1_keyDown, sizeof(INPUT));
			Sleep(20);
			SendInput(1, &_VK_NUMPAD1_keyUp, sizeof(INPUT));
			while (((GetKeyState(0x41) & 0x100) != 0) && ((GetKeyState(VK_RBUTTON) & 0x100) == 0) && enabled) {
				Sleep(20);
			}
		}
		if (((GetKeyState(0x44) & 0x100) != 0) && ((GetKeyState(VK_RBUTTON) & 0x100) == 0) && enabled) { // D key cuases left lean
			SendInput(1, &_VK_NUMPAD2_keyDown, sizeof(INPUT));
			Sleep(20);
			SendInput(1, &_VK_NUMPAD2_keyUp, sizeof(INPUT));
			while (((GetKeyState(0x44) & 0x100) != 0) && ((GetKeyState(VK_RBUTTON) & 0x100) == 0) && enabled) {
				Sleep(20);
			}
		}
		Sleep(1);
	}
}

void trackEnabled() {
	while (1) {
		while ((GetKeyState(VK_MENU) & 0x100) != 0) {
			enabled = !enabled;
			Sleep(200);
		}
		Sleep(2);
	}
}

int main() {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveRecoilCompensation, 0, 0, 0);
	//CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveLeaning, 0, 0, 0);
	//CreateThread(0, 0, (LPTHREAD_START_ROUTINE) trackEnabled, 0, 0, 0);

	POINT a, b;
	a.x = screenWidth / 2 - width / 2;
	a.y = screenHeight / 2 - height / 2;
	b.x = screenWidth / 2 + width / 2;
	b.y = screenHeight / 2 + height / 2;

	RGBQUAD* prev, curr;
	int preScanCount = 20;

	while (1) {
		if ((GetKeyState(VK_CONTROL) & 0x100) != 0) { // while ctrl pressed
			cout << "Activate motion trigger" << endl;
			prev = scan(a, b);
			while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
				curr = scan(a, b);
				POINT targetCoordiantes = compareFrames(ignore, curr);
				if (motionDetected) {
					motionDetected = false;
					shoot(targetCoordiantes);
					while ((GetKeyState(VK_CONTROL) & 0x100) != 0) {
						Sleep(20);
					}
					delete[] curr;
					break;
				}
				prev = curr;
				delete[] curr;
			}
			delete[] prev;
			cout << "Release motion trigger" << endl;
		}
		Sleep(1);
	}
	return 0;
}
