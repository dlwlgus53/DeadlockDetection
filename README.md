# DeadlockDetection

###### 2019년 1학기 OS 프로젝트3 Deadlock detection and predict

### ** 데드락 사이클 탐지를 활용한 데드락 detection and predict 프로그램 **

## Report/ Demo Video
##### [Report URL](https://github.com/dlwlgus53/DeadlockDetection/blob/master/Pa3.pdf)
##### [Demo Vidieo URL](https://youtu.be/0byCj3EWGvg)


## Build Script
##### 0. make

##### 1. abba deadlock detect
`LD_PRELOAD=./DDectector.so ./abba`

##### 2. abba deadlock detect + backtrace
```
LD_PRELOAD=./Dmonitor.so ./abba
cat dmonitor.trace
./DPredict
```

##### 3. philosopher dinning deadlock detect + backtrace
```
LD_PRELOAD=./Dmonitor.so ./dinning
cat dmonitor.trace
./DPredict
```
##### 4. deadlock like program(not a deadlock) : single_threaded 
```
LD_PRELOAD=./Dmonitor.so ./Single_threaded_example
cat dmonitor.trace
./DPredict
```

##### 5. deadlock like program(not a deadlock) : gate_lock

```
LD_PRELOAD=./Dmonitor.so ./Gate_Lock_exampe
cat dmonitor.trace
./DPredict
```
##### 6. deadlock like program(not a deadlock) : thread_creation
```
LD_PRELOAD=./Dmonitor.so ./Thread_Creation_example
cat dmonitor.trace
./DPredict
```
