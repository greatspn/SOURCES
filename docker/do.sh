#!/bin/bash

case $1 in
build)
	docker build -t greatspn:1.0 --build-arg update_version="`date`" .
	;;

run)
	docker run --rm -it --name=greatspn_image greatspn:1.0 --cap-add=SYS_PTRACE bash
	;;

start)
	docker run --rm -d --name=greatspn_image greatspn:1.0
	;;

stop)
	docker stop greatspn_image
	;;

exec)
	docker exec -it greatspn_image bash
	;;
esac
