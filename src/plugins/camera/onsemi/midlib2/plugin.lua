{
    version = "0.0",
    name = "midlib2",
    type = "camera",
    description = [[
This plugin provides access to OnSemi/Aptina cameras accessible through 
the midlib2 interface. Users need to install DevWareX.
]],
    device = "camera/usb",
    load = {
        type = "dll",
        file = "midlib2",
        when = "always",
        onfailure = "check"
    }
}
