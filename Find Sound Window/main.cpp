// Find Sound Window.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

import active_audio_process_finder;

int main()
{
	auto audioProcManager = ActiveAudioProcessFinder();
	std::cout << audioProcManager.GetActiveAudioProcessId();
}

