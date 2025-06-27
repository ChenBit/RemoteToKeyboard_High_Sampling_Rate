#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// Function to simulate key press
void simulateKey(WORD key, bool withShift = false) {
    INPUT inputs[4] = {};
    int numInputs = withShift ? 4 : 2;
    
    // Press Shift if needed
    if (withShift) {
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_SHIFT;
    }
    
    // Press the key
    inputs[withShift ? 1 : 0].type = INPUT_KEYBOARD;
    inputs[withShift ? 1 : 0].ki.wVk = key;
    
    // Release the key
    inputs[withShift ? 2 : 1].type = INPUT_KEYBOARD;
    inputs[withShift ? 2 : 1].ki.wVk = key;
    inputs[withShift ? 2 : 1].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // Release Shift if needed
    if (withShift) {
        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_SHIFT;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    
    SendInput(numInputs, inputs, sizeof(INPUT));
}

// Function to process joystick values and simulate keyboard input
void processJoystickInput(int channel1, int channel2) {
    // Channel 1 (A/D keys)
    if (channel1 > 50 && channel1 <= 100)
        simulateKey('D', true);  // Shift + D
    else if (channel1 > 5 && channel1 <= 50)
        simulateKey('D', false); // D
    else if (channel1 > -50 && channel1 < -5)
        simulateKey('A', false); // A
    else if (channel1 >= -100 && channel1 < -50)
        simulateKey('A', true);  // Shift + A

    // Channel 2 (W/S keys)
    if (channel2 > 50 && channel2 <= 100)
        simulateKey('W', true);  // Shift + W
    else if (channel2 > 5 && channel2 <= 50)
        simulateKey('W', false); // W
    else if (channel2 > -50 && channel2 < -5)
        simulateKey('S', false); // S
    else if (channel2 >= -100 && channel2 < -50)
        simulateKey('S', true);  // Shift + S
}

int main() {
    // Get the number of joysticks
    UINT numJoysticks = joyGetNumDevs();
    if (numJoysticks == 0) {
        std::cout << "No joystick support found." << std::endl;
        return 1;
    }

    // Detect available joysticks
    std::vector<UINT> availableJoysticks;
    for (UINT i = 0; i < numJoysticks; i++) {
        JOYINFO joyInfo;
        if (joyGetPos(i, &joyInfo) == JOYERR_NOERROR) {
            availableJoysticks.push_back(i);
            std::cout << "Found joystick " << i << std::endl;
        }
    }

    if (availableJoysticks.empty()) {
        std::cout << "No joysticks found." << std::endl;
        return 1;
    }

    // Let user select a joystick
    UINT selectedJoystick;
    if (availableJoysticks.size() == 1) {
        selectedJoystick = availableJoysticks[0];
        std::cout << "Using the only available joystick: " << selectedJoystick << std::endl;
    } else {
        std::cout << "Select a joystick (";
        for (size_t i = 0; i < availableJoysticks.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << availableJoysticks[i];
        }
        std::cout << "): ";
        std::cin >> selectedJoystick;

        // Validate selection
        if (std::find(availableJoysticks.begin(), availableJoysticks.end(), selectedJoystick) 
            == availableJoysticks.end()) {
            std::cout << "Invalid joystick selection." << std::endl;
            return 1;
        }
    }

    std::cout << "Monitoring joystick input. Press Ctrl+C to exit." << std::endl;

    // Main polling loop
    while (true) {
        JOYINFO joyInfo;
        if (joyGetPos(selectedJoystick, &joyInfo) == JOYERR_NOERROR) {
            // Convert joystick coordinates to -100 to 100 range
            int channel1 = ((int)joyInfo.wXpos - 32767) * 100 / 32767;
            int channel2 = ((int)joyInfo.wYpos - 32767) * 100 / 32767;

            // Process the input
            processJoystickInput(channel1, channel2);
        }

        // Sleep for ~10ms to achieve 100Hz polling rate
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}