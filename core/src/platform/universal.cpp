#include "univesal.h"

namespace XD
{
#ifdef CC_NO_PLATFORM_SPECIAL_INIT
    void platformSpecialInit() {}
#endif

#ifdef CC_NO_PLATFORM_SPECIAL_EVENT
    void platformSpecialEvent(bool& quit, const SDL_Event& event) {}
#endif
} // namespace XD
