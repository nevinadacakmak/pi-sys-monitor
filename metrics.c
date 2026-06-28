#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

void calculate_metrics(double delay, char *out, size_t out_size){

    int numofcores = 0;
    int maxfreq = 0;

    double memo_util_val = 0; //to record values by the sample size
    double cpu_value = 0; 

    char buffer[32]={0};
    char line[256];
    char dummy[128];

    FILE* cpu_info = fopen("/proc/cpuinfo","r");
    FILE* maxfreqfile = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq","r");

    if (cpu_info == NULL) { //file check
        printf("/proc/cpuinfo is empty or couldn't be opened.\n");
    }

    if (maxfreqfile == NULL) {
        printf("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq is empty or couldn't be opened.\n");
    }
    
    fgets(buffer, sizeof(buffer), maxfreqfile);
    //finds maximum frequency
    maxfreq = atoi(buffer);

    //calculates temperature

    //need to do "cat /sys/class/thermal/thermal_zone0/type"
    FILE* temp_file = fopen("/sys/class/thermal/thermal_zone0/temp","r");
    if (temp_file == NULL) {
        printf("/proc/meminfo is empty or couldn't be opened.\n");
    }
    char temp_bf[32]={0};
    fgets(temp_bf,sizeof(temp_bf),temp_file);
    double temp_f = atof(temp_bf)/1000.0;
    fclose(temp_file);

    //checks if mc server is up
    char mc_server[64] = "unknown";
    FILE* mc_server_pipe = popen("docker ps --filter name=skyfactory --format '{{.Status}}' 2>/dev/null","r");
    if (mc_server_pipe == NULL){
        printf("popen failed\n");
    }

    if (mc_server_pipe != NULL) {
        if (fgets(mc_server, sizeof(mc_server), mc_server_pipe) != NULL) {
            mc_server[strcspn(mc_server, "\r\n")] = '\0';
        }
        pclose(mc_server_pipe);
    } 
    else {
        perror("popen failed");
    }

    if (mc_server[0] == '\0') {
        strcpy(mc_server, "stopped");
    }

    //checks the pi uptime
    FILE* uptime = fopen("/proc/uptime","r");
    if (uptime == NULL){
        printf("/proc/uptime is empty or couldn't be opened\n");
    }
    char dummy0[64]={0};
    double uptime_f;
    fscanf(uptime,"%lf %s",&uptime_f, &dummy0[0]);

    fclose(uptime);

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
    
    //find used memo (line3 - line1)
    char* overall_memo = fgets(buffer2,32,memo_info);

    fgets(dummy,32,memo_info);
    char* used_memo = fgets(buffer3, 32, memo_info);

    double used_value = 0;
    double overall_value = 0;

    sscanf(overall_memo,"%s %lf %s",&dummy[0],&overall_value,&dummy[1]);
    sscanf(used_memo,"%s %lf %s",&dummy[2],&used_value,&dummy[3]);

    memo_util_val = overall_value - used_value; //ceil or floor can both be used

    //"CPU"
    //cpu util:(sum - idle / sum) x 100 is the formula.

    char* cpu_calc = fgets(buffer2, sizeof(buffer2), stat_info);

    double sum1 = 0;
    double sum2 = 0;
    double idle1 = 0;
    double idle2 = 0;
    double v1,v2,v3,v4,v5,v6,v7,v8,v9,v10;
    char bleh[32];

    sscanf(cpu_calc, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", bleh, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10); //calculation for the cpu
    sum1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    idle1 = v4 + v5;

    sleep(delay); //for good cpu calculation

    rewind(stat_info);

    cpu_calc = fgets(buffer2, sizeof(buffer2), stat_info);
    sscanf(cpu_calc, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", bleh, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10); //calculation for the cpu
    sum2 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    idle2 = v4 + v5;

    double sum_diff = sum2 - sum1;
    double idle_diff = idle2 - idle1;

    if (sum_diff != 0.0) 
    {
        cpu_value = ((sum_diff - idle_diff)/(double)sum_diff)*100.0; //divide by 0 error can happen
    }
    else
    {
        cpu_value = 0.0;
    }

    //"Cores"
    //siblings in the proc/cpuinfo
    // cpu_info is the file <- /proc/stat

    snprintf(out, out_size,
        "{\"delay\":%.2f,\"numOfCores\":%d,\"memoUtil\":%lf,"
        "\"overallValue\":%lf,\"maxFreq\":%d,\"cpuValue\":%lf,"
        "\"temperature\":%.2f, \"upTime\":%lf, \"McServer\":\"%s\"}",
        delay, numofcores, memo_util_val, overall_value,
        maxfreq, cpu_value, temp_f, uptime_f, mc_server); 
    
    sleep(delay); //delay

    fclose(memo_info);
    fclose(stat_info);

    
    fclose(cpu_info);
    fclose(maxfreqfile);
}