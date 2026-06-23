/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <pspinit.h>
#include <pspmodulemgr.h>
#include <pspiofilemgr.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>

#include "rebootex.h"
#include "config.h"
#include "missingfunc.h"


#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

enum {
    RUNLEVEL_UNKNOWN,
    RUNLEVEL_VSH,
    RUNLEVEL_UMD,
    RUNLEVEL_POPS,
    RUNLEVEL_HOMEBREW,
};



static int cur_runlevel = RUNLEVEL_UNKNOWN;


static int isVshRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype >= 0x200) cur_runlevel = RUNLEVEL_VSH;
    }
    return cur_runlevel == RUNLEVEL_VSH;
}

static int isPopsRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype == 0x144 || apitype == 0x155) cur_runlevel = RUNLEVEL_POPS;
    }
    return cur_runlevel == RUNLEVEL_POPS;
}

static int isUmdRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype == 0x120 || apitype == 0x130 || apitype == 0x160
                || (apitype >= 0x123 && apitype <= 0x126)
                || (apitype >= 0x110 && apitype <= 0x115))
            cur_runlevel = RUNLEVEL_UMD;
    }
    return cur_runlevel == RUNLEVEL_UMD;
}

static int isHomebrewRunlevel(){
    if (!cur_runlevel){
        // Fetch Apitype
        int apitype = sceKernelInitApitype();
        if (apitype == 0x141 || apitype == 0x152) cur_runlevel = RUNLEVEL_HOMEBREW;
    }
    return cur_runlevel == RUNLEVEL_HOMEBREW;
}

static int isPath(char* runlevel){
    return (
        strcasecmp(runlevel, sceKernelInitFileName())==0 ||
        strcasecmp(runlevel, sctrlSEGetUmdFile())==0
    );
}

static int isGameId(char* runlevel){
    if (rebootex_config.game_id[0] == 0) return 0;
    char gameid[10]; memset(gameid, 0, sizeof(gameid));
    memcpy(gameid, rebootex_config.game_id, 9);
    lowerString(gameid, gameid, strlen(gameid)+1);
    return (strstr(runlevel, gameid) != NULL);
}

// Runlevel Check
static int matchingRunlevel(char * runlevel)
{
    lowerString(runlevel, runlevel, strlen(runlevel)+1);

    char* cfw_type = strstr(runlevel, "cfw=");
    if (cfw_type){ // plugin is for specific CFW only
        if (strncmp(cfw_type+4, "ark", 3) != 0){
            return 0; // not for ark, treat as disabled
        }
    }

    if (strcasecmp(runlevel, "all") == 0 || strcasecmp(runlevel, "always") == 0) return 1; // always on

    if (strchr(runlevel, '/')){
        // it's a path
        return isPath(runlevel);
    }

    if(isVshRunlevel()){
        return (strstr(runlevel, "vsh") != NULL || strstr(runlevel, "xmb") != NULL);
    }

    if(isPopsRunlevel()){
        // check if plugin loads on specific game
        if (isGameId(runlevel)) return 1;
        // check keywords
        return (strstr(runlevel, "pops") != NULL || strstr(runlevel, "ps1") != NULL || strstr(runlevel, "psx") != NULL); // PS1 games only
    }
    
    if(isHomebrewRunlevel()) {
        if (strstr(runlevel, "launcher") != NULL){
        	// check if running custom launcher
        	if (sctrlArkIsLauncher()) return 1;
        }
        if (strstr(runlevel, "app") != NULL || strstr(runlevel, "homebrew") != NULL || strstr(runlevel, "game") != NULL) return 1; // homebrews only
    }

    if(isUmdRunlevel()) {
        // check if plugin loads on specific game
        if (isGameId(runlevel)) return 1;
        // check keywords
        if(strstr(runlevel, "umd") != NULL || strstr(runlevel, "psp") != NULL || strstr(runlevel, "umdemu") != NULL || strstr(runlevel, "game") != NULL) return 1; // Retail games only
    }
    
    // Unsupported Runlevel (we don't touch those to keep stability up)
    return 0;
}

// Boolean String Parser
static int booleanOn(char * text)
{
    // Different Variations of "true"
    if (strcasecmp(text, "true") == 0 || strcasecmp(text, "on") == 0 ||
        strcmp(text, "1") == 0 || strcasecmp(text, "enabled") == 0)
            return 1;
    
    // Default to False
    return 0;
}

int readLine(char* source, char *str)
{
    u8 ch = 0;
    int n = 0;
    int i = 0;
    while(1)
    {
        if( (ch = source[i]) == 0){
            *str = 0;
            return n;
        }
        n++; i++;
        if(ch < 0x20)
        {
            *str = 0;
            return n;
        }
        else
        {
            *str++ = ch;
        }
    }
}

// Parse and Process Line
static void processLine(
    const char* parent,
    char* line,
    void (*enabler)(const char*),
    void (*disabler)(const char*)
){
    // Skip Comment Lines
    if(!enabler || line == NULL || strncmp(line, "//", 2) == 0 || line[0] == ';' || line[0] == '#')
        return;
    
    // String Token
    char * runlevel = line;
    char * path = NULL;
    char * enabled = NULL;
    
    // Original String Length
    unsigned int length = strlen(line);
    
    // Fetch String Token
    unsigned int i = 0; for(; i < length; i++)
    {
        // Got all required Token
        if(enabled != NULL)
        {
            // Handle Trailing Comments as Terminators
            if(strncmp(line + i, "//", 2) == 0 || line[i] == ';' || line[i] == '#')
            {
                // Terminate String
                line[i] = 0;
                
                // Stop Token Scan
                break;
            }
        }
        
        // Found Delimiter
        if(line[i] == LINE_TOKEN_DELIMITER)
        {
            // Terminate String
            line[i] = 0;
            
            // Path Start
            if(path == NULL) path = line + i + 1;
            
            // Enabled Start
            else if(enabled == NULL) enabled = line + i + 1;
            
            // Got all Data
            else break;
        }
    }
    
    // Unsufficient Plugin Information
    if(enabled == NULL) return;
    
    // Trim Whitespaces
    runlevel = strtrim(runlevel);
    path = strtrim(path);
    enabled = strtrim(enabled);

    // Matching Plugin Runlevel
    if(matchingRunlevel(runlevel))
    {
        char full_path[256];
        if (parent && strchr(path, ':') == NULL){ // relative path
            strcpy(full_path, parent);
            strcat(full_path, path);
        }
        else{ // already full path
            strcpy(full_path, path);
        }
        // Enabled Plugin
        if(booleanOn(enabled))
        {
            // Start Plugin
            enabler(full_path);
        }
        else{
            if (disabler) disabler(full_path);
        }
    }
}

// Load Plugins
int processConfigFile(
    const char* parent,
    const char* path,
    void (*enabler)(const char*),
    void (*disabler)(const char*)
){

    int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    
    // Opened Plugin Config
    if(fd >= 0)
    {

        // allocate buffer in user ram and read entire file
        int fsize = sceIoLseek(fd, 0, PSP_SEEK_END);
        sceIoLseek(fd, 0, PSP_SEEK_SET);

        SceUID memid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_Low, fsize+1, NULL);
        u8* buf = sceKernelGetBlockHeadAddr(memid);
        if (buf == NULL){
            sceIoClose(fd);
            return -1;
        }

        sceIoRead(fd, buf, fsize);
        sceIoClose(fd);
        buf[fsize] = 0;

        // Allocate Line Buffer
        char * line = (char *)oe_malloc(LINE_BUFFER_SIZE);
        
        // Buffer Allocation Success
        if(line != NULL)
        {
            // Read Lines
            int nread = 0;
            while ((nread=readLine((char*)buf, line)) > 0)
            {
                buf += nread;
                if (line[0] == 0) continue; // empty line
                // Process Line
                processLine(parent, strtrim(line), enabler, disabler);
            }
            
            // Free Buffer
            oe_free(line);
        }

        sceKernelFreePartitionMemory(memid);
        
        // Close Plugin Config
        return 0;
    }
    return -1;
}
