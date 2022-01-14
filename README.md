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
