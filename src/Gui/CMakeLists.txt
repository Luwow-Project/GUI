add_library(Luwow.Gui STATIC)
target_include_directories(Luwow.Gui PRIVATE
    ${GUI_ROOT}/src/Gui
    ${GUI_ROOT}/src/Gui/Windows
    ${ENGINE_ROOT}
)
target_link_libraries(Luwow.Gui PRIVATE Luwow.Engine)
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
