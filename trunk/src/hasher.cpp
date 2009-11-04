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

#include "hasher.h"

const char Hasher::hash_name[][7]={"none","md5","sha1","sha256","sha512"};

Hasher::Hasher()
    {
    memset (result,0,maximum_hash_result_size);

    queue=0;
    fp=0;

    memset (hashed_stream_name, 0, hashed_stream_name_size);

    read_count=0;
    fragment_count=0;
    fragment_size=0; //Default - entire stream.
    processed_flag_mask=0;

    hash_algorithm=none;
    hash_size_bytes=0;
    }


Hasher::~Hasher()
    {
    if (fp!=0)
        {
        fclose(fp);
        fp=0;
        }
    }


void Hasher::Set_Queue(Queue *q)
    {
    assert(0!=q);
    queue=q;
    }

void Hasher::Set_Processed_Flag_Mask(uint64_t mask)
    {
    uint64_t i;
    uint32_t j;
    //counting the bits set in the mask:
    j=0;

    for (i=mask; 0!=i; i=i>>1)
        {
        j=j+(i & 1);
        }

    assert(1==j); //exactly one bit set.

    processed_flag_mask=mask;
    }

bool Hasher::Open(const char *filepath)
    {
    bool r;
    fp=fopen(filepath,"w");
    r=(0==fp ? false : true);
    return r;
    }

void Hasher::Set_Fragment_Size(uint64_t frag_size)
    {
    //0 means entire stream.
    //any other value means the size of the data to be hashed separately.
    fragment_size=frag_size;
    }

void Hasher::Start()
    {
    Job           *j=0;
    const uint8_t *job_data=0;
    uint64_t      job_data_size=0;
    uint64_t      c=0,l=0;

    assert(hash_algorithm!=none);
    assert(0!=queue);
    assert(0!=fp);
    assert(0==ferror(fp));
    assert(0!=processed_flag_mask);

    Write_Header();

    Reset_Hash_State();
    read_count=0;
    fragment_count=0;

    do
        {
        j=queue->Pop();
        j->Get_Data(&job_data,&job_data_size);

        if (job_data_size>0)
            {
            if (fragment_size>0)
                {
                c=0;
                //TODO: check this.

                while (read_count+job_data_size-c >= (fragment_count+1)*fragment_size)
                    {
                    fragment_count+=1;
                    l=(fragment_count*fragment_size)-read_count;
                    //printf("mask %"PRIu64" fragment_count %"PRIu64" c %"PRIu64" l %"PRIu64" job_data_size %"PRIu64" read_count %"PRIu64"\n",processed_flag_mask,fragment_count, c, l, job_data_size, read_count); //DEBUG
                    Update_State(job_data+c,l);
                    c+=l;
                    read_count+=l;
                    Calculate_Hash();
                    Write_Hash();
                    Reset_Hash_State();
                    }

                Update_State(job_data+c, job_data_size-c);

                read_count+=job_data_size-c;
                }
            else
                {
                Update_State(job_data, job_data_size);
                read_count=read_count+job_data_size;
                }
            }

        j->Clear_Unprocessed_Flag_Bits(processed_flag_mask);
        }
    while (job_data_size>0);

    //printf("fragment size %lld fragment count %lld, fs*fc %lld, read_count %lld\n", fragment_size, fragment_count, fragment_size*fragment_count, read_count); //DEBUG

    if (fragment_size>0)
        {
        assert(fragment_size*fragment_count <= read_count);

        if (fragment_size*fragment_count< read_count)
            {
            Calculate_Hash();
            Write_Hash();
            }
        }
    else
        {
        Calculate_Hash();
        Write_Hash();
        }
    }

void Hasher::Set_Hashed_Stream_Name(const char *name)
    {
    int32_t m;
    assert(0!=name);
    m=strlen(name);
    assert(m>=0);
    assert((uint32_t)m < hashed_stream_name_size);
    //TODO: fail softer when stream name too long.
    //TODO: fail softer when assertions fail - at least close open files.

    memcpy(hashed_stream_name,name,m);
    memset(hashed_stream_name+m,0,hashed_stream_name_size-m);
    }

void Hasher::Write_Header()
    {
    assert(0!=fp);
    assert(0==ferror(fp));
    fprintf(fp,"#hash algorithm: %s\n",hash_name[hash_algorithm]);
    fprintf(fp,"#input file: %s\n",hashed_stream_name);
    fflush(fp);
    }


void Hasher::Write_Hash()
    {
    assert(hash_size_bytes>0);
    assert(0!=fp);
    assert(0==ferror(fp));

    uint32_t i;
    const char t[]="0123456789abcdef\0";
    char hash_hex[hash_size_bytes*2+1];

    for (i=0;i<hash_size_bytes;i++)
        {
        hash_hex[i*2]=t[result[i]>>4];
        hash_hex[i*2+1]=t[result[i]&0x0000000f];
        }

    hash_hex[i*2]=0;

    if (fragment_size>0 && fragment_count>0)
        {
        assert(fragment_count*fragment_size<=read_count);
        fprintf(fp,"%"PRIu64" - %"PRIu64" %s\n",read_count-fragment_size,read_count-1,hash_hex);
        }
    else
        {
        fprintf(fp,"%u - %"PRIu64" %s\n",0,read_count-1,hash_hex);
        }

    fflush(fp);
    }
