/*
 *     Copyright (C) 2001 Steve Harris
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>

#include <jack/jack.h>

jack_port_t *input_port;

unsigned int impulse_sent = 0;
float *response;
unsigned long response_duration;
unsigned long response_pos;

int
process (jack_nframes_t nframes, void *arg)

{
	jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);
	unsigned int i;

	for (size_t i = 0; i < nframes; i++) {
		in[i] = -0.5;
	}

	return 0;
}

void
jack_shutdown (void *arg)
{
	exit (1);
}

int
main (int argc, char *argv[])

{
	jack_client_t *client;
	const char **ports;
	float fs;		// The sample rate
	float peak;
	unsigned long peak_sample;
	unsigned int i;
	float duration = 0.0f;
	unsigned int c_format = 0;
        int longopt_index = 0;
	int c;
        extern int optind, opterr;
        int show_usage = 0;
        char *optstring = "d:f:h";
        struct option long_options[] = {
                { "help", 1, 0, 'h' },
                { "duration", 1, 0, 'd' },
                { "format", 1, 0, 'f' },
                { 0, 0, 0, 0 }
        };

        while ((c = getopt_long (argc, argv, optstring, long_options, &longopt_index)) != -1) {
                switch (c) {
		case 1:
			// end of opts, but don't care
			break;
                case 'h':
                        show_usage++;
                        break;
		case 'd':
			duration = (float)atof(optarg);
			break;
		case 'f':
			if (*optarg == 'c' || *optarg == 'C') {
				c_format = 1;
			}
			break;
		default:
			show_usage++;
			break;
		}
	}

	/* try to become a client of the JACK server */

	if ((client = jack_client_open("jack-corruptor", JackNullOption, NULL)) == 0) {
		fprintf (stderr, "jack server not running?\n");
		return 1;
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (client, process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. once the client is activated
	   (see below), you should rely on your own sample rate
	   callback (see above) for this value.
	*/

	fs = jack_get_sample_rate(client);
	response_duration = (unsigned long) (fs * duration);
	response = malloc(response_duration * sizeof(float));
	fprintf(stderr,
		"Grabbing %f seconds (%lu samples) of impulse response\n",
		duration, response_duration);

	/* create two ports */

	input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

	/* tell the JACK server that we are ready to roll */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	/* connect the ports. Note: you can't do this before
	   the client is activated (this may change in the future).
	*/

	if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
		fprintf(stderr, "Cannot find any physical capture ports");
		exit(1);
	}

	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);

	/* Wait for grab to finish */
	while (true) {
		sleep (1);
	}
	jack_client_close (client);

	exit (0);
}
