#include<stdio.h> 
#include<stdlib.h>  
#include<string.h>  
#include<errno.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h>  
#include<pthread.h>
#include"parser.h"
#include"dbcomm.h"
#include"crc16.h"

#define DEFAULT_PORT 9090
#define MAXLINE 100  
//最大连接数
#define MAX_THREAD 200 
//使用物联网设备的厂数
#define MAX_FACTORY 300

//int factory_number[MAX_FACTORY] = {29};

//处理tcp连接不稳定
//缓存所有数据
float soil_data[MAX_FACTORY][2];
float air_data[MAX_FACTORY][2];
int pm_data[MAX_FACTORY][3];
int nh3_data[MAX_FACTORY][2];
float ph_data[MAX_FACTORY][2];

int num = 0;

pthread_t thread[MAX_THREAD];

void data_parse(char data[], int len){
    unsigned short facRaw = 0x00;
    int factory, device;
    facRaw |= data[len-4] << 8;
    facRaw |= data[len-3];
    factory = (int)facRaw;
    device = (int)data[0];
    if(factory > MAX_FACTORY)
	    return;
    //NH3
    if(device%5 == 0){
        parseNH3(data, nh3_data[factory]);
    //PH
    } else if(device%5 == 4){
        parsePhAndTemp(data, ph_data[factory]); 
    //PM2.5
    } else if(device%5 == 3){
        parsePM(data, pm_data[factory]);
    //air
    } else if(device%5 == 2){
        parseTempAndHum(data, air_data[factory]);
    //solid
    } else if(device%5 == 1){
        parseTempAndHum(data, soil_data[factory]);
    } else {
        printf("error data!");
    }
}

void data_resolve(int* fd){
    int pos;
    unsigned char data[20];
    unsigned int crc;
    unsigned char crch, crcl;
    //没有数据是阻塞的
    while(1){
        pos = recv(*fd, data, MAXLINE, 0);

        printf("数据: %s ,长度: %d", data, pos);
        
        crc = getCRC16(data, pos-2);
        crch = crc >> 8;
        crcl = crc & 0xFF;

        if(data[pos - 2] == crch && data[pos - 1] == crcl)
            data_parse(data, pos);

        memset(data, 0, sizeof(data));
    }
}

void create_socket(){
    int socket_fd, connect_fd;  
    struct sockaddr_in servaddr;  
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);  
        exit(0);  
    } 

    memset(&servaddr, 0, sizeof(servaddr));  
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY); //IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。  
    servaddr.sin_port = htons(DEFAULT_PORT);

    if( bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){  
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);  
        exit(0);  
    }  
    if( listen(socket_fd, 10) == -1){  
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);  
        exit(0);  
    }  
    printf("======waiting for client's request======\n");
    while(1){
        if( (connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1){  
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);  
            continue;  
        } 
        int ret = pthread_create(&thread[num], NULL, (void *)data_resolve, (void *)&connect_fd);
        if(ret != 0){
            printf("create pthread error\n");
            exit(0);
        }
        //0是mysql的线程最好不要覆盖
        num++;
        if(num == 199){
            num = 1;
            for(int i = 1; i < 199; i++){
                pthread_cancel(i);
            }
        }
    }
}

void mysqlFlush(){
    char sql[MAX_SQL_SIZE];
    while (1)
    {
        //2hours后flash
        initMySQL();
        sleep(60*45);
        for(int i = 0; i < MAX_FACTORY; i++){
            if(nh3_data[i][0] == 0 && ph_data[i][0] == 0 && ph_data[i][1] == 0 && pm_data[i][1] == 0 && air_data[i][0] == 0 && air_data[i][1] == 0 && soil_data[i][0] == 0 && soil_data[i][1] == 0)
                continue;
	        printf("write data______\n");
            memset(sql, 0, sizeof(sql));

            sprintf(sql, "INSERT INTO env_trace (factory_num,temp_soil, hum_soil,temp_indoor,hum_indoor,pm,temp_water,ph,nh3) values (%d,%f,%f,%f,%f,%d,%f,%f,%d)", 
                       i,soil_data[i][1],soil_data[i][0],air_data[i][1],air_data[i][0],pm_data[i][1],ph_data[i][0],ph_data[i][1],nh3_data[i][0]);
            execSQL(sql);

            memset(sql, 0, sizeof(sql));
            sprintf(sql, "INSERT INTO env_trace2 (factory_num,temp_soil, hum_soil,temp_indoor,hum_indoor,pm,temp_water,ph,nh3) values (%d,%f,%f,%f,%f,%d,%f,%f,%d)", 
                       i,soil_data[i][1],soil_data[i][0],air_data[i][1],air_data[i][0],pm_data[i][1],ph_data[i][0],ph_data[i][1],nh3_data[i][0]);

            execSQL(sql);
        }
        closeMySQL();

    }
}

int main(int argc, char** argv){  
    pthread_create(&thread[num], NULL, (void *)mysqlFlush, NULL);
    num++;
    create_socket();
    return 0;
}

