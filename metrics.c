#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

void calculate_metrics(double delay, char *out, size_t out_size){

    int numofcores = 0;
    int maxfreq = 0;

    long int prev_sum= 0, prev_idle = 0; //to calculate CPU by difference

    long int memo_util_val = 0; //to record values by the sample size
    long int cpu_value = 0; 

    char buffer[32];
    char line[256];

    FILE* cpu_info = fopen("/proc/cpuinfo","r");
    FILE* maxfreqfile = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq","r");

    if (cpu_info == NULL) { //file check
        printf("/proc/cpuinfo is empty or couldn't be opened.\n");
    }

    if (maxfreqfile == NULL) {
        printf("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq is empty or couldn't be opened.\n");
    }
    
    fgets(buffer, 32, maxfreqfile);
    //finds maximum frequency
    maxfreq = atoi(buffer);

    //calculates temperature

    //need to do "cat /sys/class/thermal/thermal_zone0/type"
    FILE* temp_file = fopen("/sys/class/thermal/thermal_zone0/temp","r");
    if (temp_file == NULL) {
        printf("/proc/meminfo is empty or couldn't be opened.\n");
    }
    char temp_bf[32];
    char* temperature = fgets(temp_bf,32,temp_file);
    double temp_f = atof(temperature)/1000;

    //checks if mc server is up
    //TODO: implmement later

    //calculates num of cores
    for(int i = 0; i < 11; i++)
    {
        fgets(line, 256, cpu_info);
    }
    numofcores = atoi(line+11);


    FILE* memo_info = fopen("/proc/meminfo","r");
    FILE* stat_info = fopen("/proc/stat","r");

    if (memo_info == NULL) {
        printf("/proc/meminfo is empty or couldn't be opened.\n");
    }

    if (stat_info == NULL) {
        printf("/proc/stat is empty or couldn't be opened.\n");
    }
    
    char buffer2[256];
    char buffer3[32];
    char dummy[50];
    
    //find used memo (line3 - line1)
    char* overall_memo = fgets(buffer2,32,memo_info);

    fgets(dummy,32,memo_info);
    char* used_memo = fgets(buffer3, 32, memo_info);

    long int used_value = 0;
    long int overall_value = 0;

    sscanf(overall_memo,"%s %ld %s",&dummy[0],&overall_value,&dummy[1]);
    sscanf(used_memo,"%s %ld %s",&dummy[2],&used_value,&dummy[3]);

    memo_util_val = overall_value - used_value; //ceil or floor can both be used

    //"CPU"
    //cpu util:(sum - idle / sum) x 100 is the formula.

    char* cpu_calc = fgets(buffer2, 256, stat_info);

    long int sum = 0;
    long int idle = 0;
    long int v1,v2,v3,v4,v5,v6,v7,v8,v9,v10;
    char bleh[32];

    sscanf(cpu_calc, "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", bleh, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10); //calculation for the cpu
    sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    idle = v4 + v5;

    long int sum_diff = sum - prev_sum;
    long int idle_diff = idle - prev_idle;

    //sleep(delay); //more sleep can be added if wanted

    if (sum_diff != 0.0) 
    {
        cpu_value = ((sum_diff - idle_diff)/(double)sum_diff)*100.0; //divide by 0 error can happen
    }
    else
    {
        cpu_value = 0.0;
    }

    prev_sum = sum;
    prev_idle = idle;

    //"Cores"
    //siblings in the proc/cpuinfo
    // cpu_info is the file <- /proc/stat

    snprintf(out, out_size, "delay=%lf, numofcores=%d, memo_util_arr=%ld, overall_value=%ld, maxfreq=%d, cpu_value=%ld, temperature=%lf\n",
        delay, numofcores, memo_util_val, overall_value, maxfreq, cpu_value, temp_f);
    //memo util arr ld and cpu value arr 
    
    sleep(delay); //delay

    fclose(memo_info);
    fclose(stat_info);

    
    fclose(cpu_info);
    fclose(maxfreqfile);
}