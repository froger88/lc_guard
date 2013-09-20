
#include <fstream>

#include "thread_manage.h"

using namespace std;

extern pthread_mutex_t mutex;

extern string __client_id;
extern string __active_clients;

void* threadManage(void *arg)
{
  long long int run_id =0;
  int TL = 60; //co 60s sprawdzaj baze i aktywnych
  while(1)
  {
    int NumGeo = 5; //ilosc geo-serwerów, domyslnie 5, aktualizowane z bazy przy kazdym 'swiezym' przejsciu
    bool ok;
    string SNumGeo = "5";
    pthread_mutex_lock(&mutex);
    //pobierz dane z bazy
    string DBUsername = "";
    string DBDataBase = "";
    string DBPassword = "";
    string DBHost = ""; //
    string DBQuery;
    DBQuery.clear();

    MYSQL Database;
    MYSQL_RES *Result;
    MYSQL_ROW Row;
    long res;

    __client_id = ":ps:gs15001:gs15002:gs15003:sm:";

    if(__active_clients.find(":gs15001:") == string::npos)
    {
      pthread_mutex_unlock(&mutex);
      system("geo-server 15001 &");
      sleep(30);
      pthread_mutex_lock(&mutex);
    }
    if(__active_clients.find(":gs15002:") == string::npos)
    {
      pthread_mutex_unlock(&mutex);
      system("geo-server 15002 &");
      sleep(30);
      pthread_mutex_lock(&mutex);
    }
    if(__active_clients.find(":gs15003:") == string::npos)
    {
      pthread_mutex_unlock(&mutex);
      system("geo-server 15003 &");
      sleep(30);
      pthread_mutex_lock(&mutex);
    }
    if(__active_clients.find(":sm:") == string::npos)
    {
      pthread_mutex_unlock(&mutex);
      system("stats-manager &");
      pthread_mutex_lock(&mutex);
    }
    if(__active_clients.find(":ps:") == string::npos)
    {
      system("pfr-server &");
    }

    if(!mysql_init(&Database))
    {
      //nie udalo sie, nie skzodzi ;) try again ^^
    }
    else
    {
      if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
      {
        DBQuery = "SELECT guard FROM _ec_guards LIMIT 1";
        res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
        if(res != 0)
        {
          mysql_close(&Database);
          ok = false;
        }
        else
        {
          ok = true;
          Result = mysql_store_result(&Database);
          if(Result)
          {
            Row = mysql_fetch_row(Result);
            unsigned long *lengths;
            lengths = mysql_fetch_lengths(Result);
            SNumGeo.assign(Row[0]);
            int err;
            //NumGeo = strToInt(SNumGeo, err);
          }
          mysql_free_result(Result);
        }

        if(ok)
        {
          DBQuery = "SELECT db_name, db_prefix,port, num_geo_server, max_users, max_guests, max_mrbug*domains_count, shutdown FROM _ec_clients WHERE activated=1 AND restart=0 AND shutdown=0;";
          res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
          if(res != 0)
          {
            //disconnect from database
            mysql_close(&Database);
            //nie szkodzi ze sie nie udalo ;)
          }
          else
          {
            Result = mysql_store_result(&Database);
            if(Result)
            {
              string client_id;
              __client_id.clear();
              __client_id = ":ps:gs15001:gs15002:gs15003:sm:";
              while(Row = mysql_fetch_row(Result))
              {
                unsigned long *lengths;
                lengths = mysql_fetch_lengths(Result);
                client_id = Row[1];
                client_id = ":" + client_id;
                __client_id += client_id;
                string tmp_client_id = client_id + ":";
                if(__active_clients.find(tmp_client_id) == string::npos) //klient is dead, trzeba uruchomic
                {
                  string params;
                  string r1, r2, r0;
                  string ngs, mu, mg, mb;
                  r1.assign(Row[1]); //row1 (db_prefix)
                  r2.assign(Row[2]); //row2 (port)
                  r0.assign(Row[0]); //row0 (db_name)
                  ngs.assign(Row[3]); //num geo server
                  mu.assign(Row[4]); //max user
                  mg.assign(Row[5]); //max guest
                  mb.assign(Row[6]); //max bug
                  params = r1 + " " + r0 + " " + mu + " " + mg + " " + mb + " " + r2 + " " + ngs + " > lc_"+r2+ "_"+intToStr(run_id)+".log &";
                  string run;
                  run = "lc-server " + params;
                  system(run.c_str());
                  ++run_id;
                }
              }
              mysql_free_result(Result);

              //zwiekszaj wartosc restart o 1 do momentu az restart bedzie rowny NumGeo

              string DBQuery = "UPDATE _ec_clients SET restart=0 WHERE restart=" + SNumGeo + ";";
              mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
              DBQuery = "UPDATE _ec_clients SET restart = restart + 1 WHERE restart < " + SNumGeo + " AND restart > 0;";
              mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
              mysql_close(&Database);
            }
          }
        }
      }
    }
    __client_id += ":";
    //odblokuj mutexy
    pthread_mutex_unlock(&mutex);
    sleep(TL);
  }
  pthread_exit(0);
}
