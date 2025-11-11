 #include <sys/types.h>
 #include <sys/socket.h>
 #include <stdio.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <stdlib.h>
 #define n 7
 int main ()
 {
 int dfs_serveur, dfs_client, read_size;
 int long_adr_serveur, long_adr_client;
 char buff [4];
 struct sockaddr_in adresse_serveur;
 struct sockaddr_in adresse_client;
 

 dfs_serveur = socket (AF_INET, SOCK_STREAM, 0);
 if (dfs_serveur == -1)
 {
 perror ("socket error");
 exit (1);
 }

 adresse_serveur.sin_family = AF_INET;
 adresse_serveur.sin_addr.s_addr = inet_addr("127.0.0.1");
  adresse_serveur.sin_port = htons(1234);

 long_adr_serveur = sizeof (adresse_serveur);
 if (bind(dfs_serveur, (struct sockaddr *) &adresse_serveur,long_adr_serveur) == -1)
 {
 perror ("bind error");
 exit (1);
 }

 listen (dfs_serveur, 5);
 while (1)
 {
 

 long_adr_client = sizeof (adresse_client);
 dfs_client =accept (dfs_serveur, (struct sockaddr *) &adresse_client,&long_adr_client);

 int tabReceived[n];

    while((read_size = recv(dfs_client, tabReceived, sizeof tabReceived, 0)) > 0){
        
    }


    printf("Tableau recevé:    \n");
    for(int i = 0; i < n; i++)
    {
        printf("%d  ", tabReceived[i]);
        
    }
    

    printf("\n");

      
  



 
 close (dfs_client);
 }
 }

 
