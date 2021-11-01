### INTRODUCTION

To build a sample enclave app, apart of the SGX Driver and SGX SDK + PSW libraries, you'll need to structure your application the following way:

- App folder: where you'll have every file related to the untrusted app, where you specify the normal execution of the application. This will be turned into an executable file to be executed.
- Enclave folder: where you'll specify which functions are to be run inside the enclave and which are not, that the untrusted app will call upon executing.
- Makefile: build file specifying all the libraries needed to compile the executable.

**Note:** If you want to change the layout of the project, you'll also need to change it in the Makefile. The Makefile is done for a generic case, if you want to custom your app, you'll need to pay attention to this file.

This app will be an extension of the work done in Intel's demo implementation - SampleEnclave - where they run an application inside an enclave that just prints information out. 
Here, we extend the application showing how to implement any new functions inside the enclave.

There are 3 main components:
1) App.cpp - it's the file where you code all your application's behaviour. Here you have a main() method, where you'll need to change in order to call any desired new functions that you want to try out.
2) Enclave.cpp - it's the file where you implement all the functions that the application will call, that will operate within the enclave's boundaries.
3) Enclave.edl - this file works as the interface between application and enclave. It's here where you specify which functions are to be run as an ECALL and which are to be run as an OCALL.

1 - run make
2 - files created: basic-enclave-app - an executable file for the application
                   enclave.so - enclave binary file
3 - to run the application: ./basic-enclave-app