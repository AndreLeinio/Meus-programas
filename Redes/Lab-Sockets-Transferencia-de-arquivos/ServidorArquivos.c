#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include <netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include<omp.h>

#define MSG_LEN (64*1024)
#define MAX_CON 1024
#define PORT 1337


//Realiza e download do aquivo do cliente
int RFile(char* path, int sendToSocket ){
  //opens the file
  char dataBuffer[MSG_LEN];

  //recebe tamanho do arquivo
  // bzero(dataBuffer,MSG_LEN);
  // int size;
  // printf("Aguardando tamanho do arquivo\n");
  // read(sendToSocket, &size, sizeof(size));
  // size = ntohl(size);

  //fills in the vector with the desired value (binary from file)
  int file =  open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
  if(file < 0){
		perror(path);
		return -1;
	}
  int recv;
  bzero(dataBuffer,MSG_LEN);
  // printf("Recebendo %d bytes de arquivo...\n", size);
  while(((recv = read(sendToSocket,dataBuffer,MSG_LEN)) > 0)){
    write(file,dataBuffer,MSG_LEN); //writes it to the file
    // size -= recv;
    // printf("%d bytes faltando\n",size);
  }


  //closes the file
  close(file);
  close(sendToSocket);
  return 0;
}


//Realiza o upload de arquivo para o cliente
int SFile(char* path, int sendToSocket ){

  char* dataBuffer[MSG_LEN];

  //busca arquivo
  printf("Buscando arquivo solicitado\n");
  if( access( path, F_OK ) != -1 ) {
    // file exists
    int res = 302; //FOUND
    write(sendToSocket,&res, sizeof(res));

  }
  else {
    // file doesn't exist
    int res = 404; // NOT FOUND
    write(sendToSocket,&res, sizeof(res));
    close(sendToSocket);
    return -1;

  }

  // enviar tamanho do arquivo
  int file = open(path, O_RDONLY);
  if(file < 0){
    perror(path);
    return -1;
  }
  int size = lseek (file, 0, SEEK_END);
  // int size = tell (file);
  lseek(file,0,SEEK_SET);
  //int sizeS = htonl(sizeS);
  // printf("Enviando tamanho do arquivo\n");
  // write(sendToSocket,&size, sizeof(size));

  //Envio do arquivo
  int sent = 0;
  off_t offset = 0;
  printf("Enviando %d bytes\n",size);
  while (((sent = sendfile(sendToSocket, file, &offset, MSG_LEN)) > 0) && (size > 0)) {
      size -= sent;
      printf("%d bytes faltando\n", size);
  }

  close(file);
  close(sendToSocket);
  return 0;
}



int main(int argc, char* argv[]){

printf("Iniciando Servidor\n");
//variables needed to file and socket manipulation
char filename[MSG_LEN];
int listenToSocket, socketToFile, sendToSocket; //socket handlers
struct sockaddr_in serverSocketAddress; //address of the server socket
struct sockaddr_in clientSocketAddress; //address of the client socket

  //setting the listening to the socket to identify a request
  printf("Criando Socket\n");
  listenToSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  bzero(&serverSocketAddress, sizeof(serverSocketAddress)); //cleaning (setting to zero) the memory to be used

  //socket configuration !! MAN PAGES & POSIX MANUAL !!
  serverSocketAddress.sin_family = AF_INET;
  serverSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverSocketAddress.sin_port = htons( PORT );

  //starting the bind service and the listening proccess
  printf("Binding...\n");
  if(bind(listenToSocket, (struct sockaddr*)&serverSocketAddress,
                          sizeof(serverSocketAddress))<0){
    perror("Bind error.");
    return -1;
  }
  printf("Listening...\n");
  if(listen(listenToSocket,MAX_CON) < 0 ){
    perror("Listen error.");
    return -1;
  }
  int client_len = sizeof(clientSocketAddress);

  while(1){
  //stating the client habilitation and connection

    bzero(&clientSocketAddress, client_len);
    printf("Aguardando conexao\n");
    sendToSocket = accept(listenToSocket,
                          (struct sockaddr*)&clientSocketAddress,
                          (socklen_t)&client_len);

    if(sendToSocket < 0){
      perror("Accept error.");
      continue;
    }
    //printf("Conectado com %d:%s\n",clientSocketAddress.sin_port,clientSocketAddress.sin_addr);


    //Tratar solicitação
    //Recebe operacao e path para o arquivo a ser enviado ou recebido
    printf("Aguardando operacao\n");
    bzero(&filename, sizeof(filename));
    int tam = read( sendToSocket, filename, MSG_LEN);

    filename[tam] = '\0';
    char op = filename[0];
    char path[tam];
    strncpy(path, filename+1,tam-1);


    //chamar função correspondente
    if(op == 'u'){//Receive file from client
        printf("Preparando para receber arquivo: %s\n",path);
        RFile(path,sendToSocket);
    }
    else if(op == 'd'){//Send file to client
      printf("Preparando para enviar arquivo: %s\n",path);
      SFile(path, sendToSocket);
    }
    else{
      perror("invalid operation");
      close(sendToSocket);
      continue;
    }


    printf("Operação concluida\n");

}

}
