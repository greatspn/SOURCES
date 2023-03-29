#!/bin/bash

DOCKERFILE=r-base.Dockerfile

case $1 in
build)
	docker build --target builder --tag greatspn_builder_stage \
		--build-arg update_version="`date`" -f ${DOCKERFILE} .

	docker build --target deploy_base --tag greatspn_deploy_base_stage \
		--build-arg update_version="`date`" -f ${DOCKERFILE} .

	docker build --target updater --tag greatspn_updater_stage \
		--build-arg update_version="`date`" -f ${DOCKERFILE} .

	docker build --tag greatspn \
		--build-arg update_version="`date`" -f ${DOCKERFILE} .
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

update)
	wget https://raw.githubusercontent.com/greatspn/SOURCES/master/docker/do.sh ./do.sh 
	wget https://raw.githubusercontent.com/greatspn/SOURCES/master/docker/${DOCKERFILE} ./${DOCKERFILE}
	;;

esac
