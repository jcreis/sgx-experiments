#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <iostream>
// Needed for definition of remote attestation messages.
#include "remote_attestation_result.h"

#include "enclave_u.h"

// Needed to call untrusted key exchange library APIs, i.e. sgx_ra_proc_msg2.
#include "sgx_ukey_exchange.h"

// Needed to get service provider's information, in your real project, you will
// need to talk to real server.
#include "network_ra.h"

// Needed to create enclave and do ecall.
#include "sgx_urts.h"

// Needed to query extended epid group id.
#include "sgx_uae_epid.h"
#include "sgx_uae_quote_ex.h"

#include "service_provider.h"

#define ENCLAVE_NAME "enclave.signed.so"

void PRINT_BYTE_ARRAY(
    FILE *file, void *mem, uint32_t len)
{
    if(!mem || !len)
    {
        fprintf(file, "\n( null )\n");
        return;
    }
    uint8_t *array = (uint8_t *)mem;
    fprintf(file, "%u bytes:\n{\n", len);
    uint32_t i = 0;
    for(i = 0; i < len - 1; i++)
    {
        fprintf(file, "0x%x, ", array[i]);
        if(i % 8 == 7) fprintf(file, "\n");
    }
    fprintf(file, "0x%x ", array[i]);
    fprintf(file, "\n}\n");
}

int main(int argc, char* argv[])
{
    int ret = 0;
    
    sgx_enclave_id_t enclave_id = 0;
    int enclave_lost_retry_time = 1;
    int busy_retry_time = 4;
    sgx_ra_context_t context = INT_MAX;
    sgx_status_t status = SGX_SUCCESS;
    sgx_att_key_id_t selected_key_id = {0};
    sgx_launch_token_t token = { 0 };
    int token_updated = 0;

    ra_samp_request_header_t *p_msg0_full = NULL;
    ra_samp_response_header_t *p_msg0_resp_full = NULL;
    ra_samp_request_header_t *p_msg1_full = NULL;
    ra_samp_response_header_t *p_msg2_full = NULL;
    ra_samp_request_header_t* p_msg3_full = NULL;
    sgx_ra_msg3_t *p_msg3 = NULL;
    ra_samp_response_header_t* p_att_result_msg_full = NULL;

    FILE* OUTPUT = stdout;

    fprintf(OUTPUT, "\n #### Starting ECDSA attestation ####\n");
    
    // Preparation for remote attestation by configuring extended epid group id.
    {
        //first we need the EPID GID to include in MSG0
        fprintf(OUTPUT, "\n>>>> Prepare Extended EPID GID.\n");
        uint32_t extended_epid_group_id = 0;
        std::cout << "\nExtender epid group: " << extended_epid_group_id << std::endl;

        //we can call 'sgx_get_extended_epid_group_id(&extended_epid_group_id);' but it's default value is 0
        ret = 0; 
        std::cout << "It reaches here!" << std::endl;
        if (ret != SGX_SUCCESS)
        {
            ret = -1;
            fprintf(OUTPUT, "\nError, call sgx_get_extended_epid_group_id fail [%s].\n",
                __FUNCTION__);
            return ret;
        }
        fprintf(OUTPUT, "Call sgx_get_extended_epid_group_id was a success.\n");

        //start generating MSG0
        fprintf(OUTPUT, "\n>>>> Start generating msg0.\n");

        p_msg0_full = (ra_samp_request_header_t*)malloc( sizeof(ra_samp_request_header_t)+sizeof(uint32_t) );
        if (p_msg0_full == NULL)
        {
            ret = -1;
            goto CLEANUP;
        }
        p_msg0_full->type = TYPE_RA_MSG0;
        p_msg0_full->size = sizeof(uint32_t);
        //p_msg0_full->body = extended_epid_group_id
        *(uint32_t*)( (uint8_t*)p_msg0_full + sizeof(ra_samp_request_header_t) ) = extended_epid_group_id;
        
        fprintf(OUTPUT, "\nMSG0 body generated -\n");
        PRINT_BYTE_ARRAY(OUTPUT, p_msg0_full->body, p_msg0_full->size);
        
        //send MSG0 to the SP
        fprintf(OUTPUT, "\n>>>> Sending msg0 to SP.\n");

        ret = ra_network_send_receive("url example", 
                                        p_msg0_full, 
                                        &p_msg0_resp_full);
        //now the ISV decides whether to support this extended epid group id or not
        if (ret != 0)
        {
            fprintf(OUTPUT, "\nError, ra_network_send_receive for msg0 failed. [%s]", 
                    __FUNCTION__);
            goto CLEANUP;
        }
        
        fprintf(OUTPUT, "\nSent MSG0 to SP.\n");
        //see what the response is:
        fprintf(OUTPUT, "\nTrying to find out what the response MSG 0 is: -\n");
        PRINT_BYTE_ARRAY(OUTPUT, p_msg0_resp_full->body, p_msg0_resp_full->size);
        
        fprintf(OUTPUT, "\n>>>> Selecting attestation key.\n");
        ret = sgx_select_att_key_id(p_msg0_resp_full->body, 
                                    p_msg0_resp_full->size, 
                                    &selected_key_id);
        if(ret != SGX_SUCCESS)
        {
            ret = -1;
            fprintf(OUTPUT, "\nInfo, call sgx_select_att_key_id fail, current platform configuration doesn't support this attestation key ID. [%s]\n",
                    __FUNCTION__);
            goto CLEANUP;
        }
        fprintf(OUTPUT, "\nCall sgx_select_att_key_id success.\n");
    }
    
    printf("\nEnter a character before exit ...\n");
    getchar();
    return ret;


CLEANUP:
    // Clean-up
    fprintf(OUTPUT, "\n>>>> Something went wrong, starting Cleanup process.");

    // Need to close the RA key state.
    if(context != INT_MAX)
    {   
        int ret_save = ret;
        ret = enclave_ra_close(enclave_id, 
                                &status, 
                                context);
        if(ret != SGX_SUCCESS || status)
        {
            ret = -1;
            fprintf(OUTPUT, "\nError, call enclave_ra_close fail [%s].",
                    __FUNCTION__);
        }
        else
        {
            // enclave_ra_close was successful, let's restore the value that
            // led us to this point in the code.
            ret = ret_save;
        }
        fprintf(OUTPUT, "\nCall enclave_ra_close success.");
    }

    sgx_destroy_enclave(enclave_id);


    ra_free_network_response_buffer(p_msg0_resp_full);
    p_msg0_resp_full = NULL;
    
    SAFE_FREE(p_msg0_full);

}