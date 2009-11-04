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

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <omp.h>
#include <string.h>
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <assert.h>
#include "job.h"

class Queue
    {

    public:
        Queue();
        virtual ~Queue();
        void  Push(Job *j);
        Job * Pop();
        uint32_t Get_Capacity();
        uint32_t Get_Job_Count();
        static const uint32_t queue_size=32;

    protected:
        Job * queue[queue_size];
        uint32_t queue_first;
        uint32_t queue_count;
        omp_lock_t queue_mutex;
        omp_lock_t queue_full_lock;
        omp_lock_t queue_empty_lock;

    private:

    };

#endif // _QUEUE_H_
