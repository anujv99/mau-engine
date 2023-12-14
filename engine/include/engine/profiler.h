#pragma once

#include <tracy/Tracy.hpp>

#define MAU_FRAME_MARK() FrameMark
#define MAU_PROFILE_SCOPE(name) ZoneScopedN(name)
#define MAU_PROFILE_SCOPR_COLOR(name, color) ZoneScopedNC(name, color)
