/*
  Filter module for SSHFS
  Copyright (C) 2025

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <glib.h>

static GHashTable *filter_rules = NULL;
static int filter_enabled = 0;

/* Load filter rules from file */
int filter_load_from_file(const char *filename)
{
    FILE *fp;
    char line[1024];
    int line_num = 0;
    
    if (!filename || !filename[0])
        return -1;
    
    fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open filter file: %s\n", filename);
        return -1;
    }
    
    if (filter_rules) {
        g_hash_table_destroy(filter_rules);
        filter_rules = NULL;
    }
    
    filter_rules = g_hash_table_new_full(g_str_hash, g_str_equal,
                                         g_free, NULL);
    if (!filter_rules) {
        fclose(fp);
        return -1;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        int include;
        char *pattern;
        char *key;
        int *value;
        
        line_num++;
        
        /* Skip leading whitespace */
        while (*p == ' ' || *p == '\t')
            p++;
        
        /* Skip comments and empty lines */
        if (*p == '#' || *p == '\n' || *p == '\0')
            continue;
        
        /* Parse include/exclude marker */
        if (*p == '+')
            include = 1;
        else if (*p == '-')
            include = 0;
        else {
            fprintf(stderr, "Filter file line %d: missing +/- marker\n", line_num);
            continue;
        }
        p++;
        
        /* Skip whitespace after marker */
        while (*p == ' ' || *p == '\t')
            p++;
        
        /* Remove trailing newline and spaces */
        pattern = p;
        char *end = pattern + strlen(pattern) - 1;
        while (end > pattern && (*end == '\n' || *end == ' ' || *end == '\t'))
            end--;
        *(end + 1) = '\0';
        
        if (strlen(pattern) == 0) {
            fprintf(stderr, "Filter file line %d: empty pattern\n", line_num);
            continue;
        }
        
        /* Store pattern with include flag */
        key = g_strdup(pattern);
        value = g_malloc(sizeof(int));
        *value = include;
        g_hash_table_insert(filter_rules, key, value);
        
        if (include)
            fprintf(stderr, "Filter: include pattern '%s'\n", pattern);
        else
            fprintf(stderr, "Filter: exclude pattern '%s'\n", pattern);
    }
    
    fclose(fp);
    filter_enabled = (filter_rules != NULL && g_hash_table_size(filter_rules) > 0);
    
    fprintf(stderr, "Loaded %d filter rules from %s\n",
            filter_rules ? g_hash_table_size(filter_rules) : 0, filename);
    
    return 0;
}

/* Check if a filename should be filtered out (excluded)
 * Returns: 1 if should be filtered (hidden), 0 if should be shown
 */
int filter_check(const char *name)
{
    GHashTableIter iter;
    gpointer key, value;
    int last_match_include = 1;  /* Default: include if no rule matches */
    int matched = 0;
    
    if (!filter_enabled || !filter_rules)
        return 0;
    
    /* Apply rules */
    g_hash_table_iter_init(&iter, filter_rules);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        const char *pattern = (const char *)key;
        int include = *(int *)value;
        
        if (fnmatch(pattern, name, FNM_PATHNAME) == 0) {
            matched = 1;
            last_match_include = include;
        }
    }
    
    /* If no rule matched, include by default */
    if (!matched)
        return 0;
    
    /* Filter out if last matching rule is exclude */
    return (last_match_include == 0) ? 1 : 0;
}

/* Check if a full path should be filtered (for getattr/lookup) */
int filter_check_path(const char *path)
{
    const char *name;
    
    if (!filter_enabled)
        return 0;
    
    /* Extract the last component of the path */
    name = strrchr(path, '/');
    if (name)
        name++;
    else
        name = path;
    
    /* Skip root directory */
    if (name[0] == '\0')
        return 0;
    
    return filter_check(name);
}

/* Enable/disable filtering */
void filter_set_enabled(int enabled)
{
    filter_enabled = enabled;
}

/* Check if filtering is enabled */
int filter_is_enabled(void)
{
    return filter_enabled;
}

/* Cleanup filter resources */
void filter_cleanup(void)
{
    if (filter_rules) {
        g_hash_table_destroy(filter_rules);
        filter_rules = NULL;
    }
    filter_enabled = 0;
}

