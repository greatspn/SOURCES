#!/bin/bash

case $1 in
build)
	docker build --target builder --tag greatspn_builder \
		--build-arg update_version="`date`" -f Dockerfile .

	docker build --target deploy_base --tag greatspn_deploy_base \
		--build-arg update_version="`date`" -f Dockerfile .

	docker build --target updater --tag greatspn_updater \
		--build-arg update_version="`date`" -f Dockerfile .
		
	docker build --tag greatspn \
		--build-arg update_version="`date`" -f Dockerfile .
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
