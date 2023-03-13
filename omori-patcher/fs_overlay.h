#ifndef OMORI_PATCHER_FS_OVERLAY_H
#define OMORI_PATCHER_FS_OVERLAY_H

#include "modloader.h"

void FS_RegisterDetours();
void FS_RegisterOverlay(const Mod& mod);

#endif //OMORI_PATCHER_FS_OVERLAY_H
