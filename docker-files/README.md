#### Dockerfile creates a Ubuntu 18.04 image with already installed SDK and DCAP packages

* build image based on Dockerfile 
```
$ docker build -t  .
```

#### docker-compose file creates a container based on the image created previously by the Dockerfile, mounting the SDX Driver into the container's /dev/sgx folder, as well as enabling the AESMD service that runs on the host

* create the container using the docker-compose file
```
$ docker-compose up -d
```

* enter the container we just created
```
$ docker exec -it <container_id> /bin/bash
```

* to stop the container
```
$ docker-compose stop
```