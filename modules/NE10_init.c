/*
 *  Copyright 2011-16 ARM Limited and Contributors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of ARM Limited nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY ARM LIMITED AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL ARM LIMITED AND CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "NE10.h"

// This local variable indicates whether or not the running platform supports ARM NEON
ne10_result_t is_NEON_available = NE10_ERR;

ne10_result_t ne10_HasNEON()
{
    return is_NEON_available;
}

ne10_result_t ne10_init()
{
    ne10_result_t status = NE10_ERR;
#ifndef __MACH__
    FILE*   infofile = NULL;   // To open the file /proc/cpuinfo
    char*   line = NULL;       // getline() allocates and grows this as needed
    size_t  capacity = 0;      // Current allocation of 'line'
    ne10_int32_t lines_read = 0;

    infofile = fopen ("/proc/cpuinfo", "r");

    if (!infofile)
    {
        fprintf(stderr, "ERROR: couldn't read file \"/proc/cpuinfo\".\n");
        return NE10_ERR;
    }

    // /proc/cpuinfo is a procfs seq_file, so its length is not known up front
    // (stat() reports 0) and it grows with core count and architecture revision.
    // Read it a line at a time and stop at the first NEON indicator: the tokens
    // live inside a single Features line, so no match can straddle a line break.
    while (getline (&line, &capacity, infofile) != -1)
    {
        char *c = line;
        ++lines_read;

        while ('\0' != *c)
        {
            *c = (char) tolower ((unsigned char) *c);
            ++c;
        }

        if (strstr (line, "neon") != NULL ||
            strstr (line, "asimd") != NULL)
        {
            is_NEON_available = NE10_OK;
            break;
        }
    }

    free (line);
    fclose (infofile);

    if (0 == lines_read)
    {
        fprintf (stderr, "ERROR: Couldn't read the file \"/proc/cpuinfo\". NE10_init() failed.\n");
        return NE10_ERR;
    }
#else  //__MACH__
    is_NEON_available = NE10_OK;
#endif //__MACH__

#if defined (NE10_ENABLE_MATH)
    status = ne10_init_math (is_NEON_available);
    if (status != NE10_OK)
    {
        fprintf(stderr, "ERROR: init math failed\n");
        return NE10_ERR;
    }
#endif

#if defined (NE10_ENABLE_DSP)
    status = ne10_init_dsp (is_NEON_available);
    if (status != NE10_OK)
    {
        fprintf(stderr, "ERROR: init dsp failed\n");
        return NE10_ERR;
    }
#endif

#if defined (NE10_ENABLE_IMGPROC)
    status = ne10_init_imgproc (is_NEON_available);
    if (status != NE10_OK)
    {
        fprintf(stderr, "ERROR: init imgproc failed\n");
        return NE10_ERR;
    }
#endif

#if defined (NE10_ENABLE_PHYSICS)
    status = ne10_init_physics (is_NEON_available);
    if (status != NE10_OK)
    {
        fprintf(stderr, "ERROR: init imgproc failed\n");
        return NE10_ERR;
    }
#endif

    return NE10_OK;
}
