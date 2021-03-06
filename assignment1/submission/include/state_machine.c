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
void sm_client_get()
{
    int ret;
    // int transmit_complete = 0;
    uint32_t crc32_calc;
    uint32_t last_sequence_number;
    uint32_t total_packets = 0;

    state_t previous_state = null_t;
    state_t current_state = sendCmd_t;

    event_t event = evtNull_t;
    uint32_t sequence_number = 0;
    uint32_t last_sequece_number = 0;

    while(true)
    {
        switch(current_state)
        {
            case(sendCmd_t):
                // 1. generate filtered version of user commands
                cli_generate_filtered_usr_cmd("get", cli_get_user_param_buf());

                // 2. fill packet struct fields
                packet_write_sequence_number(0);
                packet_write_payload_size(cli_get_filtered_usr_cmd_size());
                packet_write_payload(
                    cli_get_filtered_usr_cmd(), 
                    packet_get_payload_size()
                );
                packet_write_crc32(
                    crc_generate(
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );

                // 3. generate packet buffer
                ret = packet_generate();

                // Open file to write to
                file_open(cli_get_user_param_buf(), 1);

                if(ret > 0)
                {
                    event = evtAckNotRecv_t;
                    previous_state = sendCmd_t;
                    current_state = waitAck_t;
                }
                break;

            case(waitPayload_t):
                // printf("In wait payload\n");
                // loop until a packet is received
                ret = 0;
                while(ret <= 0)
                {
                    // wait for any kind of response from server
                    sock_clear_input_buffer();
                    //printf("Waiting for PAYLOAD from server\n");
                    socket_init_timeout();
                    sock_enable_timeout();
                    ret = sock_recv(1);
                    sock_disable_timeout();
                }

                // parse packet
                packet_parse(sock_get_in_buf());
                // printf("======PAYLOAD\n");
                // packet_print_struct();

                // verify correct payload
                crc32_calc = crc_generate(
                    (char *) packet_get_struct(), 
                    packet_get_packet_size_for_crc()
                );
                if(crc32_calc != packet_get_crc32())
                {
                    // data is corrupted, go back to step 1.
                    printf("CRC32 mismatch, corrupted packet!\n");
                    printf("PACKET CRC = %u | CALC CRC32 = %u\n", packet_get_crc32(), crc32_calc);
                    continue;
                }

                if ( strcmp(packet_get_payload(), "ACK") == 0 )
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
                    printf("repeat sequence number\n");
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
                        printf("Sent CMD\n");

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
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    );
                    if(crc32_calc != packet_get_crc32())
                    {
                        printf("CRC32 mismatch, corrupted packet!\n");
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
                // we just sent the get command, now get ready to receive
                else if (previous_state == sendCmd_t)
                {
                    previous_state = waitAck_t;
                    current_state = waitPayload_t;
                }
                else if(previous_state == transmitPayload_t)
                {
                    sequence_number++;
                    last_sequece_number = sequence_number;
                    previous_state = waitAck_t;
                    current_state = transmitPayload_t;
                }
                else
                {
                    printf("HUH\n");
                }

                break;

            case(savePayload_t):
                // save payload to file
                // printf("GOING TO WRITE TO FILE\n");
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
                    // printf("Wrote %d bytes to file\n", ret);
                    event = evtPayloadReceived_t;
                    previous_state = savePayload_t;
                    current_state = sendAck_t;   
                }

                break;

            case(sendAck_t):
                // generate ACK packet.
                // printf("Generating ACK packet\n");
                packet_write_sequence_number(0);
                packet_write_payload_size(sizeof("ACK"));
                packet_write_payload("ACK", packet_get_payload_size());
                packet_write_crc32(
                    crc_generate(
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );
                ret = packet_generate();
                // printf("======SENDING: ACK\n");
                // packet_print_struct();

                // send ACK packet 
                //printf("Sending ACK\n");
                ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                // we were just sending the final ACK, we are done
                if(event == evtFileTransComplete_t)
                {
                    //printf("sendAck: evtFileTransComplete\n");
                    previous_state = sendAck_t;
                    current_state = logFileInfo_t; 
                }
                // we have just sucessfully received a packet
                else if(event == evtPayloadReceived_t)
                {
                    //printf("sendAck: evtPayloadReceived\n");
                    // wait for next payload
                    total_packets++;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;    
                }
                // we just started, wait for first payload packet.
                else if (event == evtNull_t)
                {
                    //printf("sendAck: evtNull_t\n");
                    event = evtFileTransNotComplete_t;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;
                }
                else
                {
                    printf("HMM\n");
                }

                break;

            case(logFileInfo_t):
                printf("===PRINTING FILE STATS===\n");
                printf("Last sequence number: %d\n", last_sequece_number);
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
void sm_client_put()
{
    int ret;
    int transmit_complete = 0;
    uint32_t crc32_calc;

    state_t previous_state = null_t;
    state_t current_state = sendCmd_t;

    event_t event = evtNull_t;
    uint32_t sequence_number = 0;
    uint32_t last_sequece_number = 0;

    file_open(cli_get_user_param_buf(), 0);
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
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );

                // 3. generate packet buffer
                ret = packet_generate();
                // printf("====SENDING: CMD\n");
                // packet_print_struct();

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
                    // printf("file read returned %d bytes\n", ret);
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
                            (char *) packet_get_struct(), 
                            packet_get_packet_size_for_crc()
                        )
                    );

                    // generate packet buffer
                    ret = packet_generate();
                    // printf("====SENDING: PACKET");
                    // packet_print_struct();

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
                            (char *) packet_get_struct(), 
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
                        //printf("Sent PACKET\n");

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
                        (char *) packet_get_struct(), 
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
                        // printf("Got ACK\n");
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
                    sequence_number++;
                    last_sequece_number = sequence_number;
                    previous_state = waitAck_t;
                    current_state = transmitPayload_t;
                }
                else
                {
                    printf("HUH\n");
                }

                break;

            case(logFileInfo_t):
                printf("===PRINTING FILE STATS===\n");
                printf("Last sequence number: %d\n", last_sequece_number);
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
void sm_client_ls()
{
    int ret;
    // int transmit_complete = 0;
    uint32_t crc32_calc;
    uint32_t total_packets = 0;

    state_t previous_state = null_t;
    state_t current_state = sendCmd_t;

    event_t event = evtNull_t;
    uint32_t sequence_number = 0;
    uint32_t last_sequence_number = 0xFFFF;

    while(true)
    {
        switch (current_state)
        {
            case sendCmd_t:
                // 1. generate filtered version of user commands
                cli_generate_filtered_usr_cmd("ls", cli_get_user_param_buf());

                // 2. fill packet struct fields
                packet_write_sequence_number(0);
                packet_write_payload_size(cli_get_filtered_usr_cmd_size());
                packet_write_payload(
                    cli_get_filtered_usr_cmd(), 
                    packet_get_payload_size()
                );
                packet_write_crc32(
                    crc_generate(
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );

                // 3. generate packet buffer
                ret = packet_generate();

                if(ret > 0)
                {
                    event = evtAckNotRecv_t;
                    previous_state = sendCmd_t;
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
                        // printf("Sent CMD\n");

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
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    );
                    if(crc32_calc != packet_get_crc32())
                    {
                        printf("CRC32 mismatch, corrupted packet!\n");
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
                        // printf("Got ACK\n");
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
                // we just sent the get command, now get ready to receive
                else if (previous_state == sendCmd_t)
                {
                    previous_state = waitAck_t;
                    current_state = waitPayload_t;
                }
                else if(previous_state == transmitPayload_t)
                {
                    sequence_number++;
                    last_sequence_number = sequence_number;
                    previous_state = waitAck_t;
                    current_state = transmitPayload_t;
                }
                else
                {
                    printf("Error in waitAck\n");
                }

                break;

            case(waitPayload_t):
                // printf("In wait payload\n");
                // loop until a packet is received
                ret = 0;
                while(ret <= 0)
                {
                    // wait for any kind of response from server
                    sock_clear_input_buffer();
                    // printf("Waiting for PAYLOAD from server\n");
                    socket_init_timeout();
                    sock_enable_timeout();
                    ret = sock_recv(1);
                    sock_disable_timeout();
                }

                // parse packet
                packet_parse(sock_get_in_buf());
                // printf("======PAYLOAD\n");
                // packet_print_struct();

                // verify correct payload
                crc32_calc = crc_generate(
                    (char *) packet_get_struct(), 
                    packet_get_packet_size_for_crc()
                );
                if(crc32_calc != packet_get_crc32())
                {
                    // data is corrupted, go back to step 1.
                    printf("CRC32 mismatch, corrupted packet!\n");
                    printf("PACKET CRC = %u | CALC CRC32 = %u\n", packet_get_crc32(), crc32_calc);
                    continue;
                }

                // check if we got an ACK
                if ( strcmp(packet_get_payload(), "ACK") == 0 )
                {
                    // printf("ACK Received!\nEOF\n");
                    event = evtFileTransComplete_t;
                    previous_state = waitPayload_t;
                    return;
                    // transmit_complete = 1;
                }

                // we got the same packet as before. don't save and just send an ACK
                else if(last_sequence_number == packet_get_sequence_number())
                {
                    printf("Repeat sequence number.\n");
                    event = evtPayloadReceived_t;
                    previous_state = waitPayload_t;
                    current_state = sendAck_t;
                }

                // otherwise, we got a normal payload, display the payload next
                else
                {
                    last_sequence_number = packet_get_sequence_number();
                    event = evtPayloadReceived_t;
                    previous_state = waitPayload_t;
                    current_state = displayPayload_t;
                }
                break;

            case(displayPayload_t):
            printf("Files on SERVER @ local directory:\n");
                file_print_ls_buf(packet_get_payload());
                event = evtPayloadReceived_t;
                previous_state = displayPayload_t;
                current_state = sendAck_t;
                break;

            case(sendAck_t):
                // generate ACK packet.
                // printf("Generating ACK packet\n");
                packet_write_sequence_number(0);
                packet_write_payload_size(sizeof("ACK"));
                packet_write_payload("ACK", packet_get_payload_size());
                packet_write_crc32(
                    crc_generate(
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );
                ret = packet_generate();
                // printf("======SENDING: ACK\n");
                // packet_print_struct();

                // send ACK packet 
                //printf("Sending ACK\n");
                ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                // we were just sending the final ACK, we are done
                if(event == evtFileTransComplete_t)
                {
                    //printf("sendAck: evtFileTransComplete\n");
                    previous_state = sendAck_t;
                    return;
                }
                // we have just sucessfully received a packet
                else if(event == evtPayloadReceived_t)
                {
                    //printf("sendAck: evtPayloadReceived\n");
                    // wait for next payload
                    total_packets++;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;    
                }
                // we just started, wait for first payload packet.
                else if (event == evtNull_t)
                {
                    //printf("sendAck: evtNull_t\n");
                    event = evtFileTransNotComplete_t;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;
                }
                else
                {
                    printf("Error in sendAck\n");
                }

                break;

            default:
                break;
        }
    }

}
/*******************************************************************************
* Server state machines
*******************************************************************************/
void sm_server_get()
{
    // we are sending the file in this case.
    int ret;
    int final_ack = 0;
    int transmit_complete = 0;
    uint32_t crc32_calc;
    uint32_t total_packets = 0;

    state_t previous_state = null_t;
    state_t current_state = sendAck_t;

    event_t event = evtNull_t;
    uint32_t sequence_number = 0;
    uint32_t last_sequece_number = 0;

    file_open(cli_get_user_param_buf(), 0);
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
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );
                ret = packet_generate();
                // printf("======SENDING: ACK\n");
                // packet_print_struct();

                // send ACK packet 
                //printf("Sending ACK\n");
                ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                // we were just sending the final ACK, we are done
                if(event == evtFileTransComplete_t)
                {
                    //printf("sendAck: evtFileTransComplete\n");
                    previous_state = sendAck_t;
                    current_state = logFileInfo_t; 
                }
                // we have just sucessfully received a packet
                else if(event == evtPayloadReceived_t)
                {
                    //printf("sendAck: evtPayloadReceived\n");
                    // wait for next payload
                    total_packets++;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;    
                }
                // we just started, wait for first payload packet.
                else if (event == evtNull_t)
                {
                    //printf("sendAck: evtNull_t\n");
                    event = evtFileTransNotComplete_t;
                    previous_state = sendAck_t;
                    current_state = transmitPayload_t;
                }
                else
                {
                    printf("HMM\n");
                }

                break;

            case(transmitPayload_t):
                // printf("In transmit payload\n");
        
                if(!transmit_complete)
                {
                    // Read a chunk from file
                    ret = file_read_chunk(packet_get_chunk_size());
                    // printf("file read returned %d bytes\n", ret);
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
                            (char *) packet_get_struct(), 
                            packet_get_packet_size_for_crc()
                        )
                    );

                    // generate packet buffer
                    ret = packet_generate();
                    // printf("====SENDING: PACKET");
                    // packet_print_struct();

                    // prepare to transition to next state
                    event = evtAckNotRecv_t;
                    previous_state = transmitPayload_t;
                    current_state = waitAck_t;
                }
                else
                {
                    file_close();
                    // generate ACK packet.
                    // printf("Generating ACK packet\n");
                    packet_write_sequence_number(0);
                    packet_write_payload_size(sizeof("ACK"));
                    packet_write_payload("ACK", packet_get_payload_size());
                    packet_write_crc32(
                        crc_generate(
                            (char *) packet_get_struct(), 
                            packet_get_packet_size_for_crc()
                        )
                    );
                    packet_generate();

                    final_ack = 1;
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
                        ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);
                        //printf("Sent PACKET\n");

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
                        (char *) packet_get_struct(), 
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
                        // printf("Got ACK\n");
                        event = evtAckRecv_t;
                    }
                }

                // if we were sending the final ack, go to log info,
                // otherwise, prepare to send another payload packet.
                if(transmit_complete && final_ack)
                {
                    previous_state = waitAck_t;
                    current_state = logFileInfo_t;
                }
                else if(previous_state == transmitPayload_t)
                {
                    sequence_number++;
                    last_sequece_number = sequence_number;
                    previous_state = waitAck_t;
                    current_state = transmitPayload_t;
                }
                else
                {
                    printf("HUH\n");
                }

                break;

            case(logFileInfo_t):
                printf("===PRINTING FILE STATS===\n");
                printf("Last sequence number: %d\n", last_sequece_number);
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
void sm_server_put()
{
    int ret;
    // int transmit_complete = 0;
    uint32_t crc32_calc;
    uint32_t last_sequence_number;
    uint32_t total_packets = 0;

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
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );
                ret = packet_generate();
                // printf("======SENDING: ACK\n");
                // packet_print_struct();

                // send ACK packet 
                //printf("Sending ACK\n");
                ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                // we were just sending the final ACK, we are done
                if(event == evtFileTransComplete_t)
                {
                    //printf("sendAck: evtFileTransComplete\n");
                    previous_state = sendAck_t;
                    current_state = logFileInfo_t; 
                }
                // we have just sucessfully received a packet
                else if(event == evtPayloadReceived_t)
                {
                    //printf("sendAck: evtPayloadReceived\n");
                    // wait for next payload
                    total_packets++;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;    
                }
                // we just started, wait for first payload packet.
                else if (event == evtNull_t)
                {
                    //printf("sendAck: evtNull_t\n");
                    event = evtFileTransNotComplete_t;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;
                }
                else
                {
                    printf("HMM\n");
                }

                break;

            case(waitPayload_t):
                // loop until a packet is received
                ret = 0;
                while(ret <= 0)
                {
                    // wait for any kind of response from server
                    sock_clear_input_buffer();
                    //printf("Waiting for PAYLOAD from server\n");
                    socket_init_timeout();
                    sock_enable_timeout();
                    ret = sock_recv(1);
                    sock_disable_timeout();
                }

                // parse packet
                packet_parse(sock_get_in_buf());
                // printf("======PAYLOAD\n");
                // packet_print_struct();

                // verify correct payload
                crc32_calc = crc_generate(
                    (char *) packet_get_struct(), 
                    packet_get_packet_size_for_crc()
                );
                if(crc32_calc != packet_get_crc32())
                {
                    // data is corrupted, go back to step 1.
                    printf("CRC32 mismatch, corrupted packet!\n");
                    printf("PACKET CRC = %u | CALC CRC32 = %u\n", packet_get_crc32(), crc32_calc);
                    continue;
                }

                // // check if we got a command, if so, re ACK the message
                // if(strstr(packet_get_payload(), "put") != NULL)
                // {
                //     printf(">>>>FOUND A COMMAND\n");
                //     event = evtNull_t;
                //     previous_state = waitPayload_t;
                //     current_state = sendAck_t;
                // }
                // Check if we got an ACK packet (means end of transmission)
                if ( strcmp(packet_get_payload(), "ACK") == 0 )
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
                    printf("repeat sequence number\n");
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
                // printf("GOING TO WRITE TO FILE\n");
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
                    // printf("Wrote %d bytes to file\n", ret);
                    event = evtPayloadReceived_t;
                    previous_state = savePayload_t;
                    current_state = sendAck_t;   
                }

                break;

            case(logFileInfo_t):
                printf("===PRINTING FILE STATS===\n");
                printf("last sequence number received: %u\n", last_sequence_number);
                printf("total packets received: %u\n", total_packets);
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
void sm_server_ls()
{
    int ret;
    int final_ack = 0;
    int payload_size;
    int transmit_complete = 0;
    uint32_t crc32_calc;
    // uint32_t last_sequence_number;
    uint32_t total_packets = 0;

    state_t previous_state = null_t; if(previous_state != null_t) {};
    state_t current_state = sendAck_t;

    event_t event = evtNull_t;

    // printf("Performing LS command\n");

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
                        (char *) packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );
                ret = packet_generate();
                // printf("======SENDING: ACK\n");
                // packet_print_struct();

                // send ACK packet 
                // printf("Sending ACK\n");
                ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                // we just started, create first payload.
                if (event == evtNull_t)
                {
                    //printf("sendAck: evtNull_t\n");
                    event = evtFileTransNotComplete_t;
                    previous_state = sendAck_t;
                    current_state = transmitPayload_t;
                }
                // we were just sending the final ACK, we are done
                else if(event == evtFileTransComplete_t)
                {
                    //printf("sendAck: evtFileTransComplete\n");
                    final_ack = 1;
                    return;
                    previous_state = sendAck_t;
                    current_state = waitAck_t; 
                }
                // we have just sucessfully received a packet
                else if(event == evtPayloadReceived_t)
                {
                    //printf("sendAck: evtPayloadReceived\n");
                    // wait for next payload
                    total_packets++;
                    previous_state = sendAck_t;
                    current_state = waitPayload_t;    
                }

                else
                {
                    printf("HMM\n");
                }

                break;

            case(transmitPayload_t):

                if(!transmit_complete)
                {
                    // printf("Generating Payload\n");
                    // create the packet to send
                    if((payload_size = file_get_ls()) > 0)
                    {
                        if(payload_size < packet_get_chunk_size())
                        {  
                            packet_write_sequence_number(0);
                            packet_write_payload_size(payload_size);
                            packet_write_payload(file_get_file_buf(), packet_get_payload_size());
                            packet_write_crc32(
                                crc_generate(
                                    (char *) packet_get_struct(), 
                                    packet_get_packet_size_for_crc()
                                )
                            );
                            // packet_print_struct();
                            ret = packet_generate();  
                            transmit_complete = 1;
                        }
                        else
                        {
                            // payload is too big to send in one frame.
                            // will need to handle case to send in multiplie frames.
                        }
                    }
                    event = evtAckNotRecv_t;
                    previous_state = transmitPayload_t;
                    current_state = waitAck_t;
                }
                // we are done sending payloads, now send ACK to signal
                // end of transmission.
                else
                {
                    packet_write_sequence_number(0);
                    packet_write_payload_size(sizeof("ACK"));
                    packet_write_payload("ACK", packet_get_payload_size());
                    packet_write_crc32(
                        crc_generate(
                            (char *) packet_get_struct(), 
                            packet_get_packet_size_for_crc()
                        )
                    );
                    ret = packet_generate();
                    event = evtFileTransComplete_t;
                    previous_state = transmitPayload_t;
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
                        ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);
                        // printf("Sent PACKET\n");

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
                        (char *) packet_get_struct(), 
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
                        // printf("Got ACK\n");
                        event = evtAckRecv_t;
                    }
                }

                // if we were sending the final ack, go to log info,
                // otherwise, prepare to send another payload packet.
                if(transmit_complete)
                {
                    if(final_ack)
                    {
                        return;
                    }
                    event = evtFileTransComplete_t;
                    previous_state = waitAck_t;
                    current_state = sendAck_t;
                }
                else if(previous_state == transmitPayload_t)
                {
                    // sequence_number++;
                    // last_sequece_number = sequence_number;
                    previous_state = waitAck_t;
                    current_state = transmitPayload_t;
                }
                else
                {
                    printf("HUH\n");
                }

                break;

            default:
                printf("State machine error\n");
                break;
        }
    }  
}