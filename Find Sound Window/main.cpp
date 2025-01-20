// Find Sound Window.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

import active_audio_process_finder;
import focus_window_manager;

int main()
{
	auto audioProcManager = ActiveAudioProcessFinder();
	auto activeAudioProcessId = audioProcManager.GetActiveAudioProcessId();
	std::cout << "Active audio process ID: " << activeAudioProcessId << std::endl;

	FocusMainProcessWindow(activeAudioProcessId);
}

