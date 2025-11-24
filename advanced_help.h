#ifndef ADVANCED_HELP_H
#define ADVANCED_HELP_H




/////   INCLUDES   /////
#include <Windows.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wchar.h>
#include <locale.h>





/////   DEFINES   /////

#define MAX_NODE_LEVEL 16
#define NODE_LEVEL_CHAR '\t'
#define NODE_START_CHAR '\0'	// If null (= '\0'), then every new line will be interpreted as a new node (e.g. a new section or a new param), which means nodes will be one-liners

#define WTEXT_IMPL(name)    L##name
#define WTEXT(name)         WTEXT_IMPL(name)

#define ADVANCED_HELP_UNINITIALIZED_ERROR "ADVANCED HELP ERROR: help not initialized.\n"
#define ADVANCED_HELP_FORMAT_ERROR "ADVANCED HELP ERROR: help is incorrectly formatted.\n"
#define ADVANCED_HELP_NOMEM_ERROR "ADVANCED HELP ERROR: not enough memory to show the help.\n"
#define ADVANCED_HELP_KEYWORD_NOT_FOUND_INFO "ADVANCED HELP INFO: the keyword entered could not be found.\n"

#define DEFAULT_HELP_FILEPATH "help.txt"



/////   FUNCTION DEFINITIONS   /////

char* getAdvancedHelpForKeyword(_In_ const char* keyword, _In_ void* help_ptr);
WCHAR* getAdvancedHelpForKeywordW(_In_ const WCHAR* keyword, _In_ void* help_ptr);

int initAdvancedHelp(_In_ const char* help_filename, _Inout_ void** help_ptr);
int initAdvancedHelpW(_In_ const WCHAR* help_filename, _Inout_ void** help_ptr);

void freeAdvancedHelp(_In_ void** help_ptr);
void freeAdvancedHelpW(_In_ void** help_ptr);

int getTextFromFile(_In_ const char* text_filename, _Inout_ char** text_ptr);
int getTextFromFileW(_In_ const WCHAR* text_filename, _Inout_ WCHAR** text_ptr);


int strAppendRealloc(_Inout_ char** dest, _In_ const char* src);
int wcsAppendRealloc(_Inout_ WCHAR** dest, _In_ const WCHAR* src);

void saveCurrentLocaleAndSetUTF8();
void restorePreviousLocale();


#endif // ADVANCED_HELP_H
