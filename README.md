# DeadlockDetection
###### Deadlock detection and predict



URL link https://youtu.be/0byCj3EWGvg


#### build script
###### 0. make
###### 1. LD_PRELOAD=./DDectector.so ./abba

###### 2. LD_PRELOAD=./Dmonitor.so ./abba 
cat dmonitor.trace
./DPredict

###### 3. LD_PRELOAD=./Dmonitor.so ./dinning
    cat dmonitor.trace
    ./DPredict
###### 4. LD_PRELOAD=./Dmonitor.so ./Single_threaded_example
    cat dmonitor.trace
    ./DPredict
LD_PRELOAD=./Dmonitor.so ./Gate_Lock_exampe
    cat dmonitor.trace
    ./DPredict
LD_PRELOAD=./Dmonitor.so ./Thread_Creation_example
    cat dmonitor.trace
    ./DPredict
