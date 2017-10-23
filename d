#!/usr/bin/env bash

imagename=mittos64

buildroot="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"

# Check if the docker image exists
# If not, ask if it should be built
if ! docker image ls -f reference=${imagename} | grep ${imagename} >/dev/null; then
  echo "Docker image does not exist."
  echo "Do you wish to build it? (This could take a while)"
  read -r -p "[y/N]: " response
  case "$response" in
    [Yy][Ee][Ss]|[Yy])
      docker build -t ${imagename} ${buildroot}/toolchain
      ;;
    *)
      exit
      ;;
  esac
fi


# Check if a container is already running the image
# If so, execute the command in the running container
if docker ps -f name=${imagename}-run | grep ${imagename}-run ; then
  docker exec -it ${imagename}-run "$@"
else
  docker run -it --rm -v ${buildroot}:/opt --name ${imagename}-run ${imagename} "$@"
fi
