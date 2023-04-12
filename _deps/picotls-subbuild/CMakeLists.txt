# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.16.3)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(picotls-populate NONE)

include(ExternalProject)
ExternalProject_Add(picotls-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://github.com/h2o/picotls.git" "GIT_TAG" "7d294ab4f2e0ed749b4bcfaf533225a4e0c096f7"
                    SOURCE_DIR          "/home/mininet/picoquic/_deps/picotls-src"
                    BINARY_DIR          "/home/mininet/picoquic/_deps/picotls-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
