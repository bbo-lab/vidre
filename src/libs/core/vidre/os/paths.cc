/* VidRe
 * Copyright (C) 2025-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
 * Author(s):
 *      Jürgen "George" Sawinski¹
 *
 * Affiliations:
 * ¹ MPI f. Neurobiol. of Behavior — caesar, Bonn, Germany
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "paths.hh"

#if defined(_WIN32)
#elif defined(__linux__)
#include <cstdlib>
#elif defined(__APPLE__)
#endif

std::filesystem::path vidre::os::configdir() noexcept {
#if defined(_WIN32)
    FIXME
#elif defined(__linux__)
    const char *p = getenv("XDG_CONFIG_HOME");
    if (p) {
        if (p[0] == '~') {
            std::filesystem::path res = getenv("HOME");
            res /= p + 2;
            return res;
        }
        else
            return p;
    }
    else {
        std::filesystem::path res = getenv("HOME");
        res /= ".config";
        return res;
    }
#elif defined(__APPLE__)
#endif
}

// function s = paths(what)
//     %ANA.OS.PATHS   Get essential system-dependent paths.
//     %
//     %Syntax:
//     %   s = ana.os.paths()
//     %
//     %   returns a struct with the fields
//     %
//     %       configdir       System-dependent user-local configuration directory path.
//     %       tmpdir          System-dependent temporary storage path.
//     %       cachedir        System-dependent cache directory.
//     %       toolboxdir      Matlab®-ana's toolbox directory.
//     %
//     %   s = ana.os.paths(what)
//     %
//     %   where 'what' are the respective fields in the struct given above.
//     %
//     arguments
//         what = [];
//     end

//     if ismac
//         warning('Mac support is untested.');
//     end

//     if isempty(what)
//         s = struct();
//         s.configdir = ana.os.paths('configdir');
//         s.tmpdir = ana.os.paths('tmpdir');
//         s.toolboxdir = ana.os.paths('toolboxdir');
//         s.cachedir = ana.os.paths('cachedir');
//     else
//         switch(what)
//             case 'configdir'
//                 if ispc
//                     s = getenv('LOCALAPPDATA');
//                     if isempty(s) % older than Vista
//                         s = getenv('APPDATA');
//                         if isempty(s)
//                             error('ANA:OS:APPDATA', 'Configuration directory could not be identified.');
//                         end
//                     end
//                 elseif ismac
//                     s = fullfile(getenv('HOME'), 'Library', 'Application Support');
//                 elseif isunix
//                     s = fullfile(getenv('XDG_CONFIG_HOME'));
//                     if strlength(s) == 0
//                         s = fullfile(getenv('HOME'), '.config');
//                     end
//                 else
//                     error('Unsupported operating system')
//                 end

//                 s = fullfile(s, 'ana');
//                 if ~exist(s, 'dir')
//                     mkdir(s);
//                 end
//                 s = ana.fs.path(s);

//             case 'tmpdir'
//                 if ispc
//                     s = getenv('TEMP');
//                 elseif isunix
//                     s = '/tmp';
//                 elseif ismac
//                     s = getenv('TMPDIR');
//                 else
//                     error('Unsupported operating system')
//                 end

//                 s = fullfile(s, 'ana');
//                 if ~exist(s, 'dir')
//                     mkdir(s);
//                 end
//                 s = ana.fs.path(s);

//             case 'cachedir'
//                 if ispc
//                     s = getenv('LOCALAPPDATA');
//                     if isempty(s) % older than Vista
//                         s = getenv('APPDATA');
//                         if isempty(s)
//                             error('ANA:SYSTEM', 'Configuration directory could not be identified.');
//                         end
//                     end
//                 elseif ismac
//                     s = fullfile(getenv('HOME'), '.cache');
//                 elseif isunix
//                     s = fullfile(getenv('XDG_CACHE_HOME'));
//                     if strlength(s) == 0
//                         s = fullfile(getenv('HOME'), '.cache');
//                     end
//                 else
//                     error('Unsupported operating system')
//                 end

//                 s = fullfile(s, 'ana');
//                 if ~exist(s, 'dir')
//                     mkdir(s);
//                 end
//                 s = ana.fs.path(s);

//             case 'toolboxdir'
//                 s = fileparts(fileparts(which('ana.version')));
//                 s = ana.fs.path(s);

//         end
//     end
// end
// % Copyright (C) 2026 MPI f. Neurobiol. of Behavior — caesar
// % SPDX-License-Identifier: GPL-3.0-or-later
// % Author(s):
// %   Jürgen "George" Sawinski
