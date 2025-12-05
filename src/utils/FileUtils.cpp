#include "FileUtils.h"

#include <algorithm>
#include <cassert>
#include <format>
#include <array>

#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_timer.h>

#include "utils/TracyProfiler.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
  #include <shlobj.h>
#endif

std::vector<uint8_t> FileUtils::loadFile(std::string_view filePath, std::string* error)
{
    // SDL_LoadFile не используется чтобы избежать копирования памяти в вектор
    Tracy_ZoneScoped;
    SDL_IOStream* stream = SDL_IOFromFile(filePath.data(), "rb");
    if (!stream) {
        if (error)
            *error = SDL_GetError();
        return {};
    }

    auto fileSize = SDL_GetIOSize(stream);
    if (fileSize < 0) {
        // Здесь возможно чтение чанками, пока не реализовано
        // https://github.com/libsdl-org/SDL/blob/79dae1b9d60cc69251380660aca2507c6c2df481/src/io/SDL_iostream.c#L1254
        SDL_CloseIO(stream);
        if (error)
            *error = SDL_GetError();
        return {};
    } else if (fileSize == 0) {
        SDL_CloseIO(stream);
        if (error)
            *error = "Empty file";
        return {};
    }

    int64_t bytesReadTotal = 0;
    std::vector<uint8_t> result(fileSize);
    while (true) {
        auto bytesRead = SDL_ReadIO(stream, result.data() + bytesReadTotal, (size_t)(fileSize - bytesReadTotal));
        auto status = SDL_GetIOStatus(stream);
        if (status == SDL_IO_STATUS_NOT_READY) {
            SDL_Delay(1);
        } else if (status == SDL_IO_STATUS_ERROR) {
            SDL_CloseIO(stream);
            if (error)
                *error = SDL_GetError();
            return {};
        }

        bytesReadTotal += bytesRead;
        if (bytesReadTotal == fileSize) {
            break;
        }
    }
    SDL_CloseIO(stream);

    assert(bytesReadTotal == fileSize);
    return result;
}

bool FileUtils::saveFile(std::string_view filePath, std::span<const uint8_t> fileData, std::string* error)
{
    Tracy_ZoneScoped;
    bool isOk = SDL_SaveFile(filePath.data(), fileData.data(), fileData.size());
    if (!isOk && error) {
        *error = SDL_GetError();
    }
    return isOk;
}

#ifdef _WIN32
std::string normalizePathForWindows(std::string_view inputPath) {
    std::string fixed(inputPath);
    std::replace(fixed.begin(), fixed.end(), '/', '\\');
    return fixed;
}

std::wstring Utf8ToWString(std::string_view utf8)
{
    if (utf8.empty()) {
        return {};
    }

    int needed = MultiByteToWideChar(CP_UTF8, 0,
                                     utf8.data(), utf8.size(),
                                     NULL, 0);
    if (needed == 0) {
        return {};
    }

    std::wstring result;
    result.resize(needed);

    int converted = MultiByteToWideChar(CP_UTF8, 0,
                                        utf8.data(), utf8.size(),
                                        result.data(), needed);
    if (converted == 0) {
        return {};
    }

    return result;
}

std::string WStringToUtf8(std::wstring_view wstr)
{
    if (wstr.empty()) {
        return {};
    }

    int needed = WideCharToMultiByte(CP_UTF8, 0,
                                     wstr.data(), wstr.size(),
                                     NULL, 0,
                                     NULL, NULL);
    if (needed == 0) {
        return {};
    }

    std::string result;
    result.resize(needed);

    int converted = WideCharToMultiByte(CP_UTF8, 0,
                                        wstr.data(), wstr.size(),
                                        result.data(), needed,
                                        NULL, NULL);
    if (converted == 0) {
        return {};
    }

    return result;
}

std::string HResultToStringUtf8(HRESULT hr)
{
    // Если это HRESULT с FACILITY_WIN32, извлекаем код
    DWORD code = 0;
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32) {
        code = HRESULT_CODE(hr);
    } else {
        code = (DWORD)hr;
    }

    LPWSTR buf = nullptr;
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS;

    DWORD len = FormatMessageW(
        flags,
        nullptr,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&buf,
        0,
        nullptr);

    std::wstring wmsg;
    if (len == 0) {
        // Не удалось найти текст, формируем вручную
        wchar_t temp[64];
        swprintf_s(temp, _countof(temp), L"Unknown error 0x%08X", hr);
        wmsg = temp;
    } else {
        wmsg.assign(buf, len);
        // Убираем завершающие \r\n
        while (!wmsg.empty() && (wmsg.back() == L'\r' || wmsg.back() == L'\n')) {
            wmsg.pop_back();
        }
    }

    if (buf) {
        LocalFree(buf);
    }

    // Конвертируем в UTF‑8
    return WStringToUtf8(wmsg);
}

std::wstring winPath(std::string_view utf8Path) {
    return Utf8ToWString(normalizePathForWindows(utf8Path));
}

struct CoInitializer {
    explicit CoInitializer(DWORD flags) noexcept
    {
        m_hr = CoInitializeEx(nullptr, flags);
    }

    ~CoInitializer() noexcept
    {
        if (SUCCEEDED(m_hr))
            CoUninitialize();
    }

    HRESULT hResult() const noexcept { return m_hr; }
    explicit operator bool() const noexcept { return SUCCEEDED(m_hr); }

private:
    HRESULT m_hr = E_FAIL;
};
#endif

bool FileUtils::openFolderAndSelectItems(std::string_view path, std::span<const std::string_view> files, std::string* error)
{
#ifdef _WIN32
    assert(files.size() <= 8);

    CoInitializer comGuard(COINIT_APARTMENTTHREADED);
    if (!comGuard) {
        if (error)
            *error = std::format("Failed to CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED). {}", HResultToStringUtf8(comGuard.hResult()));
        return false;
    }

    PIDLIST_ABSOLUTE pidlFolder = ILCreateFromPathW(winPath(path).c_str());
    if (pidlFolder == nullptr) {
        if (error)
            *error = std::format("ILCreateFromPathW failed! (invalid path: {})", path);
        return false;
    }

    // При itemsSize == 0, если в path указана только директория (Например "C:\\Windows")
    // откроется проводник в "C:\\" с выделенной директорией "Windows"
    // Поэтому в этом случае добавим в items тот же путь и сделаем itemsSize = 1, тогда проводник откроется в "C:\\Windows"
    std::array<PIDLIST_ABSOLUTE, 8> items{};
    UINT itemsSize = std::max(1u, static_cast<UINT>(files.size()));
    if (files.empty()) {
        items[0] = pidlFolder;
    }

    bool createItemsPathsOk = true;
    for (size_t i = 0; i < files.size(); ++i) {
        items[i] = ILCreateFromPathW(winPath(files[i]).c_str());
        if (items[i] == nullptr) {
            if (error)
                *error = std::format("ILCreateFromPathW failed! (invalid path: {})", files[i]);
            createItemsPathsOk = false;
            break;
        }
    }

    // Чистим память и выходим
    // При files.empty() items[0] == pidlFolder.
    // В этом случае освобождаем только pidlFolder, чтобы избежать двойного ILFree.
    if (!createItemsPathsOk) {
        ILFree(pidlFolder);
        if (!files.empty()) {
            for (PIDLIST_ABSOLUTE item : items) {
                ILFree(item);
            }
        }
        return false;
    }

    HRESULT hResult = SHOpenFolderAndSelectItems(pidlFolder, itemsSize, (PCUITEMID_CHILD_ARRAY)items.data(), 0);

    ILFree(pidlFolder);
    if (!files.empty()) {
        for (PIDLIST_ABSOLUTE item : items) {
            ILFree(item);
        }
    }

    bool isOk = SUCCEEDED(hResult);
    if (!isOk) {
        if (error)
            *error = std::format("Failed to SHOpenFolderAndSelectItems. {}", HResultToStringUtf8(hResult));
    }
    return isOk;
#else
    if (error)
        *error = "Not implemented for current OS";
    return false;
#endif
}
