return {
    version = "0.0",
    name = "dummy",
    category = "native",
    interface = "source",
    -- FIXME:
    type = "camera",
    description = [[]],
    device = "camera/dummy",
    load = {
        type = "dll",
        file = "dummy",
        when = "always"
    }
}

