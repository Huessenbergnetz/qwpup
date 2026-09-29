# SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_executable(qwpup-master)

target_sources(qwpup-master
    PRIVATE
        main.cpp
        conductor.cpp
        conductor.h
        ../common/utils.h
        ../common/utils.cpp
        ../common/enums.h
)

target_include_directories(qwpup-master
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
)

install(TARGETS qwpup-master DESTINATION ${CMAKE_INSTALL_BINDIR})

set(qwpup-master_TS_FILES
    ${CMAKE_SOURCE_DIR}/translations/qwpup-master.ts
    ${CMAKE_SOURCE_DIR}/translations/qwpup-master_de.ts
)

set_source_files_properties(${qwpup-master_TS_FILES}
    PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/translations"
)

qt_add_lupdate(
    TS_FILES ${qwpup-master_TS_FILES}
    SOURCE_TARGETS qwpup-master
    OPTIONS -no-obsolete -locations none -source-language en
)

qt_add_lrelease(
    TS_FILES ${qwpup-master_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE qwpup-master_QM_FILES
    ${QWPUP_LRELEASE_OPTIONS}
)

install(FILES ${qwpup-master_QM_FILES} DESTINATION ${QWPUP_I18NDIR})
