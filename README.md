# Kendinsky
Project K.
## Prerequisites
At a minimum:

- GNU Make
- CMake 3.15+
- G++ (version that supports c++17)

## Build && Run
1. Go to the project's source folder.
    ```bash
    cd Kendinsky
    ```

2. Put init data in folder `data`, you can get data from following ways:
    + Use [OpenLane](https://github.com/The-OpenROAD-Project/OpenLane/tree/master) to tranform verilog into connection table.
    + We also provide some small data on [Google Drive](https://drive.google.com/drive/folders/1C6RBziY6cZXfd3lcZD8gQkuO9iqnonbb?usp=sharing) for easy testing.

3. It's a head-only project, so just use following command to build rely on the Makefile.
    ```bash
    make build
    ```
4. Parse data in advance.
    ```
    make pre DATA='XXX'
    ```
   `XXX` equals data name in data folder, for example: `make pre DATA='aes_core.hierarchy.v.table'`.

5. Use following command to run rely on the Makefile:
    ```bash
    make run
    ```

Congratulations, you have easily compiled and run the project.

Pay attention: **you only need build once, and all executable files are in folder `bin`**.

## Usage guide
+ Use `make pre DATA='XXX'` or `make pre-p DATA='XXX'` to parse data in advance, data can be hierarchy or not.

  After success parse, you would see parse data in folder `pre_data/XXX`, includes:
    + One file `.topo` represents modules' topological order.
    + One or more file `xxx.table` represents divided table file which have minor changes in data format.
    + One or more folder `xxx` represents data folder that can be load directly by Kendinsky.

+ Use `make run`, you would see:
    ```bash
    ***Project Kendinsky***
    cmd list:
    --help                      : print cmd list
    -l <folder_path>            : load data
    -x <file_path>              : save for networkx
    -s <folder_path>            : store data
    -m <store_path> 
       <file_name> 
       <support> 
       <max_edges_num> 
       <max_nodes_num> 
       <given_seed_node_id>
       <throw_nodes_after_iterations>
       <postpone_nodes_after_iterations>
                                :mining frequent module
    -q <folder_path>            : quit and store data
    -f <string>                 : filter attribute
    -u <string>                 : query attribute
    -r <file_path1> <file_path2>: replace
    --rollback                  : roll back one replace operation through log

    please input your cmd:
    ```
  Just follow the cmd list and use it.

  Since the mining method needs to input several parameters, the parameters are explained here.

### Explanation of mining method parameters
| parameters                          | value                                                                                | explanation                                                                                                                                                                  |
|-------------------------------------|--------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `<store_path>`                      | input the characters of path                                                         | the path to store mining result                                                                                                                                              |
| `<file_name>`                       | input the characters of file                                                         | the input file to mine frequent subgraph                                                                                                                                     |
| `<support>`                         | input an interger                                                                    |
| `<max_edges_num>`                   | input an interger, -1 for not set                                                    | the max num of edges in frequent subgraph                                                                                                                                    |
| `<max_nodes_num>`                   | input an interger, -1 for not set                                                    | the max num of nodes in frequent subgraph                                                                                                                                    |
| `<given_seed_node_id>`              | input an interger, -1 for not set                                                    | we can choose the seed node by specify its id, that is, candidate subgraphs are expanded based on this node and should all contain this node                                 |
| `<throw_nodes_after_iterations>`    | true for discard the timeout node, false for not                                     | decide whether to discard the nodes being checked for isomorphism to save mining time, if the data graph is big, and the support is small, we recommend that you set it true |
| `<postpone_nodes_after_iterations>` | input an interger, if <throw_nodes_after_iterations>=false, this param wouldn't work | when the number of calculating iterations exceeds the param, discard the domain's node which is being detected                                                               |

Here we provide some sample:
  ```bash
    -l ../pre_data/aes_core.hierarchy.v.table/aes_core
    -x ../output_data/networkx.xml
    -s ../output_data
    -q ../output_data
    -m ../output_data ../pre_data/aes_core.v.table 100 -1 -1 -1 true 50
    -m ../output_data ../pre_data/aes_core.v.table 200 -1 -1 -1 true 100
    -m ../output_data ../pre_data/aes_core.v.table 200 8 8 -1 true 100
    -m ../output_data ../pre_data/aes_core.v.table 200 -1 -1 7680 false 100
    -m ../output_data ../pre_data/aes_core.v.table 500 -1 -1 -1 false 100
    ...
  ```
Pay attention: **after run this project, you always need to use `-l` to get data from disks fist**.

Because the lack of API, the `-f -u -r --rollback` is still waiting todo.

When the project run cmd successfully, you would see `xxx success` in terminal.

When you encouter some problem, you may see some information in the terminal, and final get:
`xxx failed`.

Pay attention: **although you can end the project by breaking the run, we strongly recommend you use `-q` to quit**.

## More detailed features
| Cmd | Description |
| ---- | ---- |
|`-l <folder_path>`| load whole DB data from folder, demanding multi files(log.data, full_name.map, attribute.map, signal.map, storage.data, now.g, hid.g), folder_path represents load folder's path.|
|`-x <file_path>`| store now graph data into file for networkX, file_path represents store file's path.|
|`-s <folder_path>`| store whole DB data into folder, including multi files(log.data, full_name.map, attribute.map, signal.map, storage.data, now.g, hid.g), folder_path represents store folder's path. |
|`-q <folder_path>`| store whole DB data data into folder and quit, folder_path represents store folder's path.|
|...|...|
## Design
You can see the general design of the project in the figure below:
![avatar](./pic/design.jpg)

- Trie is used for mapping string to id.
- Cell equals Node.
- Signal equals multi Edges.
- Connection table equals graph.
- Now Graph represents present graph data.
- Actual graph datas are stored in Storage. You can think of those graphs as views of the actual data.
- Hid Graph and Log are used for replace opetion. For example, if we replace some data, put the before_nodes and before_edges into Hid Graph, put the after_nodes and after_edges into Now Graph, and write them into Log. And we can rollback replace operations through Log.

Pay attention: **the actual implementation may not exactly the same as the design drawing, please refer to the code for the specific implementation**.
## File architecture
```
├── CMakeLists.txt
├── framework
│   ├── CMakeLists.txt
│   ├── core.h
│   ├── Edge.hpp
│   ├── Graph.hpp
│   ├── io.hpp
│   ├── Log.hpp
│   ├── Node.hpp
│   ├── Storage.hpp
│   ├── System.hpp
│   ├── Trie.hpp
│   ├── UI.hpp
│   ├── util.cpp
│   └── util.hpp
├── main.cpp
├── Makefile
├── mining
│   ├── CMakeLists.txt
│   ├── core_file.h
│   ├── CSPSolver.cpp
│   ├── CSPSolver.h
│   ├── mining_utils.cpp
│   ├── mining_utils.h
│   ├── MyEdge.cpp
│   ├── MyEdge.h
│   ├── MyGraph.cpp
│   ├── MyGraph.h
│   ├── MyMiner.cpp
│   ├── MyMiner.h
│   ├── MyNode.cpp
│   ├── MyNode.h
│   ├── Pattern.cpp
│   ├── Pattern.h
│   ├── Settings.h
│   ├── SigMap.cpp
│   ├── SigMap.h
│   ├── Signature.cpp
│   └── Signature.h
├── pre_src
│   └── pre.cpp
└── test
    ├── io_test.cpp
    ├── mining_test.cpp
    ├── perf_test.cpp
    ├── run_test.cpp
    └── trie_test.cpp

```
## Hint
Any problem, please contact me: `xwcai98@zju.edu.cn`.