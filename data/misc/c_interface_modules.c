#include<assert.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>

#include<threads.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define ICE_CPU_IMPL
#include "ice_cpu.h"


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINDOWS
#elif defined(__unix__) || defined(__unix)
    #define UNIX
#endif

#ifdef UNIX
    #define PATH_SEP "/"
    #define DYLIB_EXTENSION ".so"
    #include<dirent.h>
    #include<dlfcn.h>
#endif
#ifdef WINDOWS
    #define PATH_SEP "\\"
    #define DYLIB_EXTENSION ".dll"
    #include<windows.h>
#endif


#ifdef DEBUG
    #define malloc debug_memory_allocate 
    #define realloc debug_memory_reallocate
    #define free debug_memory_free
#endif /* ifdef DEBUG */

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

static_assert(sizeof(u8) == 1, "u8 size is not 1 byte");
static_assert(sizeof(u16) == 2, "u16 size is not 2 bytes");
static_assert(sizeof(u32) == 4, "u32 size is not 4 bytes");
static_assert(sizeof(u64) == 8, "u64 size is not 8 bytes");

static_assert(sizeof(s8) == 1, "s8 size is not 1 byte");
static_assert(sizeof(s16) == 2, "s16 size is not 2 bytes");
static_assert(sizeof(s32) == 4, "s32 size is not 4 bytes");
static_assert(sizeof(s64) == 8, "s64 size is not 8 bytes");

typedef void SharedObject;
typedef void FunctionPointer;

typedef enum{
    INFO, WARNING, ERROR, DEBUG
}LogCategory;


// _will_ be implemented, when i need them
void *debug_memory_allocate(size_t size);
void *debug_memory_reallocate(void *ptr, size_t size);
void  debug_memory_free(size_t size);

char           **platform_enumerate_directory(char* directory_path, bool directories);  // returns dynamic array of strings
SharedObject    *platform_library_load(char *path);
FunctionPointer *platform_library_load_symbol(SharedObject *object, char* name);
void             platform_library_unload(SharedObject *object);

char *string_join(char *restrict string_a, char *restrict string_b);
char *string_duplicate(char *string);
char *string_path_to_file(char *restrict directory, char *restrict filename);

void core_log(LogCategory category, char *restrict format, ...);



int main(int argc, char** argv){    
    char *cwd = argv[0];

    //get executable directory by truncating argv[0]
    strrchr(cwd, *PATH_SEP)[1] = '\0';

    ice_cpu_info cpu_info;
    ice_cpu_get_info(&cpu_info);

    printf("CPU: %s\n%i cores\n", cpu_info.name, cpu_info.cores);

    char *mod_directory = string_join(cwd, "mods");
    char **files = platform_enumerate_directory(mod_directory, false);   
    
    for(u32 i = 0; i < arrlen(files); i++){
        char *extension = strrchr(files[i], '.'); 
        if(strcmp(extension, DYLIB_EXTENSION) != 0){
            continue;
        }

        printf("loading %s\n", files[i]);
        
        char *mod_path = string_path_to_file(mod_directory, files[i]);
        SharedObject *mod_handle = platform_library_load(mod_path);

        free(mod_path);

        if(mod_handle == NULL){
            printf("failed to load %s\n", files[i]);
            continue;
        }
        
        void(*start)() = platform_library_load_symbol(mod_handle, "start");
        
        start();
        
        platform_library_unload(mod_handle);
    }
    for(u32 i = 0; i < arrlen(files); i++){
        free(files[i]);
    }
    arrfree(files);
    free(mod_directory);

    return 0;
}


char **platform_enumerate_directory(char* directory_path, bool directories){
    if(directory_path == NULL){
        return NULL;
    }
    char **file_array = NULL;
    
    //dirent.h
    #ifdef UNIX
        DIR *directory = opendir(directory_path); 
        if(directory == NULL){
            return NULL;
        }
        struct dirent *entry;
            
        while((entry = readdir(directory))){
            if(directories && entry->d_type != 4){
                continue;
            } 
            if(!directories && entry->d_type != 8){
                continue;
            }
            char *filename = malloc(strlen(entry->d_name) + 1);
            strcpy(filename, entry->d_name);
            arrput(file_array, filename);
        }
        closedir(directory);
    #endif /* ifdef UNIX */

    return file_array;
}

SharedObject *platform_library_load(char *path){
    SharedObject *handle;
    #ifdef UNIX
        handle = dlopen(path, RTLD_LAZY);
    #endif /* ifdef UNIX */

    #ifdef WINDOWS
    
    #endif /* ifdef WINDOWS */

    return handle;
}

FunctionPointer *platform_library_load_symbol(SharedObject *object, char* name){

    FunctionPointer *pointer;
    #ifdef UNIX
        pointer = dlsym(object, name);
    #endif /* ifdef UNIX */
    #ifdef WINDOWS
        
    #endif /* ifdef WINDOWS */

    return pointer;
}
void platform_library_unload(SharedObject *object){
    #ifdef UNIX
        dlclose(object);
    #endif /* ifdef UNIX */
    #ifdef WINDOWS
    
    #endif /* ifdef WINDOWS */
}

char *string_join(char *restrict string_a, char *restrict string_b){
    char *joined_string = malloc(strlen(string_a) + strlen(string_b) + 1);
    strcpy(joined_string, string_a);
    strcat(joined_string, string_b);
    return joined_string;
}

char *string_copy(char *string){
    char *duplicated = malloc(strlen(string));
    strcpy(duplicated, string);
    return duplicated;
}

char *string_path_to_file(char *restrict directory, char *restrict filename){
    char *full_path = malloc(strlen(directory) + strlen(filename) + 1);
    strcpy(full_path, directory);
    strcat(full_path, PATH_SEP);
    strcat(full_path, filename);
    return full_path;
}
