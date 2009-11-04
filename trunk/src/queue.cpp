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
    bool release_queue_full_lock=false;

    assert(0!=j);

    //printf("Trying to push to queue. Queue has %"PRIu32" jobs before pushing.\n",queue_count);//DEBUG
    omp_set_lock(&queue_full_lock);
    omp_set_lock(&queue_mutex);

    assert(queue_count>=0);
    assert(queue_count<queue_size);
    assert(queue_first>=0);
    assert(queue_first<queue_size);

    queue_last=(queue_first+queue_count)%queue_size;
    //TODO: check for overflows when queue_size>=max_uint/2
    queue[queue_last]=j;
    queue_count+=1;
    if (queue_count<queue_size)
        {
        release_queue_full_lock=true;
        }

    //printf("Pushed to queue. Queue has now %"PRIu32" jobs.\n",queue_count);//DEBUG

    //Release  queue_full_lock AFTER releasing queue_mutex, even if
    // we decide whether or not to release queue_full_lock before releasing
    // queue_mutex.
    omp_unset_lock(&queue_mutex);
    if (release_queue_full_lock)
        {
        omp_unset_lock(&queue_full_lock);
        }
    omp_unset_lock(&queue_empty_lock);
    //TODO: test this behaviour.
    // This is meant to lock all PUSHes and allow only a POP after the queue is full.
    // Every pop will release this lock exactly once, so if the queue was full before the pop,
    //   only one PUSH per POP will be allowed afterwards.

    
    }

Job * Queue::Pop()
    {
    
    Job * j=0;
    bool release_queue_empty_lock=false;

    omp_set_lock(&queue_empty_lock);
    omp_set_lock(&queue_mutex);

    assert(queue_count>0);
    assert(queue_count<=queue_size);
    assert(queue_first>=0);
    assert(queue_first<queue_size);

    if (queue_count>0)
        {
        j=queue[queue_first];
        queue[queue_first]=0;
        queue_first=(queue_first+1)%queue_size;
        queue_count-=1;
        }

    //printf("Popped from queue. Queue has now %"PRIu32" jobs.\n",queue_count);//DEBUG

    if (queue_count>0)
        {
        release_queue_empty_lock=true;
        }

    omp_unset_lock(&queue_mutex);
    if (release_queue_empty_lock)
        {
        omp_unset_lock(&queue_empty_lock);
        }
    omp_unset_lock(&queue_full_lock);
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
    
    return queue_count;
    
    }
