#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// quick sort
void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}
int Partition(int *arr, int front, int end){
    int pivot = arr[end];
    int i = front -1;
    for (int j = front; j < end; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    i++;
    swap(&arr[i], &arr[end]);
    return i;
}
void QuickSort(int *arr, int front, int end){
    if (front < end) {
        int pivot = Partition(arr, front, end);
        QuickSort(arr, front, pivot - 1);
        QuickSort(arr, pivot + 1, end);
    }
}

int main(void){

    FILE * fp;
    fp = fopen("100_service.txt", "w+");
    FILE * fp1;
    fp1 = fopen("100_inter_arrival.txt", "w+");
    FILE * fp2;    
    fp2 = fopen("simulation_result.txt", "w+");

    float lambda[5] = {1,1.5,2,2.5,3};
    int mu[4] = {4,2,2,3}; // M/M/1 and M/M/2 service time, there are two '2' because M/M/2 need two servers
    float arrival_time[5][10000] = {0}; //arrival time for lambda = 1,1.5,2,2.5
    float inter_arrival_time[5][10000] = {0};
    float MM1_service_time[10000] = {0};
    float MM2_service_time[2][10000] = {0};    
    float x, F; // x is the inter arrival time with exponential distribution
    float MM1_waiting_time[4][10000] = {0};
    float MM2_waiting_time[4][10000] = {0};
    float mean_waiting_time[4] = {0}; // mean waiting time of MM1
    float MM2_mean_waiting_time[4] = {0}; // mean waiting time of MM2
    float departure_time[4][10000] = {0}; // departure time of MM1 
    float departure_time_1[4][10000] = {0}; // departure time of packet goes to server1 (MM2)
    float departure_time_2[4][10000] = {0}; // departure time of packet goes to server2 (MM2) 
    float MM2_departure_time[4][10000] = {0}; // departure_time_1[4][10000] and departure_time_2[4][10000] add together
    float MM1k_service_time[10000] = {0}; // M/M/1/k service time (mu = 3)
    int MM1k_max_system_length[3] = {2,4,6}; // M/M/1/k's k
    float MM1k_raw_inter_arrival_time[2][10000] = {0};    
    float MM1k_raw_arrival_time[2][10000] = {0}; // row 0 with lambda = 2 ; row 1 with lambda = 3
    float MM1k_raw_departure_time[2][10000] = {0};
    float MM1k_raw_waiting_time[2][10000] = {0};
    float MM1k_arrival_time[2][10000] = {0}; // save the unblock packets' arrival time
    float k_2_arrival[2][10000] = {0};
    float k_2_departure[2][10000] = {0}; 
    float k_4_arrival[2][10000] = {0};      
    float k_4_departure[2][10000] = {0};
    float k_6_arrival[2][10000] = {0};
    float k_6_departure[2][10000] = {0};

    srand( time((NULL)) );

    // generate four inter arrival time -> arrival time
    for(int z=0 ; z<5 ; z++){
        
        fprintf(fp1, "lambda == %.1f |\t",lambda[z]); 
        
        for(int i=0; i<10000; i++){

            double F = (double) rand() / RAND_MAX; // 產生 [0, 1] 的浮點數亂數
            x = -log(1-F) / lambda[z];
            inter_arrival_time[z][i] = x;

            if(i == 0)
                arrival_time[z][i] = x; //the arrivla time of the first packet
            else
                arrival_time[z][i] = x + arrival_time[z][i-1]; // the arrival time of second to the last packets       
            if(i<=100)
                fprintf(fp1, "%.3lf\t",x); //record the first 100 inter arrival time for assignment 1

        }
        fprintf(fp1, "\n"); 
    }

    // generate M/M/1 and M/M/2 service time
    for(int z=0 ; z<4 ; z++){

        if(z == 0)
            fprintf(fp, "M/M/1 | mu == %d |\t",mu[z]); 
        else if(z==1||z==2)
            fprintf(fp, "M/M/2 | mu == %d |\t",mu[z]); 
        else
            fprintf(fp, "M/M/1/k | mu == %d |\t",mu[z]); 

        for(int i=0; i<10000; i++){

            double F = (double) rand() / RAND_MAX; // 產生 [0, 1] 的浮點數亂數
            x = -log(1-F) / mu[z];

            if(z == 0)
                MM1_service_time[i] = x; //the service time of the M/M/1 model
            else if(z==1||z==2)
                MM2_service_time[z-1][i] = x; //the service time of the M/M/2 model
            else
                MM1k_service_time[i] = x;   //the service time of the M/M/1/k model
                    
            if(i<=100)
                fprintf(fp, "%.3lf\t",x); //record the first 100 inter arrival time for assignment 1
        }
        fprintf(fp, "\n"); 
    }

    fclose(fp);
    fclose(fp1);

    // caculate M/M/1 mean waiting time in queue
    for(int z=0 ; z<4 ; z++){ 
        // first packet's waiting time is 0
        float w = 0, w_tmp = 0;
        departure_time[z][0] = arrival_time[z][0] + MM1_service_time[0];
        for(int i=0; i<10000; i++){
            w_tmp = 0;
            if(i != 0){
                w_tmp = w + MM1_service_time[i-1] - inter_arrival_time[z][i];                                
                if(w_tmp < 0)
                    w_tmp = 0;
                MM1_waiting_time[z][i] = w_tmp; 
                w = w_tmp;
                departure_time[z][i] = arrival_time[z][i] + MM1_service_time[i] + MM1_waiting_time[z][i];
            }                     
        }
        float tmp = 0;
        for(int i=0; i<10000; i++)
            tmp = MM1_waiting_time[z][i] + tmp;

        mean_waiting_time[z] = tmp/10000;
        tmp =0;

        // print result in terminal
        if(z==0){
            printf("MM1 mean waiting time in queue\n");
            printf("-----------------------------------------------------------\n");
            printf("lambda=1\tlambda=1.5\tlambda=2\tlambda=2.5\n"); 
        } 
        printf("%f\t", mean_waiting_time[z]);
        if(z==3){
            printf("\n");    
            printf("-----------------------------------------------------------\n");        
        }
 
    }

    // calculate M/M/2 waiting time in queue
    float *ptr1 = MM2_service_time[0];
    float *ptr2 = MM2_service_time[1]; 
    for(int z=0;z<4;z++){
        float *ptr3 = departure_time_1[z];
        float *ptr4 = departure_time_2[z];  // record where the last non -1 value are      

        float w_tmp1 = 0, w_tmp2 = 0;
        ptr1 = MM2_service_time[0];
        ptr2 = MM2_service_time[1];


        for(int i=0; i<10000; i++){
            w_tmp1 = 0, w_tmp2 = 0;
            // first packet
            if(i == 0){
                float server = (float) rand() / RAND_MAX; // 產生 [0, 1] 的浮點數亂數
                if(server >= 0.5){    
                    departure_time_1[z][i] = arrival_time[z][i] + *ptr1; // if random number > 0.5 goes to server1                                                            
                    ptr1++;
                    ptr3 = departure_time_1[z];
                }
                else{
                    departure_time_2[z][i] = arrival_time[z][i] + *ptr2;
                    ptr2++;     
                    ptr4 = departure_time_2[z];                  
                }
            }
            if(i != 0){
                // two servers all empty (waiting time is zero, no need to adjust the value in waiting time array)
                if( (*ptr3 - arrival_time[z][i]<0) && (*ptr4 - arrival_time[z][i]<0) ){
                    float server = (float) rand() / RAND_MAX; // 產生 [0, 1] 的浮點數亂數
                    // if random number > 0.5 goes to server1
                    if(server >= 0.5){    
                        departure_time_1[z][i] = arrival_time[z][i] + *ptr1;                                                             
                        ptr1++;
                        ptr3 = departure_time_1[z] + i;
                    }
                    else{
                        departure_time_2[z][i] = arrival_time[z][i] + *ptr2;
                        ptr2++;     
                        ptr4 = departure_time_2[z] + i;                  
                    }
                }
                // server1 one empty, server2 busy
                else if( (*ptr3 - arrival_time[z][i]<0) && (*ptr4 - arrival_time[z][i]>0) ){                    
                    departure_time_1[z][i] = arrival_time[z][i] + *ptr1;                                                          
                    ptr1++;
                    ptr3 = departure_time_1[z] + i;                    
                }
                // server1 one busy, server2 empty                
                else if( (*ptr3 - arrival_time[z][i]>0) && (*ptr4 - arrival_time[z][i]<0) ){
                    departure_time_2[z][i] = arrival_time[z][i] + *ptr2;
                    ptr2++;     
                    ptr4 = departure_time_2[z] + i; 
                }

                // two server are busy
                else{
                    w_tmp1 = *ptr3 - arrival_time[z][i];
                    w_tmp2 = *ptr4 - arrival_time[z][i];
                    if(w_tmp1<w_tmp2){
                        MM2_waiting_time[z][i] = w_tmp1;
                        departure_time_1[z][i] = arrival_time[z][i] + *ptr1 + w_tmp1;
                        ptr1++;
                        ptr3 = departure_time_1[z] + i;                        
                    }
                    else{
                        MM2_waiting_time[z][i] = w_tmp2;
                        departure_time_2[z][i] = arrival_time[z][i] + *ptr2 + w_tmp2;
                        ptr2++;
                        ptr4 = departure_time_2[z] + i; // point the last left packet departure time of server2                   
                    }
                }                                   
            }
        }
    }   
    
    // calculate M/M/2 mean waiting time in queue
    float tmp = 0;
    for(int a=0;a<4;a++){
        tmp = 0;
        for(int b=0;b<10000;b++){
            tmp = MM2_waiting_time[a][b] + tmp;
        }
        if(departure_time_1[a][9999] != 0)
            MM2_mean_waiting_time[a] = tmp/10000;
        else
            MM2_mean_waiting_time[a] = tmp/10000;            

        if(a==0){
            printf("MM2 mean waiting time in queue\n");
            printf("-----------------------------------------------------------\n");
            printf("lambda=1\tlambda=1.5\tlambda=2\tlambda=2.5\n"); 
        } 
        printf("%f\t", MM2_mean_waiting_time[a]);
        if(a==3){
            printf("\n");    
            printf("-----------------------------------------------------------\n");        
        }
    }    
    
    // calculate the mean queue length of MM1
    int count_arrival = 0, count_departure = 0;
    int number_in_system = 0, number_in_queue = 0;
    int second = 0;
    for(int z=0;z<4;z++){
        number_in_system = 0;
        number_in_queue = 0;
        count_arrival = 0;
        count_departure = 0;

        for(second=0 ; second< ((int)departure_time[z][9999]+1) ;second++){
            while((second<=(int)arrival_time[z][count_arrival]) && ((int)arrival_time[z][count_arrival]<(second+1))){
                number_in_system = number_in_system + 1; // find one packet arrives the system during i sec to i+1 sec
                count_arrival = count_arrival + 1;
            }
            while((second<=(int)departure_time[z][count_departure]) && ((int)departure_time[z][count_departure]<(second+1))){
                number_in_system = number_in_system - 1; // find one packet arrives the system during i sec to i+1 sec
                count_departure = count_departure + 1;
            }
            if((number_in_system - 1) > 0){
                number_in_queue = number_in_queue + number_in_system - 1;
            }
        }
        // print result
        if(z==0){
            printf("MM1 mean queue length\n");
            printf("-----------------------------------------------------------\n");
            printf("lambda=1\tlambda=1.5\tlambda=2\tlambda=2.5\n");
            fprintf(fp2,"MM1 mean queue length\n");
            fprintf(fp2,"-----------------------------------------------------------\n");
            fprintf(fp2,"lambda=1\tlambda=1.5\tlambda=2\tlambda=2.5\n");

        }       
        printf("%f\t", (float)number_in_queue/departure_time[z][9999]);
        fprintf(fp2,"%f\t", (float)number_in_queue/departure_time[z][9999]);        
        if(z==3){
            printf("\n");    
            printf("-----------------------------------------------------------\n");   
            fprintf(fp2,"\n");    
            fprintf(fp2,"-----------------------------------------------------------\n");                 
        }
    }

    // merge the departure time of server1 and server2 to an array also do sorting
    for(int z=0;z<4;z++){
        for(int a=0;a<10000;a++){
            MM2_departure_time[z][a] = floor(departure_time_1[z][a] + departure_time_2[z][a]);
        }
        QuickSort((int*)MM2_departure_time[z],0,9999);
    }

    // calculate the mean queue length of MM2
    for(int z=0;z<4;z++){
        number_in_system = 0;
        number_in_queue = 0;
        count_arrival = 0;
        count_departure = 0;

        for(second=0 ; second< ((int)MM2_departure_time[z][9999]+1) ;second++){
            while(second==(int)arrival_time[z][count_arrival]){
                number_in_system = number_in_system + 1; // find one packet arrives the system during i sec to i+1 sec
                count_arrival = count_arrival + 1;
            }
            while(second==(int)MM2_departure_time[z][count_departure]){
                number_in_system = number_in_system - 1; // find one packet arrives the system during i sec to i+1 sec
                count_departure = count_departure + 1;
            }

            if((number_in_system - 2) > 0){
                number_in_queue = number_in_queue + number_in_system - 2;
            }
        }

        // print result
        if(z==0){
            printf("MM2 mean queue length\n");
            printf("-----------------------------------------------------------\n");
            printf("lambda=1\tlambda=1.5\tlambda=2\tlambda=2.5\n"); 
            fprintf(fp2,"MM2 mean queue length\n");
            fprintf(fp2,"-----------------------------------------------------------\n");
            fprintf(fp2,"lambda=1\tlambda=1.5\tlambda=2\tlambda=2.5\n"); 
        }       
        printf("%f\t", (float)number_in_queue/MM2_departure_time[z][9999]);
        fprintf(fp2,"%f\t", (float)number_in_queue/MM2_departure_time[z][9999]);        
        if(z==3){
            printf("\n");    
            printf("-----------------------------------------------------------\n");  
            fprintf(fp2,"\n");    
            fprintf(fp2,"-----------------------------------------------------------\n");        
        }        
    }

    // save the arrival time and inter arrival time of M/M/1/k in another array
    for(int z=2;z<5;z=z+2){
        for(int i=0;i<10000;i++){
            MM1k_raw_arrival_time[(z-2)/2][i] = arrival_time[z][i];
            MM1k_raw_inter_arrival_time[(z-2)/2][i] = inter_arrival_time[z][i];           
        }
    }

    // calculate MM1k raw departure time
    for(int z=0;z<2;z++){
        float w = 0, w_tmp = 0;
        MM1k_raw_departure_time[z][0] = MM1k_raw_arrival_time[z][0] + MM1k_service_time[0];
        for(int i=0; i<10000; i++){
            w_tmp = 0;
            if(i != 0){
                w_tmp = w + MM1k_service_time[i-1] - MM1k_raw_inter_arrival_time[z][i];                                
                if(w_tmp < 0)
                    w_tmp = 0;
                w = w_tmp;
                MM1k_raw_departure_time[z][i] = MM1k_raw_arrival_time[z][i] + MM1k_service_time[i] + w;
            }                   
        }
    }

    // copy so that we can deal with raw array directly
    float tmp_MM1k_arrival[2][10000] = {0};    
    float tmp_MM1k_departure[2][10000] = {0};
    for(int z=0;z<2;z++){
        for(int i =0;i<10000;i++){
            tmp_MM1k_arrival[z][i] = MM1k_raw_arrival_time[z][i];
            tmp_MM1k_departure[z][i] = MM1k_raw_departure_time[z][i];
        }
    }

    // calculate the departure time and the packet be blocked
    int arrival_number = 0; // how many packets had came to the system
    int departure_number = 0; // how many apckets had left the system
    int seen_departure = 0;
    int blocked_amount = 0; // save how many packet been blocked
    float block_rate = 0;
    for(int a=0;a<3;a++){
        for(int z=0;z<2;z++){
            float *ptr5 = MM1k_raw_departure_time[z]; //point to the last nonzero(unblock) departure time
            arrival_number = 1;
            departure_number = 0;
            seen_departure = 0;
            blocked_amount = 0;
            for(int i=0;i<10000;i++){
                if(i != 0){
                    arrival_number = arrival_number + 1;               
                    while(( (tmp_MM1k_arrival[z][i-1]<=MM1k_raw_departure_time[z][seen_departure]) && (MM1k_raw_departure_time[z][seen_departure]<tmp_MM1k_arrival[z][i])) || (MM1k_raw_departure_time[z][seen_departure] == 0)){

                        seen_departure = seen_departure + 1; 

                        if(MM1k_raw_departure_time[z][seen_departure]==0)
                            continue;
                        departure_number = departure_number + 1; // see one packet departure
                    }

                    // block
                    if(arrival_number - departure_number == MM1k_max_system_length[a]+1){
                        arrival_number = arrival_number - 1;
                        MM1k_raw_departure_time[z][i] = 0;
                        MM1k_raw_arrival_time[z][i] = 0;
                        blocked_amount = blocked_amount + 1;
                   
                    }
                    //unblock
                    else{
                        if(*ptr5>MM1k_raw_arrival_time[z][i])
                            MM1k_raw_departure_time[z][i] = *ptr5 + MM1k_service_time[i];
                        else
                            MM1k_raw_departure_time[z][i] = MM1k_raw_arrival_time[z][i] + MM1k_service_time[i];
                        ptr5 = MM1k_raw_departure_time[z] + i;
                    }
                }
            }
            block_rate = blocked_amount/(float)10000;
            if(z==0){
                printf("M/M/1/%d blocking rate\n",MM1k_max_system_length[a]);
                printf("-----------------------------------------------------------\n");
                printf("lambda = 2\tlambda = 3\n");
                fprintf(fp2,"M/M/1/%d blocking rate\n",MM1k_max_system_length[a]);
                fprintf(fp2,"-----------------------------------------------------------\n");
                fprintf(fp2,"lambda = 2\tlambda = 3\n");
            }
            printf("%f\t",block_rate);
            fprintf(fp2,"%f\t",block_rate);            
        }
        printf("\n-----------------------------------------------------------\n"); 
        fprintf(fp2,"\n-----------------------------------------------------------\n");          
        // replace raw array with tmp array
        for(int h=0;h<2;h++){
            for(int j=0;j<10000;j++){
                if(a==0){
                    k_2_arrival[h][j] = MM1k_raw_arrival_time[h][j];
                    k_2_departure[h][j] = MM1k_raw_departure_time[h][j];
                }
                else if(a==1){
                    k_4_arrival[h][j] = MM1k_raw_arrival_time[h][j];
                    k_4_departure[h][j] = MM1k_raw_departure_time[h][j];
                }
                else{
                    k_6_arrival[h][j] = MM1k_raw_arrival_time[h][j];
                    k_6_departure[h][j] = MM1k_raw_departure_time[h][j];                    
                }
                MM1k_raw_departure_time[h][j] = tmp_MM1k_departure[h][j];
                MM1k_raw_arrival_time[h][j] = tmp_MM1k_arrival[h][j];
            }
        }
    } 

    // calcualate mean queue length
    float tmp_arrival[2][10000];
    float tmp_departure[2][10000];
    for(int a=0;a<3;a++){
        if(a==0){
            for(int h=0;h<2;h++){              
                for(int j=0;j<10000;j++){
                    if(k_2_arrival[h][j]!=0){
                        tmp_arrival[h][j] = floor(k_2_arrival[h][j]);
                        tmp_departure[h][j] = floor(k_2_departure[h][j]);
                    }                      
                }
                QuickSort((int*)tmp_arrival[h],0,9999);
                QuickSort((int*)tmp_departure[h],0,9999);
            }
        }
        else if(a==1){
            for(int h=0;h<2;h++){
                for(int j=0;j<10000;j++){
                    if(k_4_arrival[h][j]!=0){
                    tmp_arrival[h][j] = floor(k_4_arrival[h][j]);
                    tmp_departure[h][j] = floor(k_4_departure[h][j]);                    
                    }                     
                }
                QuickSort((int*)tmp_arrival[h],0,9999);
                QuickSort((int*)tmp_departure[h],0,9999); 
            }
        }
        else{
            for(int h=0;h<2;h++){
                for(int j=0;j<10000;j++){
                    if(k_6_arrival[h][j]!=0){
                        tmp_arrival[h][j] = floor(k_6_arrival[h][j]);
                        tmp_departure[h][j] = floor(k_6_departure[h][j]);
                    }                   
                }
                QuickSort((int*)tmp_arrival[h],0,9999);
                QuickSort((int*)tmp_departure[h],0,9999); 
            } 
        }
        for(int z=0;z<2;z++){
            number_in_system = 0;
            number_in_queue = 0;
            count_arrival = 0;
            count_departure = 0;
            for(second=0 ; second< (int)(tmp_departure[z][9999]+1) ;second++){
                // pass through the blocked packets
                while(tmp_arrival[z][count_arrival]==0){
                    count_arrival = count_arrival + 1;
                    count_departure = count_departure + 1;
                }
                // count how many packets had came until now (e.g. 4 sec)
                while(second == ((int)tmp_arrival[z][count_arrival])){ 
                    number_in_system = number_in_system + 1; // find one packet arrives the system during i sec to i+1 sec
                    count_arrival = count_arrival + 1;
                }
                // count how many packets had left until now (e.g. 4 sec)
                while(second == ((int)tmp_departure[z][count_departure])){
                    number_in_system = number_in_system - 1; // find one packet arrives the system during i sec to i+1 sec
                    count_departure = count_departure + 1;
                }
                if( (number_in_system - 1) > 0){
                    number_in_queue = number_in_queue + number_in_system - 1;
                }
            }
            if(z==0){
                printf("M/M/1/%d mean queue length\n",MM1k_max_system_length[a]);
                printf("-----------------------------------------------------------\n");
                printf("lambda = 2\tlambda = 3\n");
                fprintf(fp2,"M/M/1/%d mean queue length\n",MM1k_max_system_length[a]);
                fprintf(fp2,"-----------------------------------------------------------\n");
                fprintf(fp2,"lambda = 2\tlambda = 3\n");
            }
            printf("%f\t",(float)number_in_queue/(float)tmp_departure[z][9999]); 
            fprintf(fp2,"%f\t",(float)number_in_queue/(float)tmp_departure[z][9999]);             
        }
        printf("\n-----------------------------------------------------------\n");
        fprintf(fp2,"\n-----------------------------------------------------------\n");  
    }

    fclose(fp2);        
}