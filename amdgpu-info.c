#include <stdio.h>
#include <stdlib.h>
#include <string.h> //to get strcspn for removing newline
#include <unistd.h> //sleep
#include <signal.h> //sigint ctrl+c
//#include <pthread.h> // TODO POSIX Thread 

#define UPDATETIME 1
#define SYSDIR "/sys/class/drm/card0/device/hwmon/hwmon1/"
#define SENSORFILE(base, file) (base file)

// https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
static volatile int keepRunning = 1;
void intHandler(int dummy) {
    keepRunning = 0;
}

// https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
char *removeNewline(char *string)
{
	string[strcspn(string, "\r\n")] = 0; // works for LF, CR, CRLF, LFCR
	return string;
}

char *readFile(const char *file)
    {
    //https://stackoverflow.com/a/14424800
    
    int length=0;
    FILE *fp;
    fp = fopen(file, "r");
    if(!fp)
    {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    length=ftell(fp);
    rewind(fp);
    
    char *res = (char *) malloc(length);
    fread(res, 1, length, fp);
    //fgets(tmp, sizeof(tmp), fp);
    fclose(fp);

    /*if (length > 0 && res[length-1] == '\n') {
        res[--length] = '\0';
    }*/

    return res;
}

/* WATTS */
int getWatts() 
{
	int watt = atoi(readFile(SENSORFILE(SYSDIR, "power1_average")));
    return watt/1000000;
}
int getWattCap() 
{
    int watt = atoi(readFile(SENSORFILE(SYSDIR, "power1_cap")));
    return watt/1000000;
}

/* TEMP */
int getTemp() 
{
    int temp = atoi(readFile(SENSORFILE(SYSDIR, "temp1_input")));
    return temp/1000;
}
int getTempCrit() 
{
    int temp = atoi(readFile(SENSORFILE(SYSDIR, "temp1_crit")));
    return temp/1000000;
}

/* FAN */
int getFanRPM() 
{
    return atoi(readFile(SENSORFILE(SYSDIR, "fan1_input")));
}  
int getFanPWM() 
{
    return atoi(readFile(SENSORFILE(SYSDIR, "pwm1")));
}
int getFanPWMMAX() 
{
    return atoi(readFile(SENSORFILE(SYSDIR, "pwm1_max")));
}

/* VOLTAGE */
char *getVoltageType() 
{
    return removeNewline(readFile(SENSORFILE(SYSDIR, "in0_label")));
}
int getVoltage() 
{
    return atoi(readFile(SENSORFILE(SYSDIR, "in0_input")));
}

// TODO memory and clock
/* MEMORY */
char *getMemoryStates() 
{
    return readFile("/sys/class/drm/card0/device/pp_dpm_mclk");
}

// /sys/class/drm/card0/device/pp_dpm_sclk
// 0: 300Mhz *
// 1: 483Mhz 
// 2....
	
int main()
{
    signal(SIGINT, intHandler);
    // http://rosettacode.org/wiki/Terminal_control/Preserve_screen#C
    printf("\033[?1049h\033[H");

    while(keepRunning) {
        // TODO Flags table https://www.eecs.wsu.edu/~cs150/reading/printf.htm
        // Move to top of screen again
        printf("\033[2J");
        printf("\033[0;0H");
        
        int fanPWMMAX = getFanPWMMAX();
        int fanPWM = getFanPWM();
        int fanPercent = (fanPWM*100)/(fanPWMMAX+fanPWM);
        
        int watt = getWatts();
        int wattCap = getWattCap();
        int wattPercent = (watt*100)/(wattCap+watt);
        
        // 11 columns space width
        printf("%-11.11s %imV\n", getVoltageType(), getVoltage());
        
        printf("%-11.11s %iW (%i%% of %iW\n", "POWER", watt, wattPercent, wattCap);
        
        printf("%-11.11s %i\u2103  (critical: %i\u00B0C)\n", "Temperature", getTemp(), getTempCrit());
        
        printf("%-11.11s %irpm (%i%% PWM:%i/%i)\n", "Fan", getFanRPM(), fanPercent, fanPWM, fanPWMMAX);
        
        fflush(stdout);
        sleep(UPDATETIME);
    }
    printf("\033[?1049l");// Clear alternative buffer
    return 0;
}
