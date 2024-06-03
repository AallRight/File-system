README
---
### Step1
+ use make file to make execution file
+ the BDS command is 
```
./BDS <disk file name> <cylinders> <sectors> <track-to-track delay time> <port>
```
+ the BDC execution command is (so as the BDC_random)
```
./BDC <server address> <BDS port>
```
+ In BDC, the commands are as below
```
I
W <cylinder num> <sector num> <data length> data
R <cylinder num> <sector num>
E
```

### Step2
+ use makefile to generate the execution file
+ the BDS execution command is 
```
./BDS <disk file name> <cylinders> <sectors> <track-to-track delay time> <port>
```
+ the FS execution command is 
```
./FS <server address> <BDS port> <FS port>
```
+ the FC execution command is 
```
./FC <server address> <BDS port>
```
+ the commands in the file system are as below
```
f
mk f
mkdir d
rm f
cd path
rmdir d
ls
cat f
w f l data
i f pos l data
d f pos l
e
```

### Step3 
+ the command to execute BDS, FS, FC is the same as those in step2
+ the command in file system is also the same as those in step3
+ when the client connect to the FS, the first command is to enter the user or create user or delete user
```command
C <user name> <user password> // create a user
E <user name> <user password> // enter a user
D <user name> <user password> // delete a user
```

```
shift // shift from current directory to shared directory or from   
         shared directory to user root 
```
The concrete operations are shown in the report.