# SPDX-FileCopyrightText: 2024 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
#
# SPDX-License-Identifier: Apache-2.0

install(DIRECTORY naja
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/python/naja
    PATTERN "*~" EXCLUDE)