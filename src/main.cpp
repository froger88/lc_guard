/* 
 * File:   main.cpp
 * Author: froger
 *
 * Created on 26 wrzesień 2009, 00:55
 */

#include <pthread.h>

#include <stdlib.h>
#include "main.h"

#include "linux/socket.h"
#include "linux/types.h"

using namespace std;


pthread_mutex_t mutex;
string __client_id;
string __active_clients;

int main(int argc, char** argv)
{
    string CROSS = "    ++\n    ++\n    ++\n++++++++++\n++++++++++\n    ++\n    ++\n    ++\n    ++\n    ++\n    ++\n    ++";
  cout << CROSS<<endl; //krzyza nic nie ruszy @wydarzenia z sierpnia 2010
  cout << "\nlc-guard by froger - port 15000\n" << endl;
  system("killall -q lc-server");
  system("killall -q geo-server");
  system("killall -q stats-manager");
  system("killall -q pfr-server");

  sleep(60); //zby wszystkie fd'ki zdazyly sie odblokowac

  int Port = 15000;
  int MaxClients = 1000;

  pthread_t t_thread, t_thread2;
  pthread_attr_t tattr_thread, tattr_thread2;
  long int sockfd, newsockfd[100000];
  size_t client_length;
  char *buffer;
  sockaddr_in server_address;
  sockaddr client_address;
  string IP = "";
  int IPtmp;

  //-----------CREATE THREAD AND CONNECTION----------

  buffer = new char[100];
  pthread_setconcurrency(MaxClients + 20);
  pthread_attr_init(&tattr_thread);
  pthread_attr_setdetachstate(&tattr_thread, PTHREAD_CREATE_DETACHED);

  pthread_attr_init(&tattr_thread2);
  pthread_attr_setdetachstate(&tattr_thread2, PTHREAD_CREATE_DETACHED);
  int cpid2 = (int) pthread_create(&t_thread2, &tattr_thread2, threadManage, NULL);
  if(cpid2 != 0)
  {
    return EXIT_FAILURE;
    //Error while creating thread
  }

  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sockfd < 0)
  {
    //tehre are critical problems. exit
    return EXIT_FAILURE;
  }

  memset(buffer, NULL, 100);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(Port);

  if(bind(sockfd, (sockaddr *) & server_address, sizeof (server_address)) != 0)
  {
    //error while binding, exit
    close(sockfd);
    return EXIT_FAILURE;
  }
  listen(sockfd, 20);
  cout << "Ready! <PORT:" << Port << ">" << endl;
  int count = 0;
  while(1)
  {
    if(count >= 100000)
    {
      count = 0;
    }
    memset(buffer, 0, 100);
    client_length = sizeof (client_address);
    newsockfd[count] = accept(sockfd, (sockaddr *) & client_address, (socklen_t*) & client_length);
    IP = "";
    string tmp;
    IPtmp = (int) client_address.sa_data[2];
    if(IPtmp < 0)
    {
      IPtmp += 256;
    }
    tmp = intToStr(IPtmp);
    IP += tmp;
    IP += ".";
    IPtmp = (int) client_address.sa_data[3];
    if(IPtmp < 0)
    {
      IPtmp += 256;
    }
    tmp = intToStr(IPtmp);
    IP += tmp;
    IP += ".";
    IPtmp = (int) client_address.sa_data[4];
    if(IPtmp < 0)
    {
      IPtmp += 256;
    }
    tmp = intToStr(IPtmp);
    IP += tmp;
    IP += ".";
    IPtmp = (int) client_address.sa_data[5];
    if(IPtmp < 0)
    {
      IPtmp += 256;
    }
    tmp = intToStr(IPtmp);
    IP += tmp;

    if(IP != "127.0.0.1")
    {
      //someone from other localization than localhost
      close(newsockfd[count]);
      return EXIT_FAILURE;
    }

    if(newsockfd[count] < 0)
    {
      //Error while creating new sock filedescriptor (newsockfd)
    }
    else
    {
      int cpid = (int) pthread_create(&t_thread, &tattr_thread, threadConnect, (void*) newsockfd[count]);
      if(cpid != 0)
      {
        close(newsockfd[count]);
        //Error while creating thread
      }
      else
      {
        count++;
      }
    }
  }


  return (EXIT_SUCCESS);
}

