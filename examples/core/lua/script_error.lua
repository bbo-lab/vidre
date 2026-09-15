-- This file is used by examples.core.script, demonstrating the vidre::script::analyze function.
function helloworld()
    return "Hello world!"
end

error("this should not happen...")

function invalid_trailing_function()
end
