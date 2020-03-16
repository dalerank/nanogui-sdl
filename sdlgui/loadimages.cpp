#include <sdlgui/common.h>

#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

#if defined(_WIN32)
#include <SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif

NAMESPACE_BEGIN(sdlgui)

ListImages loadImageDirectory(SDL_Renderer* renderer, const std::string &path) 
{
  ListImages result;
#if !defined(_WIN32)
    DIR *dp = opendir(path.c_str());
    if (!dp)
        throw std::runtime_error("Could not open image directory!");
    struct dirent *ep;
    while ((ep = readdir(dp))) {
        const char *fname = ep->d_name;
#else
    WIN32_FIND_DATA ffd;
    std::string searchPath = path + "/*.*";
    HANDLE handle = FindFirstFileA(searchPath.c_str(), &ffd);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Could not open image directory!");
    do {
        const char *fname = ffd.cFileName;
#endif
        if (strstr(fname, "png") == nullptr)
            continue;
        std::string fullName = path + "/" + std::string(fname);
        SDL_Texture* tex = IMG_LoadTexture(renderer, fullName.c_str());
        if (tex == 0)
            throw std::runtime_error("Could not open image data!");
        ImageInfo iminfo;
        iminfo.tex = tex;
        iminfo.path = fullName;
        SDL_QueryTexture(tex, nullptr, nullptr, &iminfo.w, &iminfo.h);
        
        result.push_back(iminfo);
#if !defined(_WIN32)
    }
    closedir(dp);
#else
    } while (FindNextFileA(handle, &ffd) != 0);
    FindClose(handle);
#endif
    return result;
}

NAMESPACE_END(sdlgui)