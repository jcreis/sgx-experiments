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
