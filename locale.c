/* vi: set sw=4 ts=4: */
/*
 *
 * Copyright (c) 2008  STMicroelectronics Ltd
 * Filippo Arcidiacono (filippo.arcidiacono@st.com)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * A 'locale' command implementation for uClibc.
 *
 */

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <langinfo.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <dirent.h>
#include <locale.h>
#include <stdbool.h>
#include "categories.h"


/* If set print the name of the category.  */
static bool show_category_name = 0;

/* If set print the name of the item.  */
static bool show_keyword_name = 0;

/* If set print the usage command.  */
static bool show_usage = 0;

/* Print names of all available locales.  */
static bool do_all = 0;

/* Print names of all available character maps.  */
static bool do_charmaps = 0;

static int remaining = 0;

static void usage(char *name)
{
    char *s;

    s = basename(name);
    fprintf(stderr,
            "Usage: %s [-a | -m] [FORMAT] name...\n\n"
            "\t-a, --all-locales\tWrite names of all available locales\n"
            "\t-m, --charmaps\tWrite names of available charmaps\n"
            "\nFORMAT:\n"
            "\t-c, --category-name\tWrite names of selected categories\n"
            "\t-k, --keyword-name\tWrite names of selected keywords\n"
            , s);
    free(s);
}

static int argp_parse(int argc, char *argv[])
{
    int c;
    char *progname;
    static const struct option long_options[] = {
        {"all-locales", no_argument, NULL, 'a'},
        {"charmaps", no_argument, NULL, 'm'},
        {"category-name", no_argument, NULL, 'c'},
        {"keyword-name", no_argument, NULL, 'k'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}};
    progname = *argv;
    while ((c = getopt_long(argc, argv, "amckh", long_options, NULL)) >= 0)

        switch (c) {
        case 'a':
            do_all = 1;
            break;
        case 'c':
            show_category_name = 1;
            break;
        case 'm':
            do_charmaps = 1;
            break;
        case 'k':
            show_keyword_name = 1;
            break;
        case 'h':
            show_usage = 1;
            break;
        case '?':
            fprintf(stderr, "Unknown option.\n");
            usage(progname);
            return 1;

        default:
            fprintf(stderr, "This should never happen!\n");
            return 1;
        }

    remaining = optind;
    return 0;
}

static void list_locale()
{
    const char *locpath = getenv("MUSL_LOCPATH");
    DIR *dir = opendir(locpath);
    struct dirent *pDir;
    printf("C");
    printf("C.UTF-8");
    while ((pDir = readdir(dir)) != NULL){
        printf("%s.%s\n",pDir->d_name,"UTF-8");
    }
}

static void list_charmaps()
{
    printf("ASCII\n");
    printf("UTF-8\n");
    return;
}

static void print_item (struct cat_item item)
{
    switch (item.type)
    {
    case CAT_TYPE_STRING:
        if (show_keyword_name)
            printf ("%s=\"", item.name);
        fputs (nl_langinfo (item.id) ? : "", stdout);
        if (show_keyword_name)
            putchar ('"');
        putchar ('\n');
        break;
    case CAT_TYPE_STRINGARRAY:
    {
        const char *val;
        if (show_keyword_name)
            printf ("%s=\"", item.name);

        for (int cnt = item.min; cnt <= item.max; ++cnt)
        {
            val = nl_langinfo (item.id);
            if (val != NULL)
                fputs (val, stdout);
            putchar (';');
        }
        if (show_keyword_name)
            putchar ('"');
        putchar ('\n');
    }
        break;
  }
}

/* Show the information request for NAME.  */
static void show_info(const char *name)
{
    for (size_t cat_no = 0; cat_no < LC_ALL; ++cat_no)
    {
        if (strcmp (name, get_name_for_cat(cat_no)) == 0)
        /* Print the whole category.  */
        {
            if (show_category_name != 0)
                puts (get_name_for_cat(cat_no));
            const struct cat_item* items = get_cat_for_locale_cat(cat_no);
            for(int j = 0; items[j].type != CAT_TYPE_END; j++)
                print_item(items[j]);
            return;
        }
        else
        {
            if (show_category_name != 0)
                puts (get_name_for_cat(cat_no));
            print_item(get_cat_item_for_name(name));
        }

    }
}

static void show_locale_vars()
{
    const char *lcall = getenv("LC_ALL") ? : "\0";
    const char *lang = getenv("LANG") ? : "";

    /* LANG has to be the first value.  */
    printf("LANG=%s\n", lang);
    for (size_t cat_no = 0; cat_no < LC_ALL; ++cat_no)
    {
        printf("%s=%s\n",get_name_for_cat(cat_no),lcall[0] != '\0' ? lcall
                                                                : lang[0] != '\0' ? lang
                                                                : "POSIX");
    }
    /* The last is the LC_ALL value.  */
    printf("LC_ALL=%s\n", lcall);
}

int main(int argc, char *argv[])
{
    /* Parse and process arguments.  */
    if (argp_parse(argc, argv))
        return 1;

    if (do_all) {
        list_locale();
        exit(EXIT_SUCCESS);
    }

    if (do_charmaps) {
        list_charmaps();
        exit(EXIT_SUCCESS);
    }

    if (show_usage) {
        usage(*argv);
        exit(EXIT_SUCCESS);
    }

    /* If no real argument is given we have to print the contents of the
       current locale definition variables. These are LANG and the LC_*.  */
    if (remaining == argc && show_category_name == 0
        && show_keyword_name == 0) {
        show_locale_vars();
        exit(EXIT_SUCCESS);
    }

    /* Process all given names.  */
    while (remaining < argc)
        show_info(argv[remaining++]);

    exit(EXIT_SUCCESS);
}
