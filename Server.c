//												 MODULO SERVER 
//	./mastermind_server <host remoto> <porta_ascolto>

//librerie da includere

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>	
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>


#define DIM_NOME_CLIENT 20
#define DIM_PORTA_UDP 7
#define DIM_MSG_TCP 1024
#define DIM_IP 16
#define LOGIN 10
#define WHO 11
#define CONNECT 12
#define DISCONNECT 13
#define QUIT 14
#define NO 15
#define YES 16

char *ip_client[100]; //max 100 client connessi, array di stringhe

struct st_client		// STRUTTURA ASSOCIATA AI CLIENT CONNESSI AL SERVER
  {	
    char username[DIM_NOME_CLIENT];
    char porta_udp[DIM_PORTA_UDP];
    char ip[16];
    char avversario[DIM_NOME_CLIENT]; //se stato_client = 0 conterra' il nome dell'avversario
    int  stato_client ;  	// 1 LIBERO , 0 OCCUPATO 
    int cn_sk;			//connected_socket
    struct st_client* pun;
  };	

int invia_tcp(int sk,char* stringa)
{
  int mes_len;
  int ret;

  mes_len = htonl(strlen(stringa));

  ret = send(sk, &mes_len, sizeof(mes_len), 0);
  //printf("dimesione inviata =%d\n",mes_len);

  if(ret < 0)
    return -1;
  if(ret == 0)
    return 0;
  ret = send(sk, stringa, strlen(stringa), 0);
  if(ret < 0)
    return -1;
  if(ret == 0)
    return 0;
  
  return ret;
}

int ricevi_tcp(int sk, char *stringa)
{
  int mes_len;
  int ret;
  memset(stringa,0,sizeof(DIM_MSG_TCP));
  //printf("in attesa della dimensione\n");
  ret = recv(sk, &mes_len, sizeof(mes_len), MSG_WAITALL);
  if(ret < 0)
    return -1;
  if(ret == 0)
    return 0;
  //printf("ho ricevuto al dimensione:%d\n",mes_len);
  mes_len = ntohl(mes_len);
  ret = recv(sk, stringa, mes_len, MSG_WAITALL);
  if(ret < 0)
    return -1;
  if(ret == 0)
    return 0;
  stringa[mes_len]='\0';
  //printf("riceviTCP:%s\n",stringa);
  return ret;
}

int cerca(struct st_client* lista,int sk) //se client sulla socket sk si e' gia loggato
{
  struct st_client* testa;
  testa = lista;
  while((testa != NULL) && (testa->cn_sk != sk))
    testa = testa->pun;
  if(testa == NULL)
    return 0;
  else
    return 1;
}

int stato(struct st_client* testa,int sk)
{
  struct st_client *elem;
  elem = testa;
  while((elem != NULL) && (elem->cn_sk != sk))
    elem = elem->pun;

  return elem->stato_client;
}

int inizio_partita(struct st_client* testa,int sk,char* stringa)
{
  struct st_client *elem;
  char nome[DIM_NOME_CLIENT]; //nome client sk
  char avversario[DIM_NOME_CLIENT]; //nome avversario sk
  int i, j;

  elem = testa;
  while((elem != NULL) && (elem->cn_sk != sk))
    elem = elem->pun;
  elem->stato_client = 0;

  j = 4;
  for(i=4; stringa[i] != ' ' ;i++);
  //strncpy(elem->ip,stringa+j, i-j);
  //elem->ip[i] = '\0';
  i++;
  memset(elem->avversario,0, DIM_NOME_CLIENT);
  strcpy(elem->avversario,stringa+i);
  memset(avversario,0,DIM_NOME_CLIENT);
  strcpy(avversario,elem->avversario);
  //invio dati dell'avversario al client sk
  memset(stringa,0,DIM_MSG_TCP);
  strcpy(stringa,"YES ");
  strcat(stringa,elem->avversario);
  strcat(stringa," ");
  elem = testa;
  while((elem != NULL) && (strcmp(elem->username,avversario)))
    elem = elem->pun;
  strcat(stringa,elem->ip); //dati avversario
  strcat(stringa," ");
  strcat(stringa,elem->porta_udp);
  
  j = invia_tcp(sk,stringa);

  if(j <= 0)
    return j; 

  elem = testa;
  while((elem != NULL) && (elem->cn_sk != sk))
    elem = elem->pun; 

  memset(stringa,0,DIM_MSG_TCP);
  strcpy(stringa,"YES ");
  strcat(stringa,elem->username);
  strcat(stringa," ");
  strcat(stringa,elem->ip);
  strcat(stringa," ");
  strcat(stringa,elem->porta_udp);
  
  strcpy(nome,elem->username);
  memset(avversario,0,DIM_NOME_CLIENT);
  strcpy(avversario,elem->avversario);

  elem = testa;
  while((elem != NULL) && (strcmp(elem->username,avversario) != 0))
    elem = elem->pun;
  elem->stato_client = 0;
  strcpy(elem->avversario,nome);

  j = elem->cn_sk;
  //j = invia_tcp(elem->cn_sk,stringa);

  //if(j > 0)
  printf("%s si e' connesso a %s\n",elem->username,nome);

  return j;
}

int valida_indirizzo(char* str, int dim)	//funzione controllo correttezza formale indirizzo host
{
  int i;
  for(i=0; i< dim;i++)
  {
    if(atoi(str+i) >= 256)
      return -1;
    while(str[i] != '.')
      i++;
  }
  return 0;
}

int copia_nome(struct st_client* testa,int sk,char* stringa)
{
  struct st_client *elem;
  char avv[DIM_NOME_CLIENT];
  
  elem = testa;
  
  strcpy(avv,stringa+3);
  
  while((elem != NULL) && (elem->cn_sk != sk))
    elem = elem->pun;

  memset(stringa, 0, DIM_MSG_TCP);
  strcpy(stringa,"NO ");
  strcat(stringa,elem->username);
  strcat(stringa," ha riutato la partita\n");
  elem->stato_client = 1;

  elem = testa;
  while((elem != NULL) && (strcmp(elem->username,avv) != 0))
    elem = elem->pun;

  elem->stato_client = 1;
  
  
  return elem->cn_sk;//invia_tcp(elem->cn_sk,stringa);
}

int elabora_stringa(char * stringa)
{
  char lav[12];
  int i;
   
  if(strcmp(stringa,"!who") == 0)
    return WHO;
  if(strcmp(stringa,"!disconnect") == 0)
    return DISCONNECT;
  if(strcmp(stringa,"!quit") == 0)
    return QUIT;
  

  for(i=0; (i<strlen(stringa)) && (stringa[i]!= ' ');i++)
    lav[i] = stringa[i];
  lav[i]='\0';
  if(i == strlen(stringa))
    return -1;
  if(strcmp(lav,"NO") == 0)
    return NO;
  if(strcmp(lav,"YES") == 0)
    return YES;
  if(strcmp(lav,"login") == 0)
     return LOGIN;
  if(strcmp(lav,"!connect") == 0)
    return CONNECT;
  else
    return 0;
}

int verifica_login(struct st_client* testa, char* stringa)
{
  int i, j;
  int ret;
  char nome[DIM_NOME_CLIENT];	
  
  memset(nome, 0, DIM_NOME_CLIENT);
  if(testa == NULL)
    ret = 0;
  
  for(i=0  ; (i < strlen(stringa)) && (stringa[i] != ' '); i++);
  i++;
  j = i;
  for(;(i < strlen(stringa)) && (stringa[i] != ' '); i++);
  
  strncpy(nome,stringa+j,i-j);
  while((testa!= NULL) && (strcmp(testa->username,nome) != 0))
    testa = testa->pun;
  if(testa == NULL)
    ret = 0;
  else
    ret = -1;
  
  return ret;
}

void login_client(struct st_client *testa, int fd, char *stringa)
{
  int i;
  int j;
  //salto la prima parola di comando
  for(i=0  ; (i < strlen(stringa)) && (stringa[i] != ' '); i++);
  
  i++;
  
  //cerco la struttura
  while((testa!= NULL) && (testa->cn_sk != fd))
      testa = testa->pun;
 
  memset(testa->username,0,DIM_NOME_CLIENT);
  j = i;
  //copio la seconda parola e il NOME
  for(;(i < strlen(stringa)) && (stringa[i] != ' '); i++);
  strncpy(testa->username,stringa+j,i-j);
  i++;
  //copio la porta_udp
  j=i;
  for(; stringa[i] != '\0';i++);
  
  memset(testa->porta_udp,0,DIM_PORTA_UDP);
  strncpy(testa->porta_udp,stringa+j,i-j);
  
  return;    
}

struct st_client* new_client(struct st_client* testa, char* stringa, char* addr, int sk) //inserimento in testa 
{
  struct st_client * elem;

  elem = (struct st_client*)malloc(sizeof(struct st_client));
  memset(elem, 0 ,sizeof(elem));
  
  elem->stato_client = 1;  //libero
  strcpy(elem->ip , addr);
  elem->cn_sk = sk;
  if(testa == NULL) //inserimento del primo elemento
    elem->pun = NULL;
  else
    elem->pun = testa;

  testa = elem;
  
  login_client(testa, elem->cn_sk, stringa); 
  
  printf("Connessione stabilita con il client \n");
  printf("%s si e' connesso \n",elem->username);
  printf("%s e' libero (L) \n",elem->username);
 
  return testa;
}

struct st_client* delete_client(struct st_client* testa, int fd)
{
  struct st_client *elem;
  struct st_client *app;

  app = NULL;
  elem = testa;

  while((elem->cn_sk != fd) && (elem != NULL))
  { 
    app = elem;  
    elem = elem->pun; 
  }

  if(elem == NULL)
    return testa;
 /* if(elem->stato_client == 0)
  {
    printf("%s si e' disconnesso da %s\n",elem->username,elem->avversario);
  }
  */
  printf("%s si e' disconesso\n",elem->username);

  if(elem == testa) 
    testa = testa->pun;
  else
    app->pun = elem->pun;
   
  free(ip_client[elem->cn_sk]); 	
  free(elem);
  return testa;
}

struct st_client*  disconnect_client(struct st_client *testa,char *stringa,int sk,int* numero,fd_set read_fd)
{
  struct st_client* elem;
  char  nome_avversario[DIM_NOME_CLIENT];
  int ret;
  elem = testa;
  //client connesso sk ha abbandonato la partita
  while((elem!=NULL) && (elem->cn_sk != sk))
    elem = elem->pun;
  //trovato il descrittore del client che ha chiesto la disconnessione
    printf("%s si e' disconnesso da %s\n",elem->username,elem->avversario);
    elem->stato_client = 1;
    printf("%s e' libero (L)\n",elem->username);
    strcpy(nome_avversario,elem->avversario);
    
    elem = testa;
    while(strcmp(elem->username,nome_avversario) != 0)
      elem = elem->pun;

    elem->stato_client = 1;
    printf("%s e' libero (L)\n",elem->username);
    memset(stringa,0,DIM_MSG_TCP);
    strcpy(stringa,"!disconnect");

    ret = invia_tcp(elem->cn_sk,stringa);
    
    if(ret <= 0)
    {
      perror(NULL);
      testa = delete_client(testa,elem->cn_sk);
      if(*numero == 1)
        testa = NULL;
      (*numero)--;
      close(sk);
      FD_CLR(sk,&read_fd);
    }
    
    return testa;
}

int connect_client(struct st_client* testa, char* stringa,int sk)	
{
  struct st_client* elem;
  struct st_client* app;
  char nome[DIM_NOME_CLIENT];
  int i;

  for(i=0; stringa[i]!= ' ';i++);
  i++;
  strcpy(nome,stringa+i); //nome contiene il nome da cercare..

  memset(stringa,0,DIM_MSG_TCP);
  app = testa;
  elem = testa;
  while((elem != NULL) && (strcmp(elem->username,nome)!= 0))
    elem = elem->pun;

  if(elem == NULL)
  {	
    strcpy(stringa,"NO Impossibile connetersi a ");
    strcat(stringa,nome);
    strcat(stringa,": utente inesistente \n");
    return 0;
  }
  if(elem->cn_sk == sk)
  {
    strcpy(stringa,"NO Stai cercando di giocare con te stesso\n");
    return 0;
  }
  if(elem->stato_client == 0)
  {	
    strcpy(stringa,"NO Impossibile connettersi a ");
    strcat(stringa,nome);
    strcat(stringa,": l'utente e' gia' impegnato in un altra partita\n");
    return 0;
  }
  else
  {	//ritorno porta sk_tcp del client a cui inviare la richiesta di partita

    strcpy(stringa,"PARTITA ");

    while((app != NULL) && (app->cn_sk != sk) )
    app = app->pun;

    elem->stato_client = 0;
    app->stato_client = 0;
    strcat(stringa,app->username);
    return elem->cn_sk;
  }

}  

void who_client(struct st_client* testa, int numero,int sk, char* stringa)
{
  struct st_client* elem;
  
  memset(stringa, 0, DIM_MSG_TCP);
  strcpy(stringa,"WHO ");

  if((numero == 1) || (testa == NULL) ) //non ci sono altri client collegati
  {
    if(numero == 1)
      strcat(stringa,"Sei l'unico utente collegato\n");
    else
      strcat(stringa,"Non ci sono altri utenti collegati.Aspetta!\n");
    return;
  }
  elem = testa;
  strcat(stringa,"Client connessi al server: ");
  
  while(elem != NULL)
  {
    if(elem->cn_sk != sk)
    {
      strcat(stringa,elem->username);
      if(elem->stato_client == 1)
	strcat(stringa,"(L) ");
      else
	strcat(stringa,"(O) ");
    }
      elem = elem->pun;
  }
  strcat(stringa,"\n");
  
  return;
}



int main ( int argc, char *argv[])
{
  

  //*****OPPORTUNE DICHIARAZIONI ******//
  
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  int listen_sk ;
  int len;
  struct st_client *c_client = NULL;		//Lista di strutture per gestire i client connessi
  int num_client = 0;
  int caso;
  char buffer[DIM_MSG_TCP];//dimensionaare nel modo corretto
  //char comando[12];
  int nbyte, i, j;
  int set = 1;
  int fd_pronti;
  fd_set read_fd ,allset ;
  int fdmax, newfd;
  
  //***** FINE DICHIARAZIONI ******//

  if(argc == 3) //oppurtuni parametri immessi
  {
    //oppurtuni controlli
    if( valida_indirizzo(argv[1],strlen(argv[1])) < 0)
    {
      printf("Errore nei parametri passati al comando ./mastermind_client\nIndirizzo <host remoto> errato\n");
      exit(-1);
    }
    
    if(atoi(argv[2]) >= 65536)	//atoi funzione lib_stdlib.h trasforma int la stringa puntata
    {
      printf("Errore nei parametri passati al comando ./mastermind_server\nNumero <porta> errato\n");
      exit(-1);
    }
  }
  else
  {
    printf("Errore nel numero di parametri del comdando ./mastermind_server \n");
    exit(-1);
  }
  
  
  printf("Indirizzo: %s (Porta: %s) \n",argv[1],argv[2]); 
  
  FD_ZERO(&read_fd);
  FD_ZERO(&allset);
  if((listen_sk = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Error: socket() failed\n");
    perror(NULL);
    exit(-1);
  }
  
  if(setsockopt(listen_sk, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int)) == -1 )
  {
    printf("Error: setsockopt() failed\n");
    perror(NULL);
    close(listen_sk);
    exit(-1);
  }
  
  memset(&server_addr,0,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 		
  server_addr.sin_port = htons(atoi(argv[2]));
  
  if( bind(listen_sk,(struct sockaddr *)&server_addr, sizeof(server_addr)) <0 )
  {
    printf("Error: bind() failed\n");
    perror(NULL);
    close(listen_sk);
    exit(-1);
  }
   
  if(listen(listen_sk,10) < 0 )	// 10 max connesioni pendenti
  {
    printf("Error: listen() failed\n");
    close(listen_sk);
    exit(-1);
  }
 
 // FD_SET(listen_sk, &read_fd);
  FD_SET(listen_sk, &allset);
  fdmax = listen_sk;
    
  for(;;)	
  {
    read_fd = allset;
    //printf("numero di client connessi %d\n",num_client);
    memset(buffer, 0, DIM_MSG_TCP);
    
    if((fd_pronti=select(fdmax+1, &read_fd, NULL, NULL, NULL)) < 0)
    {
      printf("Error: select() failed\n");
      perror(NULL);
      exit(-1);
    }
    
    for(i=0; i <= fdmax; i++)
    {
      
      if(FD_ISSET(i,&read_fd))
      {
		
	if(i == listen_sk) 	// ARRIVO NUOVA CONNESSIONE
 	{
	  memset(&client_addr, 0, sizeof(client_addr));
	  len = sizeof(client_addr);
	  
	  if((newfd = accept(listen_sk, (struct sockaddr*)&client_addr,(socklen_t*)&len)) < 0)
	  {
	    printf("Error: accept() failed \n");
	    perror(NULL);
	    close(listen_sk);
	    exit(-1);
	  }
	     
	  //memorizzo ip client connesso
    ip_client[newfd] = malloc(DIM_IP);
    memset(ip_client[newfd],0,DIM_IP);
    strcpy(ip_client[newfd],(char*)inet_ntoa(client_addr.sin_addr));
    
    FD_SET(newfd,&allset);  
	  if( newfd > fdmax )
	   fdmax = newfd;	  
	}
	else //ARRIVO NUOVI DATI DAL CLIENT
	{
    
	  if((nbyte = ricevi_tcp(i,buffer) ) <= 0)
	  {
	    printf("Error: ricevi_tcp() o Terminazione inaspettata del client\n");
	    perror(NULL);
	    if(num_client > 0)
	    {
          if(cerca(c_client,i) == 1)
          {
            if(stato(c_client,i) == 0)//e' inpegnato in una partita
          	c_client = disconnect_client(c_client,buffer,i,&num_client,read_fd);

		    c_client = delete_client(c_client,i) ;
          
            if(num_client == 1)
              c_client = NULL;

	        num_client--;
	       }
        }   	  
	    close(i);
	    FD_CLR(i,&allset);
	    break;
	  }
	  
    //LEGGO I DATI MANDATI DAL CLIENT
    
	 if( (caso = elabora_stringa(buffer)) < 0)
	    printf("Error: elabora_stringa() failed\n");

	 switch( caso ) //ELABORO COMANDO E MANDO EVENTUALI MESSAGGIO DI RISPOSTA
	 {
	    case LOGIN:
	    {
	      if(verifica_login(c_client, buffer) < 0) //altro utente cn stesso nome
	      {
			printf("Login utente %s con stesso username di un altro giocatore\n",buffer+6);

    		memset(buffer, 0, sizeof(buffer));
			strcpy(buffer,"NUOVO_NOME");
			
			if(( nbyte = invia_tcp(i,buffer)) <= 0)
			{
		 	 if(nbyte < 0)
		  	 {
		    	printf("Error:read() failed.Terminazione client inatessa\n");
		    	perror(NULL);
		    	close(i);
		    	FD_CLR(i,&allset);
		     }
		     else
		     {
		       close(i);
		       FD_CLR(i,&allset);
		     }
		    }

            break;
	      }
	      else
	      {
			c_client = new_client(c_client, buffer, ip_client[i]/*inet_ntoa(client_addr.sin_addr)*/,i);
			num_client++;
    		memset(buffer, 0, sizeof(buffer));
    		strcpy(buffer,"OK");
    		if((nbyte = invia_tcp(i,buffer)) <= 0)
    		{
      			perror(NULL);
      			c_client = delete_client(c_client,i);
      			if(num_client == 1)
       			   c_client = NULL;
      			num_client--;
     			close(i);
      			FD_CLR(i,&allset);
    		}
			break; 
	      }  
	           
	    }
				
	    case WHO:
	    {
	      memset(buffer,0,DIM_MSG_TCP);
	      who_client(c_client,num_client,i,buffer);
	      nbyte = invia_tcp(i,buffer);
	      if( nbyte <= 0)
	      {
			c_client = delete_client(c_client,i);
			if(num_client == 1)
		  	  c_client = NULL;
			num_client-- ;
			perror(NULL);
			close(i);
			FD_CLR(i, &allset);
	      }
	     
	      break;
	    }
	    
	    case CONNECT: 
	    {
	      j = connect_client(c_client,buffer,i);
          if(j == 0) //invio messaggio a sk_tcp
          {
            if((nbyte = invia_tcp(i,buffer)) <= 0)
            {
              perror(NULL);
              c_client = delete_client(c_client ,i);
              if(num_client == 1)
                c_client = NULL;
              num_client-- ;
              close(i);
              FD_CLR(i,&allset);
            }
          }
         else //invio richiesta per la partita al client avversario
         {
           if((nbyte = invia_tcp(j,buffer)) <= 0)
           {
              perror(NULL);
              c_client = delete_client(c_client ,i);
              if(num_client == 1)
                c_client = NULL;
              num_client-- ;
              close(i);
              FD_CLR(i,&allset);
            } 
          }

	      break;
	    }
	    
	    case DISCONNECT: 
	    {
	      c_client = disconnect_client(c_client,buffer,i,&num_client,read_fd);
	      break;
	    }
	    
	    case QUIT:
	    {
	      printf("Richiesta di disconnessione\n");
          if(stato(c_client,i) == 0)//e' inpegnato in una partita
            c_client = disconnect_client(c_client,buffer,i,&num_client,read_fd);

          c_client = delete_client(c_client,i) ;

          strcpy(buffer,"QUIT");
          invia_tcp(i,buffer);
	      
	      if(num_client ==1)
		    c_client = NULL;

	      num_client-- ;
        
          close(i);
          FD_CLR(i,&allset);
	      break;
	    }
	    
        case NO: //utente ha rifiutato la connect
        {
          j = copia_nome(c_client,i,buffer);
          nbyte = invia_tcp(j,buffer);
          if(nbyte <= 0)  
          { //invio dati fallito
            perror(NULL);
            c_client = delete_client(c_client ,i);
            if(num_client == 1)
              c_client = NULL;
            num_client-- ;
            close(i);
            FD_CLR(i,&allset);
          }
          break;
        }

        case YES: //utente ha accettato la partita
        {
          j = inizio_partita(c_client,i,buffer);
          if(j <= 0)
          {
            perror(NULL);
            c_client = disconnect_client(c_client,buffer,i,&num_client,read_fd);
            c_client = delete_client(c_client ,i);
            if(num_client == 1)
              c_client = NULL;
            num_client-- ;
            close(i);
            FD_CLR(i,&allset);
           }
           else
           {
             nbyte = invia_tcp(j,buffer);
             if(nbyte == 0)
             {
                perror(NULL);
                if(stato(c_client,j) == 0)
                  c_client = disconnect_client(c_client,buffer,j,&num_client,read_fd);
                c_client = delete_client(c_client ,j);
                if(num_client == 1)
                  c_client = NULL;
                num_client-- ;
                close(j);
                FD_CLR(j,&allset);
             }
           }
          break;
        }

	    default:	//da gestire
	    {
		  printf("comando non trovato\n");
		  break;
	    }      
		      	
	  }
	

      }//fine ELSE
            
     }//chiusura dell'IF(FD_ISSET)
    
    } //chiusura for(i; i< fdmax..)
    
  }//chiusura for(;;) 
      
  return (1);
}

