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
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "utils.h"

void path_strip_last(char *path)
{
	char *x;
	int len;

	len = strlen(path);

	if ((len > 0) && (len <= PATH_MAX)) {
		if (path[len - 1] == '/')
			path[len - 1] = '\0';
		x = strrchr(path, '/');
		if (x)
			*(++x) = '\0';
	}
}

void path_strip_slash(char *path)
{
	int len;

	len = strlen(path);
	while (path[len - 1] == '/')
		path[--len] = '\0';
}

void path_add_slash(char *path)
{
	if (path[strlen(path) - 1] != '/')
		strncat(path, "/", PATH_MAX);
}

const char *argv0_to_short_name(const char *argv0)
{
	const char *short_name;

	short_name = strrchr(argv0, '/');

	if (short_name)
		short_name++;
	else
		short_name = argv0;

	return short_name;
}

/* ISBN:0764543733 Scanning Directories */
int dir_tree_walk(struct dir_tree_global *dtg, char *dir)
{
	struct dir_tree_local dtl;

	dtl.dir = dir;

	if ((dtl.dp = opendir(dir)) == NULL) {
		fprintf(stderr, "cannot open directory: %s\n", dir);
		path_strip_last(dtg->path);
		dtg->depth--;
		return errno;
	}

	chdir(dir);

	while ((dtl.entry = readdir(dtl.dp)) != NULL) {
		lstat(dtl.entry->d_name, &dtl.statbuf);
		if (S_ISDIR(dtl.statbuf.st_mode)) {
			/* Found a directory, but ignore . and .. */
			if (strcmp(".", dtl.entry->d_name) == 0
			    || strcmp("..", dtl.entry->d_name) == 0)
				continue;

			if (dtg->cb_dir) {
				int err;
				err = dtg->cb_dir(dtg, &dtl);
				if (err)
					return err;
			}
			/* Recurse at a new indent level */
			strncat(dtg->path, dtl.entry->d_name, PATH_MAX);
			strncat(dtg->path, "/", PATH_MAX);
			dtg->depth++;
			dir_tree_walk(dtg, dtl.entry->d_name);
		} else {
			if (dtg->cb_file) {
				int err;
				err = dtg->cb_file(dtg, &dtl);
				if ( err )
					return err;
			}
		}
	}

	path_strip_last(dtg->path);
	dtg->depth--;
	chdir("..");
	closedir(dtl.dp);

	return 0;
}
