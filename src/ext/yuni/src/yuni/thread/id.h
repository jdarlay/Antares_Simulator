/*
** This file is part of libyuni, a cross-platform C++ framework (http://libyuni.org).
**
** This Source Code Form is subject to the terms of the Mozilla Public License
** v.2.0. If a copy of the MPL was not distributed with this file, You can
** obtain one at http://mozilla.org/MPL/2.0/.
**
** github: https://github.com/libyuni/libyuni/
** gitlab: https://gitlab.com/libyuni/libyuni/ (mirror)
*/
#pragma once
#include "../yuni.h"

namespace Yuni
{
namespace Thread
{
/*!
** \brief Get the Unique ID of the current thread
**
** \warning This value should be considered as an arbitrary value
**  (see implementation of pthread_self / GetCurrentThreadId)
*/
YUNI_DECL uint64 ID();

} // namespace Thread
} // namespace Yuni
