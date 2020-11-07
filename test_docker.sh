#!/bin/bash

docker build -t ipc:latest .
docker run --pid=host --ipc=host --cpuset-cpus 1 -v /dev/shm:/dev/shm -v $(pwd)/temp:/temp ipc:latest &
docker run --pid=host --ipc=host --cpuset-cpus 2 -v /dev/shm:/dev/shm -v $(pwd)/temp:/temp ipc:latest
