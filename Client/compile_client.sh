#!/bin/bash
gcc -g -Wall -c -o client_inotify_map client_inotify_map.c
gcc -g -Wall -c -o send-recv send-recv.c
gcc -g -Wall -c -o trial trial.c
gcc -g -Wall -c -o send_client_file_change send_client_file_change.c
gcc -g -Wall -o client client.c client_inotify_map.c send-recv.c trial.c send_client_file_change.c
