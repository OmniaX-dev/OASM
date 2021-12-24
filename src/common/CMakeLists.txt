list(APPEND common_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/BitEditor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Flags.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OmniaString.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Keyboard.cpp
)

list(APPEND common_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/headers/BitEditor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/Common.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/Defines.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/Enums.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/ErrorDefines.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/Flags.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/IOManager.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/OmniaString.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/TermColor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/headers/Keyboard.hpp
)
