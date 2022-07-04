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
   You can alse parse data in parallel.
    ```
    make pre-p DATA='XXX'
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
    -m <folder_path> <support> <graph_type> 
    <max_edges_num> <max_nodes_num> <debug_msg> 
    <given_seed_node_id> <use_predicted_inv_column> 
    <throw_nodes_after_iterations> <postpone_nodes_after_iterations> 
    : mine frequent modules and store them
    -q <folder_path>            : quit and store data
    -f <string>                 : filter attribute
    -u <string>                 : query attribute
    -r <file_path1> <file_path2>: replace
    --rollback                  : roll back one replace operation through log

    please input your cmd:
    ```
  Just follow the cmd list and use it.

  Here we provide some sample:
    ```bash
    -l ../pre_data/aes_core.hierarchy.v.table/aes_core
    -x ../output_data/networkx.xml
    -s ../output_data
    -q ../output_data
    -m ../output_data 100 0 -1 -1 false -1 true false 10000000
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
|`-m <folder_path>`| mine frequent module, user can specify the arguments to set different support threshold, limit the size of the module, restrict the mining process based on the seed node, throw the candidate in case of the long-time searching process and so on.|
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
├── pre_src
│   └── pre.cpp
├── src
│   ├── core.h
│   ├── Edge.hpp
│   ├── get_mem.cpp
│   ├── Graph.hpp
│   ├── io.hpp
│   ├── Log.hpp
│   ├── Node.hpp
│   ├── Storage.hpp
│   ├── System.hpp
│   ├── Trie.hpp
│   ├── UI.hpp
│   └── util.cpp
├── test
│   ├── io_test.cpp
│   ├── perf_test.cpp
│   ├── run_test.cpp
│   └── trie_test.cpp
├── CMakeLists.txt
├── main.cpp
├── Makefile
```
## Hint
Any problem, please contact me: `xwcai98@zju.edu.cn`.