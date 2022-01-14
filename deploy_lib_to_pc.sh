echo "Input your PC ip address:"
read PC_IP
echo "Input your PC password:"
read PC_PASS
echo Deploying required libs to PC: ${PC_IP}......
sshpass -p ${PC_PASS} scp /usr/local/lib/libboost_regex.so.1.56.0 nutanix@${PC_IP}:tmp
sshpass -p ${PC_PASS} scp /usr/local/lib/libopentracing.so.1 nutanix@${PC_IP}:tmp
sshpass -p ${PC_PASS} scp /usr/local/lib/libthrift-0.11.0.so nutanix@${PC_IP}:tmp
sshpass -p ${PC_PASS} scp /usr/local/lib64/libjaegertracing.so.0 nutanix@${PC_IP}:tmp
sshpass -p ${PC_PASS} ssh nutanix@${PC_IP} sudo mv /home/nutanix/tmp/libboost_regex.so.1.56.0 /usr/local/lib
sshpass -p ${PC_PASS} ssh nutanix@${PC_IP} sudo mv /home/nutanix/tmp/libopentracing.so.1 /usr/local/lib
sshpass -p ${PC_PASS} ssh nutanix@${PC_IP} sudo mv /home/nutanix/tmp/libthrift-0.11.0.so /usr/local/lib
sshpass -p ${PC_PASS} ssh nutanix@${PC_IP} sudo mv /home/nutanix/tmp/libjaegertracing.so.0 /usr/local/lib64
echo Deployment finished!
