#include "state_machine.h"
#include "file.h"
#include "crc.h"
#include "socket.h"
#include "cli.h"
#include "packet.h"

#include <stdbool.h>

/*******************************************************************************
* Cleint state machines
*******************************************************************************/
void sm_client_put()
{
    int ret;
    int transmit_complete = 0;
    uint32_t crc32_calc;

    state_t previous_state = null_t;
    state_t current_state = sendCmd_t;

    event_t event = evtNull_t;
    uint32_t sequence_number = 0;

    while(true)
    {
        switch(current_state)
        {
            case(sendCmd_t):
                // 1. generate filtered version of user commands
                cli_generate_filtered_usr_cmd("put", cli_get_user_param_buf());

                // 2. fill packet struct fields
                packet_write_sequence_number(0);
                packet_write_payload_size(cli_get_filtered_usr_cmd_size());
                    packet_write_payload(
                        cli_get_filtered_usr_cmd(), 
                        packet_get_payload_size()
                    );
                    packet_write_crc32(
                        crc_generate(
                            packet_get_buf(), 
                            packet_get_packet_size_for_crc()
                        )
                    );

                // 3. generate packet buffer
                ret = packet_generate();
                printf("====\n");
                packet_print_struct();

                // Open file to send
                file_open(cli_get_user_param_buf(), 0);
                printf("%s size (bytes): %d\n", cli_get_user_param_buf(), file_get_size());

                if(ret > 0)
                {
                    event = evtAckNotRecv_t;
                    previous_state = sendCmd_t;
                    current_state = waitAck_t;
                }
                break;

            case(transmitPayload_t):
                // printf("In transmit payload\n");

                if(!transmit_complete)
                {
                    // Read a chunk from file
                    ret = file_read_chunk(packet_get_chunk_size());
                    printf("file read returned %d bytes\n", ret);
                    if(ret != packet_get_chunk_size())
                    {
                        transmit_complete = 1;
                    }

                    // fill packet struct fields
                    packet_write_sequence_number(sequence_number);
                    packet_write_payload_size(ret);
                    packet_write_payload(
                        file_get_file_buf(), 
                        packet_get_payload_size()
                    );
                    packet_write_crc32(
                        crc_generate(
                            packet_get_buf(), 
                            packet_get_packet_size_for_crc()
                        )
                    );

                    // generate packet buffer
                    ret = packet_generate();
                    // packet_print_struct();

                    sequence_number++;

                    // prepare to transition to next state
                    event = evtAckNotRecv_t;
                    previous_state = transmitPayload_t;
                    current_state = waitAck_t;
                }
                else
                {
                    file_close();

                    // generate ACK packet.
                    printf("Generating ACK packet\n");
                    packet_write_sequence_number(0);
                    packet_write_payload_size(sizeof("ACK"));
                    packet_write_payload("ACK", packet_get_payload_size());
                    packet_write_crc32(
                        crc_generate(
                            packet_get_buf(), 
                            packet_get_packet_size_for_crc()
                        )
                    );
                    packet_generate();

                    event = evtAckNotRecv_t;
                    previous_state = sendAck_t;
                    current_state = waitAck_t;
                }
                break;


            case(waitAck_t):
                
                // 1. Attempt to send packet and wait for response.
                //    If no response is received within the timeout period
                //    attempt to transmit again. Repeat until ACK is received.
                ret = 0;

                // loop will terminate when ACK has been received.
                while(event != evtAckRecv_t)
                {
                    // break out of loop if some kind of response is received.
                    ret = 0;
                    sock_clear_input_buffer();
                    while(ret <= 0)
                    {
                        // send packet to server
                        ret = sock_sendto(packet_get_buf(), packet_get_total_size(), true);
                        printf("Sent PACKET\n");

                        // wait for any kind of response from server
                        // printf("Waiting for ACK from server\n");

                        socket_init_timeout();
                        sock_enable_timeout();
                        ret = sock_recv(1);
                        sock_disable_timeout();
                    }

                    // parse the received message
                    packet_parse(sock_get_in_buf());

                    // verify that payload contents are correct (crc32 check)
                    crc32_calc = crc_generate(
                        packet_get_buf(), 
                        packet_get_packet_size_for_crc()
                    );
                    if(crc32_calc != packet_get_crc32())
                    {
                        printf("CRC32 mismatch, corrupted packet!");
                        continue;
                    }

                    // check that the payload is ACK
                    if ( strcmp(packet_get_payload(), "ACK") != 0 )
                    {
                        // ack was not received. start over.
                        printf("ACK NOT RECEIVED\n");
                    }
                    else
                    {
                        printf("Got ACK\n");
                        event = evtAckRecv_t;
                    }
                }

                // if we were sending the final ack, go to log info,
                // otherwise, prepare to send another payload packet.
                if(previous_state == sendAck_t)
                {
                    previous_state = waitAck_t;
                    current_state = logFileInfo_t;
                }
                else if (previous_state == sendCmd_t)
                {
                    previous_state = waitAck_t;
                    current_state = transmitPayload_t;
                }
                else if(previous_state == transmitPayload_t)
                {
                    previous_state = waitAck_t;
                    current_state = transmitPayload_t;
                }

                break;

            case(logFileInfo_t):
                printf("===PRINTING FILE STATS===\n");
                file_open(cli_get_user_param_buf(), 0);
                printf("%s size (bytes): %d\n", cli_get_user_param_buf(), file_get_size());

                // Generate the complete files CRC32
                crc32_calc = crc_generate_file(file_get_fp());
                printf("File CRC: %u\n", crc32_calc);
                file_close();
                return;

                break;

            default:
                printf("State machine error\n");
                break;
        }
    }
}

/*******************************************************************************
* Server state machines
*******************************************************************************/
void sm_server_put()
{
    int ret;
    // int transmit_complete = 0;
    uint32_t crc32_calc;
    uint32_t last_sequence_number;

    state_t previous_state = null_t; if(previous_state != null_t) {};
    state_t current_state = sendAck_t;

    event_t event = evtNull_t;

    printf("Performing PUT command with param \"%s\"\n", cli_get_user_param_buf());
    file_open(cli_get_user_param_buf(), 1);

    while(true)
    {
        switch(current_state)
        {
            case(sendAck_t):
                // generate ACK packet.
                // printf("Generating ACK packet\n");
                packet_write_sequence_number(0);
                packet_write_payload_size(sizeof("ACK"));
                packet_write_payload("ACK", packet_get_payload_size());
                packet_write_crc32(
                    crc_generate(
                        packet_get_buf(), 
                        packet_get_packet_size_for_crc()
                    )
                );
                ret = packet_generate();
                printf("======SENDING: ACK\n");
                packet_print_struct();

                // send ACK packet 
                printf("Sending ACK\n");
                ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                // we were just sending the final ACK, we are done
                if(event == evtFileTransComplete_t)
                {
                    //printf("sendAck: evtFileTransComplete\n");
                    previous_state = sendAck_t;
                    current_state = logFileInfo_t; 
                }
                else if(event == evtPayloadReceived_t)
                {
                    //printf("sendAck: evtPayloadReceived\n");
                    // wait for next payload
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;    
                }
                else if (event == evtNull_t)
                {
                    //printf("sendAck: evtNull_t\n");
                    event = evtFileTransNotComplete_t;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;
                }

                // printf("prev state %d\n", previous_state);
                // printf("state = %d\n", current_state);
                // printf("event = %d\n", event);

                break;

            case(waitPayload_t):
                // loop until a packet is received
                ret = 0;
                while(ret <= 0)
                {
                    // wait for any kind of response from server
                    sock_clear_input_buffer();
                    printf("Waiting for PAYLOAD from server\n");
                    socket_init_timeout();
                    sock_enable_timeout();
                    ret = sock_recv(1);
                    sock_disable_timeout();
                }

                // parse packet
                packet_parse(sock_get_in_buf());
                printf("======PAYLOAD\n");
                packet_print_struct();

                // verify correct payload
                crc32_calc = crc_generate(
                    packet_get_buf(), 
                    packet_get_packet_size_for_crc()
                );
                if(crc32_calc != packet_get_crc32())
                {
                    // data is corrupted, go back to step 1.
                    printf("CRC32 mismatch, corrupted packet!\n");
                    printf("PACKET CRC = %u | CALC CRC32 = %u\n", packet_get_crc32(), crc32_calc);
                    continue;
                }

                // check if we got a command, if so, re ACK the message
                if(strstr(packet_get_payload(), "put") != NULL)
                {
                    event = evtNull_t;
                    previous_state = waitPayload_t;
                    current_state = sendAck_t;
                }
                // Check if we got an ACK packet (means end of transmission)
                else if ( strcmp(packet_get_payload(), "ACK") == 0 )
                {
                    printf("ACK Received!\nEOF\n");
                    event = evtFileTransComplete_t;
                    previous_state = waitPayload_t;
                    current_state = sendAck_t;
                    // transmit_complete = 1;
                    file_close();
                }
                // we got the same packet as before. don't save and just send an ACK
                else if(last_sequence_number == packet_get_sequence_number())
                {
                    event = evtPayloadReceived_t;
                    previous_state = waitPayload_t;
                    current_state = sendAck_t;
                }
                // otherwise, we got a normal payload and carry on saving
                else
                {
                    last_sequence_number = packet_get_sequence_number();
                    event = evtPayloadReceived_t;
                    previous_state = waitPayload_t;
                    current_state = savePayload_t;
                }


                break;

            case(savePayload_t):
                // save payload to file
                ret = file_write_chunk(
                    packet_get_payload(), 
                    packet_get_payload_size()
                );
                if(ret != packet_get_payload_size())
                {
                    printf("ERROR: Not all bytes written to file.\n");
                    event = evtPayloadNotReceived_t;
                    previous_state = savePayload_t;
                    current_state = waitPayload_t;
                }
                else
                {
                    event = evtPayloadReceived_t;
                    previous_state = savePayload_t;
                    current_state = sendAck_t;   
                }

                break;

            case(logFileInfo_t):
                printf("===PRINTING FILE STATS===\n");
                file_open(cli_get_user_param_buf(), 0);
                printf("%s size (bytes): %d\n", cli_get_user_param_buf(), file_get_size());

                // Generate the complete files CRC32
                crc32_calc = crc_generate_file(file_get_fp());
                printf("File CRC: %u\n", crc32_calc);
                file_close();
                return;

                break;

            default:
                printf("State machine error\n");
                break;
        }
    }
}