#!/usr/bin/env lua-5.4
-- 
-- Call this script with the full path to libavutil/pixfmt.h as argument,
-- e.g. 
--     ./ffmpeg.lua /usr/include/x86_64-linux-gnu/libavutil/pixfmt.h
--

function trim(s)
    return (s:gsub("^%s*(.-)%s*$", "%1"))
end

section = nil
comment = false
for line in io.lines(arg[1]) do
    if section then
        if line:match("};") then
            section = nil
            goto nextline
        end
        if comment then
            if line:match("[*]/") then
                comment = false
            end
            goto nextline
        elseif line:match("/[*]") then
            comment = true
            goto nextline
        end

        line = trim(line);
        if string.len(line) > 0 then
            print(line)

        end
    elseif line:match("enum AVPixelFormat {") then
        section = "pixel-format"
    end


    ::nextline::
end





