### INTRODUCTION

To build a sample enclave app, apart of the SGX Driver and SGX SDK + PSW libraries, you'll need to structure your application the following way:

- App folder: where you'll have every file related to the untrusted app, where you specify the normal execution of the application. This will be turned into an executable file to be executed.
- Enclave folder: where you'll specify which functions are to be run inside the enclave and which are not, that the untrusted app will call upon executing.
- Makefile: build file specifying all the libraries needed to compile the executable.

**Note: If you want to change the layout of the project, you'll also need to change it in the Makefile. The Makefile is done for a generic case, if you want to custom your app, you'll need to pay attention to this file.**

This app will be an extension of the work done in Intel's demo implementation - SampleEnclave - where they run an application inside an enclave that just prints information out. 
Here, we extend the application showing how to implement any new functions inside the enclave.

There are 3 main components:
1) App.cpp - it's the file where you code all your application's behaviour. Here you have a main() method, where you'll need to change in order to call any desired new functions that you want to try out.
2) Enclave.cpp - it's the file where you implement all the functions that the application will call, that will operate within the enclave's boundaries.
3) Enclave.edl - this file works as the interface between application and enclave. It's here where you specify which functions are to be run as an ECALL and which are to be run as an OCALL.

- 1) First, we need to build the file by running $ make
     * ```$make SGX_MODE=SIM``` for building in Simulation mode (no need to have Hardware that supports SGX to compile)
     * ```$make SGX_MODE=HW``` for building in Hardware mode (the default mode specified in the Makefile)
     * You should have an output similar to this: 
     ```C:/Users/MyUser/dir1/sgx-experiments/code-experiments/basic-enclave$ make
        make[1]: Entering directory '/home/defaultuser/dir1/sgx-experiments/code-experiments/basic-enclave'
        GEN  =>  App/Enclave_u.h
        CC   <=  App/Enclave_u.c
        CXX  <=  App/App.cpp
        CXX  <=  App/Edger8rSyntax/Types.cpp
        CXX  <=  App/Edger8rSyntax/Pointers.cpp
        CXX  <=  App/Edger8rSyntax/Arrays.cpp
        CXX  <=  App/Edger8rSyntax/Functions.cpp
        CXX  <=  App/TrustedLibrary/Thread.cpp
        CXX  <=  App/TrustedLibrary/Libcxx.cpp
        CXX  <=  App/TrustedLibrary/Libc.cpp
        LINK =>  basic-enclave-app
        GEN  =>  Enclave/Enclave_t.h
        CC   <=  Enclave/Enclave_t.c
        CXX  <=  Enclave/Edger8rSyntax/Arrays.cpp
        CXX  <=  Enclave/Edger8rSyntax/Functions.cpp
        CXX  <=  Enclave/Edger8rSyntax/Pointers.cpp
        CXX  <=  Enclave/Edger8rSyntax/Types.cpp
        CXX  <=  Enclave/Enclave.cpp
        CXX  <=  Enclave/TrustedLibrary/Libc.cpp
        CXX  <=  Enclave/TrustedLibrary/Libcxx.cpp
        CXX  <=  Enclave/TrustedLibrary/Thread.cpp
        LINK =>  enclave.so
        <EnclaveConfiguration>
            <ProdID>0</ProdID>
            <ISVSVN>0</ISVSVN>
            <StackMaxSize>0x40000</StackMaxSize>
            <HeapMaxSize>0x100000</HeapMaxSize>
            <TCSNum>10</TCSNum>
            <TCSPolicy>1</TCSPolicy>
            <!-- Recommend changing 'DisableDebug' to 1 to make the enclave undebuggable for enclave release -->
            <DisableDebug>0</DisableDebug>
            <MiscSelect>0</MiscSelect>
            <MiscMask>0xFFFFFFFF</MiscMask>
        </EnclaveConfiguration>
        tcs_num 10, tcs_max_num 10, tcs_min_pool 1
        The required memory is 4067328B.
        The required memory is 0x3e1000, 3972 KB.
        Succeed.
        SIGN =>  enclave.signed.so
        The project has been built in debug hardware mode.
        make[1]: Leaving directory '/home/defaultuser/dir1/sgx-experiments/code-experiments/basic-enclave'
        ```

- 2) After that, some files will be generated automatically. Some important ones are:
     * basic-enclave-app - an executable file for the application
     * enclave.so - enclave binary file
- 3) Now that we have the executable ready, we can run the application by executing the following command:
     ```$./basic-enclave-app```
     * The executable should output: 
     ```C:/Users/MyUser/dir1/sgx-experiments/code-experiments/basic-enclave$ ./basic-enclave-app
        ADD -> r = 8
        SUBTRACT -> r = -2
        MULTIPLY -> r = 15
        DIVIDE -> r = 0
        Info: Basic Enclave successfully returned.
        Enter a character before exit ...
     ```
     


