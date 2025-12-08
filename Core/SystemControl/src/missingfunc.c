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

#include <pspsdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * This file contains functions that Sony used to have in the Kernel,
 * but removed for whatever reason they had...
 *
 * To make life easier for developers, we reimplemented them and export
 * them via our NID Resolver.
 *
 * `strncat` on ansi-c/strsafe.c
 */

// Number Parser for Decimal Numbers
int strtol10(const char * str, int * res)
{
    // End Pointer
    char * endptr = NULL;

    // Forward Call (with Base 10)
    int result = strtol(str, &endptr, 10);

    // Return Result in Parameter
    if (res) *res = result;

    // Return Result
    return result;
}

// Search Needle in Haystack
size_t strcspn(const char * str1, const char * str2)
{
    // Iterate Symbols from Haystack
    unsigned int i = 0; for(; i < strlen(str1); i++)
       {
        // Iterate Symbols from Needle
        unsigned int j = 0; for(; j < strlen(str2); j++)
           {
            // Match found
            if(str1[i] == str2[j]) break;
        }
    }

    // Return Offset
    return i;
}

// Inverse Needle Search in Haystack
size_t strspn(const char * str1, const char * str2)
{
    // Iterate Symbols from Haystack
    unsigned int i = 0; for(; i < strlen(str1); i++)
       {
        // Iterate Symbols from Needle
        unsigned int j = 0; for(; j < strlen(str2); j++)
           {
            // Mismatch found
            if(str1[i] != str2[j]) break;
        }
    }

    // Return Offset
    return i;
}

char * strtok_r(char * s, const char * delim, char ** last)
{
    // Required Variables
    char * spanp = NULL;
    int c = 0;
    int sc = 0;
    char * tok = NULL;

    // Invalid Parameters
    if(s == NULL && (s = *last) == NULL) return NULL;

cont:
    // Move Pointer
    c = *s++;

    // Scan for Delimiter
    for(spanp = (char *)delim; (sc = *spanp++) != 0;)
       {
        if (c == sc)
            goto cont;
    }

    // End of String
    if(c == 0)
       {
        *last = NULL;
        return NULL;
    }

    // Update Token
    tok = s - 1;

    // Processing Loop
    for(;;)
       {
           // Fetch Symbol
        c = *s++;

        // Fetch Delimtier
        spanp = (char *)delim;

        // Process Text
        do
           {
               // Character Match
            if ((sc = *spanp++) == c)
               {
                   // End of String
                if (c == 0) s = NULL;

                // Terminate String
                else s[-1] = 0;

                // Update Pointer
                *last = s;

                // Return Token
                return tok;
            }
        } while (sc != 0);
    }
}

// String Token Function
char * strtok(char * s, const char * delim)
{
    // Last Token
    char* last = NULL;

    // Recursive Token Implementation
    return strtok_r(s, delim, &last);
}

void lowerString(char* orig, char* ret, int strSize){
    int i=0;
    while (*(orig+i) && i<strSize-1){
        *(ret+i) = *(orig+i);
        if (*(orig+i) >= 'A' && *(orig+i) <= 'Z')
            *(ret+i) += 0x20;
        i++;
    }
    *(ret+i) = 0;
}

int tolower(int s)
{
    if((s >= 'A') && (s <= 'Z'))
        s = 'a' + (s - 'A');

    return s;
}

unsigned int strnlen(const char * text, unsigned int maxlen)
{
    // String Length
    unsigned int length = 0;
    
    // Count Characters
    while (text[length] != 0 && length<maxlen) length++;
    
    // Return String Length
    return length;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;
    unsigned char c1, c2;

    if (p1 == p2 || n == 0)
        return 0;
    
    if (s1 == NULL || s2 == NULL){
        return (int)s1 - (int)s2;
    }

    do {
        c1 = tolower(*p1);
        c2 = tolower(*p2);

        if (--n == 0 || c1 == '\0')
        	break;

        ++p1;
        ++p2;
    } while (c1 == c2);

    return c1 - c2;
}

int strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, (size_t)-1);
}

size_t strncat_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count)
{
    size_t rest;

    if (!strDest || !strSource || numberOfElements == 0) {
        return 0;
    }

    rest = numberOfElements - strnlen(strDest, numberOfElements);

    if (rest == 0) {
        return 0;
    }

    rest--;
    strncat(strDest, strSource, rest < count ? rest : count);

    return strnlen(strDest, numberOfElements);
}

size_t strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count)
{
    if (!strDest || !strSource || numberOfElements == 0) {
        return 0;
    }

    strncpy(strDest, strSource, numberOfElements < count ?
            numberOfElements : count);
    strDest[numberOfElements - 1] = '\0';

    return strnlen(strDest, numberOfElements);
}
