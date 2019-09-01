# DeadlockDetection
###### Deadlock detection and predict



URL link https://youtu.be/0byCj3EWGvg


#### build script
##### 0. make

##### 1. abba deadlock detect
`LD_PRELOAD=./DDectector.so ./abba`

##### 2. abba deadlock detect + backtrace
`LD_PRELOAD=./Dmonitor.so ./abba
cat dmonitor.trace
./DPredict`

##### 3. philosopher dinning deadlock detect + backtrace
`LD_PRELOAD=./Dmonitor.so ./dinning`
`cat dmonitor.trace`
`./DPredict`

###### LD_PRELOAD=./Dmonitor.so ./Single_threaded_example
###### cat dmonitor.trace
###### ./DPredict


###### LD_PRELOAD=./Dmonitor.so ./Gate_Lock_exampe
###### cat dmonitor.trace
###### ./DPredict

###### LD_PRELOAD=./Dmonitor.so ./Thread_Creation_example
###### cat dmonitor.trace
###### ./DPredict
