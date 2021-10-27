# SGX EXPERIMENTS

## Introduction
Intel Software Guard Extensions (SGX) is Intel's latest technology to assure execution of data inside Trusted Execution Environments (TEEs). This TEEs enable software developers to protect selected code and data from being accessed and stolen by 3rd parties, regardless if they have high level permissions to do so, or even if they have physical access to the machine running that code. This comes in handy when we are dealing with cloud providers to run our sensitive data.

## Set up
We choose to work in a Linux environment, so all the steps described here and all the code here in this repository will focus only on this specific environment. Any other OS setup is outside of our scope in this repository.

To build the environment suitable to run code leveraging Intel SGX instructions, we'll need to install in our machine 3 major things:

- Intel SGX Driver
- Intel SGX Software Development Kit (SDK)
- Intel SGX Platform Software (PSW)

### Install SGX Driver
- clone from this repository to your local machine '$ git clone https://github.com/intel/linux-sgx-driver'

- Ensure that you have an operating system version supported as listed in releases: https://01.org/intel-software-guard-extensions/downloads

- Make sure you have the following hardware in your machine: 6th Generation Intel(R) Core(TM) Processor or newer

- Configure the system with the **SGX hardware enabled** option

- Ensure that the version of installed kernel headers matches the active kernel version on the system:
    * On Ubuntu: 
        * check if matching kernel headers are installed: 
            ```
            $ dpkg-query -s linux-headers-$(uname -r)
            ```
        * install matching headers: 
            ```
            $ sudo apt-get install linux-headers-$(uname -r)
            ```
    * On RHEL, Fedora or CentOS:
        * check if matching kernel headers are installed: 
            ``` 
            $ ls /usr/src/kernels/$(uname -r)
            ```
        * install matching headers: 
            ```
            $ sudo yum install kernel-devel
            ```
        * if the matching headers are still missing in /usr/src/kernels after the above command, try updating the kernel + reboot: 
            ```
            $ sudo yum install kernel
            $ sudo reboot
            ```
        * Then choose updated kernel on boot menu 
        * On RHEL 8.0 you'll need *elfutils-libelf-devel* package: 
            ```
            $ sudo yum install elfutils-libelf-devel
            ```

- Run the build command in the project's root directory (that we just cloned): 
      ```
      $ make
      ```

- Now that we have it built, we need to install it. For that, run:
```
$ sudo mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"   
$ sudo cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"  
$ sudo sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"
$ sudo /sbin/depmod
$ sudo /sbin/modprobe isgx
```

- On RHEL or CentOS, need to run the following command on each reboot: 
```
$ sudo /sbin/modprobe isgx
```

**Note:** To check if the driver is installed, check under the /dev folder ```$ ls /dev``` and see if there's a **/sgx** or **/isgx** (no DCAP driver) folder there. Both should have 2 elements inside: *'enclave'* and *'provision'*. If one of them is present, you have successfully installed the Intel SGX Driver.

**Additional Note:** If you are trying to setup this environment on a Docker container, the Driver needs to be installed on the host running that container and not in the container itself. You'll need to mount the host's **/dev/sgx** or **/dev/isgx** directory (depending on which driver you installed) into the container.

### Install SGX SDK

- Ensure that you have one of the following required OS's:
   * Ubuntu 18.04 LTS Desktop 64bits
   * Ubuntu 18.04 LTS Server 64bits
   * Ubuntu 20.04 LTS Desktop 64bits
   * Ubuntu 20.04 LTS Server 64bits
   * Red Hat Enterprise Linux Server release 8.2 64bits
   * CentOS 8.2 64bits
- Run the following commands to install the required tools to build the SDK:
   * On Ubuntu 18.04: 
      ```
      $ sudo apt-get install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
      ```
  * On Ubuntu 20.04:
      ```
      $ sudo apt-get install build-essential ocaml ocamlbuild automake autoconf libtool wget python-is-python3 libssl-dev git cmake perl
      ```
  * On Red Hat Enterprise Linux 8.2:
      ```
      $ sudo yum groupinstall 'Development Tools'
      $ sudo yum install ocaml ocaml-ocamlbuild wget python2 openssl-devel git cmake perl
      $ sudo alternatives --set python /usr/bin/python2
      ```
  * On CentOS 8.2:
      ```
      $ sudo dnf group install 'Development Tools'
      $ sudo dnf --enablerepo=PowerTools install ocaml ocaml-ocamlbuild redhat-rpm-config openssl-devel wget rpm-build git cmake perl python2
      $ sudo alternatives --set python /usr/bin/python2
      ```
      
- Use the following command to install additional required tools and latest Intel(R) SGX SDK Installer to build the Intel(R) SGX PSW:  
  1)  To install the additional required tools:
      * On Ubuntu 18.04 and Ubuntu 20.04:
      ```
        $ sudo apt-get install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
      ```
      * On Red Hat Enterprise Linux 8.2:
      ```
        $ sudo yum install openssl-devel libcurl-devel protobuf-devel cmake rpm-build createrepo yum-utils
      ```
      * On CentOS 8.2:
      ```
        $ sudo dnf --enablerepo=PowerTools install openssl-devel libcurl-devel protobuf-devel cmake rpm-build createrepo yum-utils
      ```

