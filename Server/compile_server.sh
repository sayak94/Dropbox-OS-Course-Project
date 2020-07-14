#!/bin/bash
gcc -g -Wall -c -o send-recv send-recv.c
gcc -g -Wall -c -o recv_client_file_change recv_client_file_change.c
gcc -g -Wall -o server server.c send-recv.c recv_client_file_change.c
