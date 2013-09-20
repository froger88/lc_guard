
#include "thread_connect.h"

extern pthread_mutex_t mutex;

using namespace std;

extern string __client_id;
extern string __active_clients;

void* threadConnect(void *arg)
{
  string client_id;
  string client_idtmp;
  string KMESSAGE = "kill";
  //sprawdz polaczenie
  //dopisz do bazy active threads (w razie rozlaczenia - wypisz z active threads)

  long int fd = (long int) arg;
  char *Buffer = new char[100];
  int th_str;
  ///USTAWIENIE TL
  timespec TL;
  TL.tv_sec = 1; //seconds
  TL.tv_nsec = 0; //nano seconds

  timespec *timeout = &TL;

  fd_set readmask;
  FD_ZERO(&readmask);
  FD_SET(fd, &readmask);
  ///KONIEC USTAWIANIA TL

  int PING = 0;

  bool first = true;
  while(1)
  {
    int nfound = 0;
    while(nfound == 0)
    {
      nfound = 0;
      FD_ZERO(&readmask);
      FD_SET(fd, &readmask);
      memset(Buffer, NULL, 100);
      nfound = pselect(sizeof (readmask)*8, &readmask, NULL, &readmask, timeout, NULL);

      if(nfound == 0)//jezeli TL przekroczony
      {
        if(PING == 45)
        {
          pthread_mutex_lock(&mutex);
          //usun swoje id z active_clients
          int start;
          client_idtmp = ":" + client_id + ":";
          start = __active_clients.find(client_idtmp);
          if(start != string::npos) //znalezniono
          {
            string _active_tmp;
            _active_tmp.clear();
            for(int i = 0; i < __active_clients.length(); i++)
            {
              if(i <= start || i >= start + client_idtmp.length())
              {
                _active_tmp += __active_clients[i];
              }
            }
            __active_clients.clear();
            __active_clients = _active_tmp;
          }
          string run;
          //na wszelki wypadek sprobuj ubic
          if(client_id != "gs15001" && client_id != "gs15002" && client_id != "gs15003" && client_id != "sm" && client_id != "ps")
          {
            write(fd, KMESSAGE.c_str(), KMESSAGE.length());
            pthread_mutex_unlock(&mutex);
            close(fd);
            //wyczysc zasoby
            timeout = NULL;
            delete [] Buffer;
            Buffer = NULL;
            pthread_exit(0);
          }
          else
          {
            close(fd);
            pthread_mutex_unlock(&mutex);
            //wyczysc zasoby
            timeout = NULL;
            delete [] Buffer;
            Buffer = NULL;
            pthread_exit(0);
          }
        }
        else
        {
          PING++;
        }
      }
      else
      {
        PING = 0;
        break;
      }
    }

    if(PING < 45)
    {
      memset(Buffer, NULL, 100);
      th_str = read(fd, Buffer, 100);
    }
    if((th_str == 0) || (th_str == -1) || PING >= 45)
    {
      pthread_mutex_lock(&mutex);
      //usun swoje id z active_clients
      int start;
      client_idtmp = ":" + client_id + ":";
      start = __active_clients.find(client_idtmp);
      if(start != string::npos) //znalezniono
      {
        string _active_tmp;
        _active_tmp.clear();
        for(int i = 0; i < __active_clients.length(); i++)
        {
          if(i <= start || i >= start + client_idtmp.length())
          {
            _active_tmp += __active_clients[i];
          }
        }
        __active_clients.clear();
        __active_clients = _active_tmp;
      }
      //na wszelki wypadek
      string run;
      if(client_id != "gs15001" && client_id != "gs15002" && client_id != "gs15003" && client_id != "sm" && client_id != "ps")
      {
        write(fd, KMESSAGE.c_str(), KMESSAGE.length());
        pthread_mutex_unlock(&mutex);
        close(fd);
        //wyczysc zasoby
        timeout = NULL;
        delete [] Buffer;
        Buffer = NULL;
        pthread_exit(0);
      }
      else
      {
        close(fd);
        pthread_mutex_unlock(&mutex);
        //wyczysc zasoby
        timeout = NULL;
        delete [] Buffer;
        Buffer = NULL;
        pthread_exit(0);
      }
    }
    else
    {
      if(first == true)
      {
        pthread_mutex_lock(&mutex);
        if(__active_clients[0] != ':')
        {
          __active_clients = ":" + __active_clients;
        }
        client_id.assign(Buffer);
        client_idtmp += client_id + ":";
        if(__active_clients.find(client_idtmp) == string::npos)
        {
          __active_clients += client_idtmp;
        }
        first = false;
        pthread_mutex_unlock(&mutex);
      }
      pthread_mutex_lock(&mutex);
      //ubij jezeli nadmiarowy
      if(__client_id.find(client_idtmp) == string::npos) //jezeli nie znajdzie
      {
        string run;
        if(client_id != "gs15001" && client_id != "gs15002" && client_id != "gs15003" && client_id != "sm" && client_id != "ps")
        {
          write(fd, KMESSAGE.c_str(), KMESSAGE.length());
          pthread_mutex_unlock(&mutex);
          pthread_mutex_lock(&mutex);
        }
        client_idtmp = ":" + client_id + ":";
        int start = __active_clients.find(client_idtmp);
        if(start != string::npos) //znalezniono
        {
          string _active_tmp;
          _active_tmp.clear();
          for(int i = 0; i < __active_clients.length(); i++)
          {
            if(i <= start || i >= start + client_idtmp.length())
            {
              _active_tmp += __active_clients[i];
            }
          }
          __active_clients.clear();
          __active_clients = _active_tmp;
        }
        pthread_mutex_unlock(&mutex);
        close(fd);
        //wyczysc zasoby
        //delete timeout;
        timeout = NULL;
        delete [] Buffer;
        Buffer = NULL;
        pthread_exit(0);
      }
      pthread_mutex_unlock(&mutex);
    }
  }
  //wyczysc zasoby
//  delete timeout;
  timeout = NULL;
  delete [] Buffer;
  Buffer = NULL;
  pthread_exit(0);

}
