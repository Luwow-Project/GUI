add_executable(Luwow.RunScriptWithGui)
target_include_directories(Luwow.RunScriptWithGui PRIVATE
    ${GUI_ROOT}/src/Gui
    ${GUI_ROOT}/src/GuiTest
    ${ENGINE_ROOT}
)
if(WIN32)
    target_include_directories(Luwow.RunScriptWithGui PRIVATE ${GUI_ROOT}/src/Gui/Windows)
elseif(APPLE)
    target_include_directories(Luwow.RunScriptWithGui PRIVATE ${GUI_ROOT}/src/Gui/Mac)
endif()
target_link_libraries(Luwow.RunScriptWithGui PRIVATE
    Luwow.Engine
    Luwow.Gui
    Luau.Ast
    Luau.Compiler
    Luau.VM
    Luau.Common
)
set_target_properties(Luwow.RunScriptWithGui PROPERTIES OUTPUT_NAME runscriptwithgui)
target_sources(Luwow.RunScriptWithGui PRIVATE main.cpp)
