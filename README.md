# axiomatic_query_expansion
This is for TREC query expansion.
Please refer to the paper for more details:
```
Hui Fang and ChengXiang Zhai. 2006. Semantic term matching in axiomatic approaches to information retrieval. In Proceedings of the 29th annual international ACM SIGIR conference on Research and development in information retrieval (SIGIR '06). ACM, New York, NY, USA, 115-122. 
```
```
Peilin Yang and Hui Fang. Evaluating the Effectiveness of Axiomatic Approaches in Web Track. In Proceedings of the 22nd Text REtreival Conference (TREC'2013), 2013.
```

**Usage:**

Use ```make``` to compile the binary.

The Makefile assumes that you have the indri-5.11 installed at `${HOME}/indri-bin`. Otherwise, please change the indri's path in Makefile and compile.

Please refer to the sample files folder for the sample inputs of the binary.


**NOTE:**

The query ids can only be digits (e.g. 123), otherwise the results would be strange (all queries have the same <text> field)
