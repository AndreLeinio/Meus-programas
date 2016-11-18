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

#define MSG_LEN 256
#define MAX_CON 1024
#define PORT 1337


//Realiza e download do aquivo do cliente
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
  while((recv = read(sendToSocket,dataBuffer,MSG_LEN)) > 0){

    //Salva dados no arquivo
    fwrite(dataBuffer,1,recv,file );
    bzero(dataBuffer,MSG_LEN);
    printf("Recebidos %d bytes\n",recv);
  }

  //closes the file
  fclose(file);
  close(sendToSocket);
  printf("END\n");
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
    printf("Arquivo Encontrado\n");

  }
  else {
    // file doesn't exist
    int res = 404; // NOT FOUND
    write(sendToSocket,&res, sizeof(res));
    printf("Arquivo Não Encontrado... Finalizando\n");
    close(sendToSocket);
    return -1;

  }

  // Abre arquivo para leitura
  int file = open(path, O_RDONLY);
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
  while (((sent = sendfile(sendToSocket, file, &offset, MSG_LEN)) > 0) && (size > 0)) {
      size -= sent;
      printf("%.2f%% concluido \n", 100.0-((size*100.0)/total_size) );
  }

  close(file);
  close(sendToSocket);
  printf("END\n");
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

  #pragma omp parallel private(filename)
  {

  #pragma omp single
  {

  while(1){
    bzero(&clientSocketAddress, client_len);
    printf("Aguardando conexao\n");
    sendToSocket = accept(listenToSocket,
                          (struct sockaddr*)&clientSocketAddress,
                          (socklen_t)&client_len);

    if(sendToSocket < 0){
      perror("Accept error.");
      continue;
    }

    #pragma omp task firstprivate(sendToSocket)
    {

      //Tratar solicitação
      //Recebe operacao e path para o arquivo a ser enviado ou recebido
      printf("Conectado ao servidor. Aguardando operacao\n");
      bzero(&filename, sizeof(filename));
      int tam = read( sendToSocket, filename, MSG_LEN);

      filename[tam] = '\0';
      char op = filename[0];
      char path[tam];
      strncpy(path, filename+1,tam-1);


      //chamar função correspondente
      if(op == 'u'){//Receive file from client
          printf("Sera enviado o arquivo: %s\n",path);
          RFile(path,sendToSocket);
      }
      else if(op == 'd'){//Send file to client
        printf("Solicitado o arquivo: %s\n",path);
        SFile(path, sendToSocket);
      }
      else{
        perror("invalid operation");
        close(sendToSocket);
        
      }

    }//close pragma task


  }//close while(1)
  }//close pragma single
  }//close pragma parallel

}
