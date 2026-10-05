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
endif()
