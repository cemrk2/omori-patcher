#include <mutex>
#include <sail-c++/sail-c++.h>
#include <sail-c++/image_output-c++.h>
#include <png.h>
#include "fs_overlay.h"
#include "utils.h"
#include "detours.h"

static BOOL (WINAPI* trueSetFilePointerEx)(HANDLE hFile, _LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod) = SetFilePointerEx;
static HANDLE (WINAPI* trueCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;
static BOOL (WINAPI* trueReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = ReadFile;

std::map<std::wstring, std::wstring> overlay;
std::map<std::wstring, FileData> binOverlay;
std::map<HANDLE, std::wstring> fileMap;
std::map<HANDLE, _LARGE_INTEGER> filePtrMap;
std::mutex mapMutex;

void addFileW(const Mod& mod, const wchar_t* pathWCstr)
{
    auto path = std::wstring(pathWCstr);
    auto modDir_cstr = mod.modDir.c_str();
    size_t modDir_cstrlen = strlen(modDir_cstr) + 1;
    auto modDir = new wchar_t[modDir_cstrlen];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, modDir, modDir_cstrlen, modDir_cstr, _TRUNCATE);

    auto modDirAbsWs = Utils::GetAbsolutePathW((std::wstring(L"mods/") + modDir).c_str());
    auto modDirAbs = std::wstring(modDirAbsWs);

    if (path.back() == L'\\')
    {
        HANDLE handle;
        WIN32_FIND_DATAW finfo;

        if((handle = FindFirstFileW((path + L"*").c_str(), &finfo)) != INVALID_HANDLE_VALUE)
        {
            do {
                auto name = finfo.cFileName;
                if (wcscmp(name, L".") == 0 || wcscmp(name, L"..") == 0) {
                    continue;
                }

                std::wstring istr = path + name;
                if ((GetFileAttributesW(istr.c_str()) & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                    addFileW(mod, (istr + L"\\").c_str());
                } else {
                    addFileW(mod, istr.c_str());
                }

            } while (FindNextFileW(handle, &finfo));
            FindClose(handle);
        }
    }
    else
    {
        auto asset = path.substr(modDirAbs.size()+1, path.size());
        auto fullBase = Utils::GetAbsolutePathW(asset.c_str());
        overlay[std::wstring(fullBase)] = path;
        free((void*) fullBase);
    }

    free(modDir);
    free((void*) modDirAbsWs);
}

void addFile(const Mod& mod, const Json::Value& v)
{
    auto asset = v.asString();
    auto modAsset = "mods/" + mod.modDir + "/" + asset;
    const char* assetCStr = Utils::GetAbsolutePath(asset.c_str());
    const char* modAssetCStr = Utils::GetAbsolutePath(modAsset.c_str());
    size_t assetNewSize = strlen(assetCStr) + 1;
    size_t modAssetNewSize = strlen(modAssetCStr) + 1;
    auto assetWCStr = new wchar_t[assetNewSize];
    auto modAssetWCstr = new wchar_t[modAssetNewSize];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, assetWCStr, assetNewSize, assetCStr, _TRUNCATE);
    mbstowcs_s(&convertedChars, modAssetWCstr, modAssetNewSize, modAssetCStr, _TRUNCATE);

    if (std::wstring(assetWCStr).back() == L'\\')
    {
        HANDLE handle;
        WIN32_FIND_DATAW finfo;

        if((handle = FindFirstFileW((std::wstring(modAssetWCstr) + L"*").c_str(), &finfo)) != INVALID_HANDLE_VALUE){
            do{
                auto name = finfo.cFileName;
                if (wcscmp(name, L".") == 0 || wcscmp(name, L"..") == 0)
                {
                    continue;
                }

                std::wstring istr = std::wstring(modAssetWCstr) + name;
                if ((GetFileAttributesW(istr.c_str()) & FILE_ATTRIBUTE_DIRECTORY) != 0)
                {
                    addFileW(mod, (istr + L"\\").c_str());
                }
                else
                {
                    addFileW(mod, istr.c_str());
                }

            }while(FindNextFileW(handle, &finfo));
            FindClose(handle);
        }
    }
    else
    {
        overlay[std::wstring(assetWCStr)] = modAssetWCstr;
    }
    free((void*) assetCStr);
    free((void*) assetWCStr);
    free((void*) modAssetCStr);
    free((void*) modAssetWCstr);
}

// https://gist.github.com/dobrokot/10486786
static void PngWriteCallback(png_structp  png_ptr, png_bytep data, png_size_t length) {
    auto* p = (std::vector<uint8_t>*)png_get_io_ptr(png_ptr);
    p->insert(p->end(), data, data + length);
}

bool WritePngToMemory(size_t w, size_t h, const void* dataRGBA, std::vector<uint8_t> *out) {
    out->clear();
    png_structp p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!p)
    {
        Utils::Error("png_create_write_struct() failed");
        return false;
    }

    png_infop info_ptr = png_create_info_struct(p);
    if (!info_ptr)
    {
        Utils::Error("png_create_info_struct() failed");
        return false;
    }
    if (setjmp(png_jmpbuf(p)) != 0)
    {
        Utils::Error("setjmp(png_jmpbuf(p) failed");
        return false;
    }

    png_set_IHDR(p, info_ptr, w, h, 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    std::vector<uint8_t*> rows(h);
    for (size_t y = 0; y < h; ++y)
        rows[y] = (uint8_t*)dataRGBA + y * w * 4;
    png_set_rows(p, info_ptr, &rows[0]);
    png_set_write_fn(p, out, PngWriteCallback, NULL);
    png_write_png(p, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_write_struct(&p, NULL);

    return true;
}

void addOLID(const Mod& mod, const Json::Value& deltaObj)
{
    std::string target = deltaObj["patch"].asString();
    std::string patch = "mods\\" + mod.modDir + "\\" + deltaObj["with"].asString();
    const char* targetCStr = Utils::GetAbsolutePath(target.c_str());
    size_t targetSize = strlen(targetCStr) + 1;
    auto targetWStr = new wchar_t[targetSize];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, targetWStr, targetSize, targetCStr, _TRUNCATE);
    auto olid = Utils::ReadFileData(patch.c_str());

    sail::image image(target);
    std::vector<uint8_t> imgVec(0);
    image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA);

    WritePngToMemory(image.width(), image.height(), image.pixels(), &imgVec);

    auto* buff = (uint8_t*) malloc(imgVec.size());
    memcpy(buff, imgVec.data(), imgVec.size());
    binOverlay[std::wstring(targetWStr)] = FileData{
        buff,
        imgVec.size()
    };

    free(targetWStr);
    free(olid.data);
}

void FS_RegisterOverlay(const Mod& mod)
{
    std::vector<std::string> modKeys = {"assets", "files", "maps", "data"};

    for (const auto & modKey : modKeys)
    {
        auto arr = mod.files.get(modKey, {});
        for (const auto& v : arr)
        {
            addFile(mod, v);
        }
    }

    auto image_deltas = mod.rawConfig.get("image_deltas", {});
    if (!image_deltas.empty())
    {
        for (const Json::Value& delta : image_deltas)
        {
            addOLID(mod, delta);
        }
    }

}

BOOL WINAPI hookedSetFilePointerEx(HANDLE hFile, _LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    auto filename = fileMap[hFile];
    if (binOverlay.contains(filename))
    {
        if (!filePtrMap.contains(hFile)) filePtrMap[hFile] = _LARGE_INTEGER{};
        switch (dwMoveMethod) {
            case FILE_BEGIN:
                filePtrMap[hFile].QuadPart = 0;
                *lpNewFilePointer = filePtrMap[hFile];
                break;
            case FILE_CURRENT:
                *lpNewFilePointer = filePtrMap[hFile];
                break;
            case FILE_END:
                filePtrMap[hFile].QuadPart = (long long) binOverlay[filename].size;
                *lpNewFilePointer = filePtrMap[hFile];
                break;
            default:
                Utils::Warnf("Unsupported setFilePointerEx move method: %d", dwMoveMethod);
        }
        return true;
    }
    return trueSetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
}

HANDLE WINAPI hookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    HANDLE handle;
    if (overlay.contains(Utils::GetAbsolutePathW(lpFileName)))
    {
        handle = trueCreateFileW(overlay[Utils::GetAbsolutePathW(lpFileName)].c_str(), dwDesiredAccess, dwShareMode,
                                 lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    else {
        handle = trueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                                 dwFlagsAndAttributes, hTemplateFile);
    }
    fileMap[handle] = Utils::GetAbsolutePathW(lpFileName);
    return handle;
}

BOOL WINAPI hookedReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    auto filename = fileMap[hFile];
    if (binOverlay.contains(filename))
    {
        if (lpOverlapped != nullptr)
        {
            Utils::Warn("lpOverlapped != nullptr on an overlayed file, thinks might break");
        }
        size_t len = nNumberOfBytesToRead;
        size_t offset = filePtrMap[hFile].QuadPart;
        if (len + offset > binOverlay[filename].size) len = len + offset - binOverlay[filename].size;
        filePtrMap[hFile].QuadPart += nNumberOfBytesToRead;
        if (filePtrMap[hFile].QuadPart >= binOverlay[filename].size) filePtrMap[hFile].QuadPart = len;
        memcpy(lpBuffer, binOverlay[filename].data+offset, len);
        *lpNumberOfBytesRead = len;
        return true;
    }
    return trueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

void FS_RegisterDetours()
{
    DetourAttach(&(PVOID &) trueCreateFileW, (PVOID) hookedCreateFileW);
    DetourAttach(&(PVOID &) trueReadFile, (PVOID) hookedReadFile);
    DetourAttach(&(PVOID &) trueSetFilePointerEx, (PVOID) hookedSetFilePointerEx);
}