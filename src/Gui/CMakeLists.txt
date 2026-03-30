add_library(Luwow.Gui STATIC)
target_include_directories(Luwow.Gui PRIVATE
    ${GUI_ROOT}/src/Gui
    ${ENGINE_ROOT}
)
target_link_libraries(Luwow.Gui PRIVATE Luwow.Engine)

if(WIN32)
    target_include_directories(Luwow.Gui PRIVATE ${GUI_ROOT}/src/Gui/Windows)
    target_sources(Luwow.Gui PRIVATE
        Descriptors.cpp
        Descriptors.h
        IGuiModule.h
        Windows/Button.cpp
        Windows/Button.h
        Windows/GuiModule.cpp
        Windows/GuiModule.h
        Windows/Window.cpp
        Windows/Window.h
    )
elseif(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    target_include_directories(Luwow.Gui PRIVATE ${GUI_ROOT}/src/Gui/Mac)
    target_sources(Luwow.Gui PRIVATE
        Descriptors.cpp
        Descriptors.h
        IGuiModule.h
        Mac/Button.mm
        Mac/Button.h
        Mac/GuiModule.mm
        Mac/GuiModule.h
        Mac/Window.mm
        Mac/Window.h
    )
    # PUBLIC so executables linking this static library also link AppKit/Cocoa.
    target_link_libraries(Luwow.Gui PUBLIC ${COCOA_FRAMEWORK})
    set_target_properties(Luwow.Gui PROPERTIES OBJCXX_STANDARD 20)
    target_compile_options(Luwow.Gui PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
endif()


# add_library(Luwow.Gui.Dll SHARED)
# target_include_directories(Luwow.Gui.dll PRIVATE
#     ${GUI_ROOT}/src/Gui
#     ${GUI_ROOT}/src/Gui/Windows
#     ${ENGINE_ROOT}
# )
# target_link_libraries(Luwow.Gui.dll PRIVATE Luwow.Engine)
# target_sources(Luwow.Gui.dll PRIVATE
#     Descriptors.cpp
#     Descriptors.h
#     IGuiModule.h
#     Windows/Button.cpp
#     Windows/Button.h
#     Windows/GuiModule.cpp
#     Windows/GuiModule.h
#     Windows/Window.cpp
#     Windows/Window.h
# )
# set_target_properties(Luwow.Gui.dll PROPERTIES OUTPUT_NAME gui)
