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

#ifndef SAFE_FREE
#define SAFE_FREE(ptr) {if (NULL != (ptr)) {free(ptr); (ptr) = NULL;}}
#endif

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

        ret = ra_network_send_receive("some url", 
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

        {
            // ISV application creates the ISV enclave.
            do
            {   
                fprintf(OUTPUT, "\n>>>> Creating the enclave.\n");
                ret = sgx_create_enclave(ENCLAVE_NAME,
                                            SGX_DEBUG_FLAG,
                                            &token,
                                            &token_updated,
                                            &enclave_id, 0);
                if(ret != SGX_SUCCESS)
                {
                    ret = -1;
                    fprintf(OUTPUT, "\nError, call sgx_create_enclave fail. [%s]\n",
                            __FUNCTION__);
                    goto CLEANUP;
                }
                fprintf(OUTPUT, "\nCall sgx_create_enclave success.\n");

                fprintf(OUTPUT, "\n>>>> Starting RA process.\n");
                ret = enclave_init_ra(enclave_id,
                                        &status,
                                        false,
                                        &context);
            //Ideally, this check would be around the full attestation flow.
            } while (ret == SGX_ERROR_ENCLAVE_LOST && enclave_lost_retry_time--);

            if(ret != SGX_SUCCESS || status)
            {
                ret = -1;
                fprintf(OUTPUT, "\nError, call enclave_init_ra fail. [%s]\n",
                        __FUNCTION__);
                goto CLEANUP;
            }
            fprintf(OUTPUT, "\nCall enclave_init_ra success.\n");
            
            fprintf(OUTPUT, "\n>>>> Start generating MSG1.\n");
            // application call uke sgx_ra_get_msg1
            p_msg1_full = (ra_samp_request_header_t*)malloc( sizeof(ra_samp_request_header_t) + sizeof(sgx_ra_msg1_t) );
            if(p_msg1_full == NULL)
            {
                ret = -1;
                goto CLEANUP;
            }
            p_msg1_full->type = TYPE_RA_MSG1;
            p_msg1_full->size = sizeof(sgx_ra_msg1_t);

            /*  ### Start preparing MSG1 ###

            What do we need?
            - extended EPID GID

            MSG1 = [ g_a || EPID GID ]

            g_a comes in the sgx_ra_get_msg1_ex
            */

            do
            {
                ret = sgx_ra_get_msg1_ex(&selected_key_id, 
                                            context, 
                                            enclave_id, 
                                            sgx_ra_get_ga,
                                            (sgx_ra_msg1_t*)( (uint8_t*)p_msg1_full + sizeof(ra_samp_request_header_t) ));
                sleep(3); // Wait 3s between retries
            } while (ret == SGX_ERROR_BUSY && busy_retry_time--);

            if(ret != SGX_SUCCESS )
            {
                ret = -1;
                fprintf(OUTPUT, "\nError, call sgx_ra_get_msg1_ex fail [%s].",
                        __FUNCTION__);
                goto CLEANUP;
            }
            else
            {
                fprintf(OUTPUT, "\nCall sgx_ra_get_msg1_ex success.\n");

                fprintf(OUTPUT, "\nMSG1 body generated -\n");
                PRINT_BYTE_ARRAY(OUTPUT, p_msg1_full->body, p_msg1_full->size);
            }

            // The ISV application sends msg1 to the SP to get msg2,
            // msg2 needs to be freed when no longer needed.
            // The ISV decides whether to use linkable or unlinkable signatures.

            fprintf(OUTPUT, "\n>>>> Sending msg1 to SP. " 
                            "Expecting msg2 back.\n");

            ret = ra_network_send_receive("some url",
                                            p_msg1_full,
                                            &p_msg2_full);

            if(ret != 0 || !p_msg2_full)
            {
                fprintf(OUTPUT, "\nError, ra_network_send_receive for msg1 failed [%s].", 
                        __FUNCTION__);
                goto CLEANUP;
            }
            else
            {
                // Successfully sent msg1 and received a msg2 back.
                // Time now to check msg2.
                if(p_msg2_full->type != TYPE_RA_MSG2)
                {
                    fprintf(OUTPUT, "\nError, didn't get MSG2 in response to MSG1 [%s].", 
                            __FUNCTION__);
                    goto CLEANUP;
                }
            }

            fprintf(OUTPUT, "\nSent MSG1 to remote attestation service "
                                    "provider. Received the following MSG2:\n");
            PRINT_BYTE_ARRAY(OUTPUT, p_msg2_full, (uint32_t)sizeof(ra_samp_response_header_t) + p_msg2_full->size);
        
            // prepare msg2->body
            sgx_ra_msg2_t* p_msg2_body = (sgx_ra_msg2_t*)((uint8_t*)p_msg2_full + sizeof(ra_samp_response_header_t));

            uint32_t msg3_size = 0;

            busy_retry_time = 2;
            // The ISV app now calls uKE sgx_ra_proc_msg2,
            // The ISV app is responsible for freeing the returned p_msg3!!
            do
            {
                ret = sgx_ra_proc_msg2_ex(&selected_key_id,
                                    context,
                                    enclave_id,
                                    sgx_ra_proc_msg2_trusted,
                                    sgx_ra_get_msg3_trusted,
                                    p_msg2_body,
                                    p_msg2_full->size,
                                    &p_msg3,
                                    &msg3_size);
            } while (SGX_ERROR_BUSY == ret && busy_retry_time--);

            if(!p_msg3)
            {
                fprintf(OUTPUT, "\nError, call sgx_ra_proc_msg2_ex fail. "
                                "p_msg3 = 0x%p [%s].", p_msg3, __FUNCTION__);
                ret = -1;
                goto CLEANUP;
            }
            if(SGX_SUCCESS != (sgx_status_t)ret)
            {
                fprintf(OUTPUT, "\nError, call sgx_ra_proc_msg2_ex fail. "
                                "ret = 0x%08x [%s].", ret, __FUNCTION__);
                ret = -1;
                goto CLEANUP;
            }
            else
            {
                fprintf(OUTPUT, "\nCall sgx_ra_proc_msg2_ex success.\n");
                fprintf(OUTPUT, "\nMSG3 - \n");
            }

            PRINT_BYTE_ARRAY(OUTPUT, p_msg3, msg3_size);
            /* 
            p_msg3_full = (ra_samp_request_header_t*)malloc(
                            sizeof(ra_samp_request_header_t) + msg3_size);
            if(p_msg3_full == NULL)
            {
                ret = -1;
                goto CLEANUP;
            }
            p_msg3_full->type = TYPE_RA_MSG3;
            p_msg3_full->size = msg3_size;
            if(memcpy_s(p_msg3_full->body, msg3_size, p_msg3, msg3_size))
            {
                fprintf(OUTPUT,"\nError: INTERNAL ERROR - memcpy failed in [%s].",
                        __FUNCTION__);
                ret = -1;
                goto CLEANUP;
            }

            // The ISV application sends msg3 to the SP to get the attestation
            // result message, attestation result message needs to be freed when
            // no longer needed. The ISV service provider decides whether to use
            // linkable or unlinkable signatures. The format of the attestation
            // result is up to the service provider. This format is used for
            // demonstration.  Note that the attestation result message makes use
            // of both the MK for the MAC and the SK for the secret. These keys are
            // established from the SIGMA secure channel binding.
            ret = ra_network_send_receive("some url",
                                            p_msg3_full,
                                            &p_att_result_msg_full);
            if(ret || !p_att_result_msg_full)
            {
                ret = -1;
                fprintf(OUTPUT, "\nError, sending msg3 failed [%s].", __FUNCTION__);
                goto CLEANUP;
            }

            sample_ra_att_result_msg_t * p_att_result_msg_body =
                    (sample_ra_att_result_msg_t *)((uint8_t*)p_att_result_msg_full
                                                + sizeof(ra_samp_response_header_t));
            if(TYPE_RA_ATT_RESULT != p_att_result_msg_full->type)
            {
                ret = -1;
                fprintf(OUTPUT, "\nError. Sent MSG3 successfully, but the message "
                                "received was NOT of type att_msg_result. Type = "
                                "%d. [%s].", p_att_result_msg_full->type,
                                __FUNCTION__);
                goto CLEANUP;
            }
            else
            {
                fprintf(OUTPUT, "\nSent MSG3 successfully. Received an attestation "
                                "result message back\n.");
            }

            fprintf(OUTPUT, "\nATTESTATION RESULT RECEIVED - ");
            PRINT_BYTE_ARRAY(OUTPUT, p_att_result_msg_full->body,
                            p_att_result_msg_full->size);

            // Check the MAC using MK on the attestation result message.
            // The format of the attestation result message is ISV specific.
            // This is a simple form for demonstration. In a real product,
            // the ISV may want to communicate more information.
            ret = verify_att_result_mac(enclave_id,
                    &status,
                    context,
                    (uint8_t*)&p_att_result_msg_body->platform_info_blob,
                    sizeof(ias_platform_info_blob_t),
                    (uint8_t*)&p_att_result_msg_body->mac,
                    sizeof(sgx_mac_t));
            if((SGX_SUCCESS != ret) ||
            (SGX_SUCCESS != status))
            {
                ret = -1;
                fprintf(OUTPUT, "\nError: INTEGRITY FAILED - attestation result "
                                "message MK based cmac failed in [%s].",
                                __FUNCTION__);
                goto CLEANUP;
            }

            bool attestation_passed = true;
            // Check the attestation result for pass or fail.
            // Whether attestation passes or fails is a decision made by the ISV Server.
            // When the ISV server decides to trust the enclave, then it will return success.
            // When the ISV server decided to not trust the enclave, then it will return failure.
            if(0 != p_att_result_msg_full->status[0]
            || 0 != p_att_result_msg_full->status[1])
            {
                fprintf(OUTPUT, "\nError, attestation result message MK based cmac "
                                "failed in [%s].", __FUNCTION__);
                attestation_passed = false;
            }

            // Get the shared secret sent by the server using SK (if attestation
            // passed)
            if(attestation_passed)
            {
                ret = put_secret_data(enclave_id,
                                    &status,
                                    context,
                                    p_att_result_msg_body->secret.payload,
                                    p_att_result_msg_body->secret.payload_size,
                                    p_att_result_msg_body->secret.payload_tag);
                if((SGX_SUCCESS != ret)  || (SGX_SUCCESS != status))
                {
                    fprintf(OUTPUT, "\nError, attestation result message secret "
                                    "using SK based AESGCM failed in [%s]. ret = "
                                    "0x%0x. status = 0x%0x", __FUNCTION__, ret,
                                    status);
                    goto CLEANUP;
                }
            }
            fprintf(OUTPUT, "\nSecret successfully received from server.");
            fprintf(OUTPUT, "\nRemote attestation success!"); */
        
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
    ra_free_network_response_buffer(p_msg2_full);
    p_msg2_full = NULL;
    ra_free_network_response_buffer(p_att_result_msg_full);
    p_att_result_msg_full = NULL;

    // p_msg3 is malloc'd by the untrusted KE library. App needs to free.
    SAFE_FREE(p_msg3);
    SAFE_FREE(p_msg3_full);
    SAFE_FREE(p_msg1_full);
    SAFE_FREE(p_msg0_full);

}
}