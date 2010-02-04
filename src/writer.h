/***************************************************************************
 *   Copyright (C) 2009 by Daniel Araujo Miranda                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2, as    *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef _WRITER_H_
#define _WRITER_H_

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "queue.h"

class Writer
    {
    public:
        Writer();
        virtual ~Writer();
        bool Configure(const char *output_filename);
        void Start();
        Queue queue;

    private:
        bool Open(const char *output_filename);

        bool write_to_stdout;
        FILE  *fp;
        uint64_t write_count;
    };

#endif // _WRITER_H_
