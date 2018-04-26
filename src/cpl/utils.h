/*

  Copyright (C) 2010 Alex Andreotti <alex.andreotti@gmail.com>

  This file is part of chmc.

  chmc is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  chmc is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with chmc.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef CHMC_UTILS_H
#define CHMC_UTILS_H

#include <sys/stat.h>
#include <dirent.h>

struct dir_tree_global;
struct dir_tree_local;

typedef int (*dir_tree_callback)(struct dir_tree_global *,
                                 struct dir_tree_local *);

struct dir_tree_global {
	char path[PATH_MAX];
	char prefix[PATH_MAX];
	char *_path;
	int depth;
	dir_tree_callback cb_dir;
	dir_tree_callback cb_file;
	void *user;
};

struct dir_tree_local {
	DIR *dp;
	struct stat statbuf;
	struct dirent *entry;
	char *dir;
};

const char *argv0_to_short_name(const char *argv0);
int dir_tree_walk(struct dir_tree_global *dtg, char *dir);
void path_add_slash(char *path);
void path_strip_slash(char *path);
void path_strip_last(char *path);

#endif /* CHMC_UTILS_H */
