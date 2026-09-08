# SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0076 NEW)

project(qwpup
    VERSION 0.0.1
    DESCRIPTION "Qt based wrapper for WP CLI to automate WordPress updates with some extras."
    HOMEPAGE_URL "https://github.com/Huessenbergnetz/qwpup"
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(QT_VERSION_MAJOR 6)
set(QT_MINIMUM_VERSION 6.7)

find_package(Qt${QT_VERSION_MAJOR} ${QT_MINIMUM_VERSION} COMPONENTS Core LinguistTools Concurrent REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(Brotli REQUIRED IMPORTED_TARGET libbrotlienc)

if (Qt6_VERSION VERSION_LESS "6.10.0")
    set(QWPUP_LRELEASE_OPTIONS OPTIONS -idbased)
endif (Qt6_VERSION VERSION_LESS "6.10.0")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF)

set(QWPUP_I18NDIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations" CACHE PATH "Directory to install translations")
set(QWPUP_CONFDIR "${CMAKE_INSTALL_SYSCONFDIR}" CACHE PATH "Configuration file directory")

GNUInstallDirs_get_absolute_install_dir(QWPUP_I18N_FULL_DIR QWPUP_I18NDIR DATADIR)
GNUInstallDirs_get_absolute_install_dir(QWPUP_CONF_FULL_DIR QWPUP_CONFDIR SYSCONFDIR)

add_compile_definitions(
    QT_NO_CAST_TO_ASCII
    QT_NO_CAST_FROM_ASCII
    QT_STRICT_ITERATORS
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_USE_QSTRINGBUILDER
    QT_USE_FAST_OPERATOR_PLUS
    QT_DISABLE_DEPRECATED_UP_TO=0x060700
    QWPUP_VERSION="${PROJECT_VERSION}"
    QWPUP_I18NDIR="${QWPUP_I18N_FULL_DIR}"
    QWPUP_CONFFILE="${QWPUP_CONF_FULL_DIR}/${PROJECT_NAME}.json"
)

if (ENABLE_MAINTAINER_CFLAGS)
    add_compile_options(
        -Wall
        -Wcast-align
        -Wno-uninitialized
        -Wempty-body
        -Wformat-security
        -Wformat
        -Winit-self
    )
endif (ENABLE_MAINTAINER_CFLAGS)

add_subdirectory(qwpup)
