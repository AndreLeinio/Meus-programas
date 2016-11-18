#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include <netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#define MSG_LEN 256
#define MAX_CON 1024
#define PORT 1337


//Realiza o download de aquivo do servidor
int RFile(char* path, int sendToSocket ){

  //declara e zera posições do buffer de recebimento
  char dataBuffer[MSG_LEN];
  bzero(dataBuffer,MSG_LEN);

  //Abre Arquivo para escrita
  FILE* file =  fopen(path, "w+" );
  if(file == NULL){
		perror(path);
		return -1;
	}

  //Inicio da transmissao
  int recv;
  while(((recv = read(sendToSocket,dataBuffer,MSG_LEN)) > 0)){

    //Salva dados no arquivo
    fwrite(dataBuffer,1,recv,file );
    bzero(dataBuffer,MSG_LEN);
    printf("Recebidos %d bytes\n",recv);
  }

  //closes the file
  fclose(file);
  close(sendToSocket);
  return 0;
}


//Realiza o upload de arquivo para o servidor
int SFile(char* path, int sendToSocket ){

  char* dataBuffer[MSG_LEN];

  // enviar tamanho do arquivo
  int file = open(path, O_RDONLY );
  if(file < 0){
    perror(path);
    return -1;
  }

  //Salva tamanho do arquivo
  int size = lseek (file, 0, SEEK_END);
  lseek(file,0,SEEK_SET);

  //Envio do arquivo
  int sent = 0;
  int total_size = size;

  off_t offset = 0;
    printf("Enviando %d bytes\n",size);
  while (((sent = sendfile(sendToSocket, file, &offset, MSG_LEN)) > 0) ) {
      size -= sent;
      printf("%.2f%% concluido \n", 100.0-((size*100.0)/total_size) );
  }

  close(file);
  close(sendToSocket);
  return 0;
}


int main(int argc,char**argv){

  //validates the input arguments
  if(argc != 3){
    printf("Insufficient number of parameters. \n");
    printf("Uses: ./%s <addressOfFileToBeSent> <destinationIP>:<addressOfFile>\n",argv[0]);
    printf("      ./%s <destinationIP>:<addressOfFile> <addressOfFileToBeSent> \n",argv[0]);
    exit(0);
  }
  //Processa requisição
  char* filename = argv[1];
  char* ip;
  char* path_local;
  char* path_server;
  char op = 'e';
  int i, server_not_found = 1;
  printf("Processando parametros\n");
  for(i = 0; filename[i] != '\0';i++){
    if(filename[i] == ':'){
       filename[i] = '\0';
       ip = filename;
       path_server = &filename[i+1];
       path_local = argv[2];
       server_not_found = 0;
       op = 'd';
    }
  }
  if(server_not_found){
    filename = argv[2];
    path_local = argv[1];
    for(i = 0; filename[i] != '\0';i++){
      if(filename[i] == ':'){
         filename[i] = '\0';
         ip = filename;
         path_server = &filename[i+1];
         server_not_found = 0;
         op = 'u';
      }
    }
  }
  if(server_not_found){
    perror("server ip not found");
    return -1;
  }
  printf("Op:%c\nLocal:%s\nIP:%s\nServer:%s\n", op, path_local,ip, path_server);


  //variables needed to socket and file manipulation
  int serverConnection, listenToSocket;
  char msg[MSG_LEN];
  struct sockaddr_in serverSocketAddress;
  struct sockaddr_in clientSocketAddress;

  //setting the listening to the socket to identify a response from the server
  printf("Criando Socket\n");
  listenToSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  //cleaning (setting to zero) the memory to be used
  bzero(&serverSocketAddress, sizeof(serverSocketAddress));

  //socket configuration !! MAN PAGES & POSIX MANUAL !!
  serverSocketAddress.sin_family = AF_INET;
  serverSocketAddress.sin_addr.s_addr = inet_addr(ip);
  serverSocketAddress.sin_port = htons(PORT);

  //connection to socket
  printf("Conectando ao servidor\n");
  serverConnection = connect(listenToSocket,(struct sockaddr*)&serverSocketAddress,sizeof(serverSocketAddress));
  if(serverConnection < 0){
    perror("connect()");
    return -1;
  }
  //envia operacao para o servidor
  bzero(msg,MSG_LEN);
  msg[0] = op;
  for(i = 0 ; i<strlen(path_server);i++){
    msg[i+1] = path_server[i];
  }
  msg[strlen(path_server) + 1] = '\0';

  printf("Conectado..enviando operacao ao servidor : %s\n", msg);
  write(listenToSocket, msg, MSG_LEN);

  if(op == 'd'){
    printf("Operacao de download do servidor\n");
    //recebe confirmacao do servidor se o arquivo foi encontrado
    int res;
    read(listenToSocket, &res, sizeof(res));
    // res = ntohl(res);

    if(res == 302){ //FOUND
      RFile(path_local, listenToSocket );
    }
    else{//NOT FOUND
      perror("FILE NOT FOUND");
      return -1;
    }
  }
  else if( op == 'u'){
    printf("Operacao de upload para o servidor\n");
    SFile( path_local, listenToSocket);
  }

  return 0;
}
