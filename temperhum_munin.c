/*
 * Copyright 2016   Harald Nordgård-Hansen
 * Copyright ©2009  Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "comms.h"
#include "readings.h"

int main(int argc, char *argv[]) {
	struct sht1x_device device;
	char *path = NULL;
	int err, status;
	char config[4096];
	FILE *cfg_fd;
	struct sht1x_readings readings;

	if (argc == 2) {
		if (strcmp(argv[1], "autoconf") == 0) {
			printf("no\n");
			status = EXIT_SUCCESS;
			goto done;
		}
		if (strcmp(argv[1], "config") == 0) {
			printf("graph_title Temperature and Humidity\n");
			printf("graph_category sensors\n");
			printf("graph_args --base 1000\n");
			printf("graph_vlabel C/%%\n");
			printf("temp.label Temperature (C)\n");
			printf("temp.draw LINE1\n");
			printf("hum.label Humidity (%%)\n");
			printf("hum.draw LINE1\n");
			printf("dew.label Dewpoint (C)\n");
			printf("dew.draw LINE1\n");
			status = EXIT_SUCCESS;
			goto done;
		}
		printf("Unknown parameter (%s)\n", argv[1]);
		status = EXIT_FAILURE;
		goto done;
	}
	else if (argc == 1) {
		path = getenv("device");
		if (path == NULL) {
			printf("Set 'device' environment variable (1-1.2.3)\n");
			status = EXIT_FAILURE;
			goto done;
		}
	}
	else
	{
		printf("Call according to munin plugin conventions.\n");
		status = EXIT_FAILURE;
		goto done;
	}

	memset(&device, 0, sizeof(device));
	strncpy(device.name, path, sizeof(device.name));

	sht1x_open(&device);
	device.tc_offset = 0;
	snprintf(config, sizeof(config), "%s.conf", device.name);
	errno = 0;
	cfg_fd = fopen(config, "r");
	if (cfg_fd != NULL) {
		char buf[1024];
		if (fgets(buf, sizeof(buf), cfg_fd) != NULL) {
			errno = 0;
			device.tc_offset = strtod(buf, NULL);

			if (errno != 0) {
				fprintf(stderr, "Error reading config for %s (%s)\n", device.name, strerror(errno));
				status = EXIT_FAILURE;
				goto closeall;
			}
		}
		fclose(cfg_fd);
	} else if (errno != ENOENT) {
		fprintf(stderr, "Error opening config for %s (%s)\n", device.name, strerror(errno));
		status = EXIT_FAILURE;
		goto closeall;
	}

	err = sht1x_device_reset(&device);
	if (err) {
		fprintf(stderr, "Error resetting device %s\n", device.name);
		status = EXIT_FAILURE;
		goto closeall;
	}

	device.status = sht1x_read_status(&device);
	if (!device.status.valid) {
		fprintf(stderr, "Status read failed for %s\n", device.name);
		status = EXIT_FAILURE;
		goto closeall;
	} else {
		if (device.status.low_resolution || device.status.heater) {
			device.status.low_resolution = 0;
			device.status.heater = 0;

			if (sht1x_write_status(&device, device.status)) {
				fprintf(stderr, "Status write failed for %s\n", device.name);
				status = EXIT_FAILURE;
				goto closeall;
			}
		}
	}


	readings = sht1x_getreadings(&device, device.status.low_resolution);
	printf("temp.value %6.2lf\n", readings.temperature_celsius);
	printf("hum.value %6.2lf\n", readings.relative_humidity);
	printf("dew.value %6.2lf\n", readings.dew_point);
	status = EXIT_SUCCESS;

closeall:
	sht1x_close(&device);

done:
	exit(status);
}
