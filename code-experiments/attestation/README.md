## ENVIRONMENT SETUP

### ECDSA attestation
To enable ECDSA attestation    
- Ensure that you have the following required hardware:
  * 8th Generation Intel(R) Core(TM) Processor or newer with **Flexible Launch Control** support*
  * Intel(R) Atom(TM) Processor with **Flexible Launch Control** support*
- To use ECDSA attestation, you must install Intel(R) Software Guard Extensions Driver for Data Center Attestation Primitives (Intel(R) SGX DCAP).
Please follow the [Intel(R) SGX DCAP Installation Guide for Linux* OS](https://download.01.org/intel-sgx/latest/dcap-latest/linux/docs/Intel_SGX_SW_Installation_Guide_for_Linux.pdf) to install the Intel(R) SGX DCAP driver.

**NOTE**: If you had already installed Intel(R) SGX driver without ECDSA attestation, please uninstall the driver firstly and then install the Intel(R) SGX DCAP driver. Otherwise the newly installed Intel(R) SGX DCAP driver will be unworkable.

- Install Quote Provider Library(QPL). You can use your own customized QPL or use default QPL provided by Intel(libsgx-dcap-default-qpl)

- Install PCK Caching Service. For how to install and configure PCK Caching
Service, please refer to [SGXDataCenterAttestationPrimitives](https://github.com/intel/SGXDataCenterAttestationPrimitives/tree/master/QuoteGeneration/pccs)
- Ensure the PCK Caching Service is setup correctly by local administrator or data center administrator. Also make sure that the configure file of quote provider library (/etc/sgx_default_qcnl.conf) is consistent with the real environment, for example: PCS_URL=https://your_pcs_server:8081/sgx/certification/v1/

### Start or Stop aesmd Service
The Intel(R) SGX PSW installer installs an aesmd service in your machine, which is running in a special linux account `aesmd`.  
To stop the service: `$ sudo service aesmd stop`  
To start the service: `$ sudo service aesmd start`  
To restart the service: `$ sudo service aesmd restart`

### Configure the Proxy for aesmd Service
The aesmd service uses the HTTP protocol to initialize some services.  
If a proxy is required for the HTTP protocol, you may need to manually set up the proxy for the aesmd service.  
You should manually edit the file `/etc/aesmd.conf` (refer to the comments in the file) to set the proxy for the aesmd service.  
After you configure the proxy, you need to restart the service to enable the proxy.

</br>
</br>

## INTRODUCTION
Attestation by definition means the process of proving the integrity of something. 
Here it means the process of proving that an enclave has been established in a secure hardware environment and that a component's code has been properly instantiated inside that enclave. By doing so, we can assure that code running inside the enclave can be trusted.

This attestation process can be Local (both parties inside the same machine) or Remote (challenger is a remote entity).

Looking at the Remote Attestation mechanism, it assumes that a remote party can verify that the right application is running inside an enclave on an Intel SGX enabled platform. 
Remote attestation provides verification for three things: the application’s identity, its intactness (that it has not been tampered with), and that it is running securely within an enclave on an Intel SGX enabled platform. Attestation is necessary in order to make remote access secure, since very often the enclave’s contents may have to be accessed remotely, not from the same platform.


The Remote Attestation mechanism needs one thing that ensures trustworthiness to the remote party - it uses a Quoting Enclave (QE) to sign Reports (Application Enclave data) to be verified by remote parties as proof of trust. To do that, the QE requires an Attestation Key. The details of how the platform acquires the attestation key and how the quote can be verified differs between the two remote attestation models: EPID and DCAP (ECDSA)

EPID - When using EPID attestation, the Quoting Enclave uses an EPID key as the Attestation key, that it uses to sign the quote. This attestation key is given by Intel’s Attestation Service (IAS). The Quoting Enclave proves to Intel that it is running on a genuine SGX platform (using the Root Provisioning Key) and Intel provides it an EPID key.

DCAP - When using DCAP attestation, Eliptic Curve cryptography is used to sign the quote. The Quoting Enclave generates an EC key, it then uses a derivative of the Root Provisioning Key called the Provisioning Certification Key (PCK) to sign the public part of this EC key and include it in its quotes.

Thus, instead of going to the Intel Attestation Service to have a quote verified, a verifier can acquire all the necessary verification inputs using the Platform Certification Key (PCK) certificate. It can be obtained prior to an actual attestation taking place, since the PCK certificate is valid for extended time periods (i.e., years) it can be cached and reused across many attestations. 

![Alt text](https://github.com/jcreis/jcreis/blob/main/tech-icons/java-icon.jpg?raw=true "Title")
