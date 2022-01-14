# opentracing-jaeger-cpp-sample

Build and Run:

```
cd opentracing-jaeger-cpp-sample/
mkdir build
cd build/
cmake ..
make
./Sample
```

The Jaeger config:
```
disabled: false
reporter:
  logSpans: true
  endpoint: http://prismdevtest.umvm.nutanix.com:14268/api/traces
sampler:
  samplingServerURL: http://prismdevtest.umvm.nutanix.com:5778/sampling  
  param: 1
```

Deploy the required libs to PC:(This requires sshpass installed in your dev vm)
```
# In case your have issue with sshpass not found
# sudo yum install sshpass
sh deploy_lib_to_pc.sh
```
