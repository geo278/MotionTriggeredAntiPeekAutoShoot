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
bool enabled = true;
bool preScanComplete = false;

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
	int ignoreRed, ignoreGreen, ignoreBlue, currRed, currGreen, currBlue;
	if (ignore.size() == 0) {
		ignore.push_back(curr[0]);
	}
	for (int i = 0; i < (width * height); i++) {
		currRed = (int)curr[i].rgbRed;
		currGreen = (int)curr[i].rgbGreen;
		currBlue = (int)curr[i].rgbBlue;
		for (unsigned int j = 0; j < ignore.size(); j++) {
			ignoreRed = (int)ignore[j].rgbRed;
			ignoreGreen = (int)ignore[j].rgbGreen;
			ignoreBlue = (int)ignore[j].rgbBlue;
			if ((abs(currRed - ignoreRed) + abs(currGreen - ignoreGreen) + abs(currBlue - ignoreBlue) < 3) ) {
				break;
			} else if (j == (ignore.size() - 1)) {
				ignore.push_back(curr[i]);
			}
		}
	}
}
void scanCoverageRoutine() {
	int i = 0;
	int k = 1;
	while (!preScanComplete) {
		if (i%2 == 0) {
			k = 1;
		} else {
			k = -1;
		}
		mouse_event(MOUSEEVENTF_MOVE, k * i, 0, 0, 0);
		Sleep(4);
		mouse_event(MOUSEEVENTF_MOVE, 0, k * i, 0, 0);
		Sleep(4);
		i++;
	}
	if (k < 0) { // reset position
		mouse_event(MOUSEEVENTF_MOVE, -k * (i / 2 + 1), -k * (i / 2 + 1), 0, 0);
	} else {
		mouse_event(MOUSEEVENTF_MOVE, -k * (i / 2 ), -k * (i / 2), 0, 0);
	}
}

bool findDifference(vector<RGBQUAD>& prev, RGBQUAD* curr) {
	// int diffCount = 0;
	bool result = false;
	int prevRed, prevGreen, prevBlue, currRed, currGreen, currBlue;
	for (int i = 0; i < (width * height); i++) {
		currRed = (int)curr[i].rgbRed;
		currGreen = (int)curr[i].rgbGreen;
		currBlue = (int)curr[i].rgbBlue;

		// for (unsigned int j = 0; j < prev.size(); j++) {
		for (unsigned int j = (height / 2 - 5) * width + (width / 2 - 1); j < (height / 2 - 5) * width + (width / 2 - 1) + 2; j++) {
			// index = y * width + x; // get 1d array index
			prevRed = (int)prev[j].rgbRed;
			prevGreen = (int)prev[j].rgbGreen;
			prevBlue = (int)prev[j].rgbBlue;
			if ((abs(currRed - prevRed) + abs(currGreen - prevGreen) + abs(currBlue - prevBlue) < 30) ) {
				// && (abs(currRed - prevRed) < tolerance/3 && abs(currGreen - prevGreen) < tolerance/3 && abs(currBlue - prevBlue) < tolerance/3)
				break;
			} else if (j == (prev.size() - 1) && (abs(currRed - prevRed) + abs(currGreen - prevGreen) + abs(currBlue - prevBlue) > 30)) {
				//if (diffCount < 1) {
				//	diffCount++;
				//} else {
					result = true;
					cout << currRed << " " << currGreen << " " << currBlue << "    " << j;
				//}

			}
		}
		if (result) { break; }
	}
	return result;
}

void shoot() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); // start left click
	Sleep(10);
	//mouse_event(MOUSEEVENTF_MOVE, 0, 10, 0, 0); // First shot recoil additional dampening
	Sleep(90);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); // finish Left click
	Sleep(10);
}

void passiveRecoilCompensation() { //
	double calFactor = 0.8;
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
		if (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && enabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			for (int i = 0; i < 20; i++) {
				Sleep(10);
				mouse_event(MOUSEEVENTF_MOVE, 0, 7, 0, 0);
			}
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
			Sleep(11);
		}
/*
		if ((GetKeyState(VK_OEM_MINUS) & 0x100) != 0) {
			if (calFactor == 0.8) {
				calFactor = 2;
			} else {
				calFactor = 0.8;
			}
			cout << "calFactor: " << calFactor << endl;
			Sleep(150);
		}
		while (((GetKeyState(VK_LBUTTON) & 0x100) != 0) && enabled) {
			SendInput(1, &_VK_NUMPAD0_keyDown, sizeof(INPUT));
			Sleep(6);
			mouse_event(MOUSEEVENTF_MOVE, 0, (int)16 * calFactor, 0, 0);
			Sleep(6);
			SendInput(1, &_VK_NUMPAD0_keyUp, sizeof(INPUT));
			Sleep(6);
		}
	*/	
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
		if (!enabled) {
			enabled = true;
		}
		while ((GetKeyState(VK_CAPITAL) & 0x100) != 0) {
			enabled = false;
			Sleep(200);
		}
		Sleep(100);
	}
}

int main() {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveRecoilCompensation, 0, 0, 0);
	// CreateThread(0, 0, (LPTHREAD_START_ROUTINE) passiveLeaning, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) trackEnabled, 0, 0, 0);

	POINT a, b;
	a.x = screenWidth / 2 - width / 2;
	a.y = screenHeight / 2 - height / 2;
	b.x = screenWidth / 2 + width / 2;
	b.y = screenHeight / 2 + height / 2;

	RGBQUAD* curr;
	int preScanCount = 20;

	while (1) {
		if ((GetKeyState(VK_CONTROL) & 0x100) != 0) { // while ctrl pressed
		// if ((GetKeyState(VK_LBUTTON) & 0x100) != 0) { // while lmb pressed
			cout << "Activate motion trigger" << endl;

			vector<RGBQUAD> ignore;
			for (int i = 0; i < 40; i++) {
				curr = scan(a, b);
				updateIgnore(ignore, curr);
				if (i == 0) {
					preScanComplete = false;
					CreateThread(0, 0, (LPTHREAD_START_ROUTINE)scanCoverageRoutine, 0, 0, 0);
				}
				delete[] curr;
				Sleep(0);
			}
			preScanComplete = true;
			/*
			int ignoreRed, ignoreGreen, ignoreBlue;
			for (int k = 0; k < ignore.size(); k++) {
				ignoreRed = (int)ignore[k].rgbRed;
				ignoreGreen = (int)ignore[k].rgbGreen;
				ignoreBlue = (int)ignore[k].rgbBlue;
			}
			//cout << ignoreRed << " " << ignoreGreen << " " << ignoreBlue << endl;
			*/

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
