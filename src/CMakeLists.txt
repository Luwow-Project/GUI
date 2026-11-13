set(GUI_SOURCES
    Descriptors.cpp
    Descriptors.h
    GuiModule.cpp
    GuiModule.h
    IGuiModule.h
    MainThreadCall.h
    ControlNodes/Handle.h
    ControlNodes/Property.h
)
foreach(node Node Window Button MenuBar Menu MenuItem)
    list(APPEND GUI_SOURCES ControlNodes/${node}.cpp ControlNodes/${node}.h)
endforeach()

if(WIN32)
    set(GUI_PLATFORM Windows)
    set(GUI_PLATFORM_EXTENSION .cpp)
    set(GUI_PLATFORM_LIBRARIES user32 ole32 shell32)
elseif(APPLE)
    set(GUI_PLATFORM Mac)
    set(GUI_PLATFORM_EXTENSION .mm)
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    set(GUI_PLATFORM_LIBRARIES ${COCOA_FRAMEWORK})
    list(APPEND GUI_SOURCES Mac/Mnemonic.h)
elseif(UNIX)
    # gtkmm 3 has Gtk::MenuBar and a modal Gtk::FileChooserDialog, which match this
    # library's menus and synchronous showFileDialog. gtkmm 4 removed both.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTKMM IMPORTED_TARGET gtkmm-3.0)
    if(NOT TARGET PkgConfig::GTKMM)
        message(FATAL_ERROR
            "gtkmm 3 is required to build the GUI library on ${CMAKE_SYSTEM_NAME}. "
            "Install libgtkmm-3.0-dev (Debian/Ubuntu) or gtkmm3-devel (Fedora).")
    endif()
    set(GUI_PLATFORM Linux)
    set(GUI_PLATFORM_EXTENSION .cpp)
    set(GUI_PLATFORM_LIBRARIES PkgConfig::GTKMM)
    list(APPEND GUI_SOURCES Linux/Mnemonic.h)
endif()

# Each platform implements the same controls, and the main loop in its Platform file
foreach(control Window Button MenuBar Menu MenuItem FileDialog)
    list(APPEND GUI_SOURCES ${GUI_PLATFORM}/${control}${GUI_PLATFORM_EXTENSION} ${GUI_PLATFORM}/${control}.h)
endforeach()
list(APPEND GUI_SOURCES ${GUI_PLATFORM}/Platform${GUI_PLATFORM_EXTENSION})

luwow_add_library(Luwow.Gui
    NAME gui
    MODES SERIAL
    SOURCES ${GUI_SOURCES}
)
target_include_directories(Luwow.Gui PRIVATE
    ${GUI_ROOT}/src
    ${GUI_ROOT}/src/${GUI_PLATFORM}
)
# PUBLIC so executables linking this static library also link the platform's libraries
target_link_libraries(Luwow.Gui PUBLIC ${GUI_PLATFORM_LIBRARIES})

if(APPLE)
    set_target_properties(Luwow.Gui PROPERTIES OBJCXX_STANDARD 20)
    target_compile_options(Luwow.Gui PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
endif()

# Events and property changes are signals
target_link_libraries(Luwow.Gui PRIVATE Luwow.SignalCore)
