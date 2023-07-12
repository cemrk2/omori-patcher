#ifndef OMORI_PATCHER_FS_OVERLAY_H
#define OMORI_PATCHER_FS_OVERLAY_H

void FS_RegisterDetours();
__declspec(dllexport) void AddFileMap(const char* src, const char* dst);

#endif //OMORI_PATCHER_FS_OVERLAY_H
