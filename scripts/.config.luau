return {
    luau = {
        languagemode = "strict",
        lint = {
            ["*"] = true,
            LocalUnused = false
        },
        linterrors = true,
        typeerrors = true,
        globals = {},
        aliases = {
            ["Luwow/gui"] = "./gui.luau"
        }
    }
}