#include <stdio.h>
#include <stdlib.h>
#include <string.h> //to get strcspn for removing newline
#include <unistd.h> //sleep
#include <signal.h> //sigint ctrl+c
//char *readFile(char *file);
//int getWatts();

// https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
static volatile int keepRunning = 1;
void intHandler(int dummy) {
    keepRunning = 0;
}

int main()
{
    signal(SIGINT, intHandler);
    
    // http://rosettacode.org/wiki/Terminal_control/Preserve_screen#C
    printf("\033[?1049h\033[H");
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
    char *removeNewline(char *string)
    {
        // https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
        string[strcspn(string, "\r\n")] = 0; // works for LF, CR, CRLF, LFCR
        return string;
    }
    /* WATTS */
    int getWatts() 
    {
        int watt = atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/power1_average"));
        return watt/1000000;
    }
    int getWattCap() 
    {
        int watt = atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/power1_cap"));
        return watt/1000000;
    }
    
    /* TEMP */
    int getTemp() 
    {
        int temp = atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/temp1_input"));
        return temp/1000;
    }
    int getTempCrit() 
    {
        int temp = atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/temp1_crit"));
        return temp/1000;
    }
    
    /* FAN */
    int getFanRPM() 
    {
        return atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/fan1_input"));
    }  
    int getFanPWM() 
    {
        return atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/pwm1"));
    }
    int getFanPWMMAX() 
    {
        return atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/pwm1_max"));
    }
    
    /* VOLTAGE */
    char *getVoltageType() 
    {
        return removeNewline(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/in0_label"));
    }
    int getVoltage() 
    {
        return atoi(readFile("/sys/class/drm/card0/device/hwmon/hwmon1/in0_input"));
    }
    
    /* MEMORY */
    char *getMemoryStates() 
    {
        return readFile("/sys/class/drm/card0/device/pp_dpm_mclk");
    }
    
    while(keepRunning) {
        // TODO https://www.eecs.wsu.edu/~cs150/reading/printf.htm
        printf("\033[2J");
        printf("\033[0;0H");
        printf("%s %imV\n", getVoltageType(), getVoltage());
        
        int watt = getWatts();
        int wattCap = getWattCap();
        int wattPercent = (watt*100)/(wattCap+watt);
        printf("POWER %iW (%i%% of %iW limit)\n", watt, wattPercent, wattCap);
        
        printf("Temperature %i\u2103 (critical: %i\u00B0C)\n", getTemp(), getTempCrit());
    
        int fanPWMMAX = getFanPWMMAX();
        int fanPWM = getFanPWM();
        int fanPercent = (fanPWM*100)/(fanPWMMAX+fanPWM);
        printf("FAN %irpm (%i%%)\n", getFanRPM(), fanPercent);
        
        fflush(stdout);
        sleep(1);
    }
    printf("\033[?1049l");
    return 0;
}
