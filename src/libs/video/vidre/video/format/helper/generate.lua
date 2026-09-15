#!/usr/bin/env lua-5.4
--
-- This helper extracts pixel formats from <linux/videodev2.h> and updates the
-- the "database".
--

local tooldir = arg[0]:match("(.+)/[^/]+$")

-- helper functions

local tinsert = table.insert
local tremove = table.remove
local sformat = string.format

-- Trim string (removing leading and trailing whitespaces).
function string:trim()
    return (self:gsub("^%s*(.-)%s*$", "%1"))
end

-- Simple string quoting.
function string:quote()
    local sq, dq = self:match("'"), self:match('"')
    if sq and dq then
        error("quote not implemented for string: " .. self)
    elseif dq then
        return "'" .. self .. "'"
    else
        return '"' .. self .. '"'
    end
end

-- Split a line of a CSV document into an array.
local function csvsplit(line, sep)
    sep = sep or ","

    local fields = {}
    local field = ""
    local in_quotes = false
    for i = 1, #line do
        local char = line:sub(i, i)
        if char == '"' then
            in_quotes = not in_quotes
        elseif char == sep and not in_quotes then
            tinsert(fields, field)
            field = ""
        else
            field = field .. char
        end
    end
    tinsert(fields, field)
    return fields
end

-- Create format of the internal database.
local function makedb()
    return {
        columns = {
            "GROUP",       -- Group description
            "FOURCC",      -- FourCC code
            "V4L2",        -- V4L2 name (after removing "V4L2_*_FMT_" prefix)
            "VIDRE",       -- Vidre name (if different from V4L2)
            "USB3VISION",  -- USB3 Vision name (if different from V4L2)
            "GigEVision",  -- GigE Vision name (if different from V4L2)
            "ENDIAN",      -- Endianness (BE or empty)
            "PACKED",      -- Bit packed.
            "COMPRESSED",  -- Compressed format.
            "GRAY",        -- Grayscale format (if applicable)
            "DEPTH",       -- Bit depth
            "DESCRIPTION", -- Description of format
            "CONVERTER"    -- Suggested converter (if applicable)
        },
        rows = {},
        FOURCC = {} -- FourCC row lookup
    }
end

-- Add a new or update an existing row in the database.
local function dbinsert(t, elem)
    if elem.GROUP and elem.GROUP ~= "" then
        tinsert(t.rows, { GROUP = elem.GROUP })
        return
    end

    local fourcc = elem.FOURCC..(elem.ENDIAN or "")
    local row

    if t.FOURCC[fourcc] then
        row = t.FOURCC[fourcc]
    else
        row = {}
        t.FOURCC[fourcc] = row
        tinsert(t.rows, row)
    end

    for k, v in pairs(elem) do
        row[k] = v
    end
end

-- Write database (CSV) to a file.
local function dbwrite(t, filename, sep)
    sep = sep or ","
    local f = assert(io.open(filename, "w"))
    f:write(table.concat(t.columns, sep) .. "\n")
    for _, row in ipairs(t.rows) do
        for i, k in ipairs(t.columns) do
            if row[k] then
                if row[k]:find(sep) then
                    f:write(row[k]:quote())
                else
                    f:write(row[k])
                end
            else
                f:write("")
            end
            if i < #t.columns then
                f:write(sep)
            end
        end
        f:write("\n")
    end
    f:close()
end

-- ----------------------------------------------------------------------------------------------------------------
-- read database
-- ----------------------------------------------------------------------------------------------------------------
local user = makedb()

do
    local datafile = tooldir .. "/fourcc.csv"
    local f = assert(io.open(datafile, "r"))
    local header = csvsplit(f:read("*l"), ",")

    for line in f:lines() do
        local fields = csvsplit(line, ",")
        local elem = {}
        for i, k in ipairs(header) do
            elem[k] = fields[i]
        end
        dbinsert(user, elem)
    end
    f:close()
end

-- ----------------------------------------------------------------------------------------------------------------
-- read videodev2.h and extract pixel formats
-- ----------------------------------------------------------------------------------------------------------------
local db = makedb()

do
    local header = "/usr/include/linux/videodev2.h"

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

                comment = comment .. " " .. text:trim()

                if not comment_block then
                    dbinsert(db, { GROUP = comment:trim() })
                end
            else
                -- #define pixel_format fourcc /* depth Description */
                local macro, fourcc, rest = line:match("^#define[ \t]+([^ \t]+)[ \t]+v4l2_(fourcc[^)]+[)])(.*)$")
                if macro then
                    local depth, description = rest:match("[ \t]*/[*][ \t]*([1-9][0-9]?)[^0-9-][ \t]*(.*)[*]/[ \t]*$")
                    if not depth then
                        description = rest:match("[ \t]*/[*](.*)[*]/[ \t]*$")
                        if description then
                            description = description:trim()
                        end
                    end

                    local fourcc, chars = fourcc:match("^([a-z_]+)[(](.*)[)][ \t]*$")
                    chars = chars:gsub("'([^'])'[^']*", "%1")
                    if not chars then
                        error("Invalid pattern for FourCC: " .. line)
                    end

                    local endian
                    if fourcc:match("_be$") then
                        endian = "BE"
                    else
                        endian = ""
                    end

                    -- output
                    macro = macro:match("V4L2_(.*)")
                    depth = depth or ""
                    description = (description or ""):trim()

                    dbinsert(db,
                        { V4L2 = macro, FOURCC = chars, ENDIAN = endian, DEPTH = depth, DESCRIPTION = description })

                    goto nextline
                end

                -- comment or start of comment
                local spaces, rest = line:match("^([ ]*)/[*][ ]*(.*)")
                if rest then
                    comment = rest:match("(.*)[*]/[ \t]*$")
                    if comment then -- single-line comment
                        if spaces and string.len(spaces) > 0 then
                            comment = "- " .. comment
                        end
                        dbinsert(db, { GROUP = comment:trim() })
                    else
                        comment = rest:trim()
                        comment_block = true
                    end
                end
            end
        end

        ::nextline::
    end
end

while db.rows[#db.rows].GROUP do
    tremove(db.rows)
end

dbwrite(db, tooldir .. "/raw.csv")

-- ----------------------------------------------------------------------------------------------------------------
-- merge
-- ----------------------------------------------------------------------------------------------------------------
print("Merging databases...")
for _, row in ipairs(user.rows) do
    if row and row.FOURCC and #(row.FOURCC) > 0 then
        dbinsert(db, row)
    end
end

dbwrite(db, tooldir .. "/fourcc.csv")

-- ----------------------------------------------------------------------------------------------------------------
-- ----------------------------------------------------------------------------------------------------------------
-- ----------------------------------------------------------------------------------------------------------------
-- generate headers and sources
-- ----------------------------------------------------------------------------------------------------------------
-- ----------------------------------------------------------------------------------------------------------------
-- ----------------------------------------------------------------------------------------------------------------

-- ----------------------------------------------------------------------------------------------------------------
-- fourcc.hh
-- ----------------------------------------------------------------------------------------------------------------
do
    print("Generating fourcc.hh...")

    local fourcc_hh = tooldir .. "/fourcc.hh"
    local f = assert(io.open(fourcc_hh, "w"))

    f:write("enum class fourcc : uint32_t {\n")
    f:write("    invalid = static_cast<uint32_t>(-1),\n\n")
    for _, row in ipairs(db.rows) do
        if row.FOURCC and #(row.FOURCC) > 0 then
            local name = row.VIDRE
            if #name == 0 then
                name = row.V4L2
                name = name:match("FMT_(.*)"):lower()
            end
            name = name:gsub("[^a-zA-Z0-9]", "_")
            row.NAME = name

            local endian = row.ENDIAN
            if endian and endian == "BE" then
                endian = "_be"
            else
                endian = ""
            end
            f:write(string.format("    %s = fourcc%s_code(%s),\n", name, endian, row.FOURCC:quote()))
        else
            f:write(string.format("    /* %s */\n", row.GROUP or ""))
        end
    end
    f:write("};\n")
end

-- ----------------------------------------------------------------------------------------------------------------
-- fourcc.lua
-- ----------------------------------------------------------------------------------------------------------------
do
    print("Generating fourcc.lua...")

    local fourcc_lua = tooldir .. "/fourcc.lua"
    local f = assert(io.open(fourcc_lua, "w"))

    f:write("local fourcc = {\n")

    local columns = { "NAME", "USB3VISION", "GigEVision", "DESCRIPTION", "GRAY", "CONVERTER" }
    local use = {
        NAME = 'name',
        USB3VISION = 'USB3Vision',
        GigEVision = 'GigEVision',
        DESCRIPTION = 'description',
        CONVERTER = 'converter',
        GRAY = 'gray'
    }

    -- "ENDIAN",      -- Endianness (BE or empty)
    -- "DEPTH",       -- Bit depth
    -- "DESCRIPTION", -- Description of format
    -- "CONVERTER"    -- Suggested converter (if applicable)

    for _, row in ipairs(db.rows) do
        if row.FOURCC and #(row.FOURCC) > 0 then

            local value = 0
            for i = 1, 4 do
                value = value + row.FOURCC:byte(i) * (256 ^ (4-i))
            end
            
            if row.ENDIAN and row.ENDIAN == "BE" then
                value = value | 0x80000000
            end

            f:write(string.format("    [0x%08X] = {\n", value))
            for _, col in ipairs(columns) do
                f:write(string.format("        %s = %s,\n", use[col], (row[col] and #(row[col]) > 0 and row[col]:quote() or "nil")))
            end
            f:write("    },\n")
        end
    end

    f:write("}\n")

    f:write([[

fourcc.name = {}
fourcc.USB3Vision = {}
fourcc.GigEVision = {}

for k,v in pairs(fourcc) do
    if v.name then
        fourcc.name[v.name] = k
        if v.USB3Vision then
            fourcc.USB3Vision[v.USB3Vision] = k
        end
        if v.GigEVision then
            fourcc.GigEVision[v.GigEVision] = k
        end
    end
end

]])
    f:write("return fourcc\n")
end
