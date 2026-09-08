# SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_executable(qwpup)

target_sources(qwpup
    PRIVATE
        main.cpp
        qwpup.h
        qwpup.cpp
        ../common/utils.h
        ../common/utils.cpp
        ../common/enums.h
)

target_include_directories(qwpup
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
)

target_link_libraries(qwpup
    PRIVATE
        Qt6::Concurrent
        PkgConfig::Brotli
)

install(TARGETS qwpup DESTINATION ${CMAKE_INSTALL_BINDIR})

set(qwpup_TS_FILES
    ${CMAKE_SOURCE_DIR}/translations/qwpup.ts
    ${CMAKE_SOURCE_DIR}/translations/qwpup_de.ts
)

set_source_files_properties(${qwpup_TS_FILES}
    PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/translations"
)

qt_add_lupdate(
    TS_FILES ${qwpup_TS_FILES}
    SOURCE_TARGETS qwpup
    OPTIONS -no-obsolete -locations none -source-language en
)

qt_add_lrelease(
    TS_FILES ${qwpup_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE qwpup_QM_FILES
    ${QWPUP_LRELEASE_OPTIONS}
)

install(FILES ${qwpup_QM_FILES} DESTINATION ${QWPUP_I18NDIR})
