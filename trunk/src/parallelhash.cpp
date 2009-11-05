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

#define VERSION "0.1.2"

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <assert.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#include "job.h"
#include "queue.h"
#include "reader.h"
#include "hasher-md5.h"
#include "hasher-sha1.h"
#include "hasher-sha256.h"
#include "hasher-sha512.h"

typedef struct
    {
    char     *arg1;
    char     *arg2;
    char     *arg3;
    uint64_t chunk_size;
    } arg_aux;

uint32_t    hasher_count=0;   //number of hashing streams
const char* input_filename=0; //name of the input file
const char  stdin_filename[]={"-"};
bool        interactive=true; //flag to ask for confirmation before starting
arg_aux     arguments[MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER]={{0}}; //arguments of the streams

void parse_command_line(int argc, char *argv[]);

int main(int argc, char *argv[])
    {
    //TODO: change reader logic so it will wait until a job is free instead of bailing out.
    Reader   reader;
    Job*     job_pool[Queue::queue_size+2]={0};
    Hasher*  hasher_pool[MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER]={0};
    Queue*   queue_pool[MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER]={0};

    uint32_t nthreads=0;
    bool     file_opened=true;

    parse_command_line(argc, argv);

    for(uint32_t i=0;i<Queue::queue_size+2;i++)
        {
        job_pool[i]=new Job();
        }

    for(uint32_t i=0;i<hasher_count;i++)
        {
        queue_pool[i]=new Queue();
        if(0==strcmp("md5",arguments[i].arg1))
            {
            //printf("New md5 stream\n"); //DEBUG
            hasher_pool[i]=new Hasher_MD5();
            }
        else if(0==strcmp("sha1",arguments[i].arg1))
            {
            //printf("New sha1 stream\n"); //DEBUG
            hasher_pool[i]=new Hasher_SHA1();
            }
        else if(0==strcmp("sha256",arguments[i].arg1))
            {
            //printf("New sha256 stream\n"); //DEBUG
            hasher_pool[i]=new Hasher_SHA256();
            }
        else
            {
            //printf("New sha512 stream\n"); //DEBUG
            hasher_pool[i]=new Hasher_SHA512();
            }

        hasher_pool[i]->Set_Queue(queue_pool[i]);
        hasher_pool[i]->Set_Processed_Flag_Mask(1<<i);
        hasher_pool[i]->Set_Hashed_Stream_Name(input_filename);
        hasher_pool[i]->Set_Fragment_Size(arguments[i].chunk_size);
        file_opened=hasher_pool[i]->Open(arguments[i].arg2);
        if(! file_opened)
            {
            printf("Error opening file for writing: %s\n",arguments[i].arg2);
            break;
            }
        }

    if(file_opened)
        {
        reader.Set_Queue_Pool(queue_pool,hasher_count);
        reader.Set_Job_Pool(job_pool,Queue::queue_size+2);
        file_opened=reader.Open(input_filename);
        if(! file_opened)
            {
            printf("Error opening file for reading: %s\n",input_filename);
            }
        }

    if(file_opened)
        {
        int32_t i;
        nthreads=hasher_count+1;
        //printf("Number of threads: %"PRIu32"\n",nthreads);//DEBUG

        #pragma omp parallel for default(shared) private(i) num_threads(nthreads)
        for(i=0;i<(int)nthreads;i++)
            {
            //printf ("omp_get_num_threads() returns %i\n",omp_get_num_threads());
            if(0==i)
                {
                printf("Starting reader thread.\n"); //DEBUG
                reader.Start();
                printf("Finished reader thread.\n"); //DEBUG
                }
            else
                {
                printf("Starting hasher thread %u.\n",i); //DEBUG
                hasher_pool[i-1]->Start();
                printf("Finished hasher thread %u.\n",i); //DEBUG
                }
            }
        }

    for(uint32_t i=0;i<MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER;i++)
        {
        if(0!=queue_pool[i])
            {
            delete queue_pool[i];
            queue_pool[i]=0;
            }
        if(0!=hasher_pool[i])
            {
            delete hasher_pool[i];
            hasher_pool[i]=0;
            }
        }

    for(uint32_t i=0;i<Queue::queue_size+2;i++)
        {
        if(0!=job_pool[i])
            {
            delete job_pool[i];
            job_pool[i]=0;
            }
        }

    return 0;
    }

#define max(x,y) ((x)>(y) ? (x) : (y))

void parse_command_line(int argc, char* argv[])
    {
    //Parsing command line:
    for (int i=0;i<argc;i++)
        {
        if(0==i) i=1; //argv[0] is the program name. we skip to the first argument.
        //entering with i==0 is necessary to test for an empty command line.
        if ( 1==argc || 0==strcmp(argv[i],"-h") || 0==strcmp(argv[i],"--help") )
            {
            //TODO: document entire code (doxygen?)
            printf("parallelhash version "VERSION"\n");
            printf("WARNING: this version of parallelhash is in ALPHA stage. Usual warnings and disclaimers apply.\n");
            printf("Usage: parallelhash -h|--help\n");
            printf("       parallelhash [-y|--non-interactive] [-i|--input inputfile]\n");
            printf("         -a|--hash algorithm,logfile[,hashwindow] [-a|--hash algorithm,logfile[,hashwindow]] ... \n");
            printf("Hash an input file and write the results in log files.\n");
            printf("\n");
            printf("Command line options:\n");
            printf("    -h --help\n");
            printf("    -i --input [file name] (use filename '-' or omit this option to read from stdin)\n");
            printf("    -y --non-interactive (default when reading from stdin)\n");
            printf("    -a --hash [hash algorithm],[log file],[hashwindow (optional)]\n");
            printf("\n");
            printf("Hash algorithms supported: md5, sha1, sha256 and sha512.\n");
            printf("Hashwindow size units supported: K(i)B, M(i)B, G(i)B, T(i)B. The 'B' is optional.\n");
            printf("Hashwindow examples: 1000 (1000 bytes), 1.1M or 1.1MB (1.1*10^6 bytes), 1Mi or 1MiB (2^20 bytes).\n");
            printf("If a non-zero 'hashwindow' is declared, hashes are produced for every consecutive chunk of 'hashwindow' bytes of the input file.\n");
            printf("\n");
            printf("Examples:\n");
            printf("    parallelhash -i /dev/sda -a md5,md5_log.txt\n");
            printf("    parallelhash -i file.bin -a md5,md5_log.txt,1MiB\n");
            printf("    parallelhash -i image.iso -a sha1,hash.log,10000000 -a sha256,256log.txt -y\n");
            printf("\n");
            printf("\n");
            printf("Operational constraints:\n");
            printf("    Using the same log file to write two or more logs is not supported.\n");
            printf("    Hashing multiple input files is not (yet?) supported.\n");
            //printf("    This application is designed to use multiple threads. In a single threaded environment, dcfldd may perform better.\n");
            printf("    Copying the input to another destination is not supported ('of=' dd option).\n");
            printf("    Handling of read errors is not supported. gnuddrescue is a great tool for that.\n");
            printf("\n");
            printf("Miscellaneous notes:\n");
            //printf("    parallelhash was written to hash forensic images faster than dcfldd when calculating multiple hashes simultaneously.\n");
            printf("    Several hash streams can be specified. For every hash stream an algorithm, a log file and, optionally, a hashwindow are declared.\n");
            //printf("    The md5 and sha variants code was imported from GNU Coreutils. The rest of the source code is original.\n");
            //printf("    This is a C++ project using OpenMP for multithreading.\n");
            exit(0);
            }
        else if ( 0==strcmp(argv[i],"-i") || 0==strcmp(argv[i],"--input") )
            {
            if (0!=input_filename)
                {
                //error - input file defined twice
                printf("Error. Input file defined twice. Use --input or -i only once.\n");
                exit(1);
                }

            if (i+1>=argc)
                {
                //error - missing argument for option.
                printf("Error. Missing argument for option %s. Use %s [file name].\n",argv[i],argv[i]);
                exit(1);
                }

            input_filename=argv[i+1];

            i=i+1;
            }
        else if ( 0==strcmp(argv[i],"-y") || 0==strcmp(argv[i],"--non-interactive") )
            {
            interactive=false;
            }
        else if ( 0==strcmp(argv[i],"-a") || 0==strcmp(argv[i],"--hash") )
            {

            if (hasher_count>=MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER)
                {
                //error - excess threads.
                printf("Error. Maximum number of hasher threads exceeded. Use option -a or --hash at most %i times.\n",MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER);
                exit(1);
                }

            if (i+1>=argc)
                {
                //error - missing argument for option.
                printf("Error. Missing argument for option %s. Use %s [hash algorithm],[log file],[chunk size (optional)]\n",argv[i],argv[i]);
                exit(1);
                }

            char *arg1=0;
            char *arg2=0;
            char *arg3=0;
            arg1=argv[i+1];
            arg2=strchr(arg1,',');
            if (0!=arg2)
                {
                arg2[0]='\0';
                arg2=arg2+1;
                arg3=strchr(arg2,',');
                if (0!=arg3)
                    {
                    arg3[0]='\0';
                    arg3=arg3+1;
                    }
                }
            //printf("%s %s %s %s\n",argv[i],arg1,arg2,arg3); //DEBUG
            arguments[hasher_count].arg1=arg1;
            arguments[hasher_count].arg2=arg2;
            arguments[hasher_count].arg3=arg3;
            hasher_count++;
            i++;
            }
        else
            {
            //error - unknown command line option
            printf("Error. Unknown command line option %s\n",argv[i]);
            exit(1);
            }
        }

    //Checking parsed data:
    assert(hasher_count<MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER);

    if(0==hasher_count)
            {
            //error - no hash stream declared
            printf("Error. No hash stream declared. Use -a or --hash [hash algorithm],[log file],[chunk size (optional)]\n");
            exit(1);
            }

    for (uint32_t i=0;i<hasher_count;i++)
        {
        //printf("%s %s %s %"PRIu64"\n",arguments[i].arg1,arguments[i].arg2,arguments[i].arg3,arguments[i].chunk_size); //DEBUG

        //check for presence of required parameters
        if (0==arguments[i].arg1 || 0==arguments[i].arg2)
            {
            //error - missing parameter for hash
            printf("Error. -a and --hash require two or three arguments in the form --hash [hash algorithm],[log file],[chunk size (optional)]\n");
            exit(1);
            }

        if (0==strlen(arguments[i].arg2))
            {
            //error - missing parameter for hash
            printf("Error. Log file declared with zero length.\n");
            exit(1);
            }

        //check for supported algorithm
        if ( 0!=strcmp("md5",arguments[i].arg1) && 0!=strcmp("sha1",arguments[i].arg1) && 0!=strcmp("sha256",arguments[i].arg1) && 0!=strcmp("sha512",arguments[i].arg1) )
            {
            //error - unsupported algorithm
            printf("Error. Algorithm not supported:%s",arguments[i].arg1);
            exit(1);
            }

        //TODO: implement file name sanity checking. reject special characters?

        //check and parse chunk size
        //this is all case insensitive, which is less annoying but accepts technically incorrect units.
        bool has_arg3=false;
        if (0!=arguments[i].arg3)
            {
            has_arg3=(0<strlen(arguments[i].arg3));
            }
        if (has_arg3)
            {
            size_t   s1=0;
            size_t   s2=0;
            size_t   s3=0;
            char     *suffix=0;
            uint64_t multiplier=1;

            s1=strspn(arguments[i].arg3,".1234567890");
            s2=strlen(arguments[i].arg3);
            //printf("%s %"PRIu64" %"PRIu64"\n",arguments[i].arg3, s1, s2); //DEBUG
            assert(s2>=s1);
            //if there is a suffix, calculate the multiplyer:
            if (s2>s1)
                {
                s3=s2-s1;
                //ignore optional "B" at the end
                if ('B'==arguments[i].arg3[s2-1] || 'b'==arguments[i].arg3[s2-1])
                    {
                    s3--;
                    }
                suffix=arguments[i].arg3+s1;
                switch (suffix[0])
                    {
                    case('K'):
                    case('k'):
                        if (suffix[1]=='i' && 2==s3)
                            {
                            multiplier=1024L;
                            }
                        else if (1==s3)
                            {
                            multiplier=1000L;
                            }
                       else multiplier=0;
                       break;
                    case('M'):
                    case('m'):
                        if (suffix[1]=='i' && 2==s3)
                            {
                            multiplier=1024L*1024L;
                            }
                        else if (1==s3)
                            {
                            multiplier=1000L*1000L;
                            }
                        else multiplier=0;
                        break;
                    case('G'):
                    case('g'):
                        if (suffix[1]=='i' && 2==s3)
                            {
                            multiplier=1024L*1024L*1024L;
                            }
                        else if (1==s3)
                            {
                            multiplier=1000L*1000L*1000L;
                            }
                        else multiplier=0;
                        break;
                    case('T'):
                    case('t'):
                        if (suffix[1]=='i' && 2==s3)
                            {
                            multiplier=1024L*1024L*1024L*1024L;
                            }
                        else if (1==s3)
                            {
                            multiplier=1000L*1000L*1000L*1000L;
                            }
                        else multiplier=0;
                        break;
                    default:
                        multiplier=0;
                        break;
                    }
                }

            //if the multiplier is zero, something went wrong
            if(0==multiplier)
                {
                //error - unknown suffix
                printf("Error. Unable to parse:%s\n",arguments[i].arg3);
                exit(1);
                }
            arguments[i].arg3[s1]='\0';
            //testing for floating point:
            if(0!=strchr(arguments[i].arg3,'.'))
                {
                double d;
                sscanf(arguments[i].arg3,"%lf",&d);
                if(d<=0)
                    {
                    //error - negative or floating point zero chunk size.
                    printf("Error. Negative or floating point zero chunk size:%s\n",arguments[i].arg3);
                    exit(1);
                    }
                arguments[i].chunk_size=d*multiplier;
                }
            else
                {
                uint64_t u;
                sscanf(arguments[i].arg3,"%"PRIu64,&u);
                arguments[i].chunk_size=u*multiplier;
                }
            }

        }
    //finished checking arguments separately

    //checking for repeated output files:
    for (uint32_t i=0;i<hasher_count;i++)
        {
        for (uint32_t j=i+1;j<hasher_count;j++)
            {
            if( 0==strcmp(arguments[i].arg2,arguments[j].arg2) )
                {
                printf("Error. Recording two different logs to the same log file is not supported. Repeated log file: %s\n",arguments[j].arg2);
                }
            }
        }

    assert(hasher_count>0);
    if(0==input_filename)
        {
        input_filename=stdin_filename;
        }

    //asking for confirmation by default, except if reading from stdin
    if(interactive && 0!=strcmp(input_filename,"-"))
        {
        uint32_t l1=0,l2=0,l3=0;
        char temp[64]={0};
        int c=0;

        //calculating how much space is necessary for printing the data.
        for (uint32_t i=0;i<hasher_count;i++)
            {
            l1=max(l1,strlen(arguments[i].arg1));
            l2=max(l2,strlen(arguments[i].arg2));
            snprintf(temp,64,"%"PRIu64,arguments[i].chunk_size);
            l3=max(l3,strlen(temp));
            }

        printf("(use -y or --non-interactive to suppress this prompt)\n");
        printf("Input file: %s\n",input_filename);
        printf("Hash streams:\n");
        for (uint32_t i=0;i<hasher_count;i++)
            {
            printf(" %-*s %-*s ",l1,arguments[i].arg1,l2,arguments[i].arg2);
            if(arguments[i].chunk_size>0)
                {
                printf("%*"PRIu64,l3,arguments[i].chunk_size);
                }
            printf("\n");
            }
        printf("Existing logs will be overwritten. Do you wish to continue? (y/n)\n");
        c=getchar();
        if('y'!=c && 'Y'!=c)
            {
            exit(1);
            }
        }
    }
