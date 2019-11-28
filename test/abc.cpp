#include "abc.h"
#include <string>

#include  "unistd.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include<thread>

#include<thread>





vector<string> m_vecPolicy; 


void print1()
{

atoi("10");
    while(true){
m_vecPolicy.push_back("jiang");
sleep(1);

    cout << "print1_1线程执行" << endl;
    cout << "print1_2线程执行" << endl;
    cout << "print1_3线程执行" << endl;
    }
}    

void print2()
{

   string strIP ;

	vector<string>::iterator iter ;
    while (true)
    {
   iter = m_vecPolicy.begin();

	for(;iter!= m_vecPolicy.end();iter++) {
        strIP   = *iter;
        cout<<strIP<<endl;  
    }
    sleep(1);
        
    }
    

}


void print3()
{
    //m_vecPolicy
    while (true)
    {
    m_vecPolicy.clear();
    sleep(3);
    cout << "print3_3线程执行" << endl;

    }
    
        
   

}


int main(int argc, char *argv[]) {

//m_vecPolicy.reserve(80);
        thread mythread1(print1);
                  thread mythread3(print3);
         thread mythread2(print2);


                 

    
/*
            if (pthread_create(&p_thread_udp, &pa, OnSatrtUdpServer, this) != 0) {
			TEST_ERROR_S(Create OnSatrtUdpServer Thread failed!);
			return false;
        }
		TEST_INFO_S(Create OnSatrtUdpServer Thread successed!);

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			TEST_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
*/        

 cout<<1111111<<endl;  



        
    sleep(100000);
    cout<<1111111<<endl;  
}
