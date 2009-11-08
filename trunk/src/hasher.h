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

#ifndef _HASHER_H_
#define _HASHER_H_

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
//#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "queue.h"

#define MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER 64

class Hasher
    {

    public:
        Hasher();
        virtual ~Hasher();
        bool Configure(const char *stream_name,
                       const char *log_filepath,
                       uint64_t win_size,
                       bool win_only,
                       Queue *q,
                       uint64_t thread_processed_flag);
        void Start();

    protected:
        virtual void Update_Full_State(const uint8_t *data, uint64_t data_size)=0;
        virtual void Update_Window_State(const uint8_t *data, uint64_t data_size)=0;
        virtual void Calculate_Window_Hash()=0;
        virtual void Calculate_Full_Hash()=0;
        virtual void Reset_Window_Hash_State()=0;
        virtual void Reset_Full_Hash_State()=0;

        enum HASHALGORITHM {none=0,md5=1, sha1=2, sha256=3, sha512=4};
        HASHALGORITHM hash_algorithm;
        uint32_t      hash_size_bytes;

        static const char     hash_name[][7];//={"none","md5","sha1","sha256","sha512"};
        static const uint32_t maximum_hash_result_size=64;
        uint8_t               full_result[maximum_hash_result_size];
        uint8_t               window_result[maximum_hash_result_size];

    private:
        void Set_Queue(Queue *q);
        void Set_Window_Size(uint64_t win_size);
        void Set_Processed_Flag_Mask(uint64_t thread_processed_flag);
        void Set_Hashed_Stream_Name(const char *stream_name);
        void Set_Windowed_Mode(bool win_only);
        bool Open(const char *log_filepath);

        void Update_States(const uint8_t *data, uint64_t data_size);
        void Write_Header();
        void Write_Window_Hash();
        void Write_Full_Hash();
        void Write_Window_Hash_Footer();

        Queue *queue;
        FILE  *fp;

        static const uint32_t hashed_stream_name_size=128;
        char                  hashed_stream_name[hashed_stream_name_size];

        uint64_t read_count;
        uint64_t window_count;
        uint64_t window_size;
        uint64_t processed_flag_mask;
        bool     windowed_only;
    };

#endif // _HASHER_H_
