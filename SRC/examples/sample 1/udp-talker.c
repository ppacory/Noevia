#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>

#define MAX_PORT 5
#define PORT_ARRAY_SIZE (MAX_PORT+1)
#define MAX_MSG 80
#define MSG_ARRAY_SIZE (MAX_MSG+1)
// Utilisation d'une constante x dans la d�finition
// du format de saisie
#define str(x) # x
#define xstr(x) str(x)

int main()
{
  int socketDescriptor, status;
  unsigned int msgLength;
  struct addrinfo hints, *servinfo;
  struct timeval timeVal;
  fd_set readSet;
  char msg[MSG_ARRAY_SIZE], serverPort[PORT_ARRAY_SIZE];

  puts("Entrez le nom du serveur ou son adresse IP : ");
  memset(msg, 0, sizeof msg);  // Mise � z�ro du tampon
  scanf("%"xstr(MAX_MSG)"s", msg);

  puts("Entrez le num�ro de port du serveur : ");
  memset(serverPort, 0, sizeof serverPort);  // Mise � z�ro du tampon
  scanf("%"xstr(MAX_PORT)"s", serverPort);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if ((status = getaddrinfo(msg, serverPort, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
  }

  if ((socketDescriptor = socket(servinfo->ai_family, servinfo->ai_socktype,
                                 servinfo->ai_protocol)) == -1) {
    perror("socket:");
    exit(EXIT_FAILURE);
  }

  puts("\nEntrez quelques caract�res au clavier.");
  puts("Le serveur les modifiera et les renverra.");
  puts("Pour sortir, entrez une ligne avec le caract�re '.' uniquement.");
  puts("Si une ligne d�passe "xstr(MAX_MSG)" caract�res,");
  puts("seuls les "xstr(MAX_MSG)" premiers caract�res seront utilis�s.\n");

  // Invite de commande pour l'utilisateur et lecture des caract�res jusqu'� la
  // limite MAX_MSG. Puis suppression du saut de ligne en m�moire tampon.
  puts("Saisie du message : ");
  memset(msg, 0, sizeof msg);  // Mise � z�ro du tampon
  scanf(" %"xstr(MAX_MSG)"[^\n]%*c", msg);

  // Arr�t lorsque l'utilisateur saisit une ligne ne contenant qu'un point
  while (strcmp(msg, ".")) {
    if ((msgLength = strlen(msg)) > 0) {
      // Envoi de la ligne au serveur
      if (sendto(socketDescriptor, msg, msgLength, 0,
                 servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        perror("sendto:");
        close(socketDescriptor);
        exit(EXIT_FAILURE);
      }

      // Attente de la r�ponse pendant une seconde.
      FD_ZERO(&readSet);
      FD_SET(socketDescriptor, &readSet);
      timeVal.tv_sec = 1;
      timeVal.tv_usec = 0;

      if (select(socketDescriptor+1, &readSet, NULL, NULL, &timeVal)) {
        // Lecture de la ligne modifi�e par le serveur.
        memset(msg, 0, sizeof msg);  // Mise � z�ro du tampon
        if (recv(socketDescriptor, msg, sizeof msg, 0) == -1) {
          perror("recv:");
          close(socketDescriptor);
          exit(EXIT_FAILURE);
        }

        printf("Message trait� : %s\n", msg);
      }
      else {
        puts("** Le serveur n'a r�pondu dans la seconde.");
      }
    }
    // Invite de commande pour l'utilisateur et lecture des caract�res jusqu'� la
    // limite MAX_MSG. Puis suppression du saut de ligne en m�moire tampon.
    // Comme ci-dessus.
    puts("Saisie du message : ");
    memset(msg, 0, sizeof msg);  // Mise � z�ro du tampon
    scanf(" %"xstr(MAX_MSG)"[^\n]%*c", msg);
  }

  close(socketDescriptor);

  return 0;
}