#pragma once

#include "Common.h"

namespace slade::core
{
// Last error reported by the portable core. Frontends can expose this to users.
inline thread_local string error;
}
