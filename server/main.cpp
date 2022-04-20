#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <json/json.h>
#include <fstream>
#include <iostream>

#include "httprequestparser.h"
#include "request.h"
#include "ThreadsPool.h"
using namespace httpparser;

int server_port;
int thread_num;
char *ip_address;

/*post 请求*/
//http://localhost:8080/api/upload/{"id":"1","name":"Foo"}';
void POST_method(std::string Name, std::string ID, int connfd, std::string str_text)
{
    std::string str_send = "";
    char OK[]="HTTP/1.1 200 OK\r\n";
    str_send += OK;
    char server[]="Tiny Web Server\r\n";
    str_send += server;
    char Context_type[]="text/html\r\n";
    str_send += Context_type;
    // char Context_len[]="Context_length:";

    char html_title[]="<html><title>POST Method</title><body bgcolor=ffffff>\n";
    str_send += html_title;
    /*char chName[]="Your Name: ";
    str_send += chName;
    str_send += Name;
    char chID[]="ID: ";
    str_send += chID;*/
    
    str_send += ID;
    char web[]="<hr><em>Http Web Server</em>\n</body></html>\n";
    str_send += web;

    char send_buf[1024];
    sprintf(send_buf,"%s",str_send.c_str());
    write(connfd, send_buf, strlen(send_buf));

    // close(fd);
    close(connfd);
}
/*未找到*/
void NOTFOUND_method(std::string method,std::string url,int fd){
    std::string entity1="<html><title>404 Not Found</title><body bgcolor=ffffff>\n Not Found\n";
    std::string file="<p>Could not find this file: "+url+"\n";
    std::string entity2="<hr><em>HTTP Web Server</em>\n</body></html>\n";
    std::string entity=entity1+file+entity2;
    if(method=="POST"){
        entity=entity1+entity2;
    }
    std::string str="HTTP/1.1 404 Not Found\r\nServer: Tiny Web Server\r\nContent-Type: text/html\r\nContent-Length: "+std::to_string(entity.length())+"\r\n\r\n";
    std::string total=str+entity;
    char buf[512];
    sprintf(buf,"%s",total.c_str());
    write(fd, buf, strlen(buf));
}

/*获取文件类型*/
char *http_get_mime_type(char *file_name) {
  char *file_extension = strrchr(file_name, '.');
  if (file_extension == NULL) {
    return "text/plain";
  }

  if (strcmp(file_extension, ".html") == 0 || strcmp(file_extension, ".htm") == 0) {
    return "text/html";
  } else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0) {
    return "image/jpeg";
  } else if (strcmp(file_extension, ".png") == 0) {
    return "image/png";
  } else if (strcmp(file_extension, ".css") == 0) {
    return "text/css";
  } else if (strcmp(file_extension, ".js") == 0) {
    return "application/javascript";
  } else if (strcmp(file_extension, ".pdf") == 0) {
    return "application/pdf";
  } else if (strcmp(file_extension, ".json") == 0) {
    return "application/json";
  } else {
    return "text/plain";
  }
}
/*GET 请求*/
/*http://localhost:8888/api/search?id=1&name=foo*/
void GET_method(std::string method,int connfd,std::string uri)
{
    /*获取请求uri*/
    char *curi = new char[uri.length() + 1];
    strcpy(curi, uri.c_str());
    string preuri = "";
    if(uri.size()>=12)
    	preuri = uri.substr(0,12);
    cout<<"uri: "<<uri<<" curi:"<<curi<<endl;
    cout<<uri<<" pre:"<<preuri<<endl;
    curi++;
    int fd;
    
    	if (preuri=="/api/search?")          //   /?id=xx&name=xx   有json格式
	{
		curi = "../data/data.json";  //从文件中找数据匹配
		fd = open(curi,O_RDONLY);
		if(fd==-1) //找不到文件
		{
			NOTFOUND_method(method,uri,connfd);
		}

		//printf("send=%d\n",s);
		string id = "", name = "";   //解析出url中的id和name
		int flag = 0;
		int andPos = 0;
		int wPos = 0;
		for (int i = 0; i < uri.size(); i++) {
		        if (uri[i] == '?')
		            wPos = i + 1;
			if (uri[i] == '&') {
			    flag = 2;
			    andPos = i + 1;
			    }
			if (flag == 1) {
			    id += uri[i];
			}
			else if (flag == 3) {
			    name += uri[i];
			}
			if (uri[i] == '=' && flag == 0)  {
			    flag = 1;
			    string temp = uri.substr(wPos,i - wPos);     //避免字段错误
			    if (temp != "id") {
			        flag = 0;
			        break;
			    }
			} else if (uri[i] == '=' && flag == 2){
			    flag = 3;
			    string temp = uri.substr(andPos,i - andPos);
			    if (temp != "name") {
			        flag = 0;
			        break;
			    }
			}
			
		}
		if (flag != 3)             //如果解析不成功，则404
		    NOTFOUND_method(method,uri,connfd);
		string requestStr ;
		ifstream openfile(curi);
		openfile>>requestStr;
		cout<<requestStr<<endl;
		cout<<id<<" " <<name<<endl;
		openfile.close();
		/*  徒手解析json
		string response;
		int process = 0;
		string getID;
		string getName;
		for (int i = 0; i < requestStr.size(); i++) {
			if (requestStr[i] == '{' && process == 0) {
				response += resquestStr[i];
				process = 1;
			}
			else if (process == 1 && requestStr[i] == '}') {
			        response += '}';
			        for (int j = 7; j < response.size(); j++) {
			        	if (response[j] == '"')
			        	    break;
			        	getID += response[j];
			        }
			        if (getID == id) {
			        	for (int j = response.size()-3; j < response.size(); j++) {
						if (response[j] == '"')
						    break;
						getName = response[j]+getName ;
			        	}	
			        	if (getName == name) {
			        		process = 2;   //成功匹配
			        		response = '[' + response + ']';
			        		break;
			        	}
			        }
			        process = 0;
			        response = "";

			}
			else if (process == 1) {
				response += requestStr[i];
			}
			
		}
		
		//处理json
		if (process == 0)
		    NOTFOUND_method(method,uri,connfd); 
		else if (process == 2) 
		    
		*/
		//  当前gcc版本不允许使用json高版本内容，必须要手动动态链接库
		Json::Reader reader;
		Json::Value value;
		requestStr = requestStr.substr(1,requestStr.size()-2);
		vector<string> resultData;
		int beginPos = 0;
		for (int i = 0; i < requestStr.size(); i++) {
		    if (requestStr[i] == ',' && requestStr[i + 1] == '{') {
		    	resultData.push_back(requestStr.substr(beginPos, i - beginPos));
		    	beginPos = i + 1;
		    }
		}
		resultData.push_back(requestStr.substr(beginPos, requestStr.size() - beginPos));
		int flag_find = 0;
		//name = '\"' + name + '\"';
		for (int i = 0; i < resultData.size(); i++) {
		        cout<<i<<":"<<resultData[i]<<endl;
			if(reader.parse(resultData[i], value)) {
			        string vname = value["name"].asString();
			        string vid = value["id"].asString();
			        if (vname != name)
			            cout<<vname<<" "<<name<<" "<<vid<<" "<<id<<endl;
				//if(value["name"].asString() != name )
				//    cout<<value["id"]<<" "<<value["name"]<<" "<<name<<endl;
				if(vid == id && vname == name) {
				        cout<<endl;
					ofstream fout("temp.json");
					fout<<"[{\"id\":\""<<id<<"\",\"name\":\""<<name<<"\"}]";
					curi = "temp.json";
					fd = open(curi,O_RDONLY);
					cout<<"new curi:"<<curi<<endl;
					fout.close();
					flag_find = 1;
					break;
				}
				//cout<<value["id"]<<" "<<typeid(value["name"].asString()).name<<" "<<name<<endl;
			}
			
		}
		if (flag_find == 0) {  //没找到匹配的id&name  404.json
		
		    curi = "../data/not_found.json";
		    fd = open(curi,O_RDONLY);
		    if(fd==-1) //找不到文件
		    {
		 	NOTFOUND_method(method,uri,connfd);
	     	    }
		}
		//获取文件大小
		struct stat file;
		stat(curi,&file);
		int length=file.st_size;
		std::string slength = std::to_string(length);
                
		//构造请求
		char code[]="HTTP/1.1 200 OK\r\n";
		char server[]="Server: Tiny Web Server\r\n";
		char type_head[]="\r\nContent-type:";
		char *type=http_get_mime_type(curi);
		char length_head[]="Content-length:";
		char const *clength = slength.c_str();
		char end[]="\r\n\r\n";


		//发送响应
		send(connfd,code,strlen(code),0);
		send(connfd,server,strlen(server),0);
		send(connfd,length_head,strlen(length_head),0);
		send(connfd,clength,strlen(clength),0);
		send(connfd,type_head,strlen(type_head),0);
		send(connfd,type,strlen(type),0);
		send(connfd,end,strlen(end),0);

		sendfile(connfd,fd,NULL,2500);
		//send
		send(connfd,end,strlen(end),0);

		close(fd);
		close(connfd);
	}
	else{       
		cout<<uri<<" "<<curi<<endl;
		if(uri=="/" || uri == "/index.html") curi="../static/index.html"; /*默认消息体*/
		else if (uri=="/test/test.html") curi = "../static/test/test.html";
		else if (uri=="/api/list") curi="../static/data.json"; 
		else {
			NOTFOUND_method(method,uri,connfd);
		}
		cout<<"curi:"<<curi<<endl;
		fd = open(curi,O_RDONLY);
		if(fd==-1) /*找不到文件*/
		{
			NOTFOUND_method(method,uri,connfd);
		}
		


		//获取文件大小
		struct stat file;
		stat(curi,&file);
		int length=file.st_size;
		std::string slength = std::to_string(length);

		//构造请求
		char code[]="HTTP/1.1 200 OK\r\n";
		char server[]="Server: Tiny Web Server\r\n";
		char length_head[]="Content-length:";
		char const *clength = slength.c_str();
		char type_head[]="\r\nContent-type:";
		char *type=http_get_mime_type(curi);
		char end[]="\r\n\r\n";

		//发送响应
		send(connfd,code,strlen(code),0);
		send(connfd,server,strlen(server),0);
		send(connfd,length_head,strlen(length_head),0);
		send(connfd,clength,strlen(clength),0);
		send(connfd,type_head,strlen(type_head),0);
		send(connfd,type,strlen(type),0);
		send(connfd,end,strlen(end),0);

		sendfile(connfd,fd,NULL,2500);


		close(fd);
		close(connfd);
	}
}

void NOT_Implemented(std::string method,int fd)
{
    std::string entity1="<html><title>501 Not Implemented</title><body bgcolor=ffffff>\n Not Implemented\n";
    std::string entity2="<p>Does not implement this method: "+method+"\n<hr><em>HTTP Web Server</em>\n</body></html>\n";
    std::string entity=entity1+entity2;
    std::string str="HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\nContent-Length: "+std::to_string(entity.length())+"\r\n\r\n";
    std::string total=str+entity;
    char buf[512];
    sprintf(buf,"%s",total.c_str());
    write(fd, buf, strlen(buf));
}

void handle_request(int connfd)
{
    char text[1024];
    recv(connfd,text,1024,0);
    text[strlen(text)+1]='\0';

    std::string str_text = "";
    str_text += text;

    Request request;
    HttpRequestParser parser;
    int length=strlen(text);

    HttpRequestParser::ParseResult res = parser.parse(request, text, text + length);

    if( res == HttpRequestParser::ParsingCompleted )
    {
        std::cout<<request.method<<std::endl;
        std::cout<<request.uri<<std::endl;

        std::string method=request.method;
        std::string uri=request.uri;
        //cout<<"test: method:"<<method<<endl;
        if(method=="GET")
        {
            //cout<<"get a get"<<endl;
            GET_method(method,connfd,uri);
        }
        else if(method!="GET"&&method!="POST")
        {
            NOT_Implemented(method,connfd);
        }
	else if(method=="POST")
	{
		if( uri == "/api/upload")
		{
        int position1 = str_text.find("{");
        int position2 = str_text.find("}");
        if( position1==-1 || position2==-1 || position1>=position2)
        {
          NOTFOUND_method(method,uri,connfd);
        }
        else
        {
          //GET_method(method,connfd,uri);
	      std::cout << "db db db: /Post_show\n";
        }

        std::string Name,ID;
        ID=str_text.substr(position1,position2-position1+1);
        //Name=str_text.substr(position1+5,position2-position1-5);
        int pos0 = 0, pos1 = 0,pos2 = 0,pos3 = 0;  //差错检测
		for (int i = 0; i < ID.size(); i++) {
			if (ID[i] == ':' && pos0 == 0)
			    pos0 = i;
			else if (ID[i] == ',' && pos1 == 0)
			    pos1 = i;
			else if (ID[i] == ':' && pos2 == 0) {
			    pos2 = i;
			    i++;
			}
			if (ID[i] == '"' && pos2 != 0)
			    pos3 = i;
		}
		string temp = ID.substr(0,pos0 + 2);
		cout<<"1:"<<temp<<endl;
		if (temp != "{\"id\":\"")
		    NOTFOUND_method(method,uri,connfd);
		temp = ID.substr(pos1 - 1, pos2 -pos1 + 3);
		cout<<"2:"<<temp<<endl;
		if (temp != "\",\"name\":\"")
			NOTFOUND_method(method,uri,connfd);
		temp = ID.substr(pos3, ID.size() - pos3);
		cout<<"3:"<<temp<<endl;
		if (temp != "\"}")
			NOTFOUND_method(method,uri,connfd);
    //cout<<"1:"<<temp<<endl;
        //测试：cout<<str_text<<endl;
        //cout<<str_text<<endl;
       
        POST_method(Name, ID, connfd, str_text);

			}
			if(uri != "/api/upload")
			{
				NOTFOUND_method(method,uri,connfd);
			}
			else
			{
				NOT_Implemented(method,connfd);
			}
		}
    }
}

void TCP_connect(int thread_num,char *ip_address)
{
    ThreadPool pool(thread_num);
    pool.start();

    int socketfd;
    int connfd;
    struct sockaddr_in server_address;

    socketfd = socket(AF_INET,SOCK_STREAM,0);   //建立TCP socket
    if (socketfd == -1) {
    perror("Failed to create a new socket");
    exit(errno);
    }

	int reuse = 1;
    if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
      perror("setsockopt error\n");
      return ;
    }

    bzero(&server_address,sizeof(server_address));    //清零
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_address);  //sin_addr存储IP地址，使用in_addr这个数据结构；s_addr按照网络字节顺序存储IP地址
    server_address.sin_port = htons(server_port);   //端口号

    if (bind(socketfd, (struct sockaddr *) &server_address,
        sizeof(server_address)) == -1) {
        perror("Failed to bind on socket");
        exit(errno);
    }

    if (listen(socketfd, 1024) == -1)
    {
        perror("Failed to listen on socket");
        exit(errno);
    }

    printf("Listening on port %d...\n",server_port);

    while(true)
    {
        //等待连接
        struct sockaddr_in client;
        socklen_t client_addrlength = sizeof(client);
        connfd = accept(socketfd, (struct sockaddr*)&client, &client_addrlength);   //返回连接套接字描述符
        if(connfd<0)    //该描述符不为0
        {
            perror("err connfd\n");
        }
        else
        {
            TaskEntry* task = new TaskEntry(connfd);
            task -> func = handle_request;
            pool.run(task);

        }
    }
	close(socketfd);
  pool.stop();

}

int main(int argc,char **argv)
{
    //默认参数s
    server_port=8888;
    thread_num=2;
    ip_address="127.0.0.1";

    if(argc < 5)
    {
        printf("./httpServer --ip ip_address --port port_number [--number-thread thread_number] \n");
    }

    for(int i=1;i<argc;i++)
    {
        if(strcmp("--port",argv[i])==0)  // --port
        {
            char *server_port_str=argv[++i];
            if(!server_port_str){
                fprintf(stderr, "Expected argument after --port\n");
            }
            server_port=atoi(server_port_str);
        }
        else if (strcmp("--number-thread", argv[i]) == 0)
        {
          char *cthread_num = argv[++i];
          if (!cthread_num) {
              fprintf(stderr, "Expected argument after --proxy\n");
          }
          thread_num=atoi(cthread_num);
        }
        else if (strcmp("--ip", argv[i]) == 0)
        {
          char *ip = argv[++i];
          if (!ip) {
              fprintf(stderr, "Expected argument after --ip\n");
          }
          ip_address=ip;
        }
    }

    TCP_connect(thread_num,ip_address);

    return 0;
}
