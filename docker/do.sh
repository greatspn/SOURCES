#!/bin/bash

case $1 in
build)
	docker build -t greatspn --build-arg update_version="`date`" -f Dockerfile .
	;;

run)
	docker run --rm -it --name=greatspn_image greatspn bash
	;;

run-ptrace)
	docker run --rm -it --name=greatspn_image greatspn --cap-add=SYS_PTRACE bash
	;;

start)
	docker run --rm -d --name=greatspn_image greatspn
	;;

stop)
	docker stop greatspn_image
	;;

exec)
	docker exec -it greatspn_image bash
	;;
esac
