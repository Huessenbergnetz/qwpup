/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_ENUMS_H
#define QWPUP_ENUMS_H

enum class Error : int { None = 0, Config = 1, File = 2, Internal = 3, Security = 4 };

enum class VersionPart : int { Invalid, Major, Minor, Patch };

#endif // QWPUP_ENUMS_H
