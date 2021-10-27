**Disclaimer:** *This is a summary version of the one that can be found in Intel's linux-sgx repository, with some complementary information I found useful to add upon setting up the environment myself*

---

## Set up
We choose to work in a Linux environment, so all the steps described here and all the code here in this repository will focus only on this specific environment. Any other OS setup is outside of our scope in this repository.

To build the environment suitable to run code leveraging Intel SGX instructions, we'll need to install in our machine 3 major things:

- Intel SGX Driver
- Intel SGX Software Development Kit (SDK)
- Intel SGX Platform Software (PSW)

---

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

---

### Install SGX SDK

#### 1) Setup the environment

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
      
- Use the following command to install additional required tools and latest SDK installer to build the PSW:  
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
  2) To install latest SDK installer: Ensure that you have downloaded latest Intel(R) SGX SDK Installer from the [Intel(R) SGX SDK](https://software.intel.com/en-us/sgx-sdk/download) and followed the Installation Guide in the same page to install latest Intel(R) SGX SDK Installer.

- Download the source code and prepare the submodules and prebuilt binaries:
```
   $ git clone https://github.com/intel/linux-sgx.git
   $ cd linux-sgx && make preparation
```

- Copy the mitigation tools corresponding to current OS distribution from external/toolset/{current_distr} to /usr/local/bin and make sure they have execute permission:
  ```
    $ sudo cp external/toolset/{current_distr}/{as,ld,ld.gold,objdump} /usr/local/bin
    $ which as ld ld.gold objdump
  ```
  
  #### 2) Build the SDK and SDK installer
- To build the SDK with default configuration, run the following command in the linux-sgx project's root directory:
```
  $ make sdk
```  
- To clean the files generated by previous `make sdk` command, enter the following command:  
```
  $ make clean
```

  #### 3) Install the SDK
  
- Use the following command to install the required tool to use Intel(R) SGX SDK:
  * On Ubuntu 18.04 and Ubuntu 20.04:
  ```  
    $ sudo apt-get install build-essential python
  ```
  * On Red Hat Enterprise Linux 8.2 and CentOS 8.2:
  ```
     $ sudo yum groupinstall 'Development Tools'
     $ sudo yum install python2
     $ sudo alternatives --set python /usr/bin/python2 
  ```
  
- To invoke the installer, run:
```
$ cd linux/installer/bin
$ ./build-installpkg.sh sdk
$ ./sgx_linux_x64_sdk_${version}.bin
```
The common path to install the SGX SDK is on */opt/intel/* folder, but you can install it wherever you want (just remember where you put it)

NOTE: You need to set up the needed environment variables before compiling your code. To do so, run:
```
  $ source $<sgx-sdk-install-path>/environment
```

---

### Install SGX PSW

#### 1) Setup the environment

- Ensure that you have one of the following required OS's:
   * Ubuntu 18.04 LTS Desktop 64bits
   * Ubuntu 18.04 LTS Server 64bits
   * Ubuntu 20.04 LTS Desktop 64bits
   * Ubuntu 20.04 LTS Server 64bits
   * Red Hat Enterprise Linux Server release 8.2 64bits
   * CentOS 8.2 64bits
 
 - All the other stuff needed should be covered by the previous SDK installation, so we don't need to install any more tools
  
  #### 2) Build the PSW
- To build the PSW with default configuration, run the following command in the linux-sgx project's root directory:
```
  $ make psw
```
- Or, alternatively, you can run:
  * On Ubuntu 18.04 and Ubuntu 20.04:
   ```
  $ make deb_psw_pkg
  ```
  If needed, you can find the generated PSW installers located under `linux/installer/deb/libsgx-urts`, `linux/installer/deb/libsgx-enclave-common`, `linux/installer/deb/libsgx-uae-service`, `linux/installer/deb/libsgx-epid`, `linux/installer/deb/libsgx-launch`, `linux/installer/deb/libsgx-quote-ex` and `linux/installer/deb/sgx-aesm-service` respectively, and run the build.sh script in order to build them one by one.
  
- To clean the files generated by previous `make psw` command, enter the following command:  
```
  $ make clean
```

After that, you need to update the apt:
  * On Ubuntu 18.04 and Ubuntu 20.04:
  ```
  $ sudo apt update
  ```

  - To build local RPM package repository, enter the following command:
  ```
  $ make rpm_local_repo
  ```
  You can find the local package repository located under `linux/installer/rpm/sgx_rpm_local_repo`.

- To add the local RPM package repository to the system repository configuration, you can use the following command. You need to replace PATH_TO_LOCAL_REPO with the proper path on your system:
  * On Red Hat Enterprise Linux 8.2 and CentOS 8.2:
  ```
  $ sudo yum-config-manager --add-repo file://PATH_TO_LOCAL_REPO
  ```
  - To ignore the gpgcheck when you install the package, enter the following command:
  * On Red Hat Enterprise Linux 8.2 and CentOS 8.2:
  ```
  $ sudo yum --nogpgcheck install <package>
  ```

  #### 3) Install the PSW
  
- Use the following command to install the required tools:
  * On Ubuntu 18.04 and Ubuntu 20.04:
  ```
    $ sudo apt-get install libssl-dev libcurl4-openssl-dev libprotobuf-dev
  ```
  * On Red Hat Enterprise Linux 8.2:  
  ```
    $ sudo yum install openssl-devel libcurl-devel protobuf-devel
  ```
  * On CentOS 8.2:
  ```
    $ sudo dnf --enablerepo=PowerTools install libcurl-devel protobuf-devel
  ```
  
**NOTE:** The SGX PSW provides 3 services: launch, EPID-based attestation, and algorithm agnostic attestation. Starting with the 2.8 release, the SGX PSW is split into smaller packages and the user can choose which features and services to install. There are 2 methods to install the required packages: Using individual packages or using the local repo generated by the build system. Using the local repo is recommended since the system will resolve the dependencies automatically. Currently, we support .deb and .rpm based repos.

- Using the local repository (recommended)

|   |Ubuntu 18.04 and Ubuntu 20.04|Red Hat Enterprise Linux 8.2, CentOS 8.2|
| ------------ | ------------ | ------------ |
|launch service |apt-get install libsgx-launch libsgx-urts|yum install libsgx-launch libsgx-urts|
|EPID-based attestation service|apt-get install libsgx-epid libsgx-urts|yum install libsgx-epid libsgx-urts|
|algorithm agnostic attestation service|apt-get install libsgx-quote-ex libsgx-urts|yum install libsgx-quote-ex libsgx-urts|
|DCAP ECDSA-based service |apt-get install libsgx-dcap-ql|yum install libsgx-dcap-ql|

- Using the individual packages:
Please refer [Intel_SGX_Installation_Guide_Linux](https://download.01.org/intel-sgx/latest/linux-latest/docs/) for detail.

