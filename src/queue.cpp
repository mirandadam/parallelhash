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

#include "queue.h"

Queue::Queue()
    {
    queue_first=0;
    queue_count=0;

    assert(queue_size< (UINT32_MAX)/2); //if queue_size is larger than MAX_UINT32/2 some operations may overflow

    omp_init_lock(&queue_mutex);
    omp_init_lock(&queue_full_lock);
    omp_init_lock(&queue_empty_lock);

    memset (queue, 0, queue_size);

    omp_set_lock(&queue_empty_lock);
    }

Queue::~Queue()
    {
    queue_first=0;
    queue_count=0;

    omp_destroy_lock(&queue_mutex);
    omp_destroy_lock(&queue_full_lock);
    omp_destroy_lock(&queue_empty_lock);

    memset (queue, 0, queue_size);
    }

void Queue::Push(Job *j)
    {
    uint32_t queue_last;

    assert(0!=j);

    omp_set_lock(&queue_mutex);

    //assert(queue_count>=0); //this is always true for an unsigned int
    assert(queue_count<=queue_size);
    //assert(queue_first>=0); //this is always true for an unsigned int
    assert(queue_first<queue_size);

    //if the queue is full, wait for it to have an empty slot
    if(queue_count==queue_size)
        {
        //printf("%p (push) queue_count before=%"PRIu32"\n",this,queue_count);//DEBUG
        omp_test_lock(&queue_full_lock);
        omp_unset_lock(&queue_mutex);
        omp_set_lock(&queue_full_lock);
        omp_set_lock(&queue_mutex);
        //printf("%p (push)  queue_count after=%"PRIu32"\n",this,queue_count);//DEBUG
        }

    assert(queue_count<queue_size);

    queue_last=(queue_first+queue_count)%queue_size; //this will overflow in the absurd case of queue_size>=MAX_UINT32/2
    //such a high value of queue_size is prevented by an assertion in the constructor
    queue[queue_last]=j;
    queue_count+=1;

    //printf("%p Pushed to queue. Queue has now %"PRIu32" jobs.\n",this, queue_count);//DEBUG

    assert(queue_count>0);

    omp_unset_lock(&queue_empty_lock);

    omp_unset_lock(&queue_mutex);

    //TODO: test this behaviour.
    // This is meant to lock all PUSHes and allow only a POP after the queue is full.
    // Every pop will release this lock exactly once, so if the queue was full before the pop,
    //   only one PUSH per POP will be allowed afterwards.

    }

Job * Queue::Pop()
    {
    Job * j=0;

    omp_set_lock(&queue_mutex);

    //assert(queue_count>=0); //this is always true for an unsigned int
    assert(queue_count<=queue_size);
    //assert(queue_first>=0); //this is always true for an unsigned int
    assert(queue_first<queue_size);

    //if the queue is empty, wait for it to have a job pending
    if(queue_count==0)
        {
        //printf("%p (pop) queue_count before=%"PRIu32"\n",this,queue_count);//DEBUG
        omp_test_lock(&queue_empty_lock);
        omp_unset_lock(&queue_mutex);
        omp_set_lock(&queue_empty_lock);
        omp_set_lock(&queue_mutex);
        //printf("%p (pop) queue_count after=%"PRIu32"\n",this,queue_count);//DEBUG
        }

    assert(queue_count>0);

    j=queue[queue_first];
    queue[queue_first]=0;
    queue_first=(queue_first+1)%queue_size;
    queue_count-=1;

    //printf("%p Popped from queue. Queue has now %"PRIu32" jobs.\n",this,queue_count);//DEBUG

    assert(queue_count<queue_size);

    omp_unset_lock(&queue_full_lock);

    omp_unset_lock(&queue_mutex);

    //TODO: test this behaviour.
    // This is meant to lock all PUSHes and allow only a POP after the queue is full.
    // Every pop will release this lock exactly once, so if the queue was full before the pop,
    //   only one PUSH per POP will be allowed afterwards.
    
    return j;
    }

uint32_t Queue::Get_Capacity()
    {
    return queue_size;
    }

/*!
    \fn Queue::Get_Job_Count()
 */
uint32_t Queue::Get_Job_Count()
    {
    uint32_t r;
    omp_set_lock(&queue_mutex);
    r=queue_count;
    omp_unset_lock(&queue_mutex);
    return r;
    }

