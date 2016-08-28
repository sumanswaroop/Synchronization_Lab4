/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <cstdlib>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <pthread.h>
#include <fstream>
#include <queue>
#define BACK_LOG 8

using namespace std;

//Common Lock
pthread_mutex_t mutex;
//COndition Variables
pthread_cond_t full;
pthread_cond_t empty;

//queue
queue<int> req_q;

void error(string msg)
{
	perror(msg.c_str());
	exit(1);
}


void *worker(void *)
{	
	//variable declarations
	char buffer[1024]; //buffer to take client req
	int sockfd,n;
	while(true)
	{	
		//acquire lock
		pthread_mutex_lock(&mutex);

		//wait until a req comes
		while(req_q.empty())
			pthread_cond_wait(&empty, &mutex);

		//if non-empty pop from queue
		sockfd = req_q.front();
		req_q.pop();
		//signal server
		pthread_cond_signal(&full);
		//unlock
		pthread_mutex_unlock(&mutex);



		/* read message from client */
		 bzero(buffer,1024);	 
		 if ( (n = read(sockfd,buffer,sizeof(buffer)))<0) 
		 {	
		 	//close on failure
		 	close(sockfd);
		 	error("ERROR reading from socket");
		 }
		 	 
		 //parsing request
		 int i;
		 for(i=0;i<255;i++) if(buffer[i]==' ') break;
		 i++;
		 string filename = "";
		 for(;i<255;i++) {if(buffer[i]=='.') break; filename+=buffer[i];}
		 filename+=".txt";

		 //Reading the requested file
		 ifstream file (filename.c_str(), ios::in);
		 if(file.fail()) 
	 	 {	
	 	 	//failed : close socket
	 		close(sockfd);
	 		error("File not found");
	 	 }

		 //buffer for file content
		 bzero(buffer, sizeof(buffer));
		 int length = sizeof(buffer)-1;
		 /* send reply to client */ //the requested file is being written in socket
		 while(!file.eof())
		 {
		 	file.read(buffer,length-1);
		 	n = write(sockfd,buffer,strlen(buffer));
			if (n < 0) {
				close(sockfd);
				error("ERROR writing to socket");
			}
		 }

		
		 
		 //cleanup
		 file.close();

		 //Close Socket
		 close(sockfd);
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{	
	 //socket related vars
	 int sockfd, newsockfd;
	 socklen_t clilen;
	 char buffer[256];
	 struct sockaddr_in serv_addr, cli_addr;
	 int n;

	 
	 if (argc < 4) {
		 error("Usage <executable> <port> <number of workers> <queue size>\n");
	 }
	 //Arguments
	 int portno = atoi(argv[1]);
	 int num_workers = atoi(argv[2]);
	 int queue_size = atoi(argv[3]);
	 //Check if 0 workers
	 if(num_workers<=0)
		error("Server without Workers. Exiting\n");

	 /* create socket */

	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 if (sockfd < 0) 
		error("ERROR opening socket");

	 /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

	 bzero( (char *) &serv_addr, sizeof(serv_addr));
	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_addr.s_addr = INADDR_ANY;
	 serv_addr.sin_port = htons(portno);

	 /* bind socket to this port number on this machine */

	 if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
			  error("ERROR on binding");

	 //Initialize lock for queue data structure
	 pthread_mutex_init(&mutex,0);
	 //Conditional Variables
	 pthread_cond_init(&full, 0);
	 pthread_cond_init(&empty, 0);

	 //Initialize Worker threads
	 pthread_t workers[num_workers];
	 //Create Worker threads
	 for(int i = 0;i<num_workers;i++)
	 {
	 	pthread_create(&workers[i], 0, worker, (void * )0);
	 	cout<<"Creating Worker thread"<<endl;
	 }
	 /* listen for incoming connection requests */
	 listen(sockfd, BACK_LOG);
	 while(true)
	 {
		 
		  //if there is a request and queue not full;
		  /* accept a new request, create a newsockfd */
	 	
		 
		 if ( (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen)) < 0) 
		 		error("ERROR on accept \n");
		 
		 
		  pthread_mutex_lock(&mutex);
		  //wait until not full

		  while(req_q.size() >= queue_size && queue_size !=0)
		  	 pthread_cond_wait(&full, &mutex);
		  
		  
		  
		  	 
		  //push the new req in queue
		  req_q.push(newsockfd);
		  //cout<<req_q.size()<<endl;
		  //signal worker of new req
		  pthread_cond_signal(&empty);
		  //unlock 
		  pthread_mutex_unlock(&mutex);

	  }

	  //Join Worker Threads
	  for(int i = 0;i<num_workers;i++)
	  {
	  	pthread_join(workers[i], NULL);
	  	cout<<"Exiting Worker"<<" i "<<endl;
	  }
	  //Destroy Conds and Locks
	  pthread_mutex_destroy(&mutex);
	  pthread_cond_destroy(&full);
	  pthread_cond_destroy(&empty);

	  
	 return 0; 
}
