# Save docker instance with 
## docker ps
## docker commit CONTAINER_ID name_of_docker
# docker run -it -v $(git rev-parse --show-toplevel):/home/LittleGPTracker -w /home/LittleGPTracker/projects docker_deb686 /bin/bash -c "make PLATFORM=ARCH"

docker run -it -v $(git rev-parse --show-toplevel):/home/LittleGPTracker \
-w /home/LittleGPTracker/projects --user "$(id -u):$(id -g)" \
registry.gitlab.steamos.cloud/steamrt/scout/sdk/i386:latest /bin/bash -c "make PLATFORM=STEAM"
