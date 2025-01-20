#include <iostream>
#include <wrl/client.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

export module active_audio_process_finder;

using namespace Microsoft::WRL;

export const int FATAL_ERROR_COM_INIT = 1;
export const int FATAL_ERROR_DEVICE_ENUM = 2;
export const int FATAL_ERROR_SESSION_MANAGER = 3;
export const int FATAL_ERROR_SESSION_ENUM = 4;
export const int FATAL_ERROR_GET_COUNT = 5;
export const int FATAL_ERROR_OPEN_PROCESS = 6;

/// <summary>
///  Special class for error notification with exit code supplement
/// </summary>
export class ExitCodeException : public std::exception {
public:
    ExitCodeException(int code, const std::string& message)
        : exitCode(code), message(message) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }

    int getExitCode() const {
        return exitCode;
    }

private:
    int exitCode;
    std::string message;
};


void InitializeCOM()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        throw ExitCodeException(FATAL_ERROR_COM_INIT, "Failed to initialize COM library");
    }
}


export class ActiveAudioProcessFinder {
public:
    ActiveAudioProcessFinder() {
        InitializeCOM();
    }

    ~ActiveAudioProcessFinder() {
        CoUninitialize();
    }

    DWORD GetActiveAudioProcessId() {
        HRESULT hr;

        ComPtr<IMMDevice> defaultAudioDevice = GetDefaultAudioDevice();
        ComPtr<IAudioSessionManager2> pSessionManager;
        ComPtr<IAudioSessionEnumerator> pAudioSessionEnumerator;
        int sessionCount = 0;

        // 1. Get audio session manager
        hr = GetAudioSessionManagerForDevice(defaultAudioDevice.Get(), &pSessionManager);
        if (FAILED(hr)) {
            throw ExitCodeException(FATAL_ERROR_SESSION_MANAGER, "Failed to get session manager.");
        }

        /// 2. Get enumerator from audio session manager
        hr = pSessionManager->GetSessionEnumerator(&pAudioSessionEnumerator);
        if (FAILED(hr)) {
            throw ExitCodeException(FATAL_ERROR_SESSION_ENUM, "Failed to get audio session enumerator.");
        }

        // 3. Get count of sessions from enumerator
        hr = pAudioSessionEnumerator->GetCount(&sessionCount);
        if (FAILED(hr)) {
            throw ExitCodeException(FATAL_ERROR_GET_COUNT, "Failed to get session count.");
        }

        for (auto sessionIdx = 0; sessionIdx < sessionCount; sessionIdx++) {
            AudioSessionState sessionState;
            DWORD activeSessionProcessId;

            // 4. Get standard audio session
			ComPtr<IAudioSessionControl> pSessionControl;
			hr = pAudioSessionEnumerator->GetSession(sessionIdx, &pSessionControl);
			if (FAILED(hr)) {
				continue;
			}

            // 5. Get audio session 2
			IAudioSessionControl2* pSessionControl2;
			hr = pSessionControl->QueryInterface(&pSessionControl2);
			if (FAILED(hr)) {
				continue;
			}

            // 6. Check session state (if it's active)
            hr = pSessionControl2->GetState(&sessionState);
            if (FAILED(hr) || sessionState != AudioSessionStateActive) {
                continue;
            }

            // 7. Get active session process id to pass it further
            hr = pSessionControl2->GetProcessId(&activeSessionProcessId);
            if (FAILED(hr)) {
                continue;
            }

            return activeSessionProcessId;
        }
    }

private:
    HRESULT GetAudioSessionManagerForDevice(IMMDevice* device, IAudioSessionManager2** sessionManager)
    {
        if (!device || !sessionManager)
            throw ExitCodeException(FATAL_ERROR_SESSION_MANAGER, "Invalid pointer to device or session manager");

        ComPtr<IAudioSessionManager2> pSessionManager;
        HRESULT hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pSessionManager);
        if (FAILED(hr))
            throw ExitCodeException(FATAL_ERROR_SESSION_MANAGER, "Failed to activate audio session manager");

        *sessionManager = pSessionManager.Detach();
        return S_OK;
    }

    ComPtr<IMMDevice> GetDefaultAudioDevice() {
        // Initialize the audio device enumerator
        ComPtr<IMMDeviceEnumerator> pEnumerator;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (FAILED(hr)) {
            throw ExitCodeException(FATAL_ERROR_DEVICE_ENUM, "Failed to initialize audio device enumerator.");
        }

        // Get the default audio endpoint
        ComPtr<IMMDevice> pDevice;
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
        if (FAILED(hr)) {
            throw ExitCodeException(FATAL_ERROR_DEVICE_ENUM, "Failed to get default audio endpoint.");
        }

        return pDevice;
    }
};

