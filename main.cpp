#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <vector>
#include <set>
#include <math.h>
using namespace std;


class Argument {
    public:
        int nr_of_files;
        int close_files = 0;
        string name;
        pthread_mutex_t mutex;
        fstream file_input;
        pthread_barrier_t barrier;
        int M;
        int R;   
        vector <vector<set<int> > > vec;
        vector<set<int> > reduce;
};

class Change_ID {
    public:
        int ID;
        Argument *arg;
};

double multiplay_mid(double num, int n){
    double ans = 1.0;
    for(int i = 1; i <= n; i++)
    {
        ans = ans * num;
    }
    return ans;
}

/**
 * @brief Get the Nth sqrt object
 * 
 * @param root 
 * @param number 
 * @return Nth root of a number
 */
int get_Nth_sqrt(int root, int number){
    double low = 1;
    double high = number;
    double eps = 1e-7;

    if(number == 1)
        return 1;

    while((high - low) > eps)
    {
        double mid = (low + high) / 2.0;
        if(multiplay_mid(mid, root) < number)
            low = mid;
        else
            high = mid;
    }
    low = ceil(low);

    if(pow(low, root) == number)
    {
        return number;
    }
    
    return -1;
}

/**
 * @brief fill three dimensional vector with numbers from files
 by nth root(set pozition, wich are root - 2) at Thread_ID - 1
 position in structure 
 * 
 * @param reduce_threads 
 * @param thread_ID 
 * @param number 
 * @param arg 
 */
void maping(int reduce_threads, int thread_ID, int number, Argument *arg){
    for(int root = 2; root <= reduce_threads + 1; root++)
    {

        if(get_Nth_sqrt(root, number) >= 1)
        {
            ((arg->vec.at(thread_ID - 1)).at(root - 2)).insert(number);
        }
    }
}

/**
 * @brief before filling three dimensional vector, name of files are readed
 into a mutex and opened for reading of numbers
  * 
 * @param arg 
 */
void* map (void* arg){
    Change_ID *change_id = static_cast<Change_ID*>(arg);
    Argument *argument = change_id->arg;
    string opened_file;
    int count;
    while (true)
    {
        pthread_mutex_lock(&argument->mutex);
        argument->file_input >> opened_file;
        count = ++argument->close_files;

        pthread_mutex_unlock(&argument->mutex);
        if(count > argument->nr_of_files)
            break;
        
        fstream file_test;
        file_test.open(opened_file, ios::in);
        int all_numbers;
        int number;
        file_test >> all_numbers;

        for(int i = 0; i < all_numbers; i++)
        {
            file_test >> number;
            maping(argument->R, change_id->ID, number, argument);
        }
        void close();
    }
    return NULL;
    
}

/**
 * @brief fill the two dimensional vector which will contains numbers that
 have Nth root begining with root 2 at pozition 0
 * 
 * @param arg 
 */
void reduce(void* arg){
    Change_ID *change_id = static_cast<Change_ID*>(arg);
    Argument *argument = change_id->arg;
    fstream file_out;

    
    for(int i = 0; i < argument->M; i++)
    {
        for(int num : argument->vec[i][change_id->ID - argument->M - 1])
        {
            (argument->reduce.at(change_id->ID - argument->M - 1)).insert(num);
        }
    }
    string file = "out" + to_string(change_id->ID - argument->M + 1) + ".txt";
    file_out.open(file, ios::out);
    file_out << argument->reduce[change_id->ID - argument->M - 1].size();   

}

/**
 * @brief based on ID of thread, choose between map or reduce
 * 
 * @param arg 
 */
void* map_or_reduce(void* arg){

    Change_ID *change_id = static_cast<Change_ID*>(arg);
    if(change_id->ID <= change_id->arg->M)
    {
        map(arg);
    }

    pthread_barrier_wait(&change_id->arg->barrier);

    if(change_id->ID > change_id->arg->M)
    {
        reduce(arg);
    }

    pthread_exit(NULL);
}



int main(int argc, char** argv)
{
    int M = atoi(argv[1]);
    int R = atoi(argv[2]);
    string file_in = argv[3];
    int nr_of_threads = M + R;
    pthread_t mappers_reducers[nr_of_threads];

    pthread_mutex_t mutex;
    pthread_barrier_t barrier;
    pthread_mutex_init(&mutex, NULL);
    Argument argument;
    argument.M = M;
    argument.R = R;
    argument.mutex = mutex;
    argument.barrier = barrier;

    argument.vec = vector <vector<set<int>>>(M, vector<set<int> >(R));
    argument.reduce = vector<set<int>>(R);
    argument.file_input.open(file_in, ios::in);
    if(!argument.file_input)
        cout << "File not created!";
    
    argument.file_input >> argument.nr_of_files;
    
    int r;
    
    pthread_barrier_init(&argument.barrier, NULL, nr_of_threads);
    pthread_mutex_init(&argument.mutex, NULL);
    Change_ID IDs [nr_of_threads];

    for(int i = 1; i <= nr_of_threads; i++)
    {
        IDs[i - 1].ID = i;
        IDs[i - 1].arg = &argument;
        r = pthread_create(&mappers_reducers[i], NULL, map_or_reduce, &IDs[i - 1]);
        if(r)
            cout << "Error at thread creating!";
    }

    for(int i = 1; i <= nr_of_threads; i++)
    {
        pthread_join(mappers_reducers[i], NULL);
    }

    pthread_mutex_destroy(&argument.mutex);
    pthread_barrier_destroy(&argument.barrier);
    
    return 0;
}