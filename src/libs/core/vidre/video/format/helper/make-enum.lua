#!/usr/bin/env lua-5.4
--
-- This helper creates the enum class for pixel formats (FourCC) in `pixel.hh` from `fourcc.csv`.
--
local datafile = arg[0]:match("(.+)/[^/]+$") .. "/fourcc.csv"

local sformat = string.format

local function trim(s)
    return (s:gsub("^%s*(.-)%s*$", "%1"))
end

local function csv(line, sep)
    local fields = {}
    local field = ""
    local in_quotes = false
    for i = 1, #line do
        local char = line:sub(i, i)
        if char == '"' then
            in_quotes = not in_quotes
        elseif char == sep and not in_quotes then
            table.insert(fields, field)
            field = ""
        else
            field = field .. char
        end
    end
    table.insert(fields, field)
    return fields
end

print("enum class fourcc : uint32_t {")
for line in io.lines(datafile) do
    local fields = csv(line,",")
    if #fields[1] > 0 then
        print(sformat("    /* %s */", trim(fields[1])));
    else
        local v4l2,name,packed,preferred,fourcc,endian,bit_depth,description = fields[2],fields[3],fields[4],fields[5],fields[6],fields[7],fields[8],fields[9]

        v4l2 = trim(v4l2)
        v4l2 = v4l2:match("_FMT_(.+)$") or v4l2
        print(sformat('    %s = fourcc%s("%s"), ///< %s', v4l2:lower(), (#endian > 0) and "_"..endian:lower() or "", fourcc, trim(description)))

    end
end
print("};")