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

#define VERSION "0.2"

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
    char     *arg1; //hash algorithm
    char     *arg2; //log file
    char     *arg3; //hashwindow
    char     *arg4; //"w[indowed]" or "p[artial]" for windowed hash only (do not calculate full hash)
    uint64_t hashwindow;
    bool     window_only;
    } arg_aux;

uint32_t    hasher_count=0;          //number of hashing streams
const char* input_filename=0;        //name of the input file
const char  stdin_filename[]={"-"};  //input file name to "read from stdin"
const char* output_filename=0;       //name of the output file
const char  stdout_filename[]={"-"}; //output file name to "write to stdout"
bool        interactive=true; //flag to ask for confirmation before starting
arg_aux     arguments[MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER]; //arguments of the streams

void parse_command_line(int argc, char *argv[]);

int main(int argc, char *argv[])
    {
    //TODO: change reader logic so it will wait until a job is free instead of bailing out.
    Reader   reader; //this is the reader thread
    Writer   writer; //this is the writer thread
    Hasher*  hasher_pool[MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER]={0}; //these are the hasher threads. Every thread has a queue to process data from. The queue stores pointers to jobs.

    uint32_t nthreads=0;
    bool     file_opened=true;

    memset(arguments, 0, sizeof(arg_aux));
    parse_command_line(argc, argv);

    for(uint32_t i=0;i<hasher_count;i++)
        {
        if(0==strcmp("md5",arguments[i].arg1))
            {
            hasher_pool[i]=new Hasher_MD5();
            }
        else if(0==strcmp("sha1",arguments[i].arg1))
            {
            hasher_pool[i]=new Hasher_SHA1();
            }
        else if(0==strcmp("sha256",arguments[i].arg1))
            {
            hasher_pool[i]=new Hasher_SHA256();
            }
        else
            {
            hasher_pool[i]=new Hasher_SHA512();
            }

        file_opened=hasher_pool[i]->Configure(input_filename,
                                              arguments[i].arg2,
                                              arguments[i].hashwindow,
                                              arguments[i].window_only);
        if(! file_opened)
            {
            fprintf(stderr,"Error opening file for writing: %s\n",arguments[i].arg2);
            break;
            }
        }

    if(file_opened && 0!=output_filename)
        {
        file_opened=writer.Configure(output_filename);
        if(file_opened)
            {
            reader.Set_Writer(&writer);
            }
        }

    if(file_opened)
        {
        reader.Set_Hasher_Pool(hasher_pool,hasher_count);
        file_opened=reader.Open(input_filename);
        if(! file_opened)
            {
            fprintf(stderr,"Error opening file for reading: %s\n",input_filename);
            }
        }

    if(file_opened)
        {
        int32_t i;
        nthreads=hasher_count+2;

        #pragma omp parallel for default(shared) private(i) num_threads(nthreads)
        for(i=0;i<(int)nthreads;i++)
            {
            //fprintf(stderr,"omp_get_num_threads() returns %i\n",omp_get_num_threads());
            if(0==i)
                {
                fprintf(stderr,"Starting reader thread.\n"); //DEBUG
                reader.Start();
                fprintf(stderr,"Finished reader thread.\n"); //DEBUG
                }
            else if(1==i)
                {
                if (0!=output_filename)
                   {
                   fprintf(stderr,"Starting writer thread.\n"); //DEBUG
                   writer.Start();
                   fprintf(stderr,"Finished writer thread.\n"); //DEBUG
                   }
                }
            else
                {
                fprintf(stderr,"Starting hasher thread %u.\n",i-2); //DEBUG
                hasher_pool[i-2]->Start();
                fprintf(stderr,"Finished hasher thread %u.\n",i-2); //DEBUG
                }
            }
        }

    for(uint32_t i=0;i<MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER;i++)
        {
        if(0!=hasher_pool[i])
            {
            delete hasher_pool[i];
            hasher_pool[i]=0;
            }
        }


    return 0;
    }


uint64_t parse_hashwindow ( char *arg3 )
    {
    size_t   s1=0;
    size_t   s2=0;
    size_t   s3=0;
    char     *suffix=0;
    uint64_t multiplier=1;
    uint64_t hashwindow=0;

    s1=strspn ( arg3,".1234567890" );
    s2=strlen ( arg3 );
    assert ( s2>=s1 );
    //if there is a suffix, calculate the multiplyer:
    if ( s2>s1 )
        {
        s3=s2-s1;
        //ignore optional "B" at the end
        if ( 'B'==arg3[s2-1] || 'b'==arg3[s2-1] )
            {
            s3--;
            }
        suffix=arg3+s1;
        if ( suffix[0]=='K' || suffix[0]=='k' )
            {
            if ( suffix[1]=='i' && 2==s3 )
                {
                multiplier=1024L;
                }
            else if ( 1==s3 )
                {
                multiplier=1000L;
                }
            else multiplier=0;
            }
        else if ( suffix[0]=='M' || suffix[0]=='m' )
            {
            if ( suffix[1]=='i' && 2==s3 )
                {
                multiplier=1024L*1024L;
                }
            else if ( 1==s3 )
                {
                multiplier=1000L*1000L;
                }
            else multiplier=0;
            }
        else if ( suffix[0]=='G' || suffix[0]=='g' )
            {
            if ( suffix[1]=='i' && 2==s3 )
                {
                multiplier=1024L*1024L*1024L;
                }
            else if ( 1==s3 )
                {
                multiplier=1000L*1000L*1000L;
                }
            else multiplier=0;
            }
        else if ( suffix[0]=='T' || suffix[0]=='t' )
            {
            if ( suffix[1]=='i' && 2==s3 )
                {
                multiplier=1024L*1024L*1024L*1024L;
                }
            else if ( 1==s3 )
                {
                multiplier=1000L*1000L*1000L*1000L;
                }
            else multiplier=0;
            }
        else
            {
            multiplier=0;
            }
        }

    //if the multiplier is zero, something went wrong
    if ( 0==multiplier )
        {
        //error - unknown suffix
        printf ( "Error. Unable to parse:%s\n",arg3 );
        exit ( 1 );
        }
    arg3[s1]='\0';
    //testing for floating point:
    if ( 0!=strchr ( arg3,'.' ) )
        {
        double d;
        sscanf ( arg3,"%lf",&d );
        if ( d<=0 )
            {
            //error - negative or floating point zero chunk size.
            printf ( "Error. Negative or floating point zero chunk size:%s\n",arg3 );
            exit ( 1 );
            }
        hashwindow=d*multiplier;
        }
    else
        {
        uint64_t u;
        sscanf ( arg3,"%"PRIu64,&u );
        hashwindow=u*multiplier;
        }
    return hashwindow;
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
            fprintf(stderr,"parallelhash version "VERSION"\n");
            fprintf(stderr,"WARNING: this version of parallelhash is in ALPHA stage. Usual warnings and disclaimers apply.\n");
            fprintf(stderr,"Usage: parallelhash -h|--help\n");
            fprintf(stderr,"       parallelhash [-y|--non-interactive] [-i|--input inputfile]\n");
            fprintf(stderr,"         -a|--hash algorithm,logfile[,hashwindow] [-a|--hash algorithm,logfile[,hashwindow[,w]]] ... \n");
            fprintf(stderr,"       parallelhash [-y|--non-interactive] [-i|--input inputfile]\n");
            fprintf(stderr,"         --algorithm logfile[,hashwindow[,w]] [--algorithm logfile[,hashwindow[,w]]] ... \n");
            fprintf(stderr,"Hash an input file and write the results in log files.\n");
            fprintf(stderr,"\n");
            fprintf(stderr,"Command line options:\n");
            fprintf(stderr,"    -h --help\n");
            fprintf(stderr,"    -i --input [file name] (use filename '-' or omit this option to read from stdin)\n");
            fprintf(stderr,"    -y --non-interactive (default when reading from stdin)\n");
            fprintf(stderr,"    -a|--hash algorithm,logfile[,hashwindow[,w]]]\n");
            fprintf(stderr,"    --algorithm logfile[,hashwindow[,w]]\n");
            fprintf(stderr,"\n");
            fprintf(stderr,"Valid values for algorithm: md5, sha1, sha256 and sha512.\n");
            fprintf(stderr,"Optional flag 'w' skips calculation of the full file hash. Only the hashes of individual parts are calculated.\n");
            fprintf(stderr,"Flag 'w' can also be set with 'windowed', 'p' and 'partial'.\n");
            fprintf(stderr,"Hashwindow size units supported: K(i)B, M(i)B, G(i)B, T(i)B. The 'B' is optional.\n");
            fprintf(stderr,"Hashwindow examples: 1000 (1000 bytes), 1.1M or 1.1MB (1.1*10^6 bytes), 1Mi or 1MiB (2^20 bytes).\n");
            fprintf(stderr,"If a non-zero 'hashwindow' is declared, hashes are produced for every consecutive chunk of 'hashwindow' bytes of the input file.\n");
            fprintf(stderr,"\n");
            fprintf(stderr,"Examples:\n");
            fprintf(stderr,"    parallelhash -i /dev/sda -a md5,md5_log.txt\n");
            fprintf(stderr,"    parallelhash -i /dev/sda --hash md5,md5_log.txt\n");
            fprintf(stderr,"    parallelhash -i file.bin -a md5,md5_log.txt,1MiB\n");
            fprintf(stderr,"    parallelhash -i image.iso -a sha1,hash.log,10000000 -a sha256,256log.txt -y\n");
            fprintf(stderr,"    parallelhash -i image.iso --sha1 hash.log,10000000 --sha256 256log.txt -y\n");
            fprintf(stderr,"    parallelhash -i disk.dd --sha1 sha1.log,1Gi,w\n");
            fprintf(stderr,"    parallelhash -i disk.dd --sha1 sha1.log,1Gi,p\n");
            fprintf(stderr,"    head -c 1M /dev/zero | parallelhash --sha512 hash.txt,100K,windowed\n");
            fprintf(stderr,"\n");
            fprintf(stderr,"\n");
            fprintf(stderr,"Operational constraints:\n");
            fprintf(stderr,"    Using the same log file to write two or more logs is not supported.\n");
            fprintf(stderr,"    Hashing multiple input files is not (yet?) supported.\n");
            //fprintf(stderr,"    This application is designed to use multiple threads. In a single threaded environment, dcfldd may perform better.\n");
            fprintf(stderr,"    Copying the input to another destination is not supported ('of=' dd option).\n");
            fprintf(stderr,"    Handling of read errors is not supported. gnuddrescue is a great tool for that.\n");
            fprintf(stderr,"\n");
            //fprintf(stderr,"Miscellaneous notes:\n");
            //fprintf(stderr,"    parallelhash was written to hash forensic images faster than dcfldd when calculating multiple hashes simultaneously.\n");
            //fprintf(stderr,"    Several hash streams can be specified. For every hash stream an algorithm, a log file and, optionally, a hashwindow are declared.\n");
            //fprintf(stderr,"    The md5 and sha variants code was imported from GNU Coreutils. The rest of the source code is original.\n");
            //fprintf(stderr,"    This is a C++ project using OpenMP for multithreading.\n");
            exit(0);
            }
        else if ( 0==strcmp(argv[i],"-i") || 0==strcmp(argv[i],"--input") )
            {
            if (0!=input_filename)
                {
                //error - input file defined twice
                fprintf(stderr,"Error. Input file defined twice. Use --input or -i only once.\n");
                exit(1);
                }

            if (i+1>=argc)
                {
                //error - missing argument for option.
                fprintf(stderr,"Error. Missing argument for option %s. Use %s [file name].\n",argv[i],argv[i]);
                exit(1);
                }

            input_filename=argv[i+1];

            i=i+1;
            }
        else if ( 0==strcmp(argv[i],"-o") || 0==strcmp(argv[i],"--output") )
            {
            if (0!=output_filename)
                {
                //error - output file defined twice
                fprintf(stderr,"Error. Output file defined twice. Use --output or -o only once.\n");
                exit(1);
                }

            if (i+1>=argc)
                {
                //error - missing argument for option.
                fprintf(stderr,"Error. Missing argument for option %s. Use %s [file name].\n",argv[i],argv[i]);
                exit(1);
                }

            output_filename=argv[i+1];

            i=i+1;
            }
        else if ( 0==strcmp(argv[i],"-y") || 0==strcmp(argv[i],"--non-interactive") )
            {
            interactive=false;
            }
        else if ( 0==strcmp(argv[i],"-a") || 0==strcmp(argv[i],"--hash") )
            {
            if (i+1>=argc)
                {
                //error - missing argument for option.
                fprintf(stderr,"Error. Missing argument for option %s.\n",argv[i]);
                exit(1);
                }

            if (hasher_count>=MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER)
                {
                //error - excess threads.
                fprintf(stderr,"Error. Maximum number of hasher threads exceeded.\n");
                exit(1);
                }

            char *arg1=0;
            char *arg2=0;
            char *arg3=0;
            char *arg4=0;
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
                    arg4=strchr(arg3,',');
                    if (0!=arg4)
                        {
                        arg4[0]='\0';
                        arg4=arg4+1;
                        }
                    }
                }

            arguments[hasher_count].arg1=arg1;
            arguments[hasher_count].arg2=arg2;
            arguments[hasher_count].arg3=arg3;
            arguments[hasher_count].arg4=arg4;
            hasher_count++;
            i++;
            }
        else if ( 0==strcmp(argv[i],"--md5") ||
                  0==strcmp(argv[i],"--sha1") ||
                  0==strcmp(argv[i],"--sha256") ||
                  0==strcmp(argv[i],"--sha512"))
            {
            if (i+1>=argc)
                {
                //error - missing argument for option.
                fprintf(stderr,"Error. Missing argument for option %s.\n",argv[i]);
                exit(1);
                }

            if (hasher_count>=MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER)
                {
                //error - excess threads.
                fprintf(stderr,"Error. Maximum number of hasher threads exceeded.\n");
                exit(1);
                }

            char *arg1=0;
            char *arg2=0;
            char *arg3=0;
            char *arg4=0;
            arg1=argv[i]+2;
            arg2=argv[i+1];
            arg3=strchr(arg2,',');
            if (0!=arg3)
                {
                arg3[0]='\0';
                arg3=arg3+1;
                arg4=strchr(arg3,',');
                if (0!=arg4)
                    {
                    arg4[0]='\0';
                    arg4=arg4+1;
                    }
                }

            arguments[hasher_count].arg1=arg1;
            arguments[hasher_count].arg2=arg2;
            arguments[hasher_count].arg3=arg3;
            arguments[hasher_count].arg4=arg4;
            hasher_count++;
            i++;
            }
        else
            {
            //error - unknown command line option
            fprintf(stderr,"Error. Unknown command line option %s\n",argv[i]);
            exit(1);
            }
        }

    //Checking parsed data:
    assert(hasher_count<MAXIMUM_NUMBER_OF_HASHER_THREADS_PER_READER);

    if(0==hasher_count)
            {
            //error - no hash stream declared
            fprintf(stderr,"Error. No hash stream declared.\n");
            exit(1);
            }

    for (uint32_t i=0;i<hasher_count;i++)
        {
        //fprintf(stderr,"%s %s %s %"PRIu64"\n",arguments[i].arg1,arguments[i].arg2,arguments[i].arg3,arguments[i].hashwindow); //DEBUG

        //check for presence of required parameters
        if (0==arguments[i].arg1 || 0==arguments[i].arg2)
            {
            //error - missing parameter for hash
            fprintf(stderr,"Error. -a and --hash require arguments.\n");
            exit(1);
            }

        if (0==strlen(arguments[i].arg2))
            {
            //error - missing parameter for hash
            fprintf(stderr,"Error. Log file declared with zero length.\n");
            exit(1);
            }

        //check for supported algorithm
        if ( 0!=strcmp("md5",arguments[i].arg1) &&
             0!=strcmp("sha1",arguments[i].arg1) &&
             0!=strcmp("sha256",arguments[i].arg1) &&
             0!=strcmp("sha512",arguments[i].arg1) )
            {
            //error - unsupported algorithm
            fprintf(stderr,"Error. Algorithm not supported:%s",arguments[i].arg1);
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
            arguments[i].hashwindow=parse_hashwindow(arguments[i].arg3);
            }

        if (0!=arguments[i].arg4)
            {
            if (0==strcmp("p",arguments[i].arg4) || strcmp("partial",arguments[i].arg4) ||
                0==strcmp("w",arguments[i].arg4) || strcmp("windowed",arguments[i].arg4))
                {
                arguments[i].window_only=true;
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
                fprintf(stderr,"Error. Recording two different logs to the same log file is not supported. Repeated log file: %s\n",arguments[j].arg2);
                exit(1);
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
        uint32_t l1=0,l2=0,l3=0,l4=0;
        char temp[64]={0};
        char c='\0';

        //calculating how much space is necessary for printing the data.
        for (uint32_t i=0;i<hasher_count;i++)
            {
            l1=max(l1,strlen(arguments[i].arg1));
            l2=max(l2,strlen(arguments[i].arg2));
            snprintf(temp,64,"%"PRIu64,arguments[i].hashwindow); //DEBUG
            l3=max(l3,strlen(temp));
            if(arguments[i].arg4!=0)
                {
                l4=max(l4,strlen(arguments[i].arg4));
                }
            }

        fprintf(stderr,"(use -y or --non-interactive to suppress this prompt)\n");
        fprintf(stderr,"Input file: %s\n",input_filename);
        fprintf(stderr,"Hash streams:\n");
        for (uint32_t i=0;i<hasher_count;i++)
            {
            fprintf(stderr," %-*s  %-*s  ",l1,arguments[i].arg1,l2,arguments[i].arg2);
            if(arguments[i].hashwindow>0)
                {
                fprintf(stderr,"%*"PRIu64,l3,arguments[i].hashwindow);
                if(arguments[i].arg4!=0)
                    {
                    fprintf(stderr,"  %-*s",l4,arguments[i].arg4);
                    }
                }
            fprintf(stderr,"\n");
            }
        fprintf(stderr,"Existing logs will be overwritten. Do you wish to continue? (y/n)\n");
        if(1!=scanf("%c",&c)) //we test the scanf result just to avoid a compilation warning. It is not really necessary.
            {
            exit(1);
            }
        if('y'!=c && 'Y'!=c)
            {
            exit(1);
            }
        }
    }
