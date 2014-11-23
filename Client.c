//				MODULO CLIENT

//libreirie da includere
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>	
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

//DEFINIZIONI
#define DIM_NOME_CLIENT 20
#define DIM_PORTA_UDP 7
#define DIM_MSG_TCP 512
#define DIM_MSG_UDP 256
#define DIM_COMANDO 30
#define STDIN 0
//per capire se client sta giocando
#define GAME_START 1	
#define SC ">  "
#define SP "#"

//-------- DEFINIZIONE DI VARIABILI GLOABLI-------
int  sk_tcp;
char nome_client[DIM_NOME_CLIENT];  //stringa per Nome_Client
//char my_ip[16]; //contiene indirizzo IP dell'utente
char porta_udp_avversario[DIM_PORTA_UDP]; //conterrra' la porta udp utilizzata dall'avversario per il gioco
char ip_avversario[16];
char nome_avversario[DIM_NOME_CLIENT];
char  combinazione[6]; //char combinazione[5];
char porta_udp[DIM_PORTA_UDP];//porta UDP scelta dal client per il gioco
char buffer_tcp[DIM_MSG_TCP];
int  stato = 0;
int turno = 0; //evito di fare digiatare combinazione senza rispettare il turno
struct sockaddr_in gamer_addr;
//--------------------------



//INIZIO DICHIARAZIONI FUNZIONI UTILITA'
int invia_tcp(int sk,char* stringa)
{
  int mes_len;
  int ret;
  mes_len = htonl(strlen(stringa));

  ret = send(sk, &mes_len, sizeof(mes_len), 0);
  if(ret < 0)
    return -1;
  if(ret == 0)
    return 0;
  //printf("ho inviato la dimensione:%d\n",mes_len);	
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
  memset(stringa,0, DIM_MSG_TCP);
  
  ret = recv(sk,&mes_len,sizeof(mes_len), MSG_WAITALL);
  if(ret <= 0)
  {
    perror(NULL);
    return -1;
  }
  // printf("ho ricevuto la dimensione %d",mes_len);
  mes_len = ntohl(mes_len);
 
  ret = recv(sk,stringa, mes_len, MSG_WAITALL);
  
  if(ret <= 0)
  {
    perror("my error: ");
    return -1;
  }
  
  stringa[mes_len]='\0';
  return ret;
}

int invia_udp(int sk,char* stringa)
{
  int ret;
  ret = write(sk,stringa,strlen(stringa));

  if(ret != strlen(stringa))
  {
    printf("Errore nell'invio di tutti i datagrammi UDP\n");
    return -1;
  }
  else
    return ret;
}
int ricevi_udp(int sk,char*stringa)
{
  int ret;
  int dim1, dim2, dim3, dim4;

  dim1 = strlen("HAI VINTO");
  dim2 = strlen("!combinazione     "); //[!combianzione 1234]
  dim3 = strlen("RISPOSTA    ");	//[RISPOSTA g p] g=num giusti, p=num presenti;
  dim4 = strlen("TIMEOUT");

  memset(stringa,0,DIM_MSG_UDP);

  ret = read(sk,stringa,DIM_MSG_UDP);

  if(ret < 0)
  {
    printf("Errore nella ricezione dei datagrammi UDP\n");
    return -1;
  }
  if(ret == 0)
  	return 0;

  if( ret == dim1)
  	return ret;
  if(ret == dim2)
  	return ret;
  if(ret == dim3)
  	return ret;
  if(ret == dim4)
  	return ret;
  else
  {
  	printf("Errore ricezione: Ricevuta parte del messaggio UDP\n");
  	return -1;
  }
}

void richiesta_partita(char* stringa)
{
  int ret;	
  memset(stringa,0,DIM_MSG_TCP);
  //scanf("%s",stringa); //da toglierere
  //getc(stdin);
  fgets(stringa,DIM_MSG_TCP,stdin);
  if(stringa[0] == '\0')
  {
  	printf("Comando non valido. Scrivi yes/y o no/n\n");
    richiesta_partita(stringa);
  }

  if((strlen(stringa) == DIM_MSG_TCP-1) && (stringa[strlen(stringa)] != '\n')) //buffer sporco
  {	
  	while ( ((ret= fgetc(stdin)) != EOF) && (ret!= '\n'));
  	printf("Hai inserito troppi caratteri.\n");
  }
  stringa[strlen(stringa)-1] = '\0';

  stato = GAME_START;
  turno = 0; //giocherai dopo il client che ha fatto la !connect

  if((strcmp(stringa,"yes") == 0) || (strcmp(stringa,"y") == 0))
  {
    printf("Hai accettato la partita\n");
    memset(stringa,0,DIM_MSG_TCP);
    strcpy(stringa,"YES ");
    //strcat(stringa,my_ip);
    strcat(stringa," ");
    strcat(stringa,nome_avversario);
    return ;//0;
  }
  if((strcmp(stringa,"no") == 0) || (strcmp(stringa,"n") == 0))
  {
    printf("Hai rifiutato la partita\n");
    strcpy(stringa,"NO ");
    strcat(stringa,nome_avversario);
    memset(nome_avversario,0,DIM_NOME_CLIENT);
    stato = 0;
    return ;//0;
  }
  else
  {
    printf("Comando non valido. Scrivi yes/y o no/n\n");
    richiesta_partita(stringa);
 	//return;
  }
}
void ins_combinazione()
{
  int i;
  int ret;

  memset(combinazione,0,6);
  printf("Digita la tua combinazione segreta: ");
  //scanf("%s",combinazione);
  //getc(stdin);
  fgets(combinazione,6,stdin);
  
  if(combinazione[strlen(combinazione)-1] != '\n' ) //buffer sporco
  {	
  	while ( ((ret= fgetc(stdin)) != EOF) && (ret!= '\n'));
  	printf("Combinazione non valida.Inserisci 4 cifre da [0-9]\n");
    ins_combinazione();
    return;
  }

  combinazione[4] = '\0';

  printf("La tua combinazione %s\n",combinazione);

 /* if(strlen(combinazione) >= 5 )
  {
  	printf("Combinazione non valida.Inserisci 4 cifre da [0-9]\n");
    ins_combinazione();
  }
  */
  for(i=0; i<4;i++)
  {
    if((combinazione[i] < '0') || (combinazione[i] > '9'))
    {
      printf("Combinazione non valida.Inserisci 4 cifre da [0-9]\n");
      ins_combinazione();
    }
  }
  return;
}

void inzia_partita()
{
  printf("%s ha accettato la partita\n",nome_avversario);
  printf("Partita avviata con %s\n",nome_avversario);
  stato = GAME_START;
  ins_combinazione();
  //turno = 1;
  if(turno == 1)
  	printf("E' il tuo turno\n");
  else
  	printf("E' il turno di %s\n",nome_avversario);

  return;
}

void inserisci_porta(char* porta_udp)
{
  int i;

  printf("Inserisci la porta UDP di ascolto: ");
  memset(porta_udp, 0, DIM_PORTA_UDP);
  //scanf("%s",porta_udp);
  //getc(stdin);
  fgets(porta_udp, DIM_PORTA_UDP, stdin);
  
  if((strlen(porta_udp) == DIM_PORTA_UDP-1)  && (porta_udp[strlen(porta_udp)-1] != '\n')) //buffer sporco
  {
    while ( ((i= fgetc(stdin)) != EOF) && (i!= '\n'));
    printf("Hai inserito piu' di 5 cifre\n");
    printf("Verra' creata una porta UDP con le prime 5 cifre, se comprese tra 1024 e 65535\n");
    porta_udp[5] = '\0';
    printf("Se valida la tua porta UDP sara' %s\n",porta_udp);
  }
  else
  	porta_udp[strlen(porta_udp)-1] = '\0';
  
  if((atoi(porta_udp) >= 65536) || (atoi(porta_udp) <= 1023) ) //controllo valore porta UDP digitato
  {
    printf("Errore valore porta UDP di ascolto\n");
    inserisci_porta(porta_udp);
  }
  for(i=0; i< strlen(porta_udp); i++)
  {
    if((porta_udp[i] > '9') || (porta_udp[i] < '0'))
    {
      printf("Porta UDP di ascolto non valida\n");
      inserisci_porta(porta_udp);
      return;
    }
  }
  
  return;
}

void login(int sk_tcp,char* nome_client, char* porta_udp, char* buffer_tcp)
{
  int ret;
  //int i;
  printf("Inserisci il tuo nome: ");
  memset(nome_client, 0, DIM_NOME_CLIENT);
  fgets(nome_client, DIM_NOME_CLIENT, stdin);
  if(nome_client[0] == '\n')
  {
  	printf("Non hai inserito alcun Nome\n");
  	//login(sk_tcp,nome_client,porta_udp,buffer_tcp);
  	printf("Ti assegno il nome 'default'\n");
  	memset(nome_client,0,DIM_NOME_CLIENT);
  	strcpy(nome_client,"default\n");
  	//return;
  }
  if((strlen(nome_client ) == DIM_NOME_CLIENT-1) && (nome_client[strlen(nome_client)] != '\n')) //buffer sporco
  {	
  	while ( ((ret= fgetc(stdin)) != EOF) && (ret!= '\n'));
  	printf("Hai inserito piu di %d caratteri.Il tuo Username sara'i primi %d caratteri\n",DIM_NOME_CLIENT,DIM_NOME_CLIENT);
  }
  nome_client[strlen(nome_client)-1] = '\0';
  
  inserisci_porta(porta_udp);
 
  //INVIO LOGIN(USERNAME,PORTA_UDP)
  memset(buffer_tcp,0,DIM_MSG_TCP);
  strcpy(buffer_tcp,"login ");
  strcat(buffer_tcp,nome_client);
  strcat(buffer_tcp," ");
  strcat(buffer_tcp,porta_udp);
  
 if((ret = invia_tcp(sk_tcp,buffer_tcp)) <= 0)
 {
    perror(NULL);
    close(sk_tcp);
    exit(-1);
 } 
}

int verifica_login()
{
  if(ricevi_tcp(sk_tcp,buffer_tcp) <= 0)
    return -1;

  if(strcmp(buffer_tcp,"NUOVO_NOME") == 0)
  {
    printf("Nome utente %s gia' utilizzato da un altro utente\n",nome_client);
    login(sk_tcp, nome_client, porta_udp, buffer_tcp); //invia nome utente e porta udp al server
    verifica_login();
    return 1;
  }
  if(strcmp(buffer_tcp,"OK") == 0)
  {
    printf("Login effettuato con successo\n");
    return 1;
  }

  return 0;
}

void help() //mostra la lista dei comandi
{
  printf("Sono disponibili i seguenti comdandi:\n");
  printf(" * !help  -->  mostra l'elenco dei comandi disponibili\n");
  printf(" * !who  -->  mostra l'elenco dei client connessi al server\n");
  printf(" * !connect nome_client  -->  avvia una partita con l'utente nome_client\n");
  printf(" * !disconnect  -->  disconnette il client dell'attuale partita intrapresa con un altro peer\n");
  printf("con un altro peer\n");
  printf(" * !combinazione comb  -->  permette al client di fare un tentativo con la combinazione comb\n");
  printf(" * !quit  --> disconnette il client dal server\n");
  printf("\n");
  return;
}

int valida_indirizzo(char *str, int dim)	//funzione controllo correttezza formale indirizzo host
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

int elabora_stringa_tcp(char* stringa)
{
  int i , j;
  char msg[DIM_MSG_TCP];

  memset(msg,0,DIM_MSG_TCP);

  for(i=0;  (stringa[i] != ' ') && (i<strlen(stringa)) ;i++);
  strncpy(msg,stringa,i);  
  i++;

  if( (strcmp(msg,"NO") == 0) ||(strcmp(msg,"WHO") == 0) )
  {
    //fflush(stdout);
    printf("%s",(stringa+i));
    return 1;
  }

  if(strcmp(msg,"PARTITA") == 0)
  {
    printf("%s vorrebbe iniziare una partita con te.\nAccetti ?\n",stringa+i);
    printf("Per accettare y(yes) o n(no) per rifiutare la partita\n");
    memset(nome_avversario,0,DIM_NOME_CLIENT);
    strcpy(nome_avversario,stringa+i);

    richiesta_partita(stringa);
    
    i = invia_tcp(sk_tcp,stringa);
    return i;
    
  }
  
  if(strcmp(stringa,"!disconnect") == 0)
  {
  	if( stato != GAME_START) //significa che ho ricevuto il messaggio UDP di vittoria prima!!
  	  return 1;

    printf("L'avversario si e' arreso.\n");
    printf("*********** V I T T O R I A ***************\n");
    stato = 0;

    return 1;
  }
  //riposta affermativa alla connect nuome_utente [YES Nome_Utente IP portaUDP]
  if(strcmp(msg,"YES") == 0)
  {
  	
    for(j=i; stringa[i] != ' '; i++); //scorro fino all'altro spazio
    memset(nome_avversario,0,DIM_NOME_CLIENT);	
    strncpy(nome_avversario,stringa+j,i-j);
    i++;
    for(j=i; stringa[i]!= ' ';i++);
    memset(ip_avversario,0,16);
    strncpy(ip_avversario,stringa+j,i-j);
    i++;
    strcpy(porta_udp_avversario,stringa+i);
    return 1000;
  }
  return 1;
}

int controlla_combinazione(char* stringa)
{
  int giusti ;
  int presenti;
  int i,j;
  char comb[5];
  for(i=0; stringa[i]!= ' '; i++);
  i++;
  strcpy(comb,stringa+i);
  i= 0;
  giusti=0;
  presenti =0;
  for(j=0; j<4;j++)
  {
    for(i=0;i<4;i++)
      if(comb[j] == combinazione[i])
      {
        if(j == i)
          giusti++;
        else
          presenti++;
      }
  }

  if(giusti == 4)
  {
    printf("%s ha indovinato la tua combinazione\n",nome_avversario);
    printf("*************** HAI PERSO ********************\n");
    memset(stringa,0,DIM_MSG_UDP);
    sprintf(stringa,"HAI VINTO");

    return 0;
  }
  else
  {
    printf("%s dice %s.Il suo tentativo e' sbagliato\n",nome_avversario,comb);
    memset(stringa,0,DIM_MSG_UDP);
    sprintf(stringa,"RISPOSTA %d %d",giusti,presenti);
    
    return 1;
  }
}

int elabora_stringa_udp(char* stringa)
{
  int ret;
  int i;
  int g,p;
  int ris;
  if(strcmp(stringa,"TIMEOUT") == 0)
  {
  	turno = 0;
  	stato = 0;
  	printf("E' scaduto il tempo a disposizione dell'avversario\n");
  	printf("******** VITTORIA ***********\n");
  	return 0;
  }

  for(i=0; (i < strlen(stringa)) && (stringa[i]!= ' ');i++);


  if(strncmp(stringa,"RISPOSTA",i) == 0) //[RISPOSTA G P]
  { //Risposta alla mia combinazione
  	g = atoi(stringa+9);
  	p = atoi(stringa+11);
    printf("%s dice: %d cifra giusta al posto giusto, ",nome_avversario,g);
    printf("%d cifre giuste al posto sbaglaito\n",p);
    turno = 0;
    printf("E' il turno di %s\n",nome_avversario);
    ris = 0;
  }
  
  if(strncmp(stringa,"!combinazione",i) == 0)
  {
    ret = controlla_combinazione(stringa);

    if(ret == 1)
    {
      turno = 1;
      printf("E' il tuo turno\n");
      ris = 1;
    }
    else
    { //HO PERSO!!! ----> invio "HAI VINTO"al client e "!disconnect " al server
      stato = 0;
      turno = 0;
      ris = 1;
    }
  }
  return ris;
}

int elabora_comando(char* stringa)
{
  int i ,j;
  
  //printf("elabora comando = %s  \n",stringa);

  if(strcmp(stringa,"!help") == 0)
  {
    if(strlen(stringa) != strlen("!help"))
      return 0;
    else
      return 1;
  } 

  if(strcmp(stringa,"!who") == 0)
    return 2;

  if(strcmp(stringa,"!disconnect") == 0) 
  {
    if(stato != GAME_START)
    {
      printf("Al momento non sei impegnato in una partita\n");
      return 0;
    }
    if(strlen(stringa) != strlen("!disconnect"))
      return 0;
    else
    {
      stato = 0;
      printf("Hai deciso di arrenderti.....\n");
      printf("Ti sei arreso. HAI PERSO \n");
      return 2; 
    }
  }

  if(strcmp(stringa,"!quit") == 0)
  {
    if(strlen(stringa) != strlen("!quit"))
      return 0;
    else
	  return 3;
  }

  for(i=0; (i < strlen(stringa)) && (stringa[i] != ' ')  ; i++);
  if(i == strlen(stringa))
   return 0;	
   //i++; 
  //stabilire quali sono i comandi ammessi
   
  if(strncmp(stringa,"!connect",i) == 0) //controlla formato [!connect nome_utente]
  {
    if(stato == GAME_START)
    {
      printf("Sei impegnato in una partita. !disconnect per abbandonare\n");
      return 0;
    }
    if(strlen(stringa) == strlen("!connect"))
      return 0;
    i++;
    for(; (stringa[i] != ' ') && (stringa[i]!= '\0') ; i++);
    if(stringa[i] == ' ')
      return 0;
    else
    { turno = 1;  return 2; }
  }

  if(strncmp(stringa,"!combinazione",i) == 0)
  {
    if(stato != GAME_START)
    {
      printf("Per usare !combinazione devi prima iniziare una partita\n");
      return 0;
    }
    if(strlen(stringa) == strlen("!combinazione"))
      return 0;
    if(turno == 0)
    {
      printf("Non e' il tuo turno.Aspetta la mossa dell'avversario\n");
      return 0;
    }
    for(j=i+1; j<strlen(stringa);j++)
    {
      if((stringa[j] <'0') || (stringa[j] > '9'))
	return 0;
    }
    if((j -(i+1)) !=4 )
      return 0;
    else
      return 4;
  }
  else
    return 0;
}

//FINE FUNZIONI

int max(int a, int b , int c)
{
  int ret;	
  if(a > b)
  {
    if(a > c)
      ret = a;
  }  
  else
  {  if( b > c)
      ret = b;
    else
      ret = c;
  }
  return ret;  
}

int main(int argc, char *argv[])
{
  //	Dichiarazione Variabili ,Strutture utilizzate
  
  struct sockaddr_in server_addr;
  struct sockaddr_in local_addr;
  int i, ret, nbyte, caso;
  char *ip_server; 		//conterra' <host_remoto> "127.0.0.1"
  unsigned short int p_server; 	//conterra' <porta_server> "1234"
  
  int set = 1;
  char comando[DIM_COMANDO];//conterra' i comandi digitati
  char buffer_udp[DIM_MSG_UDP];
  

  int sk_udp;
  
  //per la select()
  struct timeval timeout;
  struct timeval *tm = NULL;
    fd_set read_fd;
    int fd_pronti;
    int fdmax;
    FD_ZERO(&read_fd);
    
    timeout.tv_sec = 60;	//Durante una partita se per 1Minuto non si riceve niente(stdin,sockets) !disconnect
    timeout.tv_usec = 0;
  //
  
  //	controllo sui parametri immessi
  if(argc == 3)
  {
    if(valida_indirizzo(argv[1],strlen(argv[1])) < 0 )
    {
      printf("Errore nei parametri passati al comando ./mastermind_client \nIndirizzo <host remoto> errato\n");
      exit(-1);
    }
   
    if(atoi(argv[2]) >= 65536)
    {
      printf("Errore nei paramtri passati al comando ./mastermind_client \nIndirizzo <porta> errata\n");
      exit(-1);
    }
  }
  else
  {
    printf("Errore nel numero di parametri passati al comando ./mastermind_client \n");
    exit(-1);
  }
  
  //creo la socket per la connessione
  sk_tcp = socket(AF_INET, SOCK_STREAM, 0); //socket TCP ,prot IPv4
  if(sk_tcp < 0)
  {
    printf("Error: socket() failed \n");
    exit(-1);
  }
  if(setsockopt(sk_tcp, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int)) == -1 )
  {
    printf("Error: setsockopt() failed\n");
    perror(NULL);
    close(sk_tcp);
    exit(-1);
  }

  ip_server = argv[1];	//indirizzo <host remoto>
  p_server = (unsigned short int)atoi(argv[2]); // porta <host remoto>
  
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(p_server);
  
  if(inet_pton(AF_INET, ip_server, &server_addr.sin_addr.s_addr) < 0)
  {
    printf("Error: inet_pton() failed \n");
    exit(-1);
  }
  
  ret = connect(sk_tcp, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if(ret < 0) 
  {
    printf("Error: connect() failed \n");
    perror(NULL);
    close(sk_tcp);
    exit(-1);
  }
  
  //CONNESSIONE TCP CON IL SERVER EFFETTUATA
  printf("Connessione al server %s (porta: %d) effettuata con successo \n",ip_server,p_server);
  printf("\n");
  
  help(); 		//mostra la lista dei comandi 
  
  login(sk_tcp, nome_client, porta_udp, buffer_tcp); //invia nome utente e porta udp al Server
  //fflush(0);
  if(verifica_login() == -1) //aspetto risposta affermativa dal Server
  {
    perror(NULL);
    close(sk_tcp);
    exit(-1);
  }
    
  //creo la porta ascolto UDP per giocare  ++++++++ DA CONTROLLARE ++++++++

  if((sk_udp = socket(AF_INET, SOCK_DGRAM, 0 /*IPPROTO_UDP*/)) < 0)
  {
    printf("Error: socket_udp() failed\n");
    perror(NULL);
    exit(-1);
  }
  if(setsockopt(sk_udp, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int)) == -1 )
  {
    printf("Error: setsockopt() failed\n");
    perror(NULL);
    close(sk_udp);
    exit(-1);
  }
  
  
  memset(&local_addr, 0 , sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(atoi(porta_udp));
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  
  if(bind(sk_udp, (struct sockaddr*)&local_addr,sizeof(local_addr)) < 0) 
  {
    printf("Error: bind_udp() failed\n");
    perror(NULL);
    exit(-1);
  }
  
    
 
/**************************** INIZIO   DELLA   SESSIONE   DI   GIOCO   **************/ 
  for(;;)
  {
    FD_SET(sk_tcp ,&read_fd);
    FD_SET(sk_udp, &read_fd);  
    FD_SET(fileno(stdin),&read_fd);
    fdmax = max(sk_tcp, sk_udp, fileno(stdin)); 
    
    if(stato == GAME_START)
      printf("# ");
    else
      printf("> ");
    fflush(stdout); 

    if(stato == GAME_START)
    {
       timeout.tv_sec = 60;
       timeout.tv_usec = 10;
       tm = &timeout;//(struct timveval*)&timeout;
    }	
    else
    	tm = NULL;
	 
    if((fd_pronti = select(fdmax+1, &read_fd, NULL,NULL, tm)) <= 0)
    {
      if(fd_pronti < 0)
      {
	      printf("Error: select() failed\n");
	      perror(NULL);
	      exit(-1);
      }
      if( fd_pronti == 0)  //scaduto il timeout ..ATTENZIONE TIMEOUT ASSOCIATO SOLO ALLO STDIN E ALLA SK_UDP 
      {
	      if( stato == GAME_START ) //devo disconnettere il client
	      {    
	        printf("E' passato piu di un minuto \n");

	        memset(buffer_udp,0,DIM_MSG_UDP);
	        strcpy(buffer_udp,"TIMEOUT");
	        if(invia_udp(sk_udp,buffer_udp) <= 0)
	        {
	        	printf("Error: invia_udp_timeout() failed.Termino connessione UDP\n");
	        	close(sk_udp);
	        	FD_CLR(sk_udp,&read_fd);
	        	//break;
	        }
    		
    		  stato = 0;
    		  turno = 0;
    		  //tm = NULL;
	        memset(buffer_tcp,0,DIM_MSG_TCP);		
	        strcpy(buffer_tcp, "!disconnect"); //informo il Server
	        if(invia_tcp(sk_tcp, buffer_tcp) <= 0)
	        {
	          printf("Error:invia_tcp_timeout() failed. Crollo connessione con il server!\n");
	          perror(NULL);
	          close(sk_tcp);
	          FD_CLR(sk_tcp,&read_fd);
	          exit(-1);
	        }
          memset(combinazione,0,5);
          memset(nome_avversario,0,DIM_NOME_CLIENT);
	        printf("Disconnessione avvenuta con successo: TI SEI ARRESO \n");
	        //break;
	      }
      }  
    }
      
    for(i=0 ; i<= fdmax ; i++)
    {
      if(FD_ISSET(i, &read_fd)) //descrittore settato a 1
      {
	      if(i == sk_tcp)	//arrivo messaggio dal socket tcp
	      {  
	        memset(buffer_tcp, 0 ,DIM_MSG_TCP);

          	if((nbyte = ricevi_tcp(i, buffer_tcp)) <= 0)
	        {
	          printf("Error:tcp-recv() failed\n"); //errore o connessione chiusa dal Server
	          perror(NULL);
	          close(sk_tcp);
	          FD_CLR(sk_tcp, &read_fd);
	          exit(-1);
	        }
			
	        if((nbyte = elabora_stringa_tcp(buffer_tcp)) <= 0)
	        {
	          perror(NULL);
	          close(sk_tcp);
	          FD_CLR(sk_tcp,&read_fd);
	          exit(-1);
	        }
	        	
	        if( nbyte == 1000) //gestione inzio partita
	        {
	          memset(&gamer_addr, 0 , sizeof(gamer_addr));
	          gamer_addr.sin_family = AF_INET;
	          gamer_addr.sin_port = htons(atoi(porta_udp_avversario));
	          
	          //printf("ip avversario :%s e porta_udp:%s\n",ip_avversario,porta_udp_avversario);

	          if(inet_pton(AF_INET, ip_avversario, &gamer_addr.sin_addr.s_addr) < 0) //inizializza il campo sin_addr.s_addr;
	          {
		          printf("Error: inet_pton() failed \n");
		          close(sk_tcp);
    			  FD_CLR(sk_tcp,&read_fd);
		          exit(-1);
	          }
	          
	          ret = connect(sk_udp, (struct sockaddr *)&gamer_addr, sizeof(gamer_addr));
			      if(ret < 0) 
  			    {
    			    printf("Error: connect-udp() failed \n");
    			    perror(NULL);
    			    close(sk_udp);
    			    FD_CLR(sk_udp,&read_fd);
    			    close(sk_tcp);
    			    FD_CLR(sk_tcp,&read_fd);
    			    exit(-1);
  			    }

  			    FD_SET(sk_udp, &read_fd);

	          inzia_partita(); 
			
	        
	        }

	        break;
	      }
	
	     if(i == sk_udp)	 //arrivo messaggio dalla porta udp
	     {	
   		    nbyte = ricevi_udp(sk_udp,buffer_udp);
	        if(nbyte <= 0)
	        {
	          printf("Error: udp-read() failed\n");
	          perror(NULL);
	          close(sk_udp);
	          FD_CLR(sk_udp,&read_fd);
	          break;
	        }
	  
	        if(strcmp(buffer_udp,"HAI VINTO") == 0)
          	{
            	printf("Hai indovinato la combinazione di %s\n",nome_avversario);
            	printf("********VITTORIA********\n");
            	memset(combinazione,0,5);
            	memset(nome_avversario,0,DIM_NOME_CLIENT);
           		stato = 0;
            	turno = 0;  
            	break;          	
            }
	        if(elabora_stringa_udp(buffer_udp) == 1) //  DA RIVERE COSA FACCIO QUANDO RITORNO
	        {
	        	//  printf("stringa_udp =%s\n",buffer_udp);

           	  nbyte = invia_udp(sk_udp,buffer_udp);  //CONTROLLARE

           	  if(nbyte <= 0)
           	  {
           	  	 stato = 0;
           	  	 printf("Error: invia_udp() failed. Chiudo connessione UDP\n");
           		 close(sk_udp);
           		 //close(sk_tcp);
           		 FD_CLR(sk_udp,&read_fd);
           		 //FD_CLR(sk_tcp,&read_fd);
           		 //exit(-1);
           		 break;
           	  }

           	  if(strcmp(buffer_udp,"HAI VINTO") == 0)  //
           	  {
           	  	memset(buffer_tcp,0,DIM_MSG_TCP);
              	strcpy(buffer_tcp,"!disconnect");
              	nbyte = invia_tcp(sk_tcp,buffer_tcp);
              	if(nbyte <= 0)
              	{
              	  perror(NULL);
               	  close(sk_tcp);
               	  close(sk_udp);
                  exit(-1);
              	}
              	memset(combinazione,0,5);
            	  memset(nome_avversario,0,DIM_NOME_CLIENT);
            	  //close(sk_udp);
            	  //FD_CLR(sk_udp,&read_fd);
              }
	        }

	        break;
        }	
    
      	if(i == STDIN)	//segnale dallo standard_input
        {
          memset(comando,0,DIM_COMANDO);
	      fgets(comando, sizeof(comando), stdin);
	      if((strlen(comando) == DIM_COMANDO-1) && (comando[strlen(comando)] != '\n')) //buffer sporco
  			while ( ((ret= fgetc(stdin)) != EOF) && (ret!= '\n'));
		  
	      comando[strlen(comando)-1] = '\0';
	  
	      caso = elabora_comando(comando);
	      
	      //printf("comando=%s,caso=%d\n",comando,caso);

	      switch(caso)
	     {
	       case 0:
	       {
	          printf("Comando inserito non valido.\n");
	          printf("Digita nel modo corretto e Assicurati che il comando possa essere utilizzato\n");
	          printf("Per la lista dei comandi digita !help\n");
	          break;
	        }
	    
	        case 1: //help
	        {
	          help();
	          break;
	        }
	        case 2: //who //connect //disconnect 
	        {
	          if((nbyte = invia_tcp(sk_tcp,comando)) <= 0)
	          {
	            printf("Error:send() failed. Crollo connessione con il server!\n");
	            perror(NULL);
	            close(sk_tcp);
	            FD_CLR(sk_tcp,&read_fd);
	            exit(-1);
	          } 
	          break;
	        }
	        case 3://quit
	        {
	          if(stato == GAME_START)
	          {
	            printf("Uscendo abbandonerai la partita in corso\n");
	            memset(buffer_tcp,0,sizeof(buffer_tcp));
	            strcpy(buffer_tcp,"!disconnect");
	            nbyte = invia_tcp(sk_tcp,buffer_tcp);
	            if(nbyte <= 0)
	            {
		            printf("Error:send() failed. Crollo connessione con il server!\n");
		            perror(NULL);
		            close(sk_tcp);
		            FD_CLR(sk_tcp,&read_fd);
		            exit(-1);
	            }
	          }  	
	          printf("Disconnessione in corso...\n");
	          if((nbyte =invia_tcp(sk_tcp,comando)) <= 0)
	          {
	            printf("Error:send() failed. Crollo connessione con il server!\n");
	            perror(NULL);
	            close(sk_tcp);
	            FD_CLR(sk_tcp,&read_fd);
	            exit(-1);
	          } 
	          printf("Aspetto risposta dal Server per Terminare...\n");
	          ricevi_tcp(sk_tcp,buffer_tcp);
	          if(strcmp(buffer_tcp,"QUIT")!= 0)
	            printf("Risposta non arrivata.Termino lo stesso\n");
	          else
	            printf("++++++++++++++++++++ FINE ++++++++++++++++++++\n");
	          close(sk_tcp);
	          close(sk_udp);
	          FD_CLR(sk_tcp,&read_fd);
	          FD_CLR(sk_udp,&read_fd);
	          return 0;
	        }
	        case 4://!combinazione comb MEssaggio udp
	        {
              turno = 0;
           	  memset(buffer_udp,0,sizeof(buffer_udp));
              strcpy(buffer_udp,comando);
	          nbyte = invia_udp(sk_udp,buffer_udp);

              if(nbyte <= 0)
           	  {
           	  	stato = 0;
           	  	printf("Error: invia_udp() failed. Chiudo connessione UDP\n");
           		close(sk_udp);
           		//close(sk_tcp);
           		//FD_CLR(sk_udp,&read_fd);
           		FD_CLR(sk_tcp,&read_fd);
           		//exit(-1);
           	  }

	          break;
	        }
	    
	      }//fine switch()
	 
      }//if(i==stdin)
	
    } //fine if(FD_ISsET())
     
  }//for(i;i<=fdmax..)
  
  
 }//fine for(;;) 
 
  

 
 return (1);
}

