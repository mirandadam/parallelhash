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

#ifndef _JOB_H_
#define _JOB_H_

#include <omp.h>
#include <assert.h>
#include <string.h>
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h> //DEBUG

class Job
    {

    public:
        Job();
        virtual ~Job();
        void      Set_Unprocessed_Flags(uint64_t value);
        uint64_t  Get_Unprocessed_Flags();
        void      Clear_Unprocessed_Flag_Bits(uint64_t mask);
        uint64_t  Set_Data(const uint8_t *new_data, uint64_t new_data_count);
        void      Get_Data(const uint8_t** data_pointer, uint64_t *data_pointer_count);

    protected:
        uint64_t               unprocessed_flags;
        static const uint64_t  data_size=32*1024;
        uint8_t                data[data_size];
        uint64_t               data_count;
        omp_lock_t             job_mutex;

    private:

    };

#endif // _JOB_H_
