# pico_extras_import.cmake
# A copy of the pico_extras_import.cmake from the pico-extras repository

if (DEFINED ENV{PICO_EXTRAS_PATH} AND (NOT PICO_EXTRAS_PATH))
    set(PICO_EXTRAS_PATH $ENV{PICO_EXTRAS_PATH})
    message("Using PICO_EXTRAS_PATH from environment ('${PICO_EXTRAS_PATH}')")
endif ()

set(PICO_EXTRAS_PATH "${PICO_EXTRAS_PATH}" CACHE PATH "Path to the PICO EXTRAS")
set(PICO_EXTRAS_FETCH_FROM_GIT "${PICO_EXTRAS_FETCH_FROM_GIT}" CACHE BOOL "Set to ON to fetch copy of PICO EXTRAS from git if not otherwise locatable")
set(PICO_EXTRAS_FETCH_FROM_GIT_PATH "${PICO_EXTRAS_FETCH_FROM_GIT_PATH}" CACHE FILEPATH "location to download EXTRAS")

if (NOT PICO_EXTRAS_PATH)
    if (PICO_EXTRAS_FETCH_FROM_GIT)
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (PICO_EXTRAS_FETCH_FROM_GIT_PATH)
            get_filename_component(FETCHCONTENT_BASE_DIR "${PICO_EXTRAS_FETCH_FROM_GIT_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
        endif ()
        FetchContent_Declare(
                pico_extras
                GIT_REPOSITORY https://github.com/raspberrypi/pico-extras
                GIT_TAG master
        )
        if (NOT pico_extras)
            message("Downloading PICO EXTRAS")
            FetchContent_Populate(pico_extras)
            set(PICO_EXTRAS_PATH ${pico_extras_SOURCE_DIR})
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    else ()
        if (PICO_SDK_PATH AND EXISTS "${PICO_SDK_PATH}/../pico-extras")
            set(PICO_EXTRAS_PATH ${PICO_SDK_PATH}/../pico-extras)
            message("Defaulting PICO_EXTRAS_PATH as sibling of PICO_SDK_PATH: ${PICO_EXTRAS_PATH}")
        else ()
            message(FATAL_ERROR "PICO EXTRAS location was not specified. Please set PICO_EXTRAS_PATH or set PICO_EXTRAS_FETCH_FROM_GIT to on to fetch from git.")
        endif ()
    endif ()
endif ()

set(PICO_EXTRAS_PATH "${PICO_EXTRAS_PATH}" CACHE PATH "Path to the PICO EXTRAS" FORCE)
add_subdirectory(${PICO_EXTRAS_PATH} pico_extras)