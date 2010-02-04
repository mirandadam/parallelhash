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

#include "writer.h"

Writer::Writer()
    {
    fp=0;
    write_count=0;
    write_to_stdout=false;
    }

Writer::~Writer()
    {
    if (fp!=0)
        {
        fclose(fp);
        fp=0;
        }
    }

bool Writer::Open(const char *output_filename)
    {
    bool r;
    assert(0!=output_filename);
    if(0==strcmp(output_filename,"-"))
        {
        write_to_stdout=true;
        r=true;
        }
    else
        {
        fp=fopen(output_filename,"wb");
        r=(0==fp ? false : true);
        }
    return r;
    }

void Writer::Start()
    {
    Job           *j=0;
    const uint8_t *job_data=0;
    uint64_t      job_data_size=0;

    assert(0==ferror(fp));
    assert(fp!=0 || write_to_stdout);

    //fprintf(stderr,"Starting.\n");

    write_count=0;
    do
        {
        j=queue.Pop();
        j->Get_Data(&job_data,&job_data_size);

        if (job_data_size>0)
            {
            if((! write_to_stdout) && 0==ferror(fp))
                {
                write_count+=fwrite((void *)job_data,1,job_data_size,fp);
                if(0!=ferror(fp))
                    {
                    fprintf(stderr,"Error writing to file.\n");
                    }
                }
            if(write_to_stdout)
                {
                write_count+=fwrite((void *)job_data,1,job_data_size,stdout);
                }
            }
        j->Dec_And_Get_Pending();
        }
    while (job_data_size>0);

    if(fp!=0)
	{
        fclose(fp);
        fp=0;
	}

    }

bool Writer::Configure(const char *output_filename)
    {
    return Open(output_filename);
    }

