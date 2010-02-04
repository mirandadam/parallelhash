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

#include "reader.h"

//const uint32_t Reader::

Reader::Reader()
    : job_count(Queue::queue_size+2)
    {
    assert(0<job_count);
    hpool=0;
    hpool_count=0;

    job_current=0;

    fp=0;
    file_is_seekable=false;
    file_is_stdin=false;
    file_size=-1;

    buffer_count=0;
    read_count=0;
    is_reading=false;

    omp_init_lock(&reader_mutex);

    writer=0;

    }

Reader::~Reader()
    {
    if (fp!=0 && (!file_is_stdin))
        {
        fclose(fp);
        }

    fp=0;
    omp_destroy_lock(&reader_mutex);
    }

bool Reader::Open(const char *filepath)
    {
    uint32_t r;
    omp_unset_lock(&reader_mutex);
    assert(!is_reading);

    if(0==filepath)
        {
        file_is_stdin=true;
        }
    else if(0==strcmp(filepath,"-"))
        {
        file_is_stdin=true;
        }

    if(file_is_stdin)
        {
        file_is_seekable=false;
        fp=stdin;
        }
    else
        {
        fp=fopen(filepath,"r");
        }

    if (0!=fp && (!file_is_stdin) )
        {
        r=fseek(fp, 0, SEEK_END);

        if (0!=r)
            {
            file_is_seekable=false;
            file_size=-1;
            }
        else
            {
            file_is_seekable=true;
            long int i;
            i=ftell(fp);
            assert(i>=0); //TODO: fail less abruptly here
            file_size=i;
            fseek(fp, 0, SEEK_SET);
            }
        }

    r=( ((0!=fp)||file_is_stdin) ? true : false );

    omp_unset_lock(&reader_mutex);

    return r;
    }

void Reader::Set_Hasher_Pool(Hasher **hasher_pool, uint32_t hasher_pool_count)
    {
    omp_set_lock(&reader_mutex);

    assert(!is_reading);
    assert(hasher_pool!=0);
    assert(hasher_pool_count!=0);
    assert(hasher_pool_count!=UINT32_MAX);
    for(uint32_t i=0;i<hasher_pool_count;i++)
        {
        assert(hasher_pool[i]!=0);
        }

    hpool=hasher_pool;
    hpool_count=hasher_pool_count;

    omp_unset_lock(&reader_mutex);
    }

void Reader::Set_Writer(Writer *w)
    {
    omp_set_lock(&reader_mutex);

    assert(0!=w);
    writer=w;

    omp_unset_lock(&reader_mutex);
    }

bool Reader::Start()
    {

    bool r;

    omp_set_lock(&reader_mutex);
    Check_Configuration(); //do a lot of checks before starting to read.
    is_reading=true;
    omp_unset_lock(&reader_mutex);

    do
        {
        omp_set_lock(&reader_mutex);
        buffer_count=fread(buffer, sizeof(char), buffer_size, fp);

        if (0!=ferror(fp))
            buffer_count=0;

        if (buffer_count>0)
            {
            read_count=read_count+buffer_count;
            }

        Translate_Buffer_To_Jobs_And_Queue();
        omp_unset_lock(&reader_mutex);
        }
    while (buffer_count>0);

    omp_set_lock(&reader_mutex);

    //Feed an empty job so the threads waiting at the queues know it is over.
    Translate_Buffer_To_Jobs_And_Queue();

    r=(  (0==ferror(fp) && 0!=feof(fp))  ? true : false);

    fclose(fp);

    fp=0;

    omp_unset_lock(&reader_mutex);


    return r;
    }

void Reader::Print_Status()
    {

    omp_set_lock(&reader_mutex);
    fprintf(stderr,"File size:%"PRIu64" Read:%"PRIu64"\n",file_size,read_count);
    omp_unset_lock(&reader_mutex);

    }

void Reader::Translate_Buffer_To_Jobs_And_Queue()
    {

    uint32_t i,j;
    uint64_t c;

    c=0;
    //if buffer_count is zero, feed an empty job before exiting.

    do
        {
        i=Get_Next_Recyclable_Job();
        c=c+job_pool[i].Set_Data(buffer+c,buffer_count-c);

        for (j=0;j<hpool_count;j++)
            {
            assert(hpool[j]!=0);
            job_pool[i].Inc_Pending();
            hpool[j]->queue.Push(&job_pool[i]);
            }
        if(0!=writer)
            {
            job_pool[i].Inc_Pending();
            writer->queue.Push(&job_pool[i]);
            }
        }
    while (c<buffer_count);

    }

void Reader::Check_Configuration()
    {

    uint32_t max_depth;
    uint32_t d;
    uint32_t i;

    //consistency checking necessary before reading.
    assert(0!=fp);
    assert(0!=hpool);
    assert(0<hpool_count);
    assert(job_current<job_count);
    max_depth=0;

    for (i=0;i<hpool_count;i++)
        {
        assert(hpool[i]!=0);
        d=hpool[i]->queue.Get_Capacity();
        max_depth=(d>max_depth ? d : max_depth);
        }

    assert(job_count>=max_depth+2); // the pool has to have enough jobs to fill the deepest queue.

    //TODO: document this requisite or change logic to make pool wait.

    }

uint32_t Reader::Get_Next_Recyclable_Job()
    {

    uint32_t i;

    //Find the first processed job in the pool to recycle:
    i=(job_current+1)%job_count;
    while (job_pool[i].Get_Pending()!=0 and job_current!=i)
        {
        i=(i+1)%job_count;
        }

    assert(job_pool[i].Get_Pending()==0); //If this assertion fails,
    // it means that there are no processed jobs to recycle.
    // is possible that a thread is not decrementing the pending count in the job.
    // We already checked that there are enough jobs in the pool to fill
    // the deepest queue.
    job_current=i;

    return i;
    }
