#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

void graph(int samp, int delay, int numofcores, long int* memo_util_arr, long int overall_value, int maxfreq, double* cpu_value_arr, int j) //gonna get rid of this
{
    printf("\033[2J\n");
    printf("\033[H\n");

    printf("Nbr of samples: %d --- every %d microSecs (%.3f secs)\n", samp, delay, (float)delay/1e6);

    printf("\n");

        //MEMORY
        printf("v Memory %.2f GB\n", memo_util_arr[j] / 1e6); 

        for(int n = 12; n > 0; n--)
        {
            if (n==12)
            {
                printf("%d GB |",(int)(overall_value/1e6)); 
            }
            else
            {
                printf("      |");
            }
            for(int i = 0; i < j; i++)
            {
                    if (ceil(memo_util_arr[i]*12.0/(int)(overall_value)) == n) //how many bars does it fill?
                    {
                        printf("#");
                    }
                    else
                    {
                        printf(" ");
                    }
            }
            printf("\n");
        }
        
        printf("0 GB  ");
        for(int o = 0; o < samp+1; o++) //+1 for the _ for the axis
        {
            printf("_");
        }
        printf("\n");
        printf("\n");

        //CPU
        printf("v CPU %.2f %%\n",cpu_value_arr[j]);
        for(int n = 10; n > 0; n--)
        {
            if (n==10)
            {
                printf("100 %% |");
            }
            else {printf("      |");}
            for(int i = 0; i < j; i++)
            {
                    if (ceil(cpu_value_arr[i]*10/100) == n) //how many bars does it fill?
                    {
                        printf(":");
                    }
                    else
                    {
                        printf(" ");
                    }
            }
            printf("\n");
        }
        printf("0 %%   ");
        for(int o = 0; o < samp+1; o++)
        {
            printf("_");
        }
        printf("\n");
        printf("\n");

        //CORES
        printf("Number of Cores : %d @ %.2f GHz\n",numofcores,(double)(maxfreq/1e6));
        while(numofcores > 0)
        {
            if (numofcores%4==0)
            {
                printf("\n");
            }

            if (numofcores >= 4)
            {
                printf("+--+ +--+ +--+ +--+\n");
                printf("|  | |  | |  | |  |\n");
                printf("+--+ +--+ +--+ +--+\n");
                numofcores -= 4;
            }
            if (numofcores == 3)
            {
                printf("+--+ +--+ +--+\n");
                printf("|  | |  | |  |\n");
                printf("+--+ +--+ +--+\n");
                numofcores -= 3;
            }
            if (numofcores == 2)
            {
                printf("+--+ +--+\n");
                printf("|  | |  |\n");
                printf("+--+ +--+\n");
                numofcores -= 2;
            }
            if (numofcores == 1) //bad code, can be simplified
            {
                printf("+--+\n");
                printf("|  |\n");
                printf("+--+\n");
                numofcores -= 1;
            }
        }
        printf("\n");
}
  


int main(){

    int samp = 20; //needs to be contin.
    int delay = 500000; 

    int numofcores = 0;
    int maxfreq = 0;

    long int prev_sum= 0, prev_idle = 0; //to calculate CPU by difference

    long int* memo_util_arr = malloc((samp+1) * sizeof(long int)); //to record values by the sample size
    double* cpu_value_arr = malloc((samp+1) * sizeof(double)); 

    if (memo_util_arr == NULL || cpu_value_arr == NULL) {
        printf("Memory allocation failed");
        exit(1);
    }

    char buffer5[32];
    char line[256];

    FILE* cpu_info = fopen("/proc/cpuinfo","r");
    FILE* maxfreqfile = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq","r");

    if (cpu_info == NULL) { //file check
        printf("/proc/cpuinfo is empty or couldn't be opened.\n");
    }

    if (maxfreqfile == NULL) {
        printf("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq is empty or couldn't be opened.\n");
    }
    
    fgets(buffer5, 32, maxfreqfile);
    maxfreq = atoi(buffer5); //finds maximum frequency

    //calculates num of cores
    for(int i = 0; i < 11; i++)
    {
        fgets(line, 256, cpu_info);
    }
    numofcores = atoi(line+11);

    for(int j = 0; j < samp; j++)
    {
        FILE* memo_info = fopen("/proc/meminfo","r");
        FILE* stat_info = fopen("/proc/stat","r");

        if (memo_info == NULL) {
            printf("/proc/meminfo is empty or couldn't be opened.\n");
        }

        if (stat_info == NULL) {
            printf("/proc/stat is empty or couldn't be opened.\n");
        }
        
        char buffer[32];
        char buffer4[32];
        char dummy[50];
        char buffer2[256];
        
        //find used memo (line3 - line1)
        char* overall_memo = fgets(buffer,32,memo_info);

        fgets(dummy,32,memo_info);
        char* used_memo = fgets(buffer4, 32, memo_info);

        long int used_value = 0;
        long int overall_value = 0;

        sscanf(overall_memo,"%s %ld %s",&dummy[0],&overall_value,&dummy[1]);
        sscanf(used_memo,"%s %ld %s",&dummy[2],&used_value,&dummy[3]);

        long int memo_util = overall_value - used_value; //ceil or floor can both be used
        memo_util_arr[j] = memo_util;

        //"CPU"
        //cpu util:(sum - idle / sum) x 100 is the formula.

        char* cpu_calc = fgets(buffer2, 256, stat_info);

        long int sum = 0;
        double cpu_value = 0;
        long int idle = 0;
        long int v1,v2,v3,v4,v5,v6,v7,v8,v9,v10;
        char bleh[32];

        sscanf(cpu_calc, "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", bleh, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10); //calculation for the cpu
        sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        idle = v4 + v5;

        long int sum_diff = sum - prev_sum;
        long int idle_diff = idle - prev_idle;

        //usleep(100000); //more sleep can added if wanted

        if (sum_diff != 0.0) 
        {
            cpu_value = ((sum_diff - idle_diff)/(double)sum_diff)*100.0; //divide by 0 error can happen
        }
        else
        {
            cpu_value = 0.0;
        }

        cpu_value_arr[j] = cpu_value;

        prev_sum = sum;
        prev_idle = idle;

        //"Cores"
        //siblings in the proc/cpuinfo
        // cpu_info is the file <- /proc/stat

        graph(samp, delay, numofcores, memo_util_arr, overall_value, maxfreq, cpu_value_arr, j);

        usleep(delay); //delay

        fclose(memo_info);
        fclose(stat_info);
    }
    
    fclose(cpu_info);
    fclose(maxfreqfile);

    free(memo_util_arr);
    free(cpu_value_arr);

    return 0;
}