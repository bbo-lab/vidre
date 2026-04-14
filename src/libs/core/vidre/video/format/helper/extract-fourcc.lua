#!/usr/bin/env lua-5.4
-- 
-- This helper extracts pixel formats from <linux/videodev2.h>
--

local header = arg[1] or "/usr/include/linux/videodev2.h"

function trim(s)
    return (s:gsub("^%s*(.-)%s*$", "%1"))
end

function quote(s)
    local sq,dq = s:match("'"),s:match('"')
    if sq and dq then
        error("quote not implemented for string: "..s)
    elseif dq then
        return "'"..s.."'"
    else
        return '"'..s..'"'
    end
end

function csv(t)
    return table.concat(t,';')
end


local start_pattern = "Pixel format[ \t]+FOURCC"
local end_pattern = "V4L2_PIX_FMT_PRIV_MAGIC"

local content = false
local comment_block = false
local comment

for line in io.lines(header) do
    if line:match(end_pattern) then 
        break;
    elseif line:match(start_pattern) then
        content = true
    elseif content then
        if comment_block then
            local text
            if line:match("[*]/") then
                comment_block = false
                text = line:match("^[ \t*]*(.*)[*]/[ \t]*$")
            else
                text = line:match("^[ \t*]*(.*)")
            end

            comment = comment .. " " .. trim(text)

            if not comment_block then
                print(csv({comment,"","","","","",""}))
            end
        else
            -- #define pixel_format fourcc /* depth Description */
            local macro,fourcc,rest = line:match("^#define[ \t]+([^ \t]+)[ \t]+v4l2_(fourcc[^)]+[)])(.*)$")
            if macro then
                local depth,description = rest:match("[ \t]*/[*][ \t]*([1-9][0-9])[^-][ \t]*(.*)[*]/[ \t]*$")
                if not depth then
                    description = rest:match("[ \t]*/[*](.*)[*]/[ \t]*$")
                    if description then
                        description = trim(description)
                    end
                end

                local fourcc,chars = fourcc:match("^([a-z_]+)[(](.*)[)][ \t]*$") 
                chars = chars:gsub("'([^'])'[^']*", "%1")
                if not chars then
                    error("Invalid pattern for FourCC: "..line)
                end

                local endian
                if fourcc:match("_le$") then
                    endian = "LE"
                else
                    endian = "BE"
                end

                -- output
                macro = macro:match("V4L2_(.*)")
                depth = depth or ""
                description = quote(description or "")

                print(csv({"",macro,"",quote(chars),endian,depth,description}))

                goto nextline
            end 

            -- comment or start of comment
            local spaces,rest = line:match("^([ ]*)/[*][ ]*(.*)")
            if rest then
                comment = rest:match("(.*)[*]/[ \t]*$")
                if comment then -- single-line comment
                    if spaces and string.len(spaces) > 0 then
                        comment = "- "..comment
                    end
                    print(csv({comment,"","","","","",""}))
                else
                    comment = trim(rest)
                    comment_block = true
                end
            end
        end
    end

    ::nextline::
end





