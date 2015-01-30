/* ****************************************
 *  
 *    Programacao - MEEC - 2011/2012
 *    Projecto Final de Programação
 *    João Cardoso nº72673
 *    Luís Gomes nº72904
 *
 * **************************************** */
 
 
#define _BSD_SOURCE
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define MAXPENDING 10    /* Max connection requests */
#define BUFFSIZE 1024
void Die(char *mess) { perror(mess); exit(1); }


int cria_socket(){
    int serversock;
    struct sockaddr_in echoserver;
    char host[1024];
    struct hostent *host_entry;
    char * localIP;
    int port_number;
	
	
    port_number=5000;
    /* Create the TCP socket */
    if ((serversock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("impossivel criar socket. O servidor vai sair.\n");
		exit(1);
    }
	
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family=AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr=htonl(INADDR_ANY);   /* Incoming addr */
    port_number--;
    do{
        port_number++;
        echoserver.sin_port=htons(port_number);       /* server port */
		
        /* Bind the server socket */
    }while(bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0);
	
    /*obter o nome da máquina */
    gethostname(host, 1024);
    host_entry=gethostbyname(host);
    localIP=inet_ntoa (*(struct in_addr *)*host_entry->h_addr_list);
    printf("\n\n\tuse um dos seguintes enderecos no navegador: \n");
    printf("\t\thttp://%s:%d/\n", host, port_number);
    printf("\tou\n");
    printf("\t\tServidor em: http://%s:%d/\n", localIP, port_number);
    printf("\n\n\tSe necessária de autorizacao na Firewal.\n\n"); 
	
    /* Escuta pedidos */
    if (listen(serversock, MAXPENDING) < 0) {
        perror("Impossível criar o socket. O servidor vai sair.\n");
		exit(1);
    }
    return serversock;
}

void  espera_pedido(int serversock, FILE** fp_read, FILE ** fp_write, char *remote_address){
    int clientsock;
    struct sockaddr_in  echoclient;

    unsigned int clientlen=sizeof(echoclient);
    /* Wait for client connection */
    if ((clientsock=accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
        Die("Failed to accept client connection");
    }
    sprintf(remote_address, "%s", inet_ntoa(echoclient.sin_addr));

    *fp_read=fdopen(clientsock, "r");
    *fp_write=fdopen(clientsock, "w");

}

typedef struct pedido{
	char nome_ficheiro[BUFFSIZE],sistema_operativo[10],navegador[10],endereco_cliente[100];
	int erro, lido;
	struct pedido *apontado;
} pedido;
void Actualizar_Pedidos(pedido *base, FILE *fp_write){
	pedido *aux;
	aux=base;
	while(aux!=NULL){
		fprintf(fp_write,"<br />%-26s %d %7s %9s %s",aux->nome_ficheiro,aux->erro,aux->navegador,aux->sistema_operativo,aux->endereco_cliente);
		aux=aux->apontado;
	}
}
void Actualizar_UltimoFicheiro(pedido *base, FILE *fp_write){
	pedido *aux;
	char ultimo[100];
	aux=base;
	while(aux->apontado!=NULL)
		aux=aux->apontado;
	fprintf(fp_write,"<BODY><H2>Ultimo ficheiro: %s</H2><pre>",aux->nome_ficheiro);
	sprintf(ultimo,"%s",aux->nome_ficheiro);
	aux=base;
	while(aux!=NULL){
		if(strcmp(aux->nome_ficheiro,ultimo)==0)
			fprintf(fp_write,"<br />%-26s %d %7s %9s %s",aux->nome_ficheiro,aux->erro,aux->navegador,aux->sistema_operativo,aux->endereco_cliente);
		aux=aux->apontado;
	}
}
pedido *Actualizar_Clear(pedido *base){
	pedido *aux;
	pedido *clear;
	aux=base;
	while(aux!=NULL){
		if(aux!=NULL && strstr(aux->nome_ficheiro,"/estatisticas/")==NULL){
			clear=aux;
			aux=aux->apontado;
			free(clear);
		}else{
			if(aux->apontado!=NULL && strstr(aux->apontado->nome_ficheiro,"/estatisticas/")==NULL){
				clear=aux->apontado;
				aux->apontado=clear->apontado;
				free(clear);
			}
			aux=aux->apontado;
		}
	}
	return base;
}
pedido *Actualizar_ClearAll(pedido *base){
	FILE *fp_save;
	pedido *aux;
	fp_save=fopen("http_docs/estatisticas/Salvaguarda.txt", "w+");
	fclose(fp_save);
	while(base!=NULL){
		aux=base;
		base=base->apontado;
		free(aux);
	}
	return base;
}
void inicializar_lido(pedido *base){
	pedido *aux;
	aux=base;
	while(aux!=NULL){
		aux->lido=0;
		aux=aux->apontado;
	}
}
void Actualizar_Browser(pedido *base, FILE *fp_write){
	pedido *aux;
	pedido *sub_aux;
	pedido *aux1;
	float cont_n_op;
	int outro_n_op;
	int op;
	int n_op;
	int n_op_percentagem;
	int frequente;
	char porcento;
	char navegador[10];
	char sistema_operativo[10];
	porcento='%';
	frequente=0;
	op=0;
	aux=base;
	outro_n_op=1;
	while(aux!=NULL && outro_n_op==1){
		cont_n_op=0;
		outro_n_op=0;
		sub_aux=aux;
		while(sub_aux!=NULL){
			if(strcmp(sub_aux->navegador,aux->navegador)==0 && strcmp(sub_aux->sistema_operativo,aux->sistema_operativo)==0){
				cont_n_op++;
				sub_aux->lido=1;
				if(outro_n_op==0 && sub_aux->apontado==NULL){
					aux1=NULL;
				}
			}else{
				if(outro_n_op==0 && sub_aux->lido!=1){
					aux1=sub_aux;
					outro_n_op=1;
				}
			}
			if(aux==base){
				op=op+1;
			}
			sub_aux=sub_aux->apontado;
		}
		n_op=cont_n_op;
		n_op_percentagem=(cont_n_op/op)*100;
		if(cont_n_op>=1){
			fprintf(fp_write,"<br/>%-26s %7s %9d (%d%c)",aux->navegador,aux->sistema_operativo,n_op, n_op_percentagem,porcento);
		}
		if(cont_n_op>=frequente){
			strcpy(navegador,aux->navegador);
			strcpy(sistema_operativo,aux->sistema_operativo);
			frequente=cont_n_op;
		}
		aux=aux1;
	}
	fprintf(fp_write,"</pre><BODY><H2>Navegador e Sistema operativo mais frequente</H2>\n");	
	fprintf(fp_write,"%s / %s - %d acessos",navegador,sistema_operativo,frequente);
	inicializar_lido(base);
}
void Actualizar_Ficheiros(pedido *base, FILE *fp_write){
	pedido *aux;
	pedido *sub_aux;
	pedido *aux1;
	int cont_ficheiro;
	int outro_ficheiro;
	aux=base;
	outro_ficheiro=1;
	while(aux!=NULL && outro_ficheiro==1){
		cont_ficheiro=0;
		outro_ficheiro=0;
		sub_aux=aux;
		while(sub_aux!=NULL){
			printf("%s\n",sub_aux->nome_ficheiro);
			if(strcmp(sub_aux->nome_ficheiro,aux->nome_ficheiro)==0){
				cont_ficheiro++;
				sub_aux->lido=1;
				if(outro_ficheiro==0 && sub_aux->apontado==NULL){
					aux1=NULL;
				}
			}else{
				if(outro_ficheiro==0 && sub_aux->lido!=1){
					aux1=sub_aux;
					outro_ficheiro=1;
				}
			}
			sub_aux=sub_aux->apontado;
		}
		if(cont_ficheiro>=1){
			fprintf(fp_write,"<br/>%-26s %9d",aux->nome_ficheiro,cont_ficheiro);
		}
		aux=aux1;
	}
	inicializar_lido(base);
}
void Actualizar_Tipo(pedido *base,FILE *fp_write,char CT[2][15][25]){
	pedido *aux;
	int cont_tipo;
	int aux_total;
	int linha_ct;
	int op;
	linha_ct=0;
	aux_total=0;
	aux=base;
	while(linha_ct!=15 && aux!=NULL){
		op=0;
		cont_tipo=0;
		while(aux!=NULL){
			if((strstr(aux->nome_ficheiro,CT[0][linha_ct])!=0 || ((strstr(aux->nome_ficheiro,"/estatisticas/")!=0 || aux->nome_ficheiro[strlen(aux->nome_ficheiro)-1]=='/') && strstr(CT[0][linha_ct],".html")!=0)) && aux->lido==0){
				cont_tipo++;
				aux->lido=1;
			}
			op=op+1;
			aux=aux->apontado;
		}
		if(cont_tipo>>0){
			fprintf(fp_write,"<br/>%-26s %9d",CT[0][linha_ct],cont_tipo);
		}
		aux=base;
		aux_total=aux_total+cont_tipo;
		linha_ct++;
	}
	aux_total=op-aux_total;
	if(aux_total>>0){
		fprintf(fp_write,"</pre><br/>Outro %d",aux_total);
	}
	inicializar_lido(base);
}
void Escrever_Pedidos(pedido *base,FILE *fp_write,int versao_http){
	fprintf(fp_write,"HTTP/1.%c 200 OK\n",versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");
	fprintf(fp_write,"<HTML><HEAD><TITLE>Pedidos</TITLE></HEAD>");
	fprintf(fp_write,"<BODY><H2>Lista de Pedidos</H2><pre>");
	if(base==NULL){
		fprintf(fp_write,"<br/>Nao foi registado nenhum pedido");
	}else{
		Actualizar_Pedidos(base,fp_write);
	}
	fprintf(fp_write,"</pre></BODY></HTML>");
}	
void Escrever_UltimoFicheiro(pedido *base,FILE *fp_write,int versao_http){
	fprintf(fp_write,"HTTP/1.%c 200 OK\n",versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");	
	fprintf(fp_write,"<HTML><HEAD><TITLE>UltimoFicheiro</TITLE></HEAD>");
	if(base==NULL){
		fprintf(fp_write,"<br/>Nao foi registado nenhum pedido");
	}else{
		Actualizar_UltimoFicheiro(base,fp_write);
	}
	fprintf(fp_write,"</pre></BODY></HTML>");
}
void Escrever_Clear(pedido *base,FILE *fp_write,int versao_http){
	fprintf(fp_write,"HTTP/1.%c 200 OK\n", versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");
	fprintf(fp_write,"<HTML><HEAD><TITLE>Clear</TITLE></HEAD>");
	if(base==NULL){
		fprintf(fp_write,"<BODY><br/>Nao foi registado nenhum pedido</BODY></HTML>");
	}else{
		fprintf(fp_write,"<BODY><H2>Registos de acesso a ficheiros eliminados</H2></BODY></HTML>");
	}
}
void Escrever_ClearAll(pedido *base,FILE *fp_write,int versao_http){
	fprintf(fp_write,"HTTP/1.%c 200 OK\n",versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");
	fprintf(fp_write,"<HTML><HEAD><TITLE>ClearAll</TITLE></HEAD>");
	if(base==NULL){
		fprintf(fp_write,"<BODY><br/>Nao foi registado nenhum pedido</BODY></HTML>");
	}else{
		fprintf(fp_write,"<BODY><H2>Todos os registos eliminados</H2></BODY></HTML>");
	}
}
void Escrever_Browser(pedido *base,FILE *fp_write,int versao_http){
	fprintf(fp_write,"HTTP/1.%c 200 OK\n",versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");	
	fprintf(fp_write,"<HTML><HEAD> <TITLE>Browser</TITLE></HEAD>");
	fprintf(fp_write,"<BODY><H2>Navegadores e Sistemas operativos</H2><pre>");
	if(base==NULL){
		fprintf(fp_write,"<br/>Nao foi registado nenhum pedido");
	}else{
		Actualizar_Browser(base,fp_write);
	}
	fprintf(fp_write,"</BODY></HTML>");
}
void Escrever_Ficheiros(pedido *base,FILE *fp_write,int versao_http){
	fprintf(fp_write,"HTTP/1.%c 200 OK\n",versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");	
	fprintf(fp_write,"<HTML><HEAD><TITLE>Ficheiros</TITLE></HEAD>");
	fprintf(fp_write,"<BODY><H2>Ficheiros</H2><pre>");
	if(base==NULL){
		fprintf(fp_write,"<br/>Nao foi registado nenhum pedido");
	}else{
		Actualizar_Ficheiros(base,fp_write);
	}
	fprintf(fp_write,"</pre></BODY></HTML>");
	
}
void Escrever_Tipo(pedido *base,FILE *fp_write,int versao_http,char CT[2][15][25]){
	fprintf(fp_write,"HTTP/1.%c 200 OK\n",versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");	
	fprintf(fp_write,"<HTML><HEAD><TITLE>Tipo</TITLE></HEAD>");
	fprintf(fp_write,"<BODY><H2>Ficheiro</H2><pre>");
	if(base==NULL){
		fprintf(fp_write,"<br/>Nao foi registado nenhum pedido");
	}else{
		Actualizar_Tipo(base,fp_write,CT);
	}
	fprintf(fp_write,"</BODY></HTML>");
}
void Escrever_erro(FILE *fp_write,int versao_http,char nome_ficheiro[BUFFSIZE]){
	fprintf(fp_write,"HTTP/1.%c 404 Not Found\n",versao_http);
	fprintf(fp_write,"Content-Type:text/html\n\n");
	fprintf(fp_write,"\n" );
	fprintf(fp_write,"<HTML><HEAD> <TITLE>404 Not Found</TITLE></HEAD>");
	fprintf(fp_write,"<BODY> <H2>Not Found</H2>\n");
	fprintf(fp_write,"O ficheiro %s nao foi encontrado.</BODY></HTML>",nome_ficheiro);
}
int Actualizar_estatistica(char nome_ficheiro[BUFFSIZE], FILE *fp_write, char versao_http, pedido *base,char CT[2][15][25],int *erro){
	*erro=404;
	if(strcmp(nome_ficheiro,"/estatisticas/Pedidos")==0){
		*erro=200;
		Escrever_Pedidos(base,fp_write,versao_http);
	}
	if(strcmp(nome_ficheiro,"/estatisticas/UltimoFicheiro")==0){
		*erro=200;			
		Escrever_UltimoFicheiro(base,fp_write,versao_http);
	}
	if(strcmp(nome_ficheiro,"/estatisticas/Clear")==0){
		*erro=200;
		Escrever_Clear(base,fp_write,versao_http);
	}
	if(strcmp(nome_ficheiro,"/estatisticas/ClearAll")==0){
		*erro=200;
		Escrever_ClearAll(base,fp_write,versao_http);
	}
	if(strcmp(nome_ficheiro,"/estatisticas/Browser")==0){
		*erro=200;
		Escrever_Browser(base,fp_write,versao_http);
	}
	if(strcmp(nome_ficheiro,"/estatisticas/Ficheiros")==0){
		*erro=200;
		Escrever_Ficheiros(base,fp_write,versao_http);
	}
	if(strcmp(nome_ficheiro,"/estatisticas/Tipo")==0){
		*erro=200;
		Escrever_Tipo(base,fp_write,versao_http,CT);
	}
	/* Enviar erro se não identificar o pedido */
	if(*erro==404){
		Escrever_erro(fp_write,versao_http,nome_ficheiro);
	}
	return *erro;
}
pedido *cria_pedido(pedido *base,char str_pedido[100]){
	pedido *novo;
	pedido *aux;
	novo=malloc(sizeof(pedido));				
	sscanf(str_pedido," %s %d %s %s %s",novo->nome_ficheiro,&novo->erro,novo->navegador,novo->sistema_operativo,novo->endereco_cliente);
	novo->lido=0;
	novo->apontado=NULL;
	if(base==NULL){
		base=novo;
	}else{
		aux=base;
		while(aux->apontado!=NULL){
			aux=aux->apontado;
		}
		aux->apontado=novo;
	}
	return base;
}
pedido *criar_estatistica(){
	FILE * fp_save;
	pedido * base;
	char str_aux[100];
	base=NULL;
	fp_save=fopen("http_docs/estatisticas/Salvaguarda.txt", "a+");
	fclose(fp_save);
	fp_save=fopen("http_docs/estatisticas/Salvaguarda.txt", "r");
	while(fgets(str_aux,100,fp_save)!=NULL){
		base=cria_pedido(base,str_aux);
	}
	fclose(fp_save);
	return base;
}

pedido * Actualizar_lista(char nome_ficheiro[BUFFSIZE],pedido *base){
	if(strcmp(nome_ficheiro,"/estatisticas/Clear")==0){
		base=Actualizar_Clear(base);
	}
	if(strcmp(nome_ficheiro,"/estatisticas/ClearAll")==0){
		base=Actualizar_ClearAll(base);
	}
	return base;
}
pedido * Actualizar_salvaguarda(char str_pedido[100],pedido *base){
	FILE *fp_save;
	base=cria_pedido(base,str_pedido);
	fp_save=fopen("http_docs/estatisticas/Salvaguarda.txt","a");
	fprintf(fp_save,"%s",str_pedido);
	fclose(fp_save);
	return base;
}
int main(int argc, char *argv[]){
	FILE *fp_read, *fp_write;
	int serversock;
	char endereco_cliente[100];

	char buffer[BUFFSIZE],primeira_linha[BUFFSIZE],nome_ficheiro[BUFFSIZE],nome_fich_relativo[BUFFSIZE];

	FILE *ficheiro;
	int n_dados;
	
	int erro;
	char versao_http;
	char sistema_operativo[10],navegador[10],SO_N[2][4][10],CT[2][15][25];
	int linha_so_n,linha_ct;

	char str_pedido[100];
	pedido *base;

	base=criar_estatistica();
	serversock=cria_socket();

	/* Criar ficheiro se não existir */
	ficheiro=fopen("http_docs/estatisticas/Salvaguarda.txt","a");
	fclose(ficheiro);
	/* Criar matriz SO_N */
	strcpy(SO_N[0][0],"Macintosh");
	strcpy(SO_N[0][1],"Windows");
	strcpy(SO_N[0][2],"Android");
	strcpy(SO_N[0][3],"Linux");
	strcpy(SO_N[1][0],"Chrome");
	strcpy(SO_N[1][1],"Firefox");
	strcpy(SO_N[1][2],"Wget");
	strcpy(SO_N[1][3],"Opera");
	/* Criar matriz CT */
	strcpy(CT[0][0],".js");
 	strcpy(CT[0][1],".pdf");
	strcpy(CT[0][2],".mp3");
	strcpy(CT[0][3],".gif");
	strcpy(CT[0][4],".jpeg");
	strcpy(CT[0][5],".jpg");
	strcpy(CT[0][6],".ico");
	strcpy(CT[0][7],".png");
	strcpy(CT[0][8],".tiff");
	strcpy(CT[0][9],".tif");
	strcpy(CT[0][10],".css");
	strcpy(CT[0][11],".html");
	strcpy(CT[0][12],".htm");
	strcpy(CT[0][13],".txt");
	strcpy(CT[0][14],".mp4");
	strcpy(CT[1][0],"application/javascript");
	strcpy(CT[1][1],"application/pdf");
	strcpy(CT[1][2],"audio/mpeg");
	strcpy(CT[1][3],"image/gif");
	strcpy(CT[1][4],"image/jpeg");
	strcpy(CT[1][5],"image/jpeg");
	strcpy(CT[1][6],"image/x-icon");
	strcpy(CT[1][7],"image/png");
	strcpy(CT[1][8],"image/tiff");
	strcpy(CT[1][9],"image/tiff");
	strcpy(CT[1][10],"text/css");
	strcpy(CT[1][11],"text/html");
	strcpy(CT[1][12],"text/html");
	strcpy(CT[1][13],"text/plain");
	strcpy(CT[1][14],"video/mp4");

	/* Executa até termidado (Ctrl+C) */
	while(1){
		espera_pedido(serversock,&fp_read,&fp_write,endereco_cliente);

		fgets(primeira_linha,BUFFSIZE,fp_read);
		
		/* leitura e verificacao do fim do pedido */
		while(fgets(buffer, BUFFSIZE,fp_read)!=NULL && strcmp(buffer,"\r\n")!=0){
			/* User_Agent ? */
			if(strstr(buffer,"User-Agent")!=0){
				/* Verificar SO e Navegador */
				linha_so_n=0;
				strcpy(sistema_operativo,"Macintosh");
				strcpy(navegador,"Chrome");
				while(linha_so_n!=4){
					if(strstr(buffer, sistema_operativo)==0){
						strcpy(sistema_operativo,SO_N[0][linha_so_n]);
					}
					if(strstr(buffer,navegador)==0){
						strcpy(navegador,SO_N[1][linha_so_n]);
					}
					linha_so_n++;
				}
				if(strstr(buffer,sistema_operativo)==0){
					strcpy(sistema_operativo,"Outro");
				}
				if(strstr(buffer,navegador)==0){
					strcpy(navegador,"Outro");
				}
			}
		} /* fim leitura do pedido */

		/* primeira linha valida? */
		erro=200;
		strcpy(nome_ficheiro,"");
		versao_http='1';
		n_dados=sscanf(primeira_linha," GET %s HTTP/1.%c",nome_ficheiro,&versao_http);
		
		if(n_dados!=2 || strcmp(buffer,"\r\n")!=0){
			erro=400;
		}else{
			if(versao_http!='1' && versao_http!='0'){
				erro=400;
			}
		}

		if(erro==400){
			/* envio mensagem erro 400 */
			fprintf(fp_write,"HTTP/1.%c 400 Bad Request\n",versao_http);
			fprintf(fp_write,"\n" );
		}else{
			/* actualizar estatísticas se possuir a directoria /estatistica/*/
			if(strstr(nome_ficheiro,"/estatisticas/")!=0 && nome_ficheiro[strlen(nome_ficheiro)-1]!='/'){/**/
				erro=Actualizar_estatistica(nome_ficheiro,fp_write,versao_http,base,CT,&erro);/**/
			}else{
				/* actualizacao nome do ficheiro */
				if(nome_ficheiro[strlen(nome_ficheiro)-1]=='/'){
					sprintf(nome_fich_relativo,"http_docs%sindex.html",nome_ficheiro);
				}else{
					sprintf(nome_fich_relativo,"http_docs%s",nome_ficheiro);
				}	
				/* abertura do ficheiro */
				ficheiro=fopen(nome_fich_relativo, "r");
				/* ficheiro existe? */
				if(ficheiro==NULL){
					erro=404;
					/* envio mensagem de erro 404 */
					fprintf(fp_write,"HTTP/1.%c 404 Not Found\n",versao_http);
					fprintf(fp_write,"Content-Type:text/html\n\n");
					fprintf(fp_write,"\n");
					fprintf(fp_write,"<HTML><HEAD> <TITLE>404 Not Found</TITLE></HEAD>");
					fprintf(fp_write,"<BODY> <H2>Not Found</H2>\n");
					fprintf(fp_write,"O ficheiro %s nao foi encontrado.</BODY></HTML>",nome_ficheiro);
				}else{
					char formato[BUFFSIZE];
					char buffer_image[BUFFSIZE];
					int n_bytes;
					/* obtencao formato do ficheiro */
					linha_ct=0;
					strcpy(formato,".js");
					while(linha_so_n!=15 && strstr(nome_fich_relativo,CT[0][linha_ct])==0){
						linha_ct=linha_ct+1;
					}
					if(strstr(nome_fich_relativo,CT[0][linha_ct])==0){
						strcpy(formato,"Outro");
					}else{
						strcpy(formato,CT[1][linha_ct]);
					}
					/*formato válido? */
					if(strcmp(formato,"Outro")==0){
						erro=415;
						/* envio mensagem de erro 415 */
						fprintf(fp_write,"HTTP/1.%c 415 Unsuported Media Type\n",versao_http);
						fprintf(fp_write,"Content-Type:text/html\n\n");
						fprintf(fp_write,"\n" );
						fprintf(fp_write,"<HTML><HEAD><TITLE>415 Unsuported Media Type</TITLE></HEAD>");
						fprintf(fp_write,"<BODY><H2>Unsuported Media Type</H2>\n");
						fprintf(fp_write,"O formato do ficheiro %s nao e suportada</BODY></HTML>",nome_ficheiro);
					}else{
						erro=200;
						/* envio mensagem 200 OK */
						fprintf(fp_write,"HTTP/1.%c 200 OK\n",versao_http);
						/* envio Content-Type */
						fprintf(fp_write,"Content-Type:%s\n\n",formato);
							
						/* leitura e verificacao fim do ficheiro */
						while ((n_bytes=fread(buffer_image,1,sizeof(buffer_image),ficheiro))>0){
							/* envio dados do ficheiro */
							fwrite(buffer_image,1,n_bytes,fp_write);
						}
					}
				/* Fecho do ficheiro */
				fclose(ficheiro);
				}
			}
		}
			/* escrita doa dados do pedido */
		sprintf(str_pedido,"%-26s %d %7s %9s %s\n",nome_ficheiro,erro,navegador,sistema_operativo,endereco_cliente);
		printf("%s",str_pedido);
		if(strstr(nome_ficheiro,"/estatisticas/Clear")==0){
			base=Actualizar_salvaguarda(str_pedido,base);
		}else{
			base=Actualizar_lista(nome_ficheiro,base);
		}
			/* fecho da comunicacao */
		fflush(fp_write);
		fclose(fp_write);
		fclose(fp_read);
	}
	return 0;
}
