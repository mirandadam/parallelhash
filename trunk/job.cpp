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

#include "job.h"

Job::Job()
    {

    unprocessed_flags=0;
    data_count=0;
    omp_init_lock(&job_mutex);

    }

Job::~Job()
    {

    unprocessed_flags=0;
    data_count=0;
    omp_destroy_lock(&job_mutex);

    }

void Job::Set_Unprocessed_Flags(uint64_t value)
    {

    omp_set_lock(&job_mutex);
    unprocessed_flags=value;
    //printf("Job set unprocessed_flags=%i\n",value);//DEBUG
    omp_unset_lock(&job_mutex);

    }

uint64_t Job::Get_Unprocessed_Flags()
    {

    uint64_t r=0;
    omp_set_lock(&job_mutex);
    r=unprocessed_flags;
    omp_unset_lock(&job_mutex);
    //printf("Job get unprocessed_flags=%i\n",r); //DEBUG

    return r;
    }

void Job::Clear_Unprocessed_Flag_Bits(uint64_t mask)
    {

    omp_set_lock(&job_mutex);
    //printf("Before: %"PRIu64", mask=%"PRIu64" \n",unprocessed_flags,mask);
    assert(0!=(unprocessed_flags & mask)); //there has to be some bit to clear, or else there is a programming mistake someplace.
    unprocessed_flags= unprocessed_flags ^ (unprocessed_flags & mask) ;
    //printf("After: %"PRIu64", mask=%"PRIu64" \n",unprocessed_flags,mask);
    omp_unset_lock(&job_mutex);

    }

uint64_t Job::Set_Data(const uint8_t *new_data, uint64_t new_data_count)
    {

    uint64_t r;

    assert(new_data!=0);
    assert(new_data_count>=0);

    r=data_size;

    if (new_data_count<r)
        {
        r=new_data_count;
        }

    omp_set_lock(&job_mutex);

    if (0==unprocessed_flags) //No writing unless the job has been marked as fully processed.
        {
        data_count=0;
        memcpy(data,new_data,r);
        data_count=r;
        }
    else
        {
        r=0;
        }

    omp_unset_lock(&job_mutex);


    return r;
    }

void Job::Get_Data(const uint8_t **data_pointer, uint64_t *data_pointer_count)
    {

    assert(data_pointer!=0);
    assert(data_pointer_count!=0);
    omp_set_lock(&job_mutex);
    //Set_Data will not write to the data buffer unless the job is marked as fully processed.
    *data_pointer=data; //TODO: Test whether it is possible to write in the data buffer using data_pointer. It is not supposed to be.
    *data_pointer_count=data_count;
    omp_unset_lock(&job_mutex);

    }
