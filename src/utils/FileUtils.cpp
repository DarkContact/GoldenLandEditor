#include "FileUtils.h"

#include <cassert>

#include <algorithm>
#include <format>
#include <memory>
#include <array>

#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_timer.h>

#include "utils/DebugLog.h"
#include "utils/TracyProfiler.h"

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
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

// NOTE: ImageResourceBlock https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_34945
// NOTE: ThumbnailResource https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_74450
std::vector<uint8_t> FileUtils::loadJpegPhotoshopThumbnail(std::string_view filePath, std::string* error)
{
    Tracy_ZoneScoped;
    std::unique_ptr<SDL_IOStream, decltype(&SDL_CloseIO)> streamPtr = {
        SDL_IOFromFile(filePath.data(), "rb"),
        SDL_CloseIO
    };

    if (!streamPtr) {
        if (error)
            *error = SDL_GetError();
        return {};
    }

    const uint8_t SOS = 0xDA;   // Start of scan
    const uint8_t APP13 = 0xED; // Application #13

    // Пропускаем SOI
    SDL_SeekIO(streamPtr.get(), 2, SDL_IO_SEEK_SET);

    bool foundApp13 = false;
    uint8_t markerAndLength[4];
    uint16_t length;
    while (true) {
        // Читаем marker + length
        if (SDL_ReadIO(streamPtr.get(), markerAndLength, 4) != 4)
            break;

        uint8_t id = markerAndLength[1];
        if (id == SOS)
            break;

        // Big-Endian
        length = (markerAndLength[2] << 8) | markerAndLength[3];
        if (id == APP13) {
            foundApp13 = true;
            break;
        }

        // Пропускаем тело сегмента
        SDL_SeekIO(streamPtr.get(), length - 2, SDL_IO_SEEK_CUR);
    }

    if (!foundApp13) {
        if (error)
            *error = "No thumbnail.";
        return {};
    }

    std::vector<uint8_t> app13Buffer;
    size_t app13Size = length - 2;
    app13Buffer.resize(app13Size);

    if (SDL_ReadIO(streamPtr.get(), app13Buffer.data(), app13Size) != app13Size) {
        if (error)
            *error = "Failed to read APP13.";
        return {};
    }

    for (size_t i = 0; i < app13Size; ++i) {
        if (app13Buffer[i] == '8') {
            if (app13Buffer[i + 1] == 'B'
                && app13Buffer[i + 2] == 'I'
                && app13Buffer[i + 3] == 'M')
            {
                uint16_t uid = (app13Buffer[i + 4] << 8) | app13Buffer[i + 5];
                if (uid == 0x040C) { // Thumbnail resources
                    size_t offset = i + 6;

                    // ---- Skip Pascal name ----
                    uint8_t nameLen = app13Buffer[offset];
                    offset += 1 + nameLen;
                    if (offset & 1) offset++; // even padding

                    // ---- Resource size ----
                    if (offset + 4 > app13Size)
                        break;

                    uint32_t resourceSize =
                        (app13Buffer[offset] << 24) |
                        (app13Buffer[offset + 1] << 16) |
                        (app13Buffer[offset + 2] << 8) |
                        (app13Buffer[offset + 3]);

                    offset += 4;

                    if (offset + resourceSize > app13Size)
                        break;

                    // ---- Parse ThumbnailResource ----
                    const uint8_t* data = app13Buffer.data() + offset;

                    uint32_t format = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
                    if (format != 1) {
                        if (error)
                            *error = "Thumbnail is not JPEG.";
                        return {};
                    }

                    uint32_t sizeAfterCompression =
                        (data[20] << 24) | (data[21] << 16) | (data[22] << 8) | data[23];

                    if (28 + sizeAfterCompression > resourceSize) {
                        if (error)
                            *error = "Invalid thumbnail size.";
                        return {};
                    }

                    // --- Zero-copy: сдвигаем данные в начало вектора ---
                    size_t jpegOffset = offset + 28;
                    if (jpegOffset != 0) {
                        std::memmove(app13Buffer.data(), app13Buffer.data() + jpegOffset, sizeAfterCompression);
                    }
                    app13Buffer.resize(sizeAfterCompression);

                    return app13Buffer; // теперь vector содержит только JPEG
                }
                i += 6;
            }
        }
    }

    if (error)
        *error = "No thumbnail in APP13.";
    return {};
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
