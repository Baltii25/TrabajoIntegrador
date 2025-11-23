//Github: https://github.com/Baltii25/TrabajoIntegrador

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

struct memory {
  char *response;
  size_t size;
};
	
static size_t cb(char *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = nmemb;
  struct memory *mem = clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(!ptr)
    return 0;  /* out of memory */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

int leer_responseInt(char* src, char* buscar, int* guardar){
	int result=0;
	char strAux[256];
	char* pAux= strstr(src, buscar);
	if (pAux){
			pAux+=strlen(buscar);	
			sscanf(pAux, "%[^,]", strAux);
			result= atoi(strAux);
			*guardar= result;
			return 1;
	}
	else
		return 0;
}

int leer_responseStr(char* src, char* buscar, char* guardar, int n){
	char* pAux= strstr(src, buscar);
	if(pAux){
		pAux+=strlen(buscar);
		char* end = strstr(pAux, "\"");
		int _n = end - pAux;
		if(n > _n){
			strncpy(guardar, pAux, _n);
		}
		return 1;
	}
	else
		return 0;
}

int respuestas(int flag, char* msg, char* texto){
  if(strcasestr(texto, "hola") && flag!=1){
	msg = "Hola%20";
	return 1;
    }
    else{

        if(strcasestr(texto, "chau") && flag!=1){
	  msg = "Chau%20";
	  return 1;
    	}
    	else
	  return 0;
    	}
}

int main(int argc, char *argv[])
{
	char *api_gen = "https://api.telegram.org/bot";
        char api_url[256]={0};
	char token[50]={0};
	char *cmdGetUpdates = "/getUpdates";
	char *cmdSendMessage = "/sendMessage?chat_id=";
	char *sendMessageText = "&text=";
	char *offset= "?offset=";
	int upd_id = 0;
	int tiempo = 0;

	char nombre[30];
	char texto[50];
	char chat_id[20];
	
	if(argc < 2){
		printf("Error, no se pasó el argumento para el token");
		return 1;
	}

	FILE* fPtr;
	if((fPtr=fopen(argv[1], "r"))==NULL){
		printf("Fallo al leer el token");
		exit(1);
	}
	else{
		fscanf(fPtr, "%s", &token);
		printf("\nEl token es: %s \n", token);
		fclose(fPtr);
	}
	
  snprintf(api_url, 256, "%s%s%s", api_gen, token, cmdGetUpdates);

  printf("\n\nLink: %s \n\n", api_url);

  CURLcode res;
  

  int SMflag = 0;

  while(1){

  if((fPtr=fopen("registros.txt", "a"))==NULL){
	printf("Fallo al abrir los registros");
	exit(1);
  }

	
  CURL *curl = curl_easy_init();

  struct memory chunk = {0};

  if(curl) {

    curl_easy_setopt(curl, CURLOPT_URL, api_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);
    if (res != 0)
      printf("Error Código: %d\n", res);

    if(leer_responseInt(chunk.response,"\"update_id\":", &upd_id))
    	upd_id++;
    snprintf(api_url, 256, "%s%s%s%s%d", api_gen, token, cmdGetUpdates, offset, upd_id);
 
    memset(chat_id, 0, sizeof(chat_id));
    if(leer_responseStr(chunk.response, "\"chat\":{\"id\":", chat_id, 20)){
	printf("\nChat id: %s\n", chat_id);
    }
   
    if(leer_responseInt(chunk.response, "\"date\":", &tiempo)){
        fprintf(fPtr, "Tiempo: %d\n", tiempo);
    }

    memset(nombre, 0, sizeof(nombre));
    if(leer_responseStr(chunk.response, "\"first_name\":\"", nombre, 40)){
	fprintf(fPtr, "Habló %s\n", nombre);
    }

    memset(texto, 0, sizeof(texto));
    if(leer_responseStr(chunk.response, "\"text\":\"", texto, 50)){
	fprintf(fPtr, "Dijo: %s\n\n", texto);
    }
	

    if(strcasestr(texto, "hola") && SMflag!=1){
	snprintf(api_url, 256, "%s%s%s%s%s%s%s", api_gen, token, cmdSendMessage, chat_id, 
					         sendMessageText,"Hola,%20", nombre);
	SMflag = 1;
    }
    else{

        if(strcasestr(texto, "chau") && SMflag!=1){
	  snprintf(api_url, 256, "%s%s%s%s%s%s%s", api_gen, token, cmdSendMessage, chat_id, 
					           sendMessageText,"Chau%20", nombre);
	  SMflag = 1;
    	}
    	else
	  SMflag = 0;
    	}   

    printf("\n%s", chunk.response);
    sleep(2);

    chunk.response = '\0';
    chunk.size = 0;
    fclose(fPtr);
  }

  free(chunk.response);
  curl_easy_cleanup(curl);
 }
}
