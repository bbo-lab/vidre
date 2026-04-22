#!/usr/bin/env lua-5.4
--
-- This helper updates the "pixel.hh" and "fourcc.lua" files.
--

local tooldir = arg[0]:match("(.+)/[^/]+$")

local tinsert = table.insert
local tconcat = table.concat

-- update "pixel.hh"
do 
    -- read pixel.hh (split before and after the enumeration)
    local pixel_hh = tooldir .. "/../pixel.hh"

    local pattern="^([ \t]*)(enum class fourcc : uint32_t)"
    local indent,found
    local pre,post = {},{}

    local inpre = true;
    local inpost = false;
    for line in io.lines(pixel_hh) do
        if inpre then
            indent,found = line:match(pattern) 
            if found then
                inpre = false
                goto nextline
            end

            tinsert(pre,line)
        elseif inpost then
            tinsert(post,line)
        else
            if line:match("};") then
                inpost = true
            end
        end

        ::nextline::
    end

    -- assemble file
    local f = io.open(pixel_hh,"w")

    -- 1. pre
    f:write(tconcat(pre,"\n"))
    f:write("\n")

    -- 2. fourcc.hh
    for line in io.lines(tooldir .. "/fourcc.hh") do
        f:write(indent..line.."\n")
    end

    -- 3. post
    f:write(tconcat(post,"\n"))
    f:write("\n")
end

-- copy "fourcc.lua"
do 
    local fourcc_lua = tooldir .. "/../../../../../../../share/lua/vidre/video/format/fourcc.lua"
    local inf = assert(io.open(tooldir .. "/fourcc.lua", "r"))
    local outf = assert(io.open(fourcc_lua, "w"))
    for line in inf:lines() do
        outf:write(line.."\n")
    end
end