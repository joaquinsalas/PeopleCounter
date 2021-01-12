#!/bin/bash

g++ process_images.cpp -o process_images `pkg-config --cflags --libs opencv`
