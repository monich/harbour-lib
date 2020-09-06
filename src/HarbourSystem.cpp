/*
 * Copyright (C) 2020 Jolla Ltd.
 * Copyright (C) 2020 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourSystem.h"
#include "HarbourDebug.h"

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#ifdef LIBDIR
#  define HARBOUR_LIBDIR LIBDIR
#elif defined __aarch64__
#  define HARBOUR_LIBDIR "/usr/lib64"
#else
#  define HARBOUR_LIBDIR "/usr/lib"
#endif

void* HarbourDlopen(const char* aLibFile, int aFlags)
{
    const char* libdir = HARBOUR_LIBDIR;
    const size_t libdir_len = strlen(libdir);
    const size_t libfile_len = strlen(aLibFile);
    char* filename = (char*) malloc(libdir_len + libfile_len + 2);
    void* handle;

    memcpy(filename, libdir, libdir_len);
    memcpy(filename + libdir_len + 1, aLibFile, libfile_len);
    filename[libdir_len] = '/';
    filename[libdir_len + 1 + libfile_len] = '\0';
    handle = dlopen(filename, aFlags);
    if (!handle) {
        HWARN("Failed to load" << filename);
    }
    free(filename);
    return handle;
}
