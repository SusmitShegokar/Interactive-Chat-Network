#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <uuid/uuid.h>
// declerations
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define FAQS_FILE "FAQs.txt"

uuid_t active_uuids[MAX_CLIENTS];
int server_sock, client_sock, max_fd, activity;
struct sockaddr_in server_addr;
fd_set read_fds;
int client_sockets[MAX_CLIENTS];
char buffer[BUFFER_SIZE];
int num_clients = 0;
int chatbot_enabled[MAX_CLIENTS] = {0};
int chatbot_v2_enable[MAX_CLIENTS]={0};

// get FAQS
char *search_faq(char *question)
{
    FILE *faqs_file = fopen(FAQS_FILE, "r");
    if (faqs_file == NULL)
    {
        perror("[-]Error opening FAQs file");
        return "Error: FAQs file not found.";
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, faqs_file)) != -1)
    {
        char *q = strtok(line, "|||");
        char *a = strtok(NULL, "|||");
        if (q != NULL && a != NULL && strstr(question, q) != NULL)
        {
            fclose(faqs_file);
            return a;
        }
    }

    fclose(faqs_file);
    return "Sorry, I couldn't find an answer to your question.";
}
// save chats in files
void save_message_to_history(char *sender_uuid, char *recipient_uuid, char *message)
{
    FILE *history_file;
    char filename[100];
    snprintf(filename, sizeof(filename), "chat_history_%s_%s.txt", sender_uuid, recipient_uuid);
    history_file = fopen(filename, "a");
    if (history_file == NULL)
    {
        perror("[-]Error opening history file");
        return;
    }
    fprintf(history_file, "%s\n", message);
    fclose(history_file);
}
// history
void retrieve_chat_history(char *sender_uuid, char *recipient_uuid, int sockfd)
{
    FILE *history_file;
    char filename[100];
    char buffer[BUFFER_SIZE];
    snprintf(filename, sizeof(filename), "chat_history_%s_%s.txt", sender_uuid, recipient_uuid);
    history_file = fopen(filename, "r");
    if (history_file == NULL)
    {
        send(sockfd, "No chat history found.", strlen("No chat history found."), 0);
        return;
    }
    send(sockfd, "Chat history:\n", strlen("Chat history:\n"), 0);
    while (fgets(buffer, BUFFER_SIZE, history_file) != NULL)
    {
        send(sockfd, buffer, strlen(buffer), 0);
    }
    fclose(history_file);
}
// history_delete
void delete_chat_history(char *sender_uuid, char *recipient_uuid)
{
    char filename[100];
    printf("%s_%s", sender_uuid, recipient_uuid);
    snprintf(filename, sizeof(filename), "chat_history_%s_%s.txt", sender_uuid, recipient_uuid);
    if (remove(filename) != 0)
    {
        perror("Error deleting file");
    }
    else
    {
        printf("Chat history deleted successfully.\n");
    }
}
// delete_all
void delete_complete_chat_history(char *uuid)
{
    char filename[100];
    for (int j = 0; j < MAX_CLIENTS; j++)
    {
        char act_uuid[37];
        uuid_unparse(active_uuids[j], act_uuid);
        snprintf(filename, sizeof(filename), "chat_history_%s_%s.txt", uuid, act_uuid);
        if (remove(filename) != 0)
        {
            perror("Error deleting file");
        }
        else
        {
            printf("All Chat history deleted successfully.\n");
        }
    }
}

int main()
{
    // socket create bind and ready to listen
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5566);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    if (listen(server_sock, 5) < 0)
    {
        perror("[-]Listen error");
        exit(1);
    }

    printf("Listening for connections...\n");
    memset(client_sockets, 0, sizeof(client_sockets));
    FD_ZERO(&read_fds);
    FD_SET(server_sock, &read_fds);
    max_fd = server_sock;
    // for always awake.
    while (1)
    {
        fd_set tmp_fds = read_fds;
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(client_sockets[i], &read_fds);
            if (client_sockets[i] > max_fd)
            {
                max_fd = client_sockets[i];
            }
        }
        activity = select(max_fd + 1, &tmp_fds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("[-]Select error");
            exit(1);
        }

        if (FD_ISSET(server_sock, &tmp_fds))
        {
            client_sock = accept(server_sock, NULL, NULL);
            if (client_sock < 0)
            {
                perror("[-]Accept error");
                continue;
            }

            if (num_clients >= MAX_CLIENTS)
            {
                send(client_sock, "Current limit of server exceeded. Try again later.\n", strlen("Current limit of server exceeded. Try again later.\n"), 0);
                close(client_sock);
                continue;
            }

            printf("New connection accepted. Sending UUID...\n");

            char client_uuid_str[37];
            uuid_t client_uuid;
            uuid_generate(client_uuid);
            uuid_unparse(client_uuid, client_uuid_str);

            for (int i = 0; i < MAX_CLIENTS; ++i)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = client_sock;
                    uuid_copy(active_uuids[i], client_uuid);
                    break;
                }
            }

            char welcome[] = "welcome, your UUID is ";
            send(client_sock, strcat(welcome, client_uuid_str), strlen(strcat(welcome, client_uuid_str)), 0);

            send(client_sock,"\nIt can happen that you will not see \"user>\" still you don't have to wait. its a bug and not fixed yet so start executing directly or just press enter once..Thank you !!", strlen("\nIt can happen that you will not see \"user>\" still you don't have to wait. its a bug and not fixed yet so start executing directly or just press enter once..Thank you !!"), 0);

            FD_SET(client_sock, &read_fds);
            if (client_sock > max_fd)
            {
                max_fd = client_sock;
            }
        }

        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            int sockfd = client_sockets[i];
            if (FD_ISSET(sockfd, &tmp_fds))
            {
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
                if (bytes_received <= 0)
                {
                    // unintentional disconnected..
                    printf("Client %d disconnected.\n", sockfd);
                    close(sockfd);
                    FD_CLR(sockfd, &read_fds);
                    client_sockets[i] = 0;
                }
                else
                {

                    printf("Received message from client %d: %s\n", sockfd, buffer);
                    // /active
                    if (strncmp(buffer, "/active", 7) == 0)
                    {

                        printf("Client %d requested active clients.\n", sockfd);
                        for (int j = 0; j < MAX_CLIENTS; j++)
                        {
                            if (client_sockets[j] != 0)
                            {
                                char active_info_str[48];
                                sprintf(active_info_str, "%d : ", client_sockets[j]);
                                char active_uuid_str[37];
                                uuid_unparse(active_uuids[j], active_uuid_str);
                                strcat(active_info_str, active_uuid_str);
                                strcat(active_info_str, "\n");
                                send(sockfd, active_info_str, strlen(active_info_str), 0);
                            }
                        }
                    }
                    // /send
                    else if (strncmp(buffer, "/send", 5) == 0)
                    {
                        char dest_uuid_str[37];
                        char message[BUFFER_SIZE];
                        sscanf(buffer, "/send %s %[^\n]", dest_uuid_str, message);

                        int dest_sock = -1;
                        for (int j = 0; j < MAX_CLIENTS; j++)
                        {
                            if (client_sockets[j] != 0)
                            {
                                char curr_uuid_str[37];
                                uuid_unparse(active_uuids[j], curr_uuid_str);
                                if (strcmp(dest_uuid_str, curr_uuid_str) == 0)
                                {
                                    dest_sock = client_sockets[j];
                                    break;
                                }
                            }
                        }

                        if (dest_sock != -1)
                        {

                            char sender_uuid_str[37];
                            uuid_unparse(active_uuids[i], sender_uuid_str);

                            char complete_message[BUFFER_SIZE + 37 + 2];
                            sprintf(complete_message, "%s:%s", sender_uuid_str, message);

                            printf("%s\n", dest_uuid_str);
                            printf("%s\n", sender_uuid_str);

                            save_message_to_history(dest_uuid_str, sender_uuid_str, complete_message);
                            save_message_to_history(sender_uuid_str, dest_uuid_str, complete_message);
                            printf("Sending message to client %d: %s\n", dest_sock, complete_message);
                            send(dest_sock, complete_message, strlen(complete_message), 0);
                        }
                        else
                        {
                            char not_found_msg[] = "Destination is either offline or not Found...";
                            send(client_sock, not_found_msg, strlen(not_found_msg), 0);
                        }
                    }
                    //smartbot login
                    else if (strncmp(buffer, "/chatbot_v2 login", 17) == 0)
                    {
                        chatbot_v2_enable[i] = 1;
                        // printf("smartbot login");
                        send(sockfd, "gpt2bot> I am a Chatbot capable of answering questions for airline services but take it with grain of salt it can be wrong.", strlen("gpt2bot> I am a Chatbot capable of answering questions for airline services but take it with grain of salt it can be wrong."), 0);
                        char sender_uuid_str[37];
                        uuid_unparse(active_uuids[i], sender_uuid_str);
                        char filename[100];
                        strcpy(filename, "chatbot_details_");
                        strcat(filename, sender_uuid_str);
                        strcat(filename, ".txt");
                        FILE *que = fopen(filename,"w");
                        if (que == NULL)
                        {
                            printf("Error opening file!\n");
                            return 1;
                        }
                        fprintf(que, "User >");
                        fclose(que);
                        // send(sockfd, "stupidbot> Hi, I am stupid bot, I am able to answer a limited set of your questions\n", strlen("stupidbot> Hi, I am stupid bot, I am able to answer a limited set of your questions\n"), 0);
                    }
                    // smartbot logout
                    else if (strncmp(buffer, "/chatbot_v2 logout", 18) == 0)
                    {
                        // just making a veriable 0
                        chatbot_v2_enable[i] = 0;
                        printf("smartbot logout");
                        send(sockfd, "gpt2bot> Thanks for considering me have a nice day.", strlen("gpt2bot> Thanks for considering me have a nice day."), 0);
                        char sender_uuid_str[37];
                        uuid_unparse(active_uuids[i], sender_uuid_str);
                        char filename[100];
                        strcpy(filename, "chatbot_details_");
                        strcat(filename, sender_uuid_str);
                        strcat(filename, ".txt");
                        FILE *que = fopen(filename,"w");
                        if (que == NULL)
                        {
                            printf("Error opening file!\n");
                            return 1;
                        }
                        fprintf(que, " ");
                        fclose(que);
                        // send(sockfd, "stupidbot> Bye! Have a nice day and do not complain about me\n", strlen("stupidbot> Bye! Have a nice day and do not complain about me\n"), 0);
                    }
                    
                    // chatbot login
                    else if (strncmp(buffer, "/chatbot login", 14) == 0)
                    {
                        // just making a veriable 1
                        chatbot_enabled[i] = 1;
                        send(sockfd, "stupidbot> Hi, I am stupid bot, I am able to answer a limited set of your questions\n", strlen("stupidbot> Hi, I am stupid bot, I am able to answer a limited set of your questions\n"), 0);
                    }
                    // chatbot logout
                    else if (strncmp(buffer, "/chatbot logout", 15) == 0)
                    {
                        // just making a veriable 0
                        chatbot_enabled[i] = 0;
                        send(sockfd, "stupidbot> Bye! Have a nice day and do not complain about me\n", strlen("stupidbot> Bye! Have a nice day and do not complain about me\n"), 0);
                    }
                    // for other chatbot questions and answer
                    else if (chatbot_enabled[i] == 1)
                    {

                        char question_with_space[BUFFER_SIZE];
                        strcpy(question_with_space, buffer);
                        strcat(question_with_space, " ");
                        char *answer = search_faq(question_with_space);
                        char final_answertosend[BUFFER_SIZE];
                        strcpy(final_answertosend, "stupidbot> ");
                        strcat(final_answertosend, answer);
                        send(sockfd, final_answertosend, strlen(final_answertosend), 0);
                    }
                    // for delete all for perticular user
                    else if (strncmp(buffer, "/delete_all", 11) == 0)
                    {
                        char sender_uuid_str[37];
                        uuid_unparse(active_uuids[i], sender_uuid_str);
                        delete_complete_chat_history(sender_uuid_str);
                        send(sockfd, "All chat history deleted.\n", strlen("All chat history deleted.\n"), 0);
                    }
                    // history delete
                    else if (strncmp(buffer, "/history_delete", 15) == 0)
                    {

                        char with_uuid[37];
                        sscanf(buffer, "/history_delete %s", with_uuid);
                        char sender_uuid_str[37];
                        uuid_unparse(active_uuids[i], sender_uuid_str);
                        delete_chat_history(sender_uuid_str, with_uuid);
                        send(sockfd, "Chat history deleted.\n", strlen("Chat history deleted.\n"), 0);
                    }
                    // to display history
                    else if (strncmp(buffer, "/history", 8) == 0)
                    {
                        char with_uuid[37];
                        sscanf(buffer, "/history %s", with_uuid);
                        char sender_uuid_str[37];
                        uuid_unparse(active_uuids[i], sender_uuid_str);
                        retrieve_chat_history(sender_uuid_str, with_uuid, sockfd);
                    }
                    // to logout user...
                    else if (strncmp(buffer, "/logout", 7) == 0)
                    {
                        printf("Client %d requested logout.\n", sockfd);
                        send(sockfd, "Bye!! Have a nice day", strlen("Bye!! Have a nice day"), 0);
                        close(sockfd);
                        FD_CLR(sockfd, &read_fds);
                        client_sockets[i] = 0;
                    }
                    // for other chatbot questions and answer
                    else if (chatbot_v2_enable[i] == 1)
                    {
                        char sender_uuid_str[37];
                        uuid_unparse(active_uuids[i], sender_uuid_str);
                        char filename[100];
                        strcpy(filename, "chatbot_details_");
                        strcat(filename, sender_uuid_str);
                        strcat(filename, ".txt");
                        FILE *que = fopen(filename,"w");
                        if (que == NULL)
                        {
                            printf("Error opening file!\n");
                            return 1;
                        }
                        size_t bytes_written = fwrite(buffer, sizeof(char), strlen(buffer), que);

                        if (bytes_written != strlen(buffer)) {
                            printf("Error writing to file!\n");
                            fclose(que);
                            return 1;
                        }
                        fclose(que);
                        // printf("hiiii");
                        char pythonexecc[60];
                        memset(pythonexecc,0,60);
                        strcat(pythonexecc,"python3 gpt_2_gen.py ");
                        strcat(pythonexecc,sender_uuid_str);
                        printf("\n%s\n",pythonexecc);
                        system(pythonexecc);
                        strcpy(pythonexecc,"");

                        // remove(filename);
                        // char response_filename[100];

                        // snprintf(response_filename, 100, "response_%s.txt", sender_uuid_str);

                        FILE *response_file = fopen(filename, "r");
                        if (response_file == NULL) {
                            printf("Error opening response file!\n");
                            return 1;
                        }

                        // Read the entire content of the file into a string
                        char response_buffer[BUFFER_SIZE];
                        size_t total_bytes_read = 0;
                        size_t bytes_read;
                        while ((bytes_read = fread(response_buffer + total_bytes_read, 1, BUFFER_SIZE - total_bytes_read, response_file)) > 0) {
                            total_bytes_read += bytes_read;
                            if (total_bytes_read >= BUFFER_SIZE - 1) {
                                printf("Response file too large for buffer!\n");
                                fclose(response_file);
                                return 1;
                            }
                        }
                        response_buffer[total_bytes_read] = '\0'; // Null-terminate the string

                        // Close the response file
                        fclose(response_file);

                        // Send the entire content to the client
                        send(sockfd, response_buffer, total_bytes_read, 0);

                        // Clean up: remove files
                        remove(filename);

                        
                    }
                    // for other chatbot questions and answer
                    else if (chatbot_enabled[i] == 1)
                    {

                        char question_with_space[BUFFER_SIZE];
                        strcpy(question_with_space, buffer);
                        strcat(question_with_space, " ");
                        char *answer = search_faq(question_with_space);
                        char final_answertosend[BUFFER_SIZE];
                        strcpy(final_answertosend, "stupidbot> ");
                        strcat(final_answertosend, answer);
                        send(sockfd, final_answertosend, strlen(final_answertosend), 0);
                    }
                    // to free buffer
                    else
                    {
                        send(sockfd, "Request not matched...\n", strlen("Request not matched...\n"), 0);
                        bzero(buffer, BUFFER_SIZE);
                    }
                }
                //just for fun..
                printf("and to shutdown server ctrl+c is enough...\n");
            }
        }
    }
    // Close server socket
    close(server_sock);
    return 0;
}
