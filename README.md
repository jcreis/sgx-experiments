# SGX EXPERIMENTS

## Introduction
Intel Software Guard Extensions (SGX) is Intel's latest technology to assure execution of data inside Trusted Execution Environments (TEEs). This TEEs enable software developers to protect selected code and data from being accessed and stolen by 3rd parties, regardless if they have high level permissions to do so, or even if they have physical access to the machine running that code. This comes in handy when we are dealing with cloud providers to run our sensitive data.

TODO - some image describing a SGX-enabled System and how it works

Here we go through what we need to setup to run code that leverages Intel SGX security benefits, and we set up and run examples (based on those provided by Intel) either on your local machine or on inside a Docker container.

* */code-experiments* - environment setup guide can be found in the README.md file and some examples to run inside SGX
* */docker-files* - instructions and files to build a sgx-enabled environment to run code that leverages sgx
