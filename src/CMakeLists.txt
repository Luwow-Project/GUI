if(WIN32)
    luwow_add_library(Luwow.Gui
        NAME gui
        MODES SERIAL
        SOURCES
            Descriptors.cpp
            Descriptors.h
            IGuiModule.h
            Handles.cpp
            Handles.h
            Windows/Button.cpp
            Windows/Button.h
            Windows/FileDialog.cpp
            Windows/FileDialog.h
            Windows/MenuBar.cpp
            Windows/MenuBar.h
            Windows/MenuItem.cpp
            Windows/MenuItem.h
            Windows/GuiModule.cpp
            Windows/GuiModule.h
            Windows/Window.cpp
            Windows/Window.h
    )
    target_include_directories(Luwow.Gui PRIVATE
        ${GUI_ROOT}/src
        ${GUI_ROOT}/src/Windows
    )
    target_link_libraries(Luwow.Gui PUBLIC user32 ole32 shell32)
elseif(APPLE)
    luwow_add_library(Luwow.Gui
        NAME gui
        MODES SERIAL
        SOURCES
            Descriptors.cpp
            Descriptors.h
            IGuiModule.h
            Handles.cpp
            Handles.h
            Mac/Button.mm
            Mac/Button.h
            Mac/FileDialog.mm
            Mac/FileDialog.h
            Mac/MenuBar.mm
            Mac/MenuBar.h
            Mac/MenuItem.mm
            Mac/MenuItem.h
            Mac/GuiModule.mm
            Mac/GuiModule.h
            Mac/Window.mm
            Mac/Window.h
    )
    target_include_directories(Luwow.Gui PRIVATE
        ${GUI_ROOT}/src
        ${GUI_ROOT}/src/Mac
    )

    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    # PUBLIC so executables linking this static library also link AppKit/Cocoa.
    target_link_libraries(Luwow.Gui PUBLIC ${COCOA_FRAMEWORK})
    set_target_properties(Luwow.Gui PROPERTIES OBJCXX_STANDARD 20)
    target_compile_options(Luwow.Gui PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
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

    luwow_add_library(Luwow.Gui
        NAME gui
        MODES SERIAL
        SOURCES
            Descriptors.cpp
            Descriptors.h
            IGuiModule.h
            Handles.cpp
            Handles.h
            Linux/Button.cpp
            Linux/Button.h
            Linux/FileDialog.cpp
            Linux/FileDialog.h
            Linux/GtkLoop.cpp
            Linux/GtkLoop.h
            Linux/GuiModule.cpp
            Linux/GuiModule.h
            Linux/MenuBar.cpp
            Linux/MenuBar.h
            Linux/MenuItem.cpp
            Linux/MenuItem.h
            Linux/Mnemonic.h
            Linux/Window.cpp
            Linux/Window.h
    )
    target_include_directories(Luwow.Gui PRIVATE
        ${GUI_ROOT}/src
        ${GUI_ROOT}/src/Linux
    )
    # PUBLIC so executables linking this static library also link gtkmm.
    target_link_libraries(Luwow.Gui PUBLIC PkgConfig::GTKMM)
endif()
