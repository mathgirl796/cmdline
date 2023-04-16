#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_ARG_NUM 32 // max num for args with a name

typedef enum ArgType {INT, FLOAT, STRING, ARG_BOOL} ArgType;
static char *string_arg_type[4] = {"int", "float", "string", "bool"};

typedef struct Arg {
    char *long_name;
    char short_name;
    ArgType arg_type;
    char *description;

    int exist; // if an arg is successfully parsed, set its `exist` = 1, default 0
    char *default_value; // default NULL, if an arg is not required, it is a pointer to string
    char *value; // default NULL, if a BOOL arg is set by user, value becomes not NULL
} Arg;

typedef struct Parser {
    char *program_name;
    Arg args[MAX_ARG_NUM];
    int arg_num;
    char **rest;
    int rest_num;
    int rest_capacity;
} Parser;

static void __print_arg(Arg arg) {
    if (arg.default_value) fprintf(stderr, "\t--%s,-%c\t%s [=%s]\t%s\n", arg.long_name, arg.short_name, string_arg_type[arg.arg_type], arg.default_value, arg.description);
    else fprintf(stderr, "\t--%s,-%c\t%s\t%s\n", arg.long_name, arg.short_name, string_arg_type[arg.arg_type], arg.description);

}


// 2 for "--" key, 1 for "-" key, 0 for value 
static int __arg_type(char *arg) {
    if (strncmp(arg, "--", 2) == 0) return 2;
    else if (strncmp(arg, "-", 1) == 0) return 1;
    else return 0;
}

static int __find_name(Parser *p, char *long_name, char short_name) { // find if long_name or short_name the same as arg in arg list 
    for (int i = 0; i < p->arg_num; ++i) {
        Arg *arg = &(p->args[i]);
        if (strcmp(arg->long_name, long_name) == 0 || arg->short_name == short_name) return i;
    }
    return -1;
}

static Parser *get_parser() {
    Parser *parser = malloc(sizeof(parser));
    memset(parser, 0, sizeof(parser));
    return parser;
}

// TODO: validate data type
// return 0 on success, -1 on error
int __check_value_type(ArgType arg_type, char* default_value) {

}

// if arg is required, then set default_value to NULL
// a BOOL arg do not have default value, please set it to NULL
static void add_argument(Parser *p, char *long_name, char short_name, ArgType arg_type, char* default_value, char* description) {
    Arg *a = &(p->args[p->arg_num]);
    a->long_name = long_name;
    a->short_name = short_name;
    a->arg_type = arg_type;
    a->default_value = default_value; 
    a->description = description;
    int id;
    if ((id = __find_name(p, long_name, short_name)) != -1) { // validate name conflicts
        fprintf(stderr, "Conflict between these args: \n");
        __print_arg(p->args[id]);
        __print_arg(*a);
        exit(1);
    }
    else if (__check_value_type(arg_type, default_value) != 0) { // validate if default_value conflicts with arg_type
        fprintf(stderr, "This arg`s default_value is conflict with its type: \n");
        __print_arg(*a);
    }
    else p->arg_num += 1;
}

static void arg_parse(Parser *p, int argc, char **argv) {
    p->program_name = argv[0];
    int id;
    for (int i = 1; i < argc; ++i) {
        if (__arg_type(argv[i]) != 0) { 
            // key
            id = __find_name(p, argv[i] + 2, argv[i][1]);
            if (id == -1) { 
                // key not found
                fprintf(stderr, "Unknown key: `%s`\n", argv[i]);
                exit(1);
            }
            else if (p->args[id].arg_type == ARG_BOOL) { 
                // key is bool
                p->args[id].exist = 1;
                p->args[id].value = "true";
            }
            else if (i + 1 < argc && __arg_type(argv[i + 1]) == 0) {
                // key with a value
                p->args[id].exist = 1;
                p->args[id].value = argv[i + 1];
                i += 1;
            }
            else {
                // key without a value
                if (p->args[id].default_value == NULL) {
                    // no default value
                    fprintf(stderr, "Key `%s` needs a value\n", argv[i]);
                    exit(1);
                }
                else {
                    // have default value
                    p->args[id].exist = 1;
                    p->args[id].value = p->args[id].default_value;
                }
            }
        } 
        else {
            // a rest value
            if (p->rest_capacity == 0) {
                // alloc memory for rest values
                p->rest_capacity = 1;
                p->rest = malloc(p->rest_capacity * sizeof(char *));
            }
            else if (p->rest_num == p->rest_capacity) {
                // alloc more memory for rest values
                p->rest_capacity *= 2;
                p->rest = realloc(p->rest, p->rest_capacity * sizeof(char *));
            }
            else {
                p->rest[p->rest_num] = argv[i];
                p->rest_num += 1;
            }
        }
    }
    // check unappeared keys
    int all_required_has_value = 1;
    for (int i = 0; i < p->arg_num; ++i) {
        Arg *arg = &(p->args[i]);
        if (arg->default_value != NULL && arg->value == NULL) {
            // has default value
            arg->value = arg->default_value;
        }
        else if (arg->default_value == NULL && arg->value == NULL && arg->arg_type != ARG_BOOL) {
            // no default value
            if (all_required_has_value) {
                all_required_has_value = 0;
                fprintf(stderr, "There keys are requied and need a value:\n");
            }
            __print_arg(*arg);
        }
    }
    if (! all_required_has_value) exit(1);
}

//TODO: print usage

//TODO: add text to foot of usage