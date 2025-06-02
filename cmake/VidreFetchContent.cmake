# See https://github.com/SpontanCombust/cmake-fetchcontent-offline

# MIT License
#
# Copyright (c) 2022 Przemysław Cedro
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

if(WIN32)
    execute_process(
        COMMAND ping www.google.com -n 2
        OUTPUT_QUIET
        ERROR_QUIET
        RESULT_VARIABLE NO_CONNECTION
    )
else()
    execute_process(
        COMMAND ping www.google.com -c 2
        OUTPUT_QUIET
        ERROR_QUIET
        RESULT_VARIABLE NO_CONNECTION
    )
endif()

if(NO_CONNECTION EQUAL 0)
    set(FETCHCONTENT_FULLY_DISCONNECTED OFF)
    set(FETCHCONTENT_OFFLINE_MODE FALSE)
else()
    set(FETCHCONTENT_FULLY_DISCONNECTED ON)
    set(FETCHCONTENT_OFFLINE_MODE TRUE)
    message(WARNING "FetchContent module is now in offline mode. Dependencies won't be downloaded or updated until the internet connection is regained.")
endif()
