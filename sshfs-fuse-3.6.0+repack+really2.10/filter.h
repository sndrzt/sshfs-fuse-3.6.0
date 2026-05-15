/*
  Filter module for SSHFS
  Copyright (C) 2025

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

#ifndef FILTER_H
#define FILTER_H

/* Load filter rules from file */
int filter_load_from_file(const char *filename);

/* Check if a filename should be filtered (excluded) */
int filter_check(const char *name);

/* Check if a full path should be filtered (for getattr) */
int filter_check_path(const char *path);

/* Enable/disable filtering */
void filter_set_enabled(int enabled);

/* Cleanup filter resources */
void filter_cleanup(void);

/* Check if filtering is enabled */
int filter_is_enabled(void);

#endif /* FILTER_H */

