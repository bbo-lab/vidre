--[=[
        Project settings.

        This table contains two required entries:

            NAME            Project name.
            VERSION         Project version ({major, minor[, patch[, tweak]]}).

        as well as 

            USES            List of tools in use.

        Users are free to add additional entries.
    ]=]
PROJECT = {
    NAME = "Vidre",
    USES = {
        "",
        "C",
        "CLANG",
        "CMAKE",
        "CXX",
        "GIT"
    },
    VERSION = {
        0,
        0,
        0
    }
}

--[=[
        Amend configuration.

    ]=]
CONFIG = {
    EXTENSIONS = {
        C = {
            ".h",
            ".c"
        },
        CXX = {
            ".hh",
            ".hpp",
            ".hxx",
            ".cc",
            ".cpp",
            ".cxx"
        }
    },
    LANG = {
        C = {
            POST = {
                TIDY = "clang-format"
            },
            PRE = {}
        },
        CXX = {
            POST = {
                TIDY = "clang-format"
            },
            PRE = {}
        }
    }
}

--[=[
        Tools.
    ]=]
TOOLS = {
    ["clang-format"] = "clang-format-21 -i %q",
    ["git"] = auto,
    ["git-add"] = "git add %q",
    ["git-command"] = "git"
}

--[=[
        Package paths.

        Here, additional Lua paths may be listed, if necessary.
    ]=]
PATHS = {}
