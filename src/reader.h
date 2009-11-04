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

#ifndef _READER_H_
#define _READER_H_

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>
//#include <stdint.h>
#include <assert.h>
#include "queue.h"

class Reader
    {

    public:
        Reader();
        virtual ~Reader();
        bool Open(const char *filepath);
        void Set_Queue_Pool(Queue **queue_pool, uint32_t queue_pool_count);
        void Set_Job_Pool(Job **job_pool, uint32_t job_pool_count);
        bool Start();
        void Print_Status();

    protected:
        void Check_Configuration();
        void Translate_Buffer_To_Jobs_And_Queue();
        uint32_t Get_Next_Recyclable_Job();

        Queue**  qpool;
        uint32_t qpool_count;
        Job**    jpool;
        uint32_t jpool_count;
        uint32_t jpool_current;
        uint64_t unprocessed_flags;

        FILE     *fp;
        bool     file_is_seekable;
        bool     file_is_stdin;
        uint64_t file_size;

        static const uint64_t buffer_size=2*1024*1024;
        uint8_t               buffer[buffer_size];
        uint64_t              buffer_count;
        uint64_t              read_count;
        bool                  is_reading;

        omp_lock_t reader_mutex;

    private:

    };

#endif // _READER_H_
