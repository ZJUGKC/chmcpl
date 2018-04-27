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
#include "config.h"

#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include <getopt.h>             /* getopt_long() */

#include <libconfig.h>

#include "chmc.h"

#include "utils.h"
#include "err.h"

static const char *short_name;
static const char *output = "-";
static const char *conf = NULL;

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
	        "Usage: %s [options] directory...\n\n"
	        "Version " PACKAGE_VERSION "\n"
	        "Options:\n"
	        "-h, --help              print this message\n"
	        "-c, --conf=FILE         read config FILE\n"
	        "-o, --out=FILE          write to FILE instead of stdout\n"
	        "-t, --tmp=TMPDIR        temporary directory [/tmp]\n"
	        "", short_name);
}

static const char short_options[] = "hc:o:t:";

static const struct option long_options[] = {
	{ "help",   no_argument,          NULL, 'h' },
	{ "conf",   required_argument,    NULL, 'c' },
	{ "out",    no_argument,          NULL, 'o' },
	{ "tmp",    required_argument,    NULL, 't' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
	struct chmcFile chm;
	int err, i, c;
	config_t config;
	struct chmcConfig chm_conf;

	short_name = argv0_to_short_name(argv[0]);

	memset(&chm_conf, 0, sizeof(chm_conf));

	for (;;) {
		c = getopt_long(argc, argv, short_options, long_options, &i);

		if (-1 == c)
			break;

		switch (c) {
		case 0: /* getopt_long() flag */
			break;
		case 'h':
			usage(stdout, argc, argv);
			exit(EXIT_SUCCESS);
		case 'c':
			/* TODO check it is a file */
			conf = optarg;
			break;
		case 'o':
			output = optarg;
			break;
		case 't':
			/* TODO check it is a dir */
			chm_conf.tmpdir = optarg;
			break;
		default:
			usage(stderr, argc, argv);
			exit(EXIT_FAILURE);
		}
	}

	config_init(&config);

	if (conf != NULL) {
		config_setting_t *setting;

		err = config_read_file(&config, conf);
		if (err == CONFIG_FALSE) {
			if (config_error_type(&config) == CONFIG_ERR_PARSE)
				error(EXIT_FAILURE, 0, "%s: %d: error: %s\n",
				        config_error_file(&config),
				        config_error_line(&config),
				        config_error_text(&config));
			error(EXIT_FAILURE, errno, "reading config '%s': %s", conf,
			      config_error_text(&config));
		}

		setting = config_lookup(&config, "document.title");
		if (setting) {
			const char *val;
			val = config_setting_get_string(setting);
			if (val != NULL)
				chm_conf.title = val;
		}
		setting = config_lookup(&config, "document.hhc");
		if (setting) {
			const char *val;
			val = config_setting_get_string(setting);
			if (val != NULL)
				chm_conf.hhc = val;
		}
		setting = config_lookup(&config, "document.hhk");
		if (setting) {
			const char *val;
			val = config_setting_get_string(setting);
			if (val != NULL)
				chm_conf.hhk = val;
		}
		setting = config_lookup(&config, "document.deftopic");
		if (setting) {
			const char *val;
			val = config_setting_get_string(setting);
			if (val != NULL)
				chm_conf.deftopic = val;
		}
	}

	err = chmc_init(&chm, output, &chm_conf);
	if (err)
		error(EXIT_FAILURE, err, "chmc_init failed: %s", chmcerr_message());

	i = optind;
	if (i < argc) {
		while (i < argc) {
			/* TODO check if file */
			if (chmc_add_tree(&chm, argv[i]))
				error(EXIT_FAILURE, err, "chmc_add_tree failed: %s",
				      chmcerr_message());
			i++;
		}
	} else {
		/* NULL = current directory */
		if (chmc_add_tree(&chm, NULL))
		    error(EXIT_FAILURE, err, "chmc_add_tree failed: %s",
		          chmcerr_message());
	}

	chmc_tree_done(&chm);
	chmc_term(&chm);

	config_destroy(&config);

	return 0;
}
